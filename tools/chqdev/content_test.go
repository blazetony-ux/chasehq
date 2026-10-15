package main

import (
	"os"
	"path/filepath"
	"testing"
)

func TestContentModes(t *testing.T) {
	root := t.TempDir()
	if err := os.MkdirAll(filepath.Join(root, "roms", "chasehq"), 0755); err != nil {
		t.Fatal(err)
	}
	s, e := resolveContent(root, "")
	if e != nil || s.Mode != "rom" {
		t.Fatalf("default ROM: %+v %v", s, e)
	}
	if _, e = resolveContent(root, "pack"); e == nil {
		t.Fatal("explicit pack must fail when missing")
	}
	if err := os.MkdirAll(filepath.Join(root, "content"), 0755); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(root, "content", "chasehq-world.chqpack"), []byte("placeholder"), 0644); err != nil {
		t.Fatal(err)
	}
	s, e = resolveContent(root, "auto")
	if e != nil || s.Mode != "pack" {
		t.Fatalf("auto pack: %+v %v", s, e)
	}
	if err := os.RemoveAll(filepath.Join(root, "roms")); err != nil {
		t.Fatal(err)
	}
	s, e = resolveContent(root, "pack")
	if e != nil || s.Mode != "pack" {
		t.Fatalf("pack without ROMs: %+v %v", s, e)
	}
	if _, e = resolveContent(root, "rom"); e == nil {
		t.Fatal("explicit ROM must fail when missing")
	}
}
func TestContentRuntimeArguments(t *testing.T) {
	o, url, e := parseRuntimeOptions([]string{"--content", "pack", "--no-browser"})
	if e != nil || url != "" || o.Content != "pack" || !o.NoBrowser {
		t.Fatalf("parse: %+v %q %v", o, url, e)
	}
	args := runtimePowerShellArgs("C:/project", "start", RuntimeLaunchOptions{ContentPack: "C:/pack.chqpack"})
	found := false
	for i := 0; i < len(args)-1; i++ {
		if args[i] == "-ContentPack" && args[i+1] == "C:/pack.chqpack" {
			found = true
		}
	}
	if !found {
		t.Fatal("missing pack forwarding")
	}
}

func TestSharedContentPackLocation(t *testing.T) {
	parent := t.TempDir()
	release := filepath.Join(parent, "ChaseHQ-Native-v0.67.0.0-RC4.3.2")
	if e := os.MkdirAll(release, 0755); e != nil {
		t.Fatal(e)
	}
	shared := filepath.Join(parent, "permanent", "content", "chasehq-world.chqpack")
	if e := os.MkdirAll(filepath.Dir(shared), 0755); e != nil {
		t.Fatal(e)
	}
	if e := os.WriteFile(shared, []byte("test only"), 0644); e != nil {
		t.Fatal(e)
	}
	local := `{"romSource":"/test/roms","sdlSource":"/test/sdl","copyRoms":true,"copySdl":true,"contentPackSource":` + "\"" + filepath.ToSlash(shared) + "\"" + `}`
	if e := os.WriteFile(filepath.Join(parent, "build-local.json"), []byte(local), 0644); e != nil {
		t.Fatal(e)
	}
	c, e := contentSettingsRead(release)
	if e != nil || c.Pack != shared {
		t.Fatalf("shared pack path not resolved: %#v %v", c, e)
	}
	// Per-release explicit setting takes precedence over shared local configuration.
	if e := os.MkdirAll(filepath.Join(release, ".chq"), 0755); e != nil {
		t.Fatal(e)
	}
	if e := os.WriteFile(filepath.Join(release, contentSettingsFile), []byte(`{"mode":"auto","pack":"my-private.chqpack","roms":"roms/chasehq"}`), 0644); e != nil {
		t.Fatal(e)
	}
	c, e = contentSettingsRead(release)
	if e != nil || c.Pack != "my-private.chqpack" {
		t.Fatalf("explicit content source overridden: %#v %v", c, e)
	}
}
