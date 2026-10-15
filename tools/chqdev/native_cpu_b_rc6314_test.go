package main

import (
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestRC6314SingleBEQAcceptsOnlyInactivePair(t *testing.T) {
	p := filepath.Join(t.TempDir(), "audit.tsv")
	const beq = "B:000004DE-000004E0"
	const pair = "B:000004D8-000004E0"
	audit := "6333\tA\tA:00040540-00040560\t40540\toriginal-rom\n" +
		"6333\tB\t" + pair + "\t4d8\toriginal-rom\n" +
		"6333\tB\t" + beq + "\t4de\tnative-commit\n" +
		"6333\tB\t" + beq + "\t4de\toriginal-fallback\n"
	write := func(s string) {
		t.Helper()
		if e := os.WriteFile(p, []byte(s), 0644); e != nil {
			t.Fatal(e)
		}
	}
	write(audit)
	if e := rc6310DispatchCounts(p, beq, 2, 1, 1); e != nil {
		t.Fatalf("valid real RC6.3.13 companion rejected: %v", e)
	}
	if e := rc6314InactiveCPUBAudit(p, beq); e != nil {
		t.Fatal(e)
	}
	cases := []string{
		strings.Replace(audit, pair+"\t4d8\toriginal-rom", pair+"\t4d8\tnative-commit", 1),
		strings.Replace(audit, pair+"\t4d8\toriginal-rom", pair+"\t4d8\toriginal-fallback", 1),
		strings.Replace(audit, pair+"\t4d8\toriginal-rom", pair+"\t4de\toriginal-rom", 1),
		strings.Replace(audit, pair+"\t4d8\toriginal-rom", "B:000004D6-000004E0\t4d8\toriginal-rom", 1),
		strings.Replace(audit, "6333\tB\t"+pair, "6341\tB\t"+pair, 1),
		strings.Replace(audit, "6333\tB\t"+pair, "6333\tA\t"+pair, 1),
	}
	for i, bad := range cases {
		write(bad)
		if e := rc6310DispatchCounts(p, beq, 2, 1, 1); e == nil && i < 5 {
			t.Fatalf("accepted invalid CPU-B audit mutation %d", i)
		}
		if e := rc6314InactiveCPUBAudit(p, beq); e == nil {
			t.Fatalf("accepted invalid inactive pair mutation %d", i)
		}
	}
}

func TestRC6314PairDispatchAuditFailClosed(t *testing.T) {
	p := filepath.Join(t.TempDir(), "audit.tsv")
	const pair = "B:000004D8-000004E0"
	const beq = "B:000004DE-000004E0"
	wr := func(s string) {
		t.Helper()
		if e := os.WriteFile(p, []byte(s), 0644); e != nil {
			t.Fatal(e)
		}
	}
	baseline := "6333\tB\t" + pair + "\t4d8\toriginal-rom\n6333\tB\t" + beq + "\t4de\toriginal-rom\n"
	wr(baseline)
	if e := rc6314PairDispatchAudit(p, "original", 0, 0, 0); e != nil {
		t.Fatal(e)
	}
	native := "6333\tB\t" + pair + "\t4d8\tnative-commit\n6333\tB\t" + pair + "\t4d8\toriginal-fallback\n6333\tB\t" + beq + "\t4de\toriginal-rom\n"
	wr(native)
	if e := rc6314PairDispatchAudit(p, "generated-native", 1, 1, 0); e != nil {
		t.Fatal(e)
	}
	if e := rc6314PairDispatchAudit(p, "forced-mismatch", 1, 0, 1); e != nil {
		t.Fatal(e)
	}
	for i, broken := range []string{
		strings.Replace(native, beq+"\t4de\toriginal-rom", beq+"\t4de\tnative-commit", 1),
		strings.Replace(native, pair+"\t4d8\tnative-commit", pair+"\t4d8\toriginal-rom", 1),
		strings.Replace(native, pair+"\t4d8\tnative-commit", "B:000004D6-000004E0\t4d8\tnative-commit", 1),
		strings.Replace(native, pair+"\t4d8\tnative-commit", pair+"\t4de\tnative-commit", 1),
		strings.TrimSuffix(native, "\n"),
	} {
		wr(broken)
		if e := rc6314PairDispatchAudit(p, "generated-native", 1, 1, 0); e == nil {
			t.Fatalf("accepted bad pair audit %d", i)
		}
	}
}
