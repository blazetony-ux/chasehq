package main

import (
	"os"
	"path/filepath"
	"strings"
	"testing"
)

// Reproduces the actual RC6.2.4 Windows census evidence rather than a made-up
// CPU/PC spelling. The real run recorded 240 original ROM execution hits.
func TestRC625RealWindowsCensusCanonicalisation(t *testing.T) {
	testCases := []struct {
		name, rows string
		want       int64
	}{
		{"real Windows upper-PC", "cpu\tpc\tinstructionHits\nCPU-A\t0x00820C\t240\nCPU-B\t0x000100\t17\n", 240},
		{"uppercase numeric prefix", "cpu\tpc\tinstructionHits\nCPU-A\t0X00820C\t240\n", 240},
		{"lowercase suffix", "cpu\tpc\tinstructionHits\ncpu-a\t0x00820c\t240\n", 240},
		{"zero hit valid", "cpu\tpc\tinstructionHits\nCPU-A\t0x00820C\t0\n", 0},
	}
	for _, tt := range testCases {
		t.Run(tt.name, func(t *testing.T) {
			p := filepath.Join(t.TempDir(), "rom-execution-pc-counts.tsv")
			if err := os.WriteFile(p, []byte(tt.rows), 0644); err != nil {
				t.Fatal(err)
			}
			got, err := readNativeTsv(p)
			if err != nil {
				t.Fatal(err)
			}
			if got["A:0x00820C"] != tt.want {
				t.Fatalf("M2 writer count=%d want=%d from real-formatted row; got=%v", got["A:0x00820C"], tt.want, got)
			}
		})
	}
}
func TestRC625CensusRejectsInvalidAndAliasDuplicates(t *testing.T) {
	for _, rows := range []string{
		"cpu\tpc\tinstructionHits\nCPU-A\t0x00820C\t240\nCPU-A\t0X00820c\t240\n",
		"cpu\tpc\tinstructionHits\nCPU-C\t0x00820C\t240\n",
		"cpu\tpc\tinstructionHits\nCPU-A\t0x00820C\t-1\n",
		"cpu\tpc\tinstructionHits\nCPU-A\t0x00820D\t1\n",
		"cpu\tpc\tinstructionHits\nCPU-A\tUNKNOWN\t1\n",
	} {
		p := filepath.Join(t.TempDir(), "bad.tsv")
		if e := os.WriteFile(p, []byte(rows), 0644); e != nil {
			t.Fatal(e)
		}
		if _, e := readNativeTsv(p); e == nil {
			t.Errorf("accepted bad census %q", strings.TrimSpace(rows))
		}
	}
}
func TestRC625SignedSteeringCodeContract(t *testing.T) {
	root := testProjectRoot(t)
	conversion, e := os.ReadFile(filepath.Join(root, "src", "native_steering_conversion.cpp"))
	if e != nil {
		t.Fatal(e)
	}
	runtimeCPP, e := os.ReadFile(filepath.Join(root, "src", "runtime.cpp"))
	if e != nil {
		t.Fatal(e)
	}
	header, e := os.ReadFile(filepath.Join(root, "src", "steering_conversion.h"))
	if e != nil {
		t.Fatal(e)
	}
	for _, item := range []struct {
		label   string
		content []byte
		token   string
	}{
		{"shadow steering", conversion, "steering_ext_w_index(sample.raw)"},
		{"native guard", runtimeCPP, "signedIndex=chasehq::native::steering_ext_w_index(raw)"},
		{"native A0 contract", runtimeCPP, "0x3132c+2*signedIndex"},
		{"generated C++ library argument", runtimeCPP, "native_steering_generated_fn_(signedIndex)"},
		{"low-byte EXT.W function", header, "low - 0x100"},
	} {
		if !strings.Contains(string(item.content), item.token) {
			t.Errorf("%s does not use sign-extended index", item.label)
		}
	}
}
