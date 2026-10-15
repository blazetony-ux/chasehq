package main

// RC6.3.30: record CPU-B 0x050E BTST.B #7,($100802).L's genuine source byte
// before execution, then the immediate original-ROM 0x0516 SR and the
// already-established 0x0516 BNE decision. The byte is read-only RAM evidence,
// NOT a captured data-bus cycle or a synthetic 0x0518 path. Any new Z=1 or
// 0x0518 execution fails closed under the existing RC6.3.29 gate.
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

type rc6330Source struct {
	Ordinal, Frame, Cycles uint64
	SourceByte             uint8
	NextOrdinal            uint64
	NextPC, NextSR         uint32
	NextFrame, NextCycles  uint64
}
type rc6330Case struct {
	Case              string `json:"case"`
	Observations      int    `json:"observations"`
	SourceBit7Set     int    `json:"sourceBit7Set"`
	SourceBit7Clear   int    `json:"sourceBit7Clear"`
	BranchTo052C      int    `json:"branchTo052C"`
	FallthroughTo0518 int    `json:"fallthroughTo0518"`
}
type rc6330Report struct {
	Schema                                string       `json:"schema"`
	CPU                                   string       `json:"cpu"`
	ROMSHA256                             string       `json:"romSha256"`
	EvidenceSource                        string       `json:"evidenceSource"`
	GhidraBTSTSourceOperandVerified       bool         `json:"ghidraBtstSourceOperandVerified"`
	OriginalByteMatchesBNEZeroFlag        bool         `json:"originalByteMatchesBneZeroFlag"`
	ImmediateSuccessorMatchesOrderedHooks bool         `json:"immediateSuccessorMatchesOrderedHooks"`
	MatchesOriginalROMBranchDecisions     bool         `json:"matchesOriginalRomBranchDecisions"`
	Cases                                 []rc6330Case `json:"cases"`
	OriginalRAMReadIsBusTransactionProof  bool         `json:"originalRamReadIsBusTransactionProof"`
	Observed0518Fallthrough               bool         `json:"observed0518Fallthrough"`
	CompleteFunctionCFGVerified           bool         `json:"completeFunctionCfgVerified"`
	NativePromotionAllowed                bool         `json:"nativePromotionAllowed"`
	Blockers                              []string     `json:"blockers"`
}

