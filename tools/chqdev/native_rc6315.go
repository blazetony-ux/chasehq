package main

// RC6.3.15: Evidence-derived function dossier and CPU-B instruction ownership.
// These are verification aids, not general function generation or a promotion gate.
import (
	"bytes"
	"crypto/sha256"
	"encoding/hex"
	"fmt"
	"os"
	"path/filepath"
	"sort"
	"strconv"
	"strings"
)

type rc6315Dossier struct {
	Schema                         string                     `json:"schema"`
	CPU                            string                     `json:"cpu"`
	RawRomSHA256                   string                     `json:"rawRomSha256"`
	Candidates                     []rc6315FunctionAssessment `json:"candidates"`
	EntireFunctionVerified         bool                       `json:"entireFunctionVerified"`
	FunctionNativePromotionAllowed bool                       `json:"functionNativePromotionAllowed"`
}
type rc6315FunctionAssessment struct {
	Candidate                      string   `json:"candidate"`
	Entry                          string   `json:"entry"`
	EndExclusive                   string   `json:"endExclusive"`
	GhidraFunctionEntry            string   `json:"ghidraFunctionEntry,omitempty"`
	InstructionCount               int      `json:"instructionCount"`
	InstructionPCs                 []string `json:"instructionPCs"`
	Contiguous                     bool     `json:"contiguous"`
	EndsInRTS                      bool     `json:"endsInRTS"`
	ObservedLiveFunctionCalls      int      `json:"observedLiveFunctionCalls"`
	Status                         string   `json:"status"`
	Unresolved                     []string `json:"unresolved"`
	FunctionNativePromotionAllowed bool     `json:"functionNativePromotionAllowed"`
}

func rc6315AssessFunctions(g rc63Findings, cs []rc63Candidate) (rc6315Dossier, error) {
	out := rc6315Dossier{Schema: "chq-rc6315-function-dossier-v1", CPU: g.CPU, RawRomSHA256: g.ROM, Candidates: []rc6315FunctionAssessment{}}
	if (g.CPU != "A" && g.CPU != "B") || len(g.ROM) != 64 || len(cs) == 0 {
		return out, fmt.Errorf("missing authenticated Ghidra functions")
	}
	if _, err := hex.DecodeString(g.ROM); err != nil {
		return out, fmt.Errorf("invalid Ghidra ROM hash: %w", err)
	}
	for _, c := range cs {
		a := rc6315FunctionAssessment{Candidate: c.ID, Entry: c.Entry, EndExclusive: c.End, GhidraFunctionEntry: c.FunctionCandidate, InstructionCount: len(c.Instructions), Contiguous: true, Status: "FUNCTION_BOUNDARY_UNRESOLVED", Unresolved: []string{
			"complete enclosing function CFG and every reachable exit not proven",
			"all authentic incoming callers and exception paths not proven",
			"full-function original/native/rollback differential not performed",
			"CPU-A/CPU-B shared bus, IRQ and scheduling composition not proven",
		}}
		if len(c.Instructions) == 0 {
			return out, fmt.Errorf("candidate %s has no instructions", c.ID)
		}
		var next uint64
		for i, ins := range c.Instructions {
			pc, err := rc63Address(ins.PC)
			if err != nil || ins.Length < 2 || ins.Length%2 != 0 {
				return out, fmt.Errorf("bad instruction in %s", c.ID)
			}
			if i > 0 && pc != next {
				a.Contiguous = false
			}
			next = pc + uint64(ins.Length)
			a.InstructionPCs = append(a.InstructionPCs, ins.PC)
		}
		a.EndsInRTS = strings.EqualFold(strings.TrimSpace(c.Instructions[len(c.Instructions)-1].Text), "rts")
		if !a.Contiguous {
			a.Unresolved = append(a.Unresolved, "instruction address gap")
		}
		if !a.EndsInRTS {
			a.Unresolved = append(a.Unresolved, "candidate is not terminated by an independently proven RTS")
		}
		if !c.FullFunction {
			a.Unresolved = append(a.Unresolved, "Ghidra site window is not the complete declared function range")
		}
		out.Candidates = append(out.Candidates, a)
	}
	sort.Slice(out.Candidates, func(i, j int) bool { return out.Candidates[i].Candidate < out.Candidates[j].Candidate })
	return out, nil
}

