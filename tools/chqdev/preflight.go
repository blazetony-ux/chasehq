package main

import (
	"bufio"
	"bytes"
	"encoding/json"
	"fmt"
	"io/fs"
	"os"
	"path/filepath"
	"regexp"
	"sort"
	"strings"
	"unicode/utf8"
)

func preflight(root string, cfg Config) *Report {
	r := newReport("preflight", cfg.Release)
	if e := trustSelfTest(root); e != nil {
		r.fail("windows-trust-self-test", e.Error())
		return r
	} else {
		r.pass("windows-trust-self-test", "native Zone.Identifier create/detect/remove fixture PASS or not applicable")
	}
	tr := trustScan(root, cfg, true)
	r.Data["windowsTrust"] = tr
	if tr.Applicable {
		if len(tr.Errors) > 0 || tr.Remaining > 0 {
			r.fail("windows-trust", fmt.Sprintf("repair failed: removed=%d remaining=%d errors=%v", tr.Removed, tr.Remaining, tr.Errors))
			return r
		}
		r.pass("windows-trust", fmt.Sprintf("%d project files scanned; %d Zone.Identifier stream(s) removed; 0 remaining", tr.Scanned, tr.Removed))
	} else {
		r.pass("windows-trust", "not applicable on this host")
	}
	actual, e := readNativeVersion(root, cfg)
	if e != nil {
		r.fail("native-version", e.Error())
		return r
	}
	if actual != cfg.Release {
		r.fail("version-consistency", fmt.Sprintf("src/version.h=%s chqdev.json=%s", actual, cfg.Release))
	} else {
		r.pass("version-consistency", actual)
	}
	if e := checkCMakeProjectVersion(root, cfg); e != nil {
		r.fail("cmake-version-consistency", e.Error())
	} else {
		r.pass("cmake-version-consistency", "CMake project base version matches current release")
	}
	if e := checkFocusedRegressionIdentity(root, cfg); e != nil {
		r.fail("focused-regression-identity", e.Error())
	} else {
		r.pass("focused-regression-identity", cfg.FocusedRegression+" -> "+cfg.Release)
	}
	if e := checkBuildIdentityFiles(root, cfg); e != nil {
		r.fail("machine-build-identity", e.Error())
	} else {
		r.pass("machine-build-identity", fmt.Sprintf("%d configured files match %s", len(cfg.BuildIdentityFiles), cfg.Release))
	}
	if e := checkWindowsCTestIdentityContract(root); e != nil {
		r.fail("windows-ctest-identity-contract", e.Error())
	} else {
		r.pass("windows-ctest-identity-contract", "CLI evidence CTest receives authoritative CHQ_RELEASE_VERSION; no hard-coded build assertion")
	}
	asciiCount, bad := scanWindowsASCII(root, cfg)
	if len(bad) > 0 {
		r.fail("windows-script-ascii", strings.Join(bad, "; "))
	} else {
		r.pass("windows-script-ascii", fmt.Sprintf("%d files", asciiCount))
	}
	jc, jerrs := scanJSON(root, cfg)
	if len(jerrs) > 0 {
		r.fail("json-parse", strings.Join(jerrs, "; "))
	} else {
		r.pass("json-parse", fmt.Sprintf("%d files", jc))
	}
	if e := checkHandover(root, cfg, actual); e != nil {
		r.fail("handover-contract", e.Error())
	} else {
		r.pass("handover-contract", "current version + required sections; LF/CRLF/CR invariant")
	}
	if e := selfTestHandover(actual); e != nil {
		r.fail("handover-self-test", e.Error())
	} else {
		r.pass("handover-self-test", "LF/CRLF/CR valid; missing/stale fixtures rejected")
	}
	sc, cc, e := checkScriptCatalog(root, cfg)
	if e != nil {
		r.fail("script-catalog", e.Error())
	} else {
		r.pass("script-catalog", fmt.Sprintf("%d scripts / %d exact rows", sc, cc))
	}
	if e := checkCurrentDocs(root, cfg, actual); e != nil {
		r.fail("current-docs", e.Error())
	} else {
		r.pass("current-docs", fmt.Sprintf("%d documents identify %s", len(cfg.CurrentDocs), actual))
	}
	if e := checkGuardDocs(root, cfg); e != nil {
		r.fail("release-guard-docs", e.Error())
	} else {
		r.pass("release-guard-docs", "ASCII + LF/CRLF release rules retained")
	}
	if e := checkBuildProcessAuthority(root, cfg); e != nil {
		r.fail("build-release-authority", e.Error())
	} else {
		r.pass("build-release-authority", "chqdev is documented as the de facto/authoritative process")
	}
	if e := checkReleaseProcessContract(root, cfg); e != nil {
		r.fail("release-process-contract", e.Error())
	} else {
		r.pass("release-process-contract", "qualify --fresh + strict CTest + deterministic exact archive + regression + shell-role contract")
	}
	if e := checkPackageMetadataContract(cfg); e != nil {
		r.fail("package-metadata-contract", e.Error())
	} else {
		r.pass("package-metadata-contract", "release-owned package status/source/features are current and non-stale")
	}
	if e := checkReleaseRootHygiene(root); e != nil {
		r.fail("release-root-hygiene", e.Error())
	} else {
		r.pass("release-root-hygiene", "single CHANGELOG/VALIDATION + generic current manifests + no version-specific qualification wrappers")
	}
	if e := scanBackupFiles(root, cfg); e != nil {
		r.fail("backup-files", e.Error())
	} else {
		r.pass("backup-files", "no .bak/.orig/~ source debris")
	}
	r.Data["windowsScriptFiles"] = asciiCount
	r.Data["jsonFiles"] = jc
	r.Data["scriptFiles"] = sc
	r.Data["catalogRows"] = cc
	return r
}

