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

func rc6319Seed(t *testing.T, dir string) {
	t.Helper()
	p := filepath.Join(dir, "cpu-b-ghidra", "cpu-b-ghidra-sites.json")
	b, e := os.ReadFile(p)
	if e != nil {
		t.Fatal(e)
	}
	var d rc639Static
	if e = json.Unmarshal(b, &d); e != nil {
		t.Fatal(e)
	}
	fall := rc6317Addr(0x518)
	for i := range d.FrontierSites {
		if strings.EqualFold(d.FrontierSites[i].PC, "0x00000516") {
			d.FrontierSites[i].FallThrough = &fall
		}
	}
	d.CallReturnSites = []rc639Site{
		{PC: rc6317Addr(0x50c), Opcode: "0x4E91", Length: 2, Text: "JSR (A1)"},
		{PC: rc6317Addr(0x50e), Opcode: "0x0839", Length: 8, Text: "BTST #0,0x00001234", FallThrough: &[]string{rc6317Addr(0x516)}[0]},
		{PC: rc6317Addr(0x8ac), Opcode: "0x4E75", Length: 2, Text: "RTS"},
	}
	d.UnobservedFallthrough = &rc6319UnobservedSite{PC: "0x000518", RawOpcode: "0x4A79", DisassemblyAvailable: false, FlowTargets: []string{}}
	bb, _ := json.Marshal(d)
	if e = os.WriteFile(p, bb, 0644); e != nil {
		t.Fatal(e)
	}
	for _, c := range rc6317Cases {
		path := filepath.Join(dir, "branch-matrix", c, "live-health", "original", "cpu-b-observed-edges.tsv")
		f, e := os.OpenFile(path, os.O_APPEND|os.O_WRONLY, 0644)
		if e != nil {
			t.Fatal(e)
		}
		_, e = f.WriteString("50c\t6c2\t4e91\t7\n8ac\t50e\t4e75\t7\n50e\t516\t839\t7\n8a4\t8a6\t6606\t4\n8a4\t8ac\t6606\t3\n8a6\t4f70\t4eb9\t4\n")
		f.Close()
		if e != nil {
			t.Fatal(e)
		}
	}
}
func TestRC6319CallReturnAndMutationGuards(t *testing.T) {
	dir := rc6318Synth(t)
	rc6319Seed(t, dir)
	hash := strings.Repeat("a", 64)
	r, e := rc6319CallReturnAudit(dir, hash)
	if e != nil {
		t.Fatal(e)
	}
	if len(r.Edges) != 7 || len(r.GhidraSites) != 3 || r.CompleteFunctionVerified || r.NativePromotionAllowed || r.IndirectTargetSetComplete || r.FullCallerReturnSetVerified || r.UnobservedFallthrough.OriginalRuntimeObserved {
		t.Fatalf("unsafe dossier: %+v", r)
	}
	p := filepath.Join(dir, "cpu-b-ghidra", "cpu-b-ghidra-sites.json")
	original, _ := os.ReadFile(p)
	var doc rc639Static
	mutate := func(label string, edit func(*rc639Static)) {
		t.Helper()
		_ = json.Unmarshal(original, &doc)
		edit(&doc)
		b, _ := json.Marshal(doc)
		_ = os.WriteFile(p, b, 0644)
		if _, e = rc6319CallReturnAudit(dir, hash); e == nil {
			t.Errorf("accepted mutation: %s", label)
		}
		_ = os.WriteFile(p, original, 0644)
	}
	mutate("bad-JSR", func(d *rc639Static) { d.CallReturnSites[0].Opcode = "0x4E75" })
	mutate("wrong-return-length", func(d *rc639Static) { d.CallReturnSites[2].Length = 4 })
	mutate("missing-site", func(d *rc639Static) { d.CallReturnSites = d.CallReturnSites[:2] })
	mutate("forged-BNE-fallthrough", func(d *rc639Static) {
		s := rc6317Addr(0x51a)
		for i := range d.FrontierSites {
			if strings.EqualFold(d.FrontierSites[i].PC, "0x00000516") {
				d.FrontierSites[i].FallThrough = &s
			}
		}
	})
	mutate("claim-unobserved-taken", func(d *rc639Static) { d.UnobservedFallthrough.OriginalRuntimeObserved = true })
	mutate("claim-reviewed", func(d *rc639Static) { d.UnobservedFallthrough.CompleteFunctionReviewed = true })
	mutate("missing-unknown", func(d *rc639Static) { d.UnobservedFallthrough = nil })
	edge := filepath.Join(dir, "branch-matrix", "terminal", "live-health", "original", "cpu-b-observed-edges.tsv")
	old, _ := os.ReadFile(edge)
	for _, test := range []struct{ label, from, to string }{
		{"wrong-RTS-destination", "8ac\t50e\t4e75\t7", "8ac\t510\t4e75\t7"},
		{"bad-JSR-opcode", "50c\t6c2\t4e91\t7", "50c\t6c2\t4e75\t7"},
		{"unbalanced-return", "8ac\t50e\t4e75\t7", "8ac\t50e\t4e75\t5"},
		{"calleebne-missing", "8a4\t8ac\t6606\t3", "8a4\t8ac\t6606\t0"},
		{"unobserved-branch", "516\t52c\t6614\t7", "516\t518\t6614\t7"},
	} {
		b := strings.Replace(string(old), test.from, test.to, 1)
		if b == string(old) {
			t.Fatalf("fixture absent %s", test.label)
		}
		_ = os.WriteFile(edge, []byte(b), 0644)
		if _, e = rc6319CallReturnAudit(dir, hash); e == nil {
			t.Errorf("accepted altered runtime evidence %s", test.label)
		}
		_ = os.WriteFile(edge, old, 0644)
	}
}
func TestRC6319ReplayAuthenticWindowsROMEdges(t *testing.T) {
	source := os.Getenv("CHQ_RC6318_EVIDENCE_ZIP")
	if source == "" {
		t.Skip("set CHQ_RC6318_EVIDENCE_ZIP to replay authentic original ROM call/return paths")
	}
	z, e := zip.OpenReader(source)
	if e != nil {
		t.Fatal(e)
	}
	defer z.Close()
	dest := t.TempDir()
	found := 0
	for _, f := range z.File {
		if !strings.HasSuffix(f.Name, "/live-health/original/cpu-b-observed-edges.tsv") {
			continue
		}
		in, er := f.Open()
		if er != nil {
			t.Fatal(er)
		}
		b, er := io.ReadAll(io.LimitReader(in, 8<<20))
		in.Close()
		if er != nil {
			t.Fatal(er)
		}
		path := filepath.Join(dest, filepath.FromSlash(f.Name))
		_ = os.MkdirAll(filepath.Dir(path), 0755)
		if er = os.WriteFile(path, b, 0644); er != nil {
			t.Fatal(er)
		}
		found++
	}
	if found != 5 {
		t.Fatalf("expected five actual original-ROM traces; got %d", found)
	}
	rc6318WriteSyntheticStatic(t, dest)
	p := filepath.Join(dest, "cpu-b-ghidra", "cpu-b-ghidra-sites.json")
	b, _ := os.ReadFile(p)
	var d rc639Static
	_ = json.Unmarshal(b, &d)
	d.RawROM = "0e10fe93b9547d089cb0309e1ee5694b0bbc7ce7a52e28990498d0d1f0bd9ae8"
	b, _ = json.Marshal(d)
	_ = os.WriteFile(p, b, 0644)
	// Genuine Windows input files, explicitly synthetic Ghidra test double.
	// RC6.3.19 static Ghidra must be separately qualified on Windows.
	rc6319SeedStaticOnly(t, dest)
	result, er := rc6319CallReturnAudit(dest, d.RawROM)
	if er != nil {
		t.Fatal(er)
	}
	if result.Edges[0].HitsByCase["natural"] != 7 || result.Edges[1].HitsByCase["terminal"] != 7 || result.Edges[3].HitsByCase["overflow"] != 4 || result.Edges[4].HitsByCase["overflow"] != 3 {
		t.Fatalf("genuine original-ROM counts changed %+v", result.Edges)
	}
	t.Log("real CHQ6318 original-ROM JSR, callee BNE, RTS, postreturn path PASS; static Ghidra is SYNTHETIC test double")
}
func rc6319SeedStaticOnly(t *testing.T, dir string) {
	t.Helper()
	p := filepath.Join(dir, "cpu-b-ghidra", "cpu-b-ghidra-sites.json")
	b, _ := os.ReadFile(p)
	var d rc639Static
	_ = json.Unmarshal(b, &d)
	s := rc6317Addr(0x518)
	for i := range d.FrontierSites {
		if strings.EqualFold(d.FrontierSites[i].PC, "0x00000516") {
			d.FrontierSites[i].FallThrough = &s
		}
	}
	d.CallReturnSites = []rc639Site{{PC: "0x00050C", Opcode: "0x4E91", Text: "JSR (A1)", Length: 2}, {PC: "0x00050E", Opcode: "0x0839", Text: "BTST #0,0x100000", Length: 8, FallThrough: &[]string{rc6317Addr(0x516)}[0]}, {PC: "0x0008AC", Opcode: "0x4E75", Text: "RTS", Length: 2}}
	d.UnobservedFallthrough = &rc6319UnobservedSite{PC: "0x000518", RawOpcode: "0x0000", FlowTargets: []string{}}
	b, _ = json.Marshal(d)
	_ = os.WriteFile(p, b, 0644)
}
