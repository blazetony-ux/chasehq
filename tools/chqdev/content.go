package main

import (
	"encoding/json"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"time"
)

const contentSettingsFile = ".chq/content-source.json"

type contentSettings struct {
	Mode string `json:"mode"`
	Pack string `json:"pack"`
	Roms string `json:"roms"`
}
type contentSource struct {
	Mode string
	Pack string
	Roms string
}

func contentSettingsRead(root string) (contentSettings, error) {
	c := contentSettings{Mode: "rom", Pack: "content/chasehq-world.chqpack", Roms: "roms/chasehq"}
	// A stable shared content path is discovered from the same already-proven
	// build-local.json contract as ROM/SDL. Release-local content settings
	// retain precedence when explicitly supplied.
	lc, _ := loadLocalConfig(root)
	if lc.ContentPackSource != "" {
		c.Pack = lc.ContentPackSource
	} else if lc.ROMSource != "" {
		candidate := filepath.Join(filepath.Dir(filepath.Clean(lc.ROMSource)), "content", "chasehq-world.chqpack")
		if fileExists(candidate) {
			c.Pack = candidate
		}
	}
	b, e := os.ReadFile(filepath.Join(root, contentSettingsFile))
	if os.IsNotExist(e) {
		return c, nil
	}
	if e != nil {
		return c, e
	}
	if e = json.Unmarshal(b, &c); e != nil {
		return c, e
	}
	return c, nil
}
func contentPath(root, p string) string {
	if filepath.IsAbs(p) {
		return p
	}
	return filepath.Join(root, filepath.FromSlash(p))
}
func resolveContent(root, override string) (contentSource, error) {
	c, e := contentSettingsRead(root)
	if e != nil {
		return contentSource{}, e
	}
	mode := c.Mode
	if override != "" {
		mode = strings.ToLower(override)
	}
	if mode != "rom" && mode != "pack" && mode != "auto" {
		return contentSource{}, fmt.Errorf("invalid content mode %q", mode)
	}
	pack := contentPath(root, c.Pack)
	roms := contentPath(root, c.Roms)
	if mode == "auto" {
		if fileExists(pack) {
			mode = "pack"
		} else {
			mode = "rom"
		}
	}
	if mode == "pack" {
		if !fileExists(pack) {
			return contentSource{}, fmt.Errorf("pack mode requires %s (run chqdev content import)", pack)
		}
		return contentSource{Mode: mode, Pack: pack}, nil
	}
	if !dirExists(roms) {
		return contentSource{}, fmt.Errorf("rom mode requires %s", roms)
	}
	return contentSource{Mode: mode, Roms: roms}, nil
}
func contentCommand(root string, cfg Config, args []string) *Report {
	r := newReport("content", cfg.Release)
	if len(args) == 0 {
		r.fail("usage", "content mode rom|pack|auto; status; verify; import")
		return r
	}
	c, e := contentSettingsRead(root)
	if e != nil {
		r.fail("settings", e.Error())
		return r
	}
	switch args[0] {
	case "mode":
		if len(args) != 2 || (args[1] != "rom" && args[1] != "pack" && args[1] != "auto") {
			r.fail("mode", "expected rom|pack|auto")
			return r
		}
		c.Mode = args[1]
		p := filepath.Join(root, contentSettingsFile)
		if e = os.MkdirAll(filepath.Dir(p), 0755); e == nil {
			var b []byte
			b, e = json.MarshalIndent(c, "", "  ")
			if e == nil {
				e = os.WriteFile(p, append(b, '\n'), 0644)
			}
		}
		if e != nil {
			r.fail("save", e.Error())
		} else {
			r.pass("mode", "saved "+c.Mode)
		}
	case "status":
		r.Data["configuredMode"] = c.Mode
		r.Data["packPath"] = contentPath(root, c.Pack)
		r.Data["romPath"] = contentPath(root, c.Roms)
		r.Data["packPresent"] = fileExists(contentPath(root, c.Pack))
		r.Data["romPresent"] = dirExists(contentPath(root, c.Roms))
		source, e := resolveContent(root, "")
		if e != nil {
			r.warn("active-source", e.Error())
		} else {
			r.pass("active-source", source.Mode)
			r.Data["activeMode"] = source.Mode
		}
	case "verify":
		if !fileExists(contentPath(root, c.Pack)) {
			r.fail("pack", "missing "+contentPath(root, c.Pack))
		} else {
			r.pass("pack", "pack exists; runtime integrity validation occurs on load")
		}
		if !dirExists(contentPath(root, c.Roms)) {
			r.warn("roms", "ROM directory absent")
		} else {
			r.pass("roms", "ROM directory exists")
		}
	case "import":
		if !dirExists(contentPath(root, c.Roms)) {
			r.fail("roms", "ROM directory absent")
			return r
		}
		exe := filepath.Join(root, "out", "build", "x64-Debug", "Debug", "chqimport.exe")
		if !fileExists(exe) {
			r.fail("importer", "missing "+exe)
			return r
		}
		pack := contentPath(root, c.Pack)
		if e = os.MkdirAll(filepath.Dir(pack), 0755); e != nil {
			r.fail("directory", e.Error())
			return r
		}
		cmd := exec.Command(exe, contentPath(root, c.Roms), pack)
		cmd.Dir = root
		out, e := cmd.CombinedOutput()
		r.Data["output"] = string(out)
		if e != nil {
			r.fail("import", fmt.Sprintf("%v: %s", e, out))
		} else {
			r.pass("import", pack)
		}
	default:
		r.fail("usage", "unknown content subcommand")
	}
	r.Data["checkedAt"] = time.Now().Format(time.RFC3339)
	return r
}
