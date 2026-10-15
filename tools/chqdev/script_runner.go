package main

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"os"
	"path/filepath"
	"regexp"
	"sort"
	"strconv"
	"strings"
	"time"
)

type scriptItem struct {
	Kind       string
	Line       int
	Command    string
	Predicate  string
	ThenItems  []scriptItem
	ElseItems  []scriptItem
	BodyItems  []scriptItem
	FinalItems []scriptItem
}

type goScriptRunner struct {
	root, base, sourceRel, runName, purpose, release string
	runSource, runResolvedSource                     string
	client                                           *http.Client
	runID, sessionID, build                          string
	commandCount                                     int
	timings                                          []commandTiming
	runStarted                                       time.Time
}

type commandTiming struct {
	Sequence   int     `json:"sequence"`
	SourceLine int     `json:"sourceLine"`
	Command    string  `json:"command"`
	Category   string  `json:"category"`
	ElapsedMS  float64 `json:"elapsedMs"`
	Failed     bool    `json:"failed"`
}

type timingAggregate struct {
	Category string  `json:"category"`
	Count    int     `json:"count"`
	TotalMS  float64 `json:"totalMs"`
	MaxMS    float64 `json:"maxMs"`
}

func commandCategory(cmd string) string {
	fields := strings.Fields(strings.ToLower(strings.TrimSpace(cmd)))
	if len(fields) == 0 {
		return "empty"
	}
	if fields[0] == "api" && len(fields) > 1 {
		return "api " + fields[1]
	}
	return fields[0]
}

func (g *goScriptRunner) saveTimings() (string, error) {
	if g.runID == "" || g.sessionID == "" {
		return "", nil
	}
	dir := filepath.Join(g.root, "evidence", "sessions", g.sessionID, "runs", g.runID)
	if err := os.MkdirAll(dir, 0755); err != nil {
		return "", err
	}
	groups := map[string]*timingAggregate{}
	for _, t := range g.timings {
		a := groups[t.Category]
		if a == nil {
			a = &timingAggregate{Category: t.Category}
			groups[t.Category] = a
		}
		a.Count++
		a.TotalMS += t.ElapsedMS
		if t.ElapsedMS > a.MaxMS {
			a.MaxMS = t.ElapsedMS
		}
	}
	summary := make([]timingAggregate, 0, len(groups))
	for _, a := range groups {
		summary = append(summary, *a)
	}
	sort.Slice(summary, func(i, j int) bool { return summary[i].TotalMS > summary[j].TotalMS })
	elapsed := time.Since(g.runStarted).Milliseconds()
	report := struct {
		Schema         string            `json:"schema"`
		RunID          string            `json:"runId"`
		SessionID      string            `json:"sessionId"`
		TotalElapsedMS int64             `json:"totalElapsedMs"`
		Commands       []commandTiming   `json:"commands"`
		ByCategory     []timingAggregate `json:"byCategory"`
	}{"chqdev-command-timings-v1", g.runID, g.sessionID, elapsed, g.timings, summary}
	b, err := json.MarshalIndent(report, "", "  ")
	if err != nil {
		return "", err
	}
	path := filepath.Join(dir, "chqdev-command-timings.json")
	return path, os.WriteFile(path, append(b, '\n'), 0644)
}

var (
	reSet     = regexp.MustCompile(`(?i)^set\s+([A-Za-z_][A-Za-z0-9_]*)\s*=\s*(.*)$`)
	reFor     = regexp.MustCompile(`(?i)^for\s+(.+?)\s+in\s+(.+)$`)
	reRepeat  = regexp.MustCompile(`(?i)^repeat\s+(\d+)$`)
	reIf      = regexp.MustCompile(`(?i)^if\s+(.+)$`)
	reInclude = regexp.MustCompile(`(?i)^include\s+"([^"]+)"$`)
	reVar     = regexp.MustCompile(`\$\{([A-Za-z_][A-Za-z0-9_]*)\}`)
	reQuery   = regexp.MustCompile(`(?i)^(getregions|getprofiles|getsprites|getpaletteentries|getcheckpoints|getcapabilities|getmaps|getpalettes|getregistry|gettrackrecords|getevidence|getobjects|getaudiochannels)\((.*)\)$`)
)

