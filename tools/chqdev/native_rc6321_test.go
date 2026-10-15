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

func rc6321GhidraSites() []rc639Site {
	out := []rc639Site{}
	for _, spec := range []struct {
		pc, op, length int
		text           string
		fall           int
	}{
		{0x4f70, 0x6100, 4, "BSR.W 0x4FB2", 0x4f74},
		{0x4f74, 0x6100, 4, "BSR.W 0x4FB2", 0x4f78},
		{0x4f78, 0x6100, 4, "BSR.W 0x4FB2", 0x4f7c},
		{0x4f7c, 0x4e75, 2, "RTS", 0},
		{0x5062, 0x4e75, 2, "RTS", 0},
	} {
		s := rc639Site{PC: rc6317Addr(uint32(spec.pc)), Opcode: fmt.Sprintf("0x%04X", spec.op), Length: spec.length, Text: spec.text, FlowTargets: []string{}}
		if spec.fall != 0 {
			v := rc6317Addr(uint32(spec.fall))
			s.FallThrough = &v
			s.FlowTargets = []string{rc6317Addr(0x4fb2)}
		}
		out = append(out, s)
	}
	return out
}
func rc6321Seed(t *testing.T, dir string) {
	t.Helper()
	p := filepath.Join(dir, "cpu-b-ghidra", "cpu-b-ghidra-sites.json")
	raw, err := os.ReadFile(p)
	if err != nil {
		t.Fatal(err)
	}
	var st rc639Static
	if err = json.Unmarshal(raw, &st); err != nil {
		t.Fatal(err)
	}
	st.NestedReturnSites = rc6321GhidraSites()
	b, _ := json.Marshal(st)
	if err = os.WriteFile(p, b, 0644); err != nil {
		t.Fatal(err)
	}
	for _, c := range rc6317Cases {
		p := filepath.Join(dir, "branch-matrix", c, "live-health", "original", "cpu-b-observed-edges.tsv")
		f, e := os.OpenFile(p, os.O_APPEND|os.O_WRONLY, 0644)
		if e != nil {
			t.Fatal(e)
		}
		for _, sp := range rc6321Edges {
			fmt.Fprintf(f, "%x\t%x\t%x\t4\n", sp.from, sp.to, sp.opcode)
		}
		f.Close()
	}
}
func TestRC6321NestedReturnAndFailClosedMutations(t *testing.T) {
	dir := rc6318Synth(t)
	rc6319Seed(t, dir)
	rc6321Seed(t, dir)
	hash := strings.Repeat("a", 64)
	r, err := rc6321NestedReturnAudit(dir, hash)
	if err != nil {
		t.Fatal(err)
	}
	if len(r.Edges) != 7 || len(r.GhidraSites) != 5 || r.NativePromotionAllowed || r.TemporalStackPairingVerified || r.CompleteNestedCalleeCFG || r.Unobserved0518Executed {
		t.Fatalf("unsafe nested certificate %+v", r)
	}
	p := filepath.Join(dir, "cpu-b-ghidra", "cpu-b-ghidra-sites.json")
	old, _ := os.ReadFile(p)
	for _, mut := range []struct {
		label  string
		change func(*rc639Static)
	}{
		{"bad-BSR-length", func(st *rc639Static) { st.NestedReturnSites[0].Length = 6 }},
		{"wrong-BSR-target", func(st *rc639Static) { st.NestedReturnSites[1].FlowTargets = []string{rc6317Addr(0x4fb4)} }},
		{"wrong-RTS-opcode", func(st *rc639Static) { st.NestedReturnSites[4].Opcode = "0x4E73" }},
		{"missing-Ghidra", func(st *rc639Static) { st.NestedReturnSites = st.NestedReturnSites[:4] }},
		{"invented-review", func(st *rc639Static) { st.NestedReturnSites[3].FunctionReviewed = true }},
		{"bad-fallthrough", func(st *rc639Static) { x := "0x4F80"; st.NestedReturnSites[2].FallThrough = &x }},
	} {
		var st rc639Static
		_ = json.Unmarshal(old, &st)
		mut.change(&st)
		b, _ := json.Marshal(st)
		_ = os.WriteFile(p, b, 0644)
		if _, err := rc6321NestedReturnAudit(dir, hash); err == nil {
			t.Errorf("accepted Ghidra mutation %s", mut.label)
		}
		_ = os.WriteFile(p, old, 0644)
	}
	edge := filepath.Join(dir, "branch-matrix", "overflow", "live-health", "original", "cpu-b-observed-edges.tsv")
	orig, _ := os.ReadFile(edge)
	for _, mut := range []struct{ label, from, to string }{
		{"wrong-nested-return", "5062\t4f78\t4e75\t4", "5062\t4f7a\t4e75\t4"},
		{"unbalanced-count", "4f78\t4fb2\t6100\t4", "4f78\t4fb2\t6100\t3"},
		{"bad-BSR-opcode", "4f70\t4fb2\t6100\t4", "4f70\t4fb2\tffff\t4"},
		{"missing-return", "4f7c\t8ac\t4e75\t4", ""},
	} {
		s := strings.Replace(string(orig), mut.from, mut.to, 1)
		if s == string(orig) {
			t.Fatal(mut.label, "missing source fixture")
		}
		_ = os.WriteFile(edge, []byte(s), 0644)
		if _, err := rc6321NestedReturnAudit(dir, hash); err == nil {
			t.Errorf("accepted ROM mutation %s", mut.label)
		}
		_ = os.WriteFile(edge, orig, 0644)
	}
}
func TestRC6321ReplayAuthenticNestedROMEdges(t *testing.T) {
	source := os.Getenv("CHQ_RC6320_EVIDENCE_ZIP")
	if source == "" {
		t.Skip("set CHQ_RC6320_EVIDENCE_ZIP to use genuine Windows original-ROM edge evidence")
	}
	z, err := zip.OpenReader(source)
	if err != nil {
		t.Fatal(err)
	}
	defer z.Close()
	dir := rc6318Synth(t)
	rc6319Seed(t, dir)
	rc6321Seed(t, dir)
	// Replace *only* five synthetic edge fixtures with untouched Windows
	// original-ROM TSVs. Static Ghidra nested sites are a test double here:
	// real fresh Ghidra validation happens in the Windows reconstruct gate.
	replaced := 0
	for _, f := range z.File {
		if !strings.HasSuffix(f.Name, "/original/cpu-b-observed-edges.tsv") || !strings.HasPrefix(f.Name, "branch-matrix/") {
			continue
		}
		var known bool
		for _, c := range rc6317Cases {
			if strings.Contains(f.Name, "/"+c+"/") {
				known = true
			}
		}
		if !known {
			continue
		}
		in, err := f.Open()
		if err != nil {
			t.Fatal(err)
		}
		b, err := io.ReadAll(io.LimitReader(in, 3<<20))
		in.Close()
		if err != nil {
			t.Fatal(err)
		}
		p := filepath.Join(dir, filepath.FromSlash(f.Name))
		if err = os.WriteFile(p, b, 0644); err != nil {
			t.Fatal(err)
		}
		replaced++
	}
	if replaced != 5 {
		t.Fatalf("expected five original ROM checkpoint edge files, got %d", replaced)
	}
	// The previous prerequisite Ghidra stub has a synthetic original-ROM
	// hash. Here we replay only the *unmodified* genuine ROM edge tables.
	r, err := rc6321NestedReturnAudit(dir, strings.Repeat("a", 64))
	if err != nil {
		t.Fatal(err)
	}
	for _, c := range rc6317Cases {
		for _, edge := range r.Edges {
			if edge.HitsByCase[c] != 4 {
				t.Fatalf("authentic %s %s->%s count %d", c, edge.From, edge.To, edge.HitsByCase[c])
			}
		}
	}
	t.Logf("authentic five-checkpoint nested return edges PASS; real Ghidra to be checked on Windows; promotions %v", r.NativePromotionAllowed)
}
