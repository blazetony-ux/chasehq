package main

// RC6.3.9 CPU-B: validate and compile only from the actual original World
// CPU-B ROM + a fresh Ghidra headless disassembly, matched to the Windows
// CPU-B runtime census. This is NOT live CPU-B substitution or promotion.
import (
	"context"
	"crypto/sha256"
	"encoding/json"
	"errors"
	"fmt"
	"hash/crc32"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strconv"
	"strings"
	"time"
)

type rc639Site struct {
	PC               string   `json:"pc"`
	Opcode           string   `json:"opcode"`
	Text             string   `json:"text"`
	Length           int      `json:"length"`
	FlowTargets      []string `json:"flowTargets"`
	FallThrough      *string  `json:"fallThrough,omitempty"`
	FunctionReviewed bool     `json:"functionReviewed"`
}
type rc639Static struct {
	Schema                 string                `json:"schema"`
	CPU                    string                `json:"cpu"`
	Language               string                `json:"language"`
	RawROM                 string                `json:"rawRomSha256"`
	Sites                  []rc639Site           `json:"sites"`
	FrontierSites          []rc639Site           `json:"frontierSites,omitempty"`
	CallReturnSites        []rc639Site           `json:"callReturnSites,omitempty"`
	NestedReturnSites      []rc639Site           `json:"nestedReturnSites,omitempty"`
	UnobservedFallthrough  *rc6319UnobservedSite `json:"unobservedFallthrough,omitempty"`
	NativePromotionAllowed bool                  `json:"nativePromotionAllowed"`
}
type rc639Proof struct {
	Schema                     string   `json:"schema"`
	Status                     string   `json:"status"`
	CPUBROM_SHA256             string   `json:"cpuBRomSha256,omitempty"`
	CRCs                       []string `json:"romLaneCRCs,omitempty"`
	MatchedGhidraSites         int      `json:"matchedGhidraSites"`
	GeneratedBranchCandidate   string   `json:"generatedBranchCandidate,omitempty"`
	GeneratedABITests          int      `json:"generatedABITests"`
	GeneratedTwoInstructionABI int      `json:"generatedTwoInstructionABI"`
	PairLibrary                string   `json:"generatedPairLibrary,omitempty"`
	GeneratedLibrary           string   `json:"generatedLibrary,omitempty"`
	CPU_BLiveNativeCommits     int      `json:"cpuBLiveNativeCommits"`
	CPUBPromotionAllowed       bool     `json:"cpuBPromotionAllowed"`
	Limitations                []string `json:"limitations"`
}

