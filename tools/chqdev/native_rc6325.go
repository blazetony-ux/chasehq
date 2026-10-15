package main

// RC6.3.26 authentic original-ROM CPU-B exception-frame *memory snapshots*.
// These instruction-hook peeks prove neither CPU bus transaction order nor
// vector ACK ownership. Only sampled handler/RTE/restored-state relationships
// are checked. Complete functions and native promotion remain blocked.
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

type rc6325Event struct {
	Ordinal uint64 `json:"ordinal"`
	Frame   uint64 `json:"frame"`
	Kind    string `json:"kind"`
	PC      uint32 `json:"pc"`
	SP      uint32 `json:"sp"`
	USP     uint32 `json:"usp"`
	ISP     uint32 `json:"isp"`
	SR      uint16 `json:"sr"`
	SavedSR uint16 `json:"savedSr"`
	SavedPC uint32 `json:"savedPc"`
}
type rc6325Pair struct {
	Frame                      uint64 `json:"frame"`
	HandlerOrdinal             uint64 `json:"handlerOrdinal"`
	RTEOrdinal                 uint64 `json:"rteOrdinal"`
	ResumeOrdinal              uint64 `json:"resumeOrdinal"`
	FrameStackPointer          uint32 `json:"frameStackPointer"`
	UserStackPointerBefore     uint32 `json:"userStackPointerBefore"`
	InterruptStackPointerAfter uint32 `json:"interruptStackPointerAfter"`
	ActiveStackPointerAfter    uint32 `json:"activeStackPointerAfter"`
	RestoredSupervisorMode     bool   `json:"restoredSupervisorMode"`
	SavedInterruptedPC         string `json:"savedInterruptedPc"`
	SavedInterruptedSR         string `json:"savedInterruptedSr"`
	ResumedPC                  string `json:"resumedPc"`
	ResumedSR                  string `json:"resumedSr"`
}
type rc6325Case struct {
	Case            string         `json:"case"`
	CapturedEvents  int            `json:"capturedEvents"`
	VerifiedSamples int            `json:"verifiedSamples"`
	ReturnTargets   map[string]int `json:"returnTargets"`
	Pairs           []rc6325Pair   `json:"pairs"`
}
type rc6325Report struct {
	Schema                           string       `json:"schema"`
	EvidenceSource                   string       `json:"evidenceSource"`
	CPU                              string       `json:"cpu"`
	ROMSHA256                        string       `json:"romSha256"`
	Cases                            []rc6325Case `json:"cases"`
	SampledExceptionFrameBytesStable bool         `json:"sampledExceptionFrameBytesStable"`
	SampledInterruptedPCSRRestored   bool         `json:"sampledInterruptedPcSrRestored"`
	StackBankSwitchVerified          bool         `json:"stackBankSwitchVerified"`
	SupervisorFramePopVerified       bool         `json:"supervisorFramePopVerified"`
	ImmediatePostRTEHookVerified     bool         `json:"immediatePostRteHookVerified"`
	ExceptionStackBusOrderVerified   bool         `json:"exceptionStackBusOrderVerified"`
	IRQVectorACKOwnershipVerified    bool         `json:"irqVectorAckOwnershipVerified"`
	CompleteIRQDeliveryVerified      bool         `json:"completeIrqDeliveryVerified"`
	SchedulerParityVerified          bool         `json:"schedulerParityVerified"`
	CompleteFunctionVerified         bool         `json:"completeFunctionVerified"`
	NativePromotionAllowed           bool         `json:"nativePromotionAllowed"`
	Blockers                         []string     `json:"blockers"`
}

