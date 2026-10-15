package main

// RC6.2.4: bounded, ROM-provenance-preserving steering pipeline acceptance.
// Only verified normal-mode instruction substitution is eligible.
// Broader function boundaries remain unreviewed and never silently promoted.
import (
	"archive/zip"
	"bufio"
	"bytes"
	"context"
	"crypto/sha256"
	"encoding/binary"
	"encoding/csv"
	"encoding/hex"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"sort"
	"strconv"
	"strings"
	"time"
)

type nativeGate struct {
	Name   string `json:"name"`
	State  string `json:"state"`
	Detail string `json:"detail"`
}
type nativeFinding struct {
	PC          string `json:"pc"`
	Instruction string `json:"instruction"`
	Function    *struct {
		Entry string `json:"entry"`
		Name  string `json:"name"`
	} `json:"functionCandidate"`
	Instructions []struct {
		PC   string `json:"pc"`
		Text string `json:"text"`
	} `json:"nearbyInstructions"`
}
type nativeGhidra struct {
	Schema    string          `json:"schema"`
	CPU       string          `json:"cpu"`
	RawRomSHA string          `json:"rawRomSha256"`
	Sites     []nativeFinding `json:"sites"`
}
type nativeShadow struct {
	Schema                string `json:"schema"`
	Enabled               bool   `json:"enabled"`
	ReadOnly              bool   `json:"readOnly"`
	GeneratedLibraryUsed  bool   `json:"generatedLibraryUsed"`
	Attempts              int64  `json:"attempts"`
	Commits               int64  `json:"commits"`
	Rollbacks             int64  `json:"rollbacks"`
	DisabledAfterRollback bool   `json:"disabledAfterRollback"`
	Substitution          bool   `json:"substitution"`
	Entries               int64  `json:"entries"`
	Compared              int64  `json:"compared"`
	Matches               int64  `json:"matches"`
	Mismatches            int64  `json:"mismatches"`
	Incomplete            int64  `json:"incomplete"`
	UnsupportedMode       int64  `json:"unsupportedMode"`
	Pending               bool   `json:"pending"`
}

type nativeAccept struct {
	Schema    string            `json:"schema"`
	Release   string            `json:"release"`
	Status    string            `json:"status"`
	Scope     string            `json:"scope"`
	Inputs    map[string]string `json:"inputs"`
	Gates     []nativeGate      `json:"gates"`
	Contract  map[string]any    `json:"contract"`
	Promotion map[string]any    `json:"promotion"`
	Evidence  map[string]string `json:"evidence"`
}

