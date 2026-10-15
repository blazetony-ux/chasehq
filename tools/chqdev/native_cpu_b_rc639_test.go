package main

import (
	"encoding/json"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"
	"testing"
)

func rc639SyntheticSites() (rc639Static, []rc637CPU_BCandidate) {
	const sh = "aabbccddeeff00112233445566778899aabbccddeeff00112233445566778899"
	doc := rc639Static{Schema: "chq-rc639-cpu-b-ghidra-sites-v1", CPU: "B", Language: "68000:BE:32:default", RawROM: sh}
	spec := []struct {
		pc, op, asm string
		size        int
		flows       []string
	}{
		{"0x0004D8", "0x4A79", "TST.W (0x00108400).L", 6, nil},
		{"0x0004DE", "0x67F8", "BEQ 0x0004D8", 2, []string{"0x0004D8"}},
		{"0x004FD0", "0x3018", "MOVE.W (A0)+,D0", 2, nil},
		{"0x004FD2", "0x2440", "MOVEA.L D0,A2", 2, nil},
		{"0x004FD4", "0x2A2A", "MOVE.L (0x4,A2),D5", 4, nil},
	}
	var hot []rc637CPU_BCandidate
	for _, v := range spec {
		doc.Sites = append(doc.Sites, rc639Site{PC: v.pc, Opcode: v.op, Text: v.asm, Length: v.size, FlowTargets: v.flows})
		hot = append(hot, rc637CPU_BCandidate{PC: v.pc, OpcodeWord: v.op})
	}
	return doc, hot
}
func TestRC639SyntheticGhidraContractAndGeneratedABI(t *testing.T) {
	doc, hot := rc639SyntheticSites()
	b, _ := json.Marshal(doc)
	checked, e := rc639ValidateGhidra(b, doc.RawROM, hot)
	if e != nil {
		t.Fatal(e)
	}
	cpp, e := rc639GeneratedBranch(checked)
	if e != nil {
		t.Fatal(e)
	}
	for _, frag := range []string{"in->pc!=0x4DEU", "in->opcode!=0x67F8U", "taken?0x4D8U:0x4E0U", "out->cycles=taken?10U:8U"} {
		if !strings.Contains(cpp, frag) {
			t.Fatalf("generated semantic mismatch: %q", frag)
		}
	}
	if _, e = rc639SubROM(t.TempDir()); e == nil {
		t.Fatal("missing original ROM erroneously accepted")
	}
}
func TestRC639SyntheticGhidraProvenanceMutantsFailClosed(t *testing.T) {
	base, hot := rc639SyntheticSites()
	cases := map[string]func(*rc639Static){
		"wrong-cpu":                 func(d *rc639Static) { d.CPU = "A" },
		"wrong-hash":                func(d *rc639Static) { d.RawROM = "00" },
		"missing-site":              func(d *rc639Static) { d.Sites = d.Sites[:4] },
		"forged-opcode":             func(d *rc639Static) { d.Sites[1].Opcode = "0x67FA" },
		"forged-branch-destination": func(d *rc639Static) { d.Sites[1].FlowTargets = []string{"0x0004DA"} },
		"unsupported-length":        func(d *rc639Static) { d.Sites[1].Length = 4 },
		"unreviewed-cannot-promote": func(d *rc639Static) { d.NativePromotionAllowed = true },
		"false-function-review":     func(d *rc639Static) { d.Sites[1].FunctionReviewed = true },
		"duplicated-site":           func(d *rc639Static) { d.Sites[4] = d.Sites[3] },
		"unsupported-beq":           func(d *rc639Static) { d.Sites[1].Text = "BSR 0x0004D8" },
	}
	for name, mutate := range cases {
		t.Run(name, func(t *testing.T) {
			d := base
			d.Sites = append([]rc639Site(nil), base.Sites...)
			mutate(&d)
			b, _ := json.Marshal(d)
			if _, e := rc639ValidateGhidra(b, base.RawROM, hot); e == nil {
				t.Fatal("forged Ghidra report accepted")
			}
		})
	}
}

func TestRC639GeneratedSyntheticCXXABI(t *testing.T) {
	if _, e := exec.LookPath("cmake"); e != nil {
		t.Skip("CMake unavailable")
	}
	doc, _ := rc639SyntheticSites()
	code, e := rc639GeneratedBranch(doc)
	if e != nil {
		t.Fatal(e)
	}
	dir := t.TempDir()
	files := map[string]string{
		"generated-cpu-b.cpp":      code,
		"generated-cpu-b-test.cpp": rc639BranchTestSource(),
		"CMakeLists.txt":           "cmake_minimum_required(VERSION 3.20)\nproject(rc639_cpu_b_fixture LANGUAGES CXX)\nset(CMAKE_CXX_STANDARD 17)\nadd_library(chq_rc639_cpu_b SHARED generated-cpu-b.cpp)\nadd_executable(chq_rc639_cpu_b_abi generated-cpu-b-test.cpp)\ntarget_link_libraries(chq_rc639_cpu_b_abi PRIVATE chq_rc639_cpu_b)\n",
	}
	for name, s := range files {
		if e = os.WriteFile(filepath.Join(dir, name), []byte(s), 0600); e != nil {
			t.Fatal(e)
		}
	}
	build := filepath.Join(dir, "build")
	for _, args := range [][]string{{"-S", dir, "-B", build}, {"--build", build, "--config", "Release"}} {
		out, err := exec.Command("cmake", args...).CombinedOutput()
		if err != nil {
			t.Fatalf("CMake build: %v: %s", err, out)
		}
	}
	binary := filepath.Join(build, "chq_rc639_cpu_b_abi")
	if runtime.GOOS == "windows" {
		binary = filepath.Join(build, "Release", "chq_rc639_cpu_b_abi.exe")
	}
	out, err := exec.Command(binary).CombinedOutput()
	if err != nil || !strings.Contains(string(out), "PASS RC639 generated CPU-B Ghidra BEQ ABI 2/2") {
		t.Fatalf("C++ generated ABI mismatch: %v: %s", err, out)
	}
}
