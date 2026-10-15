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

func rc6330WriteSyntheticSource(t *testing.T, base string, hooks []rc6322Hook, total uint64) {
	t.Helper()
	var lines []string
	lines = append(lines, "ordinal\tframe\tcyclesB\tpc\topcode\tsourceAddr\tsourceByte\tnextOrdinal\tnextPc\tnextSr\tnextFrame\tnextCyclesB")
	count := 0
	for i, h := range hooks {
		if h.PC != 0x50e {
			continue
		}
		if i+1 >= len(hooks) || hooks[i+1].PC != 0x516 {
			t.Fatal("genuine BTST and BNE must be adjacent in selected hooks")
		}
		n := hooks[i+1]
		if n.Ordinal != h.Ordinal+1 {
			t.Fatalf("not consecutive original instruction hooks: %d %d", h.Ordinal, n.Ordinal)
		}
		byteVal := uint8(0x80)
		if n.SR&4 != 0 {
			byteVal = 0
		}
		lines = append(lines, fmt.Sprintf("%d\t%d\t%d\t50e\t839\t100802\t%x\t%d\t516\t%x\t%d\t%d", h.Ordinal, h.Frame, h.Cycles, byteVal, n.Ordinal, n.SR, n.Frame, n.Cycles))
		count++
	}
	if count < 1 {
		t.Fatal("no original-ROM predecessor hooks")
	}
	if err := os.WriteFile(filepath.Join(base, "cpu-b-original-050e-btst-source.tsv"), []byte(strings.Join(lines, "\n")+"\n"), 0644); err != nil {
		t.Fatal(err)
	}
	cap := fmt.Sprintf(`{"schema":"chq-rc6330-cpu-b-050e-btst-source-v1","source":"ORIGINAL_ROM_CPU_B_BTST_NONMUTATING_SOURCE_BYTE_AND_IMMEDIATE_HOOK","hits":%d,"recorded":%d,"dropped":0,"pending":false,"cpuBTransitions":%d}`, count, count, total)
	if err := os.WriteFile(filepath.Join(base, "cpu-b-original-050e-btst-cap.json"), []byte(cap), 0644); err != nil {
		t.Fatal(err)
	}
}
func TestRC6330SourceReaderMutations(t *testing.T) {
	dir := t.TempDir()
	hooks := []rc6322Hook{{Ordinal: 12, Frame: 6333, Cycles: 100, PC: 0x50e, Opcode: 0x0839}, {Ordinal: 13, Frame: 6333, Cycles: 100, PC: 0x516, Opcode: 0x6614, SR: 0}, {Ordinal: 22, Frame: 6334, Cycles: 200, PC: 0x50e, Opcode: 0x0839}, {Ordinal: 23, Frame: 6334, Cycles: 200, PC: 0x516, Opcode: 0x6614, SR: 8}}
	rc6330WriteSyntheticSource(t, dir, hooks, 100)
	good, err := rc6330ReadSourceEvidence(dir, 100)
	if err != nil || len(good) != 2 || good[0].SourceByte != 0x80 || good[1].NextOrdinal != 23 {
		t.Fatalf("valid BTST source rejected: %+v %v", good, err)
	}
	file := filepath.Join(dir, "cpu-b-original-050e-btst-source.tsv")
	orig, err := os.ReadFile(file)
	if err != nil {
		t.Fatal(err)
	}
	for _, m := range []struct{ name, from, to string }{
		{"wrong-source-address", "\t100802\t", "\t100804\t"},
		{"wrong-opcode", "\t839\t", "\t8390\t"},
		{"wrong-next-pc", "\t516\t0\t", "\t518\t0\t"},
		{"nonconsecutive-ordinal", "\t13\t516\t", "\t14\t516\t"},
		{"backwards-cycle", "\t6333\t100\n", "\t6333\t99\n"},
		{"malformed-byte", "\t100802\t80\t", "\t100802\t1ff\t"},
	} {
		changed := strings.Replace(string(orig), m.from, m.to, 1)
		if changed == string(orig) {
			t.Fatalf("mutation %s did not apply", m.name)
		}
		if err := os.WriteFile(file, []byte(changed), 0644); err != nil {
			t.Fatal(err)
		}
		if _, err := rc6330ReadSourceEvidence(dir, 100); err == nil {
			t.Fatalf("%s wrongly accepted", m.name)
		}
	}
	os.WriteFile(file, orig, 0644)
	capPath := filepath.Join(dir, "cpu-b-original-050e-btst-cap.json")
	cap, _ := os.ReadFile(capPath)
	for _, m := range []string{`"dropped":1`, `"pending":true`, `"recorded":1`} {
		altered := string(cap)
		switch {
		case strings.Contains(m, "dropped"):
			altered = strings.Replace(altered, `"dropped":0`, m, 1)
		case strings.Contains(m, "pending"):
			altered = strings.Replace(altered, `"pending":false`, m, 1)
		default:
			altered = strings.Replace(altered, `"recorded":2`, m, 1)
		}
		os.WriteFile(capPath, []byte(altered), 0644)
		if _, err := rc6330ReadSourceEvidence(dir, 100); err == nil {
			t.Fatalf("unsafe cap %s accepted", m)
		}
	}
}