func rc639SubROM(dir string) ([]byte, error) {
	lanes := []struct {
		name string
		crc  uint32
	}{{"b52-132.39", 0xa2f54789}, {"b52-133.55", 0x12232f95}}
	out := make([]byte, 0x20000)
	for i, l := range lanes {
		data, e := os.ReadFile(filepath.Join(dir, l.name))
		if e != nil {
			return nil, fmt.Errorf("original CPU-B lane missing: %s: %w", l.name, e)
		}
		if len(data) != 0x10000 || crc32.ChecksumIEEE(data) != l.crc {
			return nil, fmt.Errorf("original CPU-B lane size/CRC mismatch: %s", l.name)
		}
		for j, v := range data {
			out[j*2+i] = v
		}
	}
	return out, nil
}
func rc639ValidateGhidra(b []byte, digest string, hot []rc637CPU_BCandidate) (rc639Static, error) {
	var doc rc639Static
	if len(b) == 0 || len(b) > 256*1024 || json.Unmarshal(b, &doc) != nil {
		return doc, errors.New("malformed or oversized CPU-B Ghidra findings")
	}
	if doc.Schema != "chq-rc639-cpu-b-ghidra-sites-v1" || doc.CPU != "B" || doc.Language != "68000:BE:32:default" || !strings.EqualFold(doc.RawROM, digest) || doc.NativePromotionAllowed {
		return doc, errors.New("CPU-B Ghidra processor/ROM provenance or promotion guard mismatch")
	}
	if len(hot) != 5 || len(doc.Sites) != 5 {
		return doc, errors.New("CPU-B Ghidra requires exactly five runtime-proven sites")
	}
	expected := map[uint64]string{}
	for _, c := range hot {
		pc, e := strconv.ParseUint(c.PC, 0, 24)
		if e != nil {
			return doc, e
		}
		if _, duplicate := expected[pc]; duplicate {
			return doc, errors.New("duplicate runtime CPU-B PC")
		}
		expected[pc] = strings.ToUpper(c.OpcodeWord)
	}
	found := map[uint64]bool{}
	for _, s := range doc.Sites {
		pc, e := strconv.ParseUint(s.PC, 0, 24)
		if e != nil || pc&1 != 0 || pc >= 0x20000 {
			return doc, errors.New("invalid CPU-B Ghidra PC")
		}
		op, e := strconv.ParseUint(s.Opcode, 0, 16)
		if e != nil || strings.ToUpper(fmt.Sprintf("0x%04X", op)) != expected[pc] || s.Length < 2 || s.Length > 10 || s.Length&1 != 0 || s.Text == "" || s.FunctionReviewed {
			return doc, fmt.Errorf("CPU-B opcode or static metadata contradictory at 0x%06X", pc)
		}
		if found[pc] {
			return doc, errors.New("duplicate CPU-B Ghidra PC")
		}
		found[pc] = true
		if pc == 0x4de {
			if op != 0x67f8 || s.Length != 2 || !strings.HasPrefix(strings.ToLower(strings.TrimSpace(s.Text)), "beq") {
				return doc, errors.New("CPU-B branch instruction not supported by Ghidra generator")
			}
			ok := false
			for _, target := range s.FlowTargets {
				a, er := strconv.ParseUint(target, 0, 24)
				if er == nil && a == 0x4d8 {
					ok = true
				}
			}
			if !ok {
				return doc, errors.New("Ghidra static BEQ target inconsistent with original CPU-B runtime edges")
			}
		}
	}
	if len(found) != len(expected) {
		return doc, errors.New("missing original CPU-B Ghidra sites")
	}
	return doc, nil
}
func rc639GeneratedBranch(doc rc639Static) (string, error) {
	for _, site := range doc.Sites {
		p, _ := strconv.ParseUint(site.PC, 0, 24)
		if p != 0x4de {
			continue
		}
		op, e := strconv.ParseUint(site.Opcode, 0, 16)
		if e != nil || op&0xff00 != 0x6700 || op&0xff == 0 {
			return "", errors.New("Ghidra CPU-B BEQ unsupported opcode")
		}
		dest := uint32(p+2) + uint32(int32(int8(byte(op))))
		return fmt.Sprintf(`#include <cstdint>
#if defined(_WIN32)
#define CHQ_B_EXPORT extern "C" __declspec(dllexport)
#else
#define CHQ_B_EXPORT extern "C" __attribute__((visibility("default")))
#endif
struct BInput{uint32_t pc;uint16_t sr,opcode,version;};
struct BOutput{uint32_t pc;uint16_t sr,version;uint32_t cycles;};
// Generated from original CPU-B Ghidra BEQ.B at 0x%06X; branch target 0x%06X.
CHQ_B_EXPORT int chq_native_cpu_b_beq_v1(const BInput* in,BOutput* out){
 if(!in||!out||in->version!=1||in->pc!=0x%XU||in->opcode!=0x%04XU)return 0;
 const bool taken=(in->sr&4U)!=0;
 out->pc=taken?0x%XU:0x%XU;
 out->sr=in->sr;out->version=1;out->cycles=taken?10U:8U;return 1;
}
`, p, dest, p, op, dest, p+2), nil
	}
	return "", errors.New("CPU-B Ghidra BEQ candidate missing")
}
func rc639BranchTestSource() string {
	return `#include <cstdint>
#include <cstdio>
struct BInput{uint32_t pc;uint16_t sr,opcode,version;};
struct BOutput{uint32_t pc;uint16_t sr,version;uint32_t cycles;};
extern "C" int chq_native_cpu_b_beq_v1(const BInput*,BOutput*);
int main(){BInput in{0x4de,0x2004,0x67f8,1};BOutput out{};
 if(chq_native_cpu_b_beq_v1(&in,&out)!=1||out.pc!=0x4d8||out.sr!=in.sr||out.cycles!=10)return 1;
 in.sr=0x2000;
 if(chq_native_cpu_b_beq_v1(&in,&out)!=1||out.pc!=0x4e0||out.sr!=in.sr||out.cycles!=8)return 2;
 if(chq_native_cpu_b_beq_v1(nullptr,&out)!=0||chq_native_cpu_b_beq_v1(&in,nullptr)!=0)return 3;
 in.opcode=0x67f9;if(chq_native_cpu_b_beq_v1(&in,&out)!=0)return 4;
 in.opcode=0x67f8;in.pc=0x4dc;if(chq_native_cpu_b_beq_v1(&in,&out)!=0)return 5;
 puts("PASS RC639 generated CPU-B Ghidra BEQ ABI 2/2 paths + null/opcode/PC guards; no live substitution");return 0;
}`
}
func rc639FindHeadless(home string) (string, error) {
	roots := []string{}
	if home != "" {
		roots = append(roots, home)
	} else {
		roots = append(roots, os.Getenv("GHIDRA_HOME"), `C:\Tools\Ghidra`, `C:\Program Files\Ghidra`)
	}
	for _, r := range roots {
		if r == "" {
			continue
		}
		direct := filepath.Join(r, "support", "analyzeHeadless.bat")
		if f, e := os.Stat(direct); e == nil && !f.IsDir() {
			return direct, nil
		}
		sub, _ := filepath.Glob(filepath.Join(r, "ghidra_*", "support", "analyzeHeadless.bat"))
		for _, candidate := range sub {
			if f, e := os.Stat(candidate); e == nil && !f.IsDir() {
				return candidate, nil
			}
		}
	}
	return "", errors.New("Ghidra headless not located; configure GHIDRA_HOME or use --ghidra-home DIR; CPU-B substitution remains blocked")
}

