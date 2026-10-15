package main

import (
	"encoding/json"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"strings"
	"time"
)

const toolVersion = "0.5.52"

type Check struct {
	Name   string `json:"name"`
	Status string `json:"status"`
	Detail string `json:"detail,omitempty"`
}

type Report struct {
	Schema      string         `json:"schema"`
	ToolVersion string         `json:"toolVersion"`
	Command     string         `json:"command"`
	Release     string         `json:"release,omitempty"`
	Platform    string         `json:"platform"`
	Status      string         `json:"status"`
	Checks      []Check        `json:"checks,omitempty"`
	Data        map[string]any `json:"data,omitempty"`
	DurationMS  int64          `json:"durationMs"`
	started     time.Time
}

func newReport(cmd, release string) *Report {
	return &Report{Schema: "chqdev-report-v1", ToolVersion: toolVersion, Command: cmd, Release: release, Platform: runtime.GOOS + "/" + runtime.GOARCH, Status: "PASS", Data: map[string]any{}, started: time.Now()}
}
func (r *Report) pass(name, detail string) {
	r.Checks = append(r.Checks, Check{Name: name, Status: "PASS", Detail: detail})
}
func (r *Report) warn(name, detail string) {
	r.Checks = append(r.Checks, Check{Name: name, Status: "WARN", Detail: detail})
}
func (r *Report) fail(name, detail string) {
	r.Checks = append(r.Checks, Check{Name: name, Status: "FAIL", Detail: detail})
	r.Status = "FAIL"
}
func (r *Report) finish() { r.DurationMS = time.Since(r.started).Milliseconds() }

