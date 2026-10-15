package main

// RC6.3.28 original-ROM CPU-B BNE.B branch-outcome evidence; RC6.3.29
// correctly permits equal cyclesB snapshots at consecutive instruction hooks.
// A strictly smaller next cycle remains invalid. The transition ordinal and
// frame ordering are checked independently. The instrumented
// next CPU-B instruction is observed (not inferred from aggregate edge counts).
// If an alternative 0x0518 path becomes live, this bounded contract fails
// closed until a separate Ghidra/path/ABI analysis is completed.
import (
	"bufio"
	"encoding/json"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"strconv"
	"strings"
)

type rc6328Event struct {
	Ordinal, Frame, Cycles uint64
	PC, Opcode, SR, NextPC uint32
	NextFrame, NextCycles  uint64
}
type rc6328Case struct {
	Case                 string `json:"case"`
	BranchVisits         int    `json:"branchVisits"`
	BNENotZeroTaken      int    `json:"bneNotZeroTaken"`
	BNEZeroFallthrough   int    `json:"bneZeroFallthrough"`
	ZFlagSetAtBranch     int    `json:"zFlagSetAtBranch"`
	OriginalROMEdgeCount int64  `json:"originalRomEdgeCount"`
}
type rc6328Report struct {
	Schema                                 string       `json:"schema"`
	CPU                                    string       `json:"cpu"`
	ROMSHA256                              string       `json:"romSha256"`
	EvidenceSource                         string       `json:"evidenceSource"`
	Cases                                  []rc6328Case `json:"cases"`
	RecordedSRMatchesSelectedOriginalHooks bool         `json:"recordedSrMatchesSelectedOriginalHooks"`
	ObservedNextPCMatchesBranchCondition   bool         `json:"observedNextPcMatchesBranchCondition"`
	OrderedVisitsMatchOriginalROMEdges     bool         `json:"orderedVisitsMatchOriginalRomEdges"`
	Observed0518Fallthrough                bool         `json:"observed0518Fallthrough"`
	EntirePathCoverageVerified             bool         `json:"entirePathCoverageVerified"`
	EnclosingFunctionCFGVerified           bool         `json:"enclosingFunctionCfgVerified"`
	NativePromotionAllowed                 bool         `json:"nativePromotionAllowed"`
	Blockers                               []string     `json:"blockers"`
}

