package main

// RC6.3.7: fail-closed, shared per-candidate execution ledger and independent
// original-ROM CPU-B instruction-edge evidence. Original/native/fallback
// rows must derive from live emulator hook dispatch, NOT synthesized from
// generated C++ fixtures, aggregate PC counts or expected outcomes.
import (
	"encoding/json"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"sort"
	"strconv"
	"strings"
)

type rc637DispatchRow struct {
	Frame   uint64
	CPU, ID string
	PC      uint64
	Outcome string
}
type rc637Edge struct {
	To, Opcode string
	Hits       int64
}
type rc637CPU_BCandidate struct {
	PC                       string      `json:"pc"`
	OriginalHits             int64       `json:"originalHits"`
	EdgeHits                 int64       `json:"observedOutgoingEdges"`
	OpcodeWord               string      `json:"originalOpcodeWord"`
	Destinations             []rc637Edge `json:"observedTransitions"`
	GhidraFunctionReviewed   bool        `json:"ghidraFunctionReviewed"`
	NativeReplacementAllowed bool        `json:"nativeReplacementAllowed"`
}
type rc637SharedProof struct {
	Schema                        string                `json:"schema"`
	Status                        string                `json:"status"`
	SharedROMMode                 bool                  `json:"sharedROMMode"`
	CandidateRanges               []string              `json:"candidateRanges"`
	HealthCasesVerified           int                   `json:"healthCasesVerified"`
	CopyROMExecutions             int                   `json:"copyROMExecutions"`
	NativeCommits                 int                   `json:"nativeCommits"`
	ForcedFallbacks               int                   `json:"forcedFallbacks"`
	CPU_BObservedEdges            int                   `json:"cpuBObservedEdges"`
	CPU_BCandidates               []rc637CPU_BCandidate `json:"cpuBCandidates"`
	NativePromotionAllowed        bool                  `json:"nativePromotionAllowed"`
	CPU_BNativeSubstitutionProven bool                  `json:"cpuBNativeSubstitutionProven"`
	Limitations                   []string              `json:"limitations"`
}

