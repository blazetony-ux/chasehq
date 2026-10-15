package main

// RC6.3.6: independently exercise a second generated candidate against the
// exact same immutable original-ROM checkpoint. No promotion from offline
// fixtures, aggregate hashes, or zero observed executions.
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
	"strings"
	"time"
)

type rc636CopyReplayReport struct {
	Attempts             int64  `json:"attempts"`
	Commits              int64  `json:"commits"`
	Rejected             int64  `json:"rejected"`
	Rollbacks            int64  `json:"rollbacks"`
	GeneratedLibraryUsed bool   `json:"generatedLibraryUsed"`
	LastReason           string `json:"lastReason"`
}
type rc636CopyProof struct {
	Schema                  string `json:"schema"`
	Status                  string `json:"status"`
	Scope                   string `json:"scope"`
	OriginalReturns         int    `json:"originalReturns"`
	NativeReturns           int    `json:"nativeReturns"`
	NativeCommits           int64  `json:"nativeCommits"`
	Rollbacks               int64  `json:"forcedRollbacks"`
	CPUReturnStateEqual     bool   `json:"cpuReturnStateEqual"`
	OrderedBusEqual         bool   `json:"orderedBusEqual"`
	FullMachineEqual        bool   `json:"fullMachineEqual"`
	RollbackStateEqual      bool   `json:"rollbackStateEqual"`
	RollbackBusEqual        bool   `json:"rollbackBusEqual"`
	RollbackMachineEqual    bool   `json:"rollbackMachineEqual"`
	OriginalBusTransactions int    `json:"originalBusTransactions"`
	FunctionBoundaryProven  bool   `json:"functionBoundaryProven"`
	EligibleForPromotion    bool   `json:"eligibleForPromotion"`
}

