package main

import (
	"archive/zip"
	"bytes"
	"encoding/csv"
	"encoding/json"
	"errors"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strconv"
	"strings"
	"testing"
)

func testProjectRoot(t *testing.T) string {
	t.Helper()
	r, e := filepath.Abs(filepath.Join("..", ".."))
	if e != nil {
		t.Fatal(e)
	}
	return r
}
func TestRC622RealGhidraProvenanceAnd193Reference(t *testing.T) {
	root := testProjectRoot(t)
	p := filepath.Join(root, "research", "acceptance", "ghidra-steering-RC621-verified.zip")
	g, raw, e := readNativeGhidra(p)
	if e != nil {
		t.Fatal(e)
	}
	if g.CPU != "A" || g.RawRomSHA != "5cf1c9e786a39795ebd8e99c8e8be0d2398cc4f74221067cf2844945308d6c5a" || len(raw) < 1000 {
		t.Fatal("unexpected qualified source provenance")
	}
	// The independent 193-entry oracle's structure can be validated without any proprietary ROM bytes.
	rows, e := os.ReadFile(filepath.Join(root, "docs", "reference", "steering_reference_193.csv"))
	if e != nil {
		t.Fatal(e)
	}
	if !bytes.Contains(rows, []byte("-96,0x3126C")) || !bytes.Contains(rows, []byte("96,0x313EC")) {
		t.Fatal("193-entry reference bounds missing")
	}
}
func TestRC622RejectsTamperedGhidraManifest(t *testing.T) {
	root := testProjectRoot(t)
	original, e := zip.OpenReader(filepath.Join(root, "research", "acceptance", "ghidra-steering-RC621-verified.zip"))
	if e != nil {
		t.Fatal(e)
	}
	defer original.Close()
	tmp := filepath.Join(t.TempDir(), "bad.zip")
	f, e := os.Create(tmp)
	if e != nil {
		t.Fatal(e)
	}
	w := zip.NewWriter(f)
	for _, file := range original.File {
		rd, e := file.Open()
		if e != nil {
			t.Fatal(e)
		}
		var b bytes.Buffer
		_, e = b.ReadFrom(rd)
		_ = rd.Close()
		if e != nil {
			t.Fatal(e)
		}
		data := b.Bytes()
		if strings.HasSuffix(strings.ReplaceAll(file.Name, "\\", "/"), "/run-manifest.json") {
			data = bytes.Replace(data, []byte("0.67.0.0-RC6.2.1"), []byte("0.67.0.0-RC4.7"), 1)
		}
		dst, e := w.Create(file.Name)
		if e != nil {
			t.Fatal(e)
		}
		if _, e = dst.Write(data); e != nil {
			t.Fatal(e)
		}
	}
	if e = w.Close(); e != nil {
		t.Fatal(e)
	}
	if e = f.Close(); e != nil {
		t.Fatal(e)
	}
	if _, _, e = readNativeGhidra(tmp); e == nil {
		t.Fatal("tampered Ghidra manifest was accepted")
	}
}
func TestRC622StandaloneNegativePromotionGuards(t *testing.T) {
	c := map[string]any{"candidateInstructionReviewed": true, "generatedCandidateLoadedAndExecutedVerified": true, "registerEffectsVerified": true, "cpuFlagsVerified": true, "orderedBusEffectsVerified": true, "cycleEquivalenceVerified": true, "controlFlowVerified": true, "rollbackVerified": true, "independentLiveStateParityVerified": true}
	rom := []byte{7, 8, 9, 10}
	sha := hexDigest(rom)
	sh := nativeShadow{Schema: "chq-native-steering-shadow-v1", Enabled: true, ReadOnly: true, Entries: 3, Compared: 3, Matches: 3}
	if !nativePromotionEligibility(c, sh, sha, rom) {
		t.Fatal("valid positive eligibility fixture rejected")
	}
	sh.Mismatches = 1
	sh.Matches = 2
	if nativePromotionEligibility(c, sh, sha, rom) {
		t.Fatal("induced shadow mismatch incorrectly accepted")
	}
	sh.Mismatches = 0
	sh.Matches = 3
	c["cpuFlagsVerified"] = false
	if nativePromotionEligibility(c, sh, sha, rom) {
		t.Fatal("missing CPU flags cannot authorize substitution")
	}
	c["cpuFlagsVerified"] = true
	if nativePromotionEligibility(c, sh, sha, []byte{6, 8, 9, 10}) {
		t.Fatal("wrong ROM erroneously authorized")
	}
}
func TestRC622PackageIncludesRealGhidraEvidence(t *testing.T) {
	root := testProjectRoot(t)
	cfg, e := loadConfig(root)
	if e != nil {
		t.Fatal(e)
	}
	files, e := collectPackageFiles(root, cfg)
	if e != nil {
		t.Fatal(e)
	}
	target := "research/acceptance/ghidra-steering-RC621-verified.zip"
	for _, p := range files {
		if p == target {
			return
		}
	}
	t.Fatalf("release omitted %s; acceptance would fail on fresh extraction", target)
}
func TestRC622FailedAcceptanceAlwaysZipsEvidence(t *testing.T) {
	base := t.TempDir()
	root := testProjectRoot(t)
	artifactDir := filepath.Join(base, "research", "acceptance")
	if e := os.MkdirAll(artifactDir, 0755); e != nil {
		t.Fatal(e)
	}
	realZIP, e := os.ReadFile(filepath.Join(root, "research", "acceptance", "ghidra-steering-RC621-verified.zip"))
	if e != nil {
		t.Fatal(e)
	}
	if e = os.WriteFile(filepath.Join(artifactDir, "ghidra-steering-RC621-verified.zip"), realZIP, 0644); e != nil {
		t.Fatal(e)
	}
	dest := filepath.Join(base, "out-evidence")
	r := nativeAcceptCommand(base, Config{Release: "0.67.0.0-RC6.2.6"}, []string{"accept", "--fresh", "--out", dest})
	if r.Status != "FAIL" {
		t.Fatalf("partial acceptance must never PASS: %+v", r)
	}
	b, e := os.ReadFile(filepath.Join(dest, "acceptance.json"))
	if e != nil {
		t.Fatal(e)
	}
	var got struct {
		Status    string `json:"status"`
		Promotion struct {
			Authorized bool `json:"authorized"`
		} `json:"promotion"`
	}
	if e = json.Unmarshal(b, &got); e != nil {
		t.Fatal(e)
	}
	if got.Status != "NOT_SIGNED_OFF" || got.Promotion.Authorized {
		t.Fatal("unqualified acceptance marked signed off")
	}
	z, e := zip.OpenReader(dest + ".zip")
	if e != nil {
		t.Fatal(e)
	}
	defer z.Close()
	hasHash, hasReport := false, false
	for _, f := range z.File {
		if f.Name == "evidence-sha256.json" {
			hasHash = true
		}
		if f.Name == "acceptance.json" {
			hasReport = true
		}
	}
	if !hasHash || !hasReport {
		t.Fatal("failed run evidence ZIP omitted report or SHA256 manifest")
	}
}

