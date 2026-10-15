package main

import (
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestRC637DispatchParserFailsClosed(t *testing.T) {
	dir := t.TempDir()
	p := filepath.Join(dir, "dispatch.tsv")
	for _, invalid := range []string{"", "6334\tA\tA:0000A10C-0000A12A\ta10c\tnative-commit", "6334\tB\tA:0000A10C-0000A12A\ta10c\tnative-commit\n", "6334\tA\tA:0000A10C-0000A12A\ta10c\tunknown\n"} {
		if err := os.WriteFile(p, []byte(invalid), 0644); err != nil {
			t.Fatal(err)
		}
		if _, err := rc637ReadDispatch(p); err == nil {
			t.Fatalf("accepted untrusted/incomplete audit: %q", invalid)
		}
	}
	good := "6334\tA\tA:0000A10C-0000A12A\ta10c\tnative-commit\n"
	if err := os.WriteFile(p, []byte(good), 0644); err != nil {
		t.Fatal(err)
	}
	if n, err := rc637RequireDispatch(p, "A:0000A10C-0000A12A", "native-commit", 1); err != nil || n != 1 {
		t.Fatalf("valid generated native audit rejected: %v", err)
	}
	if _, err := rc637RequireDispatch(p, "A:0000A10C-0000A12A", "original-rom", 1); err == nil {
		t.Fatal("native misrepresented as original ROM")
	}
	if _, err := rc637RequireDispatch(p, "A:0000A10C-0000A12A", "native-commit", 2); err == nil {
		t.Fatal("zero/missing independent run accepted")
	}
}
func TestRC637ObservedEdgesRejectMutation(t *testing.T) {
	p := filepath.Join(t.TempDir(), "edges.tsv")
	good := "4d8\t4de\t4a80\t7\n4d8\t4e4\t4a80\t3\n"
	if err := os.WriteFile(p, []byte(good), 0644); err != nil {
		t.Fatal(err)
	}
	rows, n, err := rc637ReadObservedEdges(p)
	if err != nil || n != 2 || len(rows["B:0x0004D8"]) != 2 {
		t.Fatalf("original ROM edge parse: %v", err)
	}
	for _, bad := range []string{strings.TrimSuffix(good, "\n"), good + "4d8\t4de\t4a80\t1\n", "4d9\t4de\t4a80\t7\n", "4d8\t4de\t4a80\t0\n", "4d8\t4de\t4a80\tbad\n", "20000\t4de\t4a80\t7\n"} {
		if err := os.WriteFile(p, []byte(bad), 0644); err != nil {
			t.Fatal(err)
		}
		if _, _, err := rc637ReadObservedEdges(p); err == nil {
			t.Fatalf("accepted forged edges: %q", bad)
		}
	}
}
func TestRC637ProvenanceFromAuthenticGhidraCandidates(t *testing.T) {
	root := rc63Root(t)
	g, err := rc63ReadFullGhidra(filepath.Join(root, "research", "acceptance", "ghidra-steering-RC621-verified.zip"))
	if err != nil {
		t.Fatal(err)
	}
	c, err := rc63Discover(g)
	if err != nil {
		t.Fatal(err)
	}
	expected := map[string]bool{"A:0000A10C-0000A12A": false, "A:00040540-00040560": false}
	for _, candidate := range c {
		if _, ok := expected[candidate.ID]; ok {
			expected[candidate.ID] = true
		}
	}
	for id, found := range expected {
		if !found {
			t.Fatalf("shared candidate not found in real Ghidra: %s", id)
		}
	}
}
