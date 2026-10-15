package main

// RC6.3.16: cross-stage candidate reconciliation and authentic CPU-B
// bounded control-flow edge accounting. Neither can certify a full function.
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

type rc6316Edge struct {
	From   string `json:"from"`
	To     string `json:"to"`
	Opcode string `json:"opcode"`
	Count  int64  `json:"count"`
	Kind   string `json:"kind"`
}
type rc6316CaseEdges struct {
	Case                 string       `json:"case"`
	ObservedPairEntries  int64        `json:"observedPairEntries"`
	PairSequentialEdges  int64        `json:"pairSequentialEdges"`
	PairTakenEdges       int64        `json:"pairTakenEdges"`
	PairFallthroughEdges int64        `json:"pairFallthroughEdges"`
	InterruptExitEdges   int64        `json:"interruptExitEdges"`
	ExternalPredecessors []rc6316Edge `json:"externalPredecessors"`
	ExternalSuccessors   []rc6316Edge `json:"externalSuccessors"`
}
type rc6316EdgeAudit struct {
	Schema                           string            `json:"schema"`
	CPU                              string            `json:"cpu"`
	ROMSHA256                        string            `json:"romSha256"`
	Candidate                        string            `json:"candidate"`
	Cases                            []rc6316CaseEdges `json:"cases"`
	Status                           string            `json:"status"`
	CompleteCFGVerified              bool              `json:"completeCFGVerified"`
	CompleteFunctionBoundaryVerified bool              `json:"completeFunctionBoundaryVerified"`
	AutonomousPromotionAllowed       bool              `json:"autonomousPromotionAllowed"`
	Unresolved                       []string          `json:"unresolved"`
}

// All PCs are original-ROM edge samples. IRQ edges to 0x474 are not
// reclassified as ordinary BEQ/TST branches. Do not infer unseen paths.
func rc6316AuditPairEdges(dest, romHash string) (rc6316EdgeAudit, error) {
	v := rc6316EdgeAudit{Schema: "chq-rc6316-cpu-b-bounded-edges-v1", CPU: "B", ROMSHA256: romHash,
		Candidate: rc6314PairID, Status: "UNVERIFIED", Cases: []rc6316CaseEdges{},
		Unresolved: []string{"Observed opcode transitions do not establish a complete enclosing function CFG", "Indirect callers, unobserved branch and exception paths are not exhausted", "Complete function entry, exits, scheduler interleavings and whole-function parity unverified"}}
	if !rc6315VerifyHash(romHash) {
		return v, errors.New("unverified CPU-B ROM hash")
	}
	for _, name := range []string{"natural", "bypass", "nonterminal", "terminal", "overflow"} {
		p := filepath.Join(dest, "branch-matrix", name, "live-health", "original", "cpu-b-observed-edges.tsv")
		f, err := os.Open(p)
		if err != nil {
			return v, fmt.Errorf("original-ROM edge evidence %s: %w", name, err)
		}
		ce := rc6316CaseEdges{Case: name, ExternalPredecessors: []rc6316Edge{}, ExternalSuccessors: []rc6316Edge{}}
		seen := map[string]bool{}
		sc := bufio.NewScanner(f)
		sc.Buffer(make([]byte, 4096), 2<<20)
		rows := 0
		for sc.Scan() {
			rows++
			if rows > 1000000 {
				f.Close()
				return v, fmt.Errorf("oversized CPU-B edge evidence for %s", name)
			}
			fs := strings.Split(strings.TrimSpace(sc.Text()), "\t")
			if len(fs) != 4 {
				f.Close()
				return v, fmt.Errorf("invalid CPU-B edge row %d in %s", rows, name)
			}
			from, e1 := strconv.ParseUint(fs[0], 16, 32)
			to, e2 := strconv.ParseUint(fs[1], 16, 32)
			op, e3 := strconv.ParseUint(fs[2], 16, 16)
			n, e4 := strconv.ParseInt(fs[3], 10, 64)
			if e1 != nil || e2 != nil || e3 != nil || e4 != nil || n <= 0 || from%2 != 0 || to%2 != 0 {
				f.Close()
				return v, fmt.Errorf("bad CPU-B edge row %d in %s", rows, name)
			}
			key := fmt.Sprintf("%X:%X", from, to)
			if seen[key] {
				f.Close()
				return v, fmt.Errorf("duplicate CPU-B edge %s in %s", key, name)
			}
			seen[key] = true
			srcIn := from == 0x4d8 || from == 0x4de
			dstIn := to == 0x4d8 || to == 0x4de
			edge := rc6316Edge{From: fmt.Sprintf("0x%08X", from), To: fmt.Sprintf("0x%08X", to), Opcode: fmt.Sprintf("0x%04X", op), Count: n, Kind: "original-rom-edge"}
			switch {
			case from == 0x4d8 && to == 0x4de:
				if op != 0x4a79 {
					f.Close()
					return v, fmt.Errorf("wrong genuine TST.W opcode in %s: 0x%04X", name, op)
				}
				ce.PairSequentialEdges += n
			case from == 0x4de && to == 0x4d8:
				if op != 0x67f8 {
					f.Close()
					return v, fmt.Errorf("wrong genuine BEQ.B opcode in %s", name)
				}
				ce.PairTakenEdges += n
			case from == 0x4de && to == 0x4e0:
				if op != 0x67f8 {
					f.Close()
					return v, fmt.Errorf("wrong genuine BEQ.B fallthrough opcode in %s", name)
				}
				ce.PairFallthroughEdges += n
			}
			if !srcIn && dstIn {
				ce.ObservedPairEntries += n
				edge.Kind = "external-entry"
				ce.ExternalPredecessors = append(ce.ExternalPredecessors, edge)
			}
			if srcIn && !dstIn {
				if to == 0x474 {
					ce.InterruptExitEdges += n
					edge.Kind = "interrupt-observed-exit"
				} else {
					edge.Kind = "external-exit"
				}
				ce.ExternalSuccessors = append(ce.ExternalSuccessors, edge)
			}
		}
		err = sc.Err()
		f.Close()
		if err != nil {
			return v, err
		}
		if ce.PairSequentialEdges < 1 || ce.PairTakenEdges < 1 || ce.PairFallthroughEdges < 1 || ce.ObservedPairEntries < 1 || len(ce.ExternalSuccessors) == 0 {
			return v, fmt.Errorf("incomplete authentic CPU-B pair edge coverage in %s: %+v", name, ce)
		}
		v.Cases = append(v.Cases, ce)
	}
	v.Status = "AUTHENTIC_BOUNDED_ENTRY_BRANCH_EXIT_SAMPLES_FUNCTION_UNRESOLVED"
	return v, nil
}

