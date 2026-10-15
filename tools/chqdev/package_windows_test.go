package main

import (
	"os"
	"path/filepath"
	"strings"
	"testing"
)

// Permanent negative fixtures: do not permit a second missing-chqdev handover.
func TestPackagingRejectsMissingWindowsChqdev(t *testing.T) {
	root := t.TempDir()
	if e := os.WriteFile(filepath.Join(root, "chqdev.bat"), []byte("@echo off\n"), 0644); e != nil {
		t.Fatal(e)
	}
	e := verifyBundledWindowsTooling(root)
	if e == nil || !strings.Contains(e.Error(), "windows-amd64.exe") {
		t.Fatalf("missing tool not rejected: %v", e)
	}
	for _, name := range []string{"chqdev-windows-amd64.exe", "chqdev-windows-arm64.exe"} {
		p := filepath.Join(root, "tools", "chqdev", "bin", name)
		if e := os.MkdirAll(filepath.Dir(p), 0755); e != nil {
			t.Fatal(e)
		}
		if e := os.WriteFile(p, make([]byte, 5000), 0644); e != nil {
			t.Fatal(e)
		}
	}
	if e = verifyBundledWindowsTooling(root); e == nil || !strings.Contains(e.Error(), "corrupt") {
		t.Fatalf("invalid Windows binaries accepted: %v", e)
	}
}
