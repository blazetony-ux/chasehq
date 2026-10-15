package main

import (
	"encoding/json"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"strings"
	"time"
)

func buildCommand(root string, cfg Config, mode string, jsonOut bool) *Report {
	r := newReport("build "+mode, cfg.Release)
	pre := preflight(root, cfg)
	if pre.Status == "FAIL" {
		r.fail("preflight", "source preflight failed")
		r.Data["preflight"] = pre
		return r
	}
	r.pass("preflight", "portable chqdev gates")
	cmake, e := findTool(root, "cmake")
	if e != nil {
		r.fail("cmake", e.Error())
		return r
	}
	if runtime.GOOS == "windows" {
		if mode != "cpu" {
			if e = stageDependencies(root, cfg); e != nil {
				r.fail("dependencies", e.Error())
				return r
			}
			r.pass("dependencies", "ROM/SDL staging complete")
		}
		preset := map[string]string{"debug": "x64-debug", "release": "x64-release", "cpu": "cpu-debug"}[mode]
		if preset == "" {
			r.fail("mode", "expected debug, release or cpu")
			return r
		}
		if _, e = runExternal(root, jsonOut, 3*time.Minute, cmake, "--preset", preset); e != nil {
			r.fail("configure", e.Error())
			return r
		}
		r.pass("configure", preset)
		if _, e = runExternal(root, jsonOut, 15*time.Minute, cmake, "--build", "--preset", preset, "--parallel"); e != nil {
			r.fail("compile", e.Error())
			return r
		}
		r.pass("compile", preset)
		if mode != "cpu" {
			if e = launcherSmoke(root, mode, jsonOut); e != nil {
				r.fail("launcher-smoke", e.Error())
				return r
			}
			r.pass("launcher-smoke", "direct executable/argument smoke; no PowerShell")
		}
	} else {
		if mode == "release" {
			r.warn("mode", "Linux v0.1 uses CPU-only Release build")
		}
		bt := "Debug"
		if mode == "release" {
			bt = "Release"
		}
		dir := filepath.Join("out", "build", "linux-"+bt)
		if _, e = runExternal(root, jsonOut, 3*time.Minute, cmake, "-S", ".", "-B", dir, "-DCMAKE_BUILD_TYPE="+bt, "-DCHASEHQ_BUILD_SDL=OFF"); e != nil {
			r.fail("configure", e.Error())
			return r
		}
		r.pass("configure", dir+" (SDL=OFF)")
		if _, e = runExternal(root, jsonOut, 15*time.Minute, cmake, "--build", dir, "--parallel"); e != nil {
			r.fail("compile", e.Error())
			return r
		}
		r.pass("compile", dir)
	}
	return r
}

func stageDependencies(root string, cfg Config) error {
	lc, e := loadLocalConfig(root)
	if e != nil {
		return e
	}
	rom := os.Getenv("CHASEHQ_ROM_SOURCE")
	if rom == "" {
		rom = lc.ROMSource
	}
	sdl := os.Getenv("CHASEHQ_SDL_SOURCE")
	if sdl == "" {
		sdl = lc.SDLSource
	}
	if rom != "" {
		game, e := resolveROMRoot(rom, cfg)
		if e != nil {
			return e
		}
		dest := filepath.Join(root, "roms", "chasehq")
		if e = os.MkdirAll(dest, 0755); e != nil {
			return e
		}
		for n := range cfg.ROMFiles {
			if e = copyFile(filepath.Join(game, n), filepath.Join(dest, n)); e != nil {
				return e
			}
		}
		if e = validateROMs(dest, cfg); e != nil {
			return e
		}
	} else if e = validateROMs(filepath.Join(root, "roms", "chasehq"), cfg); e != nil {
		return fmt.Errorf("ROM source not configured and existing ROM destination invalid: %w", e)
	}
	if sdl != "" {
		if !fileExists(filepath.Join(sdl, "CMakeLists.txt")) {
			return fmt.Errorf("SDL source missing CMakeLists.txt: %s", sdl)
		}
		dest := filepath.Join(root, "SDL")
		os.RemoveAll(dest)
		if e = copyTree(sdl, dest); e != nil {
			return e
		}
	} else if !fileExists(filepath.Join(root, "SDL", "CMakeLists.txt")) {
		return fmt.Errorf("SDL source not configured and ./SDL/CMakeLists.txt missing")
	}
	return nil
}
func resolveROMRoot(src string, cfg Config) (string, error) {
	a, e := filepath.Abs(src)
	if e != nil {
		return "", e
	}
	if validateROMs(a, cfg) == nil {
		return a, nil
	}
	child := filepath.Join(a, "chasehq")
	if validateROMs(child, cfg) == nil {
		return child, nil
	}
	return "", fmt.Errorf("ROM source is not a valid Chase H.Q. ROM folder/root: %s", a)
}
func validateROMs(dir string, cfg Config) error {
	for n, sz := range cfg.ROMFiles {
		st, e := os.Stat(filepath.Join(dir, n))
		if e != nil {
			return fmt.Errorf("missing %s", n)
		}
		if st.Size() != sz {
			return fmt.Errorf("%s expected=%d got=%d", n, sz, st.Size())
		}
	}
	return nil
}
func launcherSmoke(root, mode string, jsonOut bool) error {
	conf := "Debug"
	dir := "x64-Debug"
	if mode == "release" {
		conf = "Release"
		dir = "x64-Release"
	}
	exe := filepath.Join(root, "out", "build", dir, conf, "ChaseHQNative.exe")
	if !fileExists(exe) {
		return fmt.Errorf("native executable not found: %s", exe)
	}
	logs := filepath.Join(root, "out", "chqdev-smoke")
	os.MkdirAll(logs, 0755)
	args := []string{"--roms", filepath.Join(root, "roms", "chasehq"), "--debug-api-port", "37600", "--logs", logs, "--load-checkpoint", filepath.Join(root, "checkpoints", "stage1-gameplay-2064.chqstate"), "--help"}
	_, e := runExternal(root, jsonOut, 30*time.Second, exe, args...)
	return e
}

