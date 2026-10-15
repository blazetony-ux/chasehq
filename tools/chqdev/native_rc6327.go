package main

// RC6.3.27: bounded authentic original-ROM CPU-B IRQ-delivery bus transactions.
// Musashi's real interrupt-ack callback and Bus::read/write supply the order.
// This does not prove external hardware vector ownership, all IRQs, 68000
// hardware cycle-level timing, whole-function closure or native promotion.
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

type rc6327Event struct {
	Sample, Frame  uint64
	Kind           byte
	Address, Value uint32
	Width          uint32
}
type rc6327Case struct {
	Case                 string `json:"case"`
	ConfirmedSequences   int    `json:"confirmedSequences"`
	ConfirmedVectorReads int    `json:"confirmedVectorReads"`
	ConfirmedStackWrites int    `json:"confirmedStackWrites"`
}
type rc6327Report struct {
	Schema                             string       `json:"schema"`
	EvidenceSource                     string       `json:"evidenceSource"`
	CPU                                string       `json:"cpu"`
	ROMSHA256                          string       `json:"romSha256"`
	Cases                              []rc6327Case `json:"cases"`
	SampledOriginalACKObserved         bool         `json:"sampledOriginalAckObserved"`
	SelectedVectorTableReadMatched     bool         `json:"selectedVectorTableReadMatched"`
	OrderedExceptionStackWritesMatched bool         `json:"orderedExceptionStackWritesMatched"`
	FirstHandlerHookMatched            bool         `json:"firstHandlerHookMatched"`
	HardwareVectorOwnershipVerified    bool         `json:"hardwareVectorOwnershipVerified"`
	CompleteIRQDeliveryVerified        bool         `json:"completeIrqDeliveryVerified"`
	ExactCycleTimingVerified           bool         `json:"exactCycleTimingVerified"`
	CompleteFunctionVerified           bool         `json:"completeFunctionVerified"`
	NativePromotionAllowed             bool         `json:"nativePromotionAllowed"`
	Blockers                           []string     `json:"blockers"`
}