type rc6315BusOwnership struct {
	Schema                    string                `json:"schema"`
	Candidate                 string                `json:"candidate"`
	ROMSource                 string                `json:"romSource"`
	Modes                     []rc6315ModeOwnership `json:"modes"`
	OriginalNativeDataEqual   bool                  `json:"originalNativeDataEqual"`
	OriginalNativeIRQEqual    bool                  `json:"originalNativeIRQEqual"`
	OriginalFallbackDataEqual bool                  `json:"originalFallbackDataEqual"`
	OriginalFallbackIRQEqual  bool                  `json:"originalFallbackIRQEqual"`
	InstrFetchAccounting      string                `json:"instructionFetchAccounting"`
	UnattributedEffects       []string              `json:"unattributedEffects"`
	WholeFunctionVerified     bool                  `json:"wholeFunctionVerified"`
	CompositionVerified       bool                  `json:"compositionVerified"`
}
type rc6315ModeOwnership struct {
	Mode                 string `json:"mode"`
	TSTWDataReads        int    `json:"tstWDataReads"`
	BEQBDataAccesses     int    `json:"beqBDataAccesses"`
	IRQAcks              int    `json:"irqAcks"`
	IRQVectorAndStackOps int    `json:"irqVectorAndStackOps"`
}

// The original ROM's CPU-B TST.W owns a single mapped R16 read; BEQ.B has
// zero mapped data effects. IRQ effects are owned by ACK, not stale trace PC.
// No implicit attribution of instruction fetches or out-of-boundary effects.
func rc6315PairSideEffects(dest, romHash string) (rc6315BusOwnership, error) {
	out := rc6315BusOwnership{Schema: "chq-rc6315-side-effects-v1", Candidate: rc6314PairID, ROMSource: romHash, InstrFetchAccounting: "NOT_IN_MAPPED_DATA_STREAM; FETCH/EXCEPTION COVERAGE REMAINS UNRESOLVED", Modes: []rc6315ModeOwnership{}}
	if len(romHash) != 64 {
		return out, fmt.Errorf("unverified CPU-B ROM provenance")
	}
	var sourceData, sourceIRQ []byte
	for _, mode := range []string{"original", "generated-native", "forced-mismatch"} {
		base := filepath.Join(dest, "live-cpu-b-pair", mode)
		data, err := os.ReadFile(filepath.Join(base, "native-cpu-b-pair-bus.tsv"))
		if err != nil {
			return out, err
		}
		irq, err := os.ReadFile(filepath.Join(base, "native-cpu-b-pair-irq.tsv"))
		if err != nil {
			return out, err
		}
		n, err := rc6313PairBus(data)
		if err != nil {
			return out, fmt.Errorf("%s side-effect attribution: %w", mode, err)
		}
		a, ops, err := rc6311ParseIRQStream(irq)
		if err != nil {
			return out, fmt.Errorf("%s IRQ ownership: %w", mode, err)
		}
		if n < 1 || a < 1 {
			return out, fmt.Errorf("%s missing authentic TST.W/IRQ events", mode)
		}
		out.Modes = append(out.Modes, rc6315ModeOwnership{Mode: mode, TSTWDataReads: n, BEQBDataAccesses: 0, IRQAcks: a, IRQVectorAndStackOps: ops})
		if mode == "original" {
			sourceData, sourceIRQ = data, irq
		} else if mode == "generated-native" {
			out.OriginalNativeDataEqual = bytes.Equal(sourceData, data)
			out.OriginalNativeIRQEqual = bytes.Equal(sourceIRQ, irq)
		} else {
			out.OriginalFallbackDataEqual = bytes.Equal(sourceData, data)
			out.OriginalFallbackIRQEqual = bytes.Equal(sourceIRQ, irq)
		}
	}
	if !out.OriginalNativeDataEqual || !out.OriginalNativeIRQEqual || !out.OriginalFallbackDataEqual || !out.OriginalFallbackIRQEqual {
		return out, fmt.Errorf("CPU-B TST/BEQ per-instruction data or ACK-owned IRQ stream mismatch")
	}
	out.UnattributedEffects = []string{"opcode/prefetch ownership not independently classified in data-only capture", "all enclosing function paths and CPU-A/B scheduling remain unreviewed"}
	return out, nil
}

type rc6315Mismatch struct {
	Schema             string `json:"schema"`
	Candidate          string `json:"candidate"`
	Comparator         string `json:"comparator"`
	FirstDifferentLine int    `json:"firstDifferentLine"`
	OriginalRow        string `json:"originalRow"`
	NativeRow          string `json:"nativeRow"`
	OriginalSHA256     string `json:"originalSha256"`
	NativeSHA256       string `json:"nativeSha256"`
	Reproducer         string `json:"reproducer"`
	MinimizedTo        string `json:"minimizedTo"`
	PromotionAllowed   bool   `json:"promotionAllowed"`
}

