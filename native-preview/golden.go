package main

import (
	"chasehq-native-preview-poc/core"
	"embed"
	"encoding/base64"
	"encoding/json"
	"fmt"
)

//go:embed golden.json
var embeddedGolden embed.FS

type oracleCase struct {
	Frame    int    `json:"frame"`
	Sprite   string `json:"sprite"`
	Source   string `json:"source"`
	Road     string `json:"road"`
	Selected int    `json:"selected"`
	Records  int    `json:"records"`
}
type oracle struct {
	Description string       `json:"description"`
	Cases       []oracleCase `json:"cases"`
}
type Validation struct {
	Scene         core.Scene
	SpriteOK      bool
	RoadOK        bool
	SourceOK      bool
	SelectedOK    bool
	Mismatches    int
	FirstMismatch string
}

func getOracle() (oracle, error) {
	var o oracle
	b, e := embeddedGolden.ReadFile("golden.json")
	if e != nil {
		return o, e
	}
	e = json.Unmarshal(b, &o)
	return o, e
}
func compareBytes(label string, got []byte, encoded string, v *Validation) bool {
	want, err := base64.StdEncoding.DecodeString(encoded)
	if err != nil {
		v.Mismatches++
		v.FirstMismatch = label + ": fixture decode error"
		return false
	}
	ok := true
	if len(got) != len(want) {
		v.Mismatches++
		v.FirstMismatch = fmt.Sprintf("%s length %d vs %d", label, len(got), len(want))
		return false
	}
	for i := range got {
		if got[i] != want[i] {
			v.Mismatches++
			if v.FirstMismatch == "" {
				v.FirstMismatch = fmt.Sprintf("%s byte %d: model %02X vs reference %02X", label, i, got[i], want[i])
			}
			ok = false
		}
	}
	return ok
}
func Validate(frame int, inject bool) (Validation, error) {
	sc, err := core.Scenario(frame)
	if err != nil {
		return Validation{}, err
	}
	o, err := getOracle()
	if err != nil {
		return Validation{}, err
	}
	if frame >= len(o.Cases) {
		return Validation{}, fmt.Errorf("no reference for frame %d", frame)
	}
	c := o.Cases[frame]
	v := Validation{Scene: sc, SelectedOK: sc.Selected == c.Selected && sc.Records == c.Records}
	sprite := core.Bytes32BE(sc.SpriteRAM)
	road := core.Bytes16BE(sc.Road)
	source := core.Bytes16BE(sc.Source)
	if inject {
		road[10] ^= 1
		sc.Road[5] ^= 1
		v.Scene = sc
	}
	v.SpriteOK = compareBytes("sprite", sprite, c.Sprite, &v)
	v.RoadOK = compareBytes("road", road, c.Road, &v)
	v.SourceOK = compareBytes("source", source, c.Source, &v)
	if !v.SelectedOK {
		v.Mismatches++
		if v.FirstMismatch == "" {
			v.FirstMismatch = "selected material/record count differs"
		}
	}
	if v.Mismatches == 0 && (!v.SpriteOK || !v.RoadOK || !v.SourceOK || !v.SelectedOK) {
		return v, fmt.Errorf("invalid comparison")
	}
	return v, nil
}
