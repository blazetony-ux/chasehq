package main

import (
	"archive/zip"
	"encoding/json"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestRC6315UnprovenFunctionDossierFailsClosed(t *testing.T) {
	g := rc63Findings{Schema: "chq-ghidra-static-findings-v1", CPU: "B", ROM: strings.Repeat("a", 64)}
	c := rc63Candidate{ID: "B:000004D8-000004E0", CPU: "B", Entry: "0x4d8", End: "0x4e0", FunctionCandidate: "0x4c0", FullFunction: false, Instructions: []rc63Instruction{{PC: "0x4d8", Text: "tst.w (0x100804).l", Length: 6}, {PC: "0x4de", Text: "beq.b 0x4d8", Length: 2}}}
	d, e := rc6315AssessFunctions(g, []rc63Candidate{c})
	if e != nil {
		t.Fatal(e)
	}
	if len(d.Candidates) != 1 || d.EntireFunctionVerified || d.FunctionNativePromotionAllowed || d.Candidates[0].FunctionNativePromotionAllowed || d.Candidates[0].EndsInRTS {
		t.Fatalf("false whole-function promotion: %+v", d)
	}
	c.FullFunction = true
	c.Instructions[1].Text = "rts" // Even misleading Ghidra range+RTS cannot auto-authorize function promotion.
	d, e = rc6315AssessFunctions(g, []rc63Candidate{c})
	if e != nil || d.FunctionNativePromotionAllowed || d.Candidates[0].FunctionNativePromotionAllowed {
		t.Fatalf("static evidence bypass: %+v %v", d, e)
	}
	c.Instructions[1].PC = "0x4e2"
	d, e = rc6315AssessFunctions(g, []rc63Candidate{c})
	if e != nil || d.Candidates[0].Contiguous {
		t.Fatalf("gap undetected: %+v %v", d, e)
	}
}
func TestRC6315EvidenceOutputPreservesPreviousRuns(t *testing.T) {
	parent := filepath.Join(t.TempDir(), "Evidence")
	dest := filepath.Join(parent, "CHQ6315")
	if e := prepareNativeEvidenceDestination(dest); e != nil {
		t.Fatal(e)
	}
	if e := os.WriteFile(filepath.Join(dest, "proof.txt"), []byte("test"), 0644); e != nil {
		t.Fatal(e)
	}
	if e := bundleNativeEvidence(dest); e != nil {
		t.Fatal(e)
	}
	if e := prepareNativeEvidenceDestination(dest); e == nil {
		t.Fatal("reused prior proof")
	}
	if e := bundleNativeEvidence(dest); e == nil {
		t.Fatal("overwrote immutable previous ZIP")
	}
	if e := prepareNativeEvidenceDestination(dest + "-RUN002"); e != nil {
		t.Fatal(e)
	}
	z, e := zip.OpenReader(dest + ".zip")
	if e != nil {
		t.Fatal(e)
	}
	defer z.Close()
	if len(z.File) != 1 || z.File[0].Name != "proof.txt" {
		t.Fatal("unexpected evidence ZIP")
	}
}
func TestRC6315FirstDivergenceIsFailClosed(t *testing.T) {
	if _, bad := rc6315FirstDivergence("B", "cpu", []byte("same\n"), []byte("same\n")); bad {
		t.Fatal("false mismatch")
	}
	m, bad := rc6315FirstDivergence("B", "cpu", []byte("a\nb\nc\n"), []byte("a\nx\nc\n"))
	if !bad || m.FirstDifferentLine != 2 || m.OriginalRow != "b" || m.NativeRow != "x" || m.PromotionAllowed || !strings.Contains(m.MinimizedTo, "NOT YET VERIFIED") {
		t.Fatalf("bad minimization provenance %+v", m)
	}
}
func TestRC6315PairSideEffectOwnership(t *testing.T) {
	dest := t.TempDir()
	bus := "6333\tR\t100804\t0\t2\n"
	irq := "6333\tA\t4d8\t4\t0\n6333\tR\t70\t474\t4\n6333\tW\t1003fc\t4d8\t4\n6333\tW\t1003fa\t4\t2\n"
	write := func(mode, b, i string) {
		t.Helper()
		p := filepath.Join(dest, "live-cpu-b-pair", mode)
		if e := os.MkdirAll(p, 0755); e != nil {
			t.Fatal(e)
		}
		for n, v := range map[string]string{"native-cpu-b-pair-bus.tsv": b, "native-cpu-b-pair-irq.tsv": i} {
			if e := os.WriteFile(filepath.Join(p, n), []byte(v), 0644); e != nil {
				t.Fatal(e)
			}
		}
	}
	for _, m := range []string{"original", "generated-native", "forced-mismatch"} {
		write(m, bus, irq)
	}
	a, e := rc6315PairSideEffects(dest, strings.Repeat("a", 64))
	if e != nil {
		t.Fatal(e)
	}
	if len(a.Modes) != 3 || a.WholeFunctionVerified || a.CompositionVerified || !a.OriginalNativeDataEqual {
		t.Fatalf("false promotion %+v", a)
	}
	raw, _ := json.Marshal(a)
	if !strings.Contains(string(raw), "unattributedEffects") {
		t.Fatal("missing uncertainty contract")
	}
	write("generated-native", "6333\tW\t100804\t0\t2\n", irq)
	if _, e = rc6315PairSideEffects(dest, strings.Repeat("a", 64)); e == nil {
		t.Fatal("unreviewed memory write accepted")
	}
	write("generated-native", bus, strings.Replace(irq, "1003fc", "1003fd", 1))
	if _, e = rc6315PairSideEffects(dest, strings.Repeat("a", 64)); e == nil {
		t.Fatal("bad IRQ attribution accepted")
	}
}