// Preserve the ability to run a standalone native reconstruction from a
// Windows Developer PowerShell without modifying its PATH. CMake's full path
// was authenticated by the already successful `build debug` CMake cache.
func rc6310FindCMake(root string) (string, error) {
	if exe, err := exec.LookPath("cmake"); err == nil {
		return exe, nil
	}
	b, err := os.ReadFile(filepath.Join(root, "out", "build", "x64-Debug", "CMakeCache.txt"))
	if err == nil {
		for _, line := range strings.Split(string(b), "\n") {
			if strings.HasPrefix(line, "CMAKE_COMMAND:INTERNAL=") {
				exe := strings.TrimSpace(strings.TrimPrefix(line, "CMAKE_COMMAND:INTERNAL="))
				if stat, e := os.Stat(exe); e == nil && !stat.IsDir() {
					return exe, nil
				}
			}
		}
	}
	for _, exe := range []string{`C:\Program Files\CMake\bin\cmake.exe`,
		`C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe`,
		`C:\Program Files\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe`} {
		if stat, e := os.Stat(exe); e == nil && !stat.IsDir() {
			return exe, nil
		}
	}
	return "", errors.New("CMake not located via PATH, successful Debug cache or Visual Studio installation")
}
func rc639DiscoverCPU_B(root, dest, romDir, home string, shared rc637SharedProof) (rc639Proof, error) {
	proof := rc639Proof{Schema: "chq-rc639-real-cpu-b-ghidra-discovery-v1", Status: "BLOCKED", CPUBPromotionAllowed: false,
		Limitations: []string{"CPU-B Ghidra export and generated BEQ ABI are OFFLINE code provenance only", "No CPU-B original/native/forced-ROM-rollback emulator differential yet", "Function boundaries/interrupt timing and whole-machine equivalence unreviewed; no autonomous promotion"}}
	rom, e := rc639SubROM(romDir)
	if e != nil {
		return proof, e
	}
	sha := fmt.Sprintf("%x", sha256.Sum256(rom))
	proof.CPUBROM_SHA256 = sha
	proof.CRCs = []string{"b52-132.39:a2f54789", "b52-133.55:12232f95"}
	if runtime.GOOS != "windows" {
		return proof, errors.New("real CPU-B Ghidra headless Windows analysis unavailable on this host; no fabricated findings accepted")
	}
	if len(shared.CPU_BCandidates) != 5 {
		return proof, errors.New("CPU-B real census/edges must be independently verified before static analysis")
	}
	headless, e := rc639FindHeadless(home)
	if e != nil {
		return proof, e
	}
	tmp, e := os.MkdirTemp("", "chq-rc639-cpub-original-*")
	if e != nil {
		return proof, e
	}
	defer os.RemoveAll(tmp)
	raw := filepath.Join(tmp, "cpu_b_world.bin")
	if e = os.WriteFile(raw, rom, 0600); e != nil {
		return proof, e
	}
	output := filepath.Join(dest, "cpu-b-ghidra")
	if e = os.MkdirAll(output, 0755); e != nil {
		return proof, e
	}
	scriptdir := filepath.Join(root, "research", "ghidra-automation")
	if _, e = os.Stat(filepath.Join(scriptdir, "ChaseHQCPU_B_Export639.py")); e != nil {
		return proof, e
	}
	args := []string{"/d", "/s", "/c", headless, tmp, "ChaseHQ_CPUB_RC639", "-import", raw, "-processor", "68000:BE:32:default", "-cspec", "default", "-loader", "BinaryLoader", "-loader-baseAddr", "0x000000", "-postScript", "ChaseHQCPU_B_Export639.py", output, sha}
	for _, site := range shared.CPU_BCandidates {
		args = append(args, site.PC+":"+site.OpcodeWord)
	}
	args = append(args, "-scriptPath", scriptdir, "-analysisTimeoutPerFile", "600", "-max-cpu", "2")
	ctx, cancel := context.WithTimeout(context.Background(), 12*time.Minute)
	defer cancel()
	cmd := exec.CommandContext(ctx, "cmd.exe", args...)
	// Local-only JDK discovery mirrors other chqdev Ghidra workflows.
	java := locateJDK25()
	if java.JavaReady {
		cmd.Env = append(os.Environ(), "JAVA_HOME="+java.JavaHome, "PATH="+filepath.Join(java.JavaHome, "bin")+string(os.PathListSeparator)+os.Getenv("PATH"))
	}
	log, runErr := cmd.CombinedOutput()
	_ = os.WriteFile(filepath.Join(output, "headless.log"), log, 0644)
	if runErr != nil {
		return proof, fmt.Errorf("real CPU-B Ghidra headless failed: %v; inspect cpu-b-ghidra/headless.log", runErr)
	}
	if !strings.Contains(string(log), "CHQ_RC639_CPU_B_GHIDRA_OK") {
		return proof, errors.New("CPU-B Ghidra success marker missing")
	}
	b, e := os.ReadFile(filepath.Join(output, "cpu-b-ghidra-sites.json"))
	if e != nil {
		return proof, e
	}
	static, e := rc639ValidateGhidra(b, sha, shared.CPU_BCandidates)
	if e != nil {
		return proof, e
	}
	proof.MatchedGhidraSites = len(static.Sites)
	generated, e := rc639GeneratedBranch(static)
	if e != nil {
		return proof, e
	}
	genDir := filepath.Join(output, "generated")
	if e = os.MkdirAll(genDir, 0755); e != nil {
		return proof, e
	}
	for name, contents := range map[string]string{"generated-cpu-b.cpp": generated, "generated-cpu-b-test.cpp": rc639BranchTestSource(), "CMakeLists.txt": `cmake_minimum_required(VERSION 3.20)
project(chq_rc639_cpu_b LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
add_library(chq_rc639_cpu_b SHARED generated-cpu-b.cpp)
add_executable(chq_rc639_cpu_b_abi generated-cpu-b-test.cpp)
target_link_libraries(chq_rc639_cpu_b_abi PRIVATE chq_rc639_cpu_b)
`} {
		if e = os.WriteFile(filepath.Join(genDir, name), []byte(contents), 0644); e != nil {
			return proof, e
		}
	}
	build := filepath.Join(output, "build")
	run := func(name string, args ...string) error {
		context, cancel := context.WithTimeout(context.Background(), 3*time.Minute)
		defer cancel()
		command := exec.CommandContext(context, name, args...)
		command.Dir = genDir
		log, e := command.CombinedOutput()
		_ = os.WriteFile(filepath.Join(output, filepath.Base(name)+"-"+strconv.Itoa(len(args))+".log"), log, 0644)
		return e
	}
	cmakeExe, cmakeErr := rc6310FindCMake(root)
	if cmakeErr != nil {
		return proof, cmakeErr
	}
	if e = run(cmakeExe, "-S", genDir, "-B", build); e != nil {
		return proof, fmt.Errorf("CPU-B generated C++ configure failed: %w", e)
	}
	if e = run(cmakeExe, "--build", build, "--config", "Release"); e != nil {
		return proof, fmt.Errorf("CPU-B generated C++ build failed: %w", e)
	}
	abi := filepath.Join(build, "Release", "chq_rc639_cpu_b_abi.exe")
	ctx2, cancel2 := context.WithTimeout(context.Background(), 30*time.Second)
	defer cancel2()
	abiLog, abiErr := exec.CommandContext(ctx2, abi).CombinedOutput()
	_ = os.WriteFile(filepath.Join(output, "cpu-b-generated-abi.log"), abiLog, 0644)
	if abiErr != nil || !strings.Contains(string(abiLog), "PASS RC639 generated CPU-B Ghidra BEQ ABI 2/2") {
		return proof, fmt.Errorf("generated CPU-B ABI rejection: %v", abiErr)
	}
	// RC6.3.12: a second independent Ghidra-derived C++ library now covers
	// TST.W absolute-long (one mapped R16 + NZVC) followed by BEQ.B.
	// This compilation is NOT a live CPU-B range replacement.
	pairCode, pairProof, pairErr := rc6312GeneratePair(static, rom, shared)
	_ = rc63WriteJSON(filepath.Join(dest, "cpu-b-two-instruction-proof.json"), pairProof)
	if pairErr != nil {
		return proof, fmt.Errorf("CPU-B two-instruction Ghidra contract: %w", pairErr)
	}
	// Separate target and ABI executable, without altering the previously
	// Windows-proven single-opcode generated DLL interface.
	pairCMake := `cmake_minimum_required(VERSION 3.20)
project(chq_rc6312_cpu_b_pair LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
add_library(chq_rc6312_cpu_b_pair SHARED generated-cpu-b-pair.cpp)
add_executable(chq_rc6312_cpu_b_pair_abi generated-cpu-b-pair-test.cpp)
target_link_libraries(chq_rc6312_cpu_b_pair_abi PRIVATE chq_rc6312_cpu_b_pair)
`
	pairDir := filepath.Join(output, "generated-pair")
	if e = os.MkdirAll(pairDir, 0755); e != nil {
		return proof, e
	}
	for name, contents := range map[string]string{"generated-cpu-b-pair.cpp": pairCode, "generated-cpu-b-pair-test.cpp": rc6312PairTestSource(pairProof.SourceAddress), "CMakeLists.txt": pairCMake} {
		if e = os.WriteFile(filepath.Join(pairDir, name), []byte(contents), 0644); e != nil {
			return proof, e
		}
	}
	// MSBuild's TryCompile .tlog creation fails under the long project/evidence
	// nested path (genuine RC6.3.12 Windows evidence). Compile in a short,
	// per-run temp directory and copy the qualified DLL/EXE into evidence.
	pairBuild := filepath.Join(output, "pair-build")
	if runtime.GOOS == "windows" {
		short, err := os.MkdirTemp(os.TempDir(), "chqpair-")
		if err != nil {
			return proof, fmt.Errorf("short CMake pair scratch: %w", err)
		}
		defer os.RemoveAll(short)
		pairBuild = filepath.Join(short, "b")
	}
	if e = run(cmakeExe, "-S", pairDir, "-B", pairBuild); e != nil {
		return proof, fmt.Errorf("CPU-B TST+BEQ configure failed: %w", e)
	}
	if e = run(cmakeExe, "--build", pairBuild, "--config", "Release"); e != nil {
		return proof, fmt.Errorf("CPU-B TST+BEQ build failed: %w", e)
	}
	pairABI := filepath.Join(pairBuild, "Release", "chq_rc6312_cpu_b_pair_abi.exe")
	pairLog, pairRunErr := exec.Command(pairABI).CombinedOutput()
	_ = os.WriteFile(filepath.Join(output, "cpu-b-pair-generated-abi.log"), pairLog, 0644)
	if pairRunErr != nil || !strings.Contains(string(pairLog), "PASS RC6312 Ghidra CPU-B TST.W+BEQ pair ABI 4/4") {
		return proof, fmt.Errorf("CPU-B Ghidra two-instruction ABI rejected: %v", pairRunErr)
	}
	if runtime.GOOS == "windows" {
		for _, name := range []string{"chq_rc6312_cpu_b_pair.dll", "chq_rc6312_cpu_b_pair_abi.exe"} {
			b, err := os.ReadFile(filepath.Join(pairBuild, "Release", name))
			if err != nil {
				return proof, fmt.Errorf("generated CPU-B pair output absent: %s: %w", name, err)
			}
			dst := filepath.Join(output, "pair-build", "Release", name)
			if err = os.MkdirAll(filepath.Dir(dst), 0755); err != nil {
				return proof, err
			}
			if err = os.WriteFile(dst, b, 0644); err != nil {
				return proof, err
			}
		}
	}
	pairProof.Status = "CPU_B_TWO_INSTRUCTION_GHIDRA_ABI_PASS_NOT_LIVE"
	_ = rc63WriteJSON(filepath.Join(dest, "cpu-b-two-instruction-proof.json"), pairProof)
	proof.GeneratedTwoInstructionABI = 4
	proof.PairLibrary = "cpu-b-ghidra/pair-build/Release/chq_rc6312_cpu_b_pair.dll"
	proof.Status = "CPU_B_GHIDRA_GENERATED_ABI_PASS_NOT_LIVE"
	proof.GeneratedBranchCandidate = "B:000004DE-000004E0"
	proof.GeneratedABITests = 2
	proof.GeneratedLibrary = "cpu-b-ghidra/build/Release/chq_rc639_cpu_b.dll"
	return proof, nil
}
