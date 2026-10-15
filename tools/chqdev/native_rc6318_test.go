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

func rc6318Synth(t *testing.T) string {
	t.Helper()
	dir := rc6317Fixture(t)
	for _, c := range rc6317Cases {
		p := filepath.Join(dir, "branch-matrix", c, "live-health", "original", "cpu-b-observed-edges.tsv")
		b0, e0 := os.ReadFile(p)
		if e0 != nil {
			t.Fatal(e0)
		}
		if e0 = os.WriteFile(p, []byte(strings.Replace(string(b0), "4e6\t4ec\t4a79", "4e6\t4ec\t5279", 1)), 0644); e0 != nil {
			t.Fatal(e0)
		}
		f, e := os.OpenFile(p, os.O_APPEND|os.O_WRONLY, 0644)
		if e != nil {
			t.Fatal(e)
		}
		_, e = f.WriteString("4ec\t4f2\t3039\t7\n516\t52c\t6614\t7\n")
		f.Close()
		if e != nil {
			t.Fatal(e)
		}
	}
	rc6318WriteSyntheticStatic(t, dir)
	return dir
}
func rc6318WriteSyntheticStatic(t *testing.T, dir string) {
	t.Helper()
	doc := rc639Static{Schema: "chq-rc639-cpu-b-ghidra-sites-v1", CPU: "B", Language: "68000:BE:32:default", RawROM: strings.Repeat("a", 64), Sites: make([]rc639Site, 5)}
	for _, sp := range rc6318Specs {
		text := sp.mnemonic + ".w 0x1234"
		if sp.branch {
			text = sp.mnemonic + ".b " + rc6317Addr(sp.next)
		}
		row := rc639Site{PC: rc6317Addr(sp.pc), Opcode: spOpcode(sp.opcode), Length: int(sp.length), Text: text, FlowTargets: []string{}}
		if sp.branch {
			row.FlowTargets = []string{rc6317Addr(sp.next)}
		} else {
			next := rc6317Addr(sp.next)
			row.FallThrough = &next
		}
		doc.FrontierSites = append(doc.FrontierSites, row)
	}
	p := filepath.Join(dir, "cpu-b-ghidra", "cpu-b-ghidra-sites.json")
	if e := os.MkdirAll(filepath.Dir(p), 0755); e != nil {
		t.Fatal(e)
	}
	b, e := json.Marshal(doc)
	if e != nil {
		t.Fatal(e)
	}
	if e = os.WriteFile(p, b, 0644); e != nil {
		t.Fatal(e)
	}
}
func spOpcode(op uint32) string {
	return "0x" + strings.ToUpper(strings.TrimPrefix(rc6317Addr(op), "0x0000"))
}
func TestRC6318CrosswalkStrictAndFailClosed(t *testing.T) {
	dir := rc6318Synth(t)
	r, e := rc6318Crosswalk(dir, strings.Repeat("a", 64))
	if e != nil {
		t.Fatal(e)
	}
	if len(r.Sites) != 5 || r.CompleteCFGVerified || r.CompleteFunctionVerified || r.NativePromotionAllowed || r.CoverageIsExhaustive || len(r.Blockers) < 3 {
		t.Fatalf("unsafe report %+v", r)
	}
	for _, s := range r.Sites {
		if s.FunctionReviewed || s.ValidatedAsCompleteFunction {
			t.Fatalf("premature function verification %+v", s)
		}
	}
	if len(r.Sites[3].KnownUnobservedAlternatives) != 1 {
		t.Fatal("unobserved BNE fallthrough not explicitly classified")
	}
	p := filepath.Join(dir, "cpu-b-ghidra", "cpu-b-ghidra-sites.json")
	original, e := os.ReadFile(p)
	if e != nil {
		t.Fatal(e)
	}
	var doc rc639Static
	if e = json.Unmarshal(original, &doc); e != nil {
		t.Fatal(e)
	}
	// Mutation 1: branch target differs from the machine displacement.
	doc.FrontierSites[4].FlowTargets = []string{"0x0000052C"}
	b, _ := json.Marshal(doc)
	_ = os.WriteFile(p, b, 0644)
	if _, e = rc6318Crosswalk(dir, strings.Repeat("a", 64)); e == nil {
		t.Fatal("wrong Ghidra BRA branch accepted")
	}
	// Mutation 2: missing site cannot imply complete function coverage.
	_ = json.Unmarshal(original, &doc)
	doc.FrontierSites = doc.FrontierSites[:4]
	b, _ = json.Marshal(doc)
	_ = os.WriteFile(p, b, 0644)
	if _, e = rc6318Crosswalk(dir, strings.Repeat("a", 64)); e == nil {
		t.Fatal("missing Ghidra site accepted")
	}
	// Mutation 3: byte/length mismatch despite plausible instruction text.
	_ = json.Unmarshal(original, &doc)
	doc.FrontierSites[0].Length = 2
	b, _ = json.Marshal(doc)
	_ = os.WriteFile(p, b, 0644)
	if _, e = rc6318Crosswalk(dir, strings.Repeat("a", 64)); e == nil {
		t.Fatal("incorrect CLR instruction length accepted")
	}
	_ = os.WriteFile(p, original, 0644)
	// Mutation 4: Ghidra ROM fingerprint must match the CPU-B World-ROM sha.
	if _, e = rc6318Crosswalk(dir, strings.Repeat("b", 64)); e == nil {
		t.Fatal("mismatched Ghidra ROM accepted")
	}
	// Mutation 5: authentic original-ROM continuation mutated on one case.
	edge := filepath.Join(dir, "branch-matrix", "terminal", "live-health", "original", "cpu-b-observed-edges.tsv")
	src, _ := os.ReadFile(edge)
	mut := strings.Replace(string(src), "516\t52c\t6614\t7", "516\t518\t6614\t7", 1)
	if e = os.WriteFile(edge, []byte(mut), 0644); e != nil {
		t.Fatal(e)
	}
	if _, e = rc6318Crosswalk(dir, strings.Repeat("a", 64)); e == nil {
		t.Fatal("unverified original-ROM alternative path accepted")
	}
}
func TestRC6318OriginalWindowsEdgesWithSyntheticStatic(t *testing.T) {
	zpath := os.Getenv("CHQ_RC6317_EVIDENCE_ZIP")
	if zpath == "" {
		t.Skip("set CHQ_RC6317_EVIDENCE_ZIP for authentic Windows edge replay")
	}
	z, e := zip.OpenReader(zpath)
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
		fd, er := f.Open()
		if er != nil {
			t.Fatal(er)
		}
		b, er := io.ReadAll(io.LimitReader(fd, 8<<20))
		fd.Close()
		if er != nil {
			t.Fatal(er)
		}
		p := filepath.Join(dest, filepath.FromSlash(f.Name))
		if er = os.MkdirAll(filepath.Dir(p), 0755); er != nil {
			t.Fatal(er)
		}
		if er = os.WriteFile(p, b, 0644); er != nil {
			t.Fatal(er)
		}
		found++
	}
	if found != 5 {
		t.Fatalf("expected 5 original-ROM traces; got %d", found)
	}
	// Static input is deliberately a TEST DOUBLE here: genuine fresh Ghidra
	// instructions will be produced and cross-verified on Windows in the CLI gate.
	rc6318WriteSyntheticStatic(t, dest)
	docpath := filepath.Join(dest, "cpu-b-ghidra", "cpu-b-ghidra-sites.json")
	raw, _ := os.ReadFile(docpath)
	var doc rc639Static
	_ = json.Unmarshal(raw, &doc)
	doc.RawROM = "0e10fe93b9547d089cb0309e1ee5694b0bbc7ce7a52e28990498d0d1f0bd9ae8"
	raw, _ = json.Marshal(doc)
	_ = os.WriteFile(docpath, raw, 0644)
	r, e := rc6318Crosswalk(dest, doc.RawROM)
	if e != nil {
		t.Fatal(e)
	}
	if len(r.Sites) != 5 || r.CompleteFunctionVerified {
		t.Fatalf("bad authentic edge replay %+v", r)
	}
	t.Log("Authentic five original-ROM Windows traces verified against synthetic Ghidra test double; fresh static Ghidra still required on Windows")
}
