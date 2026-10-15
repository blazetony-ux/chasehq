package main

import (
	"os"
	"path/filepath"
	"strings"
	"testing"
)

// Source-only regression protection: Windows gameplay parity still requires
// an actual SDL/PowerShell Full Regression execution on the exact archive.
func TestRC5FrameParityCapturesCompositorWithoutMutatingPresentedView(t *testing.T) {
	root := filepath.Join("..", "..")
	read := func(p string) string {
		t.Helper()
		b, err := os.ReadFile(filepath.Join(root, p))
		if err != nil {
			t.Fatal(err)
		}
		return string(b)
	}
	v := read("src/video.cpp")
	m := read("src/main.cpp")
	p := read("Start-ChaseHQWeb.ps1")
	h := read("src/video.h")
	required := []struct{ text, token string }{
		{v, "if (!compositor_capture && scene.show_prom_bits)"},
		{v, "if (!compositor_capture && scene.show_road_probe)"},
		{v, "if (!compositor_capture && diagnostics_frame(frame))"},
		{v, "if (!compositor_capture && diagnostic_background_"},
		{v, "if (!compositor_capture) {\n        if (research_workbench_)"},
		{m, "video.draw_runtime(machine,*runtime,scene,frame,debug_paused,timer_frozen,composited)"},
		{m, "if(composited)video.draw_runtime(machine,*runtime,scene,frame,debug_paused,timer_frozen)"},
		{m, "if(a.size()>2&&!composited)return"},
		{p, "Invoke-Chq @('screenshot',$livePath,'composited')"},
		{p, "require"},
		{p, "changedPixels"},
		{h, "bool compositor_capture = false"},
	}
	for _, pair := range required {
		if !strings.Contains(pair.text, pair.token) {
			t.Errorf("missing compositor-parity contract: %q", pair.token)
		}
	}
	if !strings.Contains(p, "if($require -and -not ([bool]$cmp.identical))") {
		t.Fatal("zero-diff parity gate missing")
	}
}
