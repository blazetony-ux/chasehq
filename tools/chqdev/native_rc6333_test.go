package main

import (
	"archive/zip"
	"bytes"
	"encoding/json"
	"io"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestRC6333ProcessCommandsAreSemanticallyBound(t *testing.T) {
	exe := filepath.Join(t.TempDir(), "runtime.exe")
	c := rc6333Command(exe, "C:/roms", "C:/generated/native.dll", "original")
	n := rc6333Command(exe, "C:/roms", "C:/generated/native.dll", "generated-native")
	f := rc6333Command(exe, "C:/roms", "C:/generated/native.dll", "forced-rollback")
	for _, cmd := range []rc6332Command{c, n, f} {
		if !strings.Contains(strings.Join(cmd.Args, " "), "--load-checkpoint {checkpoint} --frames {frame}") || cmd.Trace != "{run_dir}/native-cpu-b-pair-state.tsv" {
			t.Fatalf("missing authentic checkpoint/frame: %+v", cmd)
		}
	}
	if strings.Contains(strings.Join(c.Args, " "), "--native-cpu-b-pair-library") {
		t.Fatal("original ROM was secretly substituted")
	}
	if !strings.Contains(strings.Join(n.Args, " "), "--native-cpu-b-pair-replace") || !strings.Contains(strings.Join(f.Args, " "), "--native-cpu-b-pair-fault-probe") {
		t.Fatal("incorrect candidate or fallback mode")
	}
}

func TestRC6333CompareAndControlledNegative(t *testing.T) {
	root := t.TempDir()
	dirs := []string{filepath.Join(root, "o"), filepath.Join(root, "n"), filepath.Join(root, "f")}
	for _, d := range dirs {
		if e := os.MkdirAll(d, 0755); e != nil {
			t.Fatal(e)
		}
		for _, name := range rc6333Signals {
			if e := os.WriteFile(filepath.Join(d, name), []byte("6333\t4D8\n6334\t4E0\n"), 0644); e != nil {
				t.Fatal(e)
			}
		}
	}
	signals, mismatch, e := rc6333Compare(dirs[0], dirs[1], dirs[2])
	if e != nil || mismatch != "" || len(signals) != 5 {
		t.Fatalf("equal streams incorrectly rejected %+v %q %v", signals, mismatch, e)
	}
	ok, e := rc6333MutationGuard(root, filepath.Join(dirs[0], rc6333Signals[0]))
	if e != nil || !ok {
		t.Fatalf("mutation failed detection: %v", e)
	}
	if e := os.WriteFile(filepath.Join(dirs[1], rc6333Signals[2]), []byte("bad\n"), 0644); e != nil {
		t.Fatal(e)
	}
	_, mismatch, e = rc6333Compare(dirs[0], dirs[1], dirs[2])
	if e != nil || mismatch != rc6333Signals[2] {
		t.Fatalf("first different stream not identified %q %v", mismatch, e)
	}
	if e := os.Remove(filepath.Join(dirs[2], rc6333Signals[3])); e != nil {
		t.Fatal(e)
	}
	if _, _, e = rc6333Compare(dirs[0], dirs[1], dirs[2]); e == nil {
		t.Fatal("missing machine snapshot was silently accepted")
	}
}

// The supplied CHQ6332 archive contains real Windows original-ROM, actually
// generated CPU-B DLL and forced fallback traces. Replay and mutation tests
// must not pretend this offline fixture is a fresh executable run.
func TestRC6333WindowsRealPairEvidencePrerequisite(t *testing.T) {
	zipPath := os.Getenv("CHQ_RC6332_EVIDENCE_ZIP")
	if zipPath == "" {
		t.Skip("set CHQ_RC6332_EVIDENCE_ZIP to test genuine RC6.3.32 Windows CPU-B pair evidence")
	}
	z, e := zip.OpenReader(zipPath)
	if e != nil {
		t.Fatal(e)
	}
	defer z.Close()
	root := t.TempDir()
	for _, mode := range []string{"original", "generated-native", "forced-mismatch"} {
		d := filepath.Join(root, mode)
		if e = os.MkdirAll(d, 0755); e != nil {
			t.Fatal(e)
		}
		for _, name := range append(append([]string{}, rc6333Signals...), "native-cpu-b-pair-report.json") {
			key := "live-cpu-b-pair/" + mode + "/" + name
			found := false
			for _, f := range z.File {
				if f.Name != key {
					continue
				}
				found = true
				r, er := f.Open()
				if er != nil {
					t.Fatal(er)
				}
				b, er := io.ReadAll(io.LimitReader(r, 32<<20))
				r.Close()
				if er != nil {
					t.Fatal(er)
				}
				if er = os.WriteFile(filepath.Join(d, name), b, 0644); er != nil {
					t.Fatal(er)
				}
				break
			}
			if !found {
				t.Fatalf("missing authentic predecessor trace %s", key)
			}
		}
	}
	o, n, f := filepath.Join(root, "original"), filepath.Join(root, "generated-native"), filepath.Join(root, "forced-mismatch")
	signals, mismatch, e := rc6333Compare(o, n, f)
	if e != nil || mismatch != "" || len(signals) != 5 {
		t.Fatalf("previous genuine CPU-B pair failed semantic parity %q %v %+v", mismatch, e, signals)
	}
	or, e := rc6333ReadReport(o)
	if e != nil {
		t.Fatal(e)
	}
	nr, e := rc6333ReadReport(n)
	if e != nil {
		t.Fatal(e)
	}
	fr, e := rc6333ReadReport(f)
	if e != nil {
		t.Fatal(e)
	}
	if or.Commits != 0 || nr.Commits <= 0 || !nr.GeneratedLibraryUsed || fr.Rollbacks != 1 {
		t.Fatalf("wrong authentic counters %v %v %v", or, nr, fr)
	}
	if bytes.Equal([]byte(""), []byte(strings.Join([]string{signals[0].OriginalSHA256, signals[0].NativeSHA256}, ""))) {
		t.Fatal("empty original hash")
	}
	// Copy-mutate saved observed trace; never alter any ROM or original ZIP.
	original, e := os.ReadFile(filepath.Join(n, rc6333Signals[0]))
	if e != nil {
		t.Fatal(e)
	}
	altered := append([]byte{}, original...)
	altered[0] ^= 1
	if e = os.WriteFile(filepath.Join(n, rc6333Signals[0]), altered, 0644); e != nil {
		t.Fatal(e)
	}
	_, mismatch, e = rc6333Compare(o, n, f)
	if e != nil || mismatch != rc6333Signals[0] {
		t.Fatalf("real-trace mutation not detected %q %v", mismatch, e)
	}
	v, e := rc6331CompareStreams("CPU_B_PAIR_CONTROLLED_MUTATION", filepath.Join(o, rc6333Signals[0]), filepath.Join(n, rc6333Signals[0]))
	if e != nil || v.Equal || v.FirstDifferentLine != 1 {
		t.Fatalf("unexpected mismatch witness %v %+v", e, v)
	}
	b, er := json.Marshal(v)
	if er != nil || len(b) == 0 {
		t.Fatal(er)
	}
}