func rc637ReadDispatch(p string) ([]rc637DispatchRow, error) {
	b, e := os.ReadFile(p)
	if e != nil {
		return nil, e
	}
	if len(b) == 0 || b[len(b)-1] != '\n' {
		return nil, errors.New("empty/truncated dispatcher audit")
	}
	var rows []rc637DispatchRow
	for _, line := range strings.Split(strings.TrimSuffix(string(b), "\n"), "\n") {
		c := strings.Split(line, "\t")
		if len(c) != 5 {
			return nil, fmt.Errorf("bad dispatch fields: %q", line)
		}
		frame, e := strconv.ParseUint(c[0], 10, 64)
		if e != nil {
			return nil, e
		}
		pc, e := strconv.ParseUint(c[3], 16, 32)
		if e != nil {
			return nil, e
		}
		if (c[1] != "A" && c[1] != "B") || !strings.HasPrefix(c[2], c[1]+":") || (c[4] != "original-rom" && c[4] != "original-fallback" && c[4] != "native-commit") {
			return nil, fmt.Errorf("invalid dispatcher provenance: %q", line)
		}
		rows = append(rows, rc637DispatchRow{frame, c[1], c[2], pc, c[4]})
	}
	return rows, nil
}
func rc637RequireDispatch(p, id, mode string, want int) (int, error) {
	rows, e := rc637ReadDispatch(p)
	if e != nil {
		return 0, e
	}
	found := 0
	for _, r := range rows {
		if r.ID != id {
			continue
		}
		if r.Frame < 6333 || r.Frame >= 6340 {
			return 0, errors.New("dispatch outside authentic replay interval")
		}
		if r.Outcome == mode {
			found++
		} else if mode == "original-fallback" && r.Outcome == "original-rom" {
			// A fault probe only attempts the first copy. After rollback, the
			// remaining ROM opcodes execute in normal original mode.
		} else {
			return 0, fmt.Errorf("candidate %s unexpected dispatch %s want %s", id, r.Outcome, mode)
		}
	}
	if found != want {
		return 0, fmt.Errorf("candidate %s: %d %s events, expected %d", id, found, mode, want)
	}
	return found, nil
}
func rc637ReadObservedEdges(p string) (map[string][]rc637Edge, int, error) {
	b, e := os.ReadFile(p)
	if e != nil {
		return nil, 0, e
	}
	if len(b) == 0 || b[len(b)-1] != '\n' {
		return nil, 0, errors.New("empty/truncated CPU-B edge file")
	}
	edges := map[string][]rc637Edge{}
	count := 0
	seen := map[string]bool{}
	for _, line := range strings.Split(strings.TrimSuffix(string(b), "\n"), "\n") {
		c := strings.Split(line, "\t")
		if len(c) != 4 {
			return nil, 0, fmt.Errorf("bad edge row: %q", line)
		}
		from, e := strconv.ParseUint(c[0], 16, 32)
		if e != nil {
			return nil, 0, e
		}
		to, e := strconv.ParseUint(c[1], 16, 32)
		if e != nil {
			return nil, 0, e
		}
		opcode, e := strconv.ParseUint(c[2], 16, 16)
		if e != nil {
			return nil, 0, e
		}
		hits, e := strconv.ParseInt(c[3], 10, 64)
		if e != nil || hits <= 0 || from&1 != 0 || to&1 != 0 || from >= 0x20000 || to > 0xffffff {
			return nil, 0, fmt.Errorf("invalid CPU-B ROM edge: %q", line)
		}
		// Keep CPU-B census and edge provenance in one 24-bit address space.
		// The real Windows ROM census canonicalises PC as B:0x%06X.
		// Eight-digit keys were syntactically valid but NEVER matched that
		// six-digit census; do not weaken missing-edge detection to fix it.
		fromKey := fmt.Sprintf("B:0x%06X", from)
		rowKey := fmt.Sprintf("%s:%06X", fromKey, to)
		if seen[rowKey] {
			return nil, 0, errors.New("duplicate CPU-B edge")
		}
		seen[rowKey] = true
		edges[fromKey] = append(edges[fromKey], rc637Edge{To: fmt.Sprintf("0x%06X", to), Opcode: fmt.Sprintf("0x%04X", opcode), Hits: hits})
		count++
	}
	return edges, count, nil
}
func rc637SharedLiveProof(dest string, copyProof rc636CopyProof) (rc637SharedProof, error) {
	p := rc637SharedProof{Schema: "chq-rc637-shared-dispatch-and-cpu-b-edges-v1", Status: "BLOCKED", SharedROMMode: true,
		CandidateRanges:        []string{"A:0000A10C-0000A12A", "A:00040540-00040560"},
		NativePromotionAllowed: false, CPU_BNativeSubstitutionProven: false,
		Limitations: []string{"Static CPU-A function boundaries and all-callers not reviewed", "CPU-B original opcode/edge observations are NOT Ghidra function discovery or native CPU-B replacement", "Dispatcher admits only the two independently Ghidra-verified bounded CPU-A adapters; no arbitrary-function autonomous promotion"}}
	check := func(path, id, outcome string, want int) error {
		count, e := rc637RequireDispatch(path, id, outcome, want)
		if e == nil && outcome == "native-commit" {
			p.NativeCommits += count
		}
		if e == nil && outcome == "original-fallback" {
			p.ForcedFallbacks += count
		}
		return e
	}
	const health = "A:0000A10C-0000A12A"
	const copyID = "A:00040540-00040560"
	for _, caseName := range []string{"natural", "bypass", "nonterminal", "terminal", "overflow"} {
		base := filepath.Join(dest, "branch-matrix", caseName, "live-health")
		for _, checkMode := range []struct{ dir, outcome string }{{"original", "original-rom"}, {"generated-native", "native-commit"}, {"forced-mismatch", "original-fallback"}} {
			if err := check(filepath.Join(base, checkMode.dir, "native-dispatch.tsv"), health, checkMode.outcome, 1); err != nil {
				return p, fmt.Errorf("health %s %s: %w", caseName, checkMode.dir, err)
			}
		}
		p.HealthCasesVerified++
	}
	if copyProof.Status != "LIVE_GENERATED_COPY_BLOCK_PASS" {
		return p, errors.New("independent live copy proof not passed")
	}
	p.CopyROMExecutions = copyProof.OriginalReturns
	if p.CopyROMExecutions < 1 {
		return p, errors.New("zero original copy executions")
	}
	for _, s := range []struct {
		mode, outcome string
		want          int
	}{{"original", "original-rom", p.CopyROMExecutions}, {"generated-native", "native-commit", p.CopyROMExecutions}, {"forced-mismatch", "original-fallback", 1}} {
		if err := check(filepath.Join(dest, "live-copy", s.mode, "native-dispatch.tsv"), copyID, s.outcome, s.want); err != nil {
			return p, fmt.Errorf("copy %s: %w", s.mode, err)
		}
	}
	census, err := readNativeTsv(filepath.Join(dest, "branch-matrix", "natural", "live-health", "original", "rom-execution-pc-counts.tsv"))
	if err != nil {
		return p, err
	}
	edges, n, err := rc637ReadObservedEdges(filepath.Join(dest, "branch-matrix", "natural", "live-health", "original", "cpu-b-observed-edges.tsv"))
	if err != nil {
		return p, err
	}
	p.CPU_BObservedEdges = n
	type item struct {
		pc    string
		count int64
	}
	var ranked []item
	for pc, hits := range census {
		if strings.HasPrefix(pc, "B:0x") && hits > 0 {
			ranked = append(ranked, item{pc, hits})
		}
	}
	sort.Slice(ranked, func(i, j int) bool {
		if ranked[i].count != ranked[j].count {
			return ranked[i].count > ranked[j].count
		}
		return ranked[i].pc < ranked[j].pc
	})
	if len(ranked) < 5 {
		return p, fmt.Errorf("only %d authentic CPU-B ROM PCs observed", len(ranked))
	}
	for _, i := range ranked[:5] {
		rows := edges[i.pc]
		if len(rows) == 0 {
			return p, fmt.Errorf("hot CPU-B PC lacks observed outgoing edges: %s", i.pc)
		}
		var sum int64
		opcode := rows[0].Opcode
		for _, r := range rows {
			sum += r.Hits
			if r.Opcode != opcode {
				return p, fmt.Errorf("CPU-B ROM opcode changed at %s", i.pc)
			}
		}
		if sum != i.count && sum != i.count-1 {
			return p, fmt.Errorf("CPU-B graph/census disagree %s: transitions=%d hits=%d", i.pc, sum, i.count)
		}
		sort.Slice(rows, func(a, b int) bool {
			if rows[a].Hits != rows[b].Hits {
				return rows[a].Hits > rows[b].Hits
			}
			return rows[a].To < rows[b].To
		})
		p.CPU_BCandidates = append(p.CPU_BCandidates, rc637CPU_BCandidate{PC: strings.TrimPrefix(i.pc, "B:"), OriginalHits: i.count, EdgeHits: sum, OpcodeWord: opcode, Destinations: rows, GhidraFunctionReviewed: false, NativeReplacementAllowed: false})
	}
	p.Status = "LIVE_SHARED_DISPATCH_AND_OBSERVED_CPU_B_EDGES_PASS"
	return p, nil
}
func rc637SaveProof(dest string, p rc637SharedProof) error {
	b, e := json.MarshalIndent(p, "", "  ")
	if e != nil {
		return e
	}
	return os.WriteFile(filepath.Join(dest, "shared-dispatch-cpu-b-proof.json"), append(b, '\n'), 0644)
}
