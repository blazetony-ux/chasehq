package main

// RC6.3.31 multi-source function-blocker dossier. Writers are observed
// committed Bus::write calls from the CPU-A/B emulator during bounded windows;
// they do not prove physical RAM aliasing, missing pre-checkpoint writers,
// exhaustive control flow, or function-level native equivalence.
import (
	"bufio"
	"crypto/sha256"
	"encoding/hex"
	"encoding/json"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"strconv"
	"strings"
)

type rc6331Writer struct {
	Ordinal, Frame, Cycles            uint64
	CPU                               string
	PC, Address, Width, Before, After uint32
}
type rc6331Case struct {
	Case                     string   `json:"case"`
	ObservedOriginalBTST     int      `json:"observedOriginalBtst"`
	ObservedCPUAWrites       int      `json:"observedCpuAWrites"`
	ObservedCPUBWrites       int      `json:"observedCpuBWrites"`
	ChangedBit7Writes        int      `json:"changedBit7Writes"`
	LatestCandidateWriterPCs []string `json:"latestCandidateWriterPcs"`
	WriterSourceState        string   `json:"writerSourceState"`
}
type rc6331Dossier struct {
	Schema                         string            `json:"schema"`
	CPU                            string            `json:"cpu"`
	ROMSHA256                      string            `json:"romSha256"`
	SourceMode                     string            `json:"sourceMode"`
	EvidenceClassifications        map[string]string `json:"evidenceClassifications"`
	Cases                          []rc6331Case      `json:"cases"`
	BranchSourceAddress            string            `json:"branchSourceAddress"`
	BranchPC                       string            `json:"branchPc"`
	ObservedFunctionEdges          int               `json:"observedFunctionEdges"`
	VerifiedStaticFunctionEntry    bool              `json:"verifiedStaticFunctionEntry"`
	VerifiedCompleteCFG            bool              `json:"verifiedCompleteCfg"`
	VerifiedAllFunctionExits       bool              `json:"verifiedAllFunctionExits"`
	Observed0518Fallthrough        bool              `json:"observed0518Fallthrough"`
	WriterProvenanceComplete       bool              `json:"writerProvenanceComplete"`
	WholeFunctionNativeEquivalence bool              `json:"wholeFunctionNativeEquivalence"`
	NativePromotionAllowed         bool              `json:"nativePromotionAllowed"`
	Blockers                       []string          `json:"blockers"`
	NextExperiments                []string          `json:"nextExperiments"`
}

func rc6331ReadWriters(base string, transitions uint64) ([]rc6331Writer, error) {
	raw, err := os.ReadFile(filepath.Join(base, "cpu-b-original-100802-writer-cap.json"))
	if err != nil {
		return nil, err
	}
	var cap struct {
		Schema, Source          string
		Hits, Recorded, Dropped int
		CPUTransitions          uint64 `json:"cpuBTransitions"`
	}
	if len(raw) > 4096 || json.Unmarshal(raw, &cap) != nil ||
		cap.Schema != "chq-rc6331-cpu-b-100802-writer-window-v1" ||
		cap.Source != "CPU_A_AND_B_BUS_WRITE_POSTCOMMIT_NO_FORCED_PATCH" ||
		cap.Hits < 0 || cap.Hits > 4096 || cap.Recorded != cap.Hits || cap.Dropped != 0 || cap.CPUTransitions != transitions {
		return nil, errors.New("untrusted or truncated two-CPU writer capture cap")
	}
	f, err := os.Open(filepath.Join(base, "cpu-b-original-100802-writer-events.tsv"))
	if err != nil {
		return nil, err
	}
	defer f.Close()
	s := bufio.NewScanner(f)
	s.Buffer(make([]byte, 256), 4096)
	if !s.Scan() || s.Text() != "ordinal\tframe\tcyclesB\tcpu\tpc\taddress\twidth\tbefore\tafter" {
		return nil, errors.New("invalid writer header")
	}
	rows := make([]rc6331Writer, 0, cap.Recorded)
	for s.Scan() {
		a := strings.Split(s.Text(), "\t")
		if len(a) != 9 || len(rows) >= 4096 {
			return nil, fmt.Errorf("invalid writer row %d", len(rows)+1)
		}
		dec := func(i int) (uint64, error) { return strconv.ParseUint(a[i], 10, 64) }
		hx := func(i, bits int) (uint64, error) { return strconv.ParseUint(a[i], 16, bits) }
		ord, e0 := dec(0)
		fr, e1 := dec(1)
		cy, e2 := dec(2)
		pc, e3 := hx(4, 32)
		addr, e4 := hx(5, 32)
		width, e5 := dec(6)
		before, e6 := hx(7, 8)
		after, e7 := hx(8, 8)
		if e0 != nil || e1 != nil || e2 != nil || e3 != nil || e4 != nil || e5 != nil || e6 != nil || e7 != nil ||
			(a[3] != "A" && a[3] != "B") || ord > transitions || fr > 10000000 || pc&1 != 0 || pc >= 0x200000 ||
			(width != 1 && width != 2 && width != 4) || addr > 0x100802 || addr+width <= 0x100802 ||
			(len(rows) > 0 && (fr < rows[len(rows)-1].Frame || ord < rows[len(rows)-1].Ordinal)) {
			return nil, fmt.Errorf("untrusted writer row %d", len(rows)+1)
		}
		rows = append(rows, rc6331Writer{ord, fr, cy, a[3], uint32(pc), uint32(addr), uint32(width), uint32(before), uint32(after)})
	}
	if s.Err() != nil {
		return nil, s.Err()
	}
	if len(rows) != cap.Recorded {
		return nil, errors.New("writer record count mismatch")
	}
	return rows, nil
}