func nativeAcceptCommand(root string, cfg Config, args []string) *Report {
	r := newReport("native accept", cfg.Release)
	opts := map[string]string{}
	for i := 0; i < len(args); i++ {
		switch args[i] {
		case "accept":
		case "--fresh":
			opts["fresh"] = "true"
		case "--ghidra-zip", "--out":
			if i+1 >= len(args) {
				r.fail("arguments", "option requires path: "+args[i])
				return r
			}
			opts[args[i]] = args[i+1]
			i++
		default:
			r.fail("arguments", "unsupported option: "+args[i])
			return r
		}
	}
	if opts["fresh"] != "true" {
		r.fail("usage", "chqdev native accept --fresh [--ghidra-zip PATH] [--out DIRECTORY]")
		return r
	}
	dest := opts["--out"]
	if dest == "" {
		dest = filepath.Join(root, "evidence", "native-accept-"+time.Now().Format("20060102-150405"))
	}
	if !filepath.IsAbs(dest) {
		dest = filepath.Join(root, dest)
	}
	if err := prepareNativeEvidenceDestination(dest); err != nil {
		r.fail("output", err.Error())
		return r
	}
	result := nativeAccept{Schema: "chq-native-m1-m7-acceptance-v1", Release: cfg.Release, Status: "NOT_SIGNED_OFF", Scope: "CPU-A steering normal-mode opcode at 0x820C; full enclosing function and general autonomous reconstruction NOT claimed", Inputs: map[string]string{}, Contract: map[string]any{}, Promotion: map[string]any{"authorized": false, "substitution": false}, Evidence: map[string]string{}}
	stage := func(name string, err error, detail string) bool {
		if err != nil {
			result.Gates = append(result.Gates, nativeGate{name, "FAIL", err.Error()})
			r.fail(name, err.Error())
			return false
		}
		result.Gates = append(result.Gates, nativeGate{name, "PASS", detail})
		r.pass(name, detail)
		return true
	}
	// A complete evidence ZIP must be produced even when an intermediate gate fails.
	defer func() {
		// An incomplete run is a technical failure, even if its diagnostic stages passed.
		if result.Status != "SIGNED_OFF" {
			result.Promotion["authorized"] = false
			result.Promotion["substitution"] = false
			found := false
			for _, g := range result.Gates {
				if g.Name == "FULL_M1_M7_SIGN_OFF" {
					found = true
					break
				}
			}
			if !found {
				result.Gates = append(result.Gates, nativeGate{"FULL_M1_M7_SIGN_OFF", "BLOCKED", "one or more evidence stages aborted or full-state proof missing"})
			}
			if r.Status == "PASS" {
				r.fail("full-M1-M7-signoff", "M1–M7 instruction-scoped acceptance incomplete or blocked; see acceptance.json")
			}
		}
		b, _ := json.MarshalIndent(result, "", "  ")
		_ = os.WriteFile(filepath.Join(dest, "acceptance.json"), append(b, '\n'), 0644)
		if err := writeNativeEvidenceManifest(dest); err != nil {
			r.fail("evidence-manifest", err.Error())
			return
		}
		if err := bundleNativeEvidence(dest); err != nil {
			r.fail("evidence-zip", err.Error())
		} else {
			r.pass("evidence-zip", dest+".zip")
			r.Data["bundle"] = dest + ".zip"
		}
		r.Data["acceptanceStatus"] = result.Status
		r.Data["output"] = dest
	}()
	ghidra := opts["--ghidra-zip"]
	if ghidra == "" {
		ghidra = filepath.Join(root, "research", "acceptance", "ghidra-steering-RC621-verified.zip")
	}
	g, raw, err := readNativeGhidra(ghidra)
	if !stage("M1-Ghidra", err, "real CPU-A Ghidra evidence parsed; steering 0x820C present; function boundary provisional") {
		return r
	}
	result.Inputs["ghidraZipSha256"] = fileHash(ghidra)
	result.Inputs["ghidraFindingsSha256"] = hexDigest(raw)
	result.Inputs["rawRomSha256"] = g.RawRomSHA
	romDir, romRootErr := nativeResolveROMDir(root, cfg)
	if !stage("M1-ROM-source", romRootErr, "validated staged ROMs or shared AdditionalFiles discovery") {
		return r
	}
	rom, err := nativeMainROM(romDir)
	if err == nil && !strings.EqualFold(hexDigest(rom), g.RawRomSHA) {
		err = fmt.Errorf("ROM fingerprint mismatch: Ghidra=%s actual=%s", g.RawRomSHA, hexDigest(rom))
	}
	if !stage("M1-ROM-identity", err, "assembled CPU-A ROM exactly matches Ghidra SHA256") {
		return r
	}
	result.Inputs["romSource"] = "validated staged World ROM; raw ROM bytes excluded from evidence"
	// An exact, independently decoded opcode boundary is the protected
	// replacement unit. The enclosing Ghidra *function* remains unreviewed.
	opcodeErr := error(nil)
	if binary.BigEndian.Uint16(rom[0x820c:0x820e]) != 0x3b50 || binary.BigEndian.Uint16(rom[0x820e:0x8210]) != 0x8300 {
		opcodeErr = errors.New("authentic CPU-A ROM is not MOVE.W (A0),(-0x7d00,A5) at 0x820C")
	}
	if !stage("M1-reviewed-instruction-boundary", opcodeErr, "Ghidra instruction encoding 3B50 8300 independently confirmed in authentic original ROM; enclosing function boundaries remain provisional") {
		return r
	}
	runtimeExe := filepath.Join(root, "out", "build", "x64-Debug", "Debug", "ChaseHQRuntime.exe")
	if runtime.GOOS != "windows" {
		runtimeExe = filepath.Join(root, "out", "build", "x64-Debug", "ChaseHQRuntime")
	}
	if _, err := os.Stat(runtimeExe); !stage("M2-runtime-executable", err, "standalone ChaseHQRuntime executable from qualified build") {
		return r
	}
	checkpoint := filepath.Join(root, "checkpoints", "stage1-gameplay-2064.chqstate")
	if _, err := os.Stat(checkpoint); !stage("M2-checkpoint", err, "canonical gameplay checkpoint frame 2064 available") {
		return r
	}
	logs := filepath.Join(dest, "runtime")
	_ = os.MkdirAll(logs, 0755)
	params := []string{"--roms", romDir, "--load-checkpoint", checkpoint, "--frames", "2304", "--irq4", "--rom-census", "--native-steering-shadow", "--native-observe-entry", "A:0x820C", "--steering-signed-range", "2064:2111:-96", "--steering-signed-range", "2112:2159:0", "--steering-signed-range", "2160:2207:96", "--steering-signed-range", "2208:2255:-32", "--steering-signed-range", "2256:2303:32", "--native-steering-state-capture", "--logs", logs}
	runContext, cancelRun := context.WithTimeout(context.Background(), 8*time.Minute)
	cmd := exec.CommandContext(runContext, runtimeExe, params...)
	cmd.Dir = root
	output, runErr := cmd.CombinedOutput()
	cancelRun()
	_ = os.WriteFile(filepath.Join(dest, "runtime-console.txt"), output, 0644)
	if !stage("M2-live-runtime", runErr, "fresh standalone original CPU-A/B execution from checkpoint; 240 frames with signed steering interventions") {
		return r
	}
	counts, countErr := readNativeTsv(filepath.Join(logs, "rom-execution-pc-counts.tsv"))
	if countErr == nil && counts["A:0x00820C"] <= 0 {
		countErr = errors.New("CPU-A steering PC 0x820C received no execution hits")
	}
	if !stage("M2-runtime-coverage", countErr, fmt.Sprintf("original ROM writer hits=%d; authoritative per-PC hit census", counts["A:0x00820C"])) {
		return r
	}
	result.Evidence["steeringWriterHits"] = strconv.FormatInt(counts["A:0x00820C"], 10)
	ledger := nativeSteeringCoverage(g, counts)
	ledgerJSON, _ := json.MarshalIndent(ledger, "", "  ")
	_ = os.WriteFile(filepath.Join(dest, "coverage-ledger.json"), append(ledgerJSON, '\n'), 0644)
	if !stage("M2-census-integrity", checkRuntimeCensus(logs, counts["A:0x00820C"]), "CPU A/B instruction-hook census and observation bridge consistent; no native replacements") {
		return r
	}
	// Contract derivation is intentionally bounded. Static instruction patterns,
	// verified ROM lookups, and live observed writes all must corroborate.
	table, contract, err := deriveSteeringContract(g, rom, counts["A:0x00820C"])
	result.Contract = contract
	if !stage("M3-contract", err, "CPU-A 0x820C read/write and normal-mode ROM table dependencies correlated; unresolved effects recorded") {
		return r
	}
	vectors, vecErr := nativeSteeringVectors(filepath.Join(root, "docs", "reference", "steering_reference_193.csv"), table)
	if !stage("M3-independent-reference", vecErr, "193 prior independent CPU-A ROM steering samples match derived live-ROM table") {
		return r
	}
	code := generateNativeSteering(table, vectors)
	generated := filepath.Join(dest, "generated")
	_ = os.MkdirAll(generated, 0755)
	_ = os.WriteFile(filepath.Join(generated, "steering.cpp"), []byte(code), 0644)
	wrong := strings.Replace(code, fmt.Sprintf("table={{%d,", table[0]), fmt.Sprintf("table={{%d,", table[0]+1), 1)
	if wrong == code {
		r.fail("M4-negative", "cannot construct mutated independent native candidate")
		return r
	}
	_ = os.WriteFile(filepath.Join(generated, "steering-negative.cpp"), []byte(wrong), 0644)
	_ = os.WriteFile(filepath.Join(generated, "CMakeLists.txt"), []byte("cmake_minimum_required(VERSION 3.20)\nproject(CHQGeneratedSteering LANGUAGES CXX)\nset(CMAKE_CXX_STANDARD 17)\nadd_executable(chq_steering_accept steering.cpp)\nadd_executable(chq_steering_negative steering-negative.cpp)\nadd_library(chq_steering_library SHARED steering.cpp)\n"), 0644)
	cmake, err := findTool(root, "cmake")
	if !stage("M4-CMake", err, "host CMake found") {
		return r
	}
	buildDir := filepath.Join(dest, "generated-build")
	configureCtx, cancelConfigure := context.WithTimeout(context.Background(), 2*time.Minute)
	configure := exec.CommandContext(configureCtx, cmake, "-S", generated, "-B", buildDir)
	configure.Dir = root
	cfgLog, confErr := configure.CombinedOutput()
	cancelConfigure()
	_ = os.WriteFile(filepath.Join(dest, "generator-configure.log"), cfgLog, 0644)
	if !stage("M4-configure", confErr, "generated standalone C++17 executable configured") {
		return r
	}
	buildCtx, cancelBuild := context.WithTimeout(context.Background(), 3*time.Minute)
	build := exec.CommandContext(buildCtx, cmake, "--build", buildDir, "--config", "Release")
	build.Dir = root
	buildLog, buildErr := build.CombinedOutput()
	cancelBuild()
	_ = os.WriteFile(filepath.Join(dest, "generator-build.log"), buildLog, 0644)
	if !stage("M4-compile", buildErr, "generated C++17 built independently from original CPU emulator") {
		return r
	}
	exe := filepath.Join(buildDir, "chq_steering_accept")
	if runtime.GOOS == "windows" {
		exe = filepath.Join(buildDir, "Release", "chq_steering_accept.exe")
	}
	genCtx, cancelGen := context.WithTimeout(context.Background(), 30*time.Second)
	generatedCmd := exec.CommandContext(genCtx, exe)
	generatedCmd.Dir = root
	genLog, genErr := generatedCmd.CombinedOutput()
	cancelGen()
	_ = os.WriteFile(filepath.Join(dest, "generated-verify.log"), genLog, 0644)
	if genErr == nil && !strings.Contains(string(genLog), "PASS generated native steering vs independent 193-vector ROM evidence 193/193") {
		genErr = errors.New("generated binary omitted 193-vector PASS marker")
	}
	if !stage("M4-execute", genErr, "193 signed steering reference inputs checked against independent Run028 vectors by generated native C++") {
		return r
	}
	badExe := filepath.Join(buildDir, "chq_steering_negative")
	if runtime.GOOS == "windows" {
		badExe = filepath.Join(buildDir, "Release", "chq_steering_negative.exe")
	}
	negativeCtx, cancelNeg := context.WithTimeout(context.Background(), 30*time.Second)
	negativeCmd := exec.CommandContext(negativeCtx, badExe)
	negativeOut, negativeRunErr := negativeCmd.CombinedOutput()
	cancelNeg()
	_ = os.WriteFile(filepath.Join(dest, "generated-negative-verify.log"), negativeOut, 0644)
	var negativeErr error
	var exitErr *exec.ExitError
	if !errors.As(negativeRunErr, &exitErr) || exitErr.ExitCode() != 2 {
		negativeErr = fmt.Errorf("mutated generated binary not rejected with expected exit 2: %v", negativeRunErr)
	}
	if !stage("M4-mutated-candidate-rejection", negativeErr, "separately compiled intentionally corrupted lookup candidate rejected by independent 193-vector oracle") {
		return r
	}
	var sh nativeShadow
	data, readErr := os.ReadFile(filepath.Join(logs, "native-steering-shadow.json"))
	if readErr == nil {
		readErr = json.Unmarshal(data, &sh)
	}
	if readErr == nil && (sh.Schema != "chq-native-steering-shadow-v1" || !sh.Enabled || !sh.ReadOnly || sh.Substitution || sh.Compared <= 0 || sh.Matches != sh.Compared || sh.Mismatches != 0 || sh.Pending) {
		readErr = fmt.Errorf("shadow not complete: compared=%d matches=%d mismatches=%d pending=%v", sh.Compared, sh.Matches, sh.Mismatches, sh.Pending)
	}
	readErr = validateNativeShadow(sh, readErr)
	if !stage("M5-read-only-live-shadow", readErr, fmt.Sprintf("scalar-output only: original vs independent steering: %d/%d compared, no mismatches; full CPU state unproven", sh.Matches, sh.Compared)) {
		return r
	}
	// A/B replay runs the same 240 frames and IOC interventions under three
	// executions: original ROM, real native opcode substitution, and a
	// deliberately corrupted native candidate that MUST roll back to ROM.
	// Each captures all public CPU D/A/PC/SR registers at the return boundary,
	// cycle position, and a deterministic complete mapped-machine end-state.
	// These runs are not read-only shadows; commit/rollback counters are verified.
	libraryName := "libchq_steering_library.so"
	libraryDir := buildDir
	if runtime.GOOS == "windows" {
		libraryName = "chq_steering_library.dll"
		libraryDir = filepath.Join(buildDir, "Release")
	}
	generatedLibrary := filepath.Join(libraryDir, libraryName)
	if _, err := os.Stat(generatedLibrary); !stage("M4-generated-shared-library", err, "compiled source-candidate library, loaded during live CPU instruction replacement") {
		return r
	}
	result.Inputs["generatedCandidateSha256"] = fileHash(generatedLibrary)
	replay, replayErr := nativeReplaySteering(root, runtimeExe, romDir, checkpoint, dest, params, logs, generatedLibrary)
	result.Evidence["nativeReplay"] = "native-replay.json"
	if replay != nil {
		b, _ := json.MarshalIndent(replay, "", "  ")
		_ = os.WriteFile(filepath.Join(dest, "native-replay.json"), append(b, '\n'), 0644)
	}
	if !stage("M5-native-execution-replay", replayErr, "ROM vs executed native opcode: all D/A/PC/SR registers, return-boundary cycles, full mapped end-state, and deliberate native rollback parity") {
		return r
	}
	// Negative gates execute real validators against controlled corruptions.
	negatives := runNativeNegativeTests(g, rom, sh, result.Contract)
	negJSON, _ := json.MarshalIndent(negatives, "", "  ")
	_ = os.WriteFile(filepath.Join(dest, "negative-tests.json"), append(negJSON, '\n'), 0644)
	negErr := error(nil)
	for name, rejected := range negatives {
		if !rejected {
			negErr = fmt.Errorf("negative safety fixture was erroneously accepted: %s", name)
			break
		}
	}
	if !stage("M6-negative-rejection", negErr, "real wrong-ROM, corrupted shadow, and unreviewed-function tests all rejected") {
		return r
	}
	// Grounded contract promotion: only the exact, reviewed 68000 opcode.
	// Never misrepresent the surrounding 0x81C4-0x8211 function as replaced.
	// Each effect below requires the *real* A/B/rollback proof above, not a
	// constructed Boolean or an offline vector fixture.
	result.Contract["candidateInstructionReviewed"] = true
	result.Contract["registerEffectsVerified"] = true
	result.Contract["cpuFlagsVerified"] = true
	result.Contract["orderedBusEffectsVerified"] = replay.BusTransactionsEqual
	result.Contract["cycleEquivalenceVerified"] = replay.ReturnStateBytesEqual && replay.FullMachineEndstateEqual
	result.Contract["controlFlowVerified"] = replay.ReturnStateBytesEqual && replay.NativeCommits > 0
	result.Contract["rollbackVerified"] = replay.ForcedMismatchRollbacks == 1 && replay.RollbackRestoresOriginal
	result.Contract["independentLiveStateParityVerified"] = replay.FullMachineEndstateEqual
	result.Contract["generatedCandidateLoadedAndExecutedVerified"] = replay.NativeCommits > 0
	result.Contract["fullFunctionReviewed"] = false
	result.Contract["unresolvedDependencies"] = []string{"enclosing unreviewed function and alternate nonzero-gate branch beyond this instruction", "generalized multi-routine/CPU-B reconstruction is RC6.3 work"}
	result.Contract["scope"] = "exact original-ROM 68000 MOVE.W instruction at A:0x00820C only; NOT full enclosing Ghidra function"
	result.Contract["readyForNativeReplacement"] = true
	eligible := nativePromotionEligibility(result.Contract, sh, g.RawRomSHA, rom)
	// The negative tests above *must* have been exercised while the contract
	// was incomplete. Also reject tampering against the now-complete candidate.
	validNegatives := eligible && runNativeNegativeTestsAfterProof(result.Contract, sh, g.RawRomSHA, rom)
	if !stage("M6-verified-eligibility", boolError(!validNegatives, "completed candidate or mutation safety gates failed"), "native generated library actually executed; forced mismatch was rolled back; corrupted evidence still rejected") {
		return r
	}
	result.Promotion = map[string]any{
		"authorized": true, "substitution": true,
		"candidateScope":           "A:0x00820C MOVE.W only; function boundary not replaced",
		"decision":                 "ELIGIBLE_OPT_IN_BOUNDED_INSTRUCTION",
		"originalEmulationDefault": true,
		"generatedLibrarySha256":   result.Inputs["generatedCandidateSha256"],
		"nativeCommits":            replay.NativeCommits, "forcedMismatchRollbacks": replay.ForcedMismatchRollbacks,
		"negativeFixtures": negatives,
	}
	if !stage("M7-evidence-orchestration", nil, "unattended original/Ghidra/runtime/compiled-library/native/rollback comparison; integrity-hashed automatic evidence ZIP") {
		return r
	}
	result.Scope = "M1-M7 BOUNDED FOUNDATION: single genuine CPU-A ROM MOVE.W at 0x820C, opt-in guarded native generated-library execution; NOT an entire function, not generalized CPU A/B autonomous reconstruction"
	result.Gates = append(result.Gates, nativeGate{"FULL_M1_M7_SIGN_OFF", "PASS", "Seven milestone gates proven for precisely scoped single instruction; verified guarded native library execution, original-ROM CPU/bus/memory/timing parity and rollback; broader functions deferred to RC6.3"})
	result.Status = "SIGNED_OFF"
	r.pass("full-M1-M7-signoff", "SIGNED_OFF bounded CPU-A instruction reconstruction pipeline; Ghidra function remains original ROM; see scope and evidence")
	return r
}

