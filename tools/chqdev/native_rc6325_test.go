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

// Only the Ghidra/edges/old ordered-PC input may be genuine in local replay.
// Exception-frame RAM bytes are never invented as authentic original-ROM data.
func rc6325SeedSyntheticFrames(t *testing.T, dir string) {
	t.Helper()
	for _, name := range rc6317Cases {
		hooks, total, err := rc6322ReadHooks(dir, name)
		if err != nil {
			t.Fatal(err)
		}
		base := filepath.Join(dir, "branch-matrix", name, "live-health", "original")
		var b strings.Builder
		b.WriteString("ordinal\tframe\tkind\tpc\tsp\tusp\tisp\tsr\tsavedSr\tsavedPc\n")
		pending := false
		cnt := 0
		target := uint32(0x4d8)
		for _, h := range hooks {
			switch h.PC {
			case 0x474:
				if pending {
					t.Fatal("fixture unbalanced")
				}
				pending = true
				fmt.Fprintf(&b, "%d\t%d\thandler\t474\t%x\t100800\t%x\t%x\t0\t%x\n", h.Ordinal, h.Frame, h.SP, h.SP, h.SR, target)
				cnt++
			case 0x4d6:
				if !pending {
					t.Fatal("fixture orphan")
				}
				pending = false
				fmt.Fprintf(&b, "%d\t%d\trte\t4d6\t%x\t100800\t%x\t%x\t0\t%x\n", h.Ordinal, h.Frame, h.SP, h.SP, h.SR, target)
				fmt.Fprintf(&b, "%d\t%d\tresume\t%x\t100800\t100800\t%x\t0\t0\t0\n", h.Ordinal+1, h.Frame, target, h.SP+6)
				cnt += 2
			}
		}
		if pending || cnt == 0 {
			t.Fatal("fixture incomplete")
		}
		if err := os.WriteFile(filepath.Join(base, "cpu-b-exception-frame-hooks.tsv"), []byte(b.String()), 0644); err != nil {
			t.Fatal(err)
		}
		cap := map[string]any{"schema": "chq-rc6326-cpu-b-exception-frame-cap-v2", "source": "ORIGINAL_ROM_CPU_B_INSTRUCTION_HOOK_RAM_PEEK", "recorded": cnt, "dropped": 0, "pendingRte": false, "cpuBTransitions": total}
		raw, _ := json.Marshal(cap)
		if err := os.WriteFile(filepath.Join(base, "cpu-b-exception-frame-cap.json"), raw, 0644); err != nil {
			t.Fatal(err)
		}
		// The earlier synthetic edge fixture already includes RTE->0x4D8
		// with a larger count than these selected synthetic hooks. Do not
		// rewrite the ROM/Ghidra prerequisites to manufacture parity.

	}
}
func TestRC6325SyntheticMemoryFrameAndMutations(t *testing.T) {
	dir := rc6322Fixture(t)
	rc6325SeedSyntheticFrames(t, dir)
	hash := strings.Repeat("a", 64)
	r, err := rc6325ExceptionFrameAudit(dir, hash)
	if err != nil {
		t.Fatal(err)
	}
	if !r.SampledExceptionFrameBytesStable || !r.SampledInterruptedPCSRRestored || !r.ImmediatePostRTEHookVerified || !r.StackBankSwitchVerified || !r.SupervisorFramePopVerified || r.ExceptionStackBusOrderVerified || r.IRQVectorACKOwnershipVerified || r.CompleteIRQDeliveryVerified || r.SchedulerParityVerified || r.NativePromotionAllowed || len(r.Cases) != 5 {
		t.Fatalf("unsafe proof %+v", r)
	}
	base := filepath.Join(dir, "branch-matrix", "natural", "live-health", "original")
	path := filepath.Join(base, "cpu-b-exception-frame-hooks.tsv")
	old, err := os.ReadFile(path)
	if err != nil {
		t.Fatal(err)
	}
	for _, tc := range []struct{ name, before, after string }{
		{"saved-pc-mismatch", "\t0\t4d8\n", "\t0\t4de\n"},
		{"resume-pc-mismatch", "\tresume\t4d8\t", "\tresume\t4de\t"},
		{"corrupted-active-user-sp", "\tresume\t4d8\t100800\t", "\tresume\t4d8\t100802\t"},
		{"corrupted-restored-usp", "\tresume\t4d8\t100800\t100800\t", "\tresume\t4d8\t100800\t100802\t"},
		{"bad-sr", "\t0\t0\t0\n", "\t2000\t0\t0\n"},
		{"duplicate-return", "\trte\t4d6\t", "\thandler\t4d6\t"},
		{"missing-next-hook", "\tresume\t4d8\t", "\tresume\t4da\t"},
	} {
		patched := strings.Replace(string(old), tc.before, tc.after, 1)
		if patched == string(old) {
			t.Fatalf("failed synthetic mutation %s", tc.name)
		}
		if err := os.WriteFile(path, []byte(patched), 0644); err != nil {
			t.Fatal(err)
		}
		if _, err := rc6325ExceptionFrameAudit(dir, hash); err == nil {
			t.Errorf("accepted synthetic mutation %s", tc.name)
		}
	}
	// Independent stack-bank corruption: change the inactive USP at handler,
	// the ISP after pop, and the supervisor-mode bank selection.
	rows := strings.Split(strings.TrimSuffix(string(old), "\n"), "\n")
	for _, tc := range []struct {
		name        string
		row         int
		field       int
		replacement string
	}{
		{"bad-handler-usp", 1, 5, "100802"},
		{"bad-handler-isp", 1, 6, "100002"},
		{"bad-resume-isp", 3, 6, "100002"},
		{"bad-resume-user-mode", 3, 7, "2000"},
	} {
		changed := append([]string(nil), rows...)
		fields := strings.Split(changed[tc.row], "\t")
		fields[tc.field] = tc.replacement
		changed[tc.row] = strings.Join(fields, "\t")
		if err := os.WriteFile(path, []byte(strings.Join(changed, "\n")+"\n"), 0644); err != nil {
			t.Fatal(err)
		}
		if _, err := rc6325ExceptionFrameAudit(dir, hash); err == nil {
			t.Errorf("accepted %s", tc.name)
		}
	}
	if err := os.WriteFile(path, old, 0644); err != nil {
		t.Fatal(err)
	}
	capPath := filepath.Join(base, "cpu-b-exception-frame-cap.json")
	capRaw, err := os.ReadFile(capPath)
	if err != nil {
		t.Fatal(err)
	}
	for _, mut := range []string{`"dropped":1`, `"pendingRte":true`} {
		changed := string(capRaw)
		if strings.Contains(mut, "dropped") {
			changed = strings.Replace(changed, `"dropped":0`, mut, 1)
		} else {
			changed = strings.Replace(changed, `"pendingRte":false`, mut, 1)
		}
		if err := os.WriteFile(capPath, []byte(changed), 0644); err != nil {
			t.Fatal(err)
		}
		if _, err := rc6325ExceptionFrameAudit(dir, hash); err == nil {
			t.Errorf("accepted unsafe cap %s", mut)
		}
	}
	os.WriteFile(capPath, capRaw, 0644)
	if _, err := rc6325ExceptionFrameAudit(dir, hash); err != nil {
		t.Fatal(err)
	}
}