func isExcluded(rel string, cfg Config) bool {
	rel = filepath.ToSlash(rel)
	first := strings.Split(rel, "/")[0]
	for _, d := range cfg.PackageExcludeDirs {
		if first == d {
			return true
		}
	}
	return false
}

func scanWindowsASCII(root string, cfg Config) (int, []string) {
	exts := map[string]bool{}
	for _, x := range cfg.WindowsASCIIExtensions {
		exts[strings.ToLower(x)] = true
	}
	count := 0
	bad := []string{}
	filepath.WalkDir(root, func(p string, d fs.DirEntry, e error) error {
		if e != nil {
			return nil
		}
		rel, _ := filepath.Rel(root, p)
		if rel == "." {
			return nil
		}
		if d.IsDir() {
			if isExcluded(rel, cfg) {
				return filepath.SkipDir
			}
			return nil
		}
		if !exts[strings.ToLower(filepath.Ext(p))] {
			return nil
		}
		count++
		b, e := os.ReadFile(p)
		if e != nil {
			bad = append(bad, filepath.ToSlash(rel)+": "+e.Error())
			return nil
		}
		if len(b) >= 3 && bytes.Equal(b[:3], []byte{0xEF, 0xBB, 0xBF}) {
			b = b[3:]
		}
		line, col := 1, 1
		for i := 0; i < len(b); {
			if b[i] == '\n' {
				line++
				col = 1
				i++
				continue
			}
			if b[i] < 128 {
				col++
				i++
				continue
			}
			rr, n := utf8.DecodeRune(b[i:])
			if rr == utf8.RuneError && n == 1 {
				bad = append(bad, fmt.Sprintf("%s:%d:%d invalid UTF-8 byte 0x%02X", filepath.ToSlash(rel), line, col, b[i]))
				i++
				col++
				continue
			}
			bad = append(bad, fmt.Sprintf("%s:%d:%d U+%04X", filepath.ToSlash(rel), line, col, rr))
			i += n
			col++
		}
		return nil
	})
	return count, bad
}

