//go:build !windows

package main

func trustScan(root string, cfg Config, repair bool) TrustResult {
	files, _ := collectTrustTargets(root, cfg)
	return TrustResult{Applicable: false, Scanned: len(files)}
}

func trustSelfTest(root string) error { return nil }
