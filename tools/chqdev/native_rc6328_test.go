package main

import (
	"archive/zip"
	"encoding/json"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func rc6328SyntheticBNE(t *testing.T, base string, events []rc6322Hook, transitions uint64) {
	t.Helper()
	var b strings.Builder
	b.WriteString("ordinal\tframe\tcyclesB\tpc\topcode\tsr\tnextPc\tnextFrame\tnextCyclesB\n")
	count := 0
	for _, h := range events {
		if h.PC != 0x516 {
			continue
		}
		// Synthetic next-PC values derived from authenticated aggregate edges.
		// NOT claims of per-visit original-ROM immediate successor observations.
		fmt.Fprintf(&b, "%d\t%d\t%d\t516\t6614\t%x\t52c\t%d\t%d\n", h.Ordinal, h.Frame, h.Cycles, h.SR, h.Frame, h.Cycles+8)
		count++
	}
	if count == 0 {
		t.Fatal("no BNE synthetic seeds")
	}
	if e := os.WriteFile(filepath.Join(base, "cpu-b-original-0516-bne-decisions.tsv"), []byte(b.String()), 0644); e != nil {
		t.Fatal(e)
	}
	cap := map[string]any{"schema": "chq-rc6328-cpu-b-0516-bne-cap-v1", "source": "ORIGINAL_ROM_CPU_B_INSTRUCTION_HOOK_NEXT_PC_AND_SR", "hits": count, "recorded": count, "dropped": 0, "pending": false, "cpuBTransitions": transitions}
	j, _ := json.Marshal(cap)
	if e := os.WriteFile(filepath.Join(base, "cpu-b-original-0516-bne-cap.json"), j, 0644); e != nil {
		t.Fatal(e)
	}
}
func TestRC6328SyntheticBranchDecisionGuards(t *testing.T) {
	base := t.TempDir()
	hooks := []rc6322Hook{{Ordinal: 0, Frame: 6333, Cycles: 100, PC: 0x516, Opcode: 0x6614, SR: 0x0}, {Ordinal: 8, Frame: 6334, Cycles: 180, PC: 0x516, Opcode: 0x6614, SR: 0x8}}
	rc6328SyntheticBNE(t, base, hooks, 100)
	if rows, e := rc6328ReadDecisionEvidence(base, 100); e != nil || len(rows) != 2 {
		t.Fatalf("valid seed rejected %+v: %v", rows, e)
	}
	file := filepath.Join(base, "cpu-b-original-0516-bne-decisions.tsv")
	original, _ := os.ReadFile(file)
	for _, m := range []struct{ name, old, new string }{
		{"incorrect-opcode", "\t516\t6614\t", "\t516\t6606\t"},
		{"wrong-origin", "\t516\t6614\t", "\t518\t6614\t"},
		{"duplicate-ordinal", "8\t6334\t180", "0\t6334\t180"},
		{"backwards-cycle", "\t6333\t108", "\t6333\t99"},
		{"unaligned-target", "\t52c\t", "\t52d\t"},
		{"unexpected-future-frame", "\t52c\t6333\t108", "\t52c\t6337\t108"},
	} {
		mutated := strings.Replace(string(original), m.old, m.new, 1)
		if mutated == string(original) {
			t.Fatalf("mutation not applied %s", m.name)
		}
		os.WriteFile(file, []byte(mutated), 0644)
		if _, e := rc6328ReadDecisionEvidence(base, 100); e == nil {
			t.Errorf("accepted mutation %s", m.name)
		}
	}
	os.WriteFile(file, original, 0644)
	capFile := filepath.Join(base, "cpu-b-original-0516-bne-cap.json")
	capOriginal, _ := os.ReadFile(capFile)
	for _, m := range []struct{ old, new string }{{`"dropped":0`, `"dropped":1`}, {`"pending":false`, `"pending":true`}, {`"hits":2`, `"hits":3`}, {`"cpuBTransitions":100`, `"cpuBTransitions":99`}} {
		changed := strings.Replace(string(capOriginal), m.old, m.new, 1)
		if changed == string(capOriginal) {
			t.Fatalf("cap mutation failed %s", m.old)
		}
		os.WriteFile(capFile, []byte(changed), 0644)
		if _, e := rc6328ReadDecisionEvidence(base, 100); e == nil {
			t.Errorf("accepted cap mutation %s", m.old)
		}
	}
	os.WriteFile(capFile, capOriginal, 0644)
	os.Remove(file)
	if _, e := rc6328ReadDecisionEvidence(base, 100); e == nil {
		t.Fatal("accepted missing bounded capture")
	}
}

// OPTIONAL genuine prior-evidence integration. Original Ghidra/CPU-B edges,
// SR/ordinal samples are from user's immutable Windows RC6.3.27 evidence;
// NEW immediate next-PC rows are explicitly synthetic, awaiting RC6.3.28.
func TestRC6328GenuineRC6327PrerequisiteWithSyntheticNextPC(t *testing.T) {
	source := os.Getenv("CHQ_RC6327_EVIDENCE_ZIP")
	if source == "" {
		t.Skip("set CHQ_RC6327_EVIDENCE_ZIP to replay genuine predecessor evidence")
	}
	z, e := zip.OpenReader(source)
	if e != nil {
		t.Fatal(e)
	}
	defer z.Close()
	dir := t.TempDir()
	count := 0
	for _, f := range z.File {
		n := filepath.ToSlash(f.Name)
		if f.FileInfo().IsDir() || !(strings.HasPrefix(n, "branch-matrix/") && strings.Contains(n, "/original/") || strings.Contains(n, "ghidra") || strings.Contains(n, "cpu-b-nested") || strings.Contains(n, "cpu-b-original")) {
			continue
		}
		if strings.Contains(n, "..") || strings.HasPrefix(n, "/") || f.UncompressedSize64 > 20<<20 {
			t.Fatalf("unsafe source path %s", n)
		}
		dst := filepath.Join(dir, filepath.FromSlash(n))
		if e := os.MkdirAll(filepath.Dir(dst), 0755); e != nil {
			t.Fatal(e)
		}
		in, e := f.Open()
		if e != nil {
			t.Fatal(e)
		}
		b, e := io.ReadAll(io.LimitReader(in, 20<<20+1))
		in.Close()
		if e != nil {
			t.Fatal(e)
		}
		if e := os.WriteFile(dst, b, 0644); e != nil {
			t.Fatal(e)
		}
		count++
	}
	if count < 50 {
		t.Fatalf("too little original evidence %d", count)
	}
	ghidra, err := os.ReadFile(filepath.Join(dir, "cpu-b-ghidra", "cpu-b-ghidra-sites.json"))
	if err != nil {
		t.Fatal(err)
	}
	var g struct {
		Raw string `json:"rawRomSha256"`
	}
	if json.Unmarshal(ghidra, &g) != nil || len(g.Raw) != 64 {
		t.Fatal("bad original ROM identity")
	}
	if _, e := rc6319CallReturnAudit(dir, g.Raw); e != nil {
		t.Fatalf("genuine Ghidra/call-return gate: %v", e)
	}
	if _, e := rc6328BranchFrontierAudit(dir, g.Raw); e == nil {
		t.Fatal("missing NEW per-visit next-PC observations accepted as original")
	}
	for _, name := range rc6317Cases {
		hooks, trans, e := rc6322ReadHooks(dir, name)
		if e != nil {
			t.Fatal(e)
		}
		rc6328SyntheticBNE(t, filepath.Join(dir, "branch-matrix", name, "live-health", "original"), hooks, trans)
	}
	r, e := rc6328BranchFrontierAudit(dir, g.Raw)
	if e != nil {
		t.Fatal(e)
	}
	visits := 0
	for _, c := range r.Cases {
		if c.BranchVisits != 7 || c.BNENotZeroTaken != 7 || c.BNEZeroFallthrough != 0 || c.ZFlagSetAtBranch != 0 || c.OriginalROMEdgeCount != 7 {
			t.Fatalf("unexpected case %+v", c)
		}
		visits += c.BranchVisits
	}
	if visits != 35 || !r.RecordedSRMatchesSelectedOriginalHooks || !r.ObservedNextPCMatchesBranchCondition || !r.OrderedVisitsMatchOriginalROMEdges || r.Observed0518Fallthrough || r.NativePromotionAllowed || r.EntirePathCoverageVerified {
		t.Fatalf("unsafe result %+v", r)
	}
	t.Log("PASS 35 authentic RC6.3.27 SR/ordinal and edge prerequisites, SYNTHETIC newly added next-PC fields; fresh RC6.3.28 ROM required")
	base := filepath.Join(dir, "branch-matrix", "natural", "live-health", "original")
	file := filepath.Join(base, "cpu-b-original-0516-bne-decisions.tsv")
	original, _ := os.ReadFile(file)
	for _, m := range []struct{ name, old, new string }{{"wrong-Z-flag", "\t6614\t0\t52c", "\t6614\t4\t52c"}, {"wrong-next-PC", "\t52c\t6333", "\t518\t6333"}, {"lost-event", "\t52c\t6333", "\t474\t6333"}} {
		modified := strings.Replace(string(original), m.old, m.new, 1)
		if modified == string(original) {
			t.Fatalf("mutation absent %s", m.name)
		}
		os.WriteFile(file, []byte(modified), 0644)
		if _, e := rc6328BranchFrontierAudit(dir, g.Raw); e == nil {
			t.Errorf("accepted %s", m.name)
		}
	}
	os.WriteFile(file, original, 0644)
}
