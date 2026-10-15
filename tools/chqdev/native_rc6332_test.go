package main

import (
	"archive/zip"
	"encoding/json"
	"io"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"
	"testing"
)

func rc6332TestBinary(t *testing.T) string {
	t.Helper()
	path := filepath.Join(t.TempDir(), "chqdev")
	if runtime.GOOS == "windows" {
		path += ".exe"
	}
	cmd := exec.Command("go", "build", "-o", path, ".")
	if out, e := cmd.CombinedOutput(); e != nil {
		t.Fatalf("build child CLI %v: %s", e, out)
	}
	return path
}
func rc6332FixturePlan(bin, root, checkpoint string, from, to int) rc6332Plan {
	mk := func(mode string) rc6332Command {
		return rc6332Command{Executable: bin, Trace: "{run_dir}/trace.tsv", Args: []string{"--root", root, "native", "replay-fixture", "--checkpoint", "{checkpoint}", "--frame", "{frame}", "--mode", mode, "--trace", "{run_dir}/trace.tsv"}}
	}
	return rc6332Plan{Schema: "chq-rc6332-checkpoint-replay-plan-v1", Candidate: "SYNTHETIC:CPU-B-TEST", EvidenceClass: "SYNTHETIC_REPLAY_FIXTURE", Checkpoint: checkpoint, StartFrame: from, EndFrame: to, TimeoutSeconds: 20, Original: mk("original"), Native: mk("native")}
}
func TestRC6332ActualExecutableBisectionAndReproducer(t *testing.T) {
	root, e := findRoot("")
	if e != nil {
		t.Fatal(e)
	}
	binary := rc6332TestBinary(t)
	checkpoint := filepath.Join(t.TempDir(), "checkpoint.chqstate")
	if e := os.WriteFile(checkpoint, []byte("synthetic checkpoint explicitly NOT original-ROM"), 0644); e != nil {
		t.Fatal(e)
	}
	p := rc6332FixturePlan(binary, root, checkpoint, 1, 12)
	dir := filepath.Join(t.TempDir(), "replay")
	got, e := rc6332Reduce(p, dir)
	if e != nil {
		t.Fatal(e)
	}
	if got.EarliestReproducingEndFrame != 5 || !got.BoundedBisectionVerified || !got.OriginalNativeReplayOccurred || got.NativePromotionAllowed || got.FullFunctionEquivalenceVerified || len(got.Trials) < 5 || got.ExhaustiveEarlierFramesVerified {
		t.Fatalf("wrong reducer proof: %+v", got)
	}
	report, e := os.ReadFile(filepath.Join(dir, "replay-report.json"))
	if e != nil {
		t.Fatal(e)
	}
	var round rc6332Report
	if e := json.Unmarshal(report, &round); e != nil || round.EarliestReproducingEndFrame != 5 {
		t.Fatalf("missing durable report: %v", e)
	}
	if _, e := rc6332Reduce(p, dir); e == nil {
		t.Fatal("silently replaced immutable replay result")
	}
	p.Native.Args[9] = "original"
	if _, e := rc6332Reduce(p, filepath.Join(t.TempDir(), "same")); e == nil || !strings.Contains(e.Error(), "no divergence") {
		t.Fatalf("expected equal-mode rejection, got %v", e)
	}
}
func TestRC6332ReplayPlanFailClosed(t *testing.T) {
	p := rc6332Plan{Schema: "chq-rc6332-checkpoint-replay-plan-v1", Candidate: "B:0516", EvidenceClass: "CONTROLLED_CHECKPOINT_REPLAY", Checkpoint: "/tmp/chq.chqstate", StartFrame: 1, EndFrame: 8, TimeoutSeconds: 10}
	x := rc6332Command{Executable: "/tmp/runtime", Trace: "{run_dir}/trace.tsv", Args: []string{"--load-checkpoint", "{checkpoint}", "--frames", "{frame}"}}
	p.Original, p.Native = x, x
	if e := rc6332Validate(p); e != nil {
		t.Fatal(e)
	}
	for _, mut := range []func(*rc6332Plan){
		func(p *rc6332Plan) { p.Schema = "wrong" }, func(p *rc6332Plan) { p.EvidenceClass = "GENUINE_PROVEN" },
		func(p *rc6332Plan) { p.EndFrame = 90000 }, func(p *rc6332Plan) { p.TimeoutSeconds = 121 },
		func(p *rc6332Plan) { p.Native.Args = []string{"--frames", "{frame}"} },
		func(p *rc6332Plan) { p.Native.Trace = "../../unsafe.tsv" },
		func(p *rc6332Plan) { p.Native.Executable = "relative.exe" },
	} {
		q := p
		mut(&q)
		if e := rc6332Validate(q); e == nil {
			t.Fatalf("unsafe replay accepted: %+v", q)
		}
	}
}

func TestRC6332GenuineWindowsWriterProofAndBoundaryBlock(t *testing.T) {
	zipPath := os.Getenv("CHQ_RC6331_EVIDENCE_ZIP")
	if zipPath == "" {
		t.Skip("set CHQ_RC6331_EVIDENCE_ZIP for exact genuine Windows writer dossier acceptance")
	}
	z, e := zip.OpenReader(zipPath)
	if e != nil {
		t.Fatal(e)
	}
	defer z.Close()
	var d rc6331Dossier
	found := false
	for _, f := range z.File {
		if f.Name == "cpu-b-automated-function-and-writer-dossier.json" {
			r, e := f.Open()
			if e != nil {
				t.Fatal(e)
			}
			b, e := io.ReadAll(io.LimitReader(r, 65536))
			r.Close()
			if e != nil {
				t.Fatal(e)
			}
			if e = json.Unmarshal(b, &d); e != nil {
				t.Fatal(e)
			}
			found = true
			break
		}
	}
	if !found {
		t.Fatal("genuine Windows function dossier absent")
	}
	proof, e := rc6332FunctionBoundaryEvidence(d)
	if e != nil || proof.GenuineWriterEvents != 35 || proof.Bit7Transitions != 0 || proof.NativePromotionAllowed || proof.VerifiedWholeFunction || proof.Observed0518 {
		t.Fatalf("invalid genuine five-case bound: %+v %v", proof, e)
	}
	for _, c := range d.Cases {
		if c.ObservedCPUBWrites != 7 || c.ObservedCPUAWrites != 0 || len(c.LatestCandidateWriterPCs) != 1 || c.LatestCandidateWriterPCs[0] != "B:000004A8" {
			t.Fatalf("unexpected genuine writer: %+v", c)
		}
	}
	d.Cases[0].LatestCandidateWriterPCs = []string{"B:00000518"}
	if _, e := rc6332FunctionBoundaryEvidence(d); e == nil {
		t.Fatal("writer provenance mutation wrongly accepted")
	}
}
