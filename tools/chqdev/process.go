package main

import (
	"bytes"
	"context"
	"fmt"
	"io"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"
	"time"
)

func findTool(root, name string) (string, error) {
	exe := name
	if runtime.GOOS == "windows" && !strings.HasSuffix(strings.ToLower(name), ".exe") {
		exe = name + ".exe"
	}
	local := filepath.Join(root, "tools", "cmake", "bin", exe)
	if fileExists(local) {
		return local, nil
	}
	if p, e := exec.LookPath(exe); e == nil {
		return p, nil
	}
	if runtime.GOOS == "windows" {
		pf := os.Getenv("ProgramFiles(x86)")
		if pf == "" {
			pf = os.Getenv("ProgramFiles")
		}
		vswhere := filepath.Join(pf, "Microsoft Visual Studio", "Installer", "vswhere.exe")
		if fileExists(vswhere) {
			out, e := exec.Command(vswhere, "-latest", "-products", "*", "-requires", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64", "-property", "installationPath").Output()
			if e == nil {
				base := strings.TrimSpace(string(out))
				if base != "" {
					p := filepath.Join(base, "Common7", "IDE", "CommonExtensions", "Microsoft", "CMake", "CMake", "bin", exe)
					if fileExists(p) {
						return p, nil
					}
				}
			}
		}
	}
	return "", fmt.Errorf("%s not found", name)
}

func runExternal(root string, jsonOut bool, timeout time.Duration, exe string, args ...string) (string, error) {
	ctx, cancel := context.WithTimeout(context.Background(), timeout)
	defer cancel()
	cmd := exec.CommandContext(ctx, exe, args...)
	cmd.Dir = root
	if !jsonOut {
		cmd.Stdout = os.Stdout
		cmd.Stderr = os.Stderr
		e := cmd.Run()
		if ctx.Err() == context.DeadlineExceeded {
			return "", fmt.Errorf("timeout after %s", timeout)
		}
		return "", e
	}
	var b bytes.Buffer
	cmd.Stdout = &b
	cmd.Stderr = &b
	e := cmd.Run()
	if ctx.Err() == context.DeadlineExceeded {
		return b.String(), fmt.Errorf("timeout after %s", timeout)
	}
	return b.String(), e
}

func copyFile(src, dst string) error {
	in, e := os.Open(src)
	if e != nil {
		return e
	}
	defer in.Close()
	if e = os.MkdirAll(filepath.Dir(dst), 0755); e != nil {
		return e
	}
	st, e := in.Stat()
	if e != nil {
		return e
	}
	out, e := os.OpenFile(dst, os.O_CREATE|os.O_WRONLY|os.O_TRUNC, st.Mode().Perm())
	if e != nil {
		return e
	}
	_, ce := io.Copy(out, in)
	closeE := out.Close()
	if ce != nil {
		return ce
	}
	return closeE
}
func copyTree(src, dst string) error {
	return filepath.Walk(src, func(p string, info os.FileInfo, e error) error {
		if e != nil {
			return e
		}
		rel, e := filepath.Rel(src, p)
		if e != nil {
			return e
		}
		target := filepath.Join(dst, rel)
		if info.IsDir() {
			return os.MkdirAll(target, info.Mode().Perm())
		}
		return copyFile(p, target)
	})
}
