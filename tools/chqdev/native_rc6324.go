package main

// RC6.3.24: bounded IRQ-handler/RTE *selected-hook* chronology from authentic
// original-ROM CPU-B execution. These hooks DO NOT capture the exception-frame
// bus traffic, vector acknowledgement, intervening instructions or true stack
// contents. They must never be used to certify IRQ parity or native promotion.
import (
	"errors"
	"fmt"
)

type rc6324IRQPair struct {
	Frame                   uint64 `json:"frame"`
	EntryOrdinal            uint64 `json:"handlerEntryOrdinal"`
	RTEOrdinal              uint64 `json:"rteOrdinal"`
	EntryPC                 string `json:"handlerEntryPc"`
	ReturnInstructionPC     string `json:"rteInstructionPc"`
	SampledSP               uint32 `json:"sampledSp"`
	EntrySR                 uint16 `json:"handlerEntrySr"`
	RTESR                   uint16 `json:"rteInstructionSr"`
	ObservedTransitionGap   uint64 `json:"selectedHookOrdinalGap"`
	DuringSampledNestedCall bool   `json:"duringSampledNestedCall"`
}
type rc6324Case struct {
	Case                  string          `json:"case"`
	SourceHooks           int             `json:"sourceHooks"`
	PairedHandlerRTE      int             `json:"pairedHandlerRte"`
	HandlerWithoutRTE     int             `json:"handlerWithoutRte"`
	RTEWithoutHandler     int             `json:"rteWithoutHandler"`
	PairsDuringNestedCall int             `json:"pairsDuringNestedCall"`
	Pairs                 []rc6324IRQPair `json:"sampledPairs"`
}
type rc6324Report struct {
	Schema                                      string       `json:"schema"`
	EvidenceSource                              string       `json:"evidenceSource"`
	CPU                                         string       `json:"cpu"`
	ROMSHA256                                   string       `json:"romSha256"`
	Cases                                       []rc6324Case `json:"cases"`
	SelectedHandlerRTEOrderVerified             bool         `json:"selectedHandlerRteOrderVerified"`
	SampledHandlerRTEStackPointersMatched       bool         `json:"sampledHandlerRteStackPointersMatched"`
	AllIRQDeliveriesCaptured                    bool         `json:"allIrqDeliveriesCaptured"`
	ExceptionFrameBusVerified                   bool         `json:"exceptionFrameBusVerified"`
	IRQVectorOwnershipVerified                  bool         `json:"irqVectorOwnershipVerified"`
	IRQDeliveryAndSchedulerTimingParityVerified bool         `json:"irqDeliveryAndSchedulerTimingParityVerified"`
	CompleteFunctionVerified                    bool         `json:"completeFunctionVerified"`
	NativePromotionAllowed                      bool         `json:"nativePromotionAllowed"`
	Unobserved0518Executed                      bool         `json:"unobserved0518Executed"`
	Blockers                                    []string     `json:"blockers"`
}

