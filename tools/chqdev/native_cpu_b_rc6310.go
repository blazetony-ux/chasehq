package main

// RC6.3.10: actual original CPU-B BEQ.B instruction substitution through the
// shared runtime dispatcher. The Go verifier refuses synthetic/provenance-only
// Ghidra test results: all three modes must replay the SAME genuine original-
// ROM checkpoint and match CPU state, cycles, ordered data bus and end-state.
import (
	"bytes"
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strconv"
	"strings"
	"time"
)

type rc6310Report struct {
	Attempts             int64  `json:"attempts"`
	Commits              int64  `json:"commits"`
	Rejected             int64  `json:"rejected"`
	Rollbacks            int64  `json:"rollbacks"`
	Taken                int64  `json:"taken"`
	NotTaken             int64  `json:"notTaken"`
	InterruptEdges       int64  `json:"interruptEdges"`
	TimesliceGuards      int64  `json:"timesliceGuards"`
	GeneratedLibraryUsed bool   `json:"generatedLibraryUsed"`
	LastReason           string `json:"lastReason"`
}
type rc6310Proof struct {
	Schema                     string `json:"schema"`
	Status                     string `json:"status"`
	Scope                      string `json:"scope"`
	OriginalInstructions       int    `json:"originalInstructions"`
	NativeCommits              int64  `json:"nativeCommits"`
	ForcedFallbacks            int64  `json:"forcedFallbacks"`
	Rollbacks                  int64  `json:"rollbacks"`
	OriginalTaken              int    `json:"originalTaken"`
	OriginalNotTaken           int    `json:"originalNotTaken"`
	OriginalInterruptEdges     int    `json:"originalInterruptEdges"`
	FullCPUStateAndCyclesEqual bool   `json:"fullCPUStateAndCyclesEqual"`
	OrderedMappedDataBusEqual  bool   `json:"orderedMappedDataBusEqual"`
	OrderedIRQBusEqual         bool   `json:"orderedIRQBusEqual"`
	OriginalIRQAcks            int    `json:"originalIRQAcks"`
	OriginalIRQBusOperations   int    `json:"originalIRQBusOperations"`
	CompleteMachineStateEqual  bool   `json:"completeMachineStateEqual"`
	RollbackCPUStateEqual      bool   `json:"rollbackCPUStateEqual"`
	RollbackBusEqual           bool   `json:"rollbackBusEqual"`
	RollbackIRQBusEqual        bool   `json:"rollbackIRQBusEqual"`
	RollbackMachineEqual       bool   `json:"rollbackMachineEqual"`
	GhidraGeneratedDLL         bool   `json:"ghidraGeneratedDLL"`
	WholeFunctionVerified      bool   `json:"wholeFunctionVerified"`
	AutonomousPromotionAllowed bool   `json:"autonomousPromotionAllowed"`
}

func rc6310ParseCPU_BReturns(data []byte) (taken, notTaken, irq int, err error) {
	if len(data) == 0 || data[len(data)-1] != '\n' {
		return 0, 0, 0, errors.New("empty/truncated CPU-B return-state TSV")
	}
	for _, line := range strings.Split(strings.TrimSuffix(string(data), "\n"), "\n") {
		c := strings.Split(line, "\t")
		// entry frame, next frame, nextPC, D0-D7, A0-A7, SR, cumulative cycles
		if len(c) != 21 {
			return 0, 0, 0, fmt.Errorf("malformed CPU-B full register state: %d fields", len(c))
		}
		start, e := strconv.ParseUint(c[0], 10, 32)
		if e != nil || start < 6333 || start >= 6340 {
			return 0, 0, 0, errors.New("CPU-B original entry outside real checkpoint window")
		}
		end, e := strconv.ParseUint(c[1], 10, 32)
		if e != nil || end < start || end >= 6340 {
			return 0, 0, 0, errors.New("CPU-B return outside checkpoint window")
		}
		pc, e := strconv.ParseUint(c[2], 16, 32)
		if e != nil || pc&1 != 0 {
			return 0, 0, 0, errors.New("invalid CPU-B post-branch PC")
		}
		for _, s := range c[3:20] {
			if _, e := strconv.ParseUint(s, 16, 32); e != nil {
				return 0, 0, 0, errors.New("invalid CPU-B register or SR field")
			}
		}
		if _, e := strconv.ParseUint(c[20], 10, 64); e != nil {
			return 0, 0, 0, errors.New("invalid CPU-B cycle field")
		}
		switch pc {
		case 0x4d8:
			taken++
		case 0x4e0:
			notTaken++
		case 0x474:
			irq++
		default:
			return 0, 0, 0, fmt.Errorf("unreviewed CPU-B next-PC 0x%x", pc)
		}
	}
	return
}

