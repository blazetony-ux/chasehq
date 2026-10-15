package main

// Optional, offline REA/Ghidra -> ROM census crosswalk. REA is NOT downloaded
// or installed. Input must be an explicitly mapped operator-reviewed export;
// external AI inference is never upgraded into verified ROM provenance.
import (
	"bufio"
	"encoding/json"
	"fmt"
	"os"
	"path/filepath"
	"regexp"
	"sort"
	"strconv"
	"strings"
)

type reaFunction struct {
	Name       string `json:"name"`
	Start      string `json:"start"`
	End        string `json:"end"`
	Confidence string `json:"confidence"`
}
type reaInput struct {
	Schema    string        `json:"schema"`
	CPU       string        `json:"cpu"`
	RomSHA256 string        `json:"romSha256"`
	ImageBase string        `json:"imageBase"`
	Functions []reaFunction `json:"functions"`
}
type reaPC struct {
	PC   string `json:"pc"`
	Hits uint64 `json:"instructionHits"`
}
type reaMatch struct {
	Name                 string  `json:"name"`
	ROMStart             string  `json:"romStart"`
	ROMEnd               string  `json:"romEndExclusive"`
	ObservedInstructions uint64  `json:"observedInstructionHits"`
	DistinctObservedPCs  int     `json:"distinctObservedPCs"`
	Confidence           string  `json:"staticConfidence"`
	ObservedPCs          []reaPC `json:"observedPCs"`
}

