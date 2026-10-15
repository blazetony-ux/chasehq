package main

import (
	"fmt"
	"io/fs"
	"path/filepath"
	"runtime"
	"sort"
)

type TrustResult struct {
	Applicable bool     `json:"applicable"`
	Scanned    int      `json:"scanned"`
	Marked     int      `json:"marked"`
	Removed    int      `json:"removed"`
	Remaining  int      `json:"remaining"`
	Errors     []string `json:"errors,omitempty"`
}

func collectTrustTargets(root string, cfg Config) ([]string, error) {
	var files []string
	err := filepath.WalkDir(root, func(p string, d fs.DirEntry, err error) error {
		if err != nil {
			return err
		}
		rel, err := filepath.Rel(root, p)
		if err != nil {
			return err
		}
		if rel == "." {
			return nil
		}
		if d.IsDir() {
			if isExcluded(rel, cfg) {
				return filepath.SkipDir
			}
			return nil
		}
		files = append(files, p)
		return nil
	})
	sort.Strings(files)
	return files, err
}

func trustCommand(root string, cfg Config, repair bool) *Report {
	cmd := "trust"
	if !repair {
		cmd = "trust check"
	}
	r := newReport(cmd, cfg.Release)
	tr := trustScan(root, cfg, repair)
	r.Data["trust"] = tr
	if !tr.Applicable {
		r.pass("windows-trust", "not applicable on "+runtime.GOOS)
		return r
	}
	if len(tr.Errors) > 0 {
		r.fail("windows-trust", fmt.Sprintf("%d error(s): %v", len(tr.Errors), tr.Errors))
		return r
	}
	if repair {
		if tr.Remaining != 0 {
			r.fail("windows-trust", fmt.Sprintf("%d marked file(s) remain after repair", tr.Remaining))
			return r
		}
		r.pass("windows-trust", fmt.Sprintf("%d project files scanned; %d Zone.Identifier stream(s) removed; 0 remaining", tr.Scanned, tr.Removed))
	} else if tr.Marked > 0 {
		r.warn("windows-trust", fmt.Sprintf("%d/%d project files carry Zone.Identifier", tr.Marked, tr.Scanned))
	} else {
		r.pass("windows-trust", fmt.Sprintf("%d project files scanned; no Zone.Identifier streams", tr.Scanned))
	}
	return r
}
