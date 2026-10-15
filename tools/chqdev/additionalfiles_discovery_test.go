package main

import (
	"os"
	"path/filepath"
	"testing"
)

func TestAdditionalFilesDiscoveryInWindowsStyleLayout(t *testing.T) {
	base := t.TempDir()
	root := filepath.Join(base, "ChaseHQ-Native-v2-NoGit", "ChaseHQ-Native-v0.67.0.0-RC6.2.4")
	rom := filepath.Join(base, "AdditionalFiles", "roms")
	sdl := filepath.Join(base, "AdditionalFiles", "SDL")
	for _, p := range []string{root, rom, sdl} {
		if err := os.MkdirAll(p, 0755); err != nil {
			t.Fatal(err)
		}
	}
	if err := os.WriteFile(filepath.Join(sdl, "CMakeLists.txt"), []byte("cmake_minimum_required(VERSION 3.16)\n"), 0644); err != nil {
		t.Fatal(err)
	}
	cfg, err := loadLocalConfig(root)
	if err != nil {
		t.Fatal(err)
	}
	if cfg.ROMSource != rom || cfg.SDLSource != sdl {
		t.Fatalf("got %+v, expected %s / %s", cfg, rom, sdl)
	}
}

func TestExplicitLocalConfigOverridesAdditionalFiles(t *testing.T) {
	base := t.TempDir()
	root := filepath.Join(base, "ChaseHQ-Native-v2-NoGit", "ChaseHQ-Native-v0.67.0.0-RC6.2.4")
	if err := os.MkdirAll(root, 0755); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(root, "build-local.json"), []byte(`{"romSource":"X:/rom","sdlSource":"X:/sdl"}`), 0644); err != nil {
		t.Fatal(err)
	}
	cfg, err := loadLocalConfig(root)
	if err != nil {
		t.Fatal(err)
	}
	if cfg.ROMSource != "X:/rom" || cfg.SDLSource != "X:/sdl" {
		t.Fatalf("explicit local config lost: %+v", cfg)
	}
}