func rc6328ReadDecisionEvidence(base string, transitions uint64) ([]rc6328Event, error) {
	raw, e := os.ReadFile(filepath.Join(base, "cpu-b-original-0516-bne-cap.json"))
	if e != nil {
		return nil, e
	}
	var cap struct {
		Schema, Source string
		Hits, Recorded int
		Dropped        uint64
		Pending        bool
		CPUTransitions uint64 `json:"cpuBTransitions"`
	}
	if len(raw) > 4096 || json.Unmarshal(raw, &cap) != nil || cap.Schema != "chq-rc6328-cpu-b-0516-bne-cap-v1" || cap.Source != "ORIGINAL_ROM_CPU_B_INSTRUCTION_HOOK_NEXT_PC_AND_SR" || cap.Hits < 1 || cap.Hits > 4096 || cap.Recorded != cap.Hits || cap.Dropped != 0 || cap.Pending || cap.CPUTransitions != transitions {
		return nil, errors.New("original-ROM BNE decision cap missing/dropped/pending/untrusted")
	}
	f, e := os.Open(filepath.Join(base, "cpu-b-original-0516-bne-decisions.tsv"))
	if e != nil {
		return nil, e
	}
	defer f.Close()
	sc := bufio.NewScanner(f)
	sc.Buffer(make([]byte, 256), 4096)
	if !sc.Scan() || sc.Text() != "ordinal\tframe\tcyclesB\tpc\topcode\tsr\tnextPc\tnextFrame\tnextCyclesB" {
		return nil, errors.New("BNE decision header mismatch")
	}
	out := make([]rc6328Event, 0, cap.Recorded)
	for sc.Scan() {
		cols := strings.Split(sc.Text(), "\t")
		if len(cols) != 9 || len(out) >= 4096 {
			return nil, fmt.Errorf("invalid BNE row %d", len(out)+1)
		}
		dec := func(k int) (uint64, error) { return strconv.ParseUint(cols[k], 10, 64) }
		hx := func(k int, b int) (uint64, error) { return strconv.ParseUint(cols[k], 16, b) }
		ord, e0 := dec(0)
		frame, e1 := dec(1)
		cycle, e2 := dec(2)
		pc, e3 := hx(3, 32)
		op, e4 := hx(4, 16)
		sr, e5 := hx(5, 16)
		next, e6 := hx(6, 32)
		nf, e7 := dec(7)
		nc, e8 := dec(8)
		if e0 != nil || e1 != nil || e2 != nil || e3 != nil || e4 != nil || e5 != nil || e6 != nil || e7 != nil || e8 != nil || pc != 0x516 || op != 0x6614 || ord > transitions || frame > 10000000 || nf < frame || nf > frame+1 || nc < cycle || next >= 0x20000 || next&1 != 0 || (len(out) > 0 && (ord <= out[len(out)-1].Ordinal || cycle <= out[len(out)-1].Cycles)) {
			return nil, fmt.Errorf("untrusted BNE decision row %d", len(out)+1)
		}
		out = append(out, rc6328Event{ord, frame, cycle, uint32(pc), uint32(op), uint32(sr), uint32(next), nf, nc})
	}
	if sc.Err() != nil {
		return nil, sc.Err()
	}
	if len(out) != cap.Recorded {
		return nil, errors.New("BNE decision record count mismatch")
	}
	return out, nil
}

func rc6328ReadEdgeCounts(base string) (map[uint32]int64, error) {
	f, e := os.Open(filepath.Join(base, "cpu-b-observed-edges.tsv"))
	if e != nil {
		return nil, e
	}
	defer f.Close()
	counts := map[uint32]int64{}
	sc := bufio.NewScanner(f)
	sc.Buffer(make([]byte, 256), 4096)
	rows := 0
	for sc.Scan() {
		rows++
		if rows > 20000 {
			return nil, errors.New("oversized authentic edge file")
		}
		c := strings.Split(sc.Text(), "\t")
		if len(c) != 4 {
			return nil, errors.New("invalid authentic CPU-B edge")
		}
		from, e0 := strconv.ParseUint(c[0], 16, 32)
		to, e1 := strconv.ParseUint(c[1], 16, 32)
		op, e2 := strconv.ParseUint(c[2], 16, 16)
		n, e3 := strconv.ParseInt(c[3], 10, 64)
		if e0 != nil || e1 != nil || e2 != nil || e3 != nil || n <= 0 {
			return nil, errors.New("invalid authentic CPU-B edge numeric fields")
		}
		if from != 0x516 {
			continue
		}
		if op != 0x6614 {
			return nil, errors.New("CPU-B BNE opcode mismatch in original edge census")
		}
		if counts[uint32(to)] != 0 {
			return nil, errors.New("duplicate 0x0516 successor edge")
		}
		counts[uint32(to)] = n
	}
	if sc.Err() != nil {
		return nil, sc.Err()
	}
	return counts, nil
}

