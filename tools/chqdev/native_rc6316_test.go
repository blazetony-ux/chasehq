package main

import (
	"archive/zip"
	"encoding/json"
	"io"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestRC6316AuthenticBoundedEdgesNeverPromoteFunction(t *testing.T) {
	dest := t.TempDir()
	// Source sites, branch paths and interrupt diversion are explicit;
	// this intentionally cannot prove any enclosing function boundary.
	rows := "4d6\t4d8\t4e73\t4\n52c\t4d8\t60aa\t7\n4d8\t4de\t4a79\t17903\n4de\t4d8\t67f8\t17896\n4de\t4e0\t67f8\t7\n4de\t474\t67f8\t3\n"
	write := func(name, data string) {
		t.Helper()
		p := filepath.Join(dest, "branch-matrix", name, "live-health", "original", "cpu-b-observed-edges.tsv")
		if err := os.MkdirAll(filepath.Dir(p), 0755); err != nil {
			t.Fatal(err)
		}
		if err := os.WriteFile(p, []byte(data), 0644); err != nil {
			t.Fatal(err)
		}
	}
	for _, name := range []string{"natural", "bypass", "nonterminal", "terminal", "overflow"} {
		write(name, rows)
	}
	a, err := rc6316AuditPairEdges(dest, strings.Repeat("a", 64))
	if err != nil {
		t.Fatal(err)
	}
	if len(a.Cases) != 5 || a.Cases[0].PairTakenEdges != 17896 || a.Cases[0].PairFallthroughEdges != 7 || a.Cases[0].InterruptExitEdges != 3 || a.CompleteCFGVerified || a.CompleteFunctionBoundaryVerified || a.AutonomousPromotionAllowed {
		t.Fatalf("unsafe or incomplete edge assessment %+v", a)
	}
	write("natural", strings.Replace(rows, "4de\t4e0\t67f8\t7\n", "", 1))
	if _, err = rc6316AuditPairEdges(dest, strings.Repeat("a", 64)); err == nil {
		t.Fatal("missing fallthrough accepted")
	}
	write("natural", strings.Replace(rows, "4d8\t4de\t4a79", "4d8\t4de\tffff", 1))
	if _, err = rc6316AuditPairEdges(dest, strings.Repeat("a", 64)); err == nil {
		t.Fatal("bad opcode accepted")
	}
	write("natural", rows+"4d8\t4de\t4a79\t1\n")
	if _, err = rc6316AuditPairEdges(dest, strings.Repeat("a", 64)); err == nil {
		t.Fatal("duplicate edge accepted")
	}
}
func TestRC6316FinalPortfolioStageRequiresRealVerifiedPair(t *testing.T) {
	dest, root := t.TempDir(), t.TempDir()
	p := rc634Portfolio{Candidates: []rc634QueueItem{{ID: "B:0x0004D8", CPU: "B", CandidateEntry: "0x0004D8", Stage: "OBSERVED"}, {ID: "B:0x0004DE", CPU: "B", CandidateEntry: "0x0004DE", Stage: "OBSERVED"}, {ID: "B:0x004FD0", CPU: "B", CandidateEntry: "0x004FD0", Stage: "OBSERVED"}}}
	maturity := rc6312CurrentMaturity{CPUBBEQ: "LIVE_VERIFIED_BOUNDED_ONE_INSTRUCTION", CPUBPair: "LIVE_VERIFIED_BOUNDED_TWO_INSTRUCTIONS"}
	if err := rc6316ReconcilePortfolio(dest, root, &p, maturity); err == nil {
		t.Fatal("accepted absent pair evidence")
	}
	proof := rc6313PairLiveProof{Status: "LIVE_GHIDRA_CPU_B_TST_BEQ_PAIR_BOUNDED_PASS", NativeCommits: 8945, CompleteMachineStateEqual: true, OrderedIRQBusEqual: true, RollbackMachineEqual: true}
	b, _ := json.Marshal(proof)
	if err := os.WriteFile(filepath.Join(dest, "cpu-b-pair-live-proof.json"), b, 0644); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(dest, "cpu-b-bounded-edge-audit.json"), []byte("{}"), 0644); err != nil {
		t.Fatal(err)
	}
	if err := rc6316ReconcilePortfolio(dest, root, &p, maturity); err != nil {
		t.Fatal(err)
	}
	if p.Candidates[0].Stage != "LIVE_VERIFIED_BOUNDED_TWO_INSTRUCTIONS" || !p.Candidates[0].LiveNativeVerified || p.Candidates[0].PromotionAllowed || p.Candidates[2].LiveNativeVerified {
		t.Fatalf("portfolio reconciliation failure %+v", p)
	}
	proof.CompleteMachineStateEqual = false
	b, _ = json.Marshal(proof)
	_ = os.WriteFile(filepath.Join(dest, "cpu-b-pair-live-proof.json"), b, 0644)
	if err := rc6316ReconcilePortfolio(dest, root, &p, maturity); err == nil {
		t.Fatal("accepted corrupted live parity")
	}
	proof.CompleteMachineStateEqual = true
	proof.WholeFunctionVerified = true
	b, _ = json.Marshal(proof)
	_ = os.WriteFile(filepath.Join(dest, "cpu-b-pair-live-proof.json"), b, 0644)
	if err := rc6316ReconcilePortfolio(dest, root, &p, maturity); err == nil {
		t.Fatal("accepted inappropriate complete-function promotion")
	}
}

