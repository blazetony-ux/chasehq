package main

import (
	"archive/zip"
	"encoding/json"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"strconv"
	"strings"
	"testing"
)

// RC6.3.29: equality is legitimate because cyclesB counts are sampled at the
// emulator batch boundary; strictly decreasing next-hook cycles are not.
// Retain strict ordinal and across-visit clock monotonicity and all RC6.3.28
// opcode/flag/PC/source-count checks.
func TestRC6329EqualClockAndBackwardsClock(t *testing.T) {
	base := t.TempDir()
	events := []rc6322Hook{
		{Ordinal: 0, Frame: 6333, Cycles: 100, PC: 0x516, Opcode: 0x6614, SR: 0},
		{Ordinal: 8, Frame: 6334, Cycles: 180, PC: 0x516, Opcode: 0x6614, SR: 0x8},
	}
	rc6328SyntheticBNE(t, base, events, 100)
	file := filepath.Join(base, "cpu-b-original-0516-bne-decisions.tsv")
	b, err := os.ReadFile(file)
	if err != nil {
		t.Fatal(err)
	}
	equal := strings.ReplaceAll(string(b), "\t6333\t108", "\t6333\t100")
	equal = strings.ReplaceAll(equal, "\t6334\t188", "\t6334\t180")
	if equal == string(b) {
		t.Fatal("equal-cycle mutation not applied")
	}
	if err := os.WriteFile(file, []byte(equal), 0644); err != nil {
		t.Fatal(err)
	}
	if rows, err := rc6328ReadDecisionEvidence(base, 100); err != nil || len(rows) != 2 || rows[0].NextCycles != rows[0].Cycles || rows[1].NextCycles != rows[1].Cycles {
		t.Fatalf("valid equal-clock instruction snapshots rejected: rows=%+v error=%v", rows, err)
	}
	backwards := strings.Replace(equal, "\t6333\t100\n", "\t6333\t99\n", 1)
	if backwards == equal {
		t.Fatal("backwards-cycle mutation absent")
	}
	if err := os.WriteFile(file, []byte(backwards), 0644); err != nil {
		t.Fatal(err)
	}
	if _, err := rc6328ReadDecisionEvidence(base, 100); err == nil {
		t.Fatal("backwards next-hook cycle incorrectly accepted")
	}
	// Still reject reverse clock progress across two separate branch visits.
	regression := strings.Replace(equal, "8\t6334\t180", "8\t6334\t99", 1)
	if err := os.WriteFile(file, []byte(regression), 0644); err != nil {
		t.Fatal(err)
	}
	if _, err := rc6328ReadDecisionEvidence(base, 100); err == nil {
		t.Fatal("backwards between branch visits accepted")
	}
}