func rc6331FunctionAndWriterDossier(dest, romHash string) (rc6331Dossier, error) {
	out := rc6331Dossier{
		Schema: "chq-rc6331-function-boundary-and-writer-dossier-v1", CPU: "B", ROMSHA256: romHash,
		SourceMode: "ORIGINAL_ROM_BOUNDED_GHIDRA_STATIC_AND_CPU_A_B_EXECUTION_WINDOWS",
		BranchPC:   "0x00000516", BranchSourceAddress: "0x00100802",
		Cases: []rc6331Case{},
		EvidenceClassifications: map[string]string{
			"natural": "original ROM, natural health case", "bypass": "original ROM with controlled CPU-A health-entry state",
			"nonterminal": "original ROM with controlled CPU-A health-entry state", "terminal": "original ROM with controlled CPU-A health-entry state",
			"overflow": "original ROM with controlled CPU-A health-entry state",
		},
		Blockers: []string{
			"A checkpoint can already contain bit7=1; capture windows do not establish pre-checkpoint writers",
			"Same numeric RAM address on CPU A and B is only a writer candidate until hardware/shared-RAM aliasing is verified",
			"Ghidra nearby sites and observed CPU-B edges are not a complete statically classified function CFG or all indirect callers",
			"0x0518 path has no natural or controlled original-ROM coverage in the acceptance matrix",
			"No enclosing function-wide original/generated C++/forced rollback equivalence, cycles, interrupts or scheduler parity",
		},
		NextExperiments: []string{
			"Trace both CPU memory spaces for address 0x100802 during a longer original-ROM survey before the first BTST",
			"Confirm writer instruction and shared RAM alias mapping with ROM/P-code and bus evidence",
			"Use a separately tagged controlled input/byte experiment to exercise zero-bit 0x0518; never count it as natural gameplay",
			"Expand Ghidra function entry/exits, indirect control flow and unsupported instructions before generating or promoting a full function",
		},
	}
	if !rc6315VerifyHash(romHash) {
		return out, errors.New("invalid CPU-B ROM identity")
	}
	old, e := rc6330BTSTSourceAudit(dest, romHash)
	if e != nil {
		return out, fmt.Errorf("original BTST and branch prerequisite: %w", e)
	}
	if old.NativePromotionAllowed || old.Observed0518Fallthrough {
		return out, errors.New("unsafe source-byte predecessor maturity")
	}
	frontier, e := rc6317ObservedFrontier(dest, romHash)
	if e != nil {
		return out, fmt.Errorf("original-ROM function frontier prerequisite: %w", e)
	}
	if frontier.NativePromotionAllowed || frontier.CompleteFunctionVerified || frontier.CoverageIsExhaustive {
		return out, errors.New("unexpected promoted frontier")
	}
	out.ObservedFunctionEdges = frontier.ObservedEdges
	for _, name := range rc6317Cases {
		base := filepath.Join(dest, "branch-matrix", name, "live-health", "original")
		// Count comes from the independently read original-ROM ordered hook cap.
		_, transition, err := rc6322ReadHooks(dest, name)
		if err != nil {
			return out, fmt.Errorf("%s hooks: %w", name, err)
		}
		sources, err := rc6330ReadSourceEvidence(base, transition)
		if err != nil {
			return out, fmt.Errorf("%s BTST source: %w", name, err)
		}
		writers, err := rc6331ReadWriters(base, transition)
		if err != nil {
			return out, fmt.Errorf("%s writers: %w", name, err)
		}
		c := rc6331Case{Case: name, ObservedOriginalBTST: len(sources), LatestCandidateWriterPCs: []string{}, WriterSourceState: "NO_WRITER_OBSERVED_IN_BOUNDED_WINDOW"}
		pcs := map[string]bool{}
		for _, w := range writers {
			if w.CPU == "A" {
				c.ObservedCPUAWrites++
			} else {
				c.ObservedCPUBWrites++
			}
			if (w.Before^w.After)&0x80 != 0 {
				c.ChangedBit7Writes++
			}
			pc := fmt.Sprintf("%s:%08X", w.CPU, w.PC)
			if !pcs[pc] {
				pcs[pc] = true
				c.LatestCandidateWriterPCs = append(c.LatestCandidateWriterPCs, pc)
			}
		}
		if len(writers) > 0 {
			c.WriterSourceState = "OBSERVED_CANDIDATE_WRITES_NOT_PROVEN_FULL_PRODUCER_HISTORY"
		}
		out.Cases = append(out.Cases, c)
	}
	if len(out.Cases) != 5 {
		return out, errors.New("incomplete five-case function dossier")
	}
	// Completeness and promotion fields remain false by construction until
	// an independent static CFG + full function differential can prove them.
	return out, nil
}

