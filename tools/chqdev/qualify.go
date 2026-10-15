package main

import (
	"encoding/json"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"time"
)

type QualificationState struct {
	Schema      string            `json:"schema"`
	Release     string            `json:"release"`
	ToolVersion string            `json:"toolVersion"`
	UpdatedUTC  string            `json:"updatedUtc"`
	Archive     string            `json:"archive,omitempty"`
	ArchiveSHA  string            `json:"archiveSha256,omitempty"`
	Stages      map[string]string `json:"stages"`
}

func qualificationStatePath(root string, cfg Config) string {
	rel := cfg.QualificationState
	if rel == "" {
		rel = "out/chqdev/qualification.json"
	}
	if filepath.IsAbs(rel) {
		return rel
	}
	return filepath.Join(root, filepath.FromSlash(rel))
}

func loadQualificationState(root string, cfg Config) QualificationState {
	q := QualificationState{Schema: "chqdev-qualification-v2", Release: cfg.Release, ToolVersion: toolVersion, Stages: map[string]string{}}
	b, err := os.ReadFile(qualificationStatePath(root, cfg))
	if err == nil {
		var prior QualificationState
		if json.Unmarshal(b, &prior) == nil && prior.Release == cfg.Release {
			q = prior
			if q.Stages == nil {
				q.Stages = map[string]string{}
			}
		}
	}
	q.Schema = "chqdev-qualification-v2"
	q.Release = cfg.Release
	q.ToolVersion = toolVersion
	return q
}

func saveQualificationState(root string, cfg Config, q QualificationState) error {
	q.UpdatedUTC = time.Now().UTC().Format(time.RFC3339Nano)
	p := qualificationStatePath(root, cfg)
	if err := os.MkdirAll(filepath.Dir(p), 0755); err != nil {
		return err
	}
	b, err := json.MarshalIndent(q, "", "  ")
	if err != nil {
		return err
	}
	b = append(b, '\n')
	return os.WriteFile(p, b, 0644)
}

func qualificationRecord(r *Report, root string, cfg Config, q *QualificationState, stage, status string) {
	q.Stages[stage] = status
	if err := saveQualificationState(root, cfg, *q); err != nil {
		r.warn("qualification-state", err.Error())
	}
}

func reportFailureDetail(rep *Report, fallback string) string {
	if rep != nil {
		for _, c := range rep.Checks {
			if c.Status == "FAIL" && c.Detail != "" {
				return c.Detail
			}
		}
	}
	return fallback
}

func readinessFailureDetail(rep *Report) string {
	if rep == nil {
		return "no readiness report"
	}
	for _, c := range rep.Checks {
		if c.Status == "FAIL" {
			if c.Detail != "" {
				return c.Name + " FAIL - " + c.Detail
			}
			return c.Name + " FAIL"
		}
	}
	return "readiness incomplete"
}

func printQualificationChecks(prefix string, rep *Report) {
	if rep == nil {
		return
	}
	for _, c := range rep.Checks {
		if c.Detail != "" {
			fmt.Printf("[QUALIFY] %s - %s %s - %s\n", prefix, c.Name, c.Status, c.Detail)
		} else {
			fmt.Printf("[QUALIFY] %s - %s %s\n", prefix, c.Name, c.Status)
		}
	}
}

func cleanFreshState(root string) error {
	for _, rel := range []string{"out", "dist"} {
		if err := os.RemoveAll(filepath.Join(root, rel)); err != nil {
			return err
		}
	}
	return nil
}