func TestRC6325GenuineCHQ6324PrerequisitesSyntheticFrameOnly(t *testing.T) {
	archive := os.Getenv("CHQ_RC6324_EVIDENCE_ZIP")
	if archive == "" {
		t.Skip("set CHQ_RC6324_EVIDENCE_ZIP for genuine previous Windows ROM/Ghidra prerequisites; new frame fields remain SYNTHETIC and unverified until RC6.3.25 Windows run")
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
			t.Fatalf("invalid source %q", name)
		}
		if !(strings.HasPrefix(name, "branch-matrix/") && strings.Contains(name, "/original/") || strings.Contains(name, "ghidra") || strings.Contains(name, "cpu-b-nested") || strings.Contains(name, "cpu-b-original")) {
			continue
		}
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
		_, e := io.Copy(out, io.LimitReader(in, 20<<20))
		in.Close()
		ce := out.Close()
		if e != nil || ce != nil {
			t.Fatalf("extract %s %v %v", name, e, ce)
		}
		used++
	}
	if used < 30 {
		t.Fatalf("too few source artifacts %d", used)
	}
	raw, err := os.ReadFile(filepath.Join(dir, "cpu-b-ghidra", "cpu-b-ghidra-sites.json"))
	if err != nil {
		t.Fatal(err)
	}
	var rom struct {
		Raw string `json:"rawRomSha256"`
	}
	if json.Unmarshal(raw, &rom) != nil || len(rom.Raw) != 64 {
		t.Fatal("ROM provenance absent")
	}
	previous, err := rc6324IRQHookAudit(dir, rom.Raw)
	if err != nil {
		t.Fatalf("authentic previous IRQ evidence: %v", err)
	}
	for _, c := range previous.Cases {
		if c.PairedHandlerRTE != 7 {
			t.Fatal("unexpected authentic IRQ count")
		}
	}
	// No new RC6.3.25 frame bytes exist in old genuine evidence: must fail closed.
	if _, err := rc6325ExceptionFrameAudit(dir, rom.Raw); err == nil {
		t.Fatal("promoted with missing new exception-frame snapshots")
	}
	t.Log("PASS: authentic RC6.3.24 predecessor (35 IRQ/RTE pairs) accepted; missing NEW frame snapshots rejected. New snapshot data awaits Windows acceptance")
}