func scriptCommandLimit() int {
	const def = 2048
	if raw := strings.TrimSpace(os.Getenv("CHQ_SCRIPT_MAX_COMMANDS")); raw != "" {
		if n, err := strconv.Atoi(raw); err == nil && n >= 256 && n <= 8192 {
			return n
		}
	}
	return def
}

func scriptDefaultVars(source string) map[string]string {
	vars := map[string]string{}
	re := regexp.MustCompile(`(?i)^\s*#\s*param\s+([A-Za-z_][A-Za-z0-9_]*)\s+(.*)$`)
	def := regexp.MustCompile(`(?i)(?:^|\s)default=([^\s]+)`)
	for _, line := range strings.Split(source, "\n") {
		m := re.FindStringSubmatch(line)
		if m == nil {
			continue
		}
		d := def.FindStringSubmatch(m[2])
		if d != nil {
			vars[m[1]] = d[1]
		}
	}
	return vars
}

func substituteScriptVars(s string, vars map[string]string) string {
	return reVar.ReplaceAllStringFunc(s, func(m string) string {
		sm := reVar.FindStringSubmatch(m)
		if v, ok := vars[sm[1]]; ok {
			return v
		}
		return m
	})
}

func expandScriptIncludes(root, scriptRoot, source string, depth int) (string, error) {
	if depth > 16 {
		return "", fmt.Errorf("include nesting exceeds 16")
	}
	source = strings.ReplaceAll(strings.ReplaceAll(source, "\r\n", "\n"), "\r", "\n")
	var out []string
	for _, line := range strings.Split(source, "\n") {
		t := strings.TrimSpace(line)
		m := reInclude.FindStringSubmatch(t)
		if m == nil {
			out = append(out, line)
			continue
		}
		rel := filepath.Clean(filepath.FromSlash(m[1]))
		if filepath.IsAbs(rel) || strings.HasPrefix(rel, ".."+string(filepath.Separator)) || rel == ".." {
			return "", fmt.Errorf("unsafe include path %q", m[1])
		}
		p := filepath.Join(root, filepath.FromSlash(scriptRoot), rel)
		b, err := os.ReadFile(p)
		if err != nil {
			return "", fmt.Errorf("include %s: %w", m[1], err)
		}
		ex, err := expandScriptIncludes(root, scriptRoot, string(bytes.TrimPrefix(b, []byte{0xEF, 0xBB, 0xBF})), depth+1)
		if err != nil {
			return "", err
		}
		out = append(out, "# included "+m[1], ex)
	}
	return strings.Join(out, "\n"), nil
}

func findScriptBlock(lines []string, open int, lineOffset ...int) (end, elseAt, finallyAt int, err error) {
	off := 0
	if len(lineOffset) > 0 {
		off = lineOffset[0]
	}
	depth, elseAt, finallyAt := 0, -1, -1
	for j := open + 1; j < len(lines); j++ {
		q := strings.TrimSpace(lines[j])
		low := strings.ToLower(q)
		if strings.HasPrefix(low, "for ") || strings.HasPrefix(low, "repeat ") || strings.HasPrefix(low, "if ") || low == "try" {
			depth++
			continue
		}
		if low == "end" {
			if depth == 0 {
				return j, elseAt, finallyAt, nil
			}
			depth--
			continue
		}
		if depth == 0 && low == "else" {
			elseAt = j
		}
		if depth == 0 && low == "finally" {
			finallyAt = j
		}
	}
	return -1, -1, -1, fmt.Errorf("line %d: missing end", off+open+1)
}