// Generic ordered-stream first-divergence witness. This is a minimal *trace
// witness*, NOT an independently replayed input or scenario minimizer.
type rc6331MismatchWitness struct {
	Schema                 string   `json:"schema"`
	Candidate              string   `json:"candidate"`
	OriginalPath           string   `json:"originalPath"`
	NativePath             string   `json:"nativePath"`
	OriginalSHA256         string   `json:"originalSha256"`
	NativeSHA256           string   `json:"nativeSha256"`
	FirstDifferentLine     int      `json:"firstDifferentLine"`
	OriginalLine           string   `json:"originalLine"`
	NativeLine             string   `json:"nativeLine"`
	ContextBefore          []string `json:"contextBefore"`
	Equal                  bool     `json:"equal"`
	WindowMinimized        bool     `json:"windowMinimized"`
	InputScenarioMinimized bool     `json:"inputScenarioMinimized"`
	NativePromotionAllowed bool     `json:"nativePromotionAllowed"`
	Reproducer             string   `json:"reproducer"`
}

func rc6331CompareStreams(candidate, aPath, bPath string) (rc6331MismatchWitness, error) {
	out := rc6331MismatchWitness{Schema: "chq-rc6331-first-divergence-trace-witness-v1", Candidate: candidate,
		OriginalPath: aPath, NativePath: bPath, ContextBefore: []string{}, Reproducer: "replay exact original/native/forced checkpoint and compare these SHA-pinned streams"}
	if candidate == "" || aPath == "" || bPath == "" {
		return out, errors.New("candidate and two trace paths required")
	}
	a, e := os.ReadFile(aPath)
	if e != nil {
		return out, e
	}
	b, e := os.ReadFile(bPath)
	if e != nil {
		return out, e
	}
	if len(a) > 32<<20 || len(b) > 32<<20 {
		return out, errors.New("trace exceeds 32 MiB limit")
	}
	ah, bh := sha256.Sum256(a), sha256.Sum256(b)
	out.OriginalSHA256 = hex.EncodeToString(ah[:])
	out.NativeSHA256 = hex.EncodeToString(bh[:])
	if string(a) == string(b) {
		out.Equal = true
		return out, nil
	}
	lines := func(raw []byte) []string { v := strings.Split(strings.TrimSuffix(string(raw), "\n"), "\n"); return v }
	aa, bb := lines(a), lines(b)
	i := 0
	for i < len(aa) && i < len(bb) && aa[i] == bb[i] {
		i++
	}
	out.FirstDifferentLine = i + 1
	if i < len(aa) {
		out.OriginalLine = aa[i]
	} else {
		out.OriginalLine = "<EOF>"
	}
	if i < len(bb) {
		out.NativeLine = bb[i]
	} else {
		out.NativeLine = "<EOF>"
	}
	start := i - 4
	if start < 0 {
		start = 0
	}
	out.ContextBefore = append(out.ContextBefore, aa[start:i]...)
	out.WindowMinimized = true
	return out, nil
}

// CLI: chqdev native diagnose --candidate ID --original FILE --native FILE
// --out FILE; the emitted witness is never a whole-function promotion gate.
func rc6331DiagnoseCommand(root string, cfg Config, args []string) *Report {
	rep := newReport("native diagnose", cfg.Release)
	vals := map[string]string{}
	for i := 0; i < len(args); i++ {
		if (args[i] == "--candidate" || args[i] == "--original" || args[i] == "--native" || args[i] == "--out") && i+1 < len(args) {
			vals[args[i]] = args[i+1]
			i++
		} else {
			rep.fail("arguments", fmt.Sprintf("unknown or incomplete option %s", args[i]))
			return rep
		}
	}
	if vals["--out"] == "" {
		rep.fail("arguments", "--out JSON file required")
		return rep
	}
	v, err := rc6331CompareStreams(vals["--candidate"], vals["--original"], vals["--native"])
	if err != nil {
		rep.fail("first-divergence", err.Error())
		return rep
	}
	if err := rc63WriteJSON(vals["--out"], v); err != nil {
		rep.fail("witness-output", err.Error())
		return rep
	}
	rep.Data["witness"] = v
	if v.Equal {
		rep.pass("compare", "SHA-pinned streams identical")
	} else {
		rep.warn("first-divergence", fmt.Sprintf("line %d, context window only; no verified input minimization", v.FirstDifferentLine))
	}
	return rep
}
