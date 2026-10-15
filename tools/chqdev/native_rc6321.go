package main

// RC6.3.21: authenticate bounded nested call/return accounting with five
// genuine original-ROM checkpoint edge tables and independent fresh Ghidra
// instruction sites. Aggregated edge counts cannot prove temporal nesting,
// an exhaustive function CFG, stack memory parity or safe native replacement.
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

type rc6321Edge struct {
	From       string           `json:"from"`
	To         string           `json:"to"`
	Opcode     string           `json:"romOpcode"`
	Role       string           `json:"role"`
	HitsByCase map[string]int64 `json:"hitsByCase"`
}
type rc6321Report struct {
	Schema                       string       `json:"schema"`
	CPU                          string       `json:"cpu"`
	ROMSHA256                    string       `json:"romSha256"`
	EvidenceSource               string       `json:"evidenceSource"`
	SourceCases                  []string     `json:"sourceCases"`
	GhidraSites                  []rc639Site  `json:"ghidraSites"`
	Edges                        []rc6321Edge `json:"edges"`
	OuterNestedVisitsPerCase     int64        `json:"outerNestedVisitsPerCase"`
	TemporalStackPairingVerified bool         `json:"temporalStackPairingVerified"`
	CompleteNestedCalleeCFG      bool         `json:"completeNestedCalleeCfg"`
	AllIndirectTargetsVerified   bool         `json:"allIndirectTargetsVerified"`
	Unobserved0518Executed       bool         `json:"unobserved0518Executed"`
	NativePromotionAllowed       bool         `json:"nativePromotionAllowed"`
	Blockers                     []string     `json:"blockers"`
}
type rc6321Spec struct {
	from, to uint32
	opcode   uint16
	role     string
}

var rc6321Edges = []rc6321Spec{
	{0x4f70, 0x4fb2, 0x6100, "BSR-step-1"},
	{0x5062, 0x4f74, 0x4e75, "RTS-to-BSR-step-2"},
	{0x4f74, 0x4fb2, 0x6100, "BSR-step-2"},
	{0x5062, 0x4f78, 0x4e75, "RTS-to-BSR-step-3"},
	{0x4f78, 0x4fb2, 0x6100, "BSR-step-3"},
	{0x5062, 0x4f7c, 0x4e75, "RTS-to-final-RTS"},
	{0x4f7c, 0x8ac, 0x4e75, "RTS-to-outer-callee"},
}

