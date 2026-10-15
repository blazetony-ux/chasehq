package main

// RC6.3.18: read-only Ghidra-to-authentic-original-ROM continuation crosswalk.
// Five additional static sites are compared to independent runtime edge
// evidence. Neither statically disassembling them nor sampling five scenarios
// proves an entire function or permits native substitution.

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

type rc6318ContinuationSite struct {
	PC                          string           `json:"pc"`
	ROMOpcode                   string           `json:"romOpcode"`
	GhidraText                  string           `json:"ghidraText"`
	GhidraLength                int              `json:"ghidraLength"`
	GhidraFlowTargets           []string         `json:"ghidraFlowTargets"`
	GhidraFallThrough           *string          `json:"ghidraFallThrough"`
	ObservedNextPC              string           `json:"observedNextPc"`
	OriginalROMEdgeCountPerCase map[string]int64 `json:"originalRomEdgeCountPerCase"`
	KnownUnobservedAlternatives []string         `json:"knownUnobservedAlternatives"`
	FunctionReviewed            bool             `json:"functionReviewed"`
	ValidatedAsCompleteFunction bool             `json:"validatedAsCompleteFunction"`
}

type rc6318ContinuationCrosswalk struct {
	Schema                   string                   `json:"schema"`
	CPU                      string                   `json:"cpu"`
	ROMSHA256                string                   `json:"romSha256"`
	SourceKind               string                   `json:"sourceKind"`
	SourceCases              []string                 `json:"sourceCases"`
	Sites                    []rc6318ContinuationSite `json:"sites"`
	CoverageIsExhaustive     bool                     `json:"coverageIsExhaustive"`
	CompleteCFGVerified      bool                     `json:"completeCfgVerified"`
	CompleteFunctionVerified bool                     `json:"completeFunctionVerified"`
	NativePromotionAllowed   bool                     `json:"nativePromotionAllowed"`
	Blockers                 []string                 `json:"blockers"`
}

type rc6318Spec struct {
	pc, opcode, length, next uint32
	mnemonic                 string
	branch                   bool
}

var rc6318Specs = []rc6318Spec{
	{0x4e0, 0x4279, 6, 0x4e6, "clr", false},
	{0x4e6, 0x5279, 6, 0x4ec, "addq", false},
	{0x4ec, 0x3039, 6, 0x4f2, "move", false},
	{0x516, 0x6614, 2, 0x52c, "bne", true},
	{0x52c, 0x60aa, 2, 0x4d8, "bra", true},
}