func scanJSON(root string, cfg Config) (int, []string) {
	count := 0
	errs := []string{}
	filepath.WalkDir(root, func(p string, d fs.DirEntry, e error) error {
		if e != nil {
			return nil
		}
		rel, _ := filepath.Rel(root, p)
		if rel == "." {
			return nil
		}
		if d.IsDir() {
			if isExcluded(rel, cfg) {
				return filepath.SkipDir
			}
			return nil
		}
		if strings.ToLower(filepath.Ext(p)) != ".json" {
			return nil
		}
		count++
		b, e := os.ReadFile(p)
		if e != nil {
			errs = append(errs, filepath.ToSlash(rel)+": "+e.Error())
			return nil
		}
		if len(b) >= 3 && bytes.Equal(b[:3], []byte{0xEF, 0xBB, 0xBF}) {
			b = b[3:]
		}
		var v any
		if e = json.Unmarshal(b, &v); e != nil {
			errs = append(errs, filepath.ToSlash(rel)+": "+e.Error())
		}
		return nil
	})
	return count, errs
}

func normalizeReleaseText(s string) string {
	return strings.ReplaceAll(strings.ReplaceAll(s, "\r\n", "\n"), "\r", "\n")
}
func handoverContract(s, version string) bool {
	s = normalizeReleaseText(s)
	if !strings.Contains(s, "v"+version) {
		return false
	}
	auth, next := false, false
	for _, ln := range strings.Split(s, "\n") {
		t := strings.TrimSpace(ln)
		if strings.EqualFold(t, "## Authoritative continuation") {
			auth = true
		}
		if strings.EqualFold(t, "## Immediate next task") {
			next = true
		}
	}
	return auth && next
}
func checkHandover(root string, cfg Config, version string) error {
	b, e := os.ReadFile(filepath.Join(root, filepath.FromSlash(cfg.Handover)))
	if e != nil {
		return e
	}
	if !handoverContract(string(b), version) {
		return fmt.Errorf("%s does not satisfy current handover contract", cfg.Handover)
	}
	return nil
}
func selfTestHandover(v string) error {
	lf := fmt.Sprintf("# Continue from v%s\n## Authoritative continuation\nCurrent.\n## Immediate next task\nNext.\n", v)
	crlf := strings.ReplaceAll(lf, "\n", "\r\n")
	cr := strings.ReplaceAll(lf, "\n", "\r")
	missA := fmt.Sprintf("# Continue from v%s\n## Immediate next task\nNext.\n", v)
	missN := fmt.Sprintf("# Continue from v%s\n## Authoritative continuation\nCurrent.\n", v)
	stale := "# Continue from v0.0.0-stale\n## Authoritative continuation\nCurrent.\n## Immediate next task\nNext.\n"
	if !(handoverContract(lf, v) && handoverContract(crlf, v) && handoverContract(cr, v) && !handoverContract(missA, v) && !handoverContract(missN, v) && !handoverContract(stale, v)) {
		return fmt.Errorf("handover contract matrix failed")
	}
	return nil
}

func checkScriptCatalog(root string, cfg Config) (int, int, error) {
	scripts := []string{}
	base := filepath.Join(root, filepath.FromSlash(cfg.ScriptRoot))
	filepath.WalkDir(base, func(p string, d fs.DirEntry, e error) error {
		if e != nil {
			return nil
		}
		if !d.IsDir() && strings.HasSuffix(strings.ToLower(d.Name()), ".chqscript") {
			rel, _ := filepath.Rel(base, p)
			scripts = append(scripts, filepath.ToSlash(rel))
		}
		return nil
	})
	sort.Strings(scripts)
	f, e := os.Open(filepath.Join(root, filepath.FromSlash(cfg.ScriptCatalog)))
	if e != nil {
		return 0, 0, e
	}
	defer f.Close()
	rows := map[string]int{}
	scan := bufio.NewScanner(f)
	for scan.Scan() {
		ln := strings.TrimSpace(scan.Text())
		if !strings.HasPrefix(ln, "|") {
			continue
		}
		parts := strings.Split(ln, "|")
		if len(parts) < 3 {
			continue
		}
		p := strings.TrimSpace(parts[1])
		if p == "" || strings.EqualFold(p, "Path") || strings.Trim(p, "-: ") == "" || !strings.HasSuffix(strings.ToLower(p), ".chqscript") {
			continue
		}
		rows[p]++
	}
	if e := scan.Err(); e != nil {
		return len(scripts), len(rows), e
	}
	missing := []string{}
	for _, s := range scripts {
		if rows[s] == 0 {
			missing = append(missing, s)
		}
	}
	extra := []string{}
	dups := []string{}
	set := map[string]bool{}
	for _, s := range scripts {
		set[s] = true
	}
	for p, n := range rows {
		if !set[p] {
			extra = append(extra, p)
		}
		if n != 1 {
			dups = append(dups, fmt.Sprintf("%s x%d", p, n))
		}
	}
	sort.Strings(missing)
	sort.Strings(extra)
	sort.Strings(dups)
	if len(missing)+len(extra)+len(dups) > 0 {
		return len(scripts), len(rows), fmt.Errorf("missing=%v extra=%v duplicates=%v", missing, extra, dups)
	}
	return len(scripts), len(rows), nil
}