func parseTupleListGo(text string, arity int) ([][]string, error) {
	t := strings.TrimSpace(text)
	if len(t) < 2 || t[0] != '[' || t[len(t)-1] != ']' {
		return nil, fmt.Errorf("tuple loop source must use [(a,b),(c,d)] syntax")
	}
	body := strings.TrimSpace(t[1 : len(t)-1])
	if body == "" {
		return [][]string{}, nil
	}
	re := regexp.MustCompile(`\(([^()]*)\)`)
	matches := re.FindAllStringSubmatchIndex(body, -1)
	if len(matches) == 0 {
		return nil, fmt.Errorf("tuple loop source contains no tuples")
	}
	rows := [][]string{}
	pos := 0
	for _, m := range matches {
		between := strings.Trim(strings.TrimSpace(body[pos:m[0]]), ",")
		if strings.TrimSpace(between) != "" {
			return nil, fmt.Errorf("invalid tuple-list text %q", between)
		}
		raw := body[m[2]:m[3]]
		parts := strings.Split(raw, ",")
		for i := range parts {
			parts[i] = strings.TrimSpace(parts[i])
		}
		if len(parts) != arity {
			return nil, fmt.Errorf("tuple has %d fields; expected %d", len(parts), arity)
		}
		rows = append(rows, parts)
		pos = m[1]
	}
	if tail := strings.Trim(strings.TrimSpace(body[pos:]), ","); strings.TrimSpace(tail) != "" {
		return nil, fmt.Errorf("invalid tuple-list tail %q", tail)
	}
	return rows, nil
}

func compileScriptLines(lines []string, vars map[string]string) ([]scriptItem, error) {
	return compileScriptLinesAt(lines, vars, 0)
}

