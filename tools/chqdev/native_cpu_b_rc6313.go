package main

// RC6.3.13: compare genuine original ROM, Ghidra generated two-instruction
// TST.W+BEQ and a deliberately rejected candidate. Offline ABI is not proof.
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

type rc6313PairReport struct {
	Attempts             int64  `json:"attempts"`
	Commits              int64  `json:"commits"`
	Rejected             int64  `json:"rejected"`
	Rollbacks            int64  `json:"rollbacks"`
	TimesliceGuards      int64  `json:"timesliceGuards"`
	Taken                int64  `json:"taken"`
	NotTaken             int64  `json:"notTaken"`
	GeneratedLibraryUsed bool   `json:"generatedLibraryUsed"`
	LastReason           string `json:"lastReason"`
}
type rc6313PairLiveProof struct {
	Schema                          string `json:"schema"`
	Status                          string `json:"status"`
	Entry                           string `json:"entry"`
	OriginalCompletedPairs          int    `json:"originalCompletedPairs"`
	OriginalTaken                   int    `json:"originalTaken"`
	OriginalNotTaken                int    `json:"originalNotTaken"`
	OriginalInterruptEdges          int    `json:"originalInterruptEdges"`
	NativeCommits                   int64  `json:"nativeCommits"`
	TimesliceFallbacks              int64  `json:"timesliceFallbacks"`
	ForcedRollbacks                 int64  `json:"forcedRollbacks"`
	ReturnRegistersFlagsCyclesEqual bool   `json:"returnRegistersFlagsCyclesEqual"`
	MappedR16BusEqual               bool   `json:"mappedR16BusEqual"`
	OrderedIRQBusEqual              bool   `json:"orderedIRQBusEqual"`
	IRQEventCount                   int    `json:"irqEventCount"`
	CompleteMachineStateEqual       bool   `json:"completeMachineStateEqual"`
	RawBusLogEqual                  bool   `json:"rawBusLogEqual"`
	RollbackReturnEqual             bool   `json:"rollbackReturnEqual"`
	RollbackMappedBusEqual          bool   `json:"rollbackMappedBusEqual"`
	RollbackIRQEqual                bool   `json:"rollbackIRQEqual"`
	RollbackMachineEqual            bool   `json:"rollbackMachineEqual"`
	RollbackRawBusLogEqual          bool   `json:"rollbackRawBusLogEqual"`
	AutonomousPromotionAllowed      bool   `json:"autonomousPromotionAllowed"`
	WholeFunctionVerified           bool   `json:"wholeFunctionVerified"`
}