func main() {
	rootArg, jsonOut, args, err := parseGlobal(os.Args[1:])
	if err != nil {
		fmt.Fprintln(os.Stderr, "chqdev:", err)
		os.Exit(2)
	}
	if len(args) == 0 || args[0] == "help" || args[0] == "--help" || args[0] == "-h" {
		usage()
		return
	}
	root, err := findRoot(rootArg)
	if err != nil {
		fmt.Fprintln(os.Stderr, "chqdev:", err)
		os.Exit(2)
	}
	cfg, err := loadConfig(root)
	if err != nil {
		fmt.Fprintln(os.Stderr, "chqdev:", err)
		os.Exit(2)
	}

	cmd := args[0]
	rest := args[1:]
	var rep *Report
	switch cmd {
	case "version":
		rep = newReport("version", cfg.Release)
		actual, e := readNativeVersion(root, cfg)
		if e != nil {
			rep.fail("native-version", e.Error())
		} else {
			rep.pass("native-version", actual)
			rep.Data["nativeVersion"] = actual
		}
		rep.Data["chqdevVersion"] = toolVersion
	case "assets":
		rep = assetsCommand(root, cfg, rest)
	case "rea":
		rep = reaCommand(root, cfg, rest)
	case "native":
		if len(rest) > 0 && rest[0] == "minimize" {
			rep = rc6332ReplayCommand(root, cfg, rest[1:])
		} else if len(rest) > 0 && rest[0] == "replay-fixture" {
			rep = rc6332ReplayFixture(rest[1:])
		} else if len(rest) > 0 && rest[0] == "diagnose" {
			rep = rc6331DiagnoseCommand(root, cfg, rest[1:])
		} else if len(rest) > 0 && rest[0] == "reconstruct" {
			rep = nativeReconstructCommand(root, cfg, rest)
		} else {
			rep = nativeAcceptCommand(root, cfg, rest)
		}
	case "ghidra":
		rep = ghidraCommand(root, cfg, rest)
	case "doctor":
		rep = doctor(root, cfg)
	case "trust":
		repair := true
		for _, a := range rest {
			if a == "--check" {
				repair = false
			}
		}
		rep = trustCommand(root, cfg, repair)
	case "preflight":
		rep = preflight(root, cfg)
	case "build":
		mode := "debug"
		if len(rest) > 0 {
			mode = strings.ToLower(rest[0])
		}
		rep = buildCommand(root, cfg, mode, jsonOut)
	case "test":
		mode := "debug"
		if len(rest) > 0 {
			mode = strings.ToLower(rest[0])
		}
		rep = testCommand(root, cfg, mode, jsonOut)
	case "package":
		out := ""
		for i := 0; i < len(rest); i++ {
			if rest[i] == "--output" && i+1 < len(rest) {
				out = rest[i+1]
				i++
			}
		}
		rep = packageCommand(root, cfg, out, jsonOut)
	case "verify":
		if len(rest) == 0 {
			fmt.Fprintln(os.Stderr, "chqdev verify requires a ZIP path")
			os.Exit(2)
		}
		rep = verifyCommand(root, cfg, rest[0])
	case "content":
		rep = contentCommand(root, cfg, rest)
	case "runtime":
		mode := "check"
		if len(rest) > 0 && !strings.HasPrefix(rest[0], "--") {
			mode = strings.ToLower(rest[0])
			rest = rest[1:]
		}
		opts, url, pe := parseRuntimeOptions(rest)
		if pe != nil {
			fmt.Fprintln(os.Stderr, "chqdev runtime:", pe)
			os.Exit(2)
		}
		switch mode {
		case "check":
			rep = runtimeCheck(root, cfg, url)
		case "start", "stop", "restart":
			if mode != "stop" {
				source, e := resolveContent(root, opts.Content)
				if e != nil {
					fmt.Fprintln(os.Stderr, "chqdev runtime:", e)
					os.Exit(2)
				}
				opts.ContentPack, opts.Roms = source.Pack, source.Roms
			}
			rep = runtimeLifecycle(root, cfg, mode, opts, jsonOut)
		default:
			fmt.Fprintln(os.Stderr, "chqdev runtime supports: check|start|stop|restart")
			os.Exit(2)
		}
	case "regression":
		kind, url := "focused", ""
		if len(rest) > 0 && !strings.HasPrefix(rest[0], "--") {
			kind = strings.ToLower(rest[0])
		}
		for i := 0; i < len(rest); i++ {
			if rest[i] == "--url" && i+1 < len(rest) {
				url = rest[i+1]
				i++
			}
		}
		rep = regressionCommand(root, cfg, kind, url)
	case "qualify":
		mode, url := "preflight", ""
		for i := 0; i < len(rest); i++ {
			switch rest[i] {
			case "--build":
				mode = "build"
			case "--package":
				mode = "package"
			case "--runtime":
				mode = "runtime"
			case "--focused":
				mode = "focused"
			case "--full":
				mode = "full"
			case "--all":
				mode = "all"
			case "--fresh":
				mode = "fresh"
			case "--preflight":
				mode = "preflight"
			case "--url":
				if i+1 < len(rest) {
					url = rest[i+1]
					i++
				}
			}
		}
		rep = qualifyCommand(root, cfg, mode, url, jsonOut)
	case "status":
		rep = statusCommand(root, cfg)
	default:
		fmt.Fprintf(os.Stderr, "chqdev: unknown command %q\n\n", cmd)
		usage()
		os.Exit(2)
	}
	rep.finish()
	render(rep, jsonOut)
	if rep.Status == "FAIL" {
		os.Exit(1)
	}
}

func parseGlobal(args []string) (string, bool, []string, error) {
	root := ""
	jsonOut := false
	rest := make([]string, 0, len(args))
	for i := 0; i < len(args); i++ {
		switch args[i] {
		case "--json":
			jsonOut = true
		case "--root":
			if i+1 >= len(args) {
				return "", false, nil, errors.New("--root requires a path")
			}
			root = args[i+1]
			i++
		default:
			rest = append(rest, args[i])
		}
	}
	return root, jsonOut, rest, nil
}

