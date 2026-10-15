package main

// RC6.3.32 checkpoint-driven executable differential replay. This is a
// fail-closed evidence reducer, not a completed function verifier. Every trial
// starts a NEW pair of original/native processes with an identical SHA-pinned
// checkpoint; no shell, memory edits, native promotion or unverified claims.
import (
	"context"
	"crypto/sha256"
	"encoding/hex"
	"encoding/json"
	"errors"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"strconv"
	"strings"
	"time"
)

type rc6332Command struct {
	Executable string   `json:"executable"`
	Args       []string `json:"args"`
	Trace      string   `json:"trace"`
}
type rc6332Plan struct {
	Schema         string        `json:"schema"`
	Candidate      string        `json:"candidate"`
	EvidenceClass  string        `json:"evidenceClass"`
	Checkpoint     string        `json:"checkpoint"`
	StartFrame     int           `json:"startFrame"`
	EndFrame       int           `json:"endFrame"`
	TimeoutSeconds int           `json:"timeoutSeconds"`
	Original       rc6332Command `json:"original"`
	Native         rc6332Command `json:"native"`
}
type rc6332Trial struct {
	Frame              int    `json:"frame"`
	OriginalSHA256     string `json:"originalSha256"`
	NativeSHA256       string `json:"nativeSha256"`
	Diverged           bool   `json:"diverged"`
	FirstDifferentLine int    `json:"firstDifferentLine,omitempty"`
}
type rc6332Report struct {
	Schema                          string            `json:"schema"`
	Candidate                       string            `json:"candidate"`
	EvidenceClass                   string            `json:"evidenceClass"`
	CheckpointSHA256                string            `json:"checkpointSha256"`
	PlanSHA256                      string            `json:"planSha256"`
	ExecutableSHA256                map[string]string `json:"executableSha256"`
	StartFrame                      int               `json:"startFrame"`
	EndFrame                        int               `json:"endFrame"`
	EarliestReproducingEndFrame     int               `json:"earliestReproducingEndFrame"`
	BoundedBisectionVerified        bool              `json:"boundedBisectionVerified"`
	ExhaustiveEarlierFramesVerified bool              `json:"exhaustiveEarlierFramesVerified"`
	OriginalNativeReplayOccurred    bool              `json:"originalNativeReplayOccurred"`
	FullFunctionEquivalenceVerified bool              `json:"fullFunctionEquivalenceVerified"`
	NativePromotionAllowed          bool              `json:"nativePromotionAllowed"`
	Trials                          []rc6332Trial     `json:"trials"`
	Blockers                        []string          `json:"blockers"`
}

func rc6332SHA(p string) (string, error) {
	b, e := os.ReadFile(p)
	if e != nil {
		return "", e
	}
	if len(b) > 128<<20 {
		return "", errors.New("replay input larger than 128MiB")
	}
	h := sha256.Sum256(b)
	return hex.EncodeToString(h[:]), nil
}
func rc6332Validate(p rc6332Plan) error {
	if p.Schema != "chq-rc6332-checkpoint-replay-plan-v1" || len(p.Candidate) == 0 || len(p.Candidate) > 128 ||
		(p.EvidenceClass != "SYNTHETIC_REPLAY_FIXTURE" && p.EvidenceClass != "CONTROLLED_CHECKPOINT_REPLAY" && p.EvidenceClass != "AUTHENTIC_UNMODIFIED_CHECKPOINT_REPLAY") ||
		p.Checkpoint == "" || p.StartFrame < 0 || p.EndFrame <= p.StartFrame || p.EndFrame-p.StartFrame > 8192 ||
		p.TimeoutSeconds < 1 || p.TimeoutSeconds > 120 {
		return errors.New("invalid or unbounded replay plan")
	}
	for _, v := range []rc6332Command{p.Original, p.Native} {
		if v.Executable == "" || !filepath.IsAbs(v.Executable) || !filepath.IsAbs(p.Checkpoint) || len(v.Args) == 0 || len(v.Args) > 80 || !strings.HasPrefix(v.Trace, "{run_dir}/") || strings.Contains(v.Trace, "..") || strings.Contains(strings.TrimPrefix(v.Trace, "{run_dir}/"), "{") {
			return errors.New("replay executable and checkpoint must be absolute; trace must be an isolated {run_dir}/relative/path")
		}
		sf, sc := false, false
		for _, a := range v.Args {
			if strings.Contains(a, "{frame}") {
				sf = true
			}
			if strings.Contains(a, "{checkpoint}") {
				sc = true
			}
			if strings.Contains(a, "{") && !strings.Contains(a, "{frame}") && !strings.Contains(a, "{checkpoint}") && !strings.Contains(a, "{run_dir}") {
				return errors.New("unsupported replay placeholder")
			}
		}
		if !sf || !sc {
			return errors.New("both commands must reference the same checkpoint and frame bound")
		}
	}
	return nil
}
func rc6332Arg(s string, frame int, checkpoint, runDir string) string {
	r := strings.NewReplacer("{frame}", strconv.Itoa(frame), "{checkpoint}", checkpoint, "{run_dir}", runDir)
	return r.Replace(s)
}
func rc6332RunOne(ctx context.Context, command rc6332Command, frame int, checkpoint, dir string) (string, error) {
	if e := os.MkdirAll(dir, 0755); e != nil {
		return "", e
	}
	args := make([]string, len(command.Args))
	for i, a := range command.Args {
		args[i] = rc6332Arg(a, frame, checkpoint, dir)
	}
	cmd := exec.CommandContext(ctx, command.Executable, args...)
	// No shell, no inherited current output dir, and bounded capture.
	cmd.Dir = dir
	out, err := cmd.CombinedOutput()
	if len(out) > 1<<20 {
		out = out[:1<<20]
	}
	_ = os.WriteFile(filepath.Join(dir, "process.log"), out, 0644)
	if err != nil {
		return "", fmt.Errorf("replay subprocess failed: %w", err)
	}
	trace := rc6332Arg(command.Trace, frame, checkpoint, dir)
	st, e := os.Stat(trace)
	if e != nil || st.Size() > 32<<20 || st.Size() == 0 {
		return "", errors.New("missing/empty/oversized replay trace")
	}
	return trace, nil
}