type nativeReplayResult struct {
	Schema                       string   `json:"schema"`
	OriginalReturnEvents         int      `json:"originalReturnEvents"`
	NativeReturnEvents           int      `json:"nativeReturnEvents"`
	NativeCommits                int64    `json:"nativeCommits"`
	NativeAttempts               int64    `json:"nativeAttempts"`
	ForcedMismatchRollbacks      int64    `json:"forcedMismatchRollbacks"`
	ReturnStateBytesEqual        bool     `json:"returnStateBytesEqual"`
	FullMachineEndstateEqual     bool     `json:"fullMachineEndstateEqual"`
	BusTransactionsEqual         bool     `json:"busTransactionsEqual"`
	OriginalBusEventsSha256      string   `json:"originalBusEventsSha256"`
	NativeBusEventsSha256        string   `json:"nativeBusEventsSha256"`
	RollbackRestoresOriginal     bool     `json:"rollbackRestoresOriginal"`
	RollbackBusTransactionsEqual bool     `json:"rollbackBusTransactionsEqual"`
	OriginalReturnStatesSha256   string   `json:"originalReturnStatesSha256"`
	NativeReturnStatesSha256     string   `json:"nativeReturnStatesSha256"`
	OriginalMachineSha256        string   `json:"originalMachineSha256"`
	NativeMachineSha256          string   `json:"nativeMachineSha256"`
	Limitations                  []string `json:"limitations"`
}

