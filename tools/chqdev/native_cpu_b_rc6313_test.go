package main

import (
	"strings"
	"testing"
)

func TestRC6313PairEvidenceFailClosed(t *testing.T) {
	goodState := "6333\t6333\t0004d8" + strings.Repeat("\t00000000", 16) + "\t2004\t26\n" // 21 cols
	if _, _, _, err := rc6313ParsePairStates([]byte(goodState)); err != nil {
		t.Fatalf("valid pair state rejected: %v", err)
	}
	for _, b := range []string{"", "6333\t6333\t0004d8\n", strings.TrimSuffix(goodState, "\n"), strings.Replace(goodState, "0004d8", "0004dc", 1)} {
		if _, _, _, e := rc6313ParsePairStates([]byte(b)); e == nil {
			t.Fatalf("corrupted CPU-B pair state accepted: %q", b)
		}
	}
	goodBus := []byte("6333\tR\t100804\t0\t2\n")
	if n, e := rc6313PairBus(goodBus); e != nil || n != 1 {
		t.Fatalf("real pair R16 rejected: %v", e)
	}
	for _, b := range []string{"", "6333\tW\t100804\t0\t2\n", "6333\tR\t100806\t0\t2\n", "6333\tR\t100804\t0\t4\n", "6333\tR\t100804\t0\t2"} {
		if _, e := rc6313PairBus([]byte(b)); e == nil {
			t.Fatalf("corrupted R16 accepted: %q", b)
		}
	}
}