func TestRC6330AuthenticCHQ6329PrerequisiteAndSyntheticNewByte(t *testing.T) {
	path := os.Getenv("CHQ_RC6329_EVIDENCE_ZIP")
	if path == "" {
		t.Skip("set CHQ_RC6329_EVIDENCE_ZIP to genuine Windows CHQ6329.zip for prior-ROM+synthetic-new-byte replay")
	}
	zr, err := zip.OpenReader(path)
	if err != nil {
		t.Fatal(err)
	}
	defer zr.Close()
	dir := t.TempDir()
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
			t.Fatalf("unsafe evidence %s", name)
		}
		dst := filepath.Join(dir, filepath.FromSlash(name))
		if err := os.MkdirAll(filepath.Dir(dst), 0755); err != nil {
			t.Fatal(err)
		}
		in, err := f.Open()
		if err != nil {
			t.Fatal(err)
		}
		b, err := io.ReadAll(io.LimitReader(in, 20<<20+1))
		in.Close()
		if err != nil || uint64(len(b)) != f.UncompressedSize64 {
			t.Fatalf("incomplete %s", name)
		}
		if err := os.WriteFile(dst, b, 0644); err != nil {
			t.Fatal(err)
		}
		copied++
	}
	if copied < 50 {
		t.Fatalf("too few source files %d", copied)
	}
	b, err := os.ReadFile(filepath.Join(dir, "cpu-b-ghidra", "cpu-b-ghidra-sites.json"))
	if err != nil {
		t.Fatal(err)
	}
	var gh struct {
		Raw string `json:"rawRomSha256"`
	}
	if json.Unmarshal(b, &gh) != nil || len(gh.Raw) != 64 {
		t.Fatal("bad genuine ROM provenance")
	}
	if _, err := rc6328BranchFrontierAudit(dir, gh.Raw); err != nil {
		t.Fatalf("prior authentic ROM branch audit: %v", err)
	}
	if _, err := rc6330BTSTSourceAudit(dir, gh.Raw); err == nil {
		t.Fatal("missing new byte observations falsely certified as genuine")
	}
	for _, name := range rc6317Cases {
		hooks, total, err := rc6322ReadHooks(dir, name)
		if err != nil {
			t.Fatal(err)
		}
		rc6330WriteSyntheticSource(t, filepath.Join(dir, "branch-matrix", name, "live-health", "original"), hooks, total)
	}
	got, err := rc6330BTSTSourceAudit(dir, gh.Raw)
	if err != nil {
		t.Fatal(err)
	}
	count := 0
	for _, c := range got.Cases {
		if c.Observations != 7 || c.SourceBit7Set != 7 || c.SourceBit7Clear != 0 || c.FallthroughTo0518 != 0 {
			t.Fatalf("unexpected case: %+v", c)
		}
		count += c.Observations
	}
	if count != 35 || !got.GhidraBTSTSourceOperandVerified || !got.OriginalByteMatchesBNEZeroFlag || !got.ImmediateSuccessorMatchesOrderedHooks || !got.MatchesOriginalROMBranchDecisions || got.NativePromotionAllowed {
		t.Fatalf("unsafe report %+v", got)
	}
	t.Log("35 genuine RC6.3.29 BTST and BNE instruction-hook/flag/edge prerequisites; new BTST source byte is SYNTHETIC (0x80), not genuine ROM evidence")
	original := filepath.Join(dir, "branch-matrix", "natural", "live-health", "original", "cpu-b-original-050e-btst-source.tsv")
	data, err := os.ReadFile(original)
	if err != nil {
		t.Fatal(err)
	}
	corrupted := strings.Replace(string(data), "\t100802\t80\t", "\t100802\t0\t", 1)
	if corrupted == string(data) {
		t.Fatal("source-bit mutation failed")
	}
	os.WriteFile(original, []byte(corrupted), 0644)
	if _, err := rc6330BTSTSourceAudit(dir, gh.Raw); err == nil {
		t.Fatal("corrupt source bit did not fail complete audit")
	}
}
