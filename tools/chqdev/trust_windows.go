//go:build windows

package main

import (
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"syscall"
)

func isNoADS(err error) bool {
	if err == nil {
		return false
	}
	if os.IsNotExist(err) {
		return true
	}
	var pe *os.PathError
	if errors.As(err, &pe) {
		if errno, ok := pe.Err.(syscall.Errno); ok {
			// FILE_NOT_FOUND, PATH_NOT_FOUND, INVALID_NAME: no stream/no ADS on this filesystem.
			return errno == 2 || errno == 3 || errno == 50 || errno == 123
		}
	}
	return false
}

func hasZoneIdentifier(path string) (bool, error) {
	stream := path + ":Zone.Identifier"
	f, err := os.Open(stream)
	if err == nil {
		_ = f.Close()
		return true, nil
	}
	if isNoADS(err) {
		return false, nil
	}
	return false, err
}

func trustScan(root string, cfg Config, repair bool) TrustResult {
	tr := TrustResult{Applicable: true}
	files, err := collectTrustTargets(root, cfg)
	if err != nil {
		tr.Errors = append(tr.Errors, err.Error())
		return tr
	}
	tr.Scanned = len(files)
	for _, p := range files {
		marked, err := hasZoneIdentifier(p)
		if err != nil {
			tr.Errors = append(tr.Errors, fmt.Sprintf("%s: %v", p, err))
			continue
		}
		if !marked {
			continue
		}
		tr.Marked++
		if repair {
			if err := os.Remove(p + ":Zone.Identifier"); err != nil && !isNoADS(err) {
				tr.Errors = append(tr.Errors, fmt.Sprintf("%s: remove Zone.Identifier: %v", p, err))
				continue
			}
			tr.Removed++
		}
	}
	for _, p := range files {
		marked, err := hasZoneIdentifier(p)
		if err != nil {
			continue
		}
		if marked {
			tr.Remaining++
		}
	}
	return tr
}

func trustSelfTest(root string) error {
	dir := filepath.Join(root, "out", "chqdev", "trust-selftest")
	if err := os.MkdirAll(dir, 0755); err != nil {
		return err
	}
	defer os.RemoveAll(dir)
	p := filepath.Join(dir, "fixture.txt")
	if err := os.WriteFile(p, []byte("chqdev trust self-test\n"), 0644); err != nil {
		return err
	}
	stream := p + ":Zone.Identifier"
	if err := os.WriteFile(stream, []byte("[ZoneTransfer]\r\nZoneId=3\r\n"), 0644); err != nil {
		return fmt.Errorf("create Zone.Identifier fixture: %w", err)
	}
	marked, err := hasZoneIdentifier(p)
	if err != nil || !marked {
		return fmt.Errorf("Zone.Identifier fixture not detected: marked=%v err=%v", marked, err)
	}
	if err := os.Remove(stream); err != nil {
		return fmt.Errorf("remove Zone.Identifier fixture: %w", err)
	}
	marked, err = hasZoneIdentifier(p)
	if err != nil {
		return fmt.Errorf("verify Zone.Identifier removal: %w", err)
	}
	if marked {
		return fmt.Errorf("Zone.Identifier fixture still present after removal")
	}
	return nil
}
