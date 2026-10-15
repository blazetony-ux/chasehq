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

func rc6331WriteWriterFixture(t *testing.T, dir string, transition uint64, rows []string) {
	t.Helper()
	data := "ordinal\tframe\tcyclesB\tcpu\tpc\taddress\twidth\tbefore\tafter\n" + strings.Join(rows, "\n")
	if len(rows) > 0 {
		data += "\n"
	}
	if err := os.WriteFile(filepath.Join(dir, "cpu-b-original-100802-writer-events.tsv"), []byte(data), 0644); err != nil {
		t.Fatal(err)
	}
	cap := fmt.Sprintf(`{"schema":"chq-rc6331-cpu-b-100802-writer-window-v1","source":"CPU_A_AND_B_BUS_WRITE_POSTCOMMIT_NO_FORCED_PATCH","hits":%d,"recorded":%d,"dropped":0,"cpuBTransitions":%d}`, len(rows), len(rows), transition)
	if err := os.WriteFile(filepath.Join(dir, "cpu-b-original-100802-writer-cap.json"), []byte(cap), 0644); err != nil {
		t.Fatal(err)
	}
}
func TestRC6331WriterWindowStrictCases(t *testing.T) {
	d := t.TempDir()
	rc6331WriteWriterFixture(t, d, 100, nil)
	empty, err := rc6331ReadWriters(d, 100)
	if err != nil || len(empty) != 0 {
		t.Fatalf("a window without writers is valid but incomplete provenance: %v %+v", err, empty)
	}
	rows := []string{"15\t6333\t112\tB\t50c\t100802\t1\t80\t0", "15\t6333\t112\tA\t820c\t100800\t4\t0\t80"}
	rc6331WriteWriterFixture(t, d, 100, rows)
	got, err := rc6331ReadWriters(d, 100)
	if err != nil || len(got) != 2 || got[0].After != 0 || got[1].CPU != "A" {
		t.Fatalf("good dual-CPU writers rejected: %+v %v", got, err)
	}
	path := filepath.Join(d, "cpu-b-original-100802-writer-events.tsv")
	orig, _ := os.ReadFile(path)
	for _, m := range []struct{ name, from, to string }{
		{"bad-cpu", "\tB\t50c\t", "\tX\t50c\t"},
		{"wrong-address", "\t100802\t1\t", "\t100804\t1\t"},
		{"wrong-width", "\t100800\t4\t", "\t100800\t8\t"},
		{"backward-frame", "15\t6333\t112\tA", "15\t6332\t112\tA"},
		{"out-of-range-byte", "\t80\t0\n", "\t180\t0\n"},
		{"out-of-range-ordinal", "15\t6333", "150\t6333"},
	} {
		x := strings.Replace(string(orig), m.from, m.to, 1)
		if x == string(orig) {
			t.Fatalf("mutation %s inapplicable", m.name)
		}
		os.WriteFile(path, []byte(x), 0644)
		if _, err := rc6331ReadWriters(d, 100); err == nil {
			t.Fatalf("accepted corrupt %s", m.name)
		}
	}
	os.WriteFile(path, orig, 0644)
	capPath := filepath.Join(d, "cpu-b-original-100802-writer-cap.json")
	cap, _ := os.ReadFile(capPath)
	for _, m := range []struct{ from, to string }{{`"recorded":2`, `"recorded":1`}, {`"dropped":0`, `"dropped":1`}, {`"cpuBTransitions":100`, `"cpuBTransitions":99`}} {
		os.WriteFile(capPath, []byte(strings.Replace(string(cap), m.from, m.to, 1)), 0644)
		if _, err := rc6331ReadWriters(d, 100); err == nil {
			t.Fatalf("accepted invalid cap %s", m.to)
		}
	}
}
func TestRC6331DeterministicFirstDivergenceWitness(t *testing.T) {
	d := t.TempDir()
	a := filepath.Join(d, "original.tsv")
	b := filepath.Join(d, "native.tsv")
	os.WriteFile(a, []byte("h\nx\ny\nz\n"), 0644)
	os.WriteFile(b, []byte("h\nx\nY\nz\n"), 0644)
	got, err := rc6331CompareStreams("B:4D8", a, b)
	if err != nil || got.FirstDifferentLine != 3 || got.OriginalLine != "y" || got.NativeLine != "Y" || got.Equal || !got.WindowMinimized || got.InputScenarioMinimized || got.NativePromotionAllowed || len(got.ContextBefore) != 2 {
		t.Fatalf("bad witness: %+v %v", got, err)
	}
	os.WriteFile(b, []byte("h\nx\ny\nz\n"), 0644)
	eq, err := rc6331CompareStreams("B:4D8", a, b)
	if err != nil || !eq.Equal || eq.FirstDifferentLine != 0 {
		t.Fatalf("equal stream rejected: %+v %v", eq, err)
	}
	os.WriteFile(b, []byte("h\nx\n"), 0644)
	end, err := rc6331CompareStreams("B:4D8", a, b)
	if err != nil || end.NativeLine != "<EOF>" || end.FirstDifferentLine != 3 {
		t.Fatalf("truncated stream accepted incorrectly: %+v %v", end, err)
	}
}
func TestRC6331GenuineOldWindowsReplaySeparateNewWriterCapture(t *testing.T) {
	archive := os.Getenv("CHQ_RC6330_EVIDENCE_ZIP")
	if archive == "" {
		t.Skip("set CHQ_RC6330_EVIDENCE_ZIP for original-ROM prerequisite with synthetic ONLY new writer windows")
	}
	zr, err := zip.OpenReader(archive)
	if err != nil {
		t.Fatal(err)
	}
	defer zr.Close()
	dir := t.TempDir()
	copied := 0
	for _, f := range zr.File {
		name := filepath.ToSlash(f.Name)
		if f.FileInfo().IsDir() || !(strings.HasPrefix(name, "branch-matrix/") && strings.Contains(name, "/original/") || strings.Contains(name, "ghidra") || strings.Contains(name, "cpu-b-nested") || strings.Contains(name, "cpu-b-original")) {
			continue
		}
		if strings.Contains(name, "..") || strings.HasPrefix(name, "/") || f.UncompressedSize64 > 20<<20 {
			t.Fatalf("bad filename %s", name)
		}
		out := filepath.Join(dir, filepath.FromSlash(name))
		os.MkdirAll(filepath.Dir(out), 0755)
		r, e := f.Open()
		if e != nil {
			t.Fatal(e)
		}
		b, e := io.ReadAll(io.LimitReader(r, 20<<20+1))
		r.Close()
		if e != nil || uint64(len(b)) != f.UncompressedSize64 {
			t.Fatalf("incomplete %s", name)
		}
		os.WriteFile(out, b, 0644)
		copied++
	}
	if copied < 100 {
		t.Fatalf("missing old evidence %d", copied)
	}
	g, err := os.ReadFile(filepath.Join(dir, "cpu-b-ghidra", "cpu-b-ghidra-sites.json"))
	if err != nil {
		t.Fatal(err)
	}
	var id struct {
		Raw string `json:"rawRomSha256"`
	}
	if json.Unmarshal(g, &id) != nil || len(id.Raw) != 64 {
		t.Fatal("untrusted original ROM identity")
	}
	if _, err := rc6330BTSTSourceAudit(dir, id.Raw); err != nil {
		t.Fatalf("genuine RC6.3.30 prereq: %v", err)
	}
	if _, err := rc6331FunctionAndWriterDossier(dir, id.Raw); err == nil {
		t.Fatal("older real evidence without RC6.3.31 writer captures falsely certified")
	}
	for _, name := range rc6317Cases {
		_, transition, e := rc6322ReadHooks(dir, name)
		if e != nil {
			t.Fatal(e)
		}
		// These are SYNTHETIC new empty-writer caps to exercise only the
		// dossier structure. Not evidence that Windows observed zero writers.
		rc6331WriteWriterFixture(t, filepath.Join(dir, "branch-matrix", name, "live-health", "original"), transition, nil)
	}
	report, err := rc6331FunctionAndWriterDossier(dir, id.Raw)
	if err != nil {
		t.Fatalf("synthetic writer-cap dossier: %v", err)
	}
	if len(report.Cases) != 5 || report.NativePromotionAllowed || report.VerifiedCompleteCFG || report.WriterProvenanceComplete || report.Observed0518Fallthrough || report.ObservedFunctionEdges < 1 {
		t.Fatalf("unsafe dossier %+v", report)
	}
	for _, c := range report.Cases {
		if c.WriterSourceState != "NO_WRITER_OBSERVED_IN_BOUNDED_WINDOW" || c.ObservedOriginalBTST != 7 {
			t.Fatalf("unsafe manufactured writer %+v", c)
		}
	}
}
