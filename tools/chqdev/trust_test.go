package main

import (
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestCollectTrustTargetsHonoursExcludedDirs(t *testing.T) {
	root := t.TempDir()
	must := func(p, body string) {
		if err := os.MkdirAll(filepath.Dir(p), 0755); err != nil {
			t.Fatal(err)
		}
		if err := os.WriteFile(p, []byte(body), 0644); err != nil {
			t.Fatal(err)
		}
	}
	must(filepath.Join(root, "Start-ChaseHQ.ps1"), "x")
	must(filepath.Join(root, "docs", "a.md"), "x")
	must(filepath.Join(root, "out", "generated.ps1"), "x")
	must(filepath.Join(root, "SDL", "vendor.ps1"), "x")
	cfg := Config{PackageExcludeDirs: []string{"out", "SDL"}}
	got, err := collectTrustTargets(root, cfg)
	if err != nil {
		t.Fatal(err)
	}
	joined := strings.Join(got, "\n")
	if !strings.Contains(joined, "Start-ChaseHQ.ps1") || !strings.Contains(joined, filepath.Join("docs", "a.md")) {
		t.Fatalf("missing owned files: %v", got)
	}
	if strings.Contains(joined, filepath.Join("out", "generated.ps1")) || strings.Contains(joined, filepath.Join("SDL", "vendor.ps1")) {
		t.Fatalf("excluded files leaked: %v", got)
	}
}

func TestRuntimePowerShellArgs(t *testing.T) {
	o := RuntimeLaunchOptions{Mode: "Gameplay", Checkpoint: "checkpoints/x.chqstate", LocalOnly: true, NoBrowser: true, Port: 37600, HTTPPort: 37680}
	got := strings.Join(runtimePowerShellArgs(`C:\\chq`, "restart", o), " ")
	for _, want := range []string{"-NoProfile", "-ExecutionPolicy Bypass", "Start-ChaseHQ.ps1", "-Restart", "-Mode Gameplay", "-Checkpoint checkpoints/x.chqstate", "-LocalOnly", "-NoBrowser", "-Port 37600", "-HttpPort 37680"} {
		if !strings.Contains(got, want) {
			t.Fatalf("missing %q in %q", want, got)
		}
	}
	stop := strings.Join(runtimePowerShellArgs(`C:\\chq`, "stop", RuntimeLaunchOptions{}), " ")
	if !strings.Contains(stop, "-Stop") {
		t.Fatalf("stop args=%q", stop)
	}
}
