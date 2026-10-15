package main

import (
	"os"
	"path/filepath"
	"testing"
)

func TestLocalConfigDiscovery(t *testing.T) {
	base := t.TempDir()
	current := filepath.Join(base, "ChaseHQ-Native-RC-new")
	previous := filepath.Join(base, "ChaseHQ-Native-RC-old")
	for _, d := range []string{current, previous, filepath.Join(base, "roms"), filepath.Join(base, "SDL")} {
		if e := os.MkdirAll(d, 0755); e != nil {
			t.Fatal(e)
		}
	}
	content := []byte(`{"romSource":"` + filepath.ToSlash(filepath.Join(base, "roms")) + `","sdlSource":"` + filepath.ToSlash(filepath.Join(base, "SDL")) + `","copyRoms":true,"copySdl":true}`)
	if e := os.WriteFile(filepath.Join(previous, "build-local.json"), content, 0644); e != nil {
		t.Fatal(e)
	}
	c, e := loadLocalConfig(current)
	if e != nil || c.ROMSource == "" {
		t.Fatalf("sibling discovery: %+v %v", c, e)
	}
	if e := os.WriteFile(filepath.Join(base, "build-local.json"), []byte(`{"romSource":"shared"}`), 0644); e != nil {
		t.Fatal(e)
	}
	c, e = loadLocalConfig(current)
	if e != nil || c.ROMSource != "shared" {
		t.Fatalf("shared priority: %+v %v", c, e)
	}
	if e := os.WriteFile(filepath.Join(current, "build-local.json"), []byte(`{"romSource":"local"}`), 0644); e != nil {
		t.Fatal(e)
	}
	c, e = loadLocalConfig(current)
	if e != nil || c.ROMSource != "local" {
		t.Fatalf("local priority: %+v %v", c, e)
	}
}
