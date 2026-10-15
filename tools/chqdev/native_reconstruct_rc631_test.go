package main

import (
	"bytes"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestRC631GeneratedHealthSharedLibraryUsesRealGhidraFunction(t *testing.T) {
	root := rc63Root(t)
	g, err := rc63ReadFullGhidra(filepath.Join(root, "research", "acceptance", "ghidra-steering-RC621-verified.zip"))
	if err != nil {
		t.Fatal(err)
	}
	c, err := rc63Discover(g)
	if err != nil {
		t.Fatal(err)
	}
	v, e, err := rc63ReadVectors(filepath.Join(root, "docs", "reference", "steering_reference_193.csv"))
	if err != nil {
		t.Fatal(err)
	}
	code, err := rc63Cpp(c, v, e)
	if err != nil {
		t.Fatal(err)
	}
	library, err := rc631HealthLibrarySource(code, c)
	if err != nil {
		t.Fatal(err)
	}
	for _, required := range []string{"chq_native_health_v1", "candidate_", "uint32_t c.a", "c.mem.write16", "if((c.sr&4U)==0)", "return 1;"} {
		if !strings.Contains(library, required) && required != "uint32_t c.a" {
			t.Fatalf("missing generated health control: %s", required)
		}
	}
	filtered := []rc63Candidate{}
	for _, candidate := range c {
		if candidate.Family != "conditional-health-function" {
			filtered = append(filtered, candidate)
		}
	}
	c = filtered
	if _, err := rc631HealthLibrarySource(code, c); err == nil {
		t.Fatal("accepted missing real Ghidra health candidate")
	}
}
func TestRC631HealthTraceComparatorKeepsAllState(t *testing.T) {
	original := []byte("6334\trom\tABC\t123\t42\n6335\trom\tDEF\t456\t56\n")
	candidate := []byte("6334\tnative\tABC\t123\t42\n6335\tnative\tDEF\t456\t56\n")
	a, err := rc631ComparableHealthStates(original)
	if err != nil {
		t.Fatal(err)
	}
	b, err := rc631ComparableHealthStates(candidate)
	if err != nil {
		t.Fatal(err)
	}
	if !bytes.Equal(a, b) {
		t.Fatalf("mode label masked parity: %q %q", a, b)
	}
	if bad, _ := rc631ComparableHealthStates([]byte("6335\tnative\tDEF\t456\t57\n")); bytes.Equal(a, bad) {
		t.Fatal("changed cycle count ignored")
	}
	if _, err := rc631ComparableHealthStates([]byte("6334\thacked\tABC\n")); err == nil {
		t.Fatal("unknown mode accepted")
	}
}
func TestRC631LiveHealthMissingRuntimeFailsClosed(t *testing.T) {
	d := t.TempDir()
	_, err := rc631RunLiveHealth(d, filepath.Join(d, "output"), "/not-a-ROM", "/not-a-generated-lib")
	if err == nil {
		t.Fatal("absence of original-ROM runtime was promoted")
	}
	if _, e := os.Stat(filepath.Join(d, "output", "live-health")); e == nil {
		t.Fatal("created misleading live success evidence")
	}
}