func rc6327ReadBus(base string, transitions uint64) ([]rc6327Event, error) {
	raw, e := os.ReadFile(filepath.Join(base, "cpu-b-original-irq-delivery-cap.json"))
	if e != nil {
		return nil, e
	}
	var cap struct {
		Schema         string `json:"schema"`
		Source         string `json:"source"`
		Samples        uint64 `json:"samples"`
		Recorded       int    `json:"recorded"`
		Dropped        uint64 `json:"dropped"`
		Pending        bool   `json:"pending"`
		CPUTransitions uint64 `json:"cpuBTransitions"`
	}
	if len(raw) > 4096 || json.Unmarshal(raw, &cap) != nil || cap.Schema != "chq-rc6327-cpu-b-original-irq-delivery-cap-v1" || cap.Source != "ORIGINAL_ROM_MUSASHI_ACK_AND_CPU_B_BUS_TRANSACTIONS" || cap.Samples == 0 || cap.Samples > 5000 || cap.Dropped != 0 || cap.Pending || cap.Recorded != int(cap.Samples)*5 || cap.Recorded > 25000 || cap.CPUTransitions != transitions {
		return nil, errors.New("original-ROM IRQ transaction cap incomplete or untrusted")
	}
	f, e := os.Open(filepath.Join(base, "cpu-b-original-irq-delivery-bus.tsv"))
	if e != nil {
		return nil, e
	}
	defer f.Close()
	sc := bufio.NewScanner(f)
	sc.Buffer(make([]byte, 512), 4096)
	if !sc.Scan() || sc.Text() != "sample\tframe\tkind\taddress\tvalue\twidth" {
		return nil, errors.New("original-ROM IRQ bus header invalid")
	}
	out := make([]rc6327Event, 0, cap.Recorded)
	for sc.Scan() {
		row := strings.Split(sc.Text(), "\t")
		if len(row) != 6 || len(out) >= 25000 || len(row[2]) != 1 {
			return nil, fmt.Errorf("invalid original IRQ bus row %d", len(out)+1)
		}
		sn, e0 := strconv.ParseUint(row[0], 10, 64)
		fr, e1 := strconv.ParseUint(row[1], 10, 64)
		addr, e2 := strconv.ParseUint(row[3], 16, 32)
		value, e3 := strconv.ParseUint(row[4], 16, 32)
		width, e4 := strconv.ParseUint(row[5], 10, 32)
		if e0 != nil || e1 != nil || e2 != nil || e3 != nil || e4 != nil || fr > 10000000 || addr > 0xffffff || sn == 0 || sn > cap.Samples {
			return nil, fmt.Errorf("untrusted IRQ bus row %d", len(out)+1)
		}
		out = append(out, rc6327Event{sn, fr, row[2][0], uint32(addr), uint32(value), uint32(width)})
	}
	if sc.Err() != nil {
		return nil, sc.Err()
	}
	if len(out) != cap.Recorded {
		return nil, errors.New("IRQ bus row count mismatch")
	}
	return out, nil
}
func rc6327IRQDeliveryAudit(dest, romHash string) (rc6327Report, error) {
	r := rc6327Report{Schema: "chq-rc6327-original-rom-cpu-b-irq-vector-stack-bus-v1", EvidenceSource: "ORIGINAL_ROM_CPU_B_MUSASHI_ACK_AND_ORDERED_BUS_READ_WRITE_WITH_SEPARATE_FRAME_CROSSCHECK", CPU: "B", ROMSHA256: romHash, Cases: []rc6327Case{}, Blockers: []string{
		"The ordered bus callbacks reflect Musashi's emulated bus, not externally observed hardware bus cycles or arbitration",
		"Autovector address read is observed; physical IRQ vector ownership and board acknowledgement wiring are not independently established",
		"Only selected original-ROM checkpoint windows and one IRQ handler are covered, not every IRQ delivery or scheduler interleaving",
		"Exact 68000 bus-cycle timings, wait states, prefetch and interrupt priority nesting are not certified",
		"Unobserved CPU-B 0x0518, complete CFG/function ABI and autonomous native promotion remain blocked",
	}}
	frame, e := rc6325ExceptionFrameAudit(dest, romHash)
	if e != nil {
		return r, fmt.Errorf("prior exception-frame gate: %w", e)
	}
	if !frame.StackBankSwitchVerified || !frame.SupervisorFramePopVerified || frame.NativePromotionAllowed || len(frame.Cases) != 5 {
		return r, errors.New("unsafe prior frame maturity")
	}
	for _, caseProof := range frame.Cases {
		dir := filepath.Join(dest, "branch-matrix", caseProof.Case, "live-health", "original")
		_, transitions, e := rc6322ReadHooks(dest, caseProof.Case)
		if e != nil {
			return r, e
		}
		events, e := rc6327ReadBus(dir, transitions)
		if e != nil {
			return r, fmt.Errorf("%s: %w", caseProof.Case, e)
		}
		if len(events) != len(caseProof.Pairs)*5 {
			return r, fmt.Errorf("%s: IRQ bus/frame sample count inconsistent", caseProof.Case)
		}
		c := rc6327Case{Case: caseProof.Case}
		for i, pair := range caseProof.Pairs {
			seq := events[i*5 : (i+1)*5]
			frameTime := pair.Frame
			for j, v := range seq {
				if v.Sample != uint64(i+1) || v.Frame != frameTime {
					return r, fmt.Errorf("%s: wrong sample order/frame at index %d", caseProof.Case, i*5+j)
				}
			}
			a, vector, pcPush, srPush, handler := seq[0], seq[1], seq[2], seq[3], seq[4]
			interrupted, e := strconv.ParseUint(strings.TrimPrefix(pair.SavedInterruptedPC, "0x"), 16, 32)
			if e != nil {
				return r, e
			}
			sr, e := strconv.ParseUint(strings.TrimPrefix(pair.SavedInterruptedSR, "0x"), 16, 16)
			if e != nil {
				return r, e
			}
			sp := pair.FrameStackPointer
			if a.Kind != 'A' || a.Address != uint32(interrupted) || a.Value != 4 || a.Width != 0 ||
				vector.Kind != 'R' || vector.Address != 0x70 || vector.Value != 0x474 || vector.Width != 4 ||
				pcPush.Kind != 'W' || pcPush.Address != sp+2 || pcPush.Value != uint32(interrupted) || pcPush.Width != 4 ||
				srPush.Kind != 'W' || srPush.Address != sp || srPush.Value != uint32(sr) || srPush.Width != 2 ||
				handler.Kind != 'H' || handler.Address != 0x474 || handler.Value != 0 || handler.Width != 0 {
				return r, fmt.Errorf("%s: original IRQ ACK/vector/PC/SR/handler bus sequence mismatch sample=%d", caseProof.Case, i+1)
			}
			c.ConfirmedSequences++
			c.ConfirmedVectorReads++
			c.ConfirmedStackWrites += 2
		}
		if c.ConfirmedSequences < 1 {
			return r, fmt.Errorf("%s: no original-ROM IRQ bus evidence", caseProof.Case)
		}
		r.Cases = append(r.Cases, c)
	}
	r.SampledOriginalACKObserved = true
	r.SelectedVectorTableReadMatched = true
	r.OrderedExceptionStackWritesMatched = true
	r.FirstHandlerHookMatched = true
	return r, nil
}
