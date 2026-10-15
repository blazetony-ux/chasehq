// Package core contains deterministic synthetic fixtures and independently
// compiled native algorithms modelled after RC4.7 C++ candidate semantics.
// No original ROMs or emulator code are used or embedded.
package core

import (
	"encoding/binary"
	"errors"
	"fmt"
)

const (
	RoadEntries    = 104
	SpriteCapacity = 256
)

type Scene struct {
	Frame     int
	Road      []uint16
	Source    []uint16
	SpriteRAM []uint32
	Selected  int
	Records   int
}

type SpriteInput struct {
	Records      int
	StageAddress uint32
	Words        []uint32
	ClearCount   int
}

// Transfer implements independent packed sprite-record copying and trailing clearing.
func Transfer(in SpriteInput, initial []uint32) ([]uint32, error) {
	if in.Records < 0 || in.Records > SpriteCapacity || in.ClearCount < 0 || in.Records+in.ClearCount > SpriteCapacity {
		return nil, errors.New("invalid sprite record count or clear count")
	}
	if in.StageAddress != uint32(0x102800-8*in.Records) || len(in.Words) != 2*in.Records || len(initial) != SpriteCapacity*2 {
		return nil, errors.New("invalid staging pointer or sprite-buffer size")
	}
	out := append([]uint32(nil), initial...)
	if in.Records == 0 {
		return out, nil
	}
	copy(out[:2*in.Records], in.Words)
	for i := 2 * in.Records; i < 2*(in.Records+in.ClearCount); i++ {
		out[i] = 0
	}
	return out, nil
}

// RoadMask implements the 104-entry boundary selection and source-word transformation.
func RoadMask(source, boundary []uint16, lookup []byte, first, second, palette uint16) ([]uint16, []uint16, int, error) {
	if len(source) != RoadEntries || len(boundary) != RoadEntries || len(lookup) != 1024 {
		return nil, nil, 0, errors.New("invalid road model input dimensions")
	}
	altered := append([]uint16(nil), source...)
	output := make([]uint16, RoadEntries)
	selected := 0
	hi := uint16(uint32(0xd200)+uint32(palette)) & 0xff00
	for i := 0; i < RoadEntries; i++ {
		if first >= boundary[i] && second < boundary[i] {
			output[i] = 0x9800
			altered[i] = hi | uint16(lookup[int(source[i]&0x03ff)])
			selected++
		} else {
			output[i] = 0x8800
		}
	}
	return altered, output, selected, nil
}

// Scenario is a clean-room synthetic scenario for showing native algorithm effects.
// It is NOT a capture from Chase H.Q. and never represents original rendered game assets.
func Scenario(frame int) (Scene, error) {
	if frame < 0 || frame >= 4 {
		return Scene{}, fmt.Errorf("synthetic frame outside 0..3: %d", frame)
	}
	n := 10 + frame
	stage := make([]uint32, n*2)
	for i := 0; i < n; i++ {
		stage[2*i] = uint32(0x91000000 | ((frame*6+i*11)&0xffff)<<8 | ((i*7 + frame) & 0xff))
		stage[2*i+1] = uint32(0x00600000 | ((i*41+frame*13)&0xffff)<<4 | ((i + 3) & 15))
	}
	init := make([]uint32, SpriteCapacity*2)
	for i := range init {
		init[i] = 0xcafe0000 | uint32(i)
	}
	ram, err := Transfer(SpriteInput{n, uint32(0x102800 - 8*n), stage, 2}, init)
	if err != nil {
		return Scene{}, err
	}
	source := make([]uint16, RoadEntries)
	boundary := make([]uint16, RoadEntries)
	lookup := make([]byte, 1024)
	for i := range lookup {
		lookup[i] = byte((i*19 + 29) & 0xff)
	}
	for i := 0; i < RoadEntries; i++ {
		source[i] = uint16((i*31 + frame*9) & 1023)
		boundary[i] = uint16(120 + i*22)
	}
	src, out, selected, err := RoadMask(source, boundary, lookup, uint16(1470+frame*100), uint16(300+frame*40), 0x2400)
	return Scene{Frame: frame, Road: out, Source: src, SpriteRAM: ram, Selected: selected, Records: n}, err
}

func Bytes16BE(words []uint16) []byte {
	b := make([]byte, len(words)*2)
	for i, w := range words {
		binary.BigEndian.PutUint16(b[i*2:], w)
	}
	return b
}
func Bytes32BE(words []uint32) []byte {
	b := make([]byte, len(words)*4)
	for i, w := range words {
		binary.BigEndian.PutUint32(b[i*4:], w)
	}
	return b
}