func rc6328BranchFrontierAudit(dest, romHash string) (rc6328Report, error) {
	r := rc6328Report{Schema: "chq-rc6328-original-rom-cpu-b-0516-bne-decision-v1", CPU: "B", ROMSHA256: romHash, EvidenceSource: "ORIGINAL_ROM_CPU_B_0516_INSTRUCTION_ENTRY_SR_AND_IMMEDIATE_NEXT_HOOK_CROSSCHECK_WITH_GHIDRA_AND_EDGE_CENSUS", Cases: []rc6328Case{}, Blockers: []string{
		"All five checkpoint windows take the 0x0516->0x052C BNE path; 0x0518 fallthrough still lacks authentic observed execution",
		"Per-visit next-hook evidence does not establish every caller, indirect JSR target, exception interleaving or complete function CFG",
		"No synthetic branch forcing is labelled authentic and no new instruction is natively substituted",
		"Entire CPU-B function ABI, full scheduling equivalence and autonomous native promotion remain blocked",
	}}
	if !rc6315VerifyHash(romHash) {
		return r, errors.New("missing authentic CPU-B ROM SHA")
	}
	prior, e := rc6319CallReturnAudit(dest, romHash)
	if e != nil {
		return r, fmt.Errorf("Ghidra/real-ROM call-return prerequisite: %w", e)
	}
	if prior.UnobservedFallthrough.OriginalRuntimeObserved || prior.NativePromotionAllowed {
		return r, errors.New("unsafe predecessor maturity")
	}
	for _, name := range rc6317Cases {
		hooks, trans, e := rc6322ReadHooks(dest, name)
		if e != nil {
			return r, fmt.Errorf("%s hooks: %w", name, e)
		}
		base := filepath.Join(dest, "branch-matrix", name, "live-health", "original")
		events, e := rc6328ReadDecisionEvidence(base, trans)
		if e != nil {
			return r, fmt.Errorf("%s decisions: %w", name, e)
		}
		edges, e := rc6328ReadEdgeCounts(base)
		if e != nil {
			return r, fmt.Errorf("%s edges: %w", name, e)
		}
		selected := []rc6322Hook{}
		for _, h := range hooks {
			if h.PC == 0x516 {
				selected = append(selected, h)
			}
			if h.PC == 0x518 {
				return r, fmt.Errorf("%s newly observed 0x0518 requires separately reviewed path contract", name)
			}
		}
		if len(selected) == 0 || len(selected) != len(events) {
			return r, fmt.Errorf("%s selected hook/next-PC sample count differs", name)
		}
		cr := rc6328Case{Case: name}
		for i, x := range events {
			h := selected[i]
			if x.Ordinal != h.Ordinal || x.Frame != h.Frame || x.Cycles != h.Cycles || x.SR != uint32(h.SR) || h.Opcode != 0x6614 {
				return r, fmt.Errorf("%s original-ROM selected SR/ordinal mismatch row %d", name, i+1)
			}
			expect := uint32(0x52c)
			if x.SR&0x4 != 0 {
				expect = 0x518
				cr.ZFlagSetAtBranch++
			}
			if x.NextPC != expect {
				return r, fmt.Errorf("%s BNE Z-flag/actual next-PC mismatch row %d got %04X want %04X", name, i+1, x.NextPC, expect)
			}
			if x.NextPC == 0x518 {
				cr.BNEZeroFallthrough++
			} else {
				cr.BNENotZeroTaken++
			}
			cr.BranchVisits++
		}
		if cr.BNEZeroFallthrough != 0 || cr.ZFlagSetAtBranch != 0 {
			return r, fmt.Errorf("%s previously unobserved 0x0518 is live; expand Ghidra/path verification before acceptance", name)
		}
		if len(edges) != 1 || edges[0x52c] != int64(cr.BranchVisits) {
			return r, fmt.Errorf("%s edge aggregate does not match ordered original-ROM branch decisions", name)
		}
		cr.OriginalROMEdgeCount = edges[0x52c]
		r.Cases = append(r.Cases, cr)
	}
	if len(r.Cases) != 5 {
		return r, errors.New("incomplete 5-case CPU-B BNE frontier")
	}
	r.RecordedSRMatchesSelectedOriginalHooks = true
	r.ObservedNextPCMatchesBranchCondition = true
	r.OrderedVisitsMatchOriginalROMEdges = true
	return r, nil
}
