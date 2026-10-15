package main

import (
	"encoding/json"
	"testing"
)

// Verify the real Windows 0x00820C census key correlates to static
// Ghidra nearby instructions, not the previous zero-hit false negative.
func TestRC626GhidraCoverageLedgerCanonicalPCs(t *testing.T) {
	g := nativeGhidra{RawRomSHA: "verified-rom-sha"}
	g.Sites = []nativeFinding{{PC: "0x00820C", Instructions: []struct {
		PC   string `json:"pc"`
		Text string `json:"text"`
	}{{PC: "0x00820C", Text: "MOVE.W"}, {PC: "0X008210", Text: "RTS"}, {PC: "0x00820c", Text: "duplicate"}}}}
	ledger := nativeSteeringCoverage(g, map[string]int64{"A:0x00820C": 240, "A:0x008210": 240})
	if ledger["localWindowDistinctStaticPCs"] != 2 || ledger["localWindowObservedPCs"] != 2 || ledger["localWindowRuntimeInstructionHits"] != int64(480) {
		t.Fatalf("incorrect ledger: %v", ledger)
	}
	encoded, err := json.Marshal(ledger)
	if err != nil || len(encoded) == 0 {
		t.Fatal("ledger not serializable", err)
	}
}