func compileScriptLinesAt(lines []string, vars map[string]string, lineOffset int) ([]scriptItem, error) {
	out := []scriptItem{}
	local := map[string]string{}
	for k, v := range vars {
		local[k] = v
	}
	for i := 0; i < len(lines); i++ {
		raw, t := lines[i], strings.TrimSpace(lines[i])
		if t == "" || strings.HasPrefix(t, "#") {
			continue
		}
		low := strings.ToLower(t)
		if low == "end" || low == "else" || low == "finally" {
			return nil, fmt.Errorf("line %d: unexpected %s", lineOffset+i+1, t)
		}
		if m := reSet.FindStringSubmatch(t); m != nil {
			local[m[1]] = substituteScriptVars(strings.TrimSpace(m[2]), local)
			continue
		}
		if m := reFor.FindStringSubmatch(t); m != nil {
			end, _, fin, e := findScriptBlock(lines, i, lineOffset)
			if e != nil {
				return nil, e
			}
			if fin >= 0 {
				return nil, fmt.Errorf("line %d: finally only valid in try", lineOffset+i+1)
			}
			var bindings []string
			for _, b := range strings.Split(strings.TrimSpace(m[1]), ",") {
				b = strings.TrimSpace(b)
				if !regexp.MustCompile(`^[A-Za-z_][A-Za-z0-9_]*$`).MatchString(b) {
					return nil, fmt.Errorf("line %d: invalid for binding %q", lineOffset+i+1, b)
				}
				bindings = append(bindings, b)
			}
			src := substituteScriptVars(strings.TrimSpace(m[2]), local)
			if v, ok := local[src]; ok {
				src = v
			}
			if len(bindings) == 1 {
				vals := []string{}
				for _, v := range strings.Split(src, ",") {
					if q := strings.TrimSpace(v); q != "" {
						vals = append(vals, q)
					}
				}
				if len(vals) > 256 {
					return nil, fmt.Errorf("line %d: for loop exceeds 256 iterations", lineOffset+i+1)
				}
				for _, v := range vals {
					nv := map[string]string{}
					for k, x := range local {
						nv[k] = x
					}
					nv[bindings[0]] = v
					items, e := compileScriptLinesAt(lines[i+1:end], nv, lineOffset+i+1)
					if e != nil {
						return nil, e
					}
					out = append(out, items...)
				}
			} else {
				rows, e := parseTupleListGo(src, len(bindings))
				if e != nil {
					return nil, fmt.Errorf("line %d: %w", lineOffset+i+1, e)
				}
				if len(rows) > 256 {
					return nil, fmt.Errorf("line %d: for loop exceeds 256 iterations", lineOffset+i+1)
				}
				for _, row := range rows {
					nv := map[string]string{}
					for k, x := range local {
						nv[k] = x
					}
					for j, name := range bindings {
						nv[name] = row[j]
					}
					items, e := compileScriptLinesAt(lines[i+1:end], nv, lineOffset+i+1)
					if e != nil {
						return nil, e
					}
					out = append(out, items...)
				}
			}
			i = end
			continue
		}
		if m := reRepeat.FindStringSubmatch(t); m != nil {
			n, _ := strconv.Atoi(m[1])
			if n < 0 || n > 256 {
				return nil, fmt.Errorf("line %d: repeat out of range", lineOffset+i+1)
			}
			end, _, _, e := findScriptBlock(lines, i, lineOffset)
			if e != nil {
				return nil, e
			}
			for k := 1; k <= n; k++ {
				nv := map[string]string{}
				for z, x := range local {
					nv[z] = x
				}
				nv["repeat_index"] = strconv.Itoa(k)
				items, e := compileScriptLinesAt(lines[i+1:end], nv, lineOffset+i+1)
				if e != nil {
					return nil, e
				}
				out = append(out, items...)
			}
			i = end
			continue
		}
		if m := reIf.FindStringSubmatch(t); m != nil {
			end, el, fin, e := findScriptBlock(lines, i, lineOffset)
			if e != nil {
				return nil, e
			}
			if fin >= 0 {
				return nil, fmt.Errorf("line %d: finally only valid in try", lineOffset+i+1)
			}
			thenEnd := end
			if el >= 0 {
				thenEnd = el
			}
			thenItems, e := compileScriptLinesAt(lines[i+1:thenEnd], local, lineOffset+i+1)
			if e != nil {
				return nil, e
			}
			var elseItems []scriptItem
			if el >= 0 {
				elseItems, e = compileScriptLinesAt(lines[el+1:end], local, lineOffset+el+1)
				if e != nil {
					return nil, e
				}
			}
			out = append(out, scriptItem{Kind: "if", Line: lineOffset + i + 1, Predicate: substituteScriptVars(strings.TrimSpace(m[1]), local), ThenItems: thenItems, ElseItems: elseItems})
			i = end
			continue
		}
		if low == "try" {
			end, el, fin, e := findScriptBlock(lines, i, lineOffset)
			if e != nil {
				return nil, e
			}
			if el >= 0 {
				return nil, fmt.Errorf("line %d: else invalid inside try", lineOffset+i+1)
			}
			if fin < 0 {
				return nil, fmt.Errorf("line %d: try requires finally", lineOffset+i+1)
			}
			body, e := compileScriptLinesAt(lines[i+1:fin], local, lineOffset+i+1)
			if e != nil {
				return nil, e
			}
			final, e := compileScriptLinesAt(lines[fin+1:end], local, lineOffset+fin+1)
			if e != nil {
				return nil, e
			}
			out = append(out, scriptItem{Kind: "try", Line: lineOffset + i + 1, BodyItems: body, FinalItems: final})
			i = end
			continue
		}
		_ = raw
		out = append(out, scriptItem{Kind: "command", Line: lineOffset + i + 1, Command: substituteScriptVars(t, local)})
	}
	return out, nil
}

func countScriptItems(items []scriptItem) int {
	n := 0
	for _, x := range items {
		switch x.Kind {
		case "if":
			n += countScriptItems(x.ThenItems) + countScriptItems(x.ElseItems)
		case "try":
			n += countScriptItems(x.BodyItems) + countScriptItems(x.FinalItems)
		default:
			n++
		}
	}
	return n
}

