package main

// RC6.3.14: distinguish a separately registered but INACTIVE CPU-B pair
// from an unknown or malicious second native execution route. Do not hide
// unrelated CPU-B rows or weaken parity for the single-BEQ candidate.
import (
	"fmt"
)

const rc6314PairID = "B:000004D8-000004E0"

func rc6314KnownInactiveCPUBPair(r rc637DispatchRow) bool {
	return r.CPU == "B" && r.ID == rc6314PairID && r.PC == 0x4d8 &&
		r.Outcome == "original-rom" && r.Frame >= 6333 && r.Frame < 6340
}

func rc6314InactiveCPUBAudit(path, activeID string) error {
	rows, err := rc637ReadDispatch(path)
	if err != nil {
		return err
	}
	for _, r := range rows {
		if r.CPU != "B" || r.ID == activeID {
			continue
		}
		if !rc6314KnownInactiveCPUBPair(r) {
			return fmt.Errorf("unreviewed/active CPU-B companion %s PC=0x%x outcome=%s", r.ID, r.PC, r.Outcome)
		}
	}
	return nil
}

// Pair-mode audits apply the inverse constraint: only the original-ROM
// single BEQ may appear alongside paired native attempts. For the active pair,
// the allowed outcomes must reflect the requested original/native/fault mode.
func rc6314PairDispatchAudit(path, mode string, commits, timesliceGuards, rollbacks int64) error {
	rows, err := rc637ReadDispatch(path)
	if err != nil {
		return err
	}
	var pairRows, native, fallbacks, originals int64
	for _, r := range rows {
		if r.CPU != "B" {
			continue
		}
		if r.Frame < 6333 || r.Frame >= 6340 {
			return fmt.Errorf("CPU-B pair dispatch outside replay: %d", r.Frame)
		}
		if r.ID == "B:000004DE-000004E0" && r.PC == 0x4de && r.Outcome == "original-rom" {
			continue
		}
		if r.ID != rc6314PairID || r.PC != 0x4d8 {
			return fmt.Errorf("unexpected active/unknown CPU-B pair companion %s PC=0x%x outcome=%s", r.ID, r.PC, r.Outcome)
		}
		pairRows++
		switch r.Outcome {
		case "native-commit":
			native++
		case "original-fallback":
			fallbacks++
		case "original-rom":
			originals++
		default:
			return fmt.Errorf("unexpected pair outcome %s", r.Outcome)
		}
	}
	if pairRows < 1 {
		return fmt.Errorf("no real CPU-B pair dispatch rows")
	}
	if mode == "original" {
		if native != 0 || fallbacks != 0 || originals != pairRows || commits != 0 || rollbacks != 0 {
			return fmt.Errorf("original pair audit not exclusively ROM: %d/%d/%d", native, fallbacks, originals)
		}
		return nil
	}
	if mode != "generated-native" && mode != "forced-mismatch" {
		return fmt.Errorf("unknown CPU-B pair replay mode %s", mode)
	}
	if originals != 0 || native != commits || fallbacks != timesliceGuards+rollbacks || native+fallbacks != pairRows {
		return fmt.Errorf("pair dispatch %s mismatch commits=%d/%d guards+rollback=%d/%d originals=%d", mode, native, commits, fallbacks, timesliceGuards+rollbacks, originals)
	}
	if mode == "generated-native" && (commits < 1 || rollbacks != 0) {
		return fmt.Errorf("generated pair has no real native commits or unexpected rollback")
	}
	if mode == "forced-mismatch" && rollbacks != 1 {
		return fmt.Errorf("forced pair lacks exactly one original-ROM rollback")
	}
	return nil
}
