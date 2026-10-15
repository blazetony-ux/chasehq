package main

import (
	"chasehq-native-preview-poc/core"
	"testing"
)

func TestSyntheticReference(t *testing.T) {
	for i := 0; i < 4; i++ {
		r, e := Validate(i, false)
		if e != nil {
			t.Fatal(e)
		}
		if r.Mismatches != 0 {
			t.Fatalf("case %d mismatch %s", i, r.FirstMismatch)
		}
	}
}
func TestMismatchDetection(t *testing.T) {
	r, e := Validate(0, true)
	if e != nil {
		t.Fatal(e)
	}
	if r.Mismatches != 1 || r.RoadOK {
		t.Fatalf("expected 1 deliberate mismatch, got %#v", r)
	}
}
func TestRecordBounds(t *testing.T) {
	_, err := core.Transfer(core.SpriteInput{Records: 257}, make([]uint32, 512))
	if err == nil {
		t.Fatal("must reject overflow")
	}
}
func TestZeroRecords(t *testing.T) {
	init := make([]uint32, 512)
	init[0] = 15
	out, e := core.Transfer(core.SpriteInput{Records: 0, StageAddress: 0x102800}, init)
	if e != nil || out[0] != 15 {
		t.Fatal("zero transfer mutated state")
	}
}
func TestRoadInvalidInput(t *testing.T) {
	_, _, _, e := core.RoadMask(nil, nil, nil, 0, 0, 0)
	if e == nil {
		t.Fatal("must reject undersized buffers")
	}
}
