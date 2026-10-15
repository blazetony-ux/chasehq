package main

import (
	"bytes"
	"fmt"
	"strings"
	"testing"
)

func TestRC636SecondCandidateFromAuthenticGhidra(t *testing.T) {
	g := rc63Findings{Schema: "chq-ghidra-static-findings-v1", CPU: "A", ROM: strings.Repeat("a", 64)}
	s := rc63Site{PC: "0x040540"}
	for i := 0; i < 16; i++ {
		s.Instructions = append(s.Instructions, rc63Instruction{PC: formatCopyPC(uint64(0x40540 + 2*i)), Text: "move.l (A0)+,(A1)+", Length: 2})
	}
	g.Sites = []rc63Site{s}
	candidates, err := rc63Discover(g)
	if err != nil || len(candidates) != 1 {
		t.Fatalf("copy discovery: %v %d", err, len(candidates))
	}
	if candidates[0].FullFunction || candidates[0].Family != "sequential-longword-copy" {
		t.Fatalf("not a bounded ROM block: %+v", candidates[0])
	}
	code := "struct Cpu { };\nvoid candidate_0(Cpu& c){}\nint main(){return 0;}"
	// Generator must embed an actual candidate invocation and never claim
	// the static window has a reviewed whole-function boundary.
	output, err := rc636CopyLibrarySource(code, candidates)
	if err != nil || !strings.Contains(output, "candidate_0(c)") || !strings.Contains(output, "chq_native_copy_v1") {
		t.Fatalf("module generation: %v", err)
	}
	if _, err := rc636CopyLibrarySource(code, nil); err == nil {
		t.Fatal("missing Ghidra copy block accepted")
	}
	s.Instructions[7].Text = "unsupported-opcode"
	candidates, err = rc63Discover(rc63Findings{Schema: g.Schema, CPU: "A", ROM: g.ROM, Sites: []rc63Site{s}})
	if err != nil {
		t.Fatal(err)
	}
	if _, e := rc636CopyLibrarySource(code, candidates); e == nil {
		t.Fatal("truncated Ghidra copy block incorrectly accepted for live execution")
	}
}

func formatCopyPC(pc uint64) string    { return "0x" + strings.ToUpper(strconvFormatHex(pc)) }
func strconvFormatHex(v uint64) string { return fmt.Sprintf("%08X", v) }

func TestRC636CopyReturnStateFailClosed(t *testing.T) {
	good := []byte("6334\trom\t00040560\t01234567\t2700\t42\n")
	normalized, e := rc636ComparableCopyStates(good)
	if e != nil || !bytes.Equal(normalized, []byte("6334\t00040560\t01234567\t2700\t42\n")) {
		t.Fatalf("unexpected normalisation: %v %q", e, normalized)
	}
	bad := [][]byte{nil, []byte("6334\trom\t12345"), []byte("6334\tUNVERIFIED\t00040560\t01234567\n"), []byte("6334\trom\tshort\n")}
	for _, b := range bad {
		if _, e := rc636ComparableCopyStates(b); e == nil {
			t.Fatalf("accepted invalid state: %q", b)
		}
	}
}
