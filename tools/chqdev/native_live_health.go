package main

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

type rc631HealthReport struct {
	Schema               string `json:"schema"`
	Attempts             int64  `json:"attempts"`
	Commits              int64  `json:"commits"`
	Rejected             int64  `json:"rejected"`
	Rollbacks            int64  `json:"rollbacks"`
	GeneratedLibraryUsed bool   `json:"generatedLibraryUsed"`
	LastReason           string `json:"lastReason"`
}

type rc631HealthProof struct {
	Schema               string   `json:"schema"`
	Status               string   `json:"status"`
	Scope                string   `json:"scope"`
	OriginalReturns      int      `json:"originalReturns"`
	NativeReturns        int      `json:"nativeReturns"`
	NativeCommits        int64    `json:"nativeCommits"`
	ReturnStateEqual     bool     `json:"returnStateEqual"`
	MachineStateEqual    bool     `json:"machineStateEqual"`
	BusTransactionsEqual bool     `json:"busTransactionsEqual"`
	RollbackBusEqual     bool     `json:"rollbackBusEqual"`
	RollbackEqual        bool     `json:"rollbackEqual"`
	ForcedRollbacks      int64    `json:"forcedRollbacks"`
	EntryCase            string   `json:"entryCase"`
	EntrySeedEqual       bool     `json:"entrySeedEqual"`
	OriginalPCPath       []string `json:"originalPCPath"`
	PathClassification   string   `json:"pathClassification"`
}

// Preserve all captured state bytes while normalising only the record label.
// This avoids a guaranteed false mismatch between literal "rom" and "native"
// without masking differences in PC, registers, RAM, SR, or cycles.
func rc631ComparableHealthStates(data []byte) ([]byte, error) {
	var out bytes.Buffer
	for _, row := range bytes.Split(bytes.TrimSuffix(data, []byte{'\n'}), []byte{'\n'}) {
		fields := bytes.SplitN(row, []byte{'\t'}, 3)
		if len(fields) != 3 || (string(fields[1]) != "rom" && string(fields[1]) != "native") {
			return nil, errors.New("invalid health return-state trace schema")
		}
		out.Write(fields[0])
		out.WriteByte('\t')
		out.Write(fields[2])
		out.WriteByte('\n')
	}
	return out.Bytes(), nil
}