func (g *goScriptRunner) transient(line string) (string, error) {
	var run scriptRunResponse
	req := map[string]any{"script": line, "stopOnError": true, "validateOnly": false, "transient": true}
	if err := postJSON(g.client, g.base+"/api/v1/script", req, &run); err != nil {
		return "", err
	}
	if !run.OK {
		if len(run.Results) > 0 {
			return "", fmt.Errorf("%v", run.Results[0]["error"])
		}
		return "", fmt.Errorf("command failed: %s", line)
	}
	if len(run.Results) == 0 {
		return "", nil
	}
	return fmt.Sprint(run.Results[0]["output"]), nil
}

func jsonListField(raw, field string) ([]string, error) {
	var v any
	if err := json.Unmarshal([]byte(strings.TrimSpace(raw)), &v); err != nil {
		return nil, err
	}
	arr, ok := v.([]any)
	if !ok {
		return nil, fmt.Errorf("expected JSON array")
	}
	out := []string{}
	for _, x := range arr {
		if m, ok := x.(map[string]any); ok {
			if q, ok := m[field]; ok {
				out = append(out, fmt.Sprint(q))
			}
		} else {
			out = append(out, fmt.Sprint(x))
		}
	}
	return out, nil
}

func (g *goScriptRunner) queryFunction(expr string) ([]string, string, error) {
	m := reQuery.FindStringSubmatch(strings.TrimSpace(expr))
	if m == nil {
		return nil, "", fmt.Errorf("unknown query function %s", expr)
	}
	fn, arg := strings.ToLower(m[1]), strings.TrimSpace(m[2])
	action := ""
	field := ""
	args := ""
	switch fn {
	case "getregions":
		action, field = "regions.list", "name"
	case "getprofiles":
		action, field = "profiles.list", "id"
	case "getsprites":
		action, field = "sprites.list", "slot"
	case "getmaps":
		action = "sprites.maps"
	case "getpalettes":
		action = "sprites.palettes"
	case "getregistry":
		action, field = "registry.list", "id"
	case "getevidence":
		action, field = "evidence.names", "name"
	case "getobjects":
		action, field = "objects.list", "id"
	case "getaudiochannels":
		action, field = "audio.channels", "id"
	case "gettrackrecords":
		action, field = "track.records", "index"
		args = strings.ReplaceAll(arg, ",", " ")
	case "getpaletteentries":
		action, field = "palette.entries", "index"
		bm := regexp.MustCompile(`(?i)^bank\s*=\s*(\d+)$`).FindStringSubmatch(arg)
		if bm == nil {
			return nil, "", fmt.Errorf("getpaletteentries expects bank=N")
		}
		args = "bank=" + bm[1]
	case "getcapabilities":
		action = "capabilities"
	case "getcheckpoints":
		action = "checkpoint.list"
	}
	line := "api " + action
	if args != "" {
		line += " " + args
	}
	raw, err := g.transient(line)
	if err != nil {
		return nil, "", err
	}
	if fn == "getcapabilities" {
		return strings.Fields(strings.TrimSpace(strings.TrimPrefix(raw, "OK"))), action, nil
	}
	if fn == "getcheckpoints" {
		var j any
		if json.Unmarshal([]byte(raw), &j) == nil {
			switch x := j.(type) {
			case []any:
				o := []string{}
				for _, q := range x {
					switch z := q.(type) {
					case string:
						o = append(o, z)
					case map[string]any:
						if v := z["file"]; v != nil {
							o = append(o, fmt.Sprint(v))
						} else if v := z["name"]; v != nil {
							o = append(o, fmt.Sprint(v))
						}
					}
				}
				return o, action, nil
			case map[string]any:
				if a, ok := x["checkpoints"].([]any); ok {
					o := []string{}
					for _, q := range a {
						o = append(o, fmt.Sprint(q))
					}
					return o, action, nil
				}
			}
		}
		return nil, action, nil
	}
	if field == "" {
		var a []any
		if err := json.Unmarshal([]byte(raw), &a); err != nil {
			return nil, "", err
		}
		o := []string{}
		for _, q := range a {
			o = append(o, fmt.Sprint(q))
		}
		return o, action, nil
	}
	list, err := jsonListField(raw, field)
	if err != nil && fn == "getobjects" {
		list, err = jsonListField(raw, "name")
	}
	return list, action, err
}

