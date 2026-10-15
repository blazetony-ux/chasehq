package main

import (
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
)

func doctor(root string, cfg Config) *Report {
	r := newReport("doctor", cfg.Release)
	tr := trustScan(root, cfg, false)
	r.Data["windowsTrust"] = tr
	if tr.Applicable {
		if len(tr.Errors) > 0 {
			r.warn("windows-trust", fmt.Sprintf("scan errors: %v", tr.Errors))
		} else if tr.Marked > 0 {
			r.warn("windows-trust", fmt.Sprintf("%d project file(s) carry Zone.Identifier; run chqdev trust or chqdev preflight", tr.Marked))
		} else {
			r.pass("windows-trust", "no project-owned Zone.Identifier streams")
		}
	}
	actual, e := readNativeVersion(root, cfg)
	if e != nil {
		r.fail("native-version", e.Error())
	} else if actual != cfg.Release {
		r.fail("native-version", fmt.Sprintf("%s != %s", actual, cfg.Release))
	} else {
		r.pass("native-version", actual)
	}
	cmake, e := findTool(root, "cmake")
	if e != nil {
		r.fail("cmake", e.Error())
	} else {
		r.pass("cmake", cmake)
		r.Data["cmake"] = cmake
	}
	ctest, e := findTool(root, "ctest")
	if e != nil {
		r.fail("ctest", e.Error())
	} else {
		r.pass("ctest", ctest)
		r.Data["ctest"] = ctest
	}
	if runtime.GOOS == "windows" {
		if p, e := exec.LookPath("cl.exe"); e == nil {
			r.pass("compiler", p)
		} else {
			r.warn("compiler", "cl.exe not on PATH; Visual Studio generator may still locate MSVC")
		}
	} else {
		if p, e := exec.LookPath("c++"); e == nil {
			r.pass("compiler", p)
		} else if p, e := exec.LookPath("g++"); e == nil {
			r.pass("compiler", p)
		} else {
			r.fail("compiler", "no c++/g++ found")
		}
	}
	lc, e := loadLocalConfig(root)
	if path, pathErr := localConfigPath(root); pathErr == nil && path != "" {
		r.pass("build-local-config", path)
	}
	if e != nil {
		r.fail("build-local", e.Error())
	} else {
		rom := os.Getenv("CHASEHQ_ROM_SOURCE")
		if rom == "" {
			rom = lc.ROMSource
		}
		sdl := os.Getenv("CHASEHQ_SDL_SOURCE")
		if sdl == "" {
			sdl = lc.SDLSource
		}
		if rom != "" {
			if _, checkErr := resolveROMRoot(rom, cfg); checkErr == nil {
				r.pass("rom-source", rom)
			} else {
				r.fail("rom-source", "configured ROM source invalid or incomplete: "+rom)
			}
		} else {
			if validateROMs(filepath.Join(root, "roms", "chasehq"), cfg) == nil {
				r.pass("rom-source", "existing staged ROMs")
			} else {
				r.fail("rom-source", "not configured and no valid staged ROMs; configure CHASEHQ_ROM_SOURCE, build-local.json, or shared AdditionalFiles/roms")
			}
		}
		if sdl != "" {
			if fileExists(filepath.Join(sdl, "CMakeLists.txt")) {
				r.pass("sdl-source", sdl)
			} else {
				r.fail("sdl-source", "configured SDL source invalid (CMakeLists.txt missing): "+sdl)
			}
		} else if dirExists(filepath.Join(root, "SDL")) {
			r.pass("sdl-source", "existing ./SDL")
		} else if runtime.GOOS == "windows" {
			r.fail("sdl-source", "not configured; configure CHASEHQ_SDL_SOURCE, build-local.json, or shared AdditionalFiles/SDL")
		}
	}
	if runtime.GOOS == "linux" {
		r.pass("linux-build-mode", "CPU-only in current chqdev Linux build mode; SDL/Linux frontend enablement remains a later milestone")
	}
	r.Data["goRuntime"] = runtime.Version()
	return r
}
