package main

// RC6.3.33: execute genuine checkpoint/original/Ghidra-generated CPU-B pair/
// forced-ROM-rollback as separate processes at several frame boundaries.
// Unlike RC6.3.32's synthetic smoke, this is an authenticated original ROM
// comparison. It remains a BOUNDED two-instruction proof, never a whole function.
import (
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"strings"
)

var rc6333Signals = []string{"native-cpu-b-pair-state.tsv", "native-cpu-b-pair-bus.tsv", "native-cpu-b-pair-irq.tsv", "native-machine-endstate.json", "bus.log"}

type rc6333Signal struct {
	Name                  string `json:"name"`
	OriginalSHA256        string `json:"originalSha256"`
	NativeSHA256          string `json:"nativeSha256"`
	RollbackSHA256        string `json:"rollbackSha256"`
	OriginalNativeEqual   bool   `json:"originalNativeEqual"`
	OriginalRollbackEqual bool   `json:"originalRollbackEqual"`
}
type rc6333Bound struct {
	EndFrame           int            `json:"endFrame"`
	Signals            []rc6333Signal `json:"signals"`
	OriginalPairEvents int            `json:"originalPairEvents"`
	NativeCommits      int64          `json:"nativeCommits"`
	ForcedRollbacks    int64          `json:"forcedRollbacks"`
}
type rc6333Proof struct {
	Schema                        string        `json:"schema"`
	EvidenceClass                 string        `json:"evidenceClass"`
	Candidate                     string        `json:"candidate"`
	CheckpointSHA256              string        `json:"checkpointSha256"`
	RuntimeSHA256                 string        `json:"runtimeSha256"`
	GeneratedLibrarySHA256        string        `json:"generatedLibrarySha256"`
	TestedEndFrames               []int         `json:"testedEndFrames"`
	Bounds                        []rc6333Bound `json:"bounds"`
	CleanConfirmation             bool          `json:"cleanConfirmation"`
	NegativeTraceMutationDetected bool          `json:"negativeTraceMutationDetected"`
	FirstDivergenceWitness        string        `json:"firstDivergenceWitness,omitempty"`
	AutomaticReductionAttempted   bool          `json:"automaticReductionAttempted"`
	BoundedPairParityVerified     bool          `json:"boundedPairParityVerified"`
	WholeFunctionVerified         bool          `json:"wholeFunctionVerified"`
	NativePromotionAllowed        bool          `json:"nativePromotionAllowed"`
	Blockers                      []string      `json:"blockers"`
}

func rc6333RuntimeExecutable(root string) string {
	if runtime.GOOS == "windows" {
		return filepath.Join(root, "out", "build", "x64-Debug", "Debug", "ChaseHQRuntime.exe")
	}
	return filepath.Join(root, "out", "build", "x64-Debug", "ChaseHQRuntime")
}

// Both modes deliberately share the SAME executable and checkpoint and differ
// only in opt-in generated DLL/replacement flags. Runtime emits semantic state
// streams (unlike raw PC tracing, which changes when an opcode is substituted).
func rc6333Command(exe, roms, library, mode string) rc6332Command {
	args := []string{"--roms", roms, "--load-checkpoint", "{checkpoint}", "--frames", "{frame}", "--irq4", "--native-cpu-b-pair-state-capture", "--native-dispatch-audit", "--logs", "{run_dir}"}
	switch mode {
	case "generated-native":
		args = append(args, "--native-cpu-b-pair-library", library, "--native-cpu-b-pair-replace")
	case "forced-rollback":
		args = append(args, "--native-cpu-b-pair-library", library, "--native-cpu-b-pair-fault-probe")
	}
	return rc6332Command{Executable: exe, Args: args, Trace: "{run_dir}/native-cpu-b-pair-state.tsv"}
}