// Preserve the exact RC6.3.32 semantics; RC6.3.33 uses this for authentic
// checkpoint/process matrix trials with a per-child timeout.
func rc6332RunOneWithTimeout(c rc6332Command, frame int, checkpoint, dir string, timeoutSeconds int) (string, error) {
	ctx, cancel := context.WithTimeout(context.Background(), time.Duration(timeoutSeconds)*time.Second)
	defer cancel()
	return rc6332RunOne(ctx, c, frame, checkpoint, dir)
}

func rc6332Reduce(plan rc6332Plan, outDir string) (rc6332Report, error) {
	r := rc6332Report{Schema: "chq-rc6332-checkpoint-replay-reduction-v1", Candidate: plan.Candidate, EvidenceClass: plan.EvidenceClass,
		StartFrame: plan.StartFrame, EndFrame: plan.EndFrame, ExecutableSHA256: map[string]string{}, Trials: []rc6332Trial{},
		Blockers: []string{"Replay comparison does not establish entire function CFG, unknown 0x0518, IRQ or scheduling parity", "Prefix reduction does not prove a naturally reachable condition or complete native function replacement"}}
	if err := rc6332Validate(plan); err != nil {
		return r, err
	}
	if outDir == "" {
		return r, errors.New("output directory required")
	}
	if _, e := os.Stat(outDir); e == nil {
		return r, errors.New("output directory already exists; replay refuses to overwrite evidence")
	}
	b, e := json.Marshal(plan)
	if e != nil {
		return r, e
	}
	h := sha256.Sum256(b)
	r.PlanSHA256 = hex.EncodeToString(h[:])
	for k, p := range map[string]string{"checkpoint": plan.Checkpoint, "original": plan.Original.Executable, "native": plan.Native.Executable} {
		sha, err := rc6332SHA(p)
		if err != nil {
			return r, fmt.Errorf("%s hash: %w", k, err)
		}
		if k == "checkpoint" {
			r.CheckpointSHA256 = sha
		} else {
			r.ExecutableSHA256[k] = sha
		}
	}
	if e := os.MkdirAll(outDir, 0755); e != nil {
		return r, e
	}
	defer func() { _ = rc63WriteJSON(filepath.Join(outDir, "replay-report.json"), r) }()
	checked := map[int]bool{}
	run := func(frame int, suffix string) (bool, error) {
		if x, ok := checked[frame]; ok && suffix == "" {
			return x, nil
		}
		tag := filepath.Join(outDir, "trials", fmt.Sprintf("frame-%08d%s", frame, suffix))
		ctx, cancel := context.WithTimeout(context.Background(), time.Duration(plan.TimeoutSeconds)*time.Second)
		defer cancel()
		a, err := rc6332RunOne(ctx, plan.Original, frame, plan.Checkpoint, filepath.Join(tag, "original"))
		if err != nil {
			return false, fmt.Errorf("original frame %d: %w", frame, err)
		}
		b, err := rc6332RunOne(ctx, plan.Native, frame, plan.Checkpoint, filepath.Join(tag, "native"))
		if err != nil {
			return false, fmt.Errorf("native frame %d: %w", frame, err)
		}
		sha, err := rc6332SHA(plan.Checkpoint)
		if err != nil || sha != r.CheckpointSHA256 {
			return false, errors.New("checkpoint changed during replay")
		}
		x, err := rc6331CompareStreams(plan.Candidate, a, b)
		if err != nil {
			return false, err
		}
		trial := rc6332Trial{Frame: frame, OriginalSHA256: x.OriginalSHA256, NativeSHA256: x.NativeSHA256, Diverged: !x.Equal, FirstDifferentLine: x.FirstDifferentLine}
		r.Trials = append(r.Trials, trial)
		if suffix != "" {
			if earlier, ok := checked[frame]; !ok || earlier != trial.Diverged {
				return false, errors.New("non-deterministic divergence across fresh replay at same frame")
			}
		} else {
			checked[frame] = trial.Diverged
		}
		if e := rc63WriteJSON(filepath.Join(tag, "comparison.json"), x); e != nil {
			return false, e
		}
		return trial.Diverged, nil
	}
	if hi, e := run(plan.EndFrame, ""); e != nil {
		return r, e
	} else if !hi {
		return r, errors.New("no divergence at declared endFrame; no failure to reduce")
	}
	if lo, e := run(plan.StartFrame, ""); e != nil {
		return r, e
	} else if lo {
		r.EarliestReproducingEndFrame = plan.StartFrame
		r.Blockers = append(r.Blockers, "Divergence already occurs at the lower bound; earlier origin is unknown")
	} else {
		low, high := plan.StartFrame, plan.EndFrame
		for high-low > 1 {
			mid := low + (high-low)/2
			diverged, e := run(mid, "")
			if e != nil {
				return r, e
			}
			if diverged {
				high = mid
			} else {
				low = mid
			}
		}
		r.EarliestReproducingEndFrame = high
	}
	// Independently re-run both endpoints to reject nondeterministic decisions. Checkpoint
	// and executables were hashed at entry, verify hashes at exit too.
	min := r.EarliestReproducingEndFrame
	if v, e := run(min, "-confirm"); e != nil || !v {
		return r, errors.New("minimum frame did not reproduce")
	}
	for key, p := range map[string]string{"original": plan.Original.Executable, "native": plan.Native.Executable} {
		sha, err := rc6332SHA(p)
		if err != nil || sha != r.ExecutableSHA256[key] {
			return r, errors.New("executable changed during replay")
		}
	}
	// Trial log proves the examined prefix bounds only, not monotonicity
	// between every untested bound. Make this explicit for reviewers.
	r.OriginalNativeReplayOccurred = true
	r.BoundedBisectionVerified = true
	r.ExhaustiveEarlierFramesVerified = r.EarliestReproducingEndFrame == plan.StartFrame+1
	r.Blockers = append(r.Blockers, "Binary prefix bisection assumes monotonic divergence; unsampled intermediate bounds have not been independently verified")
	return r, nil
}
func rc6332ReplayCommand(root string, cfg Config, args []string) *Report {
	rep := newReport("native minimize", cfg.Release)
	if len(args) != 4 || args[0] != "--plan" || args[2] != "--out" {
		rep.fail("arguments", "expected --plan PLAN.json --out NEW_DIRECTORY")
		return rep
	}
	raw, e := os.ReadFile(args[1])
	if e != nil || len(raw) > 64<<10 {
		rep.fail("plan", "missing/oversized plan")
		return rep
	}
	var plan rc6332Plan
	if e := json.Unmarshal(raw, &plan); e != nil {
		rep.fail("plan", e.Error())
		return rep
	}
	result, err := rc6332Reduce(plan, args[3])
	if err != nil {
		rep.fail("replay", err.Error())
		return rep
	}
	rep.Data["replay"] = result
	rep.pass("subprocess-replay", fmt.Sprintf("%d original/native trials; earliest reproduced end frame %d; native promotion BLOCKED", len(result.Trials), result.EarliestReproducingEndFrame))
	return rep
}