func findRoot(explicit string) (string, error) {
	if explicit != "" {
		a, e := filepath.Abs(explicit)
		if e != nil {
			return "", e
		}
		if fileExists(filepath.Join(a, "chqdev.json")) {
			return a, nil
		}
		return "", fmt.Errorf("%s does not contain chqdev.json", a)
	}
	starts := []string{}
	if cwd, e := os.Getwd(); e == nil {
		starts = append(starts, cwd)
	}
	if exe, e := os.Executable(); e == nil {
		starts = append(starts, filepath.Dir(exe))
	}
	for _, s := range starts {
		p := s
		for {
			if fileExists(filepath.Join(p, "chqdev.json")) && fileExists(filepath.Join(p, "src", "version.h")) {
				return p, nil
			}
			q := filepath.Dir(p)
			if q == p {
				break
			}
			p = q
		}
	}
	return "", errors.New("cannot locate ChaseHQ-Native root (chqdev.json + src/version.h)")
}

func render(r *Report, jsonOut bool) {
	if jsonOut {
		b, _ := json.MarshalIndent(r, "", "  ")
		fmt.Println(string(b))
		return
	}
	fmt.Printf("chqdev %s | %s | %s\n", toolVersion, r.Command, r.Platform)
	if r.Release != "" {
		fmt.Printf("release: %s\n", r.Release)
	}
	for _, c := range r.Checks {
		if c.Detail != "" {
			fmt.Printf("[%s] %s - %s\n", c.Status, c.Name, c.Detail)
		} else {
			fmt.Printf("[%s] %s\n", c.Status, c.Name)
		}
	}
	fmt.Printf("CHQDEV %s: %s (%d ms)\n", strings.ToUpper(r.Command), r.Status, r.DurationMS)
}

func usage() {
	fmt.Printf(`ChaseHQ-Native chqdev v%s

Usage:
  chqdev [--root PATH] [--json] doctor
  chqdev [--root PATH] [--json] trust [--check]
  chqdev [--root PATH] [--json] preflight
  chqdev [--root PATH] [--json] build [debug|release|cpu]
  chqdev [--root PATH] [--json] test [debug|release|cpu]
  chqdev [--root PATH] [--json] package [--output PATH]
  chqdev [--root PATH] [--json] verify <archive.zip>
  chqdev [--root PATH] [--json] content mode rom|pack|auto
  chqdev [--root PATH] [--json] content status|verify|import
  chqdev [--root PATH] [--json] assets export|verify|overlay --pack PACK --out FOLDER [--overlay FILE]
  chqdev [--root PATH] [--json] rea crosswalk --static JSON --census TSV --out JSON
  chqdev [--root PATH] [--json] native accept --fresh [--ghidra-zip ZIP] [--out DIRECTORY]
  chqdev [--root PATH] [--json] native reconstruct --fresh [--live-health] [--cpu-b-discover] [--cpu-b-live] [--ghidra-zip ZIP] [--out DIRECTORY]
  chqdev [--root PATH] [--json] native diagnose --candidate ID --original TSV --native TSV --out JSON
  chqdev [--root PATH] [--json] native minimize --plan PLAN.json --out NEW_DIRECTORY
  chqdev [--root PATH] [--json] runtime check [--url URL]
  chqdev [--root PATH] [--json] runtime start|stop|restart [--mode MODE] [--content rom|pack|auto] [--checkpoint PATH] [--local-only|--lan-access] [--no-browser]
  chqdev [--root PATH] [--json] regression [focused|full] [--url URL]
  chqdev [--root PATH] [--json] qualify [--fresh|--preflight|--build|--package|--runtime|--focused|--full|--all] [--url URL]
  chqdev [--root PATH] [--json] status
  chqdev [--root PATH] [--json] version

Current rule: chqdev is the de facto/authoritative build-release and fresh qualification/runtime-lifecycle entry point. Fresh Windows qualification is .\chqdev.bat qualify --fresh; version-specific qualification wrappers are not shipped.
`, toolVersion)
}

func fileExists(p string) bool { st, e := os.Stat(p); return e == nil && !st.IsDir() }
func dirExists(p string) bool  { st, e := os.Stat(p); return e == nil && st.IsDir() }