func parseReaHex(s string) (uint64, error) {
	if !strings.HasPrefix(s, "0x") || len(s) < 3 {
		return 0, fmt.Errorf("address must use 0x hex: %q", s)
	}
	return strconv.ParseUint(s[2:], 16, 32)
}
func crosswalkRea(input []byte, census []byte) ([]byte, error) {
	var x reaInput
	if err := json.Unmarshal(input, &x); err != nil {
		return nil, err
	}
	if x.Schema != "chq-rea-static-export-v1" {
		return nil, fmt.Errorf("unsupported static input schema")
	}
	cpuMax := uint64(0x80000)
	if x.CPU == "B" {
		cpuMax = 0x20000
	} else if x.CPU != "A" {
		return nil, fmt.Errorf("CPU must be A or B")
	}
	if ok, _ := regexp.MatchString(`^[a-fA-F0-9]{64}$`, x.RomSHA256); !ok {
		return nil, fmt.Errorf("64-character raw ROM image SHA256 required")
	}
	base, err := parseReaHex(x.ImageBase)
	if err != nil {
		return nil, err
	}
	if len(x.Functions) > 4096 {
		return nil, fmt.Errorf("too many function records (max 4096)")
	}
	if len(x.Functions) == 0 {
		return nil, fmt.Errorf("no explicitly mapped functions")
	}
	type region struct {
		start, end        uint64
		label, confidence string
	}
	regions := make([]region, 0, len(x.Functions))
	for _, f := range x.Functions {
		start, e := parseReaHex(f.Start)
		if e != nil {
			return nil, e
		}
		end, e := parseReaHex(f.End)
		if e != nil {
			return nil, e
		}
		if f.Name == "" || start < base || end <= start || end > base+cpuMax || (start-base)%2 != 0 || (end-base)%2 != 0 {
			return nil, fmt.Errorf("unmapped/misaligned range %s %s..%s", f.Name, f.Start, f.End)
		}
		if f.Confidence != "hypothesis" && f.Confidence != "static-observed" {
			return nil, fmt.Errorf("confidence must be hypothesis or static-observed")
		}
		regions = append(regions, region{start - base, end - base, f.Name, f.Confidence})
	}
	sort.Slice(regions, func(i, j int) bool { return regions[i].start < regions[j].start })
	for i := 1; i < len(regions); i++ {
		if regions[i].start < regions[i-1].end {
			return nil, fmt.Errorf("overlapping static function ranges")
		}
	}
	hits := map[uint64]uint64{}
	rows := 0
	scanner := bufio.NewScanner(strings.NewReader(string(census)))
	if !scanner.Scan() || scanner.Text() != "cpu\tpc\tinstructionHits" {
		return nil, fmt.Errorf("invalid ROM census PC TSV header")
	}
	for scanner.Scan() {
		rows++
		if rows > 1000000 {
			return nil, fmt.Errorf("too many census rows")
		}
		fields := strings.Split(scanner.Text(), "\t")
		if len(fields) != 3 {
			return nil, fmt.Errorf("invalid census row")
		}
		if fields[0] != "CPU-A" && fields[0] != "CPU-B" {
			return nil, fmt.Errorf("invalid census CPU")
		}
		pc, e := parseReaHex(strings.ToLower(fields[1]))
		if e != nil {
			return nil, e
		}
		n, e := strconv.ParseUint(fields[2], 10, 64)
		if e != nil {
			return nil, e
		}
		if fields[0] == "CPU-"+x.CPU {
			if pc >= cpuMax || pc%2 != 0 {
				return nil, fmt.Errorf("census PC outside mapped ROM")
			}
			hits[pc] += n
		}
	}
	if err := scanner.Err(); err != nil {
		return nil, err
	}
	records := make([]reaMatch, 0, len(regions))
	for _, f := range regions {
		m := reaMatch{Name: f.label, ROMStart: fmt.Sprintf("0x%06X", f.start), ROMEnd: fmt.Sprintf("0x%06X", f.end), Confidence: f.confidence}
		for pc, n := range hits {
			if pc >= f.start && pc < f.end {
				m.ObservedInstructions += n
				m.DistinctObservedPCs++
				m.ObservedPCs = append(m.ObservedPCs, reaPC{PC: fmt.Sprintf("0x%06X", pc), Hits: n})
			}
		}
		sort.Slice(m.ObservedPCs, func(i, j int) bool { return m.ObservedPCs[i].PC < m.ObservedPCs[j].PC })
		if m.ObservedPCs == nil {
			m.ObservedPCs = []reaPC{}
		}
		records = append(records, m)
	}
	report := map[string]any{"schema": "chq-rea-census-crosswalk-v1", "cpu": x.CPU, "romSha256": strings.ToLower(x.RomSHA256), "explicitImageBase": x.ImageBase, "matches": records, "provenance": "operator-mapped untrusted static export + observed runtime PCs", "nativeRoutineReplacement": false, "limitations": []string{"No direct REA call; Windows REA Ghidra P0 does not support raw 68000 ROM", "Static function boundaries/hypotheses are not runtime-verified solely by PC intersections", "ROM hash and image base must be independently verified against actual imported bytes"}}
	return json.MarshalIndent(report, "", "  ")
}
func reaCommand(root string, cfg Config, args []string) *Report {
	r := newReport("rea", cfg.Release)
	if len(args) == 0 || args[0] != "crosswalk" {
		r.fail("syntax", "rea crosswalk --static JSON --census TSV --out JSON")
		return r
	}
	paths := map[string]string{}
	for i := 1; i < len(args); i++ {
		if args[i] == "--static" || args[i] == "--census" || args[i] == "--out" {
			if i+1 >= len(args) {
				r.fail("syntax", "missing path for "+args[i])
				return r
			}
			paths[args[i]] = args[i+1]
			i++
		} else {
			r.fail("syntax", "unknown argument "+args[i])
			return r
		}
	}
	if paths["--static"] == "" || paths["--census"] == "" || paths["--out"] == "" {
		r.fail("syntax", "--static, --census and --out are required")
		return r
	}
	st, e := os.ReadFile(paths["--static"])
	if e != nil {
		r.fail("static", e.Error())
		return r
	}
	pc, e := os.ReadFile(paths["--census"])
	if e != nil {
		r.fail("census", e.Error())
		return r
	}
	out, e := crosswalkRea(st, pc)
	if e != nil {
		r.fail("mapping", e.Error())
		return r
	}
	if e := os.MkdirAll(filepath.Dir(paths["--out"]), 0755); e != nil {
		r.fail("output", e.Error())
		return r
	}
	if e := os.WriteFile(paths["--out"], append(out, '\n'), 0644); e != nil {
		r.fail("output", e.Error())
		return r
	}
	r.pass("crosswalk", "Explicit ROM map and census intersection exported; zero inferred function ownership")
	r.Data["output"] = paths["--out"]
	return r
}