// RC6.3.11: IRQ ownership must be supplied by Musashi's ACK callback, not
// inferred from the previously executed branch PC. Every ACK must precede
// its ordered vector-read, stack-PC-write, and stack-SR-write transactions.
// Both missing and invented IRQ effects are disqualifying.
func rc6311ParseIRQStream(data []byte) (acks, operations int, err error) {
	if len(data) == 0 || data[len(data)-1] != '\n' {
		return 0, 0, errors.New("missing/truncated CPU-B IRQ stream")
	}
	expected := []string{"A", "R", "W", "W"}
	var lastFrame uint64
	for i, line := range strings.Split(strings.TrimSuffix(string(data), "\n"), "\n") {
		c := strings.Split(line, "\t")
		if len(c) != 5 || c[1] != expected[i%4] {
			return 0, 0, fmt.Errorf("invalid CPU-B IRQ phase event %d", i)
		}
		frame, e := strconv.ParseUint(c[0], 10, 32)
		if e != nil || frame < 6333 || frame >= 6340 || (i > 0 && frame < lastFrame) {
			return 0, 0, fmt.Errorf("invalid IRQ frame at event %d", i)
		}
		if i%4 != 0 && frame != lastFrame {
			return 0, 0, fmt.Errorf("IRQ bus effects detached from ACK at %d", i)
		}
		lastFrame = frame
		address, e := strconv.ParseUint(c[2], 16, 32)
		if e != nil || address > 0xffffff {
			return 0, 0, fmt.Errorf("invalid IRQ bus address at %d", i)
		}
		val, e := strconv.ParseUint(c[3], 16, 32)
		if e != nil {
			return 0, 0, fmt.Errorf("invalid IRQ bus value at %d", i)
		}
		width, e := strconv.ParseUint(c[4], 10, 32)
		if e != nil {
			return 0, 0, fmt.Errorf("invalid IRQ bus width at %d", i)
		}
		switch i % 4 {
		case 0:
			if width != 0 || val < 1 || val > 7 || address&1 != 0 {
				return 0, 0, fmt.Errorf("invalid CPU-B ACK at %d", i)
			}
			acks++
		case 1:
			if width != 4 || address&1 != 0 {
				return 0, 0, fmt.Errorf("invalid CPU-B vector read %d", i)
			}
			operations++
		case 2:
			if width != 4 || address&1 != 0 {
				return 0, 0, fmt.Errorf("invalid CPU-B return-PC push %d", i)
			}
			operations++
		case 3:
			if width != 2 || address&1 != 0 {
				return 0, 0, fmt.Errorf("invalid CPU-B SR push %d", i)
			}
			operations++
		}
	}
	if acks == 0 || operations != 3*acks {
		return 0, 0, errors.New("missing or incomplete CPU-B IRQ sequences")
	}
	return acks, operations, nil
}