// A first-divergence reduction is explicitly not a full deterministic input
// minimizer. Preserve both original traces and the checkpoint invocation.
func rc6315FirstDivergence(candidate, kind string, original, native []byte) (rc6315Mismatch, bool) {
	if bytes.Equal(original, native) {
		return rc6315Mismatch{}, false
	}
	oo := strings.Split(string(original), "\n")
	nn := strings.Split(string(native), "\n")
	row := 0
	for row < len(oo) && row < len(nn) && oo[row] == nn[row] {
		row++
	}
	at := func(s []string, i int) string {
		if i < len(s) {
			return s[i]
		}
		return "<EOF>"
	}
	ah := sha256.Sum256(original)
	bh := sha256.Sum256(native)
	return rc6315Mismatch{Schema: "chq-rc6315-first-divergence-v1", Candidate: candidate, Comparator: kind, FirstDifferentLine: row + 1, OriginalRow: at(oo, row), NativeRow: at(nn, row), OriginalSHA256: hex.EncodeToString(ah[:]), NativeSHA256: hex.EncodeToString(bh[:]), Reproducer: "checkpoint=stage1-target-immediate-pre-damage-6333.chqstate; replay original/native/forced with --cpu-b-live", MinimizedTo: "first differing evidence row only; input minimization NOT YET VERIFIED", PromotionAllowed: false}, true
}
func rc6315CapturePairFailure(dest string, a, b []byte, kind string) {
	m, notEqual := rc6315FirstDivergence(rc6314PairID, kind, a, b)
	if notEqual {
		_ = rc63WriteJSON(filepath.Join(dest, "cpu-b-pair-failure-"+strings.ToLower(kind)+".json"), m)
	}
}
func rc6315VerifyHash(h string) bool {
	if len(h) != 64 {
		return false
	}
	_, e := strconv.ParseUint(h[:16], 16, 64)
	return e == nil
}

// Actual CPU-B candidate discovery is an authenticated Ghidra ROM-site
// correlation. Five opcode sites and a pair do NOT define a function graph.
func rc6315CPUBFunctionDossier(p rc639Proof, pair rc6312PairProof, live *rc6313PairLiveProof) (rc6315Dossier, error) {
	out := rc6315Dossier{Schema: "chq-rc6315-function-dossier-v1", CPU: "B", RawRomSHA256: p.CPUBROM_SHA256, Candidates: []rc6315FunctionAssessment{}}
	if p.Status != "CPU_B_GHIDRA_GENERATED_ABI_PASS_NOT_LIVE" || !rc6315VerifyHash(p.CPUBROM_SHA256) || p.MatchedGhidraSites < 5 || pair.OriginalConsecutiveEdges < 1 || pair.PromotionAllowed {
		return out, fmt.Errorf("CPU-B Ghidra/ROM site provenance not verified")
	}
	for _, c := range []struct {
		id, entry, end string
		pcs            []string
		count          int
	}{
		{"B:000004DE-000004E0", "0x000004DE", "0x000004E0", []string{"0x000004DE"}, 1},
		{rc6314PairID, "0x000004D8", "0x000004E0", []string{"0x000004D8", "0x000004DE"}, 2},
	} {
		a := rc6315FunctionAssessment{Candidate: c.id, Entry: c.entry, EndExclusive: c.end, InstructionCount: c.count, InstructionPCs: c.pcs, Contiguous: true, EndsInRTS: false, Status: "ROM_GHIDRA_OPCODE_AND_EDGE_BOUNDARY_ONLY", Unresolved: []string{"enclosing CPU-B function entry/callgraph/exit and full CFG not proven", "unobserved interrupt and scheduling contexts beyond bounded replay", "full original/native enclosing-function differential NOT performed"}}
		if live != nil && c.count == 2 && live.Status == "LIVE_GHIDRA_CPU_B_TST_BEQ_PAIR_BOUNDED_PASS" && live.NativeCommits > 0 {
			a.ObservedLiveFunctionCalls = 0 // bounded instruction commits are not functions
			a.Status = "LIVE_VERIFIED_BOUNDED_TWO_INSTRUCTIONS_NOT_A_FUNCTION"
		}
		out.Candidates = append(out.Candidates, a)
	}
	return out, nil
}