func rc6330ReadSourceEvidence(base string, transitions uint64) ([]rc6330Source, error) {
	b, err := os.ReadFile(filepath.Join(base, "cpu-b-original-050e-btst-cap.json"))
	if err != nil {
		return nil, err
	}
	var cap struct {
		Schema, Source string
		Hits, Recorded int
		Dropped        uint64
		Pending        bool
		CPUTransitions uint64 `json:"cpuBTransitions"`
	}
	if len(b) > 4096 || json.Unmarshal(b, &cap) != nil || cap.Schema != "chq-rc6330-cpu-b-050e-btst-source-v1" || cap.Source != "ORIGINAL_ROM_CPU_B_BTST_NONMUTATING_SOURCE_BYTE_AND_IMMEDIATE_HOOK" || cap.Hits < 1 || cap.Hits > 4096 || cap.Hits != cap.Recorded || cap.Dropped != 0 || cap.Pending || cap.CPUTransitions != transitions {
		return nil, errors.New("missing/untrusted original-ROM BTST capture cap")
	}
	f, err := os.Open(filepath.Join(base, "cpu-b-original-050e-btst-source.tsv"))
	if err != nil {
		return nil, err
	}
	defer f.Close()
	sc := bufio.NewScanner(f)
	sc.Buffer(make([]byte, 512), 4096)
	if !sc.Scan() || sc.Text() != "ordinal\tframe\tcyclesB\tpc\topcode\tsourceAddr\tsourceByte\tnextOrdinal\tnextPc\tnextSr\tnextFrame\tnextCyclesB" {
		return nil, errors.New("BTST capture header mismatch")
	}
	out := make([]rc6330Source, 0, cap.Recorded)
	for sc.Scan() {
		c := strings.Split(sc.Text(), "\t")
		if len(c) != 12 || len(out) >= 4096 {
			return nil, fmt.Errorf("invalid BTST row %d", len(out)+1)
		}
		dec := func(i int) (uint64, error) { return strconv.ParseUint(c[i], 10, 64) }
		hex := func(i, bit int) (uint64, error) { return strconv.ParseUint(c[i], 16, bit) }
		ord, e0 := dec(0)
		fr, e1 := dec(1)
		cy, e2 := dec(2)
		pc, e3 := hex(3, 32)
		op, e4 := hex(4, 16)
		addr, e5 := hex(5, 32)
		value, e6 := hex(6, 8)
		nord, e7 := dec(7)
		npc, e8 := hex(8, 32)
		nsr, e9 := hex(9, 16)
		nfr, e10 := dec(10)
		ncy, e11 := dec(11)
		if e0 != nil || e1 != nil || e2 != nil || e3 != nil || e4 != nil || e5 != nil || e6 != nil || e7 != nil || e8 != nil || e9 != nil || e10 != nil || e11 != nil || pc != 0x50e || op != 0x0839 || addr != 0x100802 || npc != 0x516 || nord != ord+1 || nord > transitions || fr > 10000000 || nfr != fr || ncy < cy || (len(out) > 0 && (ord <= out[len(out)-1].Ordinal || cy <= out[len(out)-1].Cycles)) {
			return nil, fmt.Errorf("untrusted BTST source row %d", len(out)+1)
		}
		out = append(out, rc6330Source{ord, fr, cy, uint8(value), nord, uint32(npc), uint32(nsr), nfr, ncy})
	}
	if sc.Err() != nil {
		return nil, sc.Err()
	}
	if len(out) != cap.Recorded {
		return nil, errors.New("BTST source count mismatch")
	}
	return out, nil
}
func rc6330GhidraBTSTVerified(dest string) error {
	raw, err := os.ReadFile(filepath.Join(dest, "cpu-b-ghidra", "cpu-b-ghidra-sites.json"))
	if err != nil {
		return err
	}
	var v struct {
		CallReturnSites []struct {
			PC          string `json:"pc"`
			Opcode      string `json:"opcode"`
			Text        string `json:"text"`
			FallThrough string `json:"fallThrough"`
		} `json:"callReturnSites"`
	}
	if len(raw) > 1<<20 || json.Unmarshal(raw, &v) != nil {
		return errors.New("untrusted CPU-B Ghidra site export")
	}
	for _, site := range v.CallReturnSites {
		if site.PC == "0x00050E" && site.Opcode == "0x0839" && strings.Contains(strings.ToLower(site.Text), "btst.b #0x7,(0x00100802).l") && site.FallThrough == "0x000516" {
			return nil
		}
	}
	return errors.New("Ghidra BTST.B #7,($100802).L provenance not established")
}
func rc6330BTSTSourceAudit(dest, romHash string) (rc6330Report, error) {
	r := rc6330Report{Schema: "chq-rc6330-original-rom-cpu-b-btst-bit7-source-v1", CPU: "B", ROMSHA256: romHash, EvidenceSource: "ORIGINAL_ROM_CPU_B_BTST_BYTE_PEEK_AND_IMMEDIATE_SR_HOOK_CROSSCHECKED_WITH_GHIDRA_AND_BNE_NEXT_PC", Cases: []rc6330Case{}, Blockers: []string{
		"Original-ROM memory peek is not an observed data-bus transaction or physical arcade hardware measurement",
		"Original-ROM five-checkpoint sample retains bit7=1; no Z=1 nor 0x0518 fallthrough observed",
		"No modified-state/forced-bit run is accepted as authentic CPU-B ROM evidence",
		"Unreviewed callers, indirect targets and enclosing function CFG block whole-function native promotion",
	}}
	if !rc6315VerifyHash(romHash) {
		return r, errors.New("authentic CPU-B ROM identity required")
	}
	if err := rc6330GhidraBTSTVerified(dest); err != nil {
		return r, err
	}
	prior, err := rc6328BranchFrontierAudit(dest, romHash)
	if err != nil {
		return r, fmt.Errorf("authentic BNE/next-PC prerequisite: %w", err)
	}
	if prior.NativePromotionAllowed || prior.Observed0518Fallthrough {
		return r, errors.New("unsafe predecessor maturity")
	}
	for _, name := range rc6317Cases {
		hooks, trans, err := rc6322ReadHooks(dest, name)
		if err != nil {
			return r, fmt.Errorf("%s hooks: %w", name, err)
		}
		base := filepath.Join(dest, "branch-matrix", name, "live-health", "original")
		sources, err := rc6330ReadSourceEvidence(base, trans)
		if err != nil {
			return r, fmt.Errorf("%s source: %w", name, err)
		}
		decisions, err := rc6328ReadDecisionEvidence(base, trans)
		if err != nil {
			return r, fmt.Errorf("%s decisions: %w", name, err)
		}
		predecessors := []rc6322Hook{}
		successors := []rc6322Hook{}
		for _, h := range hooks {
			if h.PC == 0x50e {
				predecessors = append(predecessors, h)
			}
			if h.PC == 0x516 {
				successors = append(successors, h)
			}
		}
		if len(sources) == 0 || len(sources) != len(decisions) || len(sources) != len(predecessors) || len(sources) != len(successors) {
			return r, fmt.Errorf("%s BTST, BNE and selected-hook counts differ", name)
		}
		cr := rc6330Case{Case: name}
		for i, x := range sources {
			pre, post, d := predecessors[i], successors[i], decisions[i]
			if pre.Opcode != 0x0839 || post.Opcode != 0x6614 || x.Ordinal != pre.Ordinal || x.Frame != pre.Frame || x.Cycles != pre.Cycles || x.NextOrdinal != post.Ordinal || x.NextFrame != post.Frame || x.NextCycles != post.Cycles || x.NextSR != uint32(post.SR) || d.Ordinal != post.Ordinal || d.SR != x.NextSR {
				return r, fmt.Errorf("%s original-ROM Ghidra/selected-hook source mismatch row %d", name, i+1)
			}
			// Genuine BTST.B bit 7 sets Z=1 when bit7 is CLEAR, and Z=0 when SET.
			z := uint32(0)
			if x.SourceByte&0x80 == 0 {
				z = 0x4
				cr.SourceBit7Clear++
			} else {
				cr.SourceBit7Set++
			}
			if x.NextSR&0x4 != z {
				return r, fmt.Errorf("%s BTST source bit7 / original-ROM 0x0516 Z mismatch row %d byte=%02X SR=%04X", name, i+1, x.SourceByte, x.NextSR)
			}
			if z != 0 || d.NextPC == 0x518 {
				return r, fmt.Errorf("%s new fallthrough condition requires separate controlled/original-path review", name)
			}
			if d.NextPC != 0x52c {
				return r, fmt.Errorf("%s BNE destination not original observed 0x052C", name)
			}
			cr.BranchTo052C++
			cr.Observations++
		}
		r.Cases = append(r.Cases, cr)
	}
	if len(r.Cases) != 5 {
		return r, errors.New("incomplete CPU-B five-checkpoint BTST audit")
	}
	r.GhidraBTSTSourceOperandVerified = true
	r.OriginalByteMatchesBNEZeroFlag = true
	r.ImmediateSuccessorMatchesOrderedHooks = true
	r.MatchesOriginalROMBranchDecisions = true
	return r, nil
}
