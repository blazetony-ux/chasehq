package main

import (
	"bytes"
	"image"
	"image/png"
	"os"
	"path/filepath"
	"testing"
)

func TestNativeAssetCatalogueRoundTripAndOverlay(t *testing.T) {
	root := t.TempDir()
	pack := filepath.Join(root, "original.chqpack")
	folder := filepath.Join(root, "native-assets")
	original := map[string][]byte{}
	for _, name := range assetRegions() {
		_, stride := assetLayout(name)
		raw := make([]byte, stride*3)
		for i := range raw {
			raw[i] = byte((i*17 + i/13) % 256)
		}
		original[name] = raw
	}
	original["cpu/main.bin"] = []byte{1, 2, 3, 4}
	if e := writeAssetPack(pack, original); e != nil {
		t.Fatal(e)
	}
	if _, e := exportAssetCatalogue(pack, folder); e != nil {
		t.Fatal(e)
	}
	if _, changed, e := rebuiltCatalogue(pack, folder); e != nil || changed != 0 {
		t.Fatalf("unchanged exports must roundtrip byte-exact: %d %v", changed, e)
	}
	// Editing one 16x16 indexed pixel must produce a sparse, correctly placed
	// bitplane overlay. No CPU program may enter this overlay.
	path := filepath.Join(folder, "sprites", "obj-a.png")
	f, e := os.Open(path)
	if e != nil {
		t.Fatal(e)
	}
	decoded, e := png.Decode(f)
	f.Close()
	if e != nil {
		t.Fatal(e)
	}
	im := decoded.(*image.Paletted)
	before := im.ColorIndexAt(0, 0)
	im.SetColorIndex(0, 0, (before+1)%16)
	f, e = os.Create(path)
	if e != nil {
		t.Fatal(e)
	}
	if e = png.Encode(f, im); e != nil {
		t.Fatal(e)
	}
	f.Close()
	replacement, changed, e := rebuiltCatalogue(pack, folder)
	if e != nil || changed != 1 {
		t.Fatalf("expected single modified region: %d %v", changed, e)
	}
	if len(replacement) != 1 || bytes.Equal(replacement["graphics/obj-a.bin"], original["graphics/obj-a.bin"]) {
		t.Fatal("overlay is not sparse or did not change pixel")
	}
	overlay := filepath.Join(root, "changes.chqoverlay")
	if e = writeAssetPack(overlay, replacement); e != nil {
		t.Fatal(e)
	}
	loaded, e := readAssetPack(overlay)
	if e != nil || len(loaded) != 1 || loaded["cpu/main.bin"] != nil {
		t.Fatalf("unsafe sparse overlay: %v", e)
	}
	// Source pack identity is mandatory; arbitrary assets cannot be applied to a different ROM set.
	original["cpu/main.bin"] = []byte{9, 9, 9, 9}
	if e = writeAssetPack(pack, original); e != nil {
		t.Fatal(e)
	}
	if _, _, e = rebuiltCatalogue(pack, folder); e == nil {
		t.Fatal("source identity change was accepted")
	}
}

func TestNativeAssetRejectsNonIndexedEditorOutput(t *testing.T) {
	root := t.TempDir()
	pack := filepath.Join(root, "base.chqpack")
	folder := filepath.Join(root, "a")
	raw := map[string][]byte{}
	for _, name := range assetRegions() {
		_, stride := assetLayout(name)
		raw[name] = make([]byte, stride)
	}
	if e := writeAssetPack(pack, raw); e != nil {
		t.Fatal(e)
	}
	if _, e := exportAssetCatalogue(pack, folder); e != nil {
		t.Fatal(e)
	}
	if e := os.WriteFile(filepath.Join(folder, "sprites", "obj-a.png"), []byte("not a png"), 0644); e != nil {
		t.Fatal(e)
	}
	if _, _, e := rebuiltCatalogue(pack, folder); e == nil {
		t.Fatal("corrupt edited graphic was accepted")
	}
}