// Reconcile only independently authenticated live proof. A legacy per-stage
// portfolio can remain in the archive, but the authoritative *current*
// portfolio must never describe the proven two-instruction pair as offline.
func rc6316ReconcilePortfolio(dest, root string, p *rc634Portfolio, maturity rc6312CurrentMaturity) error {
	if p == nil {
		return errors.New("missing candidate portfolio")
	}
	single := maturity.CPUBBEQ == "LIVE_VERIFIED_BOUNDED_ONE_INSTRUCTION"
	pair := maturity.CPUBPair == "LIVE_VERIFIED_BOUNDED_TWO_INSTRUCTIONS"
	foundSingle, foundPair := false, false
	if pair && !single {
		return errors.New("CPU-B pair verified without prerequisite single-BEQ proof")
	}
	if pair {
		raw, err := os.ReadFile(filepath.Join(dest, "cpu-b-pair-live-proof.json"))
		if err != nil {
			return err
		}
		var proof rc6313PairLiveProof
		if err = json.Unmarshal(raw, &proof); err != nil {
			return err
		}
		if proof.Status != "LIVE_GHIDRA_CPU_B_TST_BEQ_PAIR_BOUNDED_PASS" || proof.NativeCommits <= 0 || proof.WholeFunctionVerified || proof.AutonomousPromotionAllowed || !proof.CompleteMachineStateEqual || !proof.OrderedIRQBusEqual || !proof.RollbackMachineEqual {
			return errors.New("untrusted or incomplete pair live proof")
		}
		if _, err = os.Stat(filepath.Join(dest, "cpu-b-bounded-edge-audit.json")); err != nil {
			return fmt.Errorf("CPU-B edge audit missing: %w", err)
		}
	}
	for i := range p.Candidates {
		c := &p.Candidates[i]
		if c.CPU != "B" {
			continue
		}
		switch c.CandidateEntry {
		case "0x0004DE":
			foundSingle = true
			if single {
				c.Stage = "LIVE_VERIFIED_BOUNDED_BEQ_INSTRUCTION"
				c.LiveNativeVerified = true
				c.Blocker = "17,892-class bounded BEQ replay proved independently; enclosing function CFG, callers, exits, IRQ/scheduling composition NOT certified"
			}
		case "0x0004D8":
			foundPair = true
			if pair {
				c.Stage = "LIVE_VERIFIED_BOUNDED_TWO_INSTRUCTIONS"
				c.LiveNativeVerified = true
				c.Blocker = "TST.W + BEQ.B genuine original/native/rollback parity and bounded original-ROM edges proven; enclosing CPU-B function CFG/callers/exits/scheduling NOT certified"
			} else if single {
				c.Stage = "STATIC_GHIDRA_GENERATED_ABI_NOT_LIVE"
				c.LiveNativeVerified = false
				c.Blocker = "Ghidra-generated two-opcode ABI only; live pair not run; no full function promotion"
			}
		default:
			c.Blocker = "Runtime-observed original-ROM site only; verified 0x4D8 pair/0x4DE BEQ does NOT promote this different candidate or its enclosing function"
		}
		c.PromotionAllowed = false
	}
	if (single && !foundSingle) || (pair && !foundPair) {
		return errors.New("verified CPU-B candidate missing from portfolio")
	}
	if err := rc634WritePortfolio(root, dest, *p); err != nil {
		return err
	}
	// Guard the serialized output against future stale-workflow regressions.
	b, err := os.ReadFile(filepath.Join(dest, "candidate-portfolio.json"))
	if err != nil {
		return err
	}
	var check rc634Portfolio
	if err = json.Unmarshal(b, &check); err != nil {
		return err
	}
	if check.GeneralAutonomousPromotionAllowed {
		return errors.New("unsafe autonomous promotion in current portfolio")
	}
	for _, c := range check.Candidates {
		if c.PromotionAllowed {
			return fmt.Errorf("unsafe candidate promotion: %s", c.ID)
		}
		if c.CPU == "B" && c.CandidateEntry == "0x0004D8" && pair && (!c.LiveNativeVerified || c.Stage != "LIVE_VERIFIED_BOUNDED_TWO_INSTRUCTIONS") {
			return errors.New("stale two-instruction current portfolio")
		}
	}
	return nil
}