func rc6313ParsePairStates(b []byte) (taken, notTaken, irq int, err error) {
	if len(b) == 0 || b[len(b)-1] != '\n' {
		return 0, 0, 0, errors.New("empty/truncated pair return-state record")
	}
	for _, line := range strings.Split(strings.TrimSuffix(string(b), "\n"), "\n") {
		cols := strings.Split(line, "\t")
		if len(cols) != 21 {
			return 0, 0, 0, fmt.Errorf("pair CPU register row has %d columns", len(cols))
		}
		for i, s := range cols {
			base := 16
			if i < 2 || i == 20 {
				base = 10
			}
			v, e := strconv.ParseUint(s, base, 64)
			if e != nil {
				return 0, 0, 0, fmt.Errorf("pair return state numeric field %d: %w", i, e)
			}
			if i < 2 && (v < 6333 || v >= 6340) {
				return 0, 0, 0, errors.New("pair state outside original ROM window")
			}
		}
		pc, e := strconv.ParseUint(cols[2], 16, 32)
		if e != nil {
			return 0, 0, 0, e
		}
		if pc == 0x4d8 {
			taken++
		} else if pc == 0x4e0 {
			notTaken++
		} else if pc == 0x474 {
			irq++
		} else {
			return 0, 0, 0, fmt.Errorf("unreviewed pair exit 0x%x", pc)
		}
	}
	return
}
func rc6313PairBus(b []byte) (int, error) {
	if len(b) == 0 || b[len(b)-1] != '\n' {
		return 0, errors.New("CPU-B TST.W missing R16 data-bus stream")
	}
	n := 0
	for _, line := range strings.Split(strings.TrimSuffix(string(b), "\n"), "\n") {
		c := strings.Split(line, "\t")
		if len(c) != 5 || c[1] != "R" || c[2] != "100804" || c[4] != "2" {
			return 0, fmt.Errorf("CPU-B pair unexpected data-bus transaction: %s", line)
		}
		if _, e := strconv.ParseUint(c[0], 10, 32); e != nil {
			return 0, e
		}
		if _, e := strconv.ParseUint(c[3], 16, 16); e != nil {
			return 0, e
		}
		n++
	}
	return n, nil
}
func rc6313RunCPUBPairLive(root, dest, romDir, library string) (rc6313PairLiveProof, error) {
	p := rc6313PairLiveProof{Schema: "chq-rc6313-cpu-b-two-instruction-live-v1", Status: "BLOCKED", Entry: "B:0x0004D8-0x0004E0"}
	if _, e := os.Stat(library); e != nil {
		return p, fmt.Errorf("authentic generated pair DLL missing: %w", e)
	}
	exe := filepath.Join(root, "out", "build", "x64-Debug", "Debug", "ChaseHQRuntime.exe")
	if runtime.GOOS != "windows" {
		exe = filepath.Join(root, "out", "build", "x64-Debug", "ChaseHQRuntime")
	}
	if _, e := os.Stat(exe); e != nil {
		return p, fmt.Errorf("live CPU-B runtime missing: %w", e)
	}
	checkpoint := filepath.Join(root, "checkpoints", "stage1-target-immediate-pre-damage-6333.chqstate")
	if _, e := os.Stat(checkpoint); e != nil {
		return p, fmt.Errorf("authentic CPU-B checkpoint missing: %w", e)
	}
	type record struct {
		states, bus, irq, end, rawBus []byte
		report                        rc6313PairReport
	}
	run := func(mode string, args ...string) (record, error) {
		d := filepath.Join(dest, "live-cpu-b-pair", mode)
		if e := os.MkdirAll(d, 0755); e != nil {
			return record{}, e
		}
		flags := []string{"--roms", romDir, "--load-checkpoint", checkpoint, "--frames", "6340", "--irq4", "--native-cpu-b-pair-state-capture", "--native-dispatch-audit", "--logs", d}
		flags = append(flags, args...)
		ctx, cancel := context.WithTimeout(context.Background(), 4*time.Minute)
		defer cancel()
		cmd := exec.CommandContext(ctx, exe, flags...)
		cmd.Dir = root
		output, e := cmd.CombinedOutput()
		_ = os.WriteFile(filepath.Join(d, "runtime-console.txt"), output, 0644)
		if e != nil {
			return record{}, fmt.Errorf("CPU-B pair %s ROM replay: %w", mode, e)
		}
		rec := record{}
		get := func(file string) ([]byte, error) { return os.ReadFile(filepath.Join(d, file)) }
		if rec.states, e = get("native-cpu-b-pair-state.tsv"); e != nil {
			return rec, e
		}
		if rec.bus, e = get("native-cpu-b-pair-bus.tsv"); e != nil {
			return rec, e
		}
		if rec.irq, e = get("native-cpu-b-pair-irq.tsv"); e != nil {
			return rec, e
		}
		if rec.end, e = get("native-machine-endstate.json"); e != nil {
			return rec, e
		}
		if rec.rawBus, e = get("bus.log"); e != nil {
			return rec, e
		}
		b, e := get("native-cpu-b-pair-report.json")
		if e != nil {
			return rec, e
		}
		if e = json.Unmarshal(b, &rec.report); e != nil {
			return rec, e
		}
		return rec, nil
	}
	orig, e := run("original")
	if e != nil {
		return p, e
	}
	p.OriginalTaken, p.OriginalNotTaken, p.OriginalInterruptEdges, e = rc6313ParsePairStates(orig.states)
	if e != nil {
		return p, e
	}
	p.OriginalCompletedPairs = p.OriginalTaken + p.OriginalNotTaken + p.OriginalInterruptEdges
	if e = rc6314PairDispatchAudit(filepath.Join(dest, "live-cpu-b-pair", "original", "native-dispatch.tsv"), "original", orig.report.Commits, orig.report.TimesliceGuards, orig.report.Rollbacks); e != nil {
		return p, e
	}
	if p.OriginalTaken < 1 || p.OriginalNotTaken < 1 {
		return p, errors.New("authentic original CPU-B pair lacks both branch paths")
	}
	busCount, e := rc6313PairBus(orig.bus)
	if e != nil {
		return p, e
	}
	if busCount < p.OriginalCompletedPairs {
		return p, fmt.Errorf("original TST.W data reads %d less than completed pairs %d", busCount, p.OriginalCompletedPairs)
	}
	acks, ops, e := rc6311ParseIRQStream(orig.irq)
	if e != nil {
		return p, e
	}
	p.IRQEventCount = acks + ops
	if p.IRQEventCount == 0 {
		return p, errors.New("original CPU-B has no IRQ evidence")
	}
	native, e := run("generated-native", "--native-cpu-b-pair-library", library, "--native-cpu-b-pair-replace")
	if e != nil {
		return p, e
	}
	if _, _, _, e = rc6313ParsePairStates(native.states); e != nil {
		return p, e
	}
	if _, e = rc6313PairBus(native.bus); e != nil {
		return p, e
	}
	if _, _, e = rc6311ParseIRQStream(native.irq); e != nil {
		return p, e
	}
	p.NativeCommits = native.report.Commits
	if e = rc6314PairDispatchAudit(filepath.Join(dest, "live-cpu-b-pair", "generated-native", "native-dispatch.tsv"), "generated-native", native.report.Commits, native.report.TimesliceGuards, native.report.Rollbacks); e != nil {
		return p, e
	}
	p.TimesliceFallbacks = native.report.TimesliceGuards
	p.ReturnRegistersFlagsCyclesEqual = bytes.Equal(orig.states, native.states)
	p.MappedR16BusEqual = bytes.Equal(orig.bus, native.bus)
	p.OrderedIRQBusEqual = bytes.Equal(orig.irq, native.irq)
	p.CompleteMachineStateEqual = bytes.Equal(orig.end, native.end)
	p.RawBusLogEqual = bytes.Equal(orig.rawBus, native.rawBus)
	if !p.ReturnRegistersFlagsCyclesEqual {
		rc6315CapturePairFailure(dest, orig.states, native.states, "CPU-STATE")
	}
	if !p.MappedR16BusEqual {
		rc6315CapturePairFailure(dest, orig.bus, native.bus, "TST-DATA-BUS")
	}
	if !p.OrderedIRQBusEqual {
		rc6315CapturePairFailure(dest, orig.irq, native.irq, "IRQ-ACK-STREAM")
	}
	if !p.CompleteMachineStateEqual {
		rc6315CapturePairFailure(dest, orig.end, native.end, "MACHINE-ENDSTATE")
	}
	if p.NativeCommits < 1 || !native.report.GeneratedLibraryUsed || !p.ReturnRegistersFlagsCyclesEqual || !p.MappedR16BusEqual || !p.OrderedIRQBusEqual || !p.CompleteMachineStateEqual || !p.RawBusLogEqual {
		return p, fmt.Errorf("CPU-B generated pair original/native mismatch: commits=%d CPU=%t R16=%t IRQ=%t machine=%t", p.NativeCommits, p.ReturnRegistersFlagsCyclesEqual, p.MappedR16BusEqual, p.OrderedIRQBusEqual, p.CompleteMachineStateEqual)
	}
	forced, e := run("forced-mismatch", "--native-cpu-b-pair-library", library, "--native-cpu-b-pair-fault-probe")
	if e != nil {
		return p, e
	}
	if _, _, _, e = rc6313ParsePairStates(forced.states); e != nil {
		return p, e
	}
	if _, e = rc6313PairBus(forced.bus); e != nil {
		return p, e
	}
	if _, _, e = rc6311ParseIRQStream(forced.irq); e != nil {
		return p, e
	}
	p.ForcedRollbacks = forced.report.Rollbacks
	if e = rc6314PairDispatchAudit(filepath.Join(dest, "live-cpu-b-pair", "forced-mismatch", "native-dispatch.tsv"), "forced-mismatch", forced.report.Commits, forced.report.TimesliceGuards, forced.report.Rollbacks); e != nil {
		return p, e
	}
	p.RollbackReturnEqual = bytes.Equal(orig.states, forced.states)
	p.RollbackMappedBusEqual = bytes.Equal(orig.bus, forced.bus)
	p.RollbackIRQEqual = bytes.Equal(orig.irq, forced.irq)
	p.RollbackMachineEqual = bytes.Equal(orig.end, forced.end)
	p.RollbackRawBusLogEqual = bytes.Equal(orig.rawBus, forced.rawBus)
	if !p.RollbackReturnEqual {
		rc6315CapturePairFailure(dest, orig.states, forced.states, "ROLLBACK-CPU")
	}
	if !p.RollbackMappedBusEqual {
		rc6315CapturePairFailure(dest, orig.bus, forced.bus, "ROLLBACK-DATA-BUS")
	}
	if !p.RollbackIRQEqual {
		rc6315CapturePairFailure(dest, orig.irq, forced.irq, "ROLLBACK-IRQ")
	}
	if !p.RollbackMachineEqual {
		rc6315CapturePairFailure(dest, orig.end, forced.end, "ROLLBACK-MACHINE")
	}
	if p.ForcedRollbacks != 1 || !strings.Contains(forced.report.LastReason, "original-ROM-fallback") ||
		!p.RollbackReturnEqual || !p.RollbackMappedBusEqual || !p.RollbackIRQEqual || !p.RollbackMachineEqual || !p.RollbackRawBusLogEqual {
		return p, fmt.Errorf("forced CPU-B pair rollback mismatch: rollback=%d CPU=%t R16=%t IRQ=%t machine=%t", p.ForcedRollbacks, p.RollbackReturnEqual, p.RollbackMappedBusEqual, p.RollbackIRQEqual, p.RollbackMachineEqual)
	}
	p.Status = "LIVE_GHIDRA_CPU_B_TST_BEQ_PAIR_BOUNDED_PASS"
	return p, nil
}
