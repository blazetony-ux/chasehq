package main

import (
	"encoding/json"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestRC46GhidraWorkbenchApiParityAndSafety(t *testing.T) {
	root := filepath.Clean(filepath.Join("..", ".."))
	web, err := os.ReadFile(filepath.Join(root, "Start-ChaseHQWeb.ps1"))
	if err != nil {
		t.Fatal(err)
	}
	var schema struct {
		Actions map[string]any `json:"actions"`
	}
	raw, err := os.ReadFile(filepath.Join(root, "research", "schema", "api-actions.json"))
	if err != nil {
		t.Fatal(err)
	}
	if err = json.Unmarshal(raw, &schema); err != nil {
		t.Fatal(err)
	}
	for _, name := range []string{"ghidra.status", "ghidra.start", "ghidra.results", "ghidra.import"} {
		if _, ok := schema.Actions[name]; !ok {
			t.Fatalf("schema missing %s", name)
		}
		if !strings.Contains(string(web), "'"+name+"'=[ordered]@{") {
			t.Fatalf("PowerShell schema missing %s", name)
		}
		if !strings.Contains(string(web), "'"+name+"'{") {
			t.Fatalf("PowerShell dispatcher missing %s", name)
		}
	}
	for _, route := range []string{"/api/v1/ghidra/status", "/api/v1/ghidra/start", "/api/v1/ghidra/results", "/api/v1/ghidra/import"} {
		if !strings.Contains(string(web), "$path -eq '"+route+"'") {
			t.Fatalf("HTTP route absent: %s", route)
		}
	}
	for _, require := range []string{"function Start-GhidraJob", "function Get-GhidraJobStatus", "function Get-GhidraResults", "function Import-GhidraJobCrosswalks", "Start-Process -FilePath 'powershell.exe'", "romExpectedSha256", "emulatorUnchanged=$true", "ghidraRun", "ghidraStatus", "ghidraImport"} {
		if !strings.Contains(string(web), require) {
			t.Fatalf("Workbench Ghidra contract missing: %s", require)
		}
	}
	for _, file := range []string{"Run-ChaseHQ-Ghidra.ps1", "Run-WorkbenchGhidraJob.ps1", "ChaseHQSeed68000.py", "ChaseHQExport68000.py"} {
		if _, err := os.Stat(filepath.Join(root, "research", "ghidra-automation", file)); err != nil {
			t.Fatal(err)
		}
	}
	// No arbitrary executable or script path via the API dispatcher; only fixed packaged worker.
	if strings.Contains(string(web), "Start-GhidraJob ([string]$a[") {
		t.Fatal("job start accepts arbitrary API arguments")
	}
}
