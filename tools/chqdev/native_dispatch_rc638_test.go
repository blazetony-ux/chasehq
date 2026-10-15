package main

import (
	"archive/zip"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

// Regression for the real RC6.3.7 Windows failure: rom-execution-pc-counts.tsv
// uses CPU-B\t0x0004D8; the raw CPU-B edge TSV uses the minimal 4d8 PC.
// This test goes through the WHOLE shared-dispatch acceptance rather than just
// exercising an edge parser with matching, artificially padded keys.
func rc638WriteFile(t *testing.T, root, rel, body string) {
	t.Helper()
	p := filepath.Join(root, filepath.FromSlash(rel))
	if err := os.MkdirAll(filepath.Dir(p), 0755); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(p, []byte(body), 0644); err != nil {
		t.Fatal(err)
	}
}
func rc638SyntheticRC637WindowsEvidence(t *testing.T) string {
	t.Helper()
	dest := t.TempDir()
	health := "A:0000A10C-0000A12A"
	copy := "A:00040540-00040560"
	for _, c := range []string{"natural", "bypass", "nonterminal", "terminal", "overflow"} {
		for _, m := range []struct{ dir, outcome string }{{"original", "original-rom"}, {"generated-native", "native-commit"}, {"forced-mismatch", "original-fallback"}} {
			rc638WriteFile(t, dest, "branch-matrix/"+c+"/live-health/"+m.dir+"/native-dispatch.tsv", "6334\tA\t"+health+"\ta10c\t"+m.outcome+"\n")
		}
	}
	for _, m := range []struct {
		dir, outcome string
		copies       int
	}{{"original", "original-rom", 4}, {"generated-native", "native-commit", 4}, {"forced-mismatch", "original-fallback", 1}} {
		var rows strings.Builder
		for i := 0; i < m.copies; i++ {
			fmt.Fprintf(&rows, "%d\tA\t%s\t40540\t%s\n", 6333+i, copy, m.outcome)
		}
		rc638WriteFile(t, dest, "live-copy/"+m.dir+"/native-dispatch.tsv", rows.String())
	}
	// Actual five ranked original-ROM PC hit counts from the uploaded RC6.3.7 run.
	rc638WriteFile(t, dest, "branch-matrix/natural/live-health/original/rom-execution-pc-counts.tsv",
		"cpu\tpc\tinstructionHits\nCPU-B\t0x0004D8\t17907\nCPU-B\t0x0004DE\t17906\nCPU-B\t0x004FD0\t1512\nCPU-B\t0x004FD2\t1512\nCPU-B\t0x004FD4\t1512\n")
	// Actual observed edge totals: at 0x4d8 the final callback has no next-PC.
	rc638WriteFile(t, dest, "branch-matrix/natural/live-health/original/cpu-b-observed-edges.tsv",
		"4d8\t474\t4a79\t3\n4d8\t4de\t4a79\t17903\n4de\t474\t67f8\t3\n4de\t4d8\t67f8\t17896\n4de\t4e0\t67f8\t7\n4fd0\t4fd2\t48e7\t1512\n4fd2\t4fd4\t1234\t1512\n4fd4\t4fd6\t5678\t1512\n")
	return dest
}
func TestRC638FullSharedAuditAcceptsRealWindowsPCWidths(t *testing.T) {
	dest := rc638SyntheticRC637WindowsEvidence(t)
	p, err := rc637SharedLiveProof(dest, rc636CopyProof{Status: "LIVE_GENERATED_COPY_BLOCK_PASS", OriginalReturns: 4})
	if err != nil {
		t.Fatal(err)
	}
	if p.Status != "LIVE_SHARED_DISPATCH_AND_OBSERVED_CPU_B_EDGES_PASS" || p.HealthCasesVerified != 5 || p.NativeCommits != 9 || p.ForcedFallbacks != 6 || len(p.CPU_BCandidates) != 5 || p.CPU_BObservedEdges != 8 {
		t.Fatalf("incorrect full proof: %+v", p)
	}
	if p.CPU_BCandidates[0].PC != "0x0004D8" || p.CPU_BCandidates[0].OriginalHits != 17907 || p.CPU_BCandidates[0].EdgeHits != 17906 {
		t.Fatalf("real ROM PC incorrectly correlated: %+v", p.CPU_BCandidates[0])
	}
	if p.NativePromotionAllowed || p.CPU_BNativeSubstitutionProven {
		t.Fatal("unsafe promotion from read-only edges")
	}
	edgePath := filepath.Join(dest, "branch-matrix", "natural", "live-health", "original", "cpu-b-observed-edges.tsv")
	valid, err := os.ReadFile(edgePath)
	if err != nil {
		t.Fatal(err)
	}
	for name, mutation := range map[string]string{
		"missing-hot-edge":     strings.Replace(string(valid), "4d8\t474\t4a79\t3\n4d8\t4de\t4a79\t17903\n", "", 1),
		"incorrect-hits":       strings.Replace(string(valid), "4d8\t4de\t4a79\t17903", "4d8\t4de\t4a79\t17902", 1),
		"contradictory-opcode": strings.Replace(string(valid), "4d8\t474\t4a79\t3", "4d8\t474\t4a80\t3", 1),
		"duplicate-edge":       string(valid) + "4d8\t4de\t4a79\t1\n",
	} {
		if err := os.WriteFile(edgePath, []byte(mutation), 0644); err != nil {
			t.Fatal(err)
		}
		if p, e := rc637SharedLiveProof(dest, rc636CopyProof{Status: "LIVE_GENERATED_COPY_BLOCK_PASS", OriginalReturns: 4}); e == nil {
			t.Errorf("%s: accepted corrupted original-ROM edge evidence: %+v", name, p)
		}
	}
}

// Optional forensic regression: the original RC6.3.7 Windows failure ZIP is
// NOT bundled in source. Set CHQ_RC637_EVIDENCE_ZIP to exercise its REAL bytes.
func TestRC638OriginalWindowsFailureEvidence(t *testing.T) {
	path := os.Getenv("CHQ_RC637_EVIDENCE_ZIP")
	if path == "" {
		t.Skip("set CHQ_RC637_EVIDENCE_ZIP to original RC6.3.7 Windows evidence ZIP")
	}
	z, e := zip.OpenReader(path)
	if e != nil {
		t.Fatal(e)
	}
	defer z.Close()
	dest := t.TempDir()
	for _, f := range z.File {
		name := filepath.ToSlash(f.Name)
		if !strings.HasSuffix(name, "native-dispatch.tsv") && name != "branch-matrix/natural/live-health/original/rom-execution-pc-counts.tsv" && name != "branch-matrix/natural/live-health/original/cpu-b-observed-edges.tsv" {
			continue
		}
		if strings.HasPrefix(name, "/") || strings.Contains(name, "..") {
			t.Fatalf("unsafe ZIP path %q", name)
		}
		target := filepath.Join(dest, filepath.FromSlash(name))
		if e := os.MkdirAll(filepath.Dir(target), 0755); e != nil {
			t.Fatal(e)
		}
		src, e := f.Open()
		if e != nil {
			t.Fatal(e)
		}
		out, e := os.Create(target)
		if e != nil {
			src.Close()
			t.Fatal(e)
		}
		_, copyErr := io.Copy(out, src)
		outErr := out.Close()
		srcErr := src.Close()
		if copyErr != nil || outErr != nil || srcErr != nil {
			t.Fatalf("archive copy: %v %v %v", copyErr, outErr, srcErr)
		}
	}
	proof, err := rc637SharedLiveProof(dest, rc636CopyProof{Status: "LIVE_GENERATED_COPY_BLOCK_PASS", OriginalReturns: 4})
	if err != nil {
		t.Fatal(err)
	}
	if proof.NativeCommits != 9 || proof.ForcedFallbacks != 6 || len(proof.CPU_BCandidates) != 5 || proof.CPU_BObservedEdges < 5000 {
		t.Fatalf("incomplete authentic evidence: %+v", proof)
	}
	t.Logf("real RC6.3.7 Windows ROM evidence now correlates: status=%s edges=%d commits=%d fallback=%d", proof.Status, proof.CPU_BObservedEdges, proof.NativeCommits, proof.ForcedFallbacks)
}