// Live proof requires the real original-ROM executable and a generated DLL/SO.
// It never treats offline fixture PASS as live substitution or function signoff.
func rc631RunLiveHealth(root, dest, romDir, generatedLib string) (rc631HealthProof, error) {
	return rc633RunHealthCase(root, dest, romDir, generatedLib, "natural")
}
func rc633RunHealthCase(root, dest, romDir, generatedLib, caseName string) (rc631HealthProof, error) {
	proof := rc631HealthProof{Schema: "chq-rc633-live-branch-proof-v1", Status: "NOT_VERIFIED", Scope: "CPU-A unreviewed health function, real-ROM execution; controlled entry RAM is NOT naturally occurring gameplay", EntryCase: caseName}
	if _, ok := rc633ExpectedPath[caseName]; !ok {
		return proof, fmt.Errorf("unsupported health branch case %q", caseName)
	}
	exe := filepath.Join(root, "out", "build", "x64-Debug", "Debug", "ChaseHQRuntime.exe")
	if runtime.GOOS != "windows" {
		exe = filepath.Join(root, "out", "build", "x64-Debug", "ChaseHQRuntime")
	}
	if _, err := os.Stat(exe); err != nil {
		return proof, fmt.Errorf("standalone ChaseHQRuntime missing; qualify/build Windows source first: %w", err)
	}
	cp := filepath.Join(root, "checkpoints", "stage1-target-immediate-pre-damage-6333.chqstate")
	if _, err := os.Stat(cp); err != nil {
		return proof, err
	}
	if _, err := os.Stat(generatedLib); err != nil {
		return proof, err
	}
	type replay struct {
		states, end, bus, path, seed, entry []byte
		report                              rc631HealthReport
	}
	run := func(label string, more ...string) (replay, error) {
		d := filepath.Join(dest, "live-health", label)
		if err := os.MkdirAll(d, 0755); err != nil {
			return replay{}, err
		}
		args := []string{"--roms", romDir, "--load-checkpoint", cp, "--frames", "6340", "--irq4", "--native-health-state-capture", "--native-dispatch-audit", "--native-health-case", caseName, "--logs", d}
		args = append(args, more...)
		ctx, cancel := context.WithTimeout(context.Background(), 3*time.Minute)
		defer cancel()
		cmd := exec.CommandContext(ctx, exe, args...)
		cmd.Dir = root
		out, err := cmd.CombinedOutput()
		_ = os.WriteFile(filepath.Join(d, "runtime-console.txt"), out, 0644)
		if err != nil {
			return replay{}, fmt.Errorf("%s original-ROM replay: %w", label, err)
		}
		states, err := os.ReadFile(filepath.Join(d, "native-health-state.tsv"))
		if err != nil {
			return replay{}, err
		}
		machine, err := os.ReadFile(filepath.Join(d, "native-machine-endstate.json"))
		if err != nil {
			return replay{}, err
		}
		bus, err := os.ReadFile(filepath.Join(d, "native-health-bus.tsv"))
		if err != nil {
			return replay{}, err
		}
		path, err := os.ReadFile(filepath.Join(d, "native-health-path.tsv"))
		if err != nil {
			return replay{}, err
		}
		seed, err := os.ReadFile(filepath.Join(d, "native-health-seed.tsv"))
		if err != nil {
			return replay{}, err
		}
		entry, err := os.ReadFile(filepath.Join(d, "native-health-entry.tsv"))
		if err != nil {
			return replay{}, err
		}
		var report rc631HealthReport
		if label != "original" {
			b, err := os.ReadFile(filepath.Join(d, "native-health-report.json"))
			if err != nil {
				return replay{}, err
			}
			if err = json.Unmarshal(b, &report); err != nil {
				return replay{}, err
			}
		}
		return replay{states: states, end: machine, bus: bus, path: path, seed: seed, entry: entry, report: report}, nil
	}
	reference, err := run("original", "--rom-census", "--cpu-b-edge-probe")
	if err != nil {
		return proof, err
	}
	proof.OriginalReturns = bytes.Count(reference.states, []byte{'\n'})
	if proof.OriginalReturns != 1 {
		return proof, fmt.Errorf("expected exactly one original-ROM health function return, got %d", proof.OriginalReturns)
	}
	originalPath, err := rc633ParseOriginalPath(reference.path)
	if err != nil {
		return proof, err
	}
	proof.OriginalPCPath = originalPath
	proof.PathClassification = rc633ClassifyPath(originalPath)
	expected := rc633ExpectedPath[caseName]
	if caseName != "natural" && proof.PathClassification != expected {
		return proof, fmt.Errorf("real-ROM branch path for %s: got %s want %s; actual PCs=%v", caseName, proof.PathClassification, expected, originalPath)
	}
	candidate, err := run("generated-native", "--native-health-library", generatedLib, "--native-health-replace")
	if err != nil {
		return proof, err
	}
	proof.NativeReturns = bytes.Count(candidate.states, []byte{'\n'})
	proof.NativeCommits = candidate.report.Commits
	origComparable, origErr := rc631ComparableHealthStates(reference.states)
	nativeComparable, nativeErr := rc631ComparableHealthStates(candidate.states)
	if origErr != nil {
		return proof, origErr
	}
	if nativeErr != nil {
		return proof, nativeErr
	}
	proof.ReturnStateEqual = bytes.Equal(origComparable, nativeComparable) // compare actual CPU state, never synthesized
	proof.MachineStateEqual = bytes.Equal(reference.end, candidate.end)
	proof.BusTransactionsEqual = len(reference.bus) > 0 && bytes.Equal(reference.bus, candidate.bus)
	proof.EntrySeedEqual = len(reference.seed) > 0 && bytes.Equal(reference.seed, candidate.seed) && len(reference.entry) > 0 && bytes.Equal(reference.entry, candidate.entry)
	if !proof.EntrySeedEqual || candidate.report.Commits == 0 || !candidate.report.GeneratedLibraryUsed || candidate.report.Commits != int64(proof.OriginalReturns) || candidate.report.Rollbacks != 0 || !proof.ReturnStateEqual || !proof.MachineStateEqual || !proof.BusTransactionsEqual {
		return proof, fmt.Errorf("native function mismatch: returns=%d/%d commits=%d regs=%v machine=%v bus=%v; original/native files retained", proof.NativeReturns, proof.OriginalReturns, candidate.report.Commits, proof.ReturnStateEqual, proof.MachineStateEqual, proof.BusTransactionsEqual)
	}
	rollback, err := run("forced-mismatch", "--native-health-library", generatedLib, "--native-health-fault-probe")
	if err != nil {
		return proof, err
	}
	proof.ForcedRollbacks = rollback.report.Rollbacks
	rollbackComparable, err := rc631ComparableHealthStates(rollback.states)
	if err != nil {
		return proof, err
	}
	proof.RollbackBusEqual = len(reference.bus) > 0 && bytes.Equal(reference.bus, rollback.bus)
	proof.EntrySeedEqual = proof.EntrySeedEqual && bytes.Equal(reference.seed, rollback.seed) && bytes.Equal(reference.entry, rollback.entry)
	rollbackPath, parseErr := rc633ParseOriginalPath(rollback.path)
	if parseErr != nil {
		return proof, parseErr
	}
	proof.RollbackEqual = bytes.Equal(origComparable, rollbackComparable) && bytes.Equal(reference.end, rollback.end) && proof.RollbackBusEqual && proof.EntrySeedEqual && strings.Join(originalPath, ",") == strings.Join(rollbackPath, ",")
	if rollback.report.Rollbacks != 1 || rollback.report.Commits != 0 || !proof.RollbackEqual || !strings.Contains(rollback.report.LastReason, "original-ROM-fallback") {
		return proof, fmt.Errorf("forced mismatch did not restore authentic original path: rollbacks=%d commits=%d parity=%v", rollback.report.Rollbacks, rollback.report.Commits, proof.RollbackEqual)
	}
	proof.Status = "LIVE_BOUNDED_FUNCTION_PROOF_PASS"
	return proof, nil
}
