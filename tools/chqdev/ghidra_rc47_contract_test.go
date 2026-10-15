package main

import (
	"encoding/json"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestRC47GhidraWorkspaceAndWaitContract(t *testing.T) {
	root := filepath.Clean(filepath.Join("..", ".."))
	web, e := os.ReadFile(filepath.Join(root, "Start-ChaseHQWeb.ps1"))
	if e != nil {
		t.Fatal(e)
	}
	txt := string(web)
	for _, required := range []string{
		`data-tab="rea"`, `id="staticAnalysisPanel"`, `reaFilter`,
		`function Wait-GhidraJob`, `function Attach-GhidraRunEvidence`,
		`function Get-GhidraResults`, `GetFileNameWithoutExtension($evidenceZip)`,
		`async function waitForGhidra`, `postAuthoritativeClientRecord(item,out)`,
		`'ghidra.wait'`, `'ghidra.evidence.attach'`,
		`/api/v1/ghidra/wait`, `jythonReady`, `javaReady`,
	} {
		if !strings.Contains(txt, required) {
			t.Errorf("missing: %s", required)
		}
	}
	if n := strings.Count(txt, `id="reaCase"`); n != 1 {
		t.Fatalf("duplicated REA controls: %d", n)
	}
	if n := strings.Count(txt, `id="staticAnalysisPanel"`); n != 1 {
		t.Fatalf("missing dedicated REA panel: %d", n)
	}
	var schema struct {
		Actions map[string]any `json:"actions"`
	}
	b, e := os.ReadFile(filepath.Join(root, "research/schema/api-actions.json"))
	if e != nil {
		t.Fatal(e)
	}
	if e = json.Unmarshal(b, &schema); e != nil {
		t.Fatal(e)
	}
	for _, a := range []string{"ghidra.wait", "ghidra.evidence.attach"} {
		if _, ok := schema.Actions[a]; !ok {
			t.Errorf("missing schema: %s", a)
		}
	}
}

func TestRC47GhidraScriptsCompileWithFinallyAndStableCommandCount(t *testing.T) {
	root := filepath.Clean(filepath.Join("..", ".."))
	for _, relative := range []string{"regression/regression-v0670-rc47.chqscript", "native-reconstruction/run036-ghidra-one-script.chqscript"} {
		b, err := os.ReadFile(filepath.Join(root, "research", "scripts", filepath.FromSlash(relative)))
		if err != nil {
			t.Fatal(err)
		}
		src := string(b)
		cmds, err := compileScriptLines(strings.Split(src, "\n"), scriptDefaultVars(src))
		if err != nil {
			t.Fatalf("%s: %v", relative, err)
		}
		if len(cmds) < 1 || countScriptItems(cmds) > 40 {
			t.Fatalf("%s invalid structural commands: %d", relative, countScriptItems(cmds))
		}
		if !strings.Contains(src, "finally") || !strings.Contains(src, "ghidra") {
			t.Fatalf("%s missing mandatory cleanup or instrumentation", relative)
		}
	}
}