func qualifyCommand(root string, cfg Config, mode, overrideURL string, jsonOut bool) *Report {
	r := newReport("qualify "+mode, cfg.Release)
	q := loadQualificationState(root, cfg)
	r.Data["stateFile"] = qualificationStatePath(root, cfg)

	runDoctorPreflight := func() bool {
		d := doctor(root, cfg)
		if d.Status == "FAIL" {
			r.fail("doctor", "environment check failed")
			r.Data["doctor"] = d
			qualificationRecord(r, root, cfg, &q, "doctor", "FAIL")
			return false
		}
		r.pass("doctor", "PASS")
		qualificationRecord(r, root, cfg, &q, "doctor", "PASS")
		p := preflight(root, cfg)
		if p.Status == "FAIL" {
			r.fail("preflight", "source checks failed")
			r.Data["preflight"] = p
			qualificationRecord(r, root, cfg, &q, "preflight", "FAIL")
			return false
		}
		r.pass("preflight", "PASS")
		qualificationRecord(r, root, cfg, &q, "preflight", "PASS")
		return true
	}

	if mode == "fresh" {
		// A fresh qualification is intentionally a single fail-closed sequence.
		// Windows performs the complete runtime/regression prove-off; non-Windows
		// hosts prove the portable clean build/test/package stages only.
		if runtime.GOOS == "windows" {
			stop := runtimeLifecycle(root, cfg, "stop", RuntimeLaunchOptions{}, jsonOut)
			if stop.Status == "FAIL" {
				r.warn("runtime-stop", "no clean recorded runtime to stop or stop failed; continuing to clean generated state")
			} else {
				r.pass("runtime-stop", "prior recorded runtime stopped")
			}
		}
		if err := cleanFreshState(root); err != nil {
			r.fail("clean", err.Error())
			return r
		}
		r.pass("clean", "removed out/ and dist/ generated state")
		qualificationRecord(r, root, cfg, &q, "clean", "PASS")
		if !runDoctorPreflight() {
			return r
		}
		b := buildCommand(root, cfg, "debug", jsonOut)
		if b.Status == "FAIL" {
			r.fail("build", "debug build failed")
			r.Data["build"] = b
			qualificationRecord(r, root, cfg, &q, "build", "FAIL")
			return r
		}
		r.pass("build", "debug PASS")
		qualificationRecord(r, root, cfg, &q, "build", "PASS")
		t := testCommand(root, cfg, "debug", jsonOut)
		if t.Status == "FAIL" {
			r.fail("test", "CTest failed")
			r.Data["test"] = t
			qualificationRecord(r, root, cfg, &q, "test", "FAIL")
			return r
		}
		r.pass("test", "all CTest cases PASS")
		qualificationRecord(r, root, cfg, &q, "test", "PASS")
		pr := packageCommand(root, cfg, "", jsonOut)
		if pr.Status == "FAIL" {
			r.fail("package", "package/verify/reproducibility failed")
			r.Data["package"] = pr
			qualificationRecord(r, root, cfg, &q, "package", "FAIL")
			return r
		}
		r.pass("package", "deterministic package + verify PASS")
		qualificationRecord(r, root, cfg, &q, "package", "PASS")
		q.Archive, _ = pr.Data["zip"].(string)
		q.ArchiveSHA, _ = pr.Data["sha256"].(string)
		_ = saveQualificationState(root, cfg, q)
		r.Data["archive"] = q.Archive
		r.Data["archiveSha256"] = q.ArchiveSHA
		if q.ArchiveSHA != "" {
			r.pass("candidate-sha256", q.ArchiveSHA)
		}
		if runtime.GOOS != "windows" {
			r.pass("portable-handoff", "clean portable qualification PASS; Windows runtime/focused/full gates remain pending")
			qualificationRecord(r, root, cfg, &q, "runtime", "PENDING-WINDOWS")
			r.Data["qualification"] = q
			return r
		}
		rl := runtimeLifecycle(root, cfg, "restart", RuntimeLaunchOptions{}, jsonOut)
		if startup, ok := rl.Data["runtimeStartup"].(*Report); ok && !jsonOut {
			printQualificationChecks("Runtime readiness", startup)
		}
		if rl.Status == "FAIL" {
			// Startup readiness is intentionally lightweight: Workbench health,
			// exact native identity and mobile pairing only. Authoritative run
			// history is checked later, after regressions, so a cold workspace-wide
			// history scan cannot block regression startup.
			var ready *Report
			for attempt := 1; attempt <= 4; attempt++ {
				time.Sleep(2 * time.Second)
				ready = runtimeStartupCheck(root, cfg, overrideURL)
				if !jsonOut {
					if ready.Status == "FAIL" {
						fmt.Printf("[QUALIFY] Runtime readiness retry %d/4 - %s\n", attempt, readinessFailureDetail(ready))
					} else {
						fmt.Printf("[QUALIFY] Runtime readiness retry %d/4 - PASS\n", attempt)
						printQualificationChecks("Runtime readiness", ready)
					}
				}
				if ready.Status != "FAIL" {
					break
				}
			}
			if ready == nil || ready.Status == "FAIL" {
				r.fail("runtime-restart", "runtime restart/lightweight readiness failed after retry window: "+readinessFailureDetail(ready))
				r.Data["runtimeRestart"] = rl
				r.Data["runtimeRetry"] = ready
				qualificationRecord(r, root, cfg, &q, "runtime", "FAIL")
				return r
			}
			r.pass("runtime-restart", "Workbench/native/mobile ready after lightweight readiness retry")
		} else {
			r.pass("runtime-restart", "Workbench/native/mobile ready (lightweight startup gate)")
		}
		qualificationRecord(r, root, cfg, &q, "runtime", "PASS")

		if !jsonOut {
			fmt.Printf("[QUALIFY] Focused regression START - %s\n", cfg.FocusedRegression)
		}
		focusedStarted := time.Now()
		fr := regressionCommand(root, cfg, "focused", overrideURL)
		if fr.Status == "FAIL" {
			if !jsonOut {
				fmt.Printf("[QUALIFY] Focused regression FAILED - elapsed=%s\n", time.Since(focusedStarted).Round(time.Millisecond))
			}
			r.fail("focused", reportFailureDetail(fr, "focused regression failed"))
			r.Data["focused"] = fr
			qualificationRecord(r, root, cfg, &q, "focused", "FAIL")
			return r
		}
		if !jsonOut {
			fmt.Printf("[QUALIFY] Focused regression COMPLETE - PASS run=%v commands=%v elapsed=%s\n", fr.Data["runId"], fr.Data["commands"], time.Since(focusedStarted).Round(time.Millisecond))
		}
		r.pass("focused", fmt.Sprintf("PASS run=%v commands=%v", fr.Data["runId"], fr.Data["commands"]))
		qualificationRecord(r, root, cfg, &q, "focused", "PASS")

		if !jsonOut {
			fmt.Printf("[QUALIFY] Full Regression START - %s\n", cfg.FullRegression)
		}
		fullStarted := time.Now()
		full := regressionCommand(root, cfg, "full", overrideURL)
		if full.Status == "FAIL" {
			if !jsonOut {
				fmt.Printf("[QUALIFY] Full Regression FAILED - elapsed=%s\n", time.Since(fullStarted).Round(time.Millisecond))
			}
			r.fail("full", reportFailureDetail(full, "Full Regression failed"))
			r.Data["full"] = full
			qualificationRecord(r, root, cfg, &q, "full", "FAIL")
			return r
		}
		if !jsonOut {
			fmt.Printf("[QUALIFY] Full Regression COMPLETE - PASS run=%v commands=%v elapsed=%s\n", full.Data["runId"], full.Data["commands"], time.Since(fullStarted).Round(time.Millisecond))
		}
		r.pass("full", fmt.Sprintf("PASS run=%v commands=%v", full.Data["runId"], full.Data["commands"]))
		qualificationRecord(r, root, cfg, &q, "full", "PASS")

		if !jsonOut {
			fmt.Printf("[QUALIFY] Authoritative run-history gate START\n")
		}
		history := runtimeHistoryCheck(root, cfg, overrideURL)
		if !jsonOut {
			printQualificationChecks("Authoritative run-history", history)
		}
		if history.Status == "FAIL" {
			r.fail("run-history", reportFailureDetail(history, "authoritative run-history gate failed"))
			r.Data["runHistory"] = history
			qualificationRecord(r, root, cfg, &q, "run-history", "FAIL")
			return r
		}
		if !jsonOut {
			fmt.Printf("[QUALIFY] Authoritative run-history gate COMPLETE - PASS\n")
		}
		r.pass("run-history", "authoritative endpoint PASS after regressions")
		qualificationRecord(r, root, cfg, &q, "run-history", "PASS")
		r.pass("promotable", "fresh qualification PASS on exact current source/archive/runtime")
		qualificationRecord(r, root, cfg, &q, "promotable", "PASS")
		r.Data["qualification"] = q
		return r
	}

	if mode == "runtime" || mode == "focused" || mode == "full" {
		if !runDoctorPreflight() {
			return r
		}
		switch mode {
		case "runtime":
			rr := runtimeCheck(root, cfg, overrideURL)
			if rr.Status == "FAIL" {
				r.fail("runtime", "Workbench/runtime readiness failed")
				r.Data["runtime"] = rr
				qualificationRecord(r, root, cfg, &q, "runtime", "FAIL")
				return r
			}
			r.pass("runtime", "Workbench/native/mobile + authoritative history readiness PASS")
			qualificationRecord(r, root, cfg, &q, "runtime", "PASS")
		case "focused", "full":
			rr := regressionCommand(root, cfg, mode, overrideURL)
			if rr.Status == "FAIL" {
				r.fail(mode, reportFailureDetail(rr, "regression failed"))
				r.Data[mode] = rr
				qualificationRecord(r, root, cfg, &q, mode, "FAIL")
				return r
			}
			r.pass(mode, "regression PASS")
			qualificationRecord(r, root, cfg, &q, mode, "PASS")
		}
		r.Data["qualification"] = q
		return r
	}

	if !runDoctorPreflight() {
		return r
	}
	if mode == "preflight" {
		r.pass("handoff", "no build requested")
		r.Data["qualification"] = q
		return r
	}

	b := buildCommand(root, cfg, "debug", jsonOut)
	if b.Status == "FAIL" {
		r.fail("build", "debug build failed")
		r.Data["build"] = b
		qualificationRecord(r, root, cfg, &q, "build", "FAIL")
		return r
	}
	r.pass("build", "debug PASS")
	qualificationRecord(r, root, cfg, &q, "build", "PASS")

	t := testCommand(root, cfg, "debug", jsonOut)
	if t.Status == "FAIL" {
		r.fail("test", "CTest failed")
		r.Data["test"] = t
		qualificationRecord(r, root, cfg, &q, "test", "FAIL")
		return r
	}
	r.pass("test", "all CTest cases PASS")
	qualificationRecord(r, root, cfg, &q, "test", "PASS")
	if mode == "build" {
		r.Data["qualification"] = q
		return r
	}

	pr := packageCommand(root, cfg, "", jsonOut)
	if pr.Status == "FAIL" {
		r.fail("package", "package/verify/reproducibility failed")
		r.Data["package"] = pr
		qualificationRecord(r, root, cfg, &q, "package", "FAIL")
		return r
	}
	r.pass("package", "deterministic manifest + ZIP + exact verification PASS")
	qualificationRecord(r, root, cfg, &q, "package", "PASS")
	q.Archive, _ = pr.Data["zip"].(string)
	q.ArchiveSHA, _ = pr.Data["sha256"].(string)
	saveQualificationState(root, cfg, q)
	r.Data["package"] = pr.Data
	if mode == "package" {
		r.Data["qualification"] = q
		return r
	}

	if mode == "all" {
		rr := runtimeStartupCheck(root, cfg, overrideURL)
		if rr.Status == "FAIL" {
			r.fail("runtime", "Workbench lightweight readiness failed; start Workbench from this exact extraction and rerun qualify --all/--runtime")
			r.Data["runtime"] = rr
			qualificationRecord(r, root, cfg, &q, "runtime", "FAIL")
			return r
		}
		r.pass("runtime", "Workbench/native/mobile startup readiness PASS")
		qualificationRecord(r, root, cfg, &q, "runtime", "PASS")
		fr := regressionCommand(root, cfg, "focused", overrideURL)
		if fr.Status == "FAIL" {
			r.fail("focused", reportFailureDetail(fr, "focused regression failed"))
			r.Data["focused"] = fr
			qualificationRecord(r, root, cfg, &q, "focused", "FAIL")
			return r
		}
		r.pass("focused", "PASS")
		qualificationRecord(r, root, cfg, &q, "focused", "PASS")
		full := regressionCommand(root, cfg, "full", overrideURL)
		if full.Status == "FAIL" {
			r.fail("full", reportFailureDetail(full, "Full Regression failed"))
			r.Data["full"] = full
			qualificationRecord(r, root, cfg, &q, "full", "FAIL")
			return r
		}
		r.pass("full", "PASS")
		qualificationRecord(r, root, cfg, &q, "full", "PASS")
		history := runtimeHistoryCheck(root, cfg, overrideURL)
		if history.Status == "FAIL" {
			r.fail("run-history", reportFailureDetail(history, "authoritative run-history gate failed"))
			r.Data["runHistory"] = history
			qualificationRecord(r, root, cfg, &q, "run-history", "FAIL")
			return r
		}
		r.pass("run-history", "authoritative endpoint PASS after regressions")
		qualificationRecord(r, root, cfg, &q, "run-history", "PASS")
		r.pass("promotable", "all chqdev release gates PASS on the current exact extraction")
		qualificationRecord(r, root, cfg, &q, "promotable", "PASS")
	}
	r.Data["qualification"] = q
	return r
}

func statusCommand(root string, cfg Config) *Report {
	r := newReport("status", cfg.Release)
	v, e := readNativeVersion(root, cfg)
	if e != nil {
		r.fail("version", e.Error())
	} else {
		r.pass("version", v)
	}
	for _, p := range []string{"out/build/x64-Debug", "out/build/x64-Release", "out/build/cpu-Debug", "out/build/linux-Debug", "out/build/linux-Release"} {
		if dirExists(filepath.Join(root, filepath.FromSlash(p))) {
			r.pass("build-dir", p)
		}
	}
	m := filepath.Join(root, packageManifestJSON)
	if fileExists(m) {
		r.pass("manifest", filepath.Base(m))
	} else {
		r.warn("manifest", "current manifest not generated")
	}
	if _, e = os.Stat(filepath.Join(root, "dist")); e == nil {
		r.pass("dist", "present")
	}
	q := loadQualificationState(root, cfg)
	if len(q.Stages) > 0 {
		r.pass("qualification-state", qualificationStatePath(root, cfg))
		r.Data["qualification"] = q
	} else {
		r.warn("qualification-state", "no qualification stages recorded yet")
	}
	r.Data["root"] = root
	return r
}

var _ = fmt.Sprintf
