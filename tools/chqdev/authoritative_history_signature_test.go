package main

import (
	"os"
	"path/filepath"
	"regexp"
	"strings"
	"testing"
)

// The RC4.3.1 release unexpectedly failed Windows CTest after the production
// Get-AuthoritativeScriptRuns signature acquired an optional onlySession parameter.
// Reject future CTest/source signature drift before handing off a Windows archive.
func TestAuthoritativeHistoryCTestTracksFunctionSignature(t *testing.T) {
	root := filepath.Join("..", "..")
	web, err := os.ReadFile(filepath.Join(root, "Start-ChaseHQWeb.ps1"))
	if err != nil {
		t.Fatal(err)
	}
	test, err := os.ReadFile(filepath.Join(root, "tests", "authoritative_history_tests.ps1"))
	if err != nil {
		t.Fatal(err)
	}
	match := regexp.MustCompile(`function\s+Get-AuthoritativeScriptRuns\s*\([^)]*\)\s*\{`).FindString(string(web))
	if match == "" {
		t.Fatal("authoritative run-history function missing")
	}
	if !strings.Contains(match, "onlySession") {
		t.Fatal("current-session fast path disappeared")
	}
	if !strings.Contains(string(test), `Get-AuthoritativeScriptRuns\s*\([^)]*\)`) {
		t.Fatal("Windows CTest uses obsolete zero-argument function extractor")
	}
	if !strings.Contains(string(test), "Get-AuthoritativeScriptRuns 'active'") ||
		!strings.Contains(string(test), "History was truncated before filtering") ||
		!strings.Contains(string(test), "Pagination does not select the newest matching records") {
		t.Fatal("Windows CTest must preserve all-session, pagination and current-session coverage")
	}
}
