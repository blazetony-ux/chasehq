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

func rc6322Seed(t *testing.T, dir string) {
	t.Helper()
	for _, name := range rc6317Cases {
		base := filepath.Join(dir, "branch-matrix", name, "live-health", "original")
		var b strings.Builder
		b.WriteString("ordinal\tframe\tcyclesB\tpc\topcode\tsp\tsr\n")
		ordinal := uint64(0)
		recorded := 0
		for i := 0; i < 4; i++ {
			for j, pc := range rc6322NestedPattern {
				ordinal += 7
				op := rc6322ImportantOpcodes[pc]
				sp := uint32(0x10000)
				if j >= 1 && j <= 10 {
					sp = 0xfffc
				}
				if j == 2 || j == 3 || j == 5 || j == 6 || j == 8 || j == 9 {
					sp = 0xfff8
				}
				fmt.Fprintf(&b, "%d\t%d\t%d\t%x\t%x\t%x\t%x\n", ordinal, 100+i, ordinal*4, pc, op, sp, 0x2700)
				recorded++
				if i == 0 && j == 2 { // Explicit IRQ vector and RTE *observations*, not stack certifications.
					ordinal += 7
					fmt.Fprintf(&b, "%d\t%d\t%d\t474\t48e7\tfff0\t2700\n", ordinal, 100+i, ordinal*4)
					recorded++
					ordinal += 7
					fmt.Fprintf(&b, "%d\t%d\t%d\t4d6\t4e73\tfff0\t2700\n", ordinal, 100+i, ordinal*4)
					recorded++
				}
			}
		}
		if err := os.WriteFile(filepath.Join(base, "cpu-b-ordered-hook-events.tsv"), []byte(b.String()), 0644); err != nil {
			t.Fatal(err)
		}
		cap := map[string]any{"schema": "chq-rc6322-ordered-hook-cap-v1", "source": "ORIGINAL_ROM_CPU_B_INSTRUCTION_HOOK", "recorded": recorded, "dropped": 0, "cpuBTransitions": ordinal + 100}
		raw, _ := json.Marshal(cap)
		if err := os.WriteFile(filepath.Join(base, "cpu-b-ordered-hook-cap.json"), raw, 0644); err != nil {
			t.Fatal(err)
		}
	}
}
func rc6322Fixture(t *testing.T) string {
	t.Helper()
	dir := rc6318Synth(t)
	rc6319Seed(t, dir)
	rc6321Seed(t, dir)
	rc6322Seed(t, dir)
	return dir
}
func TestRC6322SelectedOrderedNestedAndMutationGuards(t *testing.T) {
	dir := rc6322Fixture(t)
	hash := strings.Repeat("a", 64)
	r, err := rc6322OrderedAudit(dir, hash)
	if err != nil {
		t.Fatal(err)
	}
	if !r.SelectedNestedTemporalOrderVerified || !r.SelectedStackPointerPairsVerified || r.NativePromotionAllowed || r.InterruptRTEStackParityVerified || r.AllInstructionEventsCaptured || r.CompleteFunctionVerified || r.Unobserved0518Executed || len(r.Cases) != 5 {
		t.Fatalf("unsafe maturity %+v", r)
	}
	for _, c := range r.Cases {
		if c.NestedSequences != 4 || c.OrderedPairs != 16 || c.IRQEntriesDuringNested != 1 || c.RTEsDuringNested != 1 || c.IRQContextsProven {
			t.Fatalf("unexpected case %+v", c)
		}
	}
	base := filepath.Join(dir, "branch-matrix", "natural", "live-health", "original")
	path := filepath.Join(base, "cpu-b-ordered-hook-events.tsv")
	original, _ := os.ReadFile(path)
	for _, m := range []struct{ name, before, after string }{
		{"wrong-ordered-return", "5062\t4e75\tfff8", "4f74\t4e75\tfff8"},
		{"incorrect-stack-pointer", "4fb2\t41f9\tfff8", "4fb2\t41f9\tfffa"},
		{"corrupt-authentic-opcode", "4f70\t6100\tfffc", "4f70\t4e75\tfffc"},
		{"duplicate-ordinal", "14\t100\t56", "7\t100\t56"},
		{"unobserved-alternate", "4fb2\t41f9\tfff8", "518\t41f9\tfff8"},
	} {
		changed := strings.Replace(string(original), m.before, m.after, 1)
		if changed == string(original) {
			t.Fatalf("invalid mutation fixture %s", m.name)
		}
		_ = os.WriteFile(path, []byte(changed), 0644)
		if _, err := rc6322OrderedAudit(dir, hash); err == nil {
			t.Errorf("accepted mutation %s", m.name)
		}
		_ = os.WriteFile(path, original, 0644)
	}
	capPath := filepath.Join(base, "cpu-b-ordered-hook-cap.json")
	capOrig, _ := os.ReadFile(capPath)
	for _, val := range []string{`"dropped":1`, `"cpuBTransitions":2000000`} {
		s := string(capOrig)
		if strings.Contains(val, "dropped") {
			s = strings.Replace(s, `"dropped":0`, val, 1)
		} else {
			s = strings.Replace(s, `"cpuBTransitions":`+fmt.Sprint(4*12*7+2*7+100), val, 1)
		}
		if s == string(capOrig) {
			t.Fatal("bad cap mutation")
		}
		_ = os.WriteFile(capPath, []byte(s), 0644)
		if _, err := rc6322OrderedAudit(dir, hash); err == nil {
			t.Errorf("accepted cap mutation %s", val)
		}
		_ = os.WriteFile(capPath, capOrig, 0644)
	}
}
func TestRC6322AuthenticPriorROMEdgesWithDistinctSyntheticTimeline(t *testing.T) {
	source := os.Getenv("CHQ_RC6321_EVIDENCE_ZIP")
	if source == "" {
		t.Skip("set CHQ_RC6321_EVIDENCE_ZIP for authentic original-ROM edge replay (chronology remains synthetic fixture)")
	}
	z, err := zip.OpenReader(source)
	if err != nil {
		t.Fatal(err)
	}
	defer z.Close()
	dir := rc6322Fixture(t)
	used := 0
	for _, f := range z.File {
		if !strings.HasSuffix(f.Name, "/original/cpu-b-observed-edges.tsv") || !strings.HasPrefix(f.Name, "branch-matrix/") {
			continue
		}
		in, err := f.Open()
		if err != nil {
			t.Fatal(err)
		}
		b, err := io.ReadAll(io.LimitReader(in, 3<<20))
		in.Close()
		if err != nil {
			t.Fatal(err)
		}
		if err := os.WriteFile(filepath.Join(dir, filepath.FromSlash(f.Name)), b, 0644); err != nil {
			t.Fatal(err)
		}
		used++
	}
	if used != 5 {
		t.Fatalf("unexpected real original-ROM edge files %d", used)
	}
	report, err := rc6322OrderedAudit(dir, strings.Repeat("a", 64))
	if err != nil {
		t.Fatal(err)
	}
	if len(report.Cases) != 5 {
		t.Fatal("missing checkpoint cases")
	}
	t.Log("five authentic Windows original-ROM aggregate edge tables accepted; temporal rows and Ghidra sites in this local test are EXPLICITLY SYNTHETIC")
}
