package main

import (
	"archive/zip"
	"encoding/json"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func rc63Root(t *testing.T) string {
	t.Helper()
	p, e := filepath.Abs(filepath.Join("..", ".."))
	if e != nil {
		t.Fatal(e)
	}
	return p
}

func TestRC63RealGhidraDiscoversThreeDistinctStructures(t *testing.T) {
	root := rc63Root(t)
	g, e := rc63ReadFullGhidra(filepath.Join(root, "research", "acceptance", "ghidra-steering-RC621-verified.zip"))
	if e != nil {
		t.Fatal(e)
	}
	c, e := rc63Discover(g)
	if e != nil {
		t.Fatal(e)
	}
	if len(c) != 3 {
		t.Fatalf("want three real sequences, got %d: %+v", len(c), c)
	}
	families := map[string]bool{}
	hasFullSpan := false
	for _, x := range c {
		families[x.Family] = true
		if x.Family == "conditional-health-function" {
			if !x.FullFunction || x.Entry != "0x0000A10C" || x.End != "0x0000A12A" {
				t.Fatalf("health function Ghidra static range misreported: %+v", x)
			}
			hasFullSpan = true
		}
		if x.Promotion != "BLOCKED" || x.Verification != "STATIC_SUPPORTED_RUNTIME_UNVERIFIED" {
			t.Fatalf("promoted unverified candidate: %+v", x)
		}
	}
	if !hasFullSpan || !families["steering-lookup-tail"] || !families["sequential-longword-copy"] {
		t.Fatalf("wrong families: %+v", families)
	}
}
func TestRC63RejectsMalformedAndUnknown(t *testing.T) {
	if _, e := rc63Address("0X00820D"); e == nil {
		t.Fatal("accepted odd 68000 PC")
	}
	g := rc63Findings{Schema: "chq-ghidra-static-findings-v1", CPU: "B", ROM: strings.Repeat("X", 64), Sites: []rc63Site{{PC: "0x000002"}}}
	if _, e := rc63Discover(g); e == nil {
		t.Fatal("accepted forged ROM hash")
	}
	c := rc63Candidate{Instructions: []rc63Instruction{{PC: "0x000002", Text: "jmp 0x1234", Length: 2}}}
	if _, e := rc63EmitInstructions(c); e == nil {
		t.Fatal("accepted unsupported 68000 instruction")
	}
	c.Instructions = []rc63Instruction{{PC: "0x000002", Text: "bne.b 0x000010", Length: 2}}
	if _, e := rc63EmitInstructions(c); e == nil {
		t.Fatal("accepted escaping conditional branch")
	}
}
func TestRC63PreservesSafetyInOutputAndBundlesOnFailure(t *testing.T) {
	root := rc63Root(t)
	out := filepath.Join(t.TempDir(), "run")
	r := nativeReconstructCommand(root, Config{Release: "0.67.0.0-RC6.3"}, []string{"reconstruct", "--fresh", "--ghidra-zip", filepath.Join(root, "absent.zip"), "--out", out})
	if r.Status != "FAIL" {
		t.Fatalf("bad input returned %s", r.Status)
	}
	b, e := os.ReadFile(filepath.Join(out, "reconstruction.json"))
	if e != nil {
		t.Fatal(e)
	}
	var m map[string]interface{}
	if e = json.Unmarshal(b, &m); e != nil {
		t.Fatal(e)
	}
	if m["nativePromotionAllowed"] != false || m["fullFunctionM1M7SignedOff"] != false {
		t.Fatalf("unsafe report: %+v", m)
	}
	z, e := zip.OpenReader(out + ".zip")
	if e != nil {
		t.Fatal(e)
	}
	defer z.Close()
	if len(z.File) < 2 {
		t.Fatal("missing evidence on failure")
	}
}
func TestRC63GeneratedCppIsGhidraDerived(t *testing.T) {
	root := rc63Root(t)
	g, e := rc63ReadFullGhidra(filepath.Join(root, "research", "acceptance", "ghidra-steering-RC621-verified.zip"))
	if e != nil {
		t.Fatal(e)
	}
	candidates, e := rc63Discover(g)
	if e != nil {
		t.Fatal(e)
	}
	words, exp, e := rc63ReadVectors(filepath.Join(root, "docs", "reference", "steering_reference_193.csv"))
	if e != nil {
		t.Fatal(e)
	}
	code, e := rc63Cpp(candidates, words, exp)
	if e != nil {
		t.Fatal(e)
	}
	for _, want := range []string{"void candidate_0", "void candidate_1", "void candidate_2", "setSubFlags16", "goto pc_A120", "193/193", "health 8/8"} {
		if !strings.Contains(code, want) {
			t.Fatalf("missing generated control logic %s", want)
		}
	}
}
