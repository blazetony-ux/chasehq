package main

import (
	"archive/zip"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestRC47JavaDiscoveryPrefersValidOverrideAndRejectsBrokenPaths(t *testing.T) {
	good := filepath.Join("C:", "Program Files", "Eclipse Adoptium", "jdk-25")
	seen := []string{}
	probe := func(s string) bool { seen = append(seen, s); return s == filepath.Join(good, "bin", "java.exe") }
	x := detectJava25(func(k string) string {
		if k == "JAVA_HOME" {
			return filepath.Join("C:", "missing")
		}
		return ""
	}, []string{good}, probe)
	if !x.JavaReady || x.JavaHome != good || len(seen) != 2 {
		t.Fatalf("discovery failed: %+v %+v", x, seen)
	}
	x = detectJava25(func(string) string { return "" }, []string{filepath.Join("C:", "missing")}, func(string) bool { return false })
	if x.JavaReady || !strings.Contains(x.Detection, "gameplay") {
		t.Fatalf("must fail open for gameplay, closed for Ghidra: %+v", x)
	}
}

func TestRC47JythonInstallRejectsBadArchive(t *testing.T) {
	dir := t.TempDir()
	target := filepath.Join(dir, "Extensions", "Jython")
	if err := installBundledJython(filepath.Join(dir, "absent.zip"), target); err == nil {
		t.Fatal("missing ZIP should fail")
	}
	if _, err := os.Stat(target); err == nil {
		t.Fatal("failed attempt created install")
	}
}

func TestRC47JythonInstallerOnlyExtractsVerifiedLocalExtension(t *testing.T) {
	dir := t.TempDir()
	archive := filepath.Join(dir, "bundle.zip")
	f, err := os.Create(archive)
	if err != nil {
		t.Fatal(err)
	}
	z := zip.NewWriter(f)
	entry, err := z.Create("Jython/extension.properties")
	if err != nil {
		t.Fatal(err)
	}
	_, err = entry.Write([]byte("name=Jython\n"))
	if err != nil {
		t.Fatal(err)
	}
	if err = z.Close(); err != nil {
		t.Fatal(err)
	}
	if err = f.Close(); err != nil {
		t.Fatal(err)
	}
	dest := filepath.Join(dir, "Extensions", "Jython")
	if err = installBundledJython(archive, dest); err != nil {
		t.Fatal(err)
	}
	if _, err = os.Stat(filepath.Join(dest, "extension.properties")); err != nil {
		t.Fatal(err)
	}
	if err = installBundledJython(archive, dest); err == nil {
		t.Fatal("must not overwrite installed extension")
	}
}
