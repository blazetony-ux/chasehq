package main

import (
	"os"
	"path/filepath"
	"strings"
	"testing"
)

// Windows SDK FLOAT128 and Musashi softfloat FLOAT128 are different symbols;
// old include ordering compiled on Linux but failed in MSVC. The same source
// also needs NOMINMAX because it uses std::min/std::max throughout.
func TestRC624RuntimeWindowsSDKBeforeMusashi(t *testing.T) {
	path := filepath.Join("..", "..", "src", "runtime.cpp")
	b, err := os.ReadFile(path)
	if err != nil {
		t.Fatal(err)
	}
	s := string(b)
	windows := strings.Index(s, "#include <windows.h>")
	musashi := strings.Index(s, "#include \"m68kcpu.h\"")
	if windows < 0 || musashi < 0 || windows >= musashi {
		t.Fatal("Windows SDK must precede Musashi headers to avoid FLOAT128 collision")
	}
	define := strings.Index(s, "#define NOMINMAX")
	if define < 0 || define >= windows {
		t.Fatal("NOMINMAX must be defined before including windows.h")
	}
	undef := strings.Index(s, "#undef EXCEPTION_ILLEGAL_INSTRUCTION")
	if undef < windows || undef >= musashi {
		t.Fatal("Windows exception macro must be released before Musashi definitions")
	}
}
