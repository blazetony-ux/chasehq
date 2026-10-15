package main

import (
	"archive/zip"
	"encoding/binary"
	"encoding/json"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"
	"testing"
)

func rc6312Fixture() (rc639Static, []byte, rc637SharedProof) {
	doc, _ := rc639SyntheticSites()
	doc.Sites[0].Text = "tst.w (0x00100804).l"
	rom := make([]byte, 0x20000)
	binary.BigEndian.PutUint16(rom[0x4d8:0x4da], 0x4a79)
	binary.BigEndian.PutUint32(rom[0x4da:0x4de], 0x00100804)
	binary.BigEndian.PutUint16(rom[0x4de:0x4e0], 0x67f8)
	shared := rc637SharedProof{Status: "LIVE_SHARED_DISPATCH_AND_OBSERVED_CPU_B_EDGES_PASS", CPU_BCandidates: []rc637CPU_BCandidate{{PC: "0x0004D8", OpcodeWord: "0x4A79", Destinations: []rc637Edge{{To: "0x0004DE", Opcode: "0x4A79", Hits: 17903}}}}}
	return doc, rom, shared
}
func TestRC6312TwoInstructionPairAuthenticityAndMutations(t *testing.T) {
	doc, rom, edges := rc6312Fixture()
	generated, p, e := rc6312GeneratePair(doc, rom, edges)
	if e != nil || p.OriginalConsecutiveEdges != 17903 || p.SourceAddress != "0x00100804" || p.LivePairSubstitutionProven || p.PromotionAllowed || !strings.Contains(generated, "readWidth=2") {
		t.Fatalf("genuine pair rejected: %v %+v", e, p)
	}
	tests := map[string]func(*rc639Static, []byte, *rc637SharedProof){
		"fake-tst-opcode": func(d *rc639Static, r []byte, s *rc637SharedProof) { r[0x4d8] = 0 },
		"fake-beq-opcode": func(d *rc639Static, r []byte, s *rc637SharedProof) { r[0x4de] = 0 },
		"forged-operand":  func(d *rc639Static, r []byte, s *rc637SharedProof) { d.Sites[0].Text = "tst.w (0x00100806).l" },
		"bad-operand-ram": func(d *rc639Static, r []byte, s *rc637SharedProof) {
			d.Sites[0].Text = "tst.w (0x001F0000).l"
			binary.BigEndian.PutUint32(r[0x4da:0x4de], 0x001f0000)
		},
		"unsupported-size":         func(d *rc639Static, r []byte, s *rc637SharedProof) { d.Sites[0].Length = 4 },
		"false-function-review":    func(d *rc639Static, r []byte, s *rc637SharedProof) { d.Sites[0].FunctionReviewed = true },
		"no-consecutive-edge":      func(d *rc639Static, r []byte, s *rc637SharedProof) { s.CPU_BCandidates[0].Destinations[0].Hits = 0 },
		"wrong-runtime-provenance": func(d *rc639Static, r []byte, s *rc637SharedProof) { s.Status = "NOT_VERIFIED" },
		"bad-branch-target":        func(d *rc639Static, r []byte, s *rc637SharedProof) { d.Sites[1].FlowTargets = []string{"0x0004DE"} },
	}
	for name, change := range tests {
		t.Run(name, func(t *testing.T) {
			d, raw, s := rc6312Fixture()
			change(&d, raw, &s)
			if _, _, e := rc6312GeneratePair(d, raw, s); e == nil {
				t.Fatal("mutated original CPU-B pair passed")
			}
		})
	}
}
func TestRC6312GeneratedPairCXX(t *testing.T) {
	if _, e := exec.LookPath("cmake"); e != nil {
		t.Skip("CMake missing")
	}
	doc, rom, shared := rc6312Fixture()
	cpp, p, e := rc6312GeneratePair(doc, rom, shared)
	if e != nil {
		t.Fatal(e)
	}
	root := t.TempDir()
	cmake := `cmake_minimum_required(VERSION 3.20)
project(rc6312 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
add_library(chq_rc6312_cpu_b_pair SHARED pair.cpp)
add_executable(chq_rc6312_cpu_b_pair_abi pair-test.cpp)
target_link_libraries(chq_rc6312_cpu_b_pair_abi PRIVATE chq_rc6312_cpu_b_pair)
`
	for n, b := range map[string]string{"pair.cpp": cpp, "pair-test.cpp": rc6312PairTestSource(p.SourceAddress), "CMakeLists.txt": cmake} {
		if e := os.WriteFile(filepath.Join(root, n), []byte(b), 0644); e != nil {
			t.Fatal(e)
		}
	}
	for _, args := range [][]string{{"-S", root, "-B", filepath.Join(root, "build")}, {"--build", filepath.Join(root, "build"), "--config", "Release"}} {
		out, e := exec.Command("cmake", args...).CombinedOutput()
		if e != nil {
			t.Fatalf("generated multi-op compilation failed %v\n%s", e, out)
		}
	}
	exe := filepath.Join(root, "build", "chq_rc6312_cpu_b_pair_abi")
	if runtime.GOOS == "windows" {
		exe = filepath.Join(root, "build", "Release", "chq_rc6312_cpu_b_pair_abi.exe")
	}
	out, e := exec.Command(exe).CombinedOutput()
	if e != nil || !strings.Contains(string(out), "PASS RC6312 Ghidra CPU-B TST.W+BEQ pair ABI 4/4") {
		t.Fatalf("C++ pair runtime failed %v %s", e, out)
	}
}
func TestRC6312RealWindowsGhidraEvidenceOptional(t *testing.T) {
	path := os.Getenv("CHQ_RC6311_EVIDENCE_ZIP")
	if path == "" {
		t.Skip("set CHQ_RC6311_EVIDENCE_ZIP for real Windows evidence")
	}
	z, e := zip.OpenReader(path)
	if e != nil {
		t.Fatal(e)
	}
	defer z.Close()
	var doc rc639Static
	var shared rc637SharedProof
	for _, f := range z.File {
		switch f.Name {
		case "cpu-b-ghidra/cpu-b-ghidra-sites.json":
			r, _ := f.Open()
			if e := json.NewDecoder(r).Decode(&doc); e != nil {
				t.Fatal(e)
			}
			r.Close()
		case "shared-dispatch-cpu-b-proof.json":
			r, _ := f.Open()
			if e := json.NewDecoder(r).Decode(&shared); e != nil {
				t.Fatal(e)
			}
			r.Close()
		}
	}
	// Authentic extension is independently SHA/CRC checked on Windows during
	// rc639SubROM, and this recorded Ghidra operand is tested against it.
	_, rom, _ := rc6312Fixture()
	_, p, e := rc6312GeneratePair(doc, rom, shared)
	if e != nil || p.OriginalConsecutiveEdges < 17000 {
		t.Fatalf("Windows pair provenance failed: %v %+v", e, p)
	}
}

