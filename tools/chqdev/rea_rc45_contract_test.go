package main

import (
	"encoding/json"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

// Release-source contract guards the three public surfaces against API drift.
// Windows qualification still must exercise the actual PowerShell host and SDL.
func TestRC45REAWorkbenchScriptAndSchemaParity(t *testing.T) {
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
	if err := json.Unmarshal(raw, &schema); err != nil {
		t.Fatal(err)
	}
	for _, name := range []string{"rea.reports", "rea.crosswalk", "rea.functions", "rea.function.inspect"} {
		if _, ok := schema.Actions[name]; !ok {
			t.Fatalf("schema missing %s", name)
		}
		if !strings.Contains(string(web), "'"+name+"'=[ordered]@{") {
			t.Fatalf("PowerShell API schema missing %s", name)
		}
		if !strings.Contains(string(web), "'"+name+"'{") {
			t.Fatalf("Script action dispatcher missing %s", name)
		}
		if !strings.Contains(string(web), "'"+name+"'") {
			t.Fatalf("Script allowlist missing %s", name)
		}
	}
	for _, route := range []string{"/api/v1/rea/import", "/api/v1/rea/crosswalk", "/api/v1/rea/reports", "/api/v1/rea/functions"} {
		if !strings.Contains(string(web), "$path -eq '"+route+"'") {
			t.Fatalf("REA REST endpoint missing %s", route)
		}
	}
	for _, essential := range []string{"Native Reconstruction Lab", "Static Analysis / REA", "function Import-ReaResearch", "function Read-ReaResearchReport", "function Resolve-ReaRecordName", "function Invoke-ReaResearchCrosswalk", "research\\rea\\imports", "evidence\\rea-crosswalks"} {
		if !strings.Contains(string(web), essential) {
			t.Fatalf("REA Workbench essential absent: %s", essential)
		}
	}
}

func TestRC45HeadlessCheckpointCannotBeSilentlyIgnored(t *testing.T) {
	raw, e := os.ReadFile(filepath.Join("..", "..", "src", "runtime_main.cpp"))
	if e != nil {
		t.Fatal(e)
	}
	text := string(raw)
	for _, must := range []string{
		"runtime.load_checkpoint(options.load_checkpoint)",
		"for (unsigned frame = start_frame; frame < options.frames",
		"[CHECKPOINT LOAD] frame=",
		"run_manifest.json",
		"checkpointLoadVerified",
		"framesScheduled",
		"run_phase.txt",
	} {
		if !strings.Contains(text, must) {
			t.Fatalf("standalone runtime missing checkpoint contract: %s", must)
		}
	}
}