func checkCurrentDocs(root string, cfg Config, version string) error {
	bad := []string{}
	for _, f := range cfg.CurrentDocs {
		b, e := os.ReadFile(filepath.Join(root, filepath.FromSlash(f)))
		if e != nil {
			bad = append(bad, f+": "+e.Error())
			continue
		}
		if !strings.Contains(string(b), version) {
			bad = append(bad, f)
		}
	}
	if len(bad) > 0 {
		return fmt.Errorf("stale/missing current docs: %s", strings.Join(bad, ", "))
	}
	return nil
}
func checkGuardDocs(root string, cfg Config) error {
	bad := []string{}
	for _, f := range cfg.ReleaseGuardDocs {
		b, e := os.ReadFile(filepath.Join(root, filepath.FromSlash(f)))
		if e != nil {
			bad = append(bad, f)
			continue
		}
		s := string(b)
		if !strings.Contains(strings.ToUpper(s), "CRLF") || !strings.Contains(strings.ToUpper(s), "LF") {
			bad = append(bad, f)
		}
	}
	if len(bad) > 0 {
		return fmt.Errorf("release guard wording missing: %s", strings.Join(bad, ", "))
	}
	return nil
}
func checkBuildProcessAuthority(root string, cfg Config) error {
	bad := []string{}
	for _, f := range cfg.BuildProcessDocs {
		b, e := os.ReadFile(filepath.Join(root, filepath.FromSlash(f)))
		if e != nil {
			bad = append(bad, f+": "+e.Error())
			continue
		}
		s := strings.ToLower(string(b))
		if !strings.Contains(s, "chqdev") || !(strings.Contains(s, "de facto") || strings.Contains(s, "authoritative build/release")) {
			bad = append(bad, f)
		}
	}
	if len(bad) > 0 {
		return fmt.Errorf("chqdev authority wording missing: %s", strings.Join(bad, ", "))
	}
	return nil
}

func checkReleaseProcessContract(root string, cfg Config) error {
	bad := []string{}
	legacyClaims := []string{
		"canonical windows build path is build-debug.bat",
		"normal windows development entry point is build-debug.bat",
		"build only through build-debug.bat",
		"start-chasehq.ps1 is now the normal entry point",
		"historical cpu_bus_rom_tests segfault may be tolerated",
		"known windows test exception",
	}
	for _, f := range cfg.ReleaseProcessDocs {
		b, e := os.ReadFile(filepath.Join(root, filepath.FromSlash(f)))
		if e != nil {
			bad = append(bad, f+": "+e.Error())
			continue
		}
		s := strings.ToLower(normalizeReleaseText(string(b)))
		required := []struct {
			name string
			ok   bool
		}{
			{"chqdev authority", strings.Contains(s, "chqdev") && (strings.Contains(s, "de facto") || strings.Contains(s, "authoritative build/release"))},
			{"fresh qualification", strings.Contains(s, "qualify --fresh")},
			{"strict CTest", strings.Contains(s, "zero tolerated ctest") || strings.Contains(s, "any ctest failure is fatal")},
			{"deterministic archive", strings.Contains(s, "deterministic") && strings.Contains(s, "archive")},
			{"Full Regression", strings.Contains(s, "full regression")},
			{"shell compatibility role", (strings.Contains(s, "powershell/bat") || strings.Contains(s, "shell")) && strings.Contains(s, "compatibility")},
		}
		missing := []string{}
		for _, req := range required {
			if !req.ok {
				missing = append(missing, req.name)
			}
		}
		for _, phrase := range legacyClaims {
			if strings.Contains(s, phrase) {
				missing = append(missing, "forbidden legacy canonical claim: "+phrase)
			}
		}
		if len(missing) > 0 {
			bad = append(bad, f+" ["+strings.Join(missing, ", ")+"]")
		}
	}
	if len(bad) > 0 {
		return fmt.Errorf("release process documentation drift: %s", strings.Join(bad, "; "))
	}
	return nil
}