func (g *goScriptRunner) resolveQueries(source string, defaults map[string]string) (string, error) {
	vars := map[string]string{}
	for k, v := range defaults {
		vars[k] = v
	}
	var out []string
	for _, raw := range strings.Split(source, "\n") {
		t := strings.TrimSpace(raw)
		if t == "" || strings.HasPrefix(t, "#") {
			out = append(out, raw)
			continue
		}
		if m := reSet.FindStringSubmatch(t); m != nil {
			rhs := substituteScriptVars(strings.TrimSpace(m[2]), vars)
			if reQuery.MatchString(rhs) {
				list, _, e := g.queryFunction(rhs)
				if e != nil {
					return "", e
				}
				val := strings.Join(list, ",")
				vars[m[1]] = val
				out = append(out, "set "+m[1]+" = "+val)
			} else {
				vars[m[1]] = rhs
				out = append(out, raw)
			}
			continue
		}
		if reQuery.MatchString(t) {
			_, action, e := g.queryFunction(substituteScriptVars(t, vars))
			if e != nil {
				return "", e
			} // preserve args exactly through mapping when needed
			m := reQuery.FindStringSubmatch(t)
			arg := strings.TrimSpace(m[2])
			if strings.EqualFold(m[1], "gettrackrecords") && arg != "" {
				action += " " + strings.ReplaceAll(arg, ",", " ")
			} else if strings.EqualFold(m[1], "getpaletteentries") && arg != "" {
				action += " " + arg
			}
			out = append(out, "api "+action)
			continue
		}
		out = append(out, raw)
	}
	return strings.Join(out, "\n"), nil
}

func compareScriptValues(a, op, b string) (bool, error) {
	parse := func(s string) (int64, bool) {
		s = strings.TrimSpace(s)
		base := 10
		if strings.HasPrefix(strings.ToLower(s), "0x") {
			base = 16
			s = s[2:]
		} else if regexp.MustCompile(`^[0-9A-Fa-f]+$`).MatchString(s) && regexp.MustCompile(`[A-Fa-f]`).MatchString(s) {
			base = 16
		}
		v, e := strconv.ParseInt(s, base, 64)
		return v, e == nil
	}
	av, aok := parse(a)
	bv, bok := parse(b)
	if aok && bok {
		switch op {
		case "=":
			return av == bv, nil
		case "!=":
			return av != bv, nil
		case "<":
			return av < bv, nil
		case "<=":
			return av <= bv, nil
		case ">":
			return av > bv, nil
		case ">=":
			return av >= bv, nil
		}
	}
	switch op {
	case "=":
		return a == b, nil
	case "!=":
		return a != b, nil
	}
	return false, fmt.Errorf("relational comparison requires numeric values")
}