func rc6333ReadReport(dir string) (rc6313PairReport, error) {
	var r rc6313PairReport
	b, e := os.ReadFile(filepath.Join(dir, "native-cpu-b-pair-report.json"))
	if e != nil || len(b) > 65536 {
		return r, errors.New("missing/oversized native CPU-B report")
	}
	if e = json.Unmarshal(b, &r); e != nil {
		return r, e
	}
	if r.Attempts < 0 || r.Commits < 0 || r.Rollbacks < 0 {
		return r, errors.New("invalid native CPU-B counters")
	}
	return r, nil
}

func rc6333Compare(orig, native, forced string) ([]rc6333Signal, string, error) {
	signals := make([]rc6333Signal, 0, len(rc6333Signals))
	firstMismatch := ""
	for _, name := range rc6333Signals {
		a, b, c := filepath.Join(orig, name), filepath.Join(native, name), filepath.Join(forced, name)
		aa, e := os.ReadFile(a)
		if e != nil {
			return nil, "", fmt.Errorf("original %s: %w", name, e)
		}
		bb, e := os.ReadFile(b)
		if e != nil {
			return nil, "", fmt.Errorf("native %s: %w", name, e)
		}
		cc, e := os.ReadFile(c)
		if e != nil {
			return nil, "", fmt.Errorf("rollback %s: %w", name, e)
		}
		if len(aa) == 0 || len(aa) > 32<<20 || len(bb) == 0 || len(bb) > 32<<20 || len(cc) == 0 || len(cc) > 32<<20 {
			return nil, "", fmt.Errorf("truncated/oversized %s", name)
		}
		ah, e := rc6332SHA(a)
		if e != nil {
			return nil, "", e
		}
		bh, e := rc6332SHA(b)
		if e != nil {
			return nil, "", e
		}
		ch, e := rc6332SHA(c)
		if e != nil {
			return nil, "", e
		}
		x := rc6333Signal{Name: name, OriginalSHA256: ah, NativeSHA256: bh, RollbackSHA256: ch, OriginalNativeEqual: bytes.Equal(aa, bb), OriginalRollbackEqual: bytes.Equal(aa, cc)}
		signals = append(signals, x)
		if firstMismatch == "" && (!x.OriginalNativeEqual || !x.OriginalRollbackEqual) {
			firstMismatch = name
		}
	}
	return signals, firstMismatch, nil
}

// No fake divergence is introduced into ROM execution. A separate mutation of
// one COPIED real stream tests whether the first-divergence diagnostic is able
// to reject a mismatch; the mutated bytes are never presented as ROM evidence.
func rc6333MutationGuard(dir, original string) (bool, error) {
	b, e := os.ReadFile(original)
	if e != nil || len(b) < 2 {
		return false, errors.New("missing genuine base trace for mutation guard")
	}
	controlled := append([]byte(nil), b...)
	controlled[len(controlled)/2] ^= 1
	mutated := filepath.Join(dir, "CONTROLLED-TRACE-MUTATION-NOT-ROM.tsv")
	if e = os.WriteFile(mutated, controlled, 0644); e != nil {
		return false, e
	}
	cmp, e := rc6331CompareStreams("CONTROLLED_NEGATIVE_TRACE", original, mutated)
	if e != nil {
		return false, e
	}
	if cmp.Equal || cmp.FirstDifferentLine < 1 || cmp.NativePromotionAllowed {
		return false, errors.New("mutation not rejected")
	}
	if e = rc63WriteJSON(filepath.Join(dir, "controlled-negative-first-divergence.json"), cmp); e != nil {
		return false, e
	}
	return true, nil
}