func rc6325ReadEvents(dir string, upperBound uint64) ([]rc6325Event, error) {
	raw, err := os.ReadFile(filepath.Join(dir, "cpu-b-exception-frame-cap.json"))
	if err != nil {
		return nil, err
	}
	var cap struct {
		Schema      string `json:"schema"`
		Source      string `json:"source"`
		Recorded    int    `json:"recorded"`
		Dropped     uint64 `json:"dropped"`
		Pending     bool   `json:"pendingRte"`
		Transitions uint64 `json:"cpuBTransitions"`
	}
	if len(raw) > 4096 || json.Unmarshal(raw, &cap) != nil || cap.Schema != "chq-rc6326-cpu-b-exception-frame-cap-v2" || cap.Source != "ORIGINAL_ROM_CPU_B_INSTRUCTION_HOOK_RAM_PEEK" || cap.Dropped != 0 || cap.Pending || cap.Recorded < 3 || cap.Recorded > 25000 || cap.Transitions != upperBound {
		return nil, errors.New("original-ROM CPU-B frame cap absent, untrusted or incomplete")
	}
	f, err := os.Open(filepath.Join(dir, "cpu-b-exception-frame-hooks.tsv"))
	if err != nil {
		return nil, err
	}
	defer f.Close()
	sc := bufio.NewScanner(f)
	sc.Buffer(make([]byte, 1024), 4096)
	if !sc.Scan() || sc.Text() != "ordinal\tframe\tkind\tpc\tsp\tusp\tisp\tsr\tsavedSr\tsavedPc" {
		return nil, errors.New("CPU-B frame snapshot header invalid")
	}
	events := make([]rc6325Event, 0, cap.Recorded)
	var prior uint64
	for sc.Scan() {
		fields := strings.Split(sc.Text(), "\t")
		if len(fields) != 10 || len(events) >= 25000 {
			return nil, fmt.Errorf("bad frame snapshot row %d", len(events)+1)
		}
		dec := func(s string) (uint64, error) { return strconv.ParseUint(s, 10, 64) }
		hex := func(s string, b int) (uint64, error) { return strconv.ParseUint(s, 16, b) }
		n, e0 := dec(fields[0])
		fr, e1 := dec(fields[1])
		pc, e2 := hex(fields[3], 32)
		sp, e3 := hex(fields[4], 32)
		usp, e7 := hex(fields[5], 32)
		isp, e8 := hex(fields[6], 32)
		sr, e4 := hex(fields[7], 16)
		savedSR, e5 := hex(fields[8], 16)
		savedPC, e6 := hex(fields[9], 32)
		if e0 != nil || e1 != nil || e2 != nil || e3 != nil || e4 != nil || e5 != nil || e6 != nil || e7 != nil || e8 != nil || usp == 0 || isp == 0 || usp&1 != 0 || isp&1 != 0 || n > upperBound || len(events) > 0 && n <= prior || fr > 10000000 || pc >= 0x20000 || pc&1 != 0 || sp == 0 || sp&1 != 0 {
			return nil, fmt.Errorf("invalid CPU-B frame snapshot at row %d", len(events)+1)
		}
		kind := fields[2]
		if kind != "handler" && kind != "rte" && kind != "resume" || kind == "handler" && pc != 0x474 || kind == "rte" && pc != 0x4d6 || kind == "resume" && (savedSR != 0 || savedPC != 0) || kind != "resume" && (savedPC >= 0x20000 || savedPC&1 != 0) {
			return nil, fmt.Errorf("invalid CPU-B frame type/PC at row %d", len(events)+1)
		}
		events = append(events, rc6325Event{n, fr, kind, uint32(pc), uint32(sp), uint32(usp), uint32(isp), uint16(sr), uint16(savedSR), uint32(savedPC)})
		prior = n
	}
	if sc.Err() != nil {
		return nil, sc.Err()
	}
	if len(events) != cap.Recorded {
		return nil, errors.New("CPU-B frame snapshot count mismatch")
	}
	return events, nil
}

func rc6325RTEEdgeCounts(dir string) (map[uint32]int64, error) {
	f, err := os.Open(filepath.Join(dir, "cpu-b-observed-edges.tsv"))
	if err != nil {
		return nil, err
	}
	defer f.Close()
	sc := bufio.NewScanner(f)
	sc.Buffer(make([]byte, 1024), 2<<20)
	out := map[uint32]int64{}
	rows := 0
	for sc.Scan() {
		rows++
		if rows > 20000 {
			return nil, errors.New("too many original-ROM edge rows")
		}
		v := strings.Split(strings.TrimSpace(sc.Text()), "\t")
		if len(v) != 4 {
			return nil, errors.New("invalid original-ROM edge row")
		}
		from, e1 := strconv.ParseUint(v[0], 16, 32)
		to, e2 := strconv.ParseUint(v[1], 16, 32)
		opcode, e3 := strconv.ParseUint(v[2], 16, 16)
		n, e4 := strconv.ParseInt(v[3], 10, 64)
		if e1 != nil || e2 != nil || e3 != nil || e4 != nil || n <= 0 {
			return nil, errors.New("invalid original-ROM edge values")
		}
		if from == 0x4d6 {
			if opcode != 0x4e73 || to >= 0x20000 || to&1 != 0 {
				return nil, errors.New("unexpected RTE successor/opcode")
			}
			out[uint32(to)] += n
		}
	}
	return out, sc.Err()
}

