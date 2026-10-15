package main

// RC6.3.20: the 0x000516-vs-0x00000516 bug escaped synthetic-static
// tests. Exercise equivalent numeric forms AND a fresh headless-Ghidra record
// preserved in the original failed Windows CHQ6319 evidence archive.
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

func rc6320ChangeBTSTFallthrough(t *testing.T, dir string, ft *string) {
	t.Helper()
	p := filepath.Join(dir, "cpu-b-ghidra", "cpu-b-ghidra-sites.json")
	raw, err := os.ReadFile(p)
	if err != nil {
		t.Fatal(err)
	}
	var static rc639Static
	if err = json.Unmarshal(raw, &static); err != nil {
		t.Fatal(err)
	}
	found := false
	for i := range static.CallReturnSites {
		if static.CallReturnSites[i].PC == "0x00050E" || static.CallReturnSites[i].PC == "0x0000050E" {
			static.CallReturnSites[i].FallThrough = ft
			found = true
		}
	}
	if !found {
		t.Fatal("BTST site absent")
	}
	data, err := json.Marshal(static)
	if err != nil {
		t.Fatal(err)
	}
	if err = os.WriteFile(p, data, 0644); err != nil {
		t.Fatal(err)
	}
}

func TestRC6320BTSTNumericFallthroughVariants(t *testing.T) {
	dir := rc6318Synth(t)
	rc6319Seed(t, dir)
	hash := strings.Repeat("a", 64)
	for _, format := range []string{"0x000516", "0x00000516", "0x516", "0X000516"} {
		t.Run(format, func(t *testing.T) {
			rc6320ChangeBTSTFallthrough(t, dir, &format)
			r, err := rc6319CallReturnAudit(dir, hash)
			if err != nil {
				t.Fatalf("equivalent PC format rejected: %v", err)
			}
			if len(r.Edges) != 7 || r.NativePromotionAllowed || r.CompleteFunctionVerified {
				t.Fatal("unexpected promotion / edges")
			}
		})
	}
	for _, tc := range []struct {
		name string
		ft   *string
	}{
		{"nil", nil},
		{"wrong-PC", ptrRC6320("0x00051A")},
		{"malformed", ptrRC6320("0xNOPE")},
		{"overflow", ptrRC6320("0x100000516")},
	} {
		t.Run(tc.name, func(t *testing.T) {
			rc6320ChangeBTSTFallthrough(t, dir, tc.ft)
			if _, err := rc6319CallReturnAudit(dir, hash); err == nil {
				t.Fatal("unsafe/malformed fallthrough accepted")
			}
		})
	}
}
func ptrRC6320(s string) *string { return &s }

func TestRC6320AuthenticWindowsGhidraAndROMProof(t *testing.T) {
	source := os.Getenv("CHQ_RC6319_EVIDENCE_ZIP")
	if source == "" {
		t.Skip("set CHQ_RC6319_EVIDENCE_ZIP to validate original Windows Ghidra and ROM edge evidence")
	}
	z, err := zip.OpenReader(source)
	if err != nil {
		t.Fatal(err)
	}
	defer z.Close()
	dest := t.TempDir()
	copied := 0
	for _, f := range z.File {
		name := f.Name
		if name != "cpu-b-ghidra/cpu-b-ghidra-sites.json" && !strings.HasSuffix(name, "/live-health/original/cpu-b-observed-edges.tsv") {
			continue
		}
		// The file names are a fixed, bounded whitelist of audit inputs.
		if strings.Contains(name, "..") || strings.HasPrefix(name, "/") {
			t.Fatalf("unsafe archive path %q", name)
		}
		rdr, err := f.Open()
		if err != nil {
			t.Fatal(err)
		}
		b, err := io.ReadAll(io.LimitReader(rdr, 8<<20))
		rdr.Close()
		if err != nil {
			t.Fatal(err)
		}
		p := filepath.Join(dest, filepath.FromSlash(name))
		if err = os.MkdirAll(filepath.Dir(p), 0755); err != nil {
			t.Fatal(err)
		}
		if err = os.WriteFile(p, b, 0644); err != nil {
			t.Fatal(err)
		}
		copied++
	}
	if copied != 6 {
		t.Fatalf("expected exactly 5 real ROM edge tables + 1 real Ghidra export; got %d", copied)
	}
	const originalROMHash = "0e10fe93b9547d089cb0309e1ee5694b0bbc7ce7a52e28990498d0d1f0bd9ae8"
	// This MUST use the unmodified genuine Windows static evidence first;
	// no synthetic Ghidra test double is introduced or substituted.
	var static rc639Static
	raw, err := os.ReadFile(filepath.Join(dest, "cpu-b-ghidra", "cpu-b-ghidra-sites.json"))
	if err != nil {
		t.Fatal(err)
	}
	if err = json.Unmarshal(raw, &static); err != nil {
		t.Fatal(err)
	}
	if static.RawROM != originalROMHash || len(static.CallReturnSites) != 3 {
		t.Fatalf("unexpected Windows Ghidra provenance: %q", static.RawROM)
	}
	if static.CallReturnSites[1].FallThrough == nil || *static.CallReturnSites[1].FallThrough != "0x000516" {
		t.Fatalf("failed-Windows genuine BTST address format changed: %+v", static.CallReturnSites[1])
	}
	result, err := rc6319CallReturnAudit(dest, originalROMHash)
	if err != nil {
		t.Fatalf("genuine Windows original-ROM/Ghidra replay failed: %v", err)
	}
	if len(result.Edges) != 7 || result.NativePromotionAllowed || result.CompleteFunctionVerified || result.UnobservedFallthrough.OriginalRuntimeObserved {
		t.Fatal("incorrect safe-stage result")
	}
	for _, c := range rc6317Cases {
		for i, expected := range []int64{7, 7, 7, 4, 3, 4, 7} {
			if got := result.Edges[i].HitsByCase[c]; got != expected {
				t.Fatalf("case %s edge %d observed %d expected %d", c, i, got, expected)
			}
		}
	}
	t.Log(fmt.Sprintf("authentic CHQ6319 fresh Ghidra + five original-ROM traces PASS: %d edges, no function promotion", len(result.Edges)))
	alternate := "0x00000516"
	rc6320ChangeBTSTFallthrough(t, dest, &alternate)
	if _, err = rc6319CallReturnAudit(dest, originalROMHash); err != nil {
		t.Fatalf("numerically equivalent padded real-Ghidra site rejected: %v", err)
	}
	wrong := "0x00000518"
	rc6320ChangeBTSTFallthrough(t, dest, &wrong)
	if _, err = rc6319CallReturnAudit(dest, originalROMHash); err == nil {
		t.Fatal("different real-Ghidra fallthrough incorrectly accepted")
	}
}
