package main

// RC6.3.22: a selected, chronological projection of genuine CPU-B instruction
// hooks. A fully reconstructed callstack, stack bus trace or IRQ/RTE parity is
// NOT inferred from this limited evidence. No native promotion is permitted.
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

type rc6322Hook struct {
	Ordinal uint64 `json:"ordinal"`
	Frame   uint64 `json:"frame"`
	Cycles  uint64 `json:"cyclesB"`
	PC      uint32 `json:"pc"`
	Opcode  uint16 `json:"opcode"`
	SP      uint32 `json:"sp"`
	SR      uint16 `json:"sr"`
}
type rc6322Case struct {
	Case                        string `json:"case"`
	ObservedHooks               int    `json:"observedHooks"`
	CPUTransitions              uint64 `json:"cpuTransitions"`
	NestedSequences             int    `json:"nestedSequences"`
	OrderedPairs                int    `json:"orderedPairs"`
	IRQEntriesDuringNested      int    `json:"irqEntriesDuringNested"`
	RTEsDuringNested            int    `json:"rtesDuringNested"`
	IRQContextsProven           bool   `json:"irqContextsProven"`
	SampledStackPointersMatched bool   `json:"sampledStackPointersMatched"`
}
type rc6322Report struct {
	Schema                              string       `json:"schema"`
	CPU                                 string       `json:"cpu"`
	ROMSHA256                           string       `json:"romSha256"`
	EvidenceSource                      string       `json:"evidenceSource"`
	Cases                               []rc6322Case `json:"cases"`
	SelectedNestedTemporalOrderVerified bool         `json:"selectedNestedTemporalOrderVerified"`
	SelectedStackPointerPairsVerified   bool         `json:"selectedStackPointerPairsVerified"`
	AllInstructionEventsCaptured        bool         `json:"allInstructionEventsCaptured"`
	InterruptRTEStackParityVerified     bool         `json:"interruptRteStackParityVerified"`
	Unobserved0518Executed              bool         `json:"unobserved0518Executed"`
	CompleteFunctionVerified            bool         `json:"completeFunctionVerified"`
	NativePromotionAllowed              bool         `json:"nativePromotionAllowed"`
	Blockers                            []string     `json:"blockers"`
}

var rc6322NestedPattern = []uint32{0x08a6, 0x4f70, 0x4fb2, 0x5062, 0x4f74, 0x4fb2, 0x5062, 0x4f78, 0x4fb2, 0x5062, 0x4f7c, 0x08ac}
var rc6322ImportantOpcodes = map[uint32]uint16{0x08a6: 0x4eb9, 0x4f70: 0x6100, 0x4f74: 0x6100, 0x4f78: 0x6100, 0x4f7c: 0x4e75, 0x5062: 0x4e75, 0x08ac: 0x4e75, 0x4fb2: 0x41f9, 0x0516: 0x6614, 0x04d6: 0x4e73}