func rc6324IRQHookAudit(dest, romHash string) (rc6324Report, error) {
	result := rc6324Report{
		Schema:         "chq-rc6324-cpu-b-original-rom-selected-irq-rte-context-v1",
		EvidenceSource: "ORIGINAL_ROM_CPU_B_SELECTED_INSTRUCTION_HOOK_NOT_FULL_EXCEPTION_TRACE",
		CPU:            "B", ROMSHA256: romHash, Cases: []rc6324Case{},
		Blockers: []string{
			"Original-ROM hook projection filters out other CPU-B instructions and IRQ delivery events",
			"Matching handler-entry/RTE instruction-entry SP does not prove exception stack-frame bytes or bus transaction order",
			"No independently observed IRQ vector acknowledgement, interrupted PC/SR restoration or scheduler/timing equivalence",
			"Unobserved 0x0518, incomplete CPU-B CFG/indirect caller set, no complete function differential replay",
			"Autonomous native function/multi-routine promotion prohibited",
		},
	}
	// Full previously qualified sampled BSR/RTS chronology + Ghidra/edge
	// prerequisites must succeed before a narrower IRQ-context crosswalk.
	ordered, err := rc6322OrderedAudit(dest, romHash)
	if err != nil {
		return result, fmt.Errorf("ordered original-ROM prerequisite: %w", err)
	}
	if !ordered.SelectedNestedTemporalOrderVerified || !ordered.SelectedStackPointerPairsVerified || ordered.InterruptRTEStackParityVerified || ordered.NativePromotionAllowed {
		return result, errors.New("ordered prerequisite has unsafe maturity")
	}
	for _, name := range rc6317Cases {
		hooks, _, err := rc6322ReadHooks(dest, name)
		if err != nil {
			return result, fmt.Errorf("%s: %w", name, err)
		}
		c := rc6324Case{Case: name, SourceHooks: len(hooks), Pairs: []rc6324IRQPair{}}
		var pending *rc6322Hook
		nested := false
		pendingNested := false
		for _, h := range hooks {
			if h.PC == 0x08a6 {
				nested = true
			}
			if h.PC == 0x08ac {
				nested = false
			}
			if h.PC == 0x0474 {
				if pending != nil {
					c.HandlerWithoutRTE++
					return result, fmt.Errorf("%s: duplicated IRQ-handler site before RTE frame=%d ordinal=%d", name, h.Frame, h.Ordinal)
				}
				if h.Opcode != 0x48e7 || h.SP == 0 || h.SP&1 != 0 || h.SR&0x2000 == 0 {
					return result, fmt.Errorf("%s: invalid IRQ-labelled handler hook at ordinal=%d", name, h.Ordinal)
				}
				value := h
				pending = &value
				pendingNested = nested
				continue
			}
			if h.PC == 0x04d6 {
				if pending == nil {
					c.RTEWithoutHandler++
					return result, fmt.Errorf("%s: orphan RTE ordinal=%d", name, h.Ordinal)
				}
				if h.Opcode != 0x4e73 || h.SR&0x2000 == 0 || h.Frame != pending.Frame || h.SP != pending.SP || h.Ordinal <= pending.Ordinal || h.Ordinal-pending.Ordinal > 1024 {
					return result, fmt.Errorf("%s: mismatched sampled handler/RTE context entry=%d RTE=%d", name, pending.Ordinal, h.Ordinal)
				}
				if nested != pendingNested {
					return result, fmt.Errorf("%s: sampled nested-call phase changed inside IRQ handler interval", name)
				}
				pair := rc6324IRQPair{
					Frame: h.Frame, EntryOrdinal: pending.Ordinal, RTEOrdinal: h.Ordinal,
					EntryPC: "0x000474", ReturnInstructionPC: "0x0004D6",
					SampledSP: h.SP, EntrySR: pending.SR, RTESR: h.SR,
					ObservedTransitionGap: h.Ordinal - pending.Ordinal, DuringSampledNestedCall: pendingNested,
				}
				c.Pairs = append(c.Pairs, pair)
				c.PairedHandlerRTE++
				if pendingNested {
					c.PairsDuringNestedCall++
				}
				pending = nil
				continue
			}
			// In the current selected-hook sample, each matching pair has no
			// other selected hooks between it. This is not a claim that no
			// unselected instruction executed in that interval.
			if pending != nil {
				return result, fmt.Errorf("%s: selected hook %04X intervenes between handler entry and RTE", name, h.PC)
			}
		}
		if pending != nil {
			c.HandlerWithoutRTE++
			return result, fmt.Errorf("%s: missing RTE for handler entry ordinal=%d", name, pending.Ordinal)
		}
		if c.PairedHandlerRTE == 0 || c.RTEWithoutHandler != 0 || c.HandlerWithoutRTE != 0 {
			return result, fmt.Errorf("%s: no complete selected IRQ/RTE pair", name)
		}
		result.Cases = append(result.Cases, c)
	}
	result.SelectedHandlerRTEOrderVerified = true
	result.SampledHandlerRTEStackPointersMatched = true
	return result, nil
}