func rc6333GenuinePairReplay(root, dest, roms, library string) (rc6333Proof, error) {
	p := rc6333Proof{Schema: "chq-rc6333-genuine-cpu-b-checkpoint-matrix-v1", EvidenceClass: "ORIGINAL_ROM_AND_GHIDRA_GENERATED_BOUNDED_CPU_B_PAIR_WITH_CONTROLLED_FALLBACK", Candidate: "CPU_B_0x04D8_TST_WORD_AND_BEQ_PAIR", TestedEndFrames: []int{6337, 6340}, Bounds: []rc6333Bound{}, Blockers: []string{
		"Only the generated two-instruction CPU-B pair is compared; full enclosing function entry, exits, CFG and caller coverage remain unresolved",
		"Original-ROM 0x0518 fallthrough and initial pre-checkpoint producer of bit7 at 0x100802 remain unobserved",
		"Sampled emulator IRQ and machine parity are not complete hardware timing or scheduling proof",
		"Bounded parity or a trace-mutation guard never authorises whole-function or multi-routine native promotion",
	}}
	exe := rc6333RuntimeExecutable(root)
	checkpoint := filepath.Join(root, "checkpoints", "stage1-target-immediate-pre-damage-6333.chqstate")
	for what, path := range map[string]string{"runtime": exe, "checkpoint": checkpoint, "library": library} {
		h, e := rc6332SHA(path)
		if e != nil {
			return p, fmt.Errorf("%s identity: %w", what, e)
		}
		switch what {
		case "runtime":
			p.RuntimeSHA256 = h
		case "checkpoint":
			p.CheckpointSHA256 = h
		case "library":
			p.GeneratedLibrarySHA256 = h
		}
	}
	if dest == "" {
		return p, errors.New("output root required")
	}
	base := filepath.Join(dest, "rc6333-authentic-checkpoint-replay")
	if _, e := os.Stat(base); e == nil {
		return p, errors.New("replay evidence already exists; refusing to overwrite")
	}
	if e := os.MkdirAll(base, 0755); e != nil {
		return p, e
	}
	// Persist a status report even on failure; never erase a mismatching run.
	defer func() { _ = rc63WriteJSON(filepath.Join(base, "replay-proof.json"), p) }()
	modes := []string{"original", "generated-native", "forced-rollback"}
	for _, frame := range p.TestedEndFrames {
		dirs := map[string]string{}
		reports := map[string]rc6313PairReport{}
		for _, mode := range modes {
			d := filepath.Join(base, fmt.Sprintf("frame-%d", frame), mode)
			cmd := rc6333Command(exe, roms, library, mode)
			if _, e := rc6332RunOneWithTimeout(cmd, frame, checkpoint, d, 120); e != nil {
				return p, fmt.Errorf("genuine %s at %d: %w", mode, frame, e)
			}
			dirs[mode] = d
			x, e := rc6333ReadReport(d)
			if e != nil {
				return p, fmt.Errorf("%s report at %d: %w", mode, frame, e)
			}
			reports[mode] = x
		}
		o, n, f := dirs["original"], dirs["generated-native"], dirs["forced-rollback"]
		signals, different, e := rc6333Compare(o, n, f)
		if e != nil {
			return p, e
		}
		b, e := os.ReadFile(filepath.Join(o, "native-cpu-b-pair-state.tsv"))
		if e != nil {
			return p, e
		}
		t, nt, irq, e := rc6313ParsePairStates(b)
		if e != nil {
			return p, e
		}
		nr, fr := reports["generated-native"], reports["forced-rollback"]
		bound := rc6333Bound{EndFrame: frame, Signals: signals, OriginalPairEvents: t + nt + irq, NativeCommits: nr.Commits, ForcedRollbacks: fr.Rollbacks}
		p.Bounds = append(p.Bounds, bound)
		if different != "" {
			witness, err := rc6331CompareStreams(p.Candidate, filepath.Join(o, different), filepath.Join(n, different))
			if err == nil && witness.Equal {
				witness, err = rc6331CompareStreams(p.Candidate, filepath.Join(o, different), filepath.Join(f, different))
			}
			if err == nil {
				p.FirstDivergenceWitness = filepath.Join(base, fmt.Sprintf("frame-%d", frame), "first-divergence.json")
				_ = rc63WriteJSON(p.FirstDivergenceWitness, witness)
			}
			// For the true differing semantic stream run the executable-backed
			// reducer with the SAME authentic checkpoint and native DLL. Reject
			// any missing/truncated outputs, keep all trials in the evidence tree.
			// Reduce the ACTUAL differing comparison: generated-native if it
			// differed, otherwise the forced-ROM rollback stream.
			diffMode := "generated-native"
			for _, signal := range signals {
				if signal.Name == different && signal.OriginalNativeEqual {
					diffMode = "forced-rollback"
					break
				}
			}
			cmdO := rc6333Command(exe, roms, library, "original")
			cmdN := rc6333Command(exe, roms, library, diffMode)
			cmdO.Trace = "{run_dir}/" + different
			cmdN.Trace = cmdO.Trace
			plan := rc6332Plan{Schema: "chq-rc6332-checkpoint-replay-plan-v1", Candidate: p.Candidate + ":" + different, EvidenceClass: "CONTROLLED_CHECKPOINT_REPLAY", Checkpoint: checkpoint, StartFrame: 6334, EndFrame: frame, TimeoutSeconds: 120, Original: cmdO, Native: cmdN}
			p.AutomaticReductionAttempted = true
			result, reduceErr := rc6332Reduce(plan, filepath.Join(base, fmt.Sprintf("frame-%d", frame), "automatic-first-divergence-reduction"))
			_ = rc63WriteJSON(filepath.Join(base, fmt.Sprintf("frame-%d", frame), "reduction-result.json"), result)
			if reduceErr != nil {
				return p, fmt.Errorf("genuine parity mismatch in %s at frame %d; reducer: %v", different, frame, reduceErr)
			}
			return p, fmt.Errorf("genuine bounded pair parity mismatch in %s at frame %d (automatically reduced to %d)", different, frame, result.EarliestReproducingEndFrame)
		}
		if bound.OriginalPairEvents < 1 || nr.Commits < 1 || !nr.GeneratedLibraryUsed || fr.Rollbacks != 1 || !strings.Contains(fr.LastReason, "original-ROM-fallback") {
			return p, fmt.Errorf("unverified commit/rollback at frame %d (original events=%d native commits=%d rollback=%d)", frame, bound.OriginalPairEvents, nr.Commits, fr.Rollbacks)
		}
	}
	// Independently run the high-bound ORIGINAL and generated-native processes
	// again, and compare their output hashes with the initial genuine captures.
	top := p.TestedEndFrames[len(p.TestedEndFrames)-1]
	for _, mode := range []string{"original", "generated-native"} {
		d := filepath.Join(base, fmt.Sprintf("frame-%d-confirm", top), mode)
		if _, e := rc6332RunOneWithTimeout(rc6333Command(exe, roms, library, mode), top, checkpoint, d, 120); e != nil {
			return p, e
		}
		old := filepath.Join(base, fmt.Sprintf("frame-%d", top), mode)
		for _, name := range rc6333Signals {
			a, e := rc6332SHA(filepath.Join(old, name))
			if e != nil {
				return p, e
			}
			b, e := rc6332SHA(filepath.Join(d, name))
			if e != nil {
				return p, e
			}
			if a != b {
				return p, fmt.Errorf("nondeterministic genuine %s replay on %s", mode, name)
			}
		}
	}
	for what, path := range map[string]string{"runtime": exe, "checkpoint": checkpoint, "library": library} {
		h, e := rc6332SHA(path)
		if e != nil {
			return p, e
		}
		expected := p.RuntimeSHA256
		if what == "checkpoint" {
			expected = p.CheckpointSHA256
		}
		if what == "library" {
			expected = p.GeneratedLibrarySHA256
		}
		if h != expected {
			return p, fmt.Errorf("%s changed during replay", what)
		}
	}
	p.CleanConfirmation = true
	guard, e := rc6333MutationGuard(base, filepath.Join(base, fmt.Sprintf("frame-%d", top), "original", "native-cpu-b-pair-state.tsv"))
	if e != nil {
		return p, e
	}
	p.NegativeTraceMutationDetected = guard
	p.BoundedPairParityVerified = true
	return p, nil
}