func rc636ComparableCopyStates(data []byte) ([]byte, error) {
	if len(data) == 0 || data[len(data)-1] != '\n' {
		return nil, errors.New("empty/truncated copy return-state file")
	}
	var out bytes.Buffer
	for _, row := range bytes.Split(bytes.TrimSuffix(data, []byte{'\n'}), []byte{'\n'}) {
		cols := bytes.SplitN(row, []byte{'\t'}, 3)
		if len(cols) != 3 || (string(cols[1]) != "rom" && string(cols[1]) != "native") || len(cols[2]) < 16 {
			return nil, errors.New("invalid copy return-state record")
		}
		out.Write(cols[0])
		out.WriteByte('\t')
		out.Write(cols[2])
		out.WriteByte('\n')
	}
	return out.Bytes(), nil
}
func rc636RunCopyProof(root, dest, romDir, copyLib string) (rc636CopyProof, error) {
	proof := rc636CopyProof{Schema: "chq-rc636-copy-block-proof-v1", Status: "NOT_VERIFIED", Scope: "authentic CPU-A 0x40540..0x40560 16-longword contiguous ROM instruction block; function boundaries not reviewed"}
	exe := filepath.Join(root, "out", "build", "x64-Debug", "Debug", "ChaseHQRuntime.exe")
	if runtime.GOOS != "windows" {
		exe = filepath.Join(root, "out", "build", "x64-Debug", "ChaseHQRuntime")
	}
	if _, err := os.Stat(exe); err != nil {
		return proof, fmt.Errorf("standalone runtime missing: %w", err)
	}
	if _, err := os.Stat(copyLib); err != nil {
		return proof, fmt.Errorf("generated copy module missing: %w", err)
	}
	checkpoint := filepath.Join(root, "checkpoints", "stage1-target-immediate-pre-damage-6333.chqstate")
	if _, err := os.Stat(checkpoint); err != nil {
		return proof, err
	}
	type record struct {
		state, bus, end []byte
		report          rc636CopyReplayReport
	}
	run := func(mode string, extra ...string) (record, error) {
		d := filepath.Join(dest, "live-copy", mode)
		if err := os.MkdirAll(d, 0755); err != nil {
			return record{}, err
		}
		args := []string{"--roms", romDir, "--load-checkpoint", checkpoint, "--frames", "6340", "--irq4", "--native-copy-state-capture", "--native-dispatch-audit", "--logs", d}
		args = append(args, extra...)
		ctx, cancel := context.WithTimeout(context.Background(), 3*time.Minute)
		defer cancel()
		cmd := exec.CommandContext(ctx, exe, args...)
		cmd.Dir = root
		output, err := cmd.CombinedOutput()
		_ = os.WriteFile(filepath.Join(d, "runtime-console.txt"), output, 0644)
		if err != nil {
			return record{}, fmt.Errorf("%s runtime replay: %w", mode, err)
		}
		data := record{}
		if data.state, err = os.ReadFile(filepath.Join(d, "native-copy-state.tsv")); err != nil {
			return record{}, err
		}
		if data.bus, err = os.ReadFile(filepath.Join(d, "native-copy-bus.tsv")); err != nil {
			return record{}, err
		}
		if data.end, err = os.ReadFile(filepath.Join(d, "native-machine-endstate.json")); err != nil {
			return record{}, err
		}
		if mode != "original" {
			b, e := os.ReadFile(filepath.Join(d, "native-copy-report.json"))
			if e != nil {
				return record{}, e
			}
			if e = json.Unmarshal(b, &data.report); e != nil {
				return record{}, e
			}
		}
		return data, nil
	}
	orig, err := run("original", "--rom-census")
	if err != nil {
		return proof, err
	}
	originalComparable, err := rc636ComparableCopyStates(orig.state)
	if err != nil {
		return proof, err
	}
	proof.OriginalReturns = bytes.Count(orig.state, []byte{'\n'})
	proof.OriginalBusTransactions = bytes.Count(orig.bus, []byte{'\n'})
	if proof.OriginalReturns < 1 || proof.OriginalBusTransactions != proof.OriginalReturns*32 {
		return proof, fmt.Errorf("no complete original ROM copy block events: returns=%d bus=%d, want 32 operations per return", proof.OriginalReturns, proof.OriginalBusTransactions)
	}
	native, err := run("generated-native", "--native-copy-library", copyLib, "--native-copy-replace")
	if err != nil {
		return proof, err
	}
	nativeComparable, err := rc636ComparableCopyStates(native.state)
	if err != nil {
		return proof, err
	}
	proof.NativeReturns = bytes.Count(native.state, []byte{'\n'})
	proof.NativeCommits = native.report.Commits
	proof.CPUReturnStateEqual = bytes.Equal(originalComparable, nativeComparable)
	proof.OrderedBusEqual = bytes.Equal(orig.bus, native.bus)
	proof.FullMachineEqual = bytes.Equal(orig.end, native.end)
	if native.report.Attempts != int64(proof.OriginalReturns) || native.report.Commits != int64(proof.OriginalReturns) || native.report.Rollbacks != 0 || !native.report.GeneratedLibraryUsed ||
		!proof.CPUReturnStateEqual || !proof.OrderedBusEqual || !proof.FullMachineEqual {
		return proof, fmt.Errorf("second generated copy candidate mismatch: original=%d native=%d commits=%d CPU=%v bus=%v machine=%v", proof.OriginalReturns, proof.NativeReturns, native.report.Commits, proof.CPUReturnStateEqual, proof.OrderedBusEqual, proof.FullMachineEqual)
	}
	rollback, err := run("forced-mismatch", "--native-copy-library", copyLib, "--native-copy-fault-probe")
	if err != nil {
		return proof, err
	}
	rollbackComparable, err := rc636ComparableCopyStates(rollback.state)
	if err != nil {
		return proof, err
	}
	proof.Rollbacks = rollback.report.Rollbacks
	proof.RollbackStateEqual = bytes.Equal(originalComparable, rollbackComparable)
	proof.RollbackBusEqual = bytes.Equal(orig.bus, rollback.bus)
	proof.RollbackMachineEqual = bytes.Equal(orig.end, rollback.end)
	if rollback.report.Rollbacks != 1 || rollback.report.Commits != 0 || !strings.Contains(rollback.report.LastReason, "original-ROM-fallback") ||
		!proof.RollbackStateEqual || !proof.RollbackBusEqual || !proof.RollbackMachineEqual {
		return proof, fmt.Errorf("copy rollback mismatch: rollbacks=%d commits=%d CPU=%v bus=%v machine=%v", rollback.report.Rollbacks, rollback.report.Commits, proof.RollbackStateEqual, proof.RollbackBusEqual, proof.RollbackMachineEqual)
	}
	proof.Status = "LIVE_GENERATED_COPY_BLOCK_PASS"
	return proof, nil
}