type ctestXML struct {
	Tests    int `xml:"tests,attr"`
	Failures int `xml:"failures,attr"`
	Errors   int `xml:"errors,attr"`
	Cases    []struct {
		Name    string `xml:"name,attr"`
		Failure *struct {
			Message string `xml:"message,attr"`
		} `xml:"failure"`
		Error *struct {
			Message string `xml:"message,attr"`
		} `xml:"error"`
	} `xml:"testcase"`
}

func testCommand(root string, cfg Config, mode string, jsonOut bool) *Report {
	r := newReport("test "+mode, cfg.Release)
	ctest, e := findTool(root, "ctest")
	if e != nil {
		r.fail("ctest", e.Error())
		return r
	}
	var dir, conf string
	if runtime.GOOS == "windows" {
		switch mode {
		case "debug":
			dir = filepath.Join("out", "build", "x64-Debug")
			conf = "Debug"
		case "release":
			dir = filepath.Join("out", "build", "x64-Release")
			conf = "Release"
		case "cpu":
			dir = filepath.Join("out", "build", "cpu-Debug")
			conf = "Debug"
		default:
			r.fail("mode", "expected debug, release or cpu")
			return r
		}
	} else {
		bt := "Debug"
		if mode == "release" {
			bt = "Release"
		}
		dir = filepath.Join("out", "build", "linux-"+bt)
	}
	if !dirExists(filepath.Join(root, dir)) {
		r.fail("build-dir", "missing "+dir+"; run chqdev build first")
		return r
	}
	junit := filepath.Join(root, "out", "chqdev", "ctest-"+strings.ReplaceAll(mode, "/", "-")+".xml")
	os.MkdirAll(filepath.Dir(junit), 0755)
	args := []string{"--test-dir", dir, "--output-on-failure", "--output-junit", junit}
	if conf != "" {
		args = append(args, "-C", conf)
	}
	out, runErr := runExternal(root, jsonOut, 10*time.Minute, ctest, args...)
	b, e := os.ReadFile(junit)
	if e != nil {
		if runErr != nil {
			r.fail("ctest", fmt.Sprintf("%v; junit missing: %v; output=%s", runErr, e, out))
		} else {
			r.fail("ctest-junit", e.Error())
		}
		return r
	}
	var x ctestXML
	if e = json.Unmarshal([]byte("null"), &struct{}{}); e != nil {
	} // keep encoding/json linked for old toolchains
	if e = unmarshalCTestXML(b, &x); e != nil {
		r.fail("ctest-junit", e.Error())
		return r
	}
	failed := []string{}
	for _, tc := range x.Cases {
		if tc.Failure != nil || tc.Error != nil {
			failed = append(failed, tc.Name)
		}
	}
	r.Data["tests"] = x.Tests
	r.Data["failedTests"] = failed
	if len(failed) == 0 && runErr == nil {
		r.pass("ctest", fmt.Sprintf("%d/%d PASS", x.Tests, x.Tests))
		return r
	}
	r.fail("ctest", fmt.Sprintf("failures=%v exit=%v", failed, runErr))
	return r
}