func rc6318Crosswalk(dest, romHash string) (rc6318ContinuationCrosswalk, error) {
	r := rc6318ContinuationCrosswalk{
		Schema: "chq-rc6318-cpu-b-ghidra-original-rom-continuation-v1", CPU: "B", ROMSHA256: romHash,
		SourceKind:  "HEADLESS_GHIDRA_AUTHENTIC_ROM_PLUS_FIVE_ORIGINAL_ROM_CHECKPOINT_TRACES",
		SourceCases: append([]string{}, rc6317Cases...), Sites: []rc6318ContinuationSite{},
		Blockers: []string{
			"Five continuation sites do not establish an enclosing CPU-B function entry, complete callers, or all exits",
			"Observed 0x0516 BNE.B taken path does not prove the fallthrough or all paths through CPU-B firmware",
			"Full instruction-level data bus, exceptions, shared memory, scheduling and original/native/rollback parity not proven for continuation",
			"No native complete-function library has been generated or promoted",
		},
	}
	if !rc6315VerifyHash(romHash) {
		return r, errors.New("CPU-B SHA256 provenance invalid")
	}
	// Existing original-only edge audit must pass first. We deliberately do
	// not accept Ghidra-only findings as a live substitution certificate.
	if _, e := rc6317ObservedFrontier(dest, romHash); e != nil {
		return r, fmt.Errorf("original-ROM frontier prerequisite: %w", e)
	}
	raw, e := os.ReadFile(filepath.Join(dest, "cpu-b-ghidra", "cpu-b-ghidra-sites.json"))
	if e != nil {
		return r, e
	}
	if len(raw) > 256*1024 {
		return r, errors.New("Ghidra continuation source oversized")
	}
	var static rc639Static
	if e = json.Unmarshal(raw, &static); e != nil {
		return r, e
	}
	if static.Schema != "chq-rc639-cpu-b-ghidra-sites-v1" || static.CPU != "B" || static.Language != "68000:BE:32:default" || !strings.EqualFold(static.RawROM, romHash) || static.NativePromotionAllowed {
		return r, errors.New("CPU-B Ghidra provenance or promotion guard mismatch")
	}
	if len(static.Sites) != 5 || len(static.FrontierSites) != len(rc6318Specs) {
		return r, errors.New("Ghidra did not export exactly five original sites and five continuation sites")
	}
	samples := map[uint32]map[string]int64{}
	for _, spec := range rc6318Specs {
		samples[spec.pc] = map[string]int64{}
	}
	for _, name := range rc6317Cases {
		p := filepath.Join(dest, "branch-matrix", name, "live-health", "original", "cpu-b-observed-edges.tsv")
		f, er := os.Open(p)
		if er != nil {
			return r, er
		}
		sc := bufio.NewScanner(f)
		sc.Buffer(make([]byte, 4096), 2<<20)
		seen := map[string]bool{}
		rows := 0
		for sc.Scan() {
			rows++
			if rows > 20000 {
				f.Close()
				return r, fmt.Errorf("oversized authentic edges in %s", name)
			}
			cols := strings.Split(strings.TrimSpace(sc.Text()), "\t")
			if len(cols) != 4 {
				f.Close()
				return r, fmt.Errorf("bad edge columns in %s", name)
			}
			from, e1 := strconv.ParseUint(cols[0], 16, 32)
			to, e2 := strconv.ParseUint(cols[1], 16, 32)
			opcode, e3 := strconv.ParseUint(cols[2], 16, 16)
			count, e4 := strconv.ParseInt(cols[3], 10, 64)
			if e1 != nil || e2 != nil || e3 != nil || e4 != nil || count <= 0 {
				f.Close()
				return r, fmt.Errorf("invalid authentic edge row in %s", name)
			}
			if from > 0xfffff || to > 0xfffff {
				continue
			}
			if samples[uint32(from)] == nil {
				continue
			}
			id := fmt.Sprintf("%X:%X", from, to)
			if seen[id] {
				f.Close()
				return r, fmt.Errorf("duplicate continuation edge in %s", name)
			}
			seen[id] = true
			spec := rc6318Spec{}
			for _, x := range rc6318Specs {
				if x.pc == uint32(from) {
					spec = x
					break
				}
			}
			// Interrupt entry can divert between any pair of ROM instructions;
			// it is NOT a control-flow successor attributed to the opcode.
			if uint32(to) == 0x474 {
				continue
			}
			if uint32(opcode) != spec.opcode || uint32(to) != spec.next {
				f.Close()
				return r, fmt.Errorf("unexpected CPU-B continuation %06X -> %06X opcode %04X in %s", from, to, opcode, name)
			}
			samples[spec.pc][name] += count
		}
		er = sc.Err()
		f.Close()
		if er != nil {
			return r, er
		}
	}
	byPC := map[uint32]rc639Site{}
	for _, s := range static.FrontierSites {
		pc, err := strconv.ParseUint(s.PC, 0, 32)
		if err != nil || pc%2 != 0 || pc >= 0x20000 || byPC[uint32(pc)].PC != "" {
			return r, fmt.Errorf("invalid or duplicate static continuation PC %q", s.PC)
		}
		byPC[uint32(pc)] = s
	}
	for _, spec := range rc6318Specs {
		site, ok := byPC[spec.pc]
		if !ok {
			return r, fmt.Errorf("Ghidra site %06X missing", spec.pc)
		}
		op, err := strconv.ParseUint(site.Opcode, 0, 16)
		if err != nil || uint32(op) != spec.opcode || site.Length != int(spec.length) || site.FunctionReviewed || !strings.HasPrefix(strings.ToLower(strings.TrimSpace(site.Text)), spec.mnemonic) {
			return r, fmt.Errorf("Ghidra continuation opcode/length/mnemonic/review mismatch at %06X", spec.pc)
		}
		expect := rc6317Addr(spec.next)
		if spec.branch {
			// Both signed 8-bit branches are resolved independently from
			// actual authenticated machine opcodes before trusting Ghidra.
			disp := int32(int8(byte(spec.opcode)))
			branchTarget := uint32(int32(spec.pc+2) + disp)
			if branchTarget != spec.next {
				return r, fmt.Errorf("invalid ROM branch target at %06X", spec.pc)
			}
			valid := false
			for _, ft := range site.FlowTargets {
				q, er := strconv.ParseUint(ft, 0, 32)
				if er == nil && uint32(q) == branchTarget {
					valid = true
				}
			}
			if !valid {
				return r, fmt.Errorf("Ghidra branch flow mismatch at %06X", spec.pc)
			}
		} else {
			if site.FallThrough == nil {
				return r, fmt.Errorf("missing Ghidra fallthrough at %06X", spec.pc)
			}
			ft, er := strconv.ParseUint(*site.FallThrough, 0, 32)
			if er != nil || uint32(ft) != spec.next {
				return r, fmt.Errorf("Ghidra fallthrough disagrees with original ROM at %06X", spec.pc)
			}
		}
		counts := map[string]int64{}
		for _, name := range rc6317Cases {
			n := samples[spec.pc][name]
			if n <= 0 {
				return r, fmt.Errorf("original ROM continuation %06X not observed in %s", spec.pc, name)
			}
			counts[name] = n
		}
		unknown := []string{}
		if spec.pc == 0x516 {
			unknown = append(unknown, "0x00000518 BNE fallthrough is not proven by this five-checkpoint path matrix")
		}
		r.Sites = append(r.Sites, rc6318ContinuationSite{
			PC: rc6317Addr(spec.pc), ROMOpcode: fmt.Sprintf("0x%04X", spec.opcode), GhidraText: site.Text,
			GhidraLength: site.Length, GhidraFlowTargets: site.FlowTargets, GhidraFallThrough: site.FallThrough,
			ObservedNextPC: expect, OriginalROMEdgeCountPerCase: counts, KnownUnobservedAlternatives: unknown,
		})
	}
	return r, nil
}