func TestRC622GeneratedSteeringNativeCrossCheck(t *testing.T) {
	compiler, err := exec.LookPath("c++")
	if err != nil {
		t.Skip("native compiler not on PATH (Windows may use Visual Studio CMake generator)")
	}
	root := testProjectRoot(t)
	f, err := os.Open(filepath.Join(root, "docs", "reference", "steering_reference_193.csv"))
	if err != nil {
		t.Fatal(err)
	}
	defer f.Close()
	rows, err := csv.NewReader(f).ReadAll()
	if err != nil {
		t.Fatal(err)
	}
	table := make([]int16, 193)
	for i, row := range rows[1:] {
		n, err := strconv.ParseInt(row[3], 10, 16)
		if err != nil {
			t.Fatal(err)
		}
		table[i] = int16(n)
	}
	vectors, err := nativeSteeringVectors(filepath.Join(root, "docs", "reference", "steering_reference_193.csv"), table)
	if err != nil {
		t.Fatal(err)
	}
	source := generateNativeSteering(table, vectors)
	dir := t.TempDir()
	cpp := filepath.Join(dir, "steering.cpp")
	exe := filepath.Join(dir, "steering")
	if err = os.WriteFile(cpp, []byte(source), 0644); err != nil {
		t.Fatal(err)
	}
	cmd := exec.Command(compiler, "-std=c++17", "-o", exe, cpp)
	if out, err := cmd.CombinedOutput(); err != nil {
		t.Fatalf("C++ compile: %s: %v", out, err)
	}
	if out, err := exec.Command(exe).CombinedOutput(); err != nil || !bytes.Contains(out, []byte("193/193")) {
		t.Fatalf("compiled steering oracle cross-check: %s %v", out, err)
	}
	bad := strings.Replace(source, "table={{-96,", "table={{-95,", 1)
	if bad == source {
		t.Fatal("negative candidate mutation absent")
	}
	badCpp := filepath.Join(dir, "negative.cpp")
	badExe := filepath.Join(dir, "negative")
	if err = os.WriteFile(badCpp, []byte(bad), 0644); err != nil {
		t.Fatal(err)
	}
	if out, err := exec.Command(compiler, "-std=c++17", "-o", badExe, badCpp).CombinedOutput(); err != nil {
		t.Fatalf("mutant C++ compile: %s %v", out, err)
	}
	cmd = exec.Command(badExe)
	_, err = cmd.CombinedOutput()
	var exit *exec.ExitError
	if !errors.As(err, &exit) || exit.ExitCode() != 2 {
		t.Fatalf("mutant candidate failed to reject: %v", err)
	}
}

