package main

import (
	"archive/zip"
	"encoding/json"
	"io"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestRC6324SelectedIRQRTEMutations(t *testing.T) {
	dir := rc6322Fixture(t)
	hash := strings.Repeat("a", 64)
	report, err := rc6324IRQHookAudit(dir, hash)
	if err != nil {
		t.Fatal(err)
	}
	if !report.SelectedHandlerRTEOrderVerified || !report.SampledHandlerRTEStackPointersMatched || report.ExceptionFrameBusVerified || report.IRQVectorOwnershipVerified || report.IRQDeliveryAndSchedulerTimingParityVerified || report.NativePromotionAllowed || report.CompleteFunctionVerified || len(report.Cases) != 5 {
		t.Fatalf("unsafe maturity %+v", report)
	}
	for _, c := range report.Cases {
		if c.PairedHandlerRTE != 1 || c.PairsDuringNestedCall != 1 || c.Pairs[0].ObservedTransitionGap != 7 {
			t.Fatalf("unexpected synthetic case %+v", c)
		}
	}
	path := filepath.Join(dir, "branch-matrix", "natural", "live-health", "original", "cpu-b-ordered-hook-events.tsv")
	original, err := os.ReadFile(path)
	if err != nil {
		t.Fatal(err)
	}
	for _, tc := range []struct{ name, old, new string }{
		{"bad-handler-opcode", "474\t48e7\tfff0", "474\t4e75\tfff0"},
		{"wrong-rte-sp", "4d6\t4e73\tfff0", "4d6\t4e73\tffec"},
		{"orphan-rte", "474\t48e7\tfff0", "478\t48e7\tfff0"},
		{"wrong-rte-opcode", "4d6\t4e73\tfff0", "4d6\t4e71\tfff0"},
		{"entry-not-supervisor", "474\t48e7\tfff0\t2700", "474\t48e7\tfff0\t700"},
	} {
		patched := strings.Replace(string(original), tc.old, tc.new, 1)
		if patched == string(original) {
			t.Fatalf("bad mutation %s", tc.name)
		}
		if err := os.WriteFile(path, []byte(patched), 0644); err != nil {
			t.Fatal(err)
		}
		if _, err := rc6324IRQHookAudit(dir, hash); err == nil {
			t.Errorf("accepted %s", tc.name)
		}
	}
	if err := os.WriteFile(path, original, 0644); err != nil {
		t.Fatal(err)
	}
	if _, err := rc6324IRQHookAudit(dir, hash); err != nil {
		t.Fatalf("original fixture not restored: %v", err)
	}
}

func TestRC6324RealWindowsCHQ6323IRQRTESample(t *testing.T) {
	archive := os.Getenv("CHQ_RC6323_EVIDENCE_ZIP")
	if archive == "" {
		t.Skip("set CHQ_RC6323_EVIDENCE_ZIP to test full genuine RC6.3.23 Windows IRQ/RTE evidence; no ROM data distributed")
	}
	z, err := zip.OpenReader(archive)
	if err != nil {
		t.Fatal(err)
	}
	defer z.Close()
	dir := t.TempDir()
	used := 0
	for _, f := range z.File {
		name := filepath.ToSlash(f.Name)
		if f.FileInfo().IsDir() {
			continue
		}
		if strings.HasPrefix(name, "/") || strings.Contains(name, "..") || f.UncompressedSize64 > 20<<20 {
			t.Fatalf("bad evidence path %q", name)
		}
		if !(strings.HasPrefix(name, "branch-matrix/") && strings.Contains(name, "/original/") || strings.Contains(name, "ghidra") || strings.Contains(name, "cpu-b-nested") || strings.Contains(name, "cpu-b-original")) {
			continue
		}
		outPath := filepath.Join(dir, filepath.FromSlash(name))
		if err := os.MkdirAll(filepath.Dir(outPath), 0755); err != nil {
			t.Fatal(err)
		}
		src, err := f.Open()
		if err != nil {
			t.Fatal(err)
		}
		out, err := os.Create(outPath)
		if err != nil {
			src.Close()
			t.Fatal(err)
		}
		_, e := io.Copy(out, io.LimitReader(src, 20<<20))
		src.Close()
		closeErr := out.Close()
		if e != nil || closeErr != nil {
			t.Fatalf("extract %s %v %v", name, e, closeErr)
		}
		used++
	}
	if used < 30 {
		t.Fatalf("too little evidence: %d files", used)
	}
	raw, err := os.ReadFile(filepath.Join(dir, "cpu-b-ghidra", "cpu-b-ghidra-sites.json"))
	if err != nil {
		t.Fatal(err)
	}
	var prov struct {
		RawROM string `json:"rawRomSha256"`
	}
	if err := json.Unmarshal(raw, &prov); err != nil || len(prov.RawROM) != 64 {
		t.Fatalf("missing ROM provenance %v", err)
	}
	report, err := rc6324IRQHookAudit(dir, prov.RawROM)
	if err != nil {
		t.Fatalf("authentic IRQ/RTE audit: %v", err)
	}
	if !report.SelectedHandlerRTEOrderVerified || !report.SampledHandlerRTEStackPointersMatched || report.ExceptionFrameBusVerified || report.NativePromotionAllowed || len(report.Cases) != 5 {
		t.Fatalf("unsafe report %+v", report)
	}
	for _, c := range report.Cases {
		if c.PairedHandlerRTE != 7 || c.SourceHooks != 100 || c.PairsDuringNestedCall != 0 || c.Pairs[0].EntryOrdinal != 0 {
			t.Fatalf("unexpected genuine case %+v", c)
		}
		for _, p := range c.Pairs {
			if p.ObservedTransitionGap != 14 || p.SampledSP != 0x1003fa {
				t.Fatalf("unexpected genuine handler/RTE pair %+v", p)
			}
		}
	}
	t.Log("PASS: 35 authentic original-ROM sampled handler/RTE pairs; full exception-frame bus, IRQ vector and scheduler parity remain unverified")
}
