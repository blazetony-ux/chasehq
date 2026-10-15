package main

import (
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestRC621GhidraCandidateExportContract(t *testing.T) {
	root, err := filepath.Abs(filepath.Join("..", ".."))
	if err != nil {
		t.Fatal(err)
	}
	required := map[string][]string{
		"research/ghidra-automation/ChaseHQSeed68000.py":        {"0x00820C", "getScriptArgs()", "Candidate PC out of CPU-A ROM bounds/alignment"},
		"research/ghidra-automation/ChaseHQExport68000.py":      {"steering_conversion_writer_0820C", "requested_cpuA_", "functionCandidate", "hypothesis"},
		"research/ghidra-automation/Run-ChaseHQ-Ghidra.ps1":     {"CandidatePcs", "@candidateTokens", "requestedCandidatePcs", "Ghidra did not disassemble requested CPU-A candidate", "releaseBaseline = $release"},
		"research/ghidra-automation/Run-WorkbenchGhidraJob.ps1": {"build='0.67.0.0-RC6.2.4'", "manifest release identity mismatch"},
	}
	for rel, tokens := range required {
		b, e := os.ReadFile(filepath.Join(root, filepath.FromSlash(rel)))
		if e != nil {
			t.Fatal(e)
		}
		for _, token := range tokens {
			if !strings.Contains(string(b), token) {
				t.Errorf("%s missing %q", rel, token)
			}
		}
	}
}