// Internal executable fixture: exercises real subprocess launches but never
// claims a genuine ROM or a generated-C++ parity result.
func rc6332ReplayFixture(args []string) *Report {
	rep := newReport("native replay-fixture", "")
	if len(args) != 8 || args[0] != "--checkpoint" || args[2] != "--frame" || args[4] != "--mode" || args[6] != "--trace" {
		rep.fail("arguments", "fixture requires --checkpoint FILE --frame N --mode original|native --trace FILE")
		return rep
	}
	if _, e := os.ReadFile(args[1]); e != nil {
		rep.fail("checkpoint", e.Error())
		return rep
	}
	frame, e := strconv.Atoi(args[3])
	if e != nil || frame < 0 || frame > 1024 || (args[5] != "original" && args[5] != "native") {
		rep.fail("arguments", "bad fixture mode/frame")
		return rep
	}
	out := "sample\tvalue\n"
	for j := 1; j <= frame; j++ {
		value := j
		if args[5] == "native" && j >= 5 {
			value++
		}
		out += fmt.Sprintf("%d\t%d\n", j, value)
	}
	if e := os.WriteFile(args[7], []byte(out), 0644); e != nil {
		rep.fail("trace", e.Error())
		return rep
	}
	rep.pass("synthetic-fixture", "controlled original/native divergence at frame five; NOT ROM evidence")
	return rep
}

