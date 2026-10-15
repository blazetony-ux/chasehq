package main

import (
	"archive/zip"
	"encoding/json"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

// The new IRQ bus rows are SYNTHETIC in this local fixture. The prior
// original-ROM PC/stack provenance is separately checked on uploaded Windows
// evidence, never mislabeled as a new original IRQ bus capture.
func rc6327SeedSyntheticIRQBus(t *testing.T, dir string) {
	t.Helper()
	for _, caseName := range rc6317Cases {
		base := filepath.Join(dir, "branch-matrix", caseName, "live-health", "original")
		_, trans, e := rc6322ReadHooks(dir, caseName)
		if e != nil {
			t.Fatal(e)
		}
		events, e := rc6325ReadEvents(base, trans)
		if e != nil {
			t.Fatal(e)
		}
		var b strings.Builder
		b.WriteString("sample\tframe\tkind\taddress\tvalue\twidth\n")
		count := 0
		for i := 0; i < len(events); i += 3 {
			a := events[i]
			if a.Kind != "handler" {
				t.Fatal("invalid synthetic original fixture")
			}
			sample := i/3 + 1
			fmt.Fprintf(&b, "%d\t%d\tA\t%x\t4\t0\n", sample, a.Frame, a.SavedPC)
			fmt.Fprintf(&b, "%d\t%d\tR\t70\t474\t4\n", sample, a.Frame)
			fmt.Fprintf(&b, "%d\t%d\tW\t%x\t%x\t4\n", sample, a.Frame, a.SP+2, a.SavedPC)
			fmt.Fprintf(&b, "%d\t%d\tW\t%x\t%x\t2\n", sample, a.Frame, a.SP, a.SavedSR)
			fmt.Fprintf(&b, "%d\t%d\tH\t474\t0\t0\n", sample, a.Frame)
			count += 5
		}
		if e := os.WriteFile(filepath.Join(base, "cpu-b-original-irq-delivery-bus.tsv"), []byte(b.String()), 0644); e != nil {
			t.Fatal(e)
		}
		cap := map[string]any{"schema": "chq-rc6327-cpu-b-original-irq-delivery-cap-v1", "source": "ORIGINAL_ROM_MUSASHI_ACK_AND_CPU_B_BUS_TRANSACTIONS", "samples": count / 5, "recorded": count, "dropped": 0, "pending": false, "cpuBTransitions": trans}
		raw, _ := json.Marshal(cap)
		if e := os.WriteFile(filepath.Join(base, "cpu-b-original-irq-delivery-cap.json"), raw, 0644); e != nil {
			t.Fatal(e)
		}
	}
}
func TestRC6327SyntheticBusMutationRejects(t *testing.T) {
	dir := rc6322Fixture(t)
	rc6325SeedSyntheticFrames(t, dir)
	rc6327SeedSyntheticIRQBus(t, dir)
	hash := strings.Repeat("a", 64)
	r, e := rc6327IRQDeliveryAudit(dir, hash)
	if e != nil {
		t.Fatal(e)
	}
	if len(r.Cases) != 5 || !r.SampledOriginalACKObserved || !r.SelectedVectorTableReadMatched || !r.OrderedExceptionStackWritesMatched || !r.FirstHandlerHookMatched || r.NativePromotionAllowed || r.HardwareVectorOwnershipVerified || r.CompleteIRQDeliveryVerified || r.ExactCycleTimingVerified {
		t.Fatalf("unsafe report %+v", r)
	}
	base := filepath.Join(dir, "branch-matrix", "natural", "live-health", "original")
	file := filepath.Join(base, "cpu-b-original-irq-delivery-bus.tsv")
	original, e := os.ReadFile(file)
	if e != nil {
		t.Fatal(e)
	}
	for _, m := range []struct{ name, old, mut string }{
		{"ack-level", "\tA\t4d8\t4\t0", "\tA\t4d8\t5\t0"},
		{"wrong-vector", "\tR\t70\t474\t4", "\tR\t70\t476\t4"},
		{"wrong-vector-address", "\tR\t70\t474\t4", "\tR\t74\t474\t4"},
		{"pc-push-address", "\tW\tfff2\t4d8\t4", "\tW\tfff0\t4d8\t4"},
		{"pc-push-value", "\tW\tfff2\t4d8\t4", "\tW\tfff2\t4de\t4"},
		{"sr-push-value", "\tW\tfff0\t0\t2", "\tW\tfff0\t4\t2"},
		{"wrong-transaction-order", "\tR\t70\t474\t4", "\tW\t70\t474\t4"},
		{"unexpected-handler", "\tH\t474\t0\t0", "\tH\t476\t0\t0"},
		{"wrong-sample", "1\t", "2\t"},
	} {
		mutated := strings.Replace(string(original), m.old, m.mut, 1)
		if mutated == string(original) {
			t.Fatalf("mutation not applied %s", m.name)
		}
		if e := os.WriteFile(file, []byte(mutated), 0644); e != nil {
			t.Fatal(e)
		}
		if _, e := rc6327IRQDeliveryAudit(dir, hash); e == nil {
			t.Errorf("accepted %s", m.name)
		}
	}
	if e := os.WriteFile(file, original, 0644); e != nil {
		t.Fatal(e)
	}
	if _, e := rc6327IRQDeliveryAudit(dir, hash); e != nil {
		t.Fatal(e)
	}
	capFile := filepath.Join(base, "cpu-b-original-irq-delivery-cap.json")
	cap, e := os.ReadFile(capFile)
	if e != nil {
		t.Fatal(e)
	}
	for _, m := range []struct{ old, mut string }{{`"dropped":0`, `"dropped":1`}, {`"pending":false`, `"pending":true`}, {`"recorded":5`, `"recorded":6`}} {
		mutated := strings.Replace(string(cap), m.old, m.mut, 1)
		if mutated == string(cap) {
			t.Fatalf("cap mutation absent %q", m.old)
		}
		if e := os.WriteFile(capFile, []byte(mutated), 0644); e != nil {
			t.Fatal(e)
		}
		if _, e := rc6327IRQDeliveryAudit(dir, hash); e == nil {
			t.Errorf("accepted corrupt cap %s", m.old)
		}
	}
	os.WriteFile(capFile, cap, 0644)
	os.Remove(file)
	if _, e := rc6327IRQDeliveryAudit(dir, hash); e == nil {
		t.Fatal("accepted missing transaction stream")
	}
}

// RC6.3.26 authentic Windows sources establish the 35 frame/USP/ISP
// prerequisites only. A differently scoped older CPU-B native-differential
// IRQ stream is genuine but cannot be relabelled as the NEW ACK-scoped source.
func TestRC6327GenuineRC6326EvidenceDoesNotForgeBus(t *testing.T) {
	archive := os.Getenv("CHQ_RC6326_EVIDENCE_ZIP")
	if archive == "" {
		t.Skip("set CHQ_RC6326_EVIDENCE_ZIP for real RC6.3.26 ROM provenance")
	}
	z, e := zip.OpenReader(archive)
	if e != nil {
		t.Fatal(e)
	}
	defer z.Close()
	dir := t.TempDir()
	used := 0
	for _, f := range z.File {
		name := filepath.ToSlash(f.Name)
		if f.FileInfo().IsDir() || !(strings.HasPrefix(name, "branch-matrix/") && strings.Contains(name, "/original/") || strings.Contains(name, "ghidra") || strings.Contains(name, "cpu-b-nested") || strings.Contains(name, "cpu-b-original")) {
			continue
		}
		if strings.Contains(name, "..") || strings.HasPrefix(name, "/") || f.UncompressedSize64 > 20<<20 {
			t.Fatalf("unsafe evidence path %q", name)
		}
		dst := filepath.Join(dir, filepath.FromSlash(name))
		if e := os.MkdirAll(filepath.Dir(dst), 0755); e != nil {
			t.Fatal(e)
		}
		in, e := f.Open()
		if e != nil {
			t.Fatal(e)
		}
		b, e := io.ReadAll(io.LimitReader(in, 20<<20+1))
		in.Close()
		if e != nil {
			t.Fatal(e)
		}
		if e := os.WriteFile(dst, b, 0644); e != nil {
			t.Fatal(e)
		}
		used++
	}
	if used < 30 {
		t.Fatalf("insufficient original evidence: %d", used)
	}
	raw, e := os.ReadFile(filepath.Join(dir, "cpu-b-ghidra", "cpu-b-ghidra-sites.json"))
	if e != nil {
		t.Fatal(e)
	}
	var rom struct {
		Raw string `json:"rawRomSha256"`
	}
	if json.Unmarshal(raw, &rom) != nil || len(rom.Raw) != 64 {
		t.Fatal("ROM provenance missing")
	}
	prior, e := rc6325ExceptionFrameAudit(dir, rom.Raw)
	if e != nil {
		t.Fatalf("real RC6326 predecessor %v", e)
	}
	samples := 0
	for _, c := range prior.Cases {
		samples += c.VerifiedSamples
	}
	if samples != 35 {
		t.Fatalf("expected genuine 35 frame samples got %d", samples)
	}
	if _, e := rc6327IRQDeliveryAudit(dir, rom.Raw); e == nil {
		t.Fatal("missing new RC6327 original IRQ bus trace incorrectly accepted as authentic")
	}
	t.Log("PASS genuine RC6.3.26 Windows 35 frame/USP/ISP samples; NEW original-ROM IRQ delivery bus record requires fresh RC6.3.27 Windows run")
}