func checkPackageMetadataContract(cfg Config) error {
	status := strings.TrimSpace(cfg.PackageStatus)
	source := strings.TrimSpace(cfg.PackageSourceBaseline)
	if status == "" || source == "" || len(cfg.PackageFeatures) == 0 {
		return fmt.Errorf("chqdev.json must define packageStatus, packageSourceBaseline and non-empty packageFeatures")
	}
	if !strings.Contains(status, cfg.Release) {
		return fmt.Errorf("packageStatus does not identify current release %s", cfg.Release)
	}
	if !strings.Contains(source, cfg.Baseline) && !strings.Contains(source, cfg.Release) {
		return fmt.Errorf("packageSourceBaseline identifies neither current release nor configured baseline")
	}
	upper := strings.ToUpper(cfg.Release)
	statusUpper := strings.ToUpper(status)
	if !strings.Contains(upper, "-RC") && (strings.Contains(statusUpper, "RELEASE CANDIDATE") || strings.Contains(statusUpper, "PROMOTION PENDING")) {
		return fmt.Errorf("final release packageStatus still describes a candidate/pending promotion")
	}
	for i, feature := range cfg.PackageFeatures {
		if strings.TrimSpace(feature) == "" {
			return fmt.Errorf("packageFeatures[%d] is empty", i)
		}
	}
	return nil
}

func checkReleaseRootHygiene(root string) error {
	required := []string{"CHANGELOG.md", "VALIDATION.md", "chqdev.bat"}
	bad := []string{}
	for _, name := range required {
		if st, err := os.Stat(filepath.Join(root, name)); err != nil || st.IsDir() {
			bad = append(bad, "missing "+name)
		}
	}
	patterns := []string{
		"BUILD_NOTES_*",
		"HOTFIX_NOTES_*",
		"RELEASE_NOTES_*",
		"VALIDATION_*",
		"PACKAGE_MANIFEST_*",
		"Qualify-*.bat",
		"RC*_PORTABLE_VALIDATION*",
	}
	for _, pattern := range patterns {
		matches, _ := filepath.Glob(filepath.Join(root, pattern))
		for _, m := range matches {
			if st, err := os.Stat(m); err == nil && !st.IsDir() {
				bad = append(bad, filepath.Base(m))
			}
		}
	}
	toolMatches, _ := filepath.Glob(filepath.Join(root, "tools", "Invoke-RC*Qualification.ps1"))
	for _, m := range toolMatches {
		bad = append(bad, filepath.ToSlash(filepath.Join("tools", filepath.Base(m))))
	}
	sort.Strings(bad)
	if len(bad) > 0 {
		return fmt.Errorf("obsolete release-root/history wrapper files present: %s", strings.Join(bad, ", "))
	}
	return nil
}

func releaseBaseVersion(release string) string {
	re := regexp.MustCompile(`^([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+)`)
	m := re.FindStringSubmatch(release)
	if len(m) != 2 {
		return ""
	}
	return m[1]
}

