package main

// RC6.3.17: read-only, original-ROM-observed function frontier and evidence-based
// *research* prioritisation. A finite dynamic graph is never full CFG proof.
import (
	"bufio"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"sort"
	"strconv"
	"strings"
)

var rc6317Cases = []string{"natural", "bypass", "nonterminal", "terminal", "overflow"}

type rc6317FrontierEdge struct {
	From          string           `json:"from"`
	To            string           `json:"to"`
	Opcode        string           `json:"sourceOpcode"`
	Kind          string           `json:"kind"`
	PerCaseCounts map[string]int64 `json:"perCaseCounts"`
}
type rc6317FrontierNode struct {
	PC                       string `json:"pc"`
	Side                     string `json:"side"`
	Hops                     int    `json:"hops"`
	ObservedSuccessorCount   int    `json:"observedSuccessorCount"`
	ObservedPredecessorCount int    `json:"observedPredecessorCount"`
}
type rc6317ResearchItem struct {
	Target                  string           `json:"target"`
	ProposedAction          string           `json:"proposedAction"`
	ObservedVisitsPerCase   map[string]int64 `json:"observedVisitsPerCase"`
	ResearchPriority        int              `json:"researchPriority"`
	ScoreMethod             string           `json:"scoreMethod"`
	HardwareRisk            string           `json:"hardwareRisk"`
	ROMAndCFGStatus         string           `json:"romAndCfgStatus"`
	MeasuredCPUCycleSavings *int64           `json:"measuredCpuCycleSavings"`
	AutoSelectedForResearch bool             `json:"autoSelectedForResearch"`
	NativePromotionAllowed  bool             `json:"nativePromotionAllowed"`
	Blockers                []string         `json:"blockers"`
}
type rc6317FrontierReport struct {
	Schema                   string               `json:"schema"`
	CPU                      string               `json:"cpu"`
	ROMSHA256                string               `json:"romSha256"`
	Candidate                string               `json:"candidate"`
	SourceMode               string               `json:"sourceMode"`
	SourceCases              []string             `json:"sourceCases"`
	CoverageIsExhaustive     bool                 `json:"coverageIsExhaustive"`
	CompleteFunctionVerified bool                 `json:"completeFunctionVerified"`
	NativePromotionAllowed   bool                 `json:"nativePromotionAllowed"`
	ObservedEdges            int                  `json:"totalUniqueObservedEdges"`
	Nodes                    []rc6317FrontierNode `json:"frontierNodes"`
	Edges                    []rc6317FrontierEdge `json:"frontierEdges"`
	ResearchQueue            []rc6317ResearchItem `json:"researchQueue"`
	Limitations              []string             `json:"limitations"`
}

type rc6317RawEdge struct {
	from, to uint32
	op       uint16
	counts   map[string]int64
}

func rc6317Addr(pc uint32) string { return fmt.Sprintf("0x%08X", pc) }

