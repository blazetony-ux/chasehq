package main

import (
	"archive/zip"
	"io"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func rc6317Fixture(t *testing.T) string {
	t.Helper()
	dir := t.TempDir()
	// An original PC graph around the bounded pair; includes RTE and IRQ edges.
	rows := "4d6\t4d8\t4e73\t4\n52c\t4d8\t60aa\t7\n4d8\t4de\t4a79\t17903\n4de\t4d8\t67f8\t17896\n4de\t4e0\t67f8\t7\n4de\t474\t67f8\t3\n4e0\t4e6\t4279\t7\n4e6\t4ec\t4a79\t7\n"
	for _, c := range rc6317Cases {
		p := filepath.Join(dir, "branch-matrix", c, "live-health", "original", "cpu-b-observed-edges.tsv")
		if err := os.MkdirAll(filepath.Dir(p), 0755); err != nil {
			t.Fatal(err)
		}
		if err := os.WriteFile(p, []byte(rows), 0644); err != nil {
			t.Fatal(err)
		}
	}
	return dir
}
func TestRC6317ObservedFrontierAndResearchPriority(t *testing.T) {
	dir := rc6317Fixture(t)
	p, e := rc6317ObservedFrontier(dir, strings.Repeat("a", 64))
	if e != nil {
		t.Fatal(e)
	}
	if p.ObservedEdges != 8 || len(p.Edges) < 6 || len(p.Nodes) < 3 || len(p.ResearchQueue) != 3 || p.CompleteFunctionVerified || p.NativePromotionAllowed || p.CoverageIsExhaustive {
		t.Fatalf("unsafe or incomplete frontier: %+v", p)
	}
	if p.ResearchQueue[0].Target != "0x000004E0" || !p.ResearchQueue[0].AutoSelectedForResearch {
		t.Fatalf("wrong next research focus: %+v", p.ResearchQueue)
	}
	for _, q := range p.ResearchQueue {
		if q.NativePromotionAllowed || q.MeasuredCPUCycleSavings != nil {
			t.Fatal("unearned native performance/promotion proof")
		}
	}
	for _, e := range p.Edges {
		if e.To == "0x00000474" && e.Kind != "observed-irq-diversion" {
			t.Fatal("IRQ misclassified")
		}
	}
}
func TestRC6317FailsClosedOnBadOriginalEdges(t *testing.T) {
	dir := rc6317Fixture(t)
	p := filepath.Join(dir, "branch-matrix", "natural", "live-health", "original", "cpu-b-observed-edges.tsv")
	b, e := os.ReadFile(p)
	if e != nil {
		t.Fatal(e)
	}
	if err := os.WriteFile(p, []byte(strings.Replace(string(b), "4de\t4e0\t67f8\t7\n", "", 1)), 0644); err != nil {
		t.Fatal(err)
	}
	if _, e = rc6317ObservedFrontier(dir, strings.Repeat("a", 64)); e == nil {
		t.Fatal("missing genuine branch accepted")
	}
	_ = os.WriteFile(p, b, 0644)
	if _, e = rc6317ObservedFrontier(dir, "not-a-ROM-hash"); e == nil {
		t.Fatal("bad ROM hash accepted")
	}
	_ = os.WriteFile(p, append(b, []byte("4e0\t4e6\tffff\t1\n")...), 0644)
	if _, e = rc6317ObservedFrontier(dir, strings.Repeat("a", 64)); e == nil {
		t.Fatal("duplicate edge accepted")
	}
}

// Read real Windows evidence without copying original ROMs. The ZIP's
// original-ROM edge TSVs are replayed exactly as supplied by the user.
func TestRC6317AuthenticWindowsFrontier(t *testing.T) {
	source := os.Getenv("CHQ_RC6316_EVIDENCE_ZIP")
	if source == "" {
		t.Skip("set CHQ_RC6316_EVIDENCE_ZIP to replay authentic Windows edges")
	}
	z, err := zip.OpenReader(source)
	if err != nil {
		t.Fatal(err)
	}
	defer z.Close()
	dest := t.TempDir()
	found := 0
	for _, f := range z.File {
		if !strings.HasSuffix(f.Name, "/live-health/original/cpu-b-observed-edges.tsv") {
			continue
		}
		r, e := f.Open()
		if e != nil {
			t.Fatal(e)
		}
		b, e := io.ReadAll(io.LimitReader(r, 8<<20))
		r.Close()
		if e != nil {
			t.Fatal(e)
		}
		p := filepath.Join(dest, filepath.FromSlash(f.Name))
		if e = os.MkdirAll(filepath.Dir(p), 0755); e != nil {
			t.Fatal(e)
		}
		if e = os.WriteFile(p, b, 0644); e != nil {
			t.Fatal(e)
		}
		found++
	}
	if found != 5 {
		t.Fatalf("expected five authentic original-ROM checkpoint traces; got %d", found)
	}
	report, e := rc6317ObservedFrontier(dest, "0e10fe93b9547d089cb0309e1ee5694b0bbc7ce7a52e28990498d0d1f0bd9ae8")
	if e != nil {
		t.Fatal(e)
	}
	if report.ObservedEdges < 5000 || len(report.Edges) < 5 || report.ResearchQueue[0].Target != "0x000004E0" || report.CompleteFunctionVerified || report.NativePromotionAllowed {
		t.Fatalf("unexpected Windows replay frontier: %+v", report)
	}
	t.Logf("authentic original-ROM edges=%d frontier nodes=%d selected edges=%d", report.ObservedEdges, len(report.Nodes), len(report.Edges))
}
