package main

// RC6.3.19: authentic original-ROM CPU-B call/return provenance. This is an
// observed *path*, NOT proof of a complete function, all indirect JSR targets,
// all RTS callers, unobserved BNE behaviour or safe native composition.
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

type rc6319UnobservedSite struct {
	PC                       string   `json:"pc"`
	RawOpcode                string   `json:"rawOpcode"`
	DisassemblyAvailable     bool     `json:"disassemblyAvailable"`
	Text                     *string  `json:"text"`
	Length                   *int     `json:"length"`
	FlowTargets              []string `json:"flowTargets"`
	FallThrough              *string  `json:"fallThrough"`
	OriginalRuntimeObserved  bool     `json:"originalRuntimeObserved"`
	CompleteFunctionReviewed bool     `json:"completeFunctionReviewed"`
}
type rc6319EdgeSample struct {
	From       string           `json:"from"`
	To         string           `json:"to"`
	ROMOpcode  string           `json:"romOpcode"`
	Role       string           `json:"role"`
	HitsByCase map[string]int64 `json:"hitsByCase"`
}
type rc6319CallReturnReport struct {
	Schema                      string               `json:"schema"`
	CPU                         string               `json:"cpu"`
	ROMSHA256                   string               `json:"romSha256"`
	SourceMode                  string               `json:"sourceMode"`
	SourceCases                 []string             `json:"sourceCases"`
	GhidraSites                 []rc639Site          `json:"ghidraSites"`
	Edges                       []rc6319EdgeSample   `json:"edges"`
	UnobservedFallthrough       rc6319UnobservedSite `json:"unobservedFallthrough"`
	IndirectTargetSetComplete   bool                 `json:"indirectTargetSetComplete"`
	FullCallerReturnSetVerified bool                 `json:"fullCallerReturnSetVerified"`
	CompleteFunctionVerified    bool                 `json:"completeFunctionVerified"`
	NativePromotionAllowed      bool                 `json:"nativePromotionAllowed"`
	Blockers                    []string             `json:"blockers"`
}
type rc6319EdgeSpec struct {
	from, to uint32
	opcode   uint16
	role     string
}

var rc6319Specs = []rc6319EdgeSpec{
	{0x50c, 0x6c2, 0x4e91, "observed-indirect-JSR-to-callee"},
	{0x8ac, 0x50e, 0x4e75, "observed-RTS-to-0x50E-return-site"},
	{0x50e, 0x516, 0x0839, "observed-postreturn-BTST-to-BNE"},
	{0x8a4, 0x8a6, 0x6606, "observed-callee-internal-BNE-fallthrough"},
	{0x8a4, 0x8ac, 0x6606, "observed-callee-internal-BNE-taken"},
	{0x8a6, 0x4f70, 0x4eb9, "observed-nested-JSR-not-full-callee-coverage"},
	{0x516, 0x52c, 0x6614, "observed-outer-BNE-taken"},
}

