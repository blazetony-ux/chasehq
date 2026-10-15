package main

import (
	"os"
	"path/filepath"
	"strings"
	"testing"
)

// Prevent the long-running, cross-release script-history scan from regressing into
// the Workbench startup path. That scan blocked a serialized PowerShell listener,
// causing unrelated /api/health and /api/v1/status requests to time out on Windows.
func TestWorkbenchStartupCannotAwaitHistoricalEvidenceScan(t *testing.T) {
	root := filepath.Join("..", "..")
	body, err := os.ReadFile(filepath.Join(root, "Start-ChaseHQWeb.ps1"))
	if err != nil {
		t.Fatal(err)
	}
	s := string(body)
	start := strings.LastIndex(s, "  await loadScriptLibrary();")
	end := strings.Index(s[start:], "})();\n</script>")
	if start < 0 || end < 0 {
		t.Fatal("cannot locate Workbench startup hydration")
	}
	startup := s[start : start+end]
	if strings.Contains(startup, "refreshAuthoritativeRunHistory(") || strings.Contains(startup, "/api/v1/script/runs") {
		t.Fatal("historical run scan is part of the readiness-critical Workbench startup")
	}
	if !strings.Contains(s, "if(name==='script'&&!runHistoryLoaded)") {
		t.Fatal("history must remain accessible on demand when Script Console opens")
	}
	if !strings.Contains(s, "load().catch(e=>$('out').textContent=e.message)") || strings.Contains(s, "Promise.all([load(),historyBootstrap()])") {
		t.Fatal("mobile console must not start a global history scan during bootstrap")
	}
}

func TestWorkbenchRunHistoryListDoesNotParseLargeBundleManifests(t *testing.T) {
	body, err := os.ReadFile(filepath.Join("..", "..", "Start-ChaseHQWeb.ps1"))
	if err != nil {
		t.Fatal(err)
	}
	s := string(body)
	start := strings.Index(s, "function Get-AuthoritativeScriptRuns(")
	end := strings.Index(s[start:], "function Get-ForensicTimelines(")
	if start < 0 || end < 0 {
		t.Fatal("run-list implementation missing")
	}
	fn := s[start : start+end]
	if strings.Contains(fn, "Get-Content $manifestPath") || strings.Contains(fn, "foreach($entry in @($bm.files))") {
		t.Fatal("run list must not expand every historical bundle-manifest.json")
	}
	if !strings.Contains(fn, "if($onlySession -and $session.Name -ne $onlySession){continue}") || !strings.Contains(fn, "$sessionRoots=if($onlySession") {
		t.Fatal("current session queries must avoid global release scans")
	}
	if !strings.Contains(s, "$final['artifactCount']=$count;$final['artifactTypes']=$types") {
		t.Fatal("completed runs must persist compact artifact summary in metadata")
	}
	if strings.Index(s, "$final['artifactCount']=$count;$final['artifactTypes']=$types") > strings.Index(s, "$fileRecords=@(Get-RunBundleFileRecords $runRoot @('bundle.zip'") {
		t.Fatal("run-metadata summary must be committed before file manifest hash generation")
	}
}