func nativeReplaySteering(root, executable, romDir, checkpoint, dest string, originalArgs []string, originalDir string, generatedLibrary string) (*nativeReplayResult, error) {
	result := &nativeReplayResult{Schema: "chq-native-steering-instruction-ab-v1", Limitations: []string{"A/B replay demonstrates a single ROM MOVE.W at CPU-A 0x820C, not the complete enclosing function or CPU-B reconstruction", "Machine comparison covers mapped mutable RAM/video, palette, all public CPU registers and cycles; not independently timestamped external device transactions", "Generated shared library is loaded dynamically by the live substitute; Ghidra function boundaries remain unreviewed"}}
	checkFile := func(p string) ([]byte, error) { return os.ReadFile(p) }
	_ = originalArgs // sequence mirrored below explicitly to keep the baseline immutable
	run := func(label, mode string) (string, nativeShadow, error) {
		var shadow nativeShadow
		dir := filepath.Join(dest, label)
		if e := os.MkdirAll(dir, 0755); e != nil {
			return dir, shadow, e
		}
		args := []string{"--roms", romDir, "--load-checkpoint", checkpoint, "--frames", "2304", "--irq4", "--native-steering-shadow", "--native-steering-state-capture",
			"--steering-signed-range", "2064:2111:-96", "--steering-signed-range", "2112:2159:0", "--steering-signed-range", "2160:2207:96", "--steering-signed-range", "2208:2255:-32", "--steering-signed-range", "2256:2303:32", "--logs", dir}
		if mode != "" {
			args = append(args, mode, "--native-steering-library", generatedLibrary)
		}
		ctx, cancel := context.WithTimeout(context.Background(), 8*time.Minute)
		cmd := exec.CommandContext(ctx, executable, args...)
		cmd.Dir = root
		out, err := cmd.CombinedOutput()
		cancel()
		_ = os.WriteFile(filepath.Join(dir, "run-console.txt"), out, 0644)
		if err != nil {
			return dir, shadow, fmt.Errorf("%s emulator replay failed: %w", label, err)
		}
		b, e := os.ReadFile(filepath.Join(dir, "native-steering-shadow.json"))
		if e != nil {
			return dir, shadow, e
		}
		e = json.Unmarshal(b, &shadow)
		if e != nil {
			return dir, shadow, e
		}
		return dir, shadow, nil
	}
	// The first, separately collected M2 original run is reused rather than
	// manufacturing a second reference oracle.
	origStates, e := checkFile(filepath.Join(originalDir, "native-steering-state.tsv"))
	if e != nil {
		return result, e
	}
	origEnd, e := checkFile(filepath.Join(originalDir, "native-machine-endstate.json"))
	if e != nil {
		return result, e
	}
	if len(origStates) == 0 || len(origEnd) == 0 {
		return result, errors.New("original replay missing return states or machine state")
	}
	result.OriginalReturnEvents = bytes.Count(origStates, []byte{'\n'})
	result.OriginalReturnStatesSha256 = hexDigest(origStates)
	result.OriginalMachineSha256 = hexDigest(origEnd)
	candidateDir, candidate, e := run("native-replacement", "--native-steering-replace")
	if e != nil {
		return result, e
	}
	candidateStates, e := checkFile(filepath.Join(candidateDir, "native-steering-state.tsv"))
	if e != nil {
		return result, e
	}
	candidateEnd, e := checkFile(filepath.Join(candidateDir, "native-machine-endstate.json"))
	if e != nil {
		return result, e
	}
	result.NativeReturnEvents = bytes.Count(candidateStates, []byte{'\n'})
	result.NativeCommits = candidate.Commits
	result.NativeAttempts = candidate.Attempts
	result.NativeReturnStatesSha256 = hexDigest(candidateStates)
	result.NativeMachineSha256 = hexDigest(candidateEnd)
	result.ReturnStateBytesEqual = bytes.Equal(origStates, candidateStates)
	result.FullMachineEndstateEqual = bytes.Equal(origEnd, candidateEnd)
	originalBus, busErr := checkFile(filepath.Join(originalDir, "native-steering-bus.tsv"))
	if busErr != nil {
		return result, busErr
	}
	nativeBus, busErr := checkFile(filepath.Join(candidateDir, "native-steering-bus.tsv"))
	if busErr != nil {
		return result, busErr
	}
	result.OriginalBusEventsSha256 = hexDigest(originalBus)
	result.NativeBusEventsSha256 = hexDigest(nativeBus)
	result.BusTransactionsEqual = len(originalBus) > 0 && bytes.Equal(originalBus, nativeBus)
	if result.OriginalReturnEvents != 240 || candidate.Commits != int64(result.OriginalReturnEvents) || candidate.Attempts != candidate.Commits || result.NativeReturnEvents != result.OriginalReturnEvents || !candidate.GeneratedLibraryUsed || candidate.ReadOnly || !candidate.Substitution || candidate.Rollbacks != 0 || candidate.Mismatches != 0 || candidate.Incomplete != 0 || candidate.Compared != candidate.Commits || candidate.Matches != candidate.Commits || candidate.Pending {
		return result, fmt.Errorf("native substitution incomplete: original=%d nativeStates=%d attempts=%d commits=%d compared=%d matches=%d incomplete=%d mismatches=%d", result.OriginalReturnEvents, result.NativeReturnEvents, candidate.Attempts, candidate.Commits, candidate.Compared, candidate.Matches, candidate.Incomplete, candidate.Mismatches)
	}
	if !result.ReturnStateBytesEqual || !result.FullMachineEndstateEqual || !result.BusTransactionsEqual {
		return result, fmt.Errorf("native parity mismatch: return states=%v endstate=%v bus=%v; source evidence retained", result.ReturnStateBytesEqual, result.FullMachineEndstateEqual, result.BusTransactionsEqual)
	}
	rollbackDir, rollback, e := run("forced-mismatch-rollback", "--native-steering-rollback-probe")
	if e != nil {
		return result, e
	}
	result.ForcedMismatchRollbacks = rollback.Rollbacks
	afterStates, e := checkFile(filepath.Join(rollbackDir, "native-steering-state.tsv"))
	if e != nil {
		return result, e
	}
	afterEnd, e := checkFile(filepath.Join(rollbackDir, "native-machine-endstate.json"))
	if e != nil {
		return result, e
	}
	rollbackBus, e := checkFile(filepath.Join(rollbackDir, "native-steering-bus.tsv"))
	if e != nil {
		return result, e
	}
	result.RollbackBusTransactionsEqual = len(originalBus) > 0 && bytes.Equal(originalBus, rollbackBus)
	result.RollbackRestoresOriginal = bytes.Equal(origStates, afterStates) && bytes.Equal(origEnd, afterEnd) && result.RollbackBusTransactionsEqual
	if rollback.Rollbacks != 1 || rollback.Commits != 0 || rollback.Attempts != 1 || !rollback.DisabledAfterRollback || rollback.Mismatches != 0 || rollback.Incomplete != 0 || rollback.Compared != int64(result.OriginalReturnEvents) || rollback.Matches != rollback.Compared || !result.RollbackRestoresOriginal {
		return result, fmt.Errorf("induced native mismatch did not revert to identical original ROM path: rollback=%d commits=%d parity=%v bus=%v", rollback.Rollbacks, rollback.Commits, result.RollbackRestoresOriginal, result.RollbackBusTransactionsEqual)
	}
	return result, nil
}

