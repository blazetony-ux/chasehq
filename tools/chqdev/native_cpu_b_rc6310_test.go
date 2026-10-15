package main

import (
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestRC6310FullCPU_BBranchStateAuditFailsClosed(t *testing.T) {
	// entryFrame, exitFrame, PC, 8 data, 8 address, SR, cumulative cycles.
	mk := func(pc string) string {
		v := []string{"6333", "6333", pc}
		for i := 0; i < 16; i++ {
			v = append(v, "00000000")
		}
		v = append(v, "2704", "20")
		return strings.Join(v, "\t") + "\n"
	}
	t1, n1, i1, e := rc6310ParseCPU_BReturns([]byte(mk("0004d8") + mk("0004e0") + mk("000474")))
	if e != nil || t1 != 1 || n1 != 1 || i1 != 1 {
		t.Fatalf("valid CPU-B branch + IRQ evidence rejected: %v %d/%d/%d", e, t1, n1, i1)
	}
	for _, broken := range []string{
		mk("0004d8")[:len(mk("0004d8"))-1],
		strings.Replace(mk("0004d8"), "0004d8", "0004d9", 1),
		strings.Replace(mk("0004d8"), "0004d8", "0004d0", 1),
		strings.Replace(mk("0004d8"), "20\n", "NaN\n", 1),
		strings.Replace(mk("0004d8"), "6333", "9999", 1),
		strings.Replace(mk("0004d8"), "2704", "xxxx", 1),
	} {
		if _, _, _, e = rc6310ParseCPU_BReturns([]byte(broken)); e == nil {
			t.Fatalf("accepted invalid CPU-B full return state %q", broken)
		}
	}
}
func TestRC6310DispatchRejectsSilentSubstitution(t *testing.T) {
	p := filepath.Join(t.TempDir(), "native-dispatch.tsv")
	id := "B:000004DE-000004E0"
	if e := os.WriteFile(p, []byte("6333\tB\t"+id+"\t4de\tnative-commit\n6333\tB\t"+id+"\t4de\toriginal-fallback\n"), 0644); e != nil {
		t.Fatal(e)
	}
	if e := rc6310DispatchCounts(p, id, 2, 1, 1); e != nil {
		t.Fatal(e)
	}
	for _, bad := range []struct {
		total           int
		commits, guards int64
	}{{2, 2, 0}, {1, 1, 0}, {2, 0, 2}} {
		if e := rc6310DispatchCounts(p, id, bad.total, bad.commits, bad.guards); e == nil {
			t.Fatalf("accepted forged dispatch counts %+v", bad)
		}
	}
	for _, bad := range []string{"6333\tA\t" + id + "\t4de\tnative-commit\n", "6333\tB\t" + id + "\t4e0\tnative-commit\n", "6333\tB\t" + id + "\t4de\tunknown\n"} {
		if e := os.WriteFile(p, []byte(bad), 0644); e != nil {
			t.Fatal(e)
		}
		if e := rc6310DispatchCounts(p, id, 1, 1, 0); e == nil {
			t.Fatalf("accepted forged dispatcher row %q", bad)
		}
	}
}
func TestRC6310NoROMDoesNotPromote(t *testing.T) {
	d := t.TempDir()
	p, e := rc6310RunCPU_BLive(d, filepath.Join(d, "evidence"), filepath.Join(d, "missing"), filepath.Join(d, "missing.dll"))
	if e == nil || p.AutonomousPromotionAllowed || p.WholeFunctionVerified || p.Status != "BLOCKED" {
		t.Fatalf("CPU-B no-ROM test not fail-closed: %+v %v", p, e)
	}
}