// In a supervisor-mode return, RTE must leave A7 on the popped ISP rather
// than selecting USP. This fixture is explicitly SYNTHETIC, never ROM evidence.
func TestRC6326SyntheticSupervisorModeRTEReturn(t *testing.T) {
	dir := rc6322Fixture(t)
	rc6325SeedSyntheticFrames(t, dir)
	base := filepath.Join(dir, "branch-matrix", "natural", "live-health", "original")
	path := filepath.Join(base, "cpu-b-exception-frame-hooks.tsv")
	raw, err := os.ReadFile(path)
	if err != nil {
		t.Fatal(err)
	}
	lines := strings.Split(strings.TrimSuffix(string(raw), "\n"), "\n")
	if len(lines) < 4 {
		t.Fatal("short synthetic evidence")
	}
	for _, row := range []int{1, 2} {
		f := strings.Split(lines[row], "\t")
		if len(f) != 10 {
			t.Fatal("invalid synthetic fields")
		}
		f[8] = "2000"
		lines[row] = strings.Join(f, "\t")
	}
	f := strings.Split(lines[3], "\t")
	f[7] = "2000"
	f[4] = f[6] // active supervisor SP must equal popped ISP, NOT USP
	lines[3] = strings.Join(f, "\t")
	encoded := []byte(strings.Join(lines, "\n") + "\n")
	if err := os.WriteFile(path, encoded, 0644); err != nil {
		t.Fatal(err)
	}
	result, err := rc6325ExceptionFrameAudit(dir, strings.Repeat("a", 64))
	if err != nil {
		t.Fatal(err)
	}
	if !result.StackBankSwitchVerified || !result.SupervisorFramePopVerified || !result.Cases[0].Pairs[0].RestoredSupervisorMode {
		t.Fatal("supervisor return not verified")
	}
	// Selecting USP as active SP in supervisor mode must fail.
	f = strings.Split(lines[3], "\t")
	f[4] = "100800"
	lines[3] = strings.Join(f, "\t")
	if err := os.WriteFile(path, []byte(strings.Join(lines, "\n")+"\n"), 0644); err != nil {
		t.Fatal(err)
	}
	if _, err := rc6325ExceptionFrameAudit(dir, strings.Repeat("a", 64)); err == nil {
		t.Fatal("accepted wrong supervisor active stack")
	}
}
