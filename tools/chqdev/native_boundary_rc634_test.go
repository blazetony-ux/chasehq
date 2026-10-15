package main

import (
	"encoding/binary"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestRC634CallerDecodesOnlySupportedAuthenticShapes(t *testing.T) {
	rom := make([]byte, 0x2000)
	binary.BigEndian.PutUint16(rom[0x900:], 0x4eb9)
	binary.BigEndian.PutUint32(rom[0x902:], 0x1000)
	if n, ok := rc634Caller(rom, 0x906, 0x1000); !ok || n != "JSR absolute-long" {
		t.Fatal(n, ok)
	}
	if _, ok := rc634Caller(rom, 0x906, 0x1010); ok {
		t.Fatal("accepted wrong destination")
	}
	binary.BigEndian.PutUint16(rom[0x910:], 0x6100)
	binary.BigEndian.PutUint16(rom[0x912:], 0x06ee) // ret-2 = 0x912, target 0x1000
	if n, ok := rc634Caller(rom, 0x914, 0x1000); !ok || n != "BSR word" {
		t.Fatal(n, ok)
	}
	if _, ok := rc634Caller(rom, 2, 0x1000); ok {
		t.Fatal("fabricated indirect caller")
	}
}
func TestRC634ObservedBoundaryEnforcesActualStackReturnAndROM(t *testing.T) {
	d := t.TempDir()
	rom := make([]byte, 0x20000)
	binary.BigEndian.PutUint16(rom[0xa128:], 0x4e75)
	binary.BigEndian.PutUint16(rom[0xa096:], 0x4eb9)
	binary.BigEndian.PutUint32(rom[0xa098:], 0xa10c)
	m := rc633BranchMatrix{Status: "LIVE_CONTROLLED_BRANCH_MATRIX_PASS", DistinctROMBranchPaths: 3, ObservedROMInstructionSites: 9}
	c := []rc63Candidate{{Family: "conditional-health-function", Entry: "0x0000A10C", End: "0x0000A12A", FullFunction: true}}
	for _, name := range []string{"natural", "bypass", "nonterminal", "terminal", "overflow"} {
		m.Cases = append(m.Cases, rc631HealthProof{EntryCase: name})
		for _, mode := range []string{"original", "generated-native", "forced-mismatch"} {
			p := filepath.Join(d, "branch-matrix", name, "live-health", mode)
			if err := os.MkdirAll(p, 0755); err != nil {
				t.Fatal(err)
			}
			if err := os.WriteFile(filepath.Join(p, "native-health-entry.tsv"), []byte("6334\t"+name+"\t102de0\ta09c\t108000\t2700\n"), 0644); err != nil {
				t.Fatal(err)
			}
			if err := os.WriteFile(filepath.Join(p, "native-health-caller-path.tsv"), []byte("6334\t"+name+"\ta09c\tUNOBSERVED\tUNOBSERVED\tUNOBSERVED\n"), 0644); err != nil {
				t.Fatal(err)
			}
			if mode == "original" {
				if err := os.WriteFile(filepath.Join(p, "native-health-state.tsv"), []byte("6334\trom\t0000a09c\tabcdef\n"), 0644); err != nil {
					t.Fatal(err)
				}
			}
		}
	}
	audit, err := rc634AuditHealthBoundary(d, c, rom, m)
	if err != nil {
		t.Fatal(err)
	}
	if audit.Status != "OBSERVED_ENTRY_RTS_RETURN_PASS_NOT_FULL_FUNCTION_REVIEW" || len(audit.Cases) != 5 || audit.FunctionPromotionAllowed || audit.FunctionBoundaryIndependentlyReviewed || !audit.Cases[0].CallerTargetVerified {
		t.Fatalf("overstated or bad evidence: %+v", audit)
	}
	p := filepath.Join(d, "branch-matrix", "terminal", "live-health", "generated-native", "native-health-entry.tsv")
	if err := os.WriteFile(p, []byte("6334\tterminal\t102de0\ta09e\t108000\t2700\n"), 0644); err != nil {
		t.Fatal(err)
	}
	if _, err := rc634AuditHealthBoundary(d, c, rom, m); err == nil || !strings.Contains(err.Error(), "entry stack/return mismatch") {
		t.Fatalf("accepted forged return: %v", err)
	}
}

// The real Windows result can contain a valid direct BSR to an earlier
// entry, followed by authentic execution falling through into 0xA10C. The
// old audit incorrectly rejected this as a wrong function call. A corrupted
// dynamic successor or differing rollback trace must still fail closed.
func TestRC635CallIntoLargerFunctionAndTailEntry(t *testing.T) {
	d := t.TempDir()
	rom := make([]byte, 0x20000)
	binary.BigEndian.PutUint16(rom[0xa128:], 0x4e75)
	// 0xA09A: BSR.B -0x1C, returning to 0xA09C, calling 0xA080.
	binary.BigEndian.PutUint16(rom[0xa09a:], 0x61e4)
	m := rc633BranchMatrix{Status: "LIVE_CONTROLLED_BRANCH_MATRIX_PASS", DistinctROMBranchPaths: 3, ObservedROMInstructionSites: 9}
	c := []rc63Candidate{{Family: "conditional-health-function", Entry: "0x0000A10C", End: "0x0000A12A", FullFunction: true}}
	for _, name := range []string{"natural", "bypass", "nonterminal", "terminal", "overflow"} {
		m.Cases = append(m.Cases, rc631HealthProof{EntryCase: name})
		for _, mode := range []string{"original", "generated-native", "forced-mismatch"} {
			dir := filepath.Join(d, "branch-matrix", name, "live-health", mode)
			if err := os.MkdirAll(dir, 0755); err != nil {
				t.Fatal(err)
			}
			files := map[string]string{
				"native-health-entry.tsv":       "6334\t" + name + "\t102de0\ta09c\t108000\t2700\n",
				"native-health-caller-path.tsv": "6334\t" + name + "\ta09c\ta09a\ta080\ta09a,a080,a104,a108,a10c\n",
			}
			if mode == "original" {
				files["native-health-state.tsv"] = "6334\trom\t0000a09c\tabcdef\n"
			}
			for fname, value := range files {
				if err := os.WriteFile(filepath.Join(dir, fname), []byte(value), 0644); err != nil {
					t.Fatal(err)
				}
			}
		}
	}
	audit, err := rc634AuditHealthBoundary(d, c, rom, m)
	if err != nil {
		t.Fatal(err)
	}
	if audit.Status != "OBSERVED_TAIL_ENTRY_RTS_RETURN_PASS_FULL_BOUNDARY_UNVERIFIED" || !audit.ObservedFunctionTailOnly || audit.FunctionPromotionAllowed || audit.Cases[0].CallerTargetVerified || !audit.Cases[0].DynamicCallerToTailVerified || audit.Cases[0].DecodedCallerTarget != "0x0000A080" {
		t.Fatalf("bad tail classification: %+v", audit)
	}
	file := filepath.Join(d, "branch-matrix", "natural", "live-health", "original", "native-health-caller-path.tsv")
	if err := os.WriteFile(file, []byte("6334\tnatural\ta09c\tUNOBSERVED\tUNOBSERVED\tUNOBSERVED\n"), 0644); err != nil {
		t.Fatal(err)
	}
	if _, err := rc634AuditHealthBoundary(d, c, rom, m); err == nil {
		t.Fatal("accepted unobserved larger-function caller path")
	}
	// The ROM has no legitimate BSR successor at 0xA090.
	os.WriteFile(file, []byte("6334\tnatural\ta09c\ta09a\ta090\ta09a,a090,a10c\n"), 0644)
	if _, err := rc634AuditHealthBoundary(d, c, rom, m); err == nil {
		t.Fatal("forged caller path accepted")
	}
}

func TestRC634QueueDoesNotPromoteStaticOrCPU_B(t *testing.T) {
	c := []rc63Candidate{{ID: "A:Health", CPU: "A", Family: "conditional-health-function"}, {ID: "A:Copy", CPU: "A", Family: "sequential-longword-copy"}}
	p := rc634MakePortfolio("0.67.0.0-RC6.3.4", c, true)
	for _, i := range p.Candidates {
		if i.PromotionAllowed {
			t.Fatal("promoted candidate", i)
		}
		if i.CPU == "B" && !strings.Contains(i.Stage, "BLOCKED") {
			t.Fatal(i)
		}
	}
	if p.GeneralAutonomousPromotionAllowed {
		t.Fatal("promoted portfolio")
	}
	if err := rc634WritePortfolio(t.TempDir(), t.TempDir(), p); err != nil {
		t.Fatal(err)
	}
}
func TestRC634CPUBDiscoveryUsesActualCensusNotNames(t *testing.T) {
	d := t.TempDir()
	path := filepath.Join(d, "branch-matrix", "natural", "live-health", "original")
	if err := os.MkdirAll(path, 0755); err != nil {
		t.Fatal(err)
	}
	p := rc634MakePortfolio("0.67.0.0-RC6.3.4", nil, false)
	// The authoritative runtime PC census has CPU / PC / hits columns.
	data := "cpu\tpc\thits\nCPU-A\t0x00A10C\t9\nCPU-B\t0x010ABC\t12\nCPU-B\t0x010ABE\t7\n"
	if err := os.WriteFile(filepath.Join(path, "rom-execution-pc-counts.tsv"), []byte(data), 0644); err != nil {
		t.Fatal(err)
	}
	count, err := rc634DiscoverCPU_B(d, &p)
	if err != nil {
		t.Fatal(err)
	}
	if count != 2 || len(p.Candidates) != 2 || p.Candidates[0].Stage != "OBSERVED_RUNTIME_NOT_A_FUNCTION" || p.Candidates[0].PromotionAllowed {
		t.Fatalf("incorrect CPU-B census ledger %+v", p)
	}
}