func rc6322ReadHooks(dest, caseName string) ([]rc6322Hook, uint64, error) {
	dir := filepath.Join(dest, "branch-matrix", caseName, "live-health", "original")
	f, err := os.Open(filepath.Join(dir, "cpu-b-ordered-hook-events.tsv"))
	if err != nil {
		return nil, 0, err
	}
	defer f.Close()
	var capState struct {
		Schema         string `json:"schema"`
		Source         string `json:"source"`
		Recorded       int    `json:"recorded"`
		Dropped        uint64 `json:"dropped"`
		CPUTransitions uint64 `json:"cpuBTransitions"`
	}
	capRaw, err := os.ReadFile(filepath.Join(dir, "cpu-b-ordered-hook-cap.json"))
	if err != nil {
		return nil, 0, err
	}
	if len(capRaw) > 4096 || json.Unmarshal(capRaw, &capState) != nil || capState.Schema != "chq-rc6322-ordered-hook-cap-v1" || capState.Source != "ORIGINAL_ROM_CPU_B_INSTRUCTION_HOOK" || capState.Dropped != 0 || capState.CPUTransitions >= 2000000 || capState.Recorded < 1 || capState.Recorded > 25000 {
		return nil, 0, errors.New("original CPU-B hook evidence truncated, saturated or unauthenticated")
	}
	sc := bufio.NewScanner(f)
	sc.Buffer(make([]byte, 1024), 4096)
	if !sc.Scan() || sc.Text() != "ordinal\tframe\tcyclesB\tpc\topcode\tsp\tsr" {
		return nil, 0, errors.New("CPU-B hook evidence header invalid")
	}
	out := make([]rc6322Hook, 0, capState.Recorded)
	var lastOrdinal, lastCycle uint64
	for sc.Scan() {
		if len(out) >= 25000 {
			return nil, 0, errors.New("excessive ordered hooks")
		}
		col := strings.Split(sc.Text(), "\t")
		if len(col) != 7 {
			return nil, 0, fmt.Errorf("malformed hook event %d", len(out)+1)
		}
		dec := func(s string) (uint64, error) { return strconv.ParseUint(s, 10, 64) }
		hex := func(s string, b int) (uint64, error) { return strconv.ParseUint(s, 16, b) }
		ordinal, e0 := dec(col[0])
		frame, e1 := dec(col[1])
		cycles, e2 := dec(col[2])
		pc, e3 := hex(col[3], 32)
		op, e4 := hex(col[4], 16)
		sp, e5 := hex(col[5], 32)
		sr, e6 := hex(col[6], 16)
		if e0 != nil || e1 != nil || e2 != nil || e3 != nil || e4 != nil || e5 != nil || e6 != nil {
			return nil, 0, fmt.Errorf("bad hook numeric field on row %d", len(out)+1)
		}
		// Ordinals count CPU-B transitions from zero. The first captured
		// original-ROM hook can legitimately have ordinal 0; subsequent
		// hooks must remain strictly increasing, with the existing cap.
		if ordinal > capState.CPUTransitions || (len(out) > 0 && (ordinal <= lastOrdinal || cycles < lastCycle)) || pc >= 0x20000 || pc%2 != 0 || frame > 10000000 {
			return nil, 0, fmt.Errorf("invalid ordered CPU-B event at row %d", len(out)+1)
		}
		if expected, ok := rc6322ImportantOpcodes[uint32(pc)]; ok && uint16(op) != expected {
			return nil, 0, fmt.Errorf("ROM opcode mismatch at CPU-B %X", pc)
		}
		lastOrdinal, lastCycle = ordinal, cycles
		out = append(out, rc6322Hook{ordinal, frame, cycles, uint32(pc), uint16(op), uint32(sp), uint16(sr)})
	}
	if err := sc.Err(); err != nil {
		return nil, 0, err
	}
	if len(out) != capState.Recorded {
		return nil, 0, fmt.Errorf("CPU-B event count inconsistent: %d vs %d", len(out), capState.Recorded)
	}
	return out, capState.CPUTransitions, nil
}
func rc6322OrderedAudit(dest, romHash string) (rc6322Report, error) {
	r := rc6322Report{Schema: "chq-rc6322-cpu-b-original-rom-selected-temporal-v1", CPU: "B", ROMSHA256: romHash, EvidenceSource: "REAL_ORIGINAL_ROM_CPU_B_INSTRUCTION_HOOK_ORDER_NOT_AGGREGATED_EDGES", Cases: []rc6322Case{}, Blockers: []string{
		"Selected PC hooks are a filtered projection, not exhaustive CPU-B instruction or bus events",
		"IRQ vector/RTE hooks cannot establish precise exception stack, ownership, scheduler and timing parity",
		"Indirect JSR target set, callee CFG, exit paths and 0x0518 fallthrough remain incomplete",
		"No full-function native/original/rollback differential parity; autonomous native promotion forbidden",
	}}
	if !rc6315VerifyHash(romHash) {
		return r, errors.New("original-ROM digest missing")
	}
	if _, err := rc6321NestedReturnAudit(dest, romHash); err != nil {
		return r, fmt.Errorf("nested Ghidra/edge prerequisite failed: %w", err)
	}
	for _, name := range rc6317Cases {
		hooks, total, err := rc6322ReadHooks(dest, name)
		if err != nil {
			return r, fmt.Errorf("%s: %w", name, err)
		}
		c := rc6322Case{Case: name, ObservedHooks: len(hooks), CPUTransitions: total, SampledStackPointersMatched: true}
		state := 0
		var callSP uint32
		var bsrSP uint32
		for _, h := range hooks {
			if h.PC == 0x0518 {
				return r, fmt.Errorf("%s: previously unobserved 0x0518 needs separate provenance and path audit", name)
			}
			// These are exception/return-instruction hooks, NEVER normal BSR/RTS
			// events. Keep their occurrence count but do not infer IRQ stack parity.
			if h.PC == 0x474 || h.PC == 0x4d6 {
				if state != 0 {
					if h.PC == 0x474 {
						c.IRQEntriesDuringNested++
					} else {
						c.RTEsDuringNested++
					}
				}
				continue
			}
			if h.PC == 0x8a6 && state == 0 {
				callSP = h.SP
				state = 1
				continue
			}
			if state == 0 {
				continue
			}
			// Sampled unrelated instruction sites (e.g. branch predicates) are not
			// the expected subroutine return site; skip without claiming full CFG.
			tracked := false
			for _, pc := range rc6322NestedPattern {
				if pc == h.PC {
					tracked = true
					break
				}
			}
			if !tracked {
				continue
			}
			if state >= len(rc6322NestedPattern) || h.PC != rc6322NestedPattern[state] {
				return r, fmt.Errorf("%s: ordered nested stack mismatch ordinal=%d expected=%04X observed=%04X phase=%d", name, h.Ordinal, rc6322NestedPattern[state], h.PC, state)
			}
			// Stack pointer checks are on *real CPU-B instruction-entry values*.
			// JSR creates one return longword; each BSR creates another.
			var expectedSP uint32
			switch state {
			case 1:
				expectedSP = callSP - 4
			case 2, 3:
				expectedSP = bsrSP - 4
			case 4:
				expectedSP = bsrSP
			case 5, 6:
				expectedSP = bsrSP - 4
			case 7:
				expectedSP = bsrSP
			case 8, 9:
				expectedSP = bsrSP - 4
			case 10:
				expectedSP = bsrSP
			case 11:
				expectedSP = callSP
			}
			if state == 1 || state == 4 || state == 7 {
				bsrSP = h.SP
			}
			if state == 2 || state == 3 || state == 5 || state == 6 || state == 8 || state == 9 { // evaluate with saved pre-BSR stack
				if h.SP != bsrSP-4 {
					c.SampledStackPointersMatched = false
				}
			} else if state == 1 || state == 4 || state == 7 || state == 10 || state == 11 {
				if h.SP != expectedSP {
					c.SampledStackPointersMatched = false
				}
			}
			// This confirms *sampled* stack addresses only. It never verifies
			// stack RAM data, bus order or intervening IRQ processing.
			if state == 3 || state == 6 || state == 9 || state == 10 {
				c.OrderedPairs++
			}
			state++
			if state == len(rc6322NestedPattern) {
				state = 0
				c.NestedSequences++
			}
		}
		if state != 0 || c.NestedSequences != 4 || c.OrderedPairs != 16 {
			return r, fmt.Errorf("%s: incomplete nested timeline sequences=%d pairs=%d state=%d", name, c.NestedSequences, c.OrderedPairs, state)
		}
		if !c.SampledStackPointersMatched {
			return r, fmt.Errorf("%s: sampled BSR/RTS stack pointer differs", name)
		}
		r.Cases = append(r.Cases, c)
	}
	r.SelectedNestedTemporalOrderVerified = true
	r.SelectedStackPointerPairsVerified = true
	return r, nil
}