func rc6325ExceptionFrameAudit(dest, romHash string) (rc6325Report, error) {
	r := rc6325Report{Schema: "chq-rc6326-cpu-b-original-rom-exception-frame-bank-aware-v2", EvidenceSource: "ORIGINAL_ROM_CPU_B_INSTRUCTION_HOOK_READ_ONLY_MEMORY_PEEK_NOT_BUS_CAPTURE", CPU: "B", ROMSHA256: romHash, Cases: []rc6325Case{}, Blockers: []string{
		"Instruction-hook RAM snapshots are not the original 68000 exception stack read/write bus transactions",
		"USP/ISP are observed at sampled instruction hooks; intermediate stack bank transactions not independently traced",
		"Hardware IRQ vector number, ACK/ownership and full exception-delivery chronology unverified",
		"Five short checkpoint windows cannot establish all IRQ paths, scheduler or exact timing parity",
		"CPU-B 0x0518 runtime branch remains unobserved; enclosing CFG and indirect targets incomplete",
		"Native function and autonomous multi-routine promotion remain prohibited",
	}}
	previous, err := rc6324IRQHookAudit(dest, romHash)
	if err != nil {
		return r, fmt.Errorf("selected IRQ/RTE prerequisite: %w", err)
	}
	if !previous.SelectedHandlerRTEOrderVerified || previous.ExceptionFrameBusVerified || previous.NativePromotionAllowed {
		return r, errors.New("unsafe previous IRQ maturity")
	}
	for _, prior := range previous.Cases {
		dir := filepath.Join(dest, "branch-matrix", prior.Case, "live-health", "original")
		_, total, err := rc6322ReadHooks(dest, prior.Case)
		if err != nil {
			return r, err
		}
		events, err := rc6325ReadEvents(dir, total)
		if err != nil {
			return r, fmt.Errorf("%s: %w", prior.Case, err)
		}
		edgeCounts, err := rc6325RTEEdgeCounts(dir)
		if err != nil {
			return r, fmt.Errorf("%s: %w", prior.Case, err)
		}
		c := rc6325Case{Case: prior.Case, CapturedEvents: len(events), ReturnTargets: map[string]int{}, Pairs: []rc6325Pair{}}
		if len(events) != len(prior.Pairs)*3 {
			return r, fmt.Errorf("%s: missing/extra exception hooks", prior.Case)
		}
		pairTargets := map[uint32]int64{}
		for i, p := range prior.Pairs {
			a, b, next := events[3*i], events[3*i+1], events[3*i+2]
			if a.Kind != "handler" || b.Kind != "rte" || next.Kind != "resume" || a.Ordinal != p.EntryOrdinal || b.Ordinal != p.RTEOrdinal || next.Ordinal != b.Ordinal+1 || a.Frame != p.Frame || b.Frame != p.Frame || next.Frame != p.Frame {
				return r, fmt.Errorf("%s: unpaired or nonimmediate RTE frame %d", prior.Case, i)
			}
			if a.SP != p.SampledSP || b.SP != p.SampledSP || a.SR != p.EntrySR || b.SR != p.RTESR || a.SR&0x2000 == 0 || b.SR&0x2000 == 0 {
				return r, fmt.Errorf("%s: original hook/frame provenance disagrees", prior.Case)
			}
			if a.SavedSR != b.SavedSR || a.SavedPC != b.SavedPC {
				return r, fmt.Errorf("%s: exception frame changed between handler and RTE", prior.Case)
			}
			// 68000 RTE pops six bytes from ISP; if saved SR clears S,
			// the new active A7 is USP, not ISP+6. Assert both banks,
			// including the inactive ISP and preserved USP independently.
			if a.SP != a.ISP || b.SP != b.ISP || a.USP != b.USP || a.ISP != b.ISP ||
				next.PC != b.SavedPC || next.SR != b.SavedSR ||
				next.USP != b.USP || next.ISP != b.ISP+6 ||
				(next.SR&0x2000 == 0 && next.SP != next.USP) ||
				(next.SR&0x2000 != 0 && next.SP != next.ISP) {
				return r, fmt.Errorf("%s: restored PC/SR/USP/ISP/active-SP mismatch at RTE ordinal=%d", prior.Case, b.Ordinal)
			}
			pairTargets[next.PC]++
			c.ReturnTargets[fmt.Sprintf("0x%06X", next.PC)]++
			c.VerifiedSamples++
			c.Pairs = append(c.Pairs, rc6325Pair{Frame: p.Frame, HandlerOrdinal: a.Ordinal, RTEOrdinal: b.Ordinal, ResumeOrdinal: next.Ordinal, FrameStackPointer: b.SP, UserStackPointerBefore: b.USP, InterruptStackPointerAfter: next.ISP, ActiveStackPointerAfter: next.SP, RestoredSupervisorMode: next.SR&0x2000 != 0, SavedInterruptedPC: fmt.Sprintf("0x%06X", b.SavedPC), SavedInterruptedSR: fmt.Sprintf("0x%04X", b.SavedSR), ResumedPC: fmt.Sprintf("0x%06X", next.PC), ResumedSR: fmt.Sprintf("0x%04X", next.SR)})
		}
		// Selected samples need not exhaust all authentic RTE edges; check only that
		// every sampled return has corroborating original-ROM edge evidence.
		for pc, count := range pairTargets {
			if edgeCounts[pc] < count {
				return r, fmt.Errorf("%s: RTE next-PC count mismatches original-ROM edge evidence at %04X", prior.Case, pc)
			}
		}
		if c.VerifiedSamples == 0 {
			return r, fmt.Errorf("%s: no original-ROM exception samples", prior.Case)
		}
		r.Cases = append(r.Cases, c)
	}
	r.SampledExceptionFrameBytesStable = true
	r.SampledInterruptedPCSRRestored = true
	r.ImmediatePostRTEHookVerified = true
	r.StackBankSwitchVerified = true
	r.SupervisorFramePopVerified = true
	return r, nil
}