func boolError(b bool, msg string) error {
	if b {
		return errors.New(msg)
	}
	return nil
}
func hexDigest(b []byte) string { s := sha256.Sum256(b); return hex.EncodeToString(s[:]) }
func fileHash(p string) string {
	b, e := os.ReadFile(p)
	if e != nil {
		return ""
	}
	return hexDigest(b)
}
func readNativeGhidra(p string) (nativeGhidra, []byte, error) {
	var result nativeGhidra
	z, e := zip.OpenReader(p)
	if e != nil {
		return result, nil, e
	}
	defer z.Close()
	var raw []byte
	var manifest []byte
	for _, f := range z.File {
		if strings.HasSuffix(strings.ReplaceAll(f.Name, "\\", "/"), "/run-manifest.json") {
			rd, er := f.Open()
			if er != nil {
				return result, nil, er
			}
			manifest, er = io.ReadAll(io.LimitReader(rd, 1<<20))
			_ = rd.Close()
			if er != nil {
				return result, nil, er
			}
		}
		if strings.HasSuffix(strings.ReplaceAll(f.Name, "\\", "/"), "/analysis/chq-ghidra-findings.json") {
			if f.UncompressedSize64 > 1<<22 {
				return result, nil, errors.New("Ghidra findings too large")
			}
			rd, er := f.Open()
			if er != nil {
				return result, nil, er
			}
			raw, er = io.ReadAll(io.LimitReader(rd, 1<<22))
			_ = rd.Close()
			if er != nil {
				return result, nil, er
			}
			break
		}
	}
	if len(raw) == 0 || len(manifest) == 0 {
		return result, nil, errors.New("real Ghidra findings or job manifest missing")
	}
	var provenance struct {
		Schema            string   `json:"schema"`
		Release           string   `json:"releaseBaseline"`
		CPU               string   `json:"cpu"`
		ROM               string   `json:"romSha256"`
		Exit              int      `json:"ghidraExitCode"`
		Candidates        []string `json:"requestedCandidatePcs"`
		NativeReplacement bool     `json:"nativeRoutineReplacement"`
	}
	manifest = bytes.TrimPrefix(manifest, []byte{0xEF, 0xBB, 0xBF})
	if err := json.Unmarshal(manifest, &provenance); err != nil {
		return result, nil, err
	}
	requested := false
	for _, p := range provenance.Candidates {
		if strings.EqualFold(p, "0x00820C") {
			requested = true
		}
	}
	if provenance.Schema != "chq-ghidra-automation-run-v1" || provenance.Release != "0.67.0.0-RC6.2.1" || provenance.CPU != "A" || provenance.Exit != 0 || provenance.NativeReplacement || !requested {
		return result, nil, errors.New("Ghidra job provenance invalid: expected real qualified-parent Run032 steering job")
	}
	if e = json.Unmarshal(raw, &result); e != nil {
		return result, nil, e
	}
	if result.Schema != "chq-ghidra-static-findings-v1" || result.CPU != "A" || len(result.RawRomSHA) != 64 {
		return result, nil, errors.New("unexpected static findings schema, CPU or ROM identity")
	}
	if _, e = hex.DecodeString(result.RawRomSHA); e != nil {
		return result, nil, e
	}
	if !strings.EqualFold(result.RawRomSHA, provenance.ROM) {
		return result, nil, errors.New("Ghidra findings and run manifest ROM fingerprints differ")
	}
	found := false
	for _, s := range result.Sites {
		if strings.EqualFold(s.PC, "0x00820C") && s.Function != nil && len(s.Instructions) > 0 {
			found = true
		}
	}
	if !found {
		return result, nil, errors.New("steering writer 0x820C absent from Ghidra findings")
	}
	return result, raw, nil
}
func nativeMainROM(p string) ([]byte, error) {
	names := []string{"b52-130.36", "b52-136.29", "b52-131.37", "b52-129.30"}
	out := make([]byte, 0x80000)
	for lane, name := range names {
		b, e := os.ReadFile(filepath.Join(p, name))
		if e != nil {
			return nil, e
		}
		if len(b) != 0x20000 {
			return nil, fmt.Errorf("bad ROM size %s", name)
		}
		base := (lane/2)*0x40000 + lane%2
		for i, v := range b {
			out[base+i*2] = v
		}
	}
	return out, nil
}
func readNativeTsv(p string) (map[string]int64, error) {
	f, e := os.Open(p)
	if e != nil {
		return nil, e
	}
	defer f.Close()
	s := bufio.NewScanner(f)
	v := map[string]int64{}
	first := true
	for s.Scan() {
		if first {
			first = false
			continue
		}
		row := strings.Split(s.Text(), "\t")
		if len(row) != 3 {
			return nil, errors.New("census TSV columns changed")
		}
		n, er := strconv.ParseInt(row[2], 10, 64)
		if er != nil || n < 0 {
			return nil, errors.New("bad census hit count")
		}
		// Canonicalise the real Windows census format (CPU-A, 0x00820C),
		// including hex prefix case, without changing the key used by every
		// M2/M3 coverage and integrity gate: A:0x00820C.
		cpu := strings.TrimPrefix(strings.ToUpper(strings.TrimSpace(row[0])), "CPU-")
		if cpu != "A" && cpu != "B" {
			return nil, fmt.Errorf("bad census CPU %q", row[0])
		}
		pc, pcErr := strconv.ParseUint(strings.TrimSpace(row[1]), 0, 24)
		if pcErr != nil || pc&1 != 0 {
			return nil, fmt.Errorf("bad census PC %q", row[1])
		}
		key := fmt.Sprintf("%s:0x%06X", cpu, pc)
		if _, ok := v[key]; ok {
			return nil, errors.New("duplicate PC in runtime census")
		}
		v[key] = n
	}
	return v, s.Err()
}
func deriveSteeringContract(g nativeGhidra, rom []byte, hits int64) ([]int16, map[string]any, error) {
	c := map[string]any{"routineId": "A:0x00820C", "scope": "normal-mode lookup write boundary only", "romSha256": g.RawRomSHA, "writer": "0x00820C", "ramWrite": "0x100300:word", "modeGate": "0x10012E:word == 0", "input": "signed i16[-96,+96]", "lookupBase": "0x03132C", "functionReviewed": false, "registerEffectsVerified": false, "cpuFlagsVerified": false, "orderedBusEffectsVerified": false, "cycleEquivalenceVerified": false, "runtimeHits": hits, "unresolvedDependencies": []string{"function entry and complete CFG", "register and CPU flags liveness", "ordered bus effects", "full RAM and cycle parity"}, "readyForNativeReplacement": false}
	var s *nativeFinding
	for i := range g.Sites {
		if strings.EqualFold(g.Sites[i].PC, "0x00820C") {
			s = &g.Sites[i]
			break
		}
	}
	if s == nil {
		return nil, c, errors.New("Ghidra writer absent")
	}
	expected := []string{"0x0081FC", "0x008202", "0x008206", "0x008208", "0x00820A", "0x00820C", "0x008210"}
	present := map[string]string{}
	for _, i := range s.Instructions {
		present[strings.ToUpper(i.PC)] = strings.ToLower(i.Text)
	}
	for _, pc := range expected {
		if present[strings.ToUpper(pc)] == "" {
			return nil, c, fmt.Errorf("Ghidra contract instruction missing: %s", pc)
		}
	}
	if !strings.Contains(present["0X0081FC"], "0x3132c") || !strings.Contains(present["0X00820C"], "move.w") {
		return nil, c, errors.New("Ghidra disassembly no longer supports steering lookup contract")
	}
	values := make([]int16, 193)
	for x := -96; x <= 96; x++ {
		addr := 0x3132c + 2*x
		if addr < 0 || addr+2 > len(rom) {
			return nil, c, errors.New("ROM table out of bounds")
		}
		values[x+96] = int16(binary.BigEndian.Uint16(rom[addr : addr+2]))
	}
	return values, c, nil
}
func generateNativeSteering(table []int16, verifiedVectors []int16) string {
	var b strings.Builder
	b.WriteString("#include <array>\n#include <cstdint>\n#include <iostream>\n#include <algorithm>\n")
	b.WriteString("constexpr std::array<int16_t,193> table={{")
	for i, n := range table {
		if i > 0 {
			b.WriteByte(',')
		}
		b.WriteString(strconv.Itoa(int(n)))
	}
	b.WriteString("}};\nconstexpr std::array<int16_t,193> independently_observed={{")
	for i, n := range verifiedVectors {
		if i > 0 {
			b.WriteByte(',')
		}
		b.WriteString(strconv.Itoa(int(n)))
	}
	b.WriteString("}};\nint16_t convert(int16_t raw){const int i=std::clamp(static_cast<int>(raw),-96,96);return table[static_cast<unsigned>(i+96)];}\n")
	b.WriteString("#if defined(_WIN32)\n#define CHQ_EXPORT __declspec(dllexport)\n#else\n#define CHQ_EXPORT __attribute__((visibility(\"default\")))\n#endif\n")
	b.WriteString("extern \"C\" CHQ_EXPORT int16_t chq_native_steering_193(int16_t raw){return convert(raw); }\n")
	b.WriteString("int main(){int matches=0;for(int input=-96; input<=96; ++input){const int expected=independently_observed[static_cast<unsigned>(input+96)];if(convert(static_cast<int16_t>(input))!=expected)return 2;++matches;}if(convert(-32768)!=independently_observed[0]||convert(32767)!=independently_observed[192])return 3;std::cout<<\"PASS generated native steering vs independent 193-vector ROM evidence \"<<matches<<\"/193\\n\";}\n")
	return b.String()
}
func gSteeringFunctionReviewed(_ nativeGhidra) bool { return false }

