package main

import (
	"os"
	"path/filepath"
	"testing"
)

func TestDeterministicZipBytes(t *testing.T) {
	root := t.TempDir()
	if err := os.WriteFile(filepath.Join(root, "a.txt"), []byte("alpha\n"), 0644); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(root, "run.sh"), []byte("#!/bin/sh\necho ok\n"), 0755); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(root, "PACKAGE_MANIFEST.json"), []byte("{}\n"), 0644); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(root, "PACKAGE_MANIFEST.txt"), []byte("manifest\n"), 0644); err != nil {
		t.Fatal(err)
	}
	cfg := Config{Release: "test"}
	m := SourceManifest{Release: "test", ManifestExcludes: []string{packageManifestJSON, packageManifestTXT}, Files: []ManifestFile{{Path: "a.txt"}, {Path: "run.sh"}}}
	z1, z2 := filepath.Join(root, "one.zip"), filepath.Join(root, "two.zip")
	if err := createZip(root, cfg, m, z1); err != nil {
		t.Fatal(err)
	}
	if err := createZip(root, cfg, m, z2); err != nil {
		t.Fatal(err)
	}
	h1, n1, err := hashFile(z1)
	if err != nil {
		t.Fatal(err)
	}
	h2, n2, err := hashFile(z2)
	if err != nil {
		t.Fatal(err)
	}
	if h1 != h2 || n1 != n2 {
		t.Fatalf("non-deterministic ZIP: %s/%d != %s/%d", h1, n1, h2, n2)
	}
}
