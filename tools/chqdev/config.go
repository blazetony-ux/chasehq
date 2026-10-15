package main

import (
	"encoding/json"
	"fmt"
	"os"
	"path/filepath"
	"regexp"
	"strings"
)

type Config struct {
	Schema                 string           `json:"schema"`
	Release                string           `json:"release"`
	Baseline               string           `json:"baseline"`
	BaselineArchiveSHA256  string           `json:"baselineArchiveSha256"`
	VersionHeader          string           `json:"versionHeader"`
	Handover               string           `json:"handover"`
	ScriptRoot             string           `json:"scriptRoot"`
	ScriptCatalog          string           `json:"scriptCatalog"`
	CurrentDocs            []string         `json:"currentDocs"`
	ReleaseGuardDocs       []string         `json:"releaseGuardDocs"`
	BuildProcessDocs       []string         `json:"buildProcessDocs"`
	BuildIdentityFiles     []string         `json:"buildIdentityFiles"`
	ReleaseProcessDocs     []string         `json:"releaseProcessDocs"`
	PackageExcludeDirs     []string         `json:"packageExcludeDirs"`
	PackageStatus          string           `json:"packageStatus"`
	PackageSourceBaseline  string           `json:"packageSourceBaseline"`
	PackageFeatures        []string         `json:"packageFeatures"`
	WindowsASCIIExtensions []string         `json:"windowsAsciiExtensions"`
	ROMFiles               map[string]int64 `json:"romFiles"`
	WorkbenchURL           string           `json:"workbenchUrl"`
	FocusedRegression      string           `json:"focusedRegression"`
	FullRegression         string           `json:"fullRegression"`
	QualificationState     string           `json:"qualificationState"`
}

type LocalConfig struct {
	ROMSource         string `json:"romSource"`
	SDLSource         string `json:"sdlSource"`
	ContentPackSource string `json:"contentPackSource,omitempty"`
	CopyROMs          bool   `json:"copyRoms"`
	CopySDL           bool   `json:"copySdl"`
}

func loadConfig(root string) (Config, error) {
	var c Config
	b, e := os.ReadFile(filepath.Join(root, "chqdev.json"))
	if e != nil {
		return c, e
	}
	if e = json.Unmarshal(b, &c); e != nil {
		return c, fmt.Errorf("parse chqdev.json: %w", e)
	}
	if c.Schema != "chqdev-config-v1" {
		return c, fmt.Errorf("unsupported chqdev config schema %q", c.Schema)
	}
	if c.Release == "" || c.VersionHeader == "" {
		return c, fmt.Errorf("chqdev.json missing release/versionHeader")
	}
	return c, nil
}

// localConfigPath resolves machine-local dependency settings without putting user paths
// in distributable source archives. Explicit per-release settings win, then a stable
// shared parent config, then existing sibling release configs (newest name first).
// A sibling is accepted only when its configured ROM and SDL sources exist.
func localConfigPath(root string) (string, error) {
	parent := filepath.Dir(root)
	candidates := []string{filepath.Join(root, "build-local.json"), filepath.Join(parent, "build-local.json")}
	siblings, err := os.ReadDir(parent)
	if err == nil {
		for i := len(siblings) - 1; i >= 0; i-- {
			entry := siblings[i]
			if !entry.IsDir() || entry.Name() == filepath.Base(root) || !strings.HasPrefix(entry.Name(), "ChaseHQ-Native-") {
				continue
			}
			candidates = append(candidates, filepath.Join(parent, entry.Name(), "build-local.json"))
		}
	}
	for index, path := range candidates {
		data, err := os.ReadFile(path)
		if os.IsNotExist(err) {
			continue
		}
		if err != nil {
			return "", fmt.Errorf("read %s: %w", path, err)
		}
		var cfg LocalConfig
		if err := json.Unmarshal(data, &cfg); err != nil {
			return "", fmt.Errorf("parse %s: %w", path, err)
		}
		if index <= 1 {
			return path, nil
		} // local/shared config must fail visibly if stale
		if cfg.ROMSource != "" && cfg.SDLSource != "" && dirExists(cfg.ROMSource) && dirExists(cfg.SDLSource) {
			return path, nil
		}
	}
	return "", nil
}

func loadLocalConfig(root string) (LocalConfig, error) {
	var c LocalConfig
	p, err := localConfigPath(root)
	if err != nil {
		return c, err
	}
	if p != "" {
		b, err := os.ReadFile(p)
		if err != nil {
			return c, err
		}
		if err := json.Unmarshal(b, &c); err != nil {
			return c, fmt.Errorf("parse %s: %w", p, err)
		}
	}
	// A shared dependency directory can be discovered without storing absolute,
	// machine-specific paths in the release archive. Explicit config and env
	// settings always take precedence.
	if c.ROMSource == "" || c.SDLSource == "" {
		base := filepath.Dir(filepath.Dir(root))
		for _, shared := range []string{filepath.Join(base, "AdditionalFiles"), filepath.Join(filepath.Dir(root), "AdditionalFiles")} {
			if c.ROMSource == "" && dirExists(filepath.Join(shared, "roms")) {
				c.ROMSource = filepath.Join(shared, "roms")
			}
			if c.SDLSource == "" && fileExists(filepath.Join(shared, "SDL", "CMakeLists.txt")) {
				c.SDLSource = filepath.Join(shared, "SDL")
			}
		}
	}
	return c, nil
}

func readNativeVersion(root string, c Config) (string, error) {
	b, e := os.ReadFile(filepath.Join(root, filepath.FromSlash(c.VersionHeader)))
	if e != nil {
		return "", e
	}
	re := regexp.MustCompile(`kNativeVersion\s*=\s*"([^"]+)"`)
	m := re.FindSubmatch(b)
	if len(m) != 2 {
		return "", fmt.Errorf("kNativeVersion not found in %s", c.VersionHeader)
	}
	return string(m[1]), nil
}