// Independently recorded Run028 193-vector oracle; no on-demand generation from current ROM.
func nativeSteeringVectors(path string, romTable []int16) ([]int16, error) {
	f, e := os.Open(path)
	if e != nil {
		return nil, e
	}
	defer f.Close()
	rd := csv.NewReader(f)
	rows, e := rd.ReadAll()
	if e != nil {
		return nil, e
	}
	if len(rows) != 194 || len(rows[0]) != 4 || rows[0][0] != "signed_input" || rows[0][3] != "signed_output" {
		return nil, errors.New("193-vector independent reference CSV structure invalid")
	}
	v := make([]int16, 193)
	for i, row := range rows[1:] {
		if len(row) != 4 {
			return nil, fmt.Errorf("reference row %d has wrong columns", i)
		}
		input, e := strconv.Atoi(row[0])
		if e != nil || input != i-96 {
			return nil, fmt.Errorf("reference sequence mismatch at %d", i)
		}
		addr, e := strconv.ParseUint(strings.TrimPrefix(strings.ToLower(row[1]), "0x"), 16, 32)
		if e != nil || addr != uint64(0x3132c+2*input) {
			return nil, fmt.Errorf("reference ROM address mismatch at %d", i)
		}
		word, e := strconv.ParseUint(strings.TrimPrefix(strings.ToLower(row[2]), "0x"), 16, 16)
		if e != nil {
			return nil, fmt.Errorf("reference hex word invalid at %d", i)
		}
		out, e := strconv.ParseInt(row[3], 10, 16)
		if e != nil || int16(uint16(word)) != int16(out) || romTable[i] != int16(out) {
			return nil, fmt.Errorf("independent original-ROM vector disagrees at input %d", input)
		}
		v[i] = int16(out)
	}
	return v, nil
}
func nativeResolveROMDir(root string, cfg Config) (string, error) {
	dir := filepath.Join(root, "roms", "chasehq")
	if validateROMs(dir, cfg) == nil {
		return dir, nil
	}
	local, e := loadLocalConfig(root)
	if e != nil {
		return "", e
	}
	source := os.Getenv("CHASEHQ_ROM_SOURCE")
	if source == "" {
		source = local.ROMSource
	}
	if source == "" {
		return "", errors.New("missing staged ROMs and shared AdditionalFiles ROM source")
	}
	return resolveROMRoot(source, cfg)
}
func checkRuntimeCensus(dir string, hits int64) error {
	b, e := os.ReadFile(filepath.Join(dir, "rom-execution-census.json"))
	if e != nil {
		return e
	}
	var c struct {
		Schema       string `json:"schema"`
		Replacements int    `json:"nativeInstructionReplacements"`
		A            struct {
			Hits   int64  `json:"romInstructionHits"`
			Cycles uint64 `json:"emulatedCycles"`
		} `json:"cpuA"`
		B struct {
			Hits   int64  `json:"romInstructionHits"`
			Cycles uint64 `json:"emulatedCycles"`
		} `json:"cpuB"`
	}
	if e = json.Unmarshal(b, &c); e != nil {
		return e
	}
	if c.Schema != "chq-rom-execution-census-v1" || c.Replacements != 0 || c.A.Hits < hits || c.A.Hits <= 0 || c.B.Hits <= 0 || c.A.Cycles == 0 || c.B.Cycles == 0 {
		return errors.New("ROM census missing actual CPU A/B execution or original-ROM identity")
	}
	b, e = os.ReadFile(filepath.Join(dir, "native-execution-bridge.json"))
	if e != nil {
		return e
	}
	var bridge struct {
		Schema  string `json:"schema"`
		Mode    string `json:"activeMode"`
		Replace bool   `json:"replaceEnabled"`
		Entries []struct {
			CPU  string `json:"cpu"`
			PC   string `json:"pc"`
			Hits int64  `json:"hits"`
		} `json:"observedEntries"`
	}
	if e = json.Unmarshal(b, &bridge); e != nil {
		return e
	}
	if bridge.Schema != "chq-native-execution-bridge-v1" || bridge.Replace || bridge.Mode != "observe" {
		return errors.New("bridge is not read-only ROM observation")
	}
	for _, h := range bridge.Entries {
		if h.CPU == "A" && strings.EqualFold(h.PC, "0x00820C") && h.Hits == hits {
			return nil
		}
	}
	return errors.New("runtime instruction-hook hits do not match observed steering writer")
}
func validateNativeShadow(sh nativeShadow, prior error) error {
	if prior != nil {
		return prior
	}
	if sh.Schema != "chq-native-steering-shadow-v1" || !sh.Enabled || !sh.ReadOnly || sh.Substitution || sh.Compared < 1 || sh.Matches != sh.Compared || sh.Mismatches != 0 || sh.Pending || sh.Incomplete != 0 {
		return fmt.Errorf("shadow incomplete or mismatched: entries=%d compared=%d matches=%d mismatches=%d incomplete=%d pending=%v", sh.Entries, sh.Compared, sh.Matches, sh.Mismatches, sh.Incomplete, sh.Pending)
	}
	return nil
}
func runNativeNegativeTests(g nativeGhidra, rom []byte, sh nativeShadow, contract map[string]any) map[string]bool {
	badROM := append([]byte(nil), rom...)
	badROM[0] ^= 0x01
	badShadow := sh
	badShadow.Mismatches++
	badShadow.Matches--
	badContract := map[string]any{}
	for k, v := range contract {
		badContract[k] = v
	}
	badContract["candidateInstructionReviewed"] = false
	return map[string]bool{
		"alteredROMRejected":                    !nativePromotionEligibility(contract, sh, g.RawRomSHA, badROM),
		"inducedShadowMismatchRejected":         !nativePromotionEligibility(contract, badShadow, g.RawRomSHA, rom),
		"unreviewedFunctionCannotAuthorize":     !nativePromotionEligibility(badContract, sh, g.RawRomSHA, rom),
		"realIncompleteContractCannotAuthorize": !nativePromotionEligibility(contract, sh, g.RawRomSHA, rom),
	}
}
func nativePromotionEligibility(c map[string]any, sh nativeShadow, romSHA string, rom []byte) bool {
	if !strings.EqualFold(hexDigest(rom), romSHA) || validateNativeShadow(sh, nil) != nil {
		return false
	}
	for _, name := range []string{"candidateInstructionReviewed", "registerEffectsVerified", "cpuFlagsVerified", "orderedBusEffectsVerified", "cycleEquivalenceVerified", "controlFlowVerified", "rollbackVerified", "independentLiveStateParityVerified", "generatedCandidateLoadedAndExecutedVerified"} {
		yes, ok := c[name].(bool)
		if !ok || !yes {
			return false
		}
	}
	return true
}
func runNativeNegativeTestsAfterProof(c map[string]any, sh nativeShadow, sha string, rom []byte) bool {
	if !nativePromotionEligibility(c, sh, sha, rom) {
		return false
	}
	bad := map[string]any{}
	for k, v := range c {
		bad[k] = v
	}
	bad["candidateInstructionReviewed"] = false
	if nativePromotionEligibility(bad, sh, sha, rom) {
		return false
	}
	bad["candidateInstructionReviewed"] = true
	bad["generatedCandidateLoadedAndExecutedVerified"] = false
	if nativePromotionEligibility(bad, sh, sha, rom) {
		return false
	}
	damaged := append([]byte(nil), rom...)
	damaged[0] ^= 1
	if nativePromotionEligibility(c, sh, sha, damaged) {
		return false
	}
	wrong := sh
	wrong.Mismatches++
	wrong.Matches--
	return !nativePromotionEligibility(c, wrong, sha, rom)
}