// The generated C++ must be loadable as the shared library invoked by the
// live Musashi instruction replacement. Standalone vector PASS is insufficient.
func TestRC623GeneratedSharedLibraryIsCallable(t *testing.T) {
	if runtime.GOOS != "linux" {
		t.Skip("native shared-library fixture is Linux-specific; real Windows DLL proved by acceptance")
	}
	if _, e := exec.LookPath("c++"); e != nil {
		t.Skip("c++ not installed")
	}
	root := testProjectRoot(t)
	csvFile, e := os.Open(filepath.Join(root, "docs", "reference", "steering_reference_193.csv"))
	if e != nil {
		t.Fatal(e)
	}
	defer csvFile.Close()
	rows, e := csv.NewReader(csvFile).ReadAll()
	if e != nil {
		t.Fatal(e)
	}
	table := make([]int16, 193)
	for i, row := range rows[1:] {
		v, e := strconv.ParseInt(row[3], 10, 16)
		if e != nil {
			t.Fatal(e)
		}
		table[i] = int16(v)
	}
	src := generateNativeSteering(table, table)
	dir := t.TempDir()
	source := filepath.Join(dir, "steering.cpp")
	lib := filepath.Join(dir, "libgenerated-steering.so")
	if e := os.WriteFile(source, []byte(src), 0644); e != nil {
		t.Fatal(e)
	}
	if output, e := exec.Command("c++", "-std=c++17", "-shared", "-fPIC", "-o", lib, source).CombinedOutput(); e != nil {
		t.Fatalf("generated shared library compilation: %s %v", output, e)
	}
	probe := `#include <dlfcn.h>
#include <cstdint>
int main(int argc,char **argv){ if(argc!=2)return 2;void *h=dlopen(argv[1],RTLD_NOW|RTLD_LOCAL);if(!h)return 3;auto f=reinterpret_cast<int16_t(*)(int16_t)>(dlsym(h,"chq_native_steering_193"));if(!f)return 4;return f(0)==` + strconv.Itoa(int(table[96])) + `&& f(-96)==` + strconv.Itoa(int(table[0])) + `&&f(96)==` + strconv.Itoa(int(table[192])) + `?0:5;}`
	probePath := filepath.Join(dir, "load.cpp")
	probeExe := filepath.Join(dir, "load")
	if e := os.WriteFile(probePath, []byte(probe), 0644); e != nil {
		t.Fatal(e)
	}
	if output, e := exec.Command("c++", "-std=c++17", "-o", probeExe, probePath, "-ldl").CombinedOutput(); e != nil {
		t.Fatalf("probe compilation: %s %v", output, e)
	}
	if output, e := exec.Command(probeExe, lib).CombinedOutput(); e != nil {
		t.Fatalf("live exported generated library invocation: %s %v", output, e)
	}
}
