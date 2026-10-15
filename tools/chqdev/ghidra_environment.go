package main

// GhidraEnvironment is deliberately local-only. It never downloads executables,
// changes global Windows settings, or executes caller-supplied commands.
import (
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
)

type GhidraEnvironment struct {
	JavaHome       string `json:"javaHome"`
	JavaExecutable string `json:"javaExecutable"`
	JavaReady      bool   `json:"javaReady"`
	Detection      string `json:"detection"`
}

func detectJava25(getenv func(string) string, candidates []string, probe func(string) bool) GhidraEnvironment {
	roots := []string{}
	if s := strings.TrimSpace(getenv("JAVA_HOME")); s != "" {
		roots = append(roots, s)
	}
	roots = append(roots, candidates...)
	seen := map[string]bool{}
	for _, home := range roots {
		if home == "" {
			continue
		}
		clean := filepath.Clean(home)
		key := strings.ToLower(clean)
		if seen[key] {
			continue
		}
		seen[key] = true
		executable := filepath.Join(clean, "bin", "java.exe")
		if probe(executable) {
			return GhidraEnvironment{JavaHome: clean, JavaExecutable: executable, JavaReady: true, Detection: "validated JDK 25"}
		}
	}
	return GhidraEnvironment{Detection: "Java 25 JDK not detected; Ghidra unavailable but gameplay remains supported"}
}

func locateJDK25() GhidraEnvironment {
	roots := []string{}
	for _, top := range []string{`C:\Program Files\Eclipse Adoptium`, `C:\Program Files\Java`, `C:\Program Files\Microsoft`, `C:\Program Files\Amazon Corretto`} {
		entries, e := os.ReadDir(top)
		if e != nil {
			continue
		}
		for _, entry := range entries {
			if entry.IsDir() && strings.Contains(strings.ToLower(entry.Name()), "25") {
				roots = append(roots, filepath.Join(top, entry.Name()))
			}
		}
	}
	probe := func(executable string) bool {
		if info, e := os.Stat(executable); e != nil || info.IsDir() {
			return false
		}
		out, e := exec.Command(executable, "-version").CombinedOutput()
		if e != nil {
			return false
		}
		return strings.Contains(string(out), `version "25`) || strings.Contains(string(out), `version "25.`)
	}
	found := detectJava25(os.Getenv, roots, probe)
	if found.JavaReady {
		return found
	}
	if path, e := exec.LookPath("java.exe"); e == nil && probe(path) {
		home := filepath.Dir(filepath.Dir(path))
		return GhidraEnvironment{JavaHome: home, JavaExecutable: path, JavaReady: true, Detection: "validated Java 25 on PATH"}
	}
	return found
}

func setupGhidraChildJava(r *Report) {
	status := locateJDK25()
	r.Data["ghidraJava"] = status
	if !status.JavaReady {
		r.warn("ghidra-java", status.Detection)
		return
	}
	// Only chqdev and its launcher subprocesses inherit this environment; no user/machine changes.
	if e := os.Setenv("JAVA_HOME", status.JavaHome); e != nil {
		r.warn("ghidra-java", fmt.Sprintf("JAVA_HOME: %v", e))
		return
	}
	_ = os.Setenv("PATH", filepath.Join(status.JavaHome, "bin")+string(os.PathListSeparator)+os.Getenv("PATH"))
	r.pass("ghidra-java", "JDK 25 passed subprocess verification and is available to Workbench children")
}