// Optional original Windows RC6.3.28 integration. Unlike the older RC6.3.27
// integration, no new successor events are synthesised. The entire Ghidra ->
// original-ROM hooks -> observed next-PC -> edge audit uses uploaded bytes.
func TestRC6329AuthenticWindowsCHQ6328Integration(t *testing.T) {
	source := os.Getenv("CHQ_RC6328_EVIDENCE_ZIP")
	if source == "" {
		t.Skip("set CHQ_RC6328_EVIDENCE_ZIP to the immutable Windows CHQ6328.zip")
	}
	zr, err := zip.OpenReader(source)
	if err != nil {
		t.Fatal(err)
	}
	defer zr.Close()
	dest := t.TempDir()
	copied := 0
	for _, f := range zr.File {
		name := filepath.ToSlash(f.Name)
		if f.FileInfo().IsDir() {
			continue
		}
		if !(strings.HasPrefix(name, "branch-matrix/") && strings.Contains(name, "/original/") || strings.Contains(name, "ghidra") || strings.Contains(name, "cpu-b-nested") || strings.Contains(name, "cpu-b-original")) {
			continue
		}
		if strings.Contains(name, "..") || strings.HasPrefix(name, "/") || f.UncompressedSize64 > 20<<20 {
			t.Fatalf("unsafe evidence %q", name)
		}
		path := filepath.Join(dest, filepath.FromSlash(name))
		if err := os.MkdirAll(filepath.Dir(path), 0755); err != nil {
			t.Fatal(err)
		}
		r, err := f.Open()
		if err != nil {
			t.Fatal(err)
		}
		b, err := io.ReadAll(io.LimitReader(r, 20<<20+1))
		r.Close()
		if err != nil {
			t.Fatal(err)
		}
		if uint64(len(b)) != f.UncompressedSize64 {
			t.Fatalf("incomplete evidence %s", name)
		}
		if err := os.WriteFile(path, b, 0644); err != nil {
			t.Fatal(err)
		}
		copied++
	}
	if copied < 50 {
		t.Fatalf("insufficient original ROM evidence %d", copied)
	}
	b, err := os.ReadFile(filepath.Join(dest, "cpu-b-ghidra", "cpu-b-ghidra-sites.json"))
	if err != nil {
		t.Fatal(err)
	}
	var ghidra struct {
		Raw string `json:"rawRomSha256"`
	}
	if err := json.Unmarshal(b, &ghidra); err != nil || len(ghidra.Raw) != 64 {
		t.Fatalf("untrusted ROM SHA %s: %v", ghidra.Raw, err)
	}
	report, err := rc6328BranchFrontierAudit(dest, ghidra.Raw)
	if err != nil {
		t.Fatalf("complete authentic Ghidra/original-ROM branch audit failed: %v", err)
	}
	total := 0
	if len(report.Cases) != 5 {
		t.Fatalf("not five authentic cases: %+v", report.Cases)
	}
	for _, c := range report.Cases {
		if c.BranchVisits != 7 || c.BNENotZeroTaken != 7 || c.BNEZeroFallthrough != 0 || c.ZFlagSetAtBranch != 0 || c.OriginalROMEdgeCount != 7 {
			t.Fatalf("unexpected authentic case %+v", c)
		}
		base := filepath.Join(dest, "branch-matrix", c.Case, "live-health", "original")
		data, err := os.ReadFile(filepath.Join(base, "cpu-b-original-0516-bne-decisions.tsv"))
		if err != nil {
			t.Fatal(err)
		}
		lines := strings.Split(strings.TrimSpace(string(data)), "\n")
		if len(lines) != 8 {
			t.Fatalf("genuine data row count %s %d", c.Case, len(lines))
		}
		for i, ln := range lines[1:] {
			cols := strings.Split(ln, "\t")
			if len(cols) != 9 {
				t.Fatalf("invalid authentic row %d: %q", i+1, ln)
			}
			cycle, _ := strconv.ParseUint(cols[2], 10, 64)
			next, _ := strconv.ParseUint(cols[8], 10, 64)
			if cycle != next {
				t.Fatalf("expected genuine equal-clock sample %s row %d: %d != %d", c.Case, i+1, cycle, next)
			}
		}
		total += c.BranchVisits
	}
	if total != 35 || !report.RecordedSRMatchesSelectedOriginalHooks || !report.ObservedNextPCMatchesBranchCondition || !report.OrderedVisitsMatchOriginalROMEdges || report.Observed0518Fallthrough || report.EntirePathCoverageVerified || report.EnclosingFunctionCFGVerified || report.NativePromotionAllowed {
		t.Fatalf("untrusted maturity result %+v", report)
	}
	t.Logf("PASS real CHQ6328.zip: %d authentic BNE visits, 35 equal-clock immediate successors, 0x0518 unobserved, whole function blocked", total)
	// Mutation against real captured cycles, not synthetic seeds.
	base := filepath.Join(dest, "branch-matrix", "natural", "live-health", "original")
	file := filepath.Join(base, "cpu-b-original-0516-bne-decisions.tsv")
	data, err := os.ReadFile(file)
	if err != nil {
		t.Fatal(err)
	}
	rows := strings.Split(string(data), "\n")
	cols := strings.Split(rows[1], "\t")
	if len(cols) != 9 {
		t.Fatal("bad first genuine row")
	}
	cycle, err := strconv.ParseUint(cols[2], 10, 64)
	if err != nil || cycle == 0 {
		t.Fatal("bad original cycle")
	}
	cols[8] = fmt.Sprint(cycle - 1)
	rows[1] = strings.Join(cols, "\t")
	if err := os.WriteFile(file, []byte(strings.Join(rows, "\n")), 0644); err != nil {
		t.Fatal(err)
	}
	if _, err := rc6328BranchFrontierAudit(dest, ghidra.Raw); err == nil {
		t.Fatal("complete Ghidra/ROM audit accepted corrupted earlier next-hook cycle")
	}
}
