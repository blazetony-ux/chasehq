package main

// RC6.3.23: regression for the RC6.3.22 authentic Windows ordinal-zero
// failure. Keep synthetic fixtures distinct from ROM-backed integration.
import (
	"archive/zip"
	"encoding/json"
	"io"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestRC6323ZeroBasedOrdinalIsValidButRepeatedZeroIsNot(t *testing.T) {
	dir := rc6322Fixture(t)
	path := filepath.Join(dir, "branch-matrix", "natural", "live-health", "original", "cpu-b-ordered-hook-events.tsv")
	raw, err := os.ReadFile(path)
	if err != nil {
		t.Fatal(err)
	}
	// The RC6.3.22 synthetic tests started at ordinal 7. The real Windows
	// trace starts at ordinal 0 (IRQ-vector PC 0x0474) and is otherwise
	// strictly increasing. Exercise the exact initial-ordinal invariant.
	s := string(raw)
	lines := strings.Split(s, "\n")
	if len(lines) < 4 || !strings.HasPrefix(lines[1], "7\t") || !strings.HasPrefix(lines[2], "14\t") {
		t.Fatalf("fixture layout unexpectedly changed: %q", strings.Join(lines[:3], " | "))
	}
	lines[1] = "0" + lines[1][1:]
	patched := strings.Join(lines, "\n")
	if err := os.WriteFile(path, []byte(patched), 0644); err != nil {
		t.Fatal(err)
	}
	hooks, _, err := rc6322ReadHooks(dir, "natural")
	if err != nil {
		t.Fatalf("zero-based first ordinal rejected: %v", err)
	}
	if len(hooks) == 0 || hooks[0].Ordinal != 0 {
		t.Fatalf("first event should be ordinal zero: %+v", hooks)
	}
	if _, err := rc6322OrderedAudit(dir, strings.Repeat("a", 64)); err != nil {
		t.Fatalf("valid zero-based ordinal rejected by full audit: %v", err)
	}
	// A repeated ordinal zero later is invalid and must fail closed.
	lines[2] = "0" + lines[2][2:]
	if err := os.WriteFile(path, []byte(strings.Join(lines, "\n")), 0644); err != nil {
		t.Fatal(err)
	}
	if _, _, err := rc6322ReadHooks(dir, "natural"); err == nil {
		t.Fatal("accepted duplicated ordinal zero")
	}
	// The original strictly positive fixture remains valid.
	if err := os.WriteFile(path, raw, 0644); err != nil {
		t.Fatal(err)
	}
	if _, err := rc6322OrderedAudit(dir, strings.Repeat("a", 64)); err != nil {
		t.Fatal(err)
	}
}

func TestRC6323RealWindowsCHQ6322OrderedStackIntegration(t *testing.T) {
	source := os.Getenv("CHQ_RC6322_EVIDENCE_ZIP")
	if source == "" {
		t.Skip("set CHQ_RC6322_EVIDENCE_ZIP to replay authentic RC6.3.22 Windows hook/Ghidra/ROM evidence")
	}
	z, err := zip.OpenReader(source)
	if err != nil {
		t.Fatal(err)
	}
	defer z.Close()
	dir := t.TempDir()
	used := 0
	for _, f := range z.File {
		// Use only original-case evidence and upstream static-analysis records.
		// The ZIP is an authentic user-supplied artifact, never source for code.
		name := filepath.ToSlash(f.Name)
		if f.FileInfo().IsDir() || strings.HasPrefix(name, "/") || strings.Contains(name, "..") {
			t.Fatalf("invalid ZIP path %q", name)
		}
		if f.UncompressedSize64 > 20<<20 {
			t.Fatalf("evidence entry unreasonably large: %q", name)
		}
		if strings.HasPrefix(name, "branch-matrix/") && strings.Contains(name, "/original/") ||
			strings.Contains(name, "ghidra") || strings.Contains(name, "cpu-b-nested") || strings.Contains(name, "cpu-b-original") {
			target := filepath.Join(dir, filepath.FromSlash(name))
			if err := os.MkdirAll(filepath.Dir(target), 0755); err != nil {
				t.Fatal(err)
			}
			in, err := f.Open()
			if err != nil {
				t.Fatal(err)
			}
			out, err := os.Create(target)
			if err != nil {
				in.Close()
				t.Fatal(err)
			}
			_, err = io.Copy(out, io.LimitReader(in, 20<<20))
			in.Close()
			cerr := out.Close()
			if err != nil || cerr != nil {
				t.Fatalf("extract %q: %v %v", name, err, cerr)
			}
			used++
		}
	}
	if used < 30 {
		t.Fatalf("expected authentic Ghidra and original-ROM evidence, got %d files", used)
	}
	for _, name := range rc6317Cases {
		hooks, _, err := rc6322ReadHooks(dir, name)
		if err != nil {
			t.Fatalf("%s: %v", name, err)
		}
		if len(hooks) != 100 || hooks[0].Ordinal != 0 {
			t.Fatalf("%s: expected genuine first event ordinal=0 and 100 observations, got %+v", name, hooks)
		}
	}
	staticRaw, err := os.ReadFile(filepath.Join(dir, "cpu-b-ghidra", "cpu-b-ghidra-sites.json"))
	if err != nil {
		t.Fatal(err)
	}
	var staticProvenance struct {
		RawROM string `json:"rawRomSha256"`
	}
	if err := json.Unmarshal(staticRaw, &staticProvenance); err != nil || len(staticProvenance.RawROM) != 64 {
		t.Fatalf("genuine Ghidra ROM provenance unavailable: %v", err)
	}
	report, err := rc6322OrderedAudit(dir, staticProvenance.RawROM)
	if err != nil {
		t.Fatalf("real Windows original-ROM/Ghidra replay failed: %v", err)
	}
	if len(report.Cases) != 5 || !report.SelectedNestedTemporalOrderVerified || !report.SelectedStackPointerPairsVerified || report.NativePromotionAllowed || report.CompleteFunctionVerified || report.InterruptRTEStackParityVerified {
		t.Fatalf("incorrect maturity for authentic evidence: %+v", report)
	}
	for _, c := range report.Cases {
		if c.NestedSequences != 4 || c.OrderedPairs != 16 || !c.SampledStackPointersMatched {
			t.Fatalf("invalid genuine checkpoint: %+v", c)
		}
	}
	t.Log("PASS: genuine CHQ6322 Windows CPU-B hook order + Ghidra/ROM edges; no complete function or IRQ stack parity claimed")
}