// Run a genuine child-process proof of the replay reducer's control flow,
// WITHOUT pretending that the generated frames are original-ROM gameplay.
func rc6332Smoke(dest, root string) (rc6332Report, error) {
	bin, e := os.Executable()
	if e != nil {
		return rc6332Report{}, e
	}
	fixtureDir := filepath.Join(dest, "rc6332-synthetic-replay-fixture")
	if e := os.MkdirAll(fixtureDir, 0755); e != nil {
		return rc6332Report{}, e
	}
	checkpoint := filepath.Join(fixtureDir, "synthetic-checkpoint.NOT-A-ROM-STATE")
	if e := os.WriteFile(checkpoint, []byte("RC6332 SYNTHETIC REPLAY FIXTURE. NEVER CLASSIFY AS ORIGINAL ROM."), 0644); e != nil {
		return rc6332Report{}, e
	}
	mk := func(mode string) rc6332Command {
		return rc6332Command{Executable: bin, Trace: "{run_dir}/trace.tsv", Args: []string{"--root", root, "native", "replay-fixture", "--checkpoint", "{checkpoint}", "--frame", "{frame}", "--mode", mode, "--trace", "{run_dir}/trace.tsv"}}
	}
	p := rc6332Plan{Schema: "chq-rc6332-checkpoint-replay-plan-v1", Candidate: "SYNTHETIC_SUBPROCESS_REPLAY_FIXTURE", EvidenceClass: "SYNTHETIC_REPLAY_FIXTURE", Checkpoint: checkpoint, StartFrame: 1, EndFrame: 8, TimeoutSeconds: 30, Original: mk("original"), Native: mk("native")}
	r, e := rc6332Reduce(p, filepath.Join(fixtureDir, "evidence"))
	if e != nil {
		return r, e
	}
	if r.EarliestReproducingEndFrame != 5 || !r.OriginalNativeReplayOccurred || r.NativePromotionAllowed {
		return r, errors.New("replay smoke did not reproduce synthetic first divergence at frame 5")
	}
	return r, nil
}

type rc6332FunctionFrontier struct {
	Schema                 string   `json:"schema"`
	SourceMode             string   `json:"sourceMode"`
	WriterPC               string   `json:"writerPc"`
	BranchPC               string   `json:"branchPc"`
	WriterAddress          string   `json:"writerAddress"`
	GenuineWriterEvents    int      `json:"genuineWriterEvents"`
	Bit7Transitions        int      `json:"bit7Transitions"`
	Observed0518           bool     `json:"observed0518"`
	VerifiedWholeFunction  bool     `json:"verifiedWholeFunction"`
	NativePromotionAllowed bool     `json:"nativePromotionAllowed"`
	OutstandingProof       []string `json:"outstandingProof"`
}

func rc6332FunctionBoundaryEvidence(d rc6331Dossier) (rc6332FunctionFrontier, error) {
	f := rc6332FunctionFrontier{Schema: "chq-rc6332-function-boundary-confidence-v1", SourceMode: "ORIGINAL_ROM_CHECKPOINT_WINDOWS_WITH_CONTROLLED_CPU_A_HEALTH_CASES", WriterPC: "B:000004A8", BranchPC: "B:00000516", WriterAddress: "0x100802", OutstandingProof: []string{"Writer overwrites 0x80 with 0x80: initial producer before checkpoint not captured", "0x0518 and original-ROM alternate path not observed", "Ghidra static full CFG, indirect callers, complete exits and complete function-native parity not proven"}}
	if d.NativePromotionAllowed || d.VerifiedCompleteCFG || len(d.Cases) != 5 {
		return f, errors.New("unsafe or incomplete previous frontier")
	}
	for _, c := range d.Cases {
		f.GenuineWriterEvents += c.ObservedCPUBWrites
		f.Bit7Transitions += c.ChangedBit7Writes
		if c.ObservedCPUBWrites > 0 {
			found := false
			for _, pc := range c.LatestCandidateWriterPCs {
				if pc == f.WriterPC {
					found = true
				}
			}
			if !found {
				return f, fmt.Errorf("unreviewed writer candidate in %s", c.Case)
			}
		}
	}
	return f, nil
}
