package main

import (
	"archive/zip"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"regexp"
	"runtime"
	"strings"
)

// This command never downloads or installs Ghidra or Java. Bundled Jython
// installation is explicit opt-in and is restricted to the current user's
// Ghidra Extensions directory. No arbitrary directory/executable arguments.
func ghidraCommand(root string, cfg Config, args []string) *Report {
	report := newReport("ghidra", cfg.Release)
	if runtime.GOOS != "windows" {
		report.warn("platform", "Ghidra environment setup supported on Windows host only")
		return report
	}
	java := locateJDK25()
	report.Data["java"] = java
	if java.JavaReady {
		report.pass("java", java.JavaHome)
	} else {
		report.warn("java", java.Detection)
	}
	home := os.Getenv("GHIDRA_HOME")
	if home == "" {
		home = `C:\Tools\Ghidra`
	}
	headless := filepath.Join(home, "support", "analyzeHeadless.bat")
	if _, err := os.Stat(headless); err != nil {
		report.warn("ghidra", "analyzeHeadless.bat unavailable at "+headless)
		return report
	}
	report.pass("ghidra", "headless launcher found")
	extensions, _ := filepath.Glob(filepath.Join(home, "Extensions", "Ghidra", "*Jython.zip"))
	if len(extensions) == 0 {
		report.warn("jython", "No matching bundled Jython extension found: use Ghidra File > Install Extensions")
		return report
	}
	ext := extensions[0]
	re := regexp.MustCompile(`ghidra_(\d+\.\d+\.\d+)_PUBLIC_[0-9]{8}_Jython\.zip$`)
	match := re.FindStringSubmatch(filepath.Base(ext))
	if match == nil {
		report.warn("jython", "Bundled Jython version could not be established; refusing installation")
		return report
	}
	version := match[1]
	ghidraVersionFile := filepath.Join(home, "Ghidra", "application.properties")
	if b, e := os.ReadFile(ghidraVersionFile); e == nil && !strings.Contains(string(b), "application.version="+version) {
		report.warn("jython", "Bundled Jython version mismatches Ghidra distribution")
		return report
	}
	appdata := os.Getenv("APPDATA")
	if appdata == "" {
		report.warn("jython", "APPDATA not configured")
		return report
	}
	versionRoots, _ := filepath.Glob(filepath.Join(appdata, "ghidra", "ghidra_"+version+"_PUBLIC*"))
	// Bundled version alone cannot identify an arbitrary user profile.
	if len(versionRoots) != 1 {
		report.warn("jython", "Expected exactly one matching Ghidra user profile; launch Ghidra once, then retry")
		return report
	}
	target := filepath.Join(versionRoots[0], "Extensions", "Jython")
	report.Data["jythonDirectory"] = target
	if info, e := os.Stat(target); e == nil && info.IsDir() {
		report.pass("jython", "matching user extension already installed")
		return report
	}
	if len(args) == 0 || args[0] == "doctor" {
		report.warn("jython", "Bundled Jython available but not installed. Explicitly opt in: chqdev ghidra setup --install-jython")
		return report
	}
	if len(args) != 2 || args[0] != "setup" || args[1] != "--install-jython" {
		report.fail("usage", "ghidra doctor | ghidra setup --install-jython")
		return report
	}
	if err := installBundledJython(ext, target); err != nil {
		report.fail("jython", err.Error())
		return report
	}
	report.pass("jython", "bundled matching Jython installed for current user; restart Ghidra before headless use")
	return report
}

func installBundledJython(archive, target string) error {
	z, e := zip.OpenReader(archive)
	if e != nil {
		return e
	}
	defer z.Close()
	if len(z.File) > 5000 {
		return fmt.Errorf("extension archive too many files")
	}
	parent := filepath.Dir(target)
	if e := os.MkdirAll(parent, 0755); e != nil {
		return e
	}
	if _, e := os.Stat(target); e == nil {
		return fmt.Errorf("target already exists: %s", target)
	}
	tmp, e := os.MkdirTemp(parent, ".jython-install-")
	if e != nil {
		return e
	}
	defer os.RemoveAll(tmp)
	sawManifest := false
	for _, f := range z.File {
		name := strings.ReplaceAll(f.Name, "\\", "/")
		if strings.HasPrefix(name, "Jython/") {
			name = strings.TrimPrefix(name, "Jython/")
		}
		if name == "" {
			continue
		}
		clean := filepath.Clean(filepath.FromSlash(name))
		if clean == "." || clean == ".." || filepath.IsAbs(clean) || strings.HasPrefix(clean, ".."+string(filepath.Separator)) || strings.Contains(clean, ":") {
			return fmt.Errorf("unsafe Jython archive entry: %s", f.Name)
		}
		if f.Mode()&os.ModeSymlink != 0 {
			return fmt.Errorf("symlink extension entries are not allowed")
		}
		dst := filepath.Join(tmp, clean)
		if f.FileInfo().IsDir() {
			if e := os.MkdirAll(dst, 0755); e != nil {
				return e
			}
			continue
		}
		if f.UncompressedSize64 > 64*1024*1024 {
			return fmt.Errorf("extension member too large")
		}
		if e := os.MkdirAll(filepath.Dir(dst), 0755); e != nil {
			return e
		}
		source, e := f.Open()
		if e != nil {
			return e
		}
		out, e := os.OpenFile(dst, os.O_WRONLY|os.O_CREATE|os.O_EXCL, 0644)
		if e != nil {
			source.Close()
			return e
		}
		_, copyErr := io.CopyN(out, source, int64(f.UncompressedSize64))
		closeErr := out.Close()
		source.Close()
		if copyErr != nil {
			return copyErr
		}
		if closeErr != nil {
			return closeErr
		}
		if clean == "extension.properties" {
			sawManifest = true
		}
	}
	if !sawManifest {
		return fmt.Errorf("bundled Jython missing extension.properties; no installation performed")
	}
	return os.Rename(tmp, target)
}
