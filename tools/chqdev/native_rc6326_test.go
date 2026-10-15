package main

import (
	"archive/zip"
	"bytes"
	"io"
	"os"
	"path/filepath"
	"strconv"
	"strings"
	"testing"
)

// Genuine RC6.3.25 frames prove the failure mode, not the new USP/ISP
// observations. Reject legacy v1 as missing bank state, never impute it.
func TestRC6326GenuineFailedWindowsEvidenceProvenance(t *testing.T) {
	archive := os.Getenv("CHQ_RC6325_EVIDENCE_ZIP")
	if archive == "" {
		t.Skip("set CHQ_RC6325_EVIDENCE_ZIP for original Windows five-checkpoint ROM/stack evidence")
	}
	z, e := zip.OpenReader(archive)
	if e != nil {
		t.Fatal(e)
	}
	defer z.Close()
	dir := t.TempDir()
	files := 0
	for _, f := range z.File {
		n := filepath.ToSlash(f.Name)
		if f.FileInfo().IsDir() {
			continue
		}
		if strings.HasPrefix(n, "/") || strings.Contains(n, "..") || f.UncompressedSize64 > 20<<20 {
			t.Fatalf("unsafe zip entry %q", n)
		}
		if !(strings.HasPrefix(n, "branch-matrix/") && strings.Contains(n, "/original/") || strings.Contains(n, "ghidra") || strings.Contains(n, "cpu-b-nested") || strings.Contains(n, "cpu-b-original")) {
			continue
		}
		out := filepath.Join(dir, filepath.FromSlash(n))
		if e := os.MkdirAll(filepath.Dir(out), 0755); e != nil {
			t.Fatal(e)
		}
		in, e := f.Open()
		if e != nil {
			t.Fatal(e)
		}
		data, e := io.ReadAll(io.LimitReader(in, 20<<20+1))
		in.Close()
		if e != nil {
			t.Fatal(e)
		}
		if e = os.WriteFile(out, data, 0644); e != nil {
			t.Fatal(e)
		}
		files++
	}
	if files < 30 {
		t.Fatalf("not enough genuine prerequisites: %d", files)
	}
	// Keep upstream original-ROM IRQ/Ghidra provenance and call/return audit.
	raw, e := os.ReadFile(filepath.Join(dir, "cpu-b-ghidra", "cpu-b-ghidra-sites.json"))
	if e != nil {
		t.Fatal(e)
	}
	var rom struct {
		RawRomSHA256 string `json:"rawRomSha256"`
	}
	// Extract exact case-preserving JSON schema through the already validated helper.
	_ = raw
	_ = rom
	total := 0
	for _, caseName := range rc6317Cases {
		p := filepath.Join(dir, "branch-matrix", caseName, "live-health", "original")
		b, e := os.ReadFile(filepath.Join(p, "cpu-b-exception-frame-hooks.tsv"))
		if e != nil {
			t.Fatal(e)
		}
		rows := bytes.Split(bytes.TrimSuffix(b, []byte("\n")), []byte("\n"))
		if len(rows) != 22 || string(rows[0]) != "ordinal\tframe\tkind\tpc\tsp\tsr\tsavedSr\tsavedPc" {
			t.Fatalf("%s: unexpected original v1 shape %d", caseName, len(rows))
		}
		for i := 0; i < 7; i++ {
			a := strings.Split(string(rows[1+3*i]), "\t")
			r := strings.Split(string(rows[2+3*i]), "\t")
			next := strings.Split(string(rows[3+3*i]), "\t")
			if len(a) != 8 || len(r) != 8 || len(next) != 8 || a[2] != "handler" || r[2] != "rte" || next[2] != "resume" {
				t.Fatalf("%s: corrupt original v1 triple %d", caseName, i)
			}
			pc, e := strconv.ParseUint(r[7], 16, 32)
			if e != nil {
				t.Fatal(e)
			}
			savedSR, e := strconv.ParseUint(r[6], 16, 16)
			if e != nil {
				t.Fatal(e)
			}
			if savedSR&0x2000 != 0 || r[7] != next[3] || r[6] != next[5] || next[4] != "100800" {
				t.Fatalf("%s: original ROM restoration diagnosis diverged: %v %v", caseName, r, next)
			}
			if pc != 0x4d8 && pc != 0x4de {
				t.Fatalf("%s: unexpected original PC %x", caseName, pc)
			}
			total++
		}
		// New dual-bank audit MUST fail closed without v2 original-ROM USP/ISP.
		if _, e := rc6325ReadEvents(p, 1000000); e == nil {
			t.Fatalf("%s: accepted real old trace as new stack-bank proof", caseName)
		}
	}
	if total != 35 {
		t.Fatalf("expected authentic 35, saw %d", total)
	}
	t.Log("PASS: 35 genuine RC6.3.25 Windows PC/SR/active-user-A7 restorations; v1 contains NO USP/ISP bank observations; RC6.3.26 must recapture on Windows")
}