func rc6310RunCPU_BLive(root, dest, romDir, library string) (rc6310Proof, error) {
	proof := rc6310Proof{Schema: "chq-rc6310-cpu-b-native-live-v1", Status: "BLOCKED", Scope: "CPU-B ROM BEQ.B 0x0004DE only; IRQ redirects observed, function boundaries unreviewed"}
	if _, e := os.Stat(library); e != nil {
		return proof, fmt.Errorf("Ghidra CPU-B generated DLL missing: %w", e)
	}
	exe := filepath.Join(root, "out", "build", "x64-Debug", "Debug", "ChaseHQRuntime.exe")
	if runtime.GOOS != "windows" {
		exe = filepath.Join(root, "out", "build", "x64-Debug", "ChaseHQRuntime")
	}
	if _, e := os.Stat(exe); e != nil {
		return proof, fmt.Errorf("native runtime absent: %w", e)
	}
	checkpoint := filepath.Join(root, "checkpoints", "stage1-target-immediate-pre-damage-6333.chqstate")
	if _, e := os.Stat(checkpoint); e != nil {
		return proof, fmt.Errorf("authentic checkpoint absent: %w", e)
	}
	type record struct {
		states, bus, irqBus, end, dispatch []byte
		report                             rc6310Report
	}
	run := func(mode string, extra ...string) (record, error) {
		dir := filepath.Join(dest, "live-cpu-b", mode)
		if e := os.MkdirAll(dir, 0755); e != nil {
			return record{}, e
		}
		args := []string{"--roms", romDir, "--load-checkpoint", checkpoint, "--frames", "6340", "--irq4", "--native-cpu-b-state-capture", "--native-dispatch-audit", "--logs", dir}
		args = append(args, extra...)
		ctx, cancel := context.WithTimeout(context.Background(), 4*time.Minute)
		defer cancel()
		cmd := exec.CommandContext(ctx, exe, args...)
		cmd.Dir = root
		output, e := cmd.CombinedOutput()
		_ = os.WriteFile(filepath.Join(dir, "runtime-console.txt"), output, 0644)
		if e != nil {
			return record{}, fmt.Errorf("CPU-B %s actual ROM replay failed: %w", mode, e)
		}
		var rec record
		read := func(name string) ([]byte, error) { return os.ReadFile(filepath.Join(dir, name)) }
		if rec.states, e = read("native-cpu-b-state.tsv"); e != nil {
			return rec, e
		}
		if rec.bus, e = read("native-cpu-b-bus.tsv"); e != nil {
			return rec, e
		}
		if rec.irqBus, e = read("native-cpu-b-irq.tsv"); e != nil {
			return rec, e
		}
		if rec.end, e = read("native-machine-endstate.json"); e != nil {
			return rec, e
		}
		if rec.dispatch, e = read("native-dispatch.tsv"); e != nil {
			return rec, e
		}
		b, e := read("native-cpu-b-report.json")
		if e != nil {
			return rec, e
		}
		if e = json.Unmarshal(b, &rec.report); e != nil {
			return rec, e
		}
		return rec, nil
	}
	orig, e := run("original", "--rom-census")
	if e != nil {
		return proof, e
	}
	t, nt, irq, e := rc6310ParseCPU_BReturns(orig.states)
	if e != nil {
		return proof, e
	}
	proof.OriginalTaken = t
	proof.OriginalNotTaken = nt
	proof.OriginalInterruptEdges = irq
	proof.OriginalInstructions = t + nt + irq
	// The real Windows ROM census recorded both branches. Zero coverage must
	// never silently qualify a constant branch generator or synthetic fixture.
	if t < 1 || nt < 1 || proof.OriginalInstructions < 10 {
		return proof, fmt.Errorf("original CPU-B has incomplete genuine two-path branch coverage: %d/%d/%d", t, nt, irq)
	}
	proof.OriginalIRQAcks, proof.OriginalIRQBusOperations, e = rc6311ParseIRQStream(orig.irqBus)
	if e != nil {
		return proof, fmt.Errorf("original CPU-B interrupt ownership invalid: %w", e)
	}
	if len(orig.bus) != 0 {
		return proof, errors.New("CPU-B original BEQ emitted mapped data-bus effects; branch-only contract invalid")
	}
	const id = "B:000004DE-000004E0"
	if _, e = rc637RequireDispatch(filepath.Join(dest, "live-cpu-b", "original", "native-dispatch.tsv"), id, "original-rom", proof.OriginalInstructions); e != nil {
		return proof, e
	}
	// RC6.3.14: the registered but disabled TST+BEQ route also emits original-ROM
	// audit rows. Accept ONLY that genuine inactive route; fail closed on any
	// unreviewed CPU-B identity, substituted pair or malformed pair PC/outcome.
	if e = rc6314InactiveCPUBAudit(filepath.Join(dest, "live-cpu-b", "original", "native-dispatch.tsv"), id); e != nil {
		return proof, e
	}
	native, e := run("generated-native", "--native-cpu-b-library", library, "--native-cpu-b-replace")
	if e != nil {
		return proof, e
	}
	if _, _, _, e = rc6310ParseCPU_BReturns(native.states); e != nil {
		return proof, e
	}
	proof.NativeCommits = native.report.Commits
	proof.GhidraGeneratedDLL = native.report.GeneratedLibraryUsed
	proof.FullCPUStateAndCyclesEqual = bytes.Equal(orig.states, native.states)
	proof.OrderedMappedDataBusEqual = bytes.Equal(orig.bus, native.bus)
	if _, _, e = rc6311ParseIRQStream(native.irqBus); e != nil {
		return proof, fmt.Errorf("native CPU-B IRQ stream malformed: %w", e)
	}
	proof.OrderedIRQBusEqual = bytes.Equal(orig.irqBus, native.irqBus)
	proof.CompleteMachineStateEqual = bytes.Equal(orig.end, native.end)
	if native.report.Attempts != int64(proof.OriginalInstructions) || native.report.Commits+native.report.TimesliceGuards != int64(proof.OriginalInstructions) ||
		native.report.Taken < 1 || native.report.NotTaken < 1 ||
		!proof.GhidraGeneratedDLL || !proof.FullCPUStateAndCyclesEqual || !proof.OrderedMappedDataBusEqual ||
		!proof.OrderedIRQBusEqual || !proof.CompleteMachineStateEqual {
		return proof, fmt.Errorf("CPU-B Ghidra native mismatch: original=%d native=%d taken=%d/%d notTaken=%d/%d CPU=%t bus=%t irq=%t machine=%t", proof.OriginalInstructions, proof.NativeCommits, t, native.report.Taken, nt, native.report.NotTaken, proof.FullCPUStateAndCyclesEqual, proof.OrderedMappedDataBusEqual, proof.OrderedIRQBusEqual, proof.CompleteMachineStateEqual)
	}
	if e = rc6310DispatchCounts(filepath.Join(dest, "live-cpu-b", "generated-native", "native-dispatch.tsv"), id, proof.OriginalInstructions, native.report.Commits, native.report.TimesliceGuards); e != nil {
		return proof, e
	}
	fault, e := run("forced-mismatch", "--native-cpu-b-library", library, "--native-cpu-b-fault-probe")
	if e != nil {
		return proof, e
	}
	if _, _, _, e = rc6310ParseCPU_BReturns(fault.states); e != nil {
		return proof, e
	}
	proof.Rollbacks = fault.report.Rollbacks
	proof.ForcedFallbacks = 1
	proof.RollbackCPUStateEqual = bytes.Equal(orig.states, fault.states)
	proof.RollbackBusEqual = bytes.Equal(orig.bus, fault.bus)
	if _, _, e = rc6311ParseIRQStream(fault.irqBus); e != nil {
		return proof, fmt.Errorf("fallback CPU-B IRQ stream malformed: %w", e)
	}
	proof.RollbackIRQBusEqual = bytes.Equal(orig.irqBus, fault.irqBus)
	proof.RollbackMachineEqual = bytes.Equal(orig.end, fault.end)
	if fault.report.Rollbacks != 1 || fault.report.Commits+fault.report.TimesliceGuards != int64(proof.OriginalInstructions-1) ||
		!strings.Contains(fault.report.LastReason, "original-ROM-fallback") ||
		!proof.RollbackCPUStateEqual || !proof.RollbackBusEqual || !proof.RollbackIRQBusEqual || !proof.RollbackMachineEqual {
		return proof, fmt.Errorf("CPU-B forced ROM rollback parity failure: fallbacks=%d CPU=%v bus=%v irq=%v machine=%v", fault.report.Rollbacks, proof.RollbackCPUStateEqual, proof.RollbackBusEqual, proof.RollbackIRQBusEqual, proof.RollbackMachineEqual)
	}
	// Check every dispatcher row, not just aggregate counts; there must be
	// exactly one original fallback and all later branches genuine native commits.
	rows, e := rc637ReadDispatch(filepath.Join(dest, "live-cpu-b", "forced-mismatch", "native-dispatch.tsv"))
	if e != nil {
		return proof, e
	}
	fallback, commits := 0, 0
	for _, row := range rows {
		if row.ID != id {
			if row.CPU == "B" && !rc6314KnownInactiveCPUBPair(row) {
				return proof, fmt.Errorf("unexpected CPU-B dispatch candidate %s PC=0x%x outcome=%s", row.ID, row.PC, row.Outcome)
			}
			continue // unrelated existing CPU-A candidates share this dispatcher
		}
		switch row.Outcome {
		case "original-fallback":
			fallback++
		case "native-commit":
			commits++
		default:
			return proof, errors.New("unexpected CPU-B dispatcher outcome")
		}
	}
	if fallback != int(fault.report.TimesliceGuards+1) || commits != int(fault.report.Commits) || fallback+commits != proof.OriginalInstructions {
		return proof, fmt.Errorf("invalid forced CPU-B dispatch counts %d/%d", fallback, commits)
	}
	proof.Status = "LIVE_GHIDRA_CPU_B_BEQ_BOUNDED_PASS"
	return proof, nil
}

func rc6310DispatchCounts(path, id string, total int, commits, guards int64) error {
	rows, e := rc637ReadDispatch(path)
	if e != nil {
		return e
	}
	c, f := 0, 0
	for _, r := range rows {
		if r.ID != id {
			if r.CPU == "B" && !rc6314KnownInactiveCPUBPair(r) {
				return fmt.Errorf("unknown/active CPU-B companion candidate %s PC=0x%x outcome=%s", r.ID, r.PC, r.Outcome)
			}
			continue // preserve audit of unrelated CPU-A adapters
		}
		if r.Frame < 6333 || r.Frame >= 6340 {
			return errors.New("CPU-B dispatch outside authentic replay window")
		}
		if r.CPU != "B" || r.PC != 0x4de {
			return errors.New("CPU-B shared dispatch unexpected candidate/PC")
		}
		switch r.Outcome {
		case "native-commit":
			c++
		case "original-fallback":
			f++
		default:
			return errors.New("CPU-B live dispatcher left original ROM without a recorded guarded decision")
		}
	}
	if c+f != total || int64(c) != commits || int64(f) != guards {
		return fmt.Errorf("CPU-B native/guard dispatch mismatch: observed %d/%d want %d/%d", c, f, commits, guards)
	}
	return nil
}
