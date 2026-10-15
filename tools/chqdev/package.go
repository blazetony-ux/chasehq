package main

import (
	"archive/zip"
	"bytes"
	"crypto/sha256"
	"debug/pe"
	"encoding/hex"
	"encoding/json"
	"fmt"
	"io"
	"io/fs"
	"os"
	"path/filepath"
	"sort"
	"strings"
	"time"
)

const packageManifestJSON = "PACKAGE_MANIFEST.json"
const packageManifestTXT = "PACKAGE_MANIFEST.txt"

type ManifestFile struct {
	Path   string `json:"path"`
	Size   int64  `json:"size"`
	SHA256 string `json:"sha256"`
}
type SourceManifest struct {
	Schema           string         `json:"schema"`
	Release          string         `json:"release"`
	Baseline         string         `json:"baseline"`
	SourceBaseline   string         `json:"sourceBaseline"`
	Status           string         `json:"status"`
	ManifestExcludes []string       `json:"manifestExcludes"`
	Qualification    map[string]any `json:"qualification"`
	Features         []string       `json:"features"`
	FileCount        int            `json:"fileCount"`
	Files            []ManifestFile `json:"files"`
}

func collectPackageFiles(root string, cfg Config) ([]string, error) {
	files := []string{}
	currentJSON := packageManifestJSON
	currentTXT := packageManifestTXT
	e := filepath.WalkDir(root, func(p string, d fs.DirEntry, e error) error {
		if e != nil {
			return e
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
		slash := filepath.ToSlash(rel)
		if slash == currentJSON || slash == currentTXT {
			return nil
		}
		low := strings.ToLower(slash)
		if (strings.HasSuffix(low, ".zip") && slash != "research/acceptance/ghidra-steering-RC621-verified.zip") || strings.HasSuffix(low, ".sha256.txt") {
			return nil
		}
		files = append(files, slash)
		return nil
	})
	sort.Strings(files)
	return files, e
}
func hashFile(p string) (string, int64, error) {
	f, e := os.Open(p)
	if e != nil {
		return "", 0, e
	}
	defer f.Close()
	h := sha256.New()
	n, e := io.Copy(h, f)
	if e != nil {
		return "", 0, e
	}
	return hex.EncodeToString(h.Sum(nil)), n, nil
}

func generateManifest(root string, cfg Config) (SourceManifest, error) {
	var m SourceManifest
	m.Schema = "chq-release-source-manifest-v1"
	m.Release = cfg.Release
	m.Baseline = cfg.Baseline
	m.SourceBaseline = cfg.PackageSourceBaseline
	m.Status = cfg.PackageStatus
	m.ManifestExcludes = []string{packageManifestJSON, packageManifestTXT}
	m.Qualification = map[string]any{"schema": "chqdev-qualification-v2", "authoritativeController": "chqdev v" + toolVersion, "legacyShellRole": "compatibility/fallback only", "rule": "New build/release logic MUST be added to chqdev, not PowerShell/BAT", "exactArchiveGate": "chqdev qualify --fresh; zero CTest failures; deterministic exact archive; runtime readiness; focused + Full Regression; promotable"}
	m.Features = append([]string(nil), cfg.PackageFeatures...)
	files, e := collectPackageFiles(root, cfg)
	if e != nil {
		return m, e
	}
	for _, rel := range files {
		sha, n, e := hashFile(filepath.Join(root, filepath.FromSlash(rel)))
		if e != nil {
			return m, e
		}
		m.Files = append(m.Files, ManifestFile{Path: rel, Size: n, SHA256: sha})
	}
	m.FileCount = len(m.Files)
	return m, nil
}
func writeManifest(root string, cfg Config, m SourceManifest) error {
	jp := filepath.Join(root, packageManifestJSON)
	tp := filepath.Join(root, packageManifestTXT)
	b, e := json.MarshalIndent(m, "", "  ")
	if e != nil {
		return e
	}
	b = append(b, '\n')
	if e = os.WriteFile(jp, b, 0644); e != nil {
		return e
	}
	var sb strings.Builder
	fmt.Fprintf(&sb, "ChaseHQ-Native %s authoritative source manifest\n", cfg.Release)
	fmt.Fprintf(&sb, "files=%d\n", m.FileCount)
	for _, f := range m.Files {
		fmt.Fprintf(&sb, "%s  %d  %s\n", f.SHA256, f.Size, f.Path)
	}
	return os.WriteFile(tp, []byte(sb.String()), 0644)
}

// verifyBundledWindowsTooling is a fail-closed package/verify gate: the exact
// executable architecture must be present in the actual source/extracted ZIP.
// Checking filenames alone previously allowed a broken Windows RC to ship.
func verifyBundledWindowsTooling(root string) error {
	launch := filepath.Join(root, "chqdev.bat")
	if !fileExists(launch) {
		return fmt.Errorf("required Windows launcher absent: %s", launch)
	}
	for _, candidate := range []struct {
		name    string
		machine uint16
	}{
		{"chqdev-windows-amd64.exe", pe.IMAGE_FILE_MACHINE_AMD64},
		{"chqdev-windows-arm64.exe", pe.IMAGE_FILE_MACHINE_ARM64},
	} {
		p := filepath.Join(root, "tools", "chqdev", "bin", candidate.name)
		f, e := os.Open(p)
		if e != nil {
			return fmt.Errorf("required Windows chqdev executable missing: %s: %w", candidate.name, e)
		}
		info, e := f.Stat()
		if e != nil {
			f.Close()
			return e
		}
		if info.Size() < 4096 {
			f.Close()
			return fmt.Errorf("Windows chqdev binary unreasonably small: %s", candidate.name)
		}
		exe, e := pe.NewFile(f)
		if e != nil {
			f.Close()
			return fmt.Errorf("Windows chqdev binary corrupt: %s: %w", candidate.name, e)
		}
		got := exe.FileHeader.Machine
		exe.Close()
		f.Close()
		if got != candidate.machine {
			return fmt.Errorf("Windows chqdev binary wrong machine: %s got %#x want %#x", candidate.name, got, candidate.machine)
		}
	}
	return nil
}

func packageCommand(root string, cfg Config, out string, jsonOut bool) *Report {
	r := newReport("package", cfg.Release)
	pre := preflight(root, cfg)
	if pre.Status == "FAIL" {
		r.fail("preflight", "portable preflight failed")
		r.Data["preflight"] = pre
		return r
	}
	r.pass("preflight", "PASS")
	if e := verifyBundledWindowsTooling(root); e != nil {
		r.fail("windows-tooling", e.Error())
		return r
	}
	r.pass("windows-tooling", "Windows x64/ARM64 PE images and launcher validated")
	m, e := generateManifest(root, cfg)
	if e != nil {
		r.fail("manifest", e.Error())
		return r
	}
	if e = writeManifest(root, cfg, m); e != nil {
		r.fail("manifest", e.Error())
		return r
	}
	r.pass("manifest", fmt.Sprintf("%d authoritative files", m.FileCount))
	if out == "" {
		out = filepath.Join(root, "dist", "ChaseHQ-Native-v"+cfg.Release+".zip")
	}
	if !filepath.IsAbs(out) {
		out = filepath.Join(root, out)
	}
	if e = os.MkdirAll(filepath.Dir(out), 0755); e != nil {
		r.fail("output", e.Error())
		return r
	}
	if e = createZip(root, cfg, m, out); e != nil {
		r.fail("zip", e.Error())
		return r
	}
	sha, n, e := hashFile(out)
	if e != nil {
		r.fail("zip-sha256", e.Error())
		return r
	}
	r.pass("zip", fmt.Sprintf("%s (%d bytes)", out, n))
	r.pass("sha256", sha)

	repro, e := os.CreateTemp(filepath.Dir(out), ".chqdev-repro-*.zip")
	if e != nil {
		r.fail("reproducibility", e.Error())
		return r
	}
	reproPath := repro.Name()
	repro.Close()
	defer os.Remove(reproPath)
	if e = createZip(root, cfg, m, reproPath); e != nil {
		r.fail("reproducibility", e.Error())
		return r
	}
	reproSHA, reproN, e := hashFile(reproPath)
	if e != nil {
		r.fail("reproducibility", e.Error())
		return r
	}
	if sha != reproSHA || n != reproN {
		r.fail("reproducibility", fmt.Sprintf("first=%s/%d second=%s/%d", sha, n, reproSHA, reproN))
		return r
	}
	r.pass("reproducibility", "second independent package is byte-identical")

	vr := verifyArchive(out)
	if vr.err != nil {
		r.fail("verify", vr.err.Error())
		return r
	}
	r.pass("verify", fmt.Sprintf("%d authoritative + %d manifest extras; 0 missing/extra/hash mismatch", vr.authoritative, vr.manifestExtras))
	shaPath := out + ".sha256.txt"
	if e = os.WriteFile(shaPath, []byte(sha+"  "+filepath.Base(out)+"\n"), 0644); e != nil {
		r.fail("sha256-file", e.Error())
		return r
	}
	r.pass("sha256-file", shaPath)
	r.Data["zip"] = out
	r.Data["sha256"] = sha
	r.Data["sha256File"] = shaPath
	r.Data["fileCount"] = m.FileCount
	r.Data["deterministic"] = true
	return r
}

var deterministicZipTime = time.Date(2000, 1, 1, 0, 0, 0, 0, time.UTC)

func deterministicMode(rel string) os.FileMode {
	low := strings.ToLower(filepath.ToSlash(rel))
	if strings.HasSuffix(low, ".sh") || strings.Contains(low, "tools/chqdev/bin/chqdev-") {
		return 0755
	}
	return 0644
}

func createZip(root string, cfg Config, m SourceManifest, out string) error {
	f, e := os.Create(out)
	if e != nil {
		return e
	}
	zw := zip.NewWriter(f)
	prefix := "ChaseHQ-Native-v" + cfg.Release + "/"
	all := make([]string, 0, len(m.Files)+len(m.ManifestExcludes))
	for _, x := range m.Files {
		all = append(all, x.Path)
	}
	all = append(all, m.ManifestExcludes...)
	sort.Strings(all)
	for _, rel := range all {
		p := filepath.Join(root, filepath.FromSlash(rel))
		if _, e := os.Stat(p); e != nil {
			zw.Close()
			f.Close()
			return e
		}
		h := &zip.FileHeader{Name: prefix + filepath.ToSlash(rel), Method: zip.Deflate}
		h.SetModTime(deterministicZipTime)
		h.SetMode(deterministicMode(rel))
		h.Modified = deterministicZipTime
		h.Comment = ""
		h.Extra = nil
		w, e := zw.CreateHeader(h)
		if e != nil {
			zw.Close()
			f.Close()
			return e
		}
		in, e := os.Open(p)
		if e != nil {
			zw.Close()
			f.Close()
			return e
		}
		_, copyErr := io.Copy(w, in)
		closeErr := in.Close()
		if copyErr != nil {
			zw.Close()
			f.Close()
			return copyErr
		}
		if closeErr != nil {
			zw.Close()
			f.Close()
			return closeErr
		}
	}
	if e = zw.Close(); e != nil {
		f.Close()
		return e
	}
	return f.Close()
}

type verifyResult struct {
	authoritative  int
	manifestExtras int
	err            error
}

func verifyArchive(zipPath string) verifyResult {
	zr, e := zip.OpenReader(zipPath)
	if e != nil {
		return verifyResult{err: e}
	}
	defer zr.Close()
	roots := map[string]bool{}
	for _, f := range zr.File {
		parts := strings.SplitN(f.Name, "/", 2)
		if len(parts) >= 2 && parts[1] != "" {
			roots[parts[0]] = true
		}
	}
	if len(roots) != 1 {
		return verifyResult{err: fmt.Errorf("archive must contain exactly one project root, found %d", len(roots))}
	}
	var rootName string
	for k := range roots {
		rootName = k
	}
	const prefix = "ChaseHQ-Native-v"
	if !strings.HasPrefix(rootName, prefix) {
		return verifyResult{err: fmt.Errorf("unexpected project root %s", rootName)}
	}
	release := strings.TrimPrefix(rootName, prefix)
	expectedManifest := packageManifestJSON
	var manifestBytes []byte
	for _, f := range zr.File {
		parts := strings.SplitN(f.Name, "/", 2)
		if len(parts) != 2 || parts[0] != rootName || parts[1] != expectedManifest {
			continue
		}
		rc, err := f.Open()
		if err != nil {
			return verifyResult{err: err}
		}
		manifestBytes, err = io.ReadAll(rc)
		rc.Close()
		if err != nil {
			return verifyResult{err: err}
		}
		break
	}
	if manifestBytes == nil {
		return verifyResult{err: fmt.Errorf("authoritative source manifest not found: %s", expectedManifest)}
	}
	if len(manifestBytes) >= 3 && bytes.Equal(manifestBytes[:3], []byte{0xEF, 0xBB, 0xBF}) {
		manifestBytes = manifestBytes[3:]
	}
	var m SourceManifest
	if e = json.Unmarshal(manifestBytes, &m); e != nil {
		return verifyResult{err: e}
	}
	if m.Release != release {
		return verifyResult{err: fmt.Errorf("manifest release %s does not match archive root %s", m.Release, release)}
	}
	tmp, e := os.MkdirTemp("", "chqdev-verify-")
	if e != nil {
		return verifyResult{err: e}
	}
	defer os.RemoveAll(tmp)
	if e = extractZip(zr.File, tmp); e != nil {
		return verifyResult{err: e}
	}
	base := filepath.Join(tmp, rootName)
	if e := verifyBundledWindowsTooling(base); e != nil {
		return verifyResult{err: e}
	}
	allowed := map[string]bool{}
	for _, f := range m.Files {
		allowed[f.Path] = true
	}
	for _, x := range m.ManifestExcludes {
		allowed[x] = true
	}
	actual := map[string]bool{}
	filepath.WalkDir(base, func(p string, d fs.DirEntry, e error) error {
		if e != nil || d.IsDir() {
			return nil
		}
		rel, _ := filepath.Rel(base, p)
		actual[filepath.ToSlash(rel)] = true
		return nil
	})
	missing, extra, bad := []string{}, []string{}, []string{}
	for p := range allowed {
		if !actual[p] {
			missing = append(missing, p)
		}
	}
	for p := range actual {
		if !allowed[p] {
			extra = append(extra, p)
		}
	}
	for _, f := range m.Files {
		sha, n, err := hashFile(filepath.Join(base, filepath.FromSlash(f.Path)))
		if err != nil || sha != strings.ToLower(f.SHA256) || n != f.Size {
			bad = append(bad, f.Path)
		}
	}
	sort.Strings(missing)
	sort.Strings(extra)
	sort.Strings(bad)
	if len(missing)+len(extra)+len(bad) > 0 {
		return verifyResult{err: fmt.Errorf("missing=%v extra=%v hashMismatch=%v", missing, extra, bad)}
	}
	return verifyResult{authoritative: len(m.Files), manifestExtras: len(m.ManifestExcludes)}
}
func extractZip(files []*zip.File, dst string) error {
	for _, f := range files {
		clean := filepath.Clean(filepath.FromSlash(f.Name))
		target := filepath.Join(dst, clean)
		if !strings.HasPrefix(target, filepath.Clean(dst)+string(os.PathSeparator)) {
			return fmt.Errorf("unsafe archive path %s", f.Name)
		}
		if f.FileInfo().IsDir() {
			if e := os.MkdirAll(target, f.Mode()); e != nil {
				return e
			}
			continue
		}
		if e := os.MkdirAll(filepath.Dir(target), 0755); e != nil {
			return e
		}
		rc, e := f.Open()
		if e != nil {
			return e
		}
		out, e := os.OpenFile(target, os.O_CREATE|os.O_WRONLY|os.O_TRUNC, f.Mode().Perm())
		if e != nil {
			rc.Close()
			return e
		}
		_, ce := io.Copy(out, rc)
		rc.Close()
		oe := out.Close()
		if ce != nil {
			return ce
		}
		if oe != nil {
			return oe
		}
	}
	return nil
}
func verifyCommand(root string, cfg Config, p string) *Report {
	r := newReport("verify", cfg.Release)
	if !filepath.IsAbs(p) {
		p = filepath.Join(root, p)
	}
	v := verifyArchive(p)
	if v.err != nil {
		r.fail("archive", v.err.Error())
		return r
	}
	sha, n, e := hashFile(p)
	if e != nil {
		r.fail("sha256", e.Error())
		return r
	}
	r.pass("archive", fmt.Sprintf("%d authoritative + %d manifest files; exact", v.authoritative, v.manifestExtras))
	r.pass("sha256", sha)
	r.Data["path"] = p
	r.Data["bytes"] = n
	r.Data["sha256"] = sha
	return r
}