func (g *goScriptRunner) predicate(pred string) (bool, error) {
	sm := regexp.MustCompile(`(?i)^status\s+([A-Za-z_][A-Za-z0-9_.-]*)\s*(=|!=|<=|>=|<|>)\s*(\S+)$`).FindStringSubmatch(pred)
	if sm != nil {
		resp, e := g.client.Get(g.base + "/api/status")
		if e != nil {
			return false, e
		}
		defer resp.Body.Close()
		b, _ := io.ReadAll(resp.Body)
		re := regexp.MustCompile(`([A-Za-z_][A-Za-z0-9_.-]*)=([^\s]+)`)
		vals := map[string]string{}
		for _, m := range re.FindAllStringSubmatch(string(b), -1) {
			vals[m[1]] = m[2]
		}
		v, ok := vals[sm[1]]
		if !ok {
			return false, fmt.Errorf("unknown status key %s", sm[1])
		}
		return compareScriptValues(v, sm[2], sm[3])
	}
	mm := regexp.MustCompile(`(?i)^memory\s+cpu=(A|B)\s+address=([0-9A-Fa-fx]+)\s+width=(8|16|32)\s+value\s*(=|!=|<=|>=|<|>)\s*(0x[0-9A-Fa-f]+|[0-9A-Fa-f]+)$`).FindStringSubmatch(pred)
	if mm != nil {
		raw, e := g.transient(fmt.Sprintf("api memory.read cpu=%s address=%s width=%s", mm[1], mm[2], mm[3]))
		if e != nil {
			return false, e
		}
		vm := regexp.MustCompile(`(?i)value=0x([0-9A-F]+)`).FindStringSubmatch(raw)
		if vm == nil {
			return false, fmt.Errorf("memory predicate read failed: %s", raw)
		}
		rhs := strings.TrimPrefix(strings.ToLower(mm[5]), "0x")
		return compareScriptValues("0x"+vm[1], mm[4], "0x"+rhs)
	}
	cm := regexp.MustCompile(`(?i)^capability\s+(\S+)$`).FindStringSubmatch(pred)
	if cm != nil {
		raw, e := g.transient("api capabilities")
		if e != nil {
			return false, e
		}
		for _, x := range strings.Fields(raw) {
			if strings.EqualFold(x, cm[1]) {
				return true, nil
			}
		}
		return false, nil
	}
	return false, fmt.Errorf("invalid if predicate: %s", pred)
}

func (g *goScriptRunner) command(line string, sourceLine int, ignoreFailure bool) (output string, err error) {
	started := time.Now()
	defer func() {
		g.timings = append(g.timings, commandTiming{Sequence: len(g.timings) + 1, SourceLine: sourceLine, Command: line, Category: commandCategory(line), ElapsedMS: float64(time.Since(started).Microseconds()) / 1000, Failed: err != nil})
	}()
	trimmed := strings.TrimSpace(line)
	clientRecord := false
	clientOutput := ""
	if wm := regexp.MustCompile(`(?i)^wait\s+(\d+)$`).FindStringSubmatch(trimmed); wm != nil {
		ms, _ := strconv.Atoi(wm[1])
		if ms < 0 || ms > 5000 {
			return "", fmt.Errorf("wait must be 0..5000 ms")
		}
		time.Sleep(time.Duration(ms) * time.Millisecond)
		clientRecord = true
		clientOutput = fmt.Sprintf("OK waited %dms", ms)
	}
	req := map[string]any{"script": line, "stopOnError": true, "validateOnly": false, "runName": g.runName, "runPurpose": g.purpose, "sourceType": "file", "sourcePath": filepath.ToSlash(g.sourceRel), "sourceLine": sourceLine}
	if clientRecord {
		req["clientRecord"] = true
		req["clientOutput"] = clientOutput
	}
	// The Workbench persists the authored and fully resolved sources when the authoritative run is created.
	// chqdev executes resolved commands one at a time, so the first request must carry both views.
	if g.runID == "" {
		if g.runSource != "" {
			req["runSource"] = g.runSource
		}
		if g.runResolvedSource != "" {
			req["runResolvedSource"] = g.runResolvedSource
		}
	} else {
		req["runId"] = g.runID
	}
	var run scriptRunResponse
	if err := postJSON(g.client, g.base+"/api/v1/script", req, &run); err != nil {
		return "", err
	}
	if run.RunID != "" {
		g.runID = run.RunID
	}
	if run.SessionID != "" {
		g.sessionID = run.SessionID
	}
	if run.Build != "" {
		g.build = run.Build
	}
	g.commandCount++
	if !run.OK {
		detail := "command failed"
		if len(run.Results) > 0 && run.Results[0]["error"] != nil {
			detail = fmt.Sprint(run.Results[0]["error"])
		}
		if ignoreFailure {
			return detail, nil
		}
		return "", fmt.Errorf("line %d command %q: %s", sourceLine, line, detail)
	}
	if clientRecord {
		return clientOutput, nil
	}
	if len(run.Results) > 0 {
		return fmt.Sprint(run.Results[0]["output"]), nil
	}
	return "OK", nil
}
func (g *goScriptRunner) exec(items []scriptItem) (err error) {
	for _, it := range items {
		switch it.Kind {
		case "command":
			_, err = g.command(it.Command, it.Line, false)
			if err != nil {
				return err
			}
		case "if":
			var ok bool
			ok, err = g.predicate(it.Predicate)
			if err != nil {
				return fmt.Errorf("line %d predicate: %w", it.Line, err)
			}
			if ok {
				err = g.exec(it.ThenItems)
			} else {
				err = g.exec(it.ElseItems)
			}
			if err != nil {
				return err
			}
		case "try":
			bodyErr := g.exec(it.BodyItems)
			finalErr := g.exec(it.FinalItems)
			if bodyErr != nil {
				return bodyErr
			}
			if finalErr != nil {
				return finalErr
			}
		}
	}
	return nil
}

