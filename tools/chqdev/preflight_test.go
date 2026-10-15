package main

import (
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestHandoverContractLineEndings(t *testing.T) {
	v := "0.66.9.0-RC3.4.4"
	lf := "# Continue from v" + v + "\n## Authoritative continuation\nA\n## Immediate next task\nB\n"
	for name, s := range map[string]string{"lf": lf, "crlf": strings.ReplaceAll(lf, "\n", "\r\n"), "cr": strings.ReplaceAll(lf, "\n", "\r")} {
		if !handoverContract(s, v) {
			t.Fatalf("%s valid fixture rejected", name)
		}
	}
	if handoverContract(strings.ReplaceAll(lf, "## Authoritative continuation\nA\n", ""), v) {
		t.Fatal("missing continuation accepted")
	}
	if handoverContract(strings.ReplaceAll(lf, "v"+v, "v0.0.0-stale"), v) {
		t.Fatal("stale version accepted")
	}
}

func TestWindowsASCIIDiagnostics(t *testing.T) {
	root := t.TempDir()
	os.WriteFile(filepath.Join(root, "chqdev.json"), []byte(`{"schema":"chqdev-config-v1"}`), 0644)
	os.WriteFile(filepath.Join(root, "bad.ps1"), []byte("Write-Host 'x · y'\n"), 0644)
	cfg := Config{WindowsASCIIExtensions: []string{".ps1"}}
	n, bad := scanWindowsASCII(root, cfg)
	if n != 1 || len(bad) != 1 {
		t.Fatalf("n=%d bad=%v", n, bad)
	}
	if !strings.Contains(bad[0], "U+00B7") {
		t.Fatalf("diagnostic=%s", bad[0])
	}
}

func TestNormalizeReleaseText(t *testing.T) {
	if got := normalizeReleaseText("a\r\nb\rc\n"); got != "a\nb\nc\n" {
		t.Fatalf("got %q", got)
	}
}

func TestReleaseProcessContract(t *testing.T) {
	root := t.TempDir()
	good := `# v0.66.9.1
chqdev is the de facto and authoritative build/release process.
Use chqdev qualify --fresh.
There are zero tolerated CTest failures; any CTest failure is fatal.
Deterministic exact archive verification is mandatory.
Focused regression and Full Regression must pass.
PowerShell/BAT release tooling is compatibility/fallback only.
`
	if e := os.WriteFile(filepath.Join(root, "good.md"), []byte(good), 0644); e != nil {
		t.Fatal(e)
	}
	cfg := Config{ReleaseProcessDocs: []string{"good.md"}}
	if e := checkReleaseProcessContract(root, cfg); e != nil {
		t.Fatalf("valid contract rejected: %v", e)
	}

	bad := good + "\nThe canonical Windows build path is Build-Debug.bat.\n"
	if e := os.WriteFile(filepath.Join(root, "bad.md"), []byte(bad), 0644); e != nil {
		t.Fatal(e)
	}
	cfg.ReleaseProcessDocs = []string{"bad.md"}
	if e := checkReleaseProcessContract(root, cfg); e == nil {
		t.Fatal("legacy canonical claim accepted")
	}
}

func TestReleaseRootHygiene(t *testing.T) {
	root := t.TempDir()
	for _, name := range []string{"CHANGELOG.md", "VALIDATION.md", "chqdev.bat"} {
		if err := os.WriteFile(filepath.Join(root, name), []byte("ok\n"), 0644); err != nil {
			t.Fatal(err)
		}
	}
	if err := checkReleaseRootHygiene(root); err != nil {
		t.Fatalf("clean root rejected: %v", err)
	}
	if err := os.WriteFile(filepath.Join(root, "Qualify-0.66.9.2-RC3.bat"), []byte("obsolete\n"), 0644); err != nil {
		t.Fatal(err)
	}
	if err := checkReleaseRootHygiene(root); err == nil || !strings.Contains(err.Error(), "Qualify-0.66.9.2-RC3.bat") {
		t.Fatalf("version-specific wrapper accepted: %v", err)
	}
}

func TestWindowsCTestIdentityContract(t *testing.T) {
	cmake := `add_test(NAME cli_evidence_tests COMMAND powershell -File tests/cli_evidence_tests.ps1 -Exe exe -ExpectedBuild "${CHQ_RELEASE_VERSION}")`
	good := `param([string]$Exe,[string]$ExpectedBuild)
if($m.build -ne $ExpectedBuild -or $m.startFrame -ne 2352){throw 'bad'}`
	if err := windowsCTestIdentityContract(cmake, good); err != nil {
		t.Fatalf("valid contract rejected: %v", err)
	}
	stale := `param([string]$Exe,[string]$ExpectedBuild)
if($m.build -ne '0.66.9.2'){throw 'bad'}`
	if err := windowsCTestIdentityContract(cmake, stale); err == nil {
		t.Fatal("hard-coded evidence build accepted")
	}
	if err := windowsCTestIdentityContract(`add_test(NAME cli_evidence_tests COMMAND powershell -File tests/cli_evidence_tests.ps1)`, good); err == nil {
		t.Fatal("missing CMake ExpectedBuild wiring accepted")
	}
}

func TestFocusedRegressionIdentity(t *testing.T) {
	root := t.TempDir()
	p := filepath.Join(root, "focused.chqscript")
	if err := os.WriteFile(p, []byte("# regression-version: 0.66.9.3-RC5\napi release.assert-version\n"), 0644); err != nil {
		t.Fatal(err)
	}
	cfg := Config{Release: "0.66.9.3-RC5", FocusedRegression: "focused.chqscript"}
	if err := checkFocusedRegressionIdentity(root, cfg); err != nil {
		t.Fatalf("valid focused identity rejected: %v", err)
	}
	cfg.Release = "0.66.9.3-RC6"
	if err := checkFocusedRegressionIdentity(root, cfg); err == nil {
		t.Fatal("stale focused regression accepted")
	}
}

func TestCMakeProjectVersionConsistency(t *testing.T) {
	root := t.TempDir()
	if err := os.WriteFile(filepath.Join(root, "CMakeLists.txt"), []byte("project(ChaseHQNative VERSION 0.66.9.3 LANGUAGES C CXX)\n"), 0644); err != nil {
		t.Fatal(err)
	}
	cfg := Config{Release: "0.66.9.3-RC5"}
	if err := checkCMakeProjectVersion(root, cfg); err != nil {
		t.Fatalf("valid CMake version rejected: %v", err)
	}
	cfg.Release = "0.66.9.5-RC1.1"
	if err := checkCMakeProjectVersion(root, cfg); err == nil {
		t.Fatal("stale CMake project version accepted")
	}
}

func TestBuildIdentityFiles(t *testing.T) {
	root := t.TempDir()
	if err := os.WriteFile(filepath.Join(root, "a.json"), []byte(`{"build":"0.66.9.3-RC5"}`), 0644); err != nil {
		t.Fatal(err)
	}
	cfg := Config{Release: "0.66.9.3-RC5", BuildIdentityFiles: []string{"a.json"}}
	if err := checkBuildIdentityFiles(root, cfg); err != nil {
		t.Fatalf("valid build identity rejected: %v", err)
	}
	cfg.Release = "0.66.9.3-RC6"
	if err := checkBuildIdentityFiles(root, cfg); err == nil {
		t.Fatal("stale machine-readable build identity accepted")
	}
}

func TestPackageMetadataContract(t *testing.T) {
	good := Config{
		Release:               "0.66.9.3",
		Baseline:              "0.66.9.2",
		PackageStatus:         "PROMOTED RELEASE: 0.66.9.3; exact-archive qualification applies.",
		PackageSourceBaseline: "Promotion source 0.66.9.3-RC5; previous baseline 0.66.9.2.",
		PackageFeatures:       []string{"feature one", "feature two"},
	}
	if err := checkPackageMetadataContract(good); err != nil {
		t.Fatalf("valid final package metadata rejected: %v", err)
	}
	bad := good
	bad.PackageStatus = "RELEASE CANDIDATE: 0.66.9.3; Windows promotion pending"
	if err := checkPackageMetadataContract(bad); err == nil {
		t.Fatal("final release accepted stale candidate/pending package status")
	}
	bad = good
	bad.PackageFeatures = nil
	if err := checkPackageMetadataContract(bad); err == nil {
		t.Fatal("empty package feature summary accepted")
	}
}