// Optional immutable Windows proof replay. The user ROM set is not shipped;
// this uses only uploaded previous evidence, never raw ROM content.
func TestRC6316AuthenticWindowsEvidenceReconciliation(t *testing.T) {
	source := os.Getenv("CHQ_RC6315_EVIDENCE_ZIP")
	if source == "" {
		t.Skip("set CHQ_RC6315_EVIDENCE_ZIP to authenticate against actual Windows evidence")
	}
	z, err := zip.OpenReader(source)
	if err != nil {
		t.Fatal(err)
	}
	defer z.Close()
	dest, root := t.TempDir(), t.TempDir()
	required := map[string]bool{"candidate-portfolio.json": true, "native-maturity-current.json": true, "cpu-b-pair-live-proof.json": true}
	for _, name := range []string{"natural", "bypass", "nonterminal", "terminal", "overflow"} {
		required[filepath.ToSlash(filepath.Join("branch-matrix", name, "live-health", "original", "cpu-b-observed-edges.tsv"))] = true
	}
	for _, f := range z.File {
		if !required[f.Name] {
			continue
		}
		r, e := f.Open()
		if e != nil {
			t.Fatal(e)
		}
		b, e := io.ReadAll(io.LimitReader(r, 4<<20))
		r.Close()
		if e != nil {
			t.Fatal(e)
		}
		p := filepath.Join(dest, filepath.FromSlash(f.Name))
		if e = os.MkdirAll(filepath.Dir(p), 0755); e != nil {
			t.Fatal(e)
		}
		if e = os.WriteFile(p, b, 0644); e != nil {
			t.Fatal(e)
		}
		delete(required, f.Name)
	}
	if len(required) != 0 {
		t.Fatalf("Windows input missing: %v", required)
	}
	var m rc6312CurrentMaturity
	var portfolio rc634Portfolio
	b, _ := os.ReadFile(filepath.Join(dest, "native-maturity-current.json"))
	if err = json.Unmarshal(b, &m); err != nil {
		t.Fatal(err)
	}
	b, _ = os.ReadFile(filepath.Join(dest, "candidate-portfolio.json"))
	if err = json.Unmarshal(b, &portfolio); err != nil {
		t.Fatal(err)
	}
	edge, err := rc6316AuditPairEdges(dest, "0e10fe93b9547d089cb0309e1ee5694b0bbc7ce7a52e28990498d0d1f0bd9ae8")
	if err != nil || len(edge.Cases) != 5 || edge.Cases[0].PairSequentialEdges != 17903 {
		t.Fatalf("actual Windows pair edge replay failed: %+v %v", edge, err)
	}
	data, _ := json.Marshal(edge)
	_ = os.WriteFile(filepath.Join(dest, "cpu-b-bounded-edge-audit.json"), data, 0644)
	if err = rc6316ReconcilePortfolio(dest, root, &portfolio, m); err != nil {
		t.Fatal(err)
	}
	valid := false
	for _, c := range portfolio.Candidates {
		if c.CPU == "B" && c.CandidateEntry == "0x0004D8" {
			valid = c.LiveNativeVerified && c.Stage == "LIVE_VERIFIED_BOUNDED_TWO_INSTRUCTIONS" && !c.PromotionAllowed
		}
	}
	if !valid {
		t.Fatal("real Windows evidence failed to resolve stale portfolio without granting promotion")
	}
}
