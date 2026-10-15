//go:build !windows

package main

import (
	"fmt"
	"os"
)

func main() {
	fmt.Println("ChaseHQ-Native Preview POC — SYNTHETIC ROM-FREE VALIDATION (not RC5)")
	for i := 0; i < 4; i++ {
		v, err := Validate(i, false)
		if err != nil {
			fmt.Fprintln(os.Stderr, err)
			os.Exit(1)
		}
		fmt.Printf("case %d: road=%t source=%t sprite=%t mismatches=%d selected=%d records=%d\n", i, v.RoadOK, v.SourceOK, v.SpriteOK, v.Mismatches, v.Scene.Selected, v.Scene.Records)
		if v.Mismatches != 0 {
			os.Exit(1)
		}
	}
}