func rc6321NestedReturnAudit(dest, romHash string) (rc6321Report, error) {
	r := rc6321Report{
		Schema: "chq-rc6321-cpu-b-nested-original-rom-return-v1", CPU: "B", ROMSHA256: romHash,
		EvidenceSource: "FRESH_GHIDRA_AUTHENTIC_CPU_B_ROM_AND_FIVE_UNMODIFIED_CHECKPOINT_EDGE_TABLES",
		SourceCases:    append([]string{}, rc6317Cases...), GhidraSites: []rc639Site{}, Edges: []rc6321Edge{},
		Blockers: []string{
			"Aggregate edge counts do not prove temporal call-stack pairing or intervening IRQ/scheduler state",
			"Nested 0x4FB2 callee has loops/branches; enclosing CFG and memory/flags/cycles effects not exhaustively verified",
			"Indirect outer JSR target set not exhaustive; 0x0518 branch alternate remains runtime-unobserved",
			"No full-function generated C++ native/original/fallback whole-machine parity or safe promotion",
		},
	}
	if !rc6315VerifyHash(romHash) {
		return r, errors.New("invalid original-ROM CPU-B digest")
	}
	if _, err := rc6319CallReturnAudit(dest, romHash); err != nil {
		return r, fmt.Errorf("original-ROM call/return prerequisite: %w", err)
	}
	b, err := os.ReadFile(filepath.Join(dest, "cpu-b-ghidra", "cpu-b-ghidra-sites.json"))
	if err != nil {
		return r, err
	}
	if len(b) > 256*1024 {
		return r, errors.New("oversized static Ghidra source")
	}
	var st rc639Static
	if err = json.Unmarshal(b, &st); err != nil {
		return r, err
	}
	if st.Schema != "chq-rc639-cpu-b-ghidra-sites-v1" || st.CPU != "B" || st.Language != "68000:BE:32:default" || !strings.EqualFold(st.RawROM, romHash) || st.NativePromotionAllowed {
		return r, errors.New("nested ROM provenance/maturity mismatch")
	}
	if len(st.NestedReturnSites) != 5 {
		return r, errors.New("requires five independent Ghidra nested sites")
	}
	specs := []struct {
		pc, op uint32
		length int
		prefix string
		fall   uint32
	}{
		{0x4f70, 0x6100, 4, "bsr", 0x4f74}, {0x4f74, 0x6100, 4, "bsr", 0x4f78}, {0x4f78, 0x6100, 4, "bsr", 0x4f7c},
		{0x4f7c, 0x4e75, 2, "rts", 0}, {0x5062, 0x4e75, 2, "rts", 0},
	}
	byPC := map[uint32]rc639Site{}
	for _, s := range st.NestedReturnSites {
		pc, e := strconv.ParseUint(s.PC, 0, 32)
		if e != nil || pc >= 0x20000 || pc%2 != 0 || byPC[uint32(pc)].PC != "" {
			return r, fmt.Errorf("invalid/duplicate nested static PC %q", s.PC)
		}
		byPC[uint32(pc)] = s
	}
	for _, spec := range specs {
		s, ok := byPC[spec.pc]
		if !ok {
			return r, fmt.Errorf("missing nested Ghidra site %X", spec.pc)
		}
		op, e := strconv.ParseUint(s.Opcode, 0, 16)
		if e != nil || op != uint64(spec.op) || s.Length != spec.length || s.FunctionReviewed || !strings.HasPrefix(strings.ToLower(strings.TrimSpace(s.Text)), spec.prefix) {
			return r, fmt.Errorf("nested Ghidra opcode/length/mnemonic mismatch %X", spec.pc)
		}
		if spec.fall != 0 && s.FallThrough != nil {
			v, e := strconv.ParseUint(*s.FallThrough, 0, 32)
			if e != nil || v != uint64(spec.fall) {
				return r, fmt.Errorf("nested Ghidra BSR return PC mismatch %X", spec.pc)
			}
		}
		if spec.fall != 0 {
			target := false
			for _, t := range s.FlowTargets {
				v, e := strconv.ParseUint(t, 0, 32)
				if e == nil && v == 0x4fb2 {
					target = true
				}
			}
			if !target {
				return r, fmt.Errorf("nested Ghidra BSR target mismatch %X", spec.pc)
			}
		}
		r.GhidraSites = append(r.GhidraSites, s)
	}
	for _, s := range rc6321Edges {
		r.Edges = append(r.Edges, rc6321Edge{From: rc6317Addr(s.from), To: rc6317Addr(s.to), Opcode: fmt.Sprintf("0x%04X", s.opcode), Role: s.role, HitsByCase: map[string]int64{}})
	}
	for _, name := range rc6317Cases {
		f, e := os.Open(filepath.Join(dest, "branch-matrix", name, "live-health", "original", "cpu-b-observed-edges.tsv"))
		if e != nil {
			return r, e
		}
		sc := bufio.NewScanner(f)
		sc.Buffer(make([]byte, 4096), 2<<20)
		countRows := 0
		seen := map[string]bool{}
		for sc.Scan() {
			countRows++
			if countRows > 20000 {
				f.Close()
				return r, errors.New("original nested trace too large")
			}
			col := strings.Split(strings.TrimSpace(sc.Text()), "\t")
			if len(col) != 4 {
				f.Close()
				return r, fmt.Errorf("invalid edge row in %s", name)
			}
			from, e1 := strconv.ParseUint(col[0], 16, 32)
			to, e2 := strconv.ParseUint(col[1], 16, 32)
			opcode, e3 := strconv.ParseUint(col[2], 16, 16)
			hits, e4 := strconv.ParseInt(col[3], 10, 64)
			if e1 != nil || e2 != nil || e3 != nil || e4 != nil || hits <= 0 {
				f.Close()
				return r, fmt.Errorf("invalid original edge in %s", name)
			}
			// Relevant instruction sites have only the enumerated successors
			// plus an asynchronous IRQ entry. Reject new unexplained paths;
			// never silently reinterpret them as a verified return.
			relevant := false
			matched := false
			for i, s := range rc6321Edges {
				if uint32(from) != s.from {
					continue
				}
				relevant = true
				if uint32(to) == s.to {
					if opcode != uint64(s.opcode) {
						f.Close()
						return r, fmt.Errorf("nested opcode changed %X->%X", from, to)
					}
					k := fmt.Sprintf("%X/%X", from, to)
					if seen[k] {
						f.Close()
						return r, fmt.Errorf("duplicate edge %s", k)
					}
					seen[k] = true
					r.Edges[i].HitsByCase[name] = hits
					matched = true
				}
			}
			if relevant && !matched && to != 0x474 {
				f.Close()
				return r, fmt.Errorf("unverified nested exit %X->%X in %s", from, to, name)
			}
		}
		readErr := sc.Err()
		f.Close()
		if readErr != nil {
			return r, readErr
		}
		// All seven nested edges must be observed, in all five authentic
		// checkpoint cases. Reconciliation only of per-edge counts.
		outer := int64(0)
		for i := range r.Edges {
			n := r.Edges[i].HitsByCase[name]
			if n <= 0 {
				return r, fmt.Errorf("missing nested return edge %s->%s in %s", r.Edges[i].From, r.Edges[i].To, name)
			}
			if i == 0 {
				outer = n
			}
			if n != outer {
				return r, fmt.Errorf("unbalanced nested BSR/RTS edges in %s", name)
			}
		}
		if outer != 4 {
			return r, fmt.Errorf("unexpected original-ROM nested path sample count in %s: %d", name, outer)
		}
		if r.OuterNestedVisitsPerCase == 0 {
			r.OuterNestedVisitsPerCase = outer
		}
	}
	return r, nil
}
