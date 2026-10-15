package main

// RC6.3.3 derives branch coverage from real original-ROM instruction hooks,
// not from generated C++ or fixture names. Controlled entry-state changes are
// explicit, auditable debugger interventions; they are not natural-gameplay proof.
import (
	"encoding/json"
	"fmt"
	"os"
	"path/filepath"
	"sort"
	"strings"
)

var rc633ExpectedPath = map[string]string{
	"natural": "", "bypass": "gate-bypass", "nonterminal": "decrement-nonterminal",
	"terminal": "terminal-underflow", "overflow": "decrement-nonterminal",
}

func rc633ParseOriginalPath(contents []byte) ([]string, error) {
	lines := strings.Split(strings.TrimSpace(string(contents)), "\n")
	if len(lines) != 1 {
		return nil, fmt.Errorf("expected exactly one original-ROM function path, got %d", len(lines))
	}
	f := strings.Split(strings.TrimSpace(lines[0]), "\t")
	if len(f) != 4 || f[1] != "rom" || f[3] == "" {
		return nil, fmt.Errorf("invalid original-ROM function path %q", lines[0])
	}
	pcs := strings.Split(strings.ToLower(f[3]), ",")
	if pcs[0] != "a10c" || pcs[len(pcs)-1] != "a128" {
		return nil, fmt.Errorf("invalid health entry/RTS path %v", pcs)
	}
	allowed := map[string]bool{"a10c": true, "a110": true, "a112": true, "a116": true, "a118": true, "a11e": true, "a120": true, "a124": true, "a128": true}
	for _, pc := range pcs {
		if !allowed[pc] {
			return nil, fmt.Errorf("unreviewed instruction %s in runtime path", pc)
		}
	}
	return pcs, nil
}
func rc633ClassifyPath(pcs []string) string {
	actual := strings.Join(pcs, ",")
	switch actual {
	case "a10c,a110,a120,a124,a128":
		return "gate-bypass"
	case "a10c,a110,a112,a116,a120,a124,a128":
		return "decrement-nonterminal"
	case "a10c,a110,a112,a116,a118,a11e,a128":
		return "terminal-underflow"
	default:
		return "UNKNOWN"
	}
}

type rc633BranchMatrix struct {
	Schema                      string             `json:"schema"`
	Status                      string             `json:"status"`
	Scope                       string             `json:"scope"`
	DistinctROMBranchPaths      int                `json:"distinctROMBranchPaths"`
	GhidraCandidateSites        int                `json:"ghidraCandidateSites"`
	ObservedROMInstructionSites int                `json:"observedROMInstructionSites"`
	MissingCandidateSites       []string           `json:"missingCandidateSites"`
	Cases                       []rc631HealthProof `json:"cases"`
	AutonomousPromotionAllowed  bool               `json:"autonomousPromotionAllowed"`
}

func rc633RunBranchMatrix(root, dest, romDir, lib string, candidates []rc63Candidate) (rc633BranchMatrix, error) {
	result := rc633BranchMatrix{Schema: "chq-rc633-branch-matrix-v1", Status: "NOT_VERIFIED", Scope: "one authentic CPU-A function entry, 1 natural checkpoint + 4 CONTROLLED RAM entry-state cases; Ghidra boundary remains unreviewed", AutonomousPromotionAllowed: false}
	expectedSites := map[string]bool{}
	for _, c := range candidates {
		if c.Family != "conditional-health-function" {
			continue
		}
		for _, ins := range c.Instructions {
			pc, err := rc63Address(ins.PC)
			if err != nil {
				return result, err
			}
			expectedSites[fmt.Sprintf("%x", pc)] = true
		}
	}
	result.GhidraCandidateSites = len(expectedSites)
	if result.GhidraCandidateSites != 9 {
		return result, fmt.Errorf("Ghidra did not provide the nine-instruction health candidate: %d", result.GhidraCandidateSites)
	}
	observedSites := map[string]bool{}
	distinct := map[string]bool{}
	for _, name := range []string{"natural", "bypass", "nonterminal", "terminal", "overflow"} {
		proof, err := rc633RunHealthCase(root, filepath.Join(dest, "branch-matrix", name), romDir, lib, name)
		result.Cases = append(result.Cases, proof)
		_ = rc633SaveBranchMatrix(dest, result)
		if err != nil {
			return result, fmt.Errorf("health branch case %s failed: %w", name, err)
		}
		if proof.PathClassification == "UNKNOWN" {
			return result, fmt.Errorf("unclassified original-ROM execution in %s: %v", name, proof.OriginalPCPath)
		}
		distinct[strings.Join(proof.OriginalPCPath, ",")] = true
		for _, pc := range proof.OriginalPCPath {
			observedSites[pc] = true
		}
	}
	result.DistinctROMBranchPaths = len(distinct)
	result.ObservedROMInstructionSites = len(observedSites)
	for pc := range expectedSites {
		if !observedSites[pc] {
			result.MissingCandidateSites = append(result.MissingCandidateSites, pc)
		}
	}
	sort.Strings(result.MissingCandidateSites)
	if len(result.MissingCandidateSites) > 0 {
		return result, fmt.Errorf("real ROM did not execute candidate sites: missing %v", result.MissingCandidateSites)
	}
	if result.DistinctROMBranchPaths < 3 {
		return result, fmt.Errorf("less than three distinct real ROM execution paths: %d", len(distinct))
	}
	result.Status = "LIVE_CONTROLLED_BRANCH_MATRIX_PASS"
	_ = rc633SaveBranchMatrix(dest, result)
	return result, nil
}
func rc633SaveBranchMatrix(dest string, report rc633BranchMatrix) error {
	b, err := json.MarshalIndent(report, "", "  ")
	if err != nil {
		return err
	}
	return os.WriteFile(filepath.Join(dest, "branch-matrix-proof.json"), append(b, '\n'), 0644)
}