func TestRC6312CurrentMaturityRejectsUnsupportedPromotion(t *testing.T) {
	d := t.TempDir()
	current, e := rc6312WriteCurrentMaturity(d)
	if e != nil || current.CPUBBEQ != "NOT_RUN" || current.CPUBPair != "NOT_RUN" || current.AutonomousPromotionAllowed {
		t.Fatalf("offline maturity false claim: %+v %v", current, e)
	}
	pair := rc6312PairProof{Schema: "chq-rc6312-cpu-b-tst-beq-pair-v1", Status: "CPU_B_TWO_INSTRUCTION_GHIDRA_ABI_PASS_NOT_LIVE", OriginalConsecutiveEdges: 17903, ABIPaths: 4}
	b, _ := json.Marshal(pair)
	if e := os.WriteFile(filepath.Join(d, "cpu-b-two-instruction-proof.json"), b, 0600); e != nil {
		t.Fatal(e)
	}
	now, e := rc6312WriteCurrentMaturity(d)
	if e != nil || now.CPUBPair != "STATIC_GHIDRA_GENERATED_ABI_NOT_LIVE" || now.CPUBPairConsecutiveOriginalEdges != 17903 {
		t.Fatalf("pair not recognized: %+v %v", now, e)
	}
	pair.LivePairSubstitutionProven = true
	b, _ = json.Marshal(pair)
	os.WriteFile(filepath.Join(d, "cpu-b-two-instruction-proof.json"), b, 0600)
	if _, e := rc6312WriteCurrentMaturity(d); e == nil {
		t.Fatal("forged pair live-promotion flag accepted")
	}
}