func rc6319CallReturnAudit(dest, romHash string) (rc6319CallReturnReport, error) {
	r := rc6319CallReturnReport{
		Schema: "chq-rc6319-cpu-b-original-rom-call-return-v1", CPU: "B", ROMSHA256: romHash,
		SourceMode:  "FRESH_GHIDRA_ORIGINAL_ROM_SHA_PLUS_FIVE_AUTHENTIC_ORIGINAL_CHECKPOINT_EDGES",
		SourceCases: append([]string{}, rc6317Cases...), GhidraSites: []rc639Site{}, Edges: []rc6319EdgeSample{},
		Blockers: []string{"Indirect JSR target set not exhaustive: 0x050C->0x06C2 is observed, not all possible A1 values",
			"RTS at 0x08AC returning to 0x050E does not establish complete stack/caller/callee coverage",
			"Nested JSR 0x08A6->0x4F70 and its return is not function-wide proven",
			"Unobserved BNE fallthrough 0x0516->0x0518 remains static-only; no runtime parity",
			"No complete CPU-B CFG, memory/IRQ/scheduling parity or native whole-function replacement"},
	}
	if !rc6315VerifyHash(romHash) {
		return r, errors.New("invalid CPU-B ROM SHA provenance")
	}
	// Verify all prior authentic edges and static instruction sites first.
	if _, e := rc6318Crosswalk(dest, romHash); e != nil {
		return r, fmt.Errorf("RC6318 prerequisite: %w", e)
	}
	raw, e := os.ReadFile(filepath.Join(dest, "cpu-b-ghidra", "cpu-b-ghidra-sites.json"))
	if e != nil {
		return r, e
	}
	if len(raw) > 256*1024 {
		return r, errors.New("oversized Ghidra record")
	}
	var st rc639Static
	if e = json.Unmarshal(raw, &st); e != nil {
		return r, e
	}
	if st.Schema != "chq-rc639-cpu-b-ghidra-sites-v1" || st.CPU != "B" || !strings.EqualFold(st.RawROM, romHash) || st.NativePromotionAllowed {
		return r, errors.New("Ghidra CPU-B provenance/maturity mismatch")
	}
	if len(st.CallReturnSites) != 3 || st.UnobservedFallthrough == nil {
		return r, errors.New("call/return static sites or unobserved branch missing")
	}
	expected := []struct {
		pc, op   uint32
		length   int
		mnemonic string
	}{
		{0x50c, 0x4e91, 2, "jsr"}, {0x50e, 0x0839, 8, "btst"}, {0x8ac, 0x4e75, 2, "rts"},
	}
	byPC := map[uint32]rc639Site{}
	for _, s := range st.CallReturnSites {
		pc, err := strconv.ParseUint(s.PC, 0, 32)
		if err != nil || pc%2 != 0 || byPC[uint32(pc)].PC != "" {
			return r, fmt.Errorf("invalid/duplicate call-return PC %q", s.PC)
		}
		byPC[uint32(pc)] = s
	}
	for _, spec := range expected {
		site, ok := byPC[spec.pc]
		if !ok {
			return r, fmt.Errorf("missing Ghidra call/return site %06X", spec.pc)
		}
		op, err := strconv.ParseUint(site.Opcode, 0, 16)
		if err != nil || uint32(op) != spec.op || site.Length != spec.length || site.FunctionReviewed || !strings.HasPrefix(strings.ToLower(strings.TrimSpace(site.Text)), spec.mnemonic) {
			return r, fmt.Errorf("Ghidra call/return opcode, length, or text mismatch %06X", spec.pc)
		}
		if spec.pc == 0x50e {
			// Ghidra renders CPU-B addresses with variable leading-zero
			// widths (e.g. 0x000516), whereas the runtime canonicalises
			// to eight digits (0x00000516). Compare the semantic address,
			// rejecting missing, malformed or different fallthroughs.
			if site.FallThrough == nil {
				return r, errors.New("post-return BTST fallthrough missing")
			}
			ft, parseErr := strconv.ParseUint(*site.FallThrough, 0, 32)
			if parseErr != nil || ft != 0x516 {
				return r, fmt.Errorf("post-return BTST fallthrough mismatch: got %q; want CPU-B 0x000516", *site.FallThrough)
			}
		}
		r.GhidraSites = append(r.GhidraSites, site)
	}
	u := st.UnobservedFallthrough
	uPC, uErr := strconv.ParseUint(u.PC, 0, 32)
	if uErr != nil || uPC != 0x518 || u.OriginalRuntimeObserved || u.CompleteFunctionReviewed {
		return r, errors.New("Ghidra unknown BNE path is improperly marked observed/reviewed")
	}
	if _, err := strconv.ParseUint(u.RawOpcode, 0, 16); err != nil {
		return r, errors.New("unobserved BNE fallback lacks raw ROM opcode")
	}
	if u.DisassemblyAvailable {
		if u.Text == nil || u.Length == nil || *u.Length < 2 || *u.Length > 22 || *u.Length%2 != 0 || strings.TrimSpace(*u.Text) == "" {
			return r, errors.New("invalid static-only unobserved instruction")
		}
	} else if u.Text != nil || u.Length != nil || len(u.FlowTargets) != 0 || u.FallThrough != nil {
		return r, errors.New("Ghidra unavailable path must not imply decoded instruction")
	}
	// Existing Ghidra 0x0516 BNE must independently assert the alternative
	// fallthrough 0x0518, even though no observed checkpoint took that path.
	hasBNE := false
	for _, s := range st.FrontierSites {
		if pc, err := strconv.ParseUint(s.PC, 0, 32); err == nil && pc == 0x516 {
			hasBNE = true
			branchPC := uint64(0)
			if s.FallThrough != nil {
				branchPC, _ = strconv.ParseUint(*s.FallThrough, 0, 32)
			}
			if branchPC != 0x518 {
				return r, errors.New("Ghidra BNE static fallthrough must be 0x0518")
			}
		}
	}
	if !hasBNE {
		return r, errors.New("static BNE site missing")
	}
	r.UnobservedFallthrough = *u
	for _, spec := range rc6319Specs {
		sample := rc6319EdgeSample{From: rc6317Addr(spec.from), To: rc6317Addr(spec.to), ROMOpcode: fmt.Sprintf("0x%04X", spec.opcode), Role: spec.role, HitsByCase: map[string]int64{}}
		r.Edges = append(r.Edges, sample)
	}
	for _, name := range rc6317Cases {
		p := filepath.Join(dest, "branch-matrix", name, "live-health", "original", "cpu-b-observed-edges.tsv")
		f, err := os.Open(p)
		if err != nil {
			return r, err
		}
		sc := bufio.NewScanner(f)
		sc.Buffer(make([]byte, 4096), 2<<20)
		seen := map[string]bool{}
		rows := 0
		for sc.Scan() {
			rows++
			if rows > 20000 {
				f.Close()
				return r, errors.New("excessive original-ROM edge rows")
			}
			cols := strings.Split(strings.TrimSpace(sc.Text()), "\t")
			if len(cols) != 4 {
				f.Close()
				return r, fmt.Errorf("invalid original-ROM edge row in %s", name)
			}
			from, er1 := strconv.ParseUint(cols[0], 16, 32)
			to, er2 := strconv.ParseUint(cols[1], 16, 32)
			op, er3 := strconv.ParseUint(cols[2], 16, 16)
			count, er4 := strconv.ParseInt(cols[3], 10, 64)
			if er1 != nil || er2 != nil || er3 != nil || er4 != nil || count <= 0 {
				f.Close()
				return r, fmt.Errorf("bad CPU-B edge in %s", name)
			}
			for i, s := range rc6319Specs {
				if uint32(from) != s.from {
					continue
				}
				if uint32(to) == 0x474 {
					continue
				} // asynchronous IRQ diversion; never call edge
				if uint32(to) == s.to {
					if uint16(op) != s.opcode {
						f.Close()
						return r, fmt.Errorf("CPU-B opcode mismatch on edge %06X->%06X", from, to)
					}
					key := fmt.Sprintf("%X/%X", from, to)
					if seen[key] {
						f.Close()
						return r, fmt.Errorf("duplicate CPU-B call edge %s", key)
					}
					seen[key] = true
					r.Edges[i].HitsByCase[name] = count
				} else {
					// The only allowed alternative on our observed instruction set
					// is the other arm of the internal callee BNE at 0x8A4.
					legal := false
					for _, v := range rc6319Specs {
						if v.from == uint32(from) && v.to == uint32(to) && v.opcode == uint16(op) {
							legal = true
							break
						}
					}
					if !legal {
						f.Close()
						return r, fmt.Errorf("unexpected observed CPU-B call/return edge %06X->%06X opcode %04X in %s", from, to, op, name)
					}
				}
			}
			// Explicitly forbid the previously unobserved alternative becoming
			// silently classified as witnessed without a new path-specific proof.
			if from == 0x516 && to == 0x518 {
				f.Close()
				return r, errors.New("new BNE fallthrough observed: requires expanded path-specific contract")
			}
		}
		err = sc.Err()
		f.Close()
		if err != nil {
			return r, err
		}
		for i := range r.Edges {
			if r.Edges[i].HitsByCase[name] <= 0 {
				return r, fmt.Errorf("missing CPU-B call/return edge %s->%s in %s", r.Edges[i].From, r.Edges[i].To, name)
			}
		}
		// The indirect JSR invocation, ensuing RTS, and postreturn BTST must
		// be balanced in these checkpoint observations, not just independently >0.
		if r.Edges[0].HitsByCase[name] != r.Edges[1].HitsByCase[name] || r.Edges[1].HitsByCase[name] != r.Edges[2].HitsByCase[name] {
			return r, fmt.Errorf("unbalanced observed call/return visits in %s", name)
		}
		if r.Edges[3].HitsByCase[name]+r.Edges[4].HitsByCase[name] != r.Edges[1].HitsByCase[name] || r.Edges[3].HitsByCase[name] != r.Edges[5].HitsByCase[name] {
			return r, fmt.Errorf("callee internal BNE/call accounting mismatch in %s", name)
		}
	}
	return r, nil
}
