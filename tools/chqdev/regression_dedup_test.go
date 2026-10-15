package main

import (
	"os"
	"path/filepath"
	"strings"
	"testing"
)

// Regression contracts may be deduplicated only when their executable lines
// (including checkpoint isolation, assertions, try/finally and API calls) match.
func TestBGShadowHistoricalScenarioParity(t *testing.T) {
	root := filepath.Join("..", "..", "research", "scripts", "regression")
	body := func(name string) string {
		b, err := os.ReadFile(filepath.Join(root, name))
		if err != nil {
			t.Fatal(err)
		}
		var lines []string
		for _, line := range strings.Split(string(b), "\n") {
			s := strings.TrimSpace(line)
			if s == "" || strings.HasPrefix(s, "#") || strings.HasPrefix(s, "echo ") {
				continue
			}
			lines = append(lines, s)
		}
		return strings.Join(lines, "\n")
	}
	canonical := body("regression-v0670-rc421.chqscript")
	for _, historical := range []string{"regression-v0670-rc21.chqscript", "regression-v0670-rc32.chqscript", "regression-v0670-rc42.chqscript"} {
		if actual := body(historical); actual != canonical {
			t.Errorf("%s differs from canonical BG Shadow executable body", historical)
		}
	}
	b, err := os.ReadFile(filepath.Join(root, "full-regression.chqscript"))
	if err != nil {
		t.Fatal(err)
	}
	src := string(b)
	if strings.Count(src, "include \"regression/regression-v0670-rc421.chqscript\"") != 1 {
		t.Error("canonical BG Shadow must be included exactly once")
	}
	for _, historical := range []string{"regression-v0670-rc21.chqscript", "regression-v0670-rc32.chqscript", "regression-v0670-rc42.chqscript"} {
		if strings.Contains(src, "include \"regression/"+historical+"\"") {
			t.Errorf("historical BG Shadow still auto-included: %s", historical)
		}
	}
}