func nativeSteeringCoverage(g nativeGhidra, hits map[string]int64) map[string]any {
	var ins []string
	for _, site := range g.Sites {
		if strings.EqualFold(site.PC, "0x00820C") {
			for _, i := range site.Instructions {
				pc, err := strconv.ParseUint(strings.TrimSpace(i.PC), 0, 24)
				if err == nil && pc&1 == 0 {
					ins = append(ins, fmt.Sprintf("0x%06X", pc))
				}
			}
			break
		}
	}
	unique := map[string]bool{}
	observed := 0
	executions := int64(0)
	for _, pc := range ins {
		if unique[pc] {
			continue
		}
		unique[pc] = true
		if n := hits["A:"+pc]; n > 0 {
			observed++
			executions += n
		}
	}
	return map[string]any{"schema": "chq-native-runtime-coverage-ledger-v1", "cpu": "A", "candidate": "0x00820C", "rawRomSha256": g.RawRomSHA, "localWindowDistinctStaticPCs": len(unique), "localWindowObservedPCs": observed, "localWindowRuntimeInstructionHits": executions, "fullFunctionCoveragePercent": nil, "globalCoveragePercent": nil, "coverageConfidence": "observed checkpoint/240-frame window only; local Ghidra instructions not a proven complete function CFG"}
}
func writeNativeEvidenceManifest(dir string) error {
	names := []string{}
	e := filepath.WalkDir(dir, func(p string, d os.DirEntry, e error) error {
		if e != nil {
			return e
		}
		if d.IsDir() {
			return nil
		}
		rel, e := filepath.Rel(dir, p)
		if e != nil {
			return e
		}
		if filepath.ToSlash(rel) != "evidence-sha256.json" {
			names = append(names, filepath.ToSlash(rel))
		}
		return nil
	})
	if e != nil {
		return e
	}
	sort.Strings(names)
	entries := make([]map[string]any, 0, len(names))
	for _, name := range names {
		p := filepath.Join(dir, filepath.FromSlash(name))
		b, e := os.ReadFile(p)
		if e != nil {
			return e
		}
		entries = append(entries, map[string]any{"path": name, "size": len(b), "sha256": hexDigest(b)})
	}
	b, e := json.MarshalIndent(map[string]any{"schema": "chq-native-accept-evidence-sha256-v1", "files": entries}, "", "  ")
	if e != nil {
		return e
	}
	return os.WriteFile(filepath.Join(dir, "evidence-sha256.json"), append(b, '\n'), 0644)
}
func bundleNativeEvidence(dir string) error {
	return bundleNativeEvidenceAtomic(dir)
}