func rc6317ObservedFrontier(dest, romHash string) (rc6317FrontierReport, error) {
	r := rc6317FrontierReport{
		Schema: "chq-rc6317-original-rom-function-frontier-v1", CPU: "B", ROMSHA256: romHash,
		Candidate: rc6314PairID, SourceMode: "ORIGINAL_ROM_OBSERVED_PC_EDGES_ONLY",
		SourceCases: append([]string{}, rc6317Cases...), Nodes: []rc6317FrontierNode{}, Edges: []rc6317FrontierEdge{}, ResearchQueue: []rc6317ResearchItem{},
		Limitations: []string{"Observed graph is not a complete static/dynamic CFG; unobserved incoming edges and indirect calls remain unknown", "RTE to candidate is an observed resume, not automatically a function call", "IRQ vector/stack ownership and prefetch are not part of ordinary flow", "Native CPU-cycle savings and full-function scheduling or return semantics have not been measured", "No complete-function original/native/rollback comparison has been conducted"},
	}
	if !rc6315VerifyHash(romHash) {
		return r, errors.New("CPU-B ROM SHA256 is not authentic-looking")
	}
	audited, err := rc6316AuditPairEdges(dest, romHash)
	if err != nil || len(audited.Cases) != len(rc6317Cases) || audited.CompleteFunctionBoundaryVerified || audited.AutonomousPromotionAllowed {
		return r, fmt.Errorf("bounded original-ROM edge prerequisite not satisfied: %v", err)
	}
	edges := map[string]*rc6317RawEdge{}
	for _, name := range rc6317Cases {
		p := filepath.Join(dest, "branch-matrix", name, "live-health", "original", "cpu-b-observed-edges.tsv")
		f, e := os.Open(p)
		if e != nil {
			return r, e
		}
		scanner := bufio.NewScanner(f)
		scanner.Buffer(make([]byte, 4096), 2<<20)
		seen := map[string]bool{}
		rows := 0
		for scanner.Scan() {
			rows++
			if rows > 20000 {
				f.Close()
				return r, fmt.Errorf("excessive edge rows in %s", name)
			}
			fs := strings.Split(strings.TrimSpace(scanner.Text()), "\t")
			if len(fs) != 4 {
				f.Close()
				return r, fmt.Errorf("malformed edge row in %s", name)
			}
			from, e1 := strconv.ParseUint(fs[0], 16, 32)
			to, e2 := strconv.ParseUint(fs[1], 16, 32)
			op, e3 := strconv.ParseUint(fs[2], 16, 16)
			cnt, e4 := strconv.ParseInt(fs[3], 10, 64)
			if e1 != nil || e2 != nil || e3 != nil || e4 != nil || cnt <= 0 || from%2 != 0 || to%2 != 0 {
				f.Close()
				return r, fmt.Errorf("invalid edge fields in %s", name)
			}
			key := fmt.Sprintf("%08X:%08X", from, to)
			if seen[key] {
				f.Close()
				return r, fmt.Errorf("duplicate edge %s in %s", key, name)
			}
			seen[key] = true
			x, ok := edges[key]
			if !ok {
				x = &rc6317RawEdge{from: uint32(from), to: uint32(to), op: uint16(op), counts: map[string]int64{}}
				edges[key] = x
			}
			if x.op != uint16(op) {
				f.Close()
				return r, fmt.Errorf("inconsistent ROM opcode for %s across checkpoints", key)
			}
			x.counts[name] = cnt
		}
		e = scanner.Err()
		f.Close()
		if e != nil {
			return r, e
		}
		if rows == 0 {
			return r, fmt.Errorf("empty original-ROM edges: %s", name)
		}
	}
	r.ObservedEdges = len(edges)
	// Traverse at most two observed predecessor and successor edges from the
	// bounded candidate. Do not traverse IRQ entry or backwards across RTE.
	rev := map[uint32][]*rc6317RawEdge{}
	fwd := map[uint32][]*rc6317RawEdge{}
	for _, v := range edges {
		rev[v.to] = append(rev[v.to], v)
		fwd[v.from] = append(fwd[v.from], v)
	}
	type walk struct {
		pc   uint32
		hops int
		side string
	}
	queue := []walk{{0x4d8, 0, "upstream"}, {0x4e0, 0, "downstream"}}
	visits := map[string]walk{}
	selected := map[string]*rc6317RawEdge{}
	for len(queue) > 0 {
		cur := queue[0]
		queue = queue[1:]
		k := fmt.Sprintf("%s:%08X", cur.side, cur.pc)
		if old, ok := visits[k]; ok && old.hops <= cur.hops {
			continue
		}
		visits[k] = cur
		if cur.hops >= 2 {
			continue
		}
		choices := fwd[cur.pc]
		if cur.side == "upstream" {
			choices = rev[cur.pc]
		}
		for _, v := range choices {
			// Native substitute boundary cannot cross an IRQ handler, and a
			// return-from-exception is not evidence of an ordinary caller.
			if v.from == 0x474 || v.to == 0x474 || v.op == 0x4e73 {
				continue
			}
			nk := fmt.Sprintf("%08X:%08X", v.from, v.to)
			selected[nk] = v
			next := v.to
			if cur.side == "upstream" {
				next = v.from
			}
			if next != cur.pc {
				queue = append(queue, walk{next, cur.hops + 1, cur.side})
			}
		}
		if len(visits) > 128 {
			return r, errors.New("function frontier exceeds safety node cap")
		}
	}
	// Always retain verified pair edges and interrupt diversion in the view.
	for _, v := range edges {
		if v.from == 0x4d8 || v.from == 0x4de || v.to == 0x4d8 || v.to == 0x4de {
			selected[fmt.Sprintf("%08X:%08X", v.from, v.to)] = v
		}
	}
	order := make([]string, 0, len(selected))
	for k := range selected {
		order = append(order, k)
	}
	sort.Strings(order)
	if len(order) > 128 {
		return r, errors.New("frontier edge cap exceeded")
	}
	for _, k := range order {
		v := selected[k]
		kind := "observed-outside-candidate"
		switch {
		case v.to == 0x474:
			kind = "observed-irq-diversion"
		case v.op == 0x4e73:
			kind = "observed-rte-resume-not-a-caller"
		case v.from == 0x4d8 && v.to == 0x4de:
			kind = "verified-bounded-tst-sequence"
		case v.from == 0x4de && v.to == 0x4d8:
			kind = "verified-bounded-beq-taken"
		case v.from == 0x4de && v.to == 0x4e0:
			kind = "verified-bounded-beq-fallthrough"
		}
		r.Edges = append(r.Edges, rc6317FrontierEdge{From: rc6317Addr(v.from), To: rc6317Addr(v.to), Opcode: fmt.Sprintf("0x%04X", v.op), Kind: kind, PerCaseCounts: v.counts})
	}
	vk := make([]string, 0, len(visits))
	for k := range visits {
		vk = append(vk, k)
	}
	sort.Strings(vk)
	for _, k := range vk {
		v := visits[k]
		r.Nodes = append(r.Nodes, rc6317FrontierNode{PC: rc6317Addr(v.pc), Side: v.side, Hops: v.hops, ObservedSuccessorCount: len(fwd[v.pc]), ObservedPredecessorCount: len(rev[v.pc])})
	}
	// Research utility is an explicitly *heuristic* priority, never estimated
	// native speedup. Count only entries recorded in each authentic checkpoint.
	choices := []struct {
		pc                      uint32
		action, hardware, stage string
		base                    int
	}{
		{0x4e0, "Expand fallthrough into Ghidra CFG and find authentic exits", "UNKNOWN_CPU_B_SIDE_EFFECTS", "OBSERVED_EXIT_NOT_GENERATED", 70},
		{0x52c, "Trace original-ROM loop-back predecessor and identify enclosing entry", "UNKNOWN_CALLER_AND_SHARED_MEMORY", "OBSERVED_PREDECESSOR_NOT_GENERATED", 65},
		{0x4d6, "Classify RTE resume, stack/vector provenance and interrupt return", "HIGH_IRQ_EXCEPTION_RISK", "OBSERVED_RTE_NOT_FUNCTION_ENTRY", 35},
	}
	for _, x := range choices {
		count := map[string]int64{}
		for _, name := range rc6317Cases {
			count[name] = 0
		}
		for _, e := range fwd[x.pc] {
			for name, n := range e.counts {
				count[name] += n
			}
		}
		observed := 0
		for _, n := range count {
			if n > 0 {
				observed++
			}
		}
		score := x.base + observed*2
		if observed == 0 {
			score = 0
		}
		r.ResearchQueue = append(r.ResearchQueue, rc6317ResearchItem{Target: rc6317Addr(x.pc), ProposedAction: x.action, ObservedVisitsPerCase: count, ResearchPriority: score, ScoreMethod: "HEURISTIC_RESEARCH_VALUE_BASE_PLUS_TWO_PER_OBSERVED_CASE_NOT_CYCLE_SAVINGS", HardwareRisk: x.hardware, ROMAndCFGStatus: x.stage, AutoSelectedForResearch: false, NativePromotionAllowed: false, Blockers: []string{"Ghidra original-ROM site/function contract not yet established", "Full entry/return/IRQ and whole-machine function-level differential missing"}})
	}
	sort.SliceStable(r.ResearchQueue, func(i, j int) bool { return r.ResearchQueue[i].ResearchPriority > r.ResearchQueue[j].ResearchPriority })
	if len(r.ResearchQueue) > 0 && r.ResearchQueue[0].ResearchPriority > 0 {
		r.ResearchQueue[0].AutoSelectedForResearch = true
	}
	return r, nil
}