func checkCMakeProjectVersion(root string, cfg Config) error {
	base := releaseBaseVersion(cfg.Release)
	if base == "" {
		return fmt.Errorf("cannot derive numeric base version from %q", cfg.Release)
	}
	b, e := os.ReadFile(filepath.Join(root, "CMakeLists.txt"))
	if e != nil {
		return e
	}
	re := regexp.MustCompile(`project\(ChaseHQNative\s+VERSION\s+([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+)`)
	m := re.FindStringSubmatch(string(b))
	if len(m) != 2 {
		return fmt.Errorf("CMake project version not found")
	}
	if m[1] != base {
		return fmt.Errorf("CMake project=%s release base=%s", m[1], base)
	}
	return nil
}

func checkFocusedRegressionIdentity(root string, cfg Config) error {
	if strings.TrimSpace(cfg.FocusedRegression) == "" {
		return fmt.Errorf("focusedRegression not configured")
	}
	b, e := os.ReadFile(filepath.Join(root, filepath.FromSlash(cfg.FocusedRegression)))
	if e != nil {
		return e
	}
	s := normalizeReleaseText(string(b))
	want := "# regression-version: " + cfg.Release
	if !strings.Contains(s, want) {
		return fmt.Errorf("%s missing %q", cfg.FocusedRegression, want)
	}
	if !strings.Contains(s, "api release.assert-version") {
		return fmt.Errorf("%s missing release.assert-version", cfg.FocusedRegression)
	}
	return nil
}

func checkBuildIdentityFiles(root string, cfg Config) error {
	bad := []string{}
	for _, rel := range cfg.BuildIdentityFiles {
		b, e := os.ReadFile(filepath.Join(root, filepath.FromSlash(rel)))
		if e != nil {
			bad = append(bad, rel+": "+e.Error())
			continue
		}
		var v map[string]any
		if e := json.Unmarshal(b, &v); e != nil {
			bad = append(bad, rel+": "+e.Error())
			continue
		}
		got, _ := v["build"].(string)
		if got != cfg.Release {
			bad = append(bad, fmt.Sprintf("%s build=%q", rel, got))
		}
	}
	if len(bad) > 0 {
		return fmt.Errorf("stale machine-readable build identity: %s", strings.Join(bad, "; "))
	}
	return nil
}

func windowsCTestIdentityContract(cmake, script string) error {
	if !strings.Contains(cmake, `-ExpectedBuild "${CHQ_RELEASE_VERSION}"`) {
		return fmt.Errorf("CMake cli_evidence_tests does not inject CHQ_RELEASE_VERSION as ExpectedBuild")
	}
	if !strings.Contains(script, "[string]$ExpectedBuild") || !strings.Contains(script, "$m.build -ne $ExpectedBuild") {
		return fmt.Errorf("cli_evidence_tests does not compare evidence build against ExpectedBuild")
	}
	hard := regexp.MustCompile(`(?i)\$m\.build\s+-ne\s+['\"](?:v)?[0-9]+\.[0-9]+\.[0-9]+(?:\.[0-9]+)?(?:-RC[0-9.]+)?['\"]`)
	if hard.MatchString(script) {
		return fmt.Errorf("cli_evidence_tests contains a hard-coded build identity assertion")
	}
	return nil
}

func checkWindowsCTestIdentityContract(root string) error {
	cmake, e := os.ReadFile(filepath.Join(root, "CMakeLists.txt"))
	if e != nil {
		return e
	}
	script, e := os.ReadFile(filepath.Join(root, "tests", "cli_evidence_tests.ps1"))
	if e != nil {
		return e
	}
	return windowsCTestIdentityContract(string(cmake), string(script))
}

func scanBackupFiles(root string, cfg Config) error {
	bad := []string{}
	filepath.WalkDir(root, func(p string, d fs.DirEntry, e error) error {
		if e != nil {
			return nil
		}
		rel, _ := filepath.Rel(root, p)
		if d.IsDir() {
			if isExcluded(rel, cfg) {
				return filepath.SkipDir
			}
			return nil
		}
		n := strings.ToLower(d.Name())
		if strings.HasSuffix(n, ".bak") || strings.HasSuffix(n, ".orig") || strings.HasSuffix(n, "~") {
			bad = append(bad, filepath.ToSlash(rel))
		}
		return nil
	})
	if len(bad) > 0 {
		return fmt.Errorf("%s", strings.Join(bad, ", "))
	}
	return nil
}
