package main

// A current, cross-stage reconciliation record. Earlier per-stage proof JSON
// files are immutable evidence of what was known *at that stage*, not a source
// for the latest portfolio status. This file is generated only after all
// requested stages run and never labels offline C++ generation as live proof.
import (
	"encoding/json"
	"fmt"
	"os"
	"path/filepath"
)

type rc6312CurrentMaturity struct {
	Schema                              string   `json:"schema"`
	Status                              string   `json:"status"`
	CPUBBEQ                             string   `json:"cpuBBEQ"`
	CPUBPair                            string   `json:"cpuBTwoInstructionPair"`
	CPUBSingleLiveCommits               int64    `json:"cpuBSingleLiveNativeCommits"`
	CPUBLiveIRQEqual                    bool     `json:"cpuBSingleLiveIRQParity"`
	CPUBPairConsecutiveOriginalEdges    int64    `json:"cpuBPairConsecutiveOriginalEdges"`
	CPUAHealthAndCopy                   string   `json:"cpuAHealthAndCopy"`
	EnclosingFunctionBoundariesVerified bool     `json:"enclosingFunctionBoundariesVerified"`
	AutonomousPromotionAllowed          bool     `json:"autonomousPromotionAllowed"`
	FullGoRegressionQualified           bool     `json:"fullGoRegressionQualified"`
	Limitations                         []string `json:"limitations"`
}

func rc6312WriteCurrentMaturity(dest string) (rc6312CurrentMaturity, error) {
	v := rc6312CurrentMaturity{Schema: "chq-rc6312-cross-stage-maturity-v1", Status: "IN_PROGRESS_NOT_AUTONOMOUS", CPUBBEQ: "NOT_RUN", CPUBPair: "NOT_RUN", CPUAHealthAndCopy: "SEE_INDEPENDENT_M5_EVIDENCE", Limitations: []string{"CPU-B pair native live verification not run in this evidence; original ROM remains authoritative", "CPU-A/B enclosing function boundaries and complete caller coverage are not signed off", "Structural Full Regression corpus is guarded at 1176..1536 commands; Windows Workbench runtime Full Regression has not been qualified by native reconstruct"}}
	liveData, e := os.ReadFile(filepath.Join(dest, "cpu-b-live-proof.json"))
	if e == nil {
		var p rc6310Proof
		if e = json.Unmarshal(liveData, &p); e != nil {
			return v, e
		}
		if p.Status != "LIVE_GHIDRA_CPU_B_BEQ_BOUNDED_PASS" || !p.GhidraGeneratedDLL || !p.FullCPUStateAndCyclesEqual || !p.OrderedIRQBusEqual || !p.CompleteMachineStateEqual || !p.RollbackMachineEqual || p.AutonomousPromotionAllowed || p.WholeFunctionVerified {
			return v, fmt.Errorf("cannot reconcile unverified CPU-B BEQ live status: %s", p.Status)
		}
		v.CPUBBEQ = "LIVE_VERIFIED_BOUNDED_ONE_INSTRUCTION"
		v.CPUBSingleLiveCommits = p.NativeCommits
		v.CPUBLiveIRQEqual = true
	}
	pairData, e := os.ReadFile(filepath.Join(dest, "cpu-b-two-instruction-proof.json"))
	if e == nil {
		var p rc6312PairProof
		if e = json.Unmarshal(pairData, &p); e != nil {
			return v, e
		}
		if p.Status != "CPU_B_TWO_INSTRUCTION_GHIDRA_ABI_PASS_NOT_LIVE" || p.PromotionAllowed || p.LivePairSubstitutionProven || p.OriginalConsecutiveEdges < 1 {
			return v, fmt.Errorf("cannot reconcile unverified CPU-B pair: %s", p.Status)
		}
		v.CPUBPair = "STATIC_GHIDRA_GENERATED_ABI_NOT_LIVE"
		v.CPUBPairConsecutiveOriginalEdges = p.OriginalConsecutiveEdges
	}
	if live, e := os.ReadFile(filepath.Join(dest, "cpu-b-pair-live-proof.json")); e == nil {
		var p rc6313PairLiveProof
		if e = json.Unmarshal(live, &p); e != nil {
			return v, e
		}
		if p.Status != "LIVE_GHIDRA_CPU_B_TST_BEQ_PAIR_BOUNDED_PASS" || !p.ReturnRegistersFlagsCyclesEqual || !p.MappedR16BusEqual || !p.OrderedIRQBusEqual || !p.CompleteMachineStateEqual || !p.RollbackMachineEqual || p.WholeFunctionVerified || p.AutonomousPromotionAllowed {
			return v, fmt.Errorf("CPU-B generated two-opcode pair not live verified: %s", p.Status)
		}
		v.CPUBPair = "LIVE_VERIFIED_BOUNDED_TWO_INSTRUCTIONS"
		v.Limitations[0] = "Two-instruction CPU-B pair VERIFIED only in bounded real ROM replay; enclosing function, caller/CFG and other contexts unreviewed"
	}
	if _, e := os.Stat(filepath.Join(dest, "live-copy-proof.json")); e == nil {
		v.CPUAHealthAndCopy = "LIVE_VERIFIED_BOUNDED_CPU_A_CANDIDATES"
	}
	b, e := json.MarshalIndent(v, "", "  ")
	if e != nil {
		return v, e
	}
	e = os.WriteFile(filepath.Join(dest, "native-maturity-current.json"), append(b, '\n'), 0644)
	return v, e
}