func (g *goScriptRunner) finalize(status string) error {
	if g.runID == "" {
		return nil
	}
	var fin finalizeResponse
	return postJSON(g.client, g.base+"/api/v1/script", map[string]any{"script": "", "stopOnError": true, "validateOnly": false, "runId": g.runID, "runName": g.runName, "runPurpose": g.purpose, "sourceType": "file", "sourcePath": filepath.ToSlash(g.sourceRel), "finalizeRun": true, "runStatus": status, "commandCount": g.commandCount}, &fin)
}

func runScriptWithGoRunner(root string, cfg Config, kind, overrideURL string) (*goScriptRunner, error) {
	rel, name, err := regressionPath(cfg, kind)
	if err != nil {
		return nil, err
	}
	b, err := os.ReadFile(filepath.Join(root, filepath.FromSlash(rel)))
	if err != nil {
		return nil, err
	}
	authoredSource := string(bytes.TrimPrefix(b, []byte{0xEF, 0xBB, 0xBF}))
	src := authoredSource
	src, err = expandScriptIncludes(root, cfg.ScriptRoot, src, 0)
	if err != nil {
		return nil, err
	}
	g := &goScriptRunner{runStarted: time.Now(), root: root, base: workbenchBase(cfg, overrideURL), sourceRel: rel, runName: name, purpose: "chqdev authoritative Go Script Console runner", release: cfg.Release, runSource: authoredSource, runResolvedSource: src, client: &http.Client{Timeout: 45 * time.Minute}}
	defaults := scriptDefaultVars(src)
	src, err = g.resolveQueries(src, defaults)
	if err != nil {
		return g, fmt.Errorf("query preflight: %w", err)
	}
	items, err := compileScriptLines(strings.Split(src, "\n"), defaults)
	if err != nil {
		return g, fmt.Errorf("compile: %w", err)
	}
	if n, limit := countScriptItems(items), scriptCommandLimit(); n > limit {
		return g, fmt.Errorf("expanded script exceeds %d commands (%d)", limit, n)
	}
	err = g.exec(items)
	status := "PASS"
	if err != nil {
		status = "FAIL"
	}
	// Preserve diagnostics on both PASS and FAIL, without changing the executed script.
	timingPath, timingErr := g.saveTimings()
	if timingPath != "" {
		fmt.Printf("[REGRESSION TIMING] %s (%d commands)\n", timingPath, len(g.timings))
	}
	if timingErr != nil && err == nil {
		err = fmt.Errorf("save command timings: %w", timingErr)
	}
	finErr := g.finalize(status)
	if err == nil && finErr != nil {
		err = finErr
	}
	if g.build != "" && g.build != cfg.Release && err == nil {
		err = fmt.Errorf("run build=%s expected=%s", g.build, cfg.Release)
	}
	return g, err
}
