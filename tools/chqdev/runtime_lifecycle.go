package main

import (
	"fmt"
	"os/exec"
	"path/filepath"
	"runtime"
	"strconv"
	"strings"
	"time"
)

type RuntimeLaunchOptions struct {
	Mode        string
	Content     string
	ContentPack string
	Roms        string
	Checkpoint  string
	LocalOnly   bool
	LanAccess   bool
	NoBrowser   bool
	NoWeb       bool
	Port        int
	HTTPPort    int
	LanAddress  string
	AccessToken string
}

func runtimePowerShellArgs(root, action string, o RuntimeLaunchOptions) []string {
	args := []string{"-NoLogo", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", filepath.Join(root, "Start-ChaseHQ.ps1")}
	switch action {
	case "stop":
		return append(args, "-Stop")
	case "restart":
		args = append(args, "-Restart")
	}
	if o.ContentPack != "" {
		args = append(args, "-ContentPack", o.ContentPack)
	}
	if o.Roms != "" {
		args = append(args, "-Roms", o.Roms)
	}
	if o.Mode != "" {
		args = append(args, "-Mode", o.Mode)
	}
	if o.Checkpoint != "" {
		args = append(args, "-Checkpoint", o.Checkpoint)
	}
	if o.LocalOnly {
		args = append(args, "-LocalOnly")
	}
	if o.LanAccess {
		args = append(args, "-LanAccess")
	}
	if o.NoBrowser {
		args = append(args, "-NoBrowser")
	}
	if o.NoWeb {
		args = append(args, "-NoWeb")
	}
	if o.Port > 0 {
		args = append(args, "-Port", strconv.Itoa(o.Port))
	}
	if o.HTTPPort > 0 {
		args = append(args, "-HttpPort", strconv.Itoa(o.HTTPPort))
	}
	if o.LanAddress != "" {
		args = append(args, "-LanAddress", o.LanAddress)
	}
	if o.AccessToken != "" {
		args = append(args, "-AccessToken", o.AccessToken)
	}
	return args
}

func runtimeLifecycle(root string, cfg Config, action string, opts RuntimeLaunchOptions, jsonOut bool) *Report {
	r := newReport("runtime "+action, cfg.Release)
	if runtime.GOOS != "windows" {
		r.fail("runtime-lifecycle", "RC3.4.9 start/stop/restart currently bridges the Windows Workbench host; use runtime check on non-Windows hosts")
		return r
	}
	tr := trustScan(root, cfg, true)
	r.Data["trust"] = tr
	if len(tr.Errors) > 0 || tr.Remaining > 0 {
		r.fail("windows-trust", fmt.Sprintf("repair failed: remaining=%d errors=%v", tr.Remaining, tr.Errors))
		return r
	}
	r.pass("windows-trust", fmt.Sprintf("%d Zone.Identifier stream(s) removed; 0 remaining", tr.Removed))
	ps, err := exec.LookPath("powershell.exe")
	if err != nil {
		r.fail("powershell-host", "powershell.exe not found; Workbench runtime host is still PowerShell-backed in RC3.4.9")
		return r
	}
	if !fileExists(filepath.Join(root, "Start-ChaseHQ.ps1")) {
		r.fail("launcher", "Start-ChaseHQ.ps1 missing")
		return r
	}
	if action != "stop" {
		setupGhidraChildJava(r)
	}
	args := runtimePowerShellArgs(root, action, opts)
	r.Data["bridge"] = "Start-ChaseHQ.ps1"
	r.Data["bridgePolicy"] = "compatibility runtime host only; chqdev remains authoritative lifecycle entry point"
	if _, err = runExternal(root, jsonOut, 3*time.Minute, ps, args...); err != nil {
		r.fail("runtime-lifecycle", err.Error())
		return r
	}
	r.pass("runtime-lifecycle", action+" completed through chqdev compatibility bridge")
	if action != "stop" && !opts.NoWeb {
		rr := runtimeStartupCheck(root, cfg, "")
		r.Data["runtimeStartup"] = rr
		if rr.Status == "FAIL" {
			r.fail("runtime-readiness", "launcher completed but lightweight Workbench readiness check failed")
			return r
		}
		r.pass("runtime-readiness", "Workbench/native/mobile ready (lightweight startup gate)")
	}
	return r
}

func parseRuntimeOptions(args []string) (RuntimeLaunchOptions, string, error) {
	o := RuntimeLaunchOptions{}
	url := ""
	for i := 0; i < len(args); i++ {
		switch strings.ToLower(args[i]) {
		case "--mode":
			if i+1 >= len(args) {
				return o, url, fmt.Errorf("--mode requires a value")
			}
			o.Mode = args[i+1]
			i++
		case "--content":
			if i+1 >= len(args) {
				return o, url, fmt.Errorf("--content requires rom|pack|auto")
			}
			o.Content = args[i+1]
			i++
		case "--checkpoint":
			if i+1 >= len(args) {
				return o, url, fmt.Errorf("--checkpoint requires a path")
			}
			o.Checkpoint = args[i+1]
			i++
		case "--local-only":
			o.LocalOnly = true
		case "--lan-access":
			o.LanAccess = true
		case "--no-browser":
			o.NoBrowser = true
		case "--no-web":
			o.NoWeb = true
		case "--port":
			if i+1 >= len(args) {
				return o, url, fmt.Errorf("--port requires a number")
			}
			n, e := strconv.Atoi(args[i+1])
			if e != nil {
				return o, url, e
			}
			o.Port = n
			i++
		case "--http-port":
			if i+1 >= len(args) {
				return o, url, fmt.Errorf("--http-port requires a number")
			}
			n, e := strconv.Atoi(args[i+1])
			if e != nil {
				return o, url, e
			}
			o.HTTPPort = n
			i++
		case "--lan-address":
			if i+1 >= len(args) {
				return o, url, fmt.Errorf("--lan-address requires a value")
			}
			o.LanAddress = args[i+1]
			i++
		case "--access-token":
			if i+1 >= len(args) {
				return o, url, fmt.Errorf("--access-token requires a value")
			}
			o.AccessToken = args[i+1]
			i++
		case "--url":
			if i+1 >= len(args) {
				return o, url, fmt.Errorf("--url requires a value")
			}
			url = args[i+1]
			i++
		}
	}
	return o, url, nil
}
