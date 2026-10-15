package main

import (
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestRC51SpriteComparisonAtCompletedBoundary(t *testing.T) {
	root := filepath.Join("..", "..")
	code, err := os.ReadFile(filepath.Join(root, "src", "live_candidate_shadow.cpp"))
	if err != nil {
		t.Fatal(err)
	}
	s := string(code)
	for _, want := range []string{"pc==0x40576", "sprite_bulk_end(peek)", "pc==0x40174", "sprite_end(peek)", "sprite_incomplete_", "bulk_pass_", "replacementEnabled\\\":false"} {
		if !strings.Contains(s, want) {
			t.Errorf("live shadow missing %s", want)
		}
	}
	if strings.Contains(s, "pc==0x40576 && sprite_pending_)sprite_end") {
		t.Fatal("early full sprite comparison is forbidden")
	}
	p, err := os.ReadFile(filepath.Join(root, "tools", "Run-RC5.1-Native-Shadow-Corpus.ps1"))
	if err != nil {
		t.Fatal(err)
	}
	for _, want := range []string{"sprite-peak-8647", "palette-transition-7507", "class06-5587", "OBSERVED_ALL_MATCH", "MISMATCH_OR_INCOMPLETE", "Compress-Archive", "CHQ_NATIVE_SPRITE_TRANSFER_SHADOW", "Resolve-ShadowCheckpoint", "--patch-at-frame", "5587:B:8:109064:", "road-class4-5587", "road-class6-5587", "ERROR_MISSING_REQUIRED", "probe-errors.txt"} {
		if !strings.Contains(string(p), want) {
			t.Errorf("consolidated probe missing %s", want)
		}
	}
}
