// Native evidence bridge (M1->M2/M3). Never patches the emulator or promotes code.
package main

import (
	"archive/zip"
	"crypto/sha256"
	"encoding/hex"
	"encoding/json"
	"errors"
	"flag"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"strings"
)

type instruction struct {
	PC     string `json:"pc"`
	Text   string `json:"text"`
	Length int    `json:"length"`
}
type fn struct {
	Name    string `json:"name"`
	Entry   string `json:"entry"`
	BodyMin string `json:"bodyMin"`
	BodyMax string `json:"bodyMaxInclusive"`
}
type site struct {
	PC           string                   `json:"pc"`
	Instruction  string                   `json:"instruction"`
	Function     *fn                      `json:"functionCandidate"`
	Instructions []instruction            `json:"nearbyInstructions"`
	Incoming     []map[string]interface{} `json:"incomingReferences"`
	Outgoing     []map[string]interface{} `json:"outgoingReferences"`
}
type findings struct {
	Schema string `json:"schema"`
	CPU    string `json:"cpu"`
	ROM    string `json:"rawRomSha256"`
	Sites  []site `json:"sites"`
}
type hit struct {
	CPU  string `json:"cpu"`
	PC   string `json:"pc"`
	ROM  string `json:"romSha256"`
	Hits int    `json:"hits"`
}

func bridge(zipPath, runtimePath, out string) error {
	z, e := zip.OpenReader(zipPath)
	if e != nil {
		return e
	}
	defer z.Close()
	var raw []byte
	for _, f := range z.File {
		if strings.HasSuffix(strings.ReplaceAll(f.Name, "\\", "/"), "/analysis/chq-ghidra-findings.json") {
			r, e := f.Open()
			if e != nil {
				return e
			}
			raw, e = func() ([]byte, error) { defer r.Close(); return io.ReadAll(r) }()
			if e != nil {
				return e
			}
			break
		}
	}
	if len(raw) == 0 {
		return errors.New("real Ghidra findings missing from ZIP")
	}
	var src findings
	if e = json.Unmarshal(raw, &src); e != nil {
		return e
	}
	if src.Schema != "chq-ghidra-static-findings-v1" || (src.CPU != "A" && src.CPU != "B") || len(src.ROM) != 64 {
		return errors.New("invalid Ghidra provenance")
	}
	if _, e = hex.DecodeString(src.ROM); e != nil {
		return errors.New("invalid ROM sha256")
	}
	observations := map[string]int{}
	runtimeProvided := runtimePath != ""
	if runtimeProvided {
		b, e := os.ReadFile(runtimePath)
		if e != nil {
			return e
		}
		var hits []hit
		if e = json.Unmarshal(b, &hits); e != nil {
			return fmt.Errorf("runtime census must be JSON array: %w", e)
		}
		for _, h := range hits {
			if h.Hits < 0 || h.CPU != src.CPU || !strings.EqualFold(h.ROM, src.ROM) {
				return fmt.Errorf("invalid runtime provenance/hits for %s", h.PC)
			}
			p := strings.ToUpper(h.PC)
			if _, ok := observations[p]; ok {
				return fmt.Errorf("duplicate runtime PC %s", p)
			}
			observations[p] = h.Hits
		}
	}
	var rows []map[string]interface{}
	var contracts []map[string]interface{}
	for _, s := range src.Sites {
		if s.Function == nil {
			continue
		}
		unique := map[string]bool{}
		executed := 0
		hits := 0
		ins := []instruction{}
		for _, v := range s.Instructions {
			if !strings.HasPrefix(strings.ToLower(v.PC), "0x") || v.Length <= 0 {
				return fmt.Errorf("invalid instruction in %s", s.PC)
			}
			if !unique[strings.ToUpper(v.PC)] {
				unique[strings.ToUpper(v.PC)] = true
				ins = append(ins, v)
				if n, ok := observations[strings.ToUpper(v.PC)]; ok {
					executed++
					hits += n
				}
			}
		}
		rows = append(rows, map[string]interface{}{"cpu": src.CPU, "pc": s.PC, "functionCandidate": s.Function, "romSha256": src.ROM, "staticWindowInstructionCount": len(unique), "runtimeCensusProvided": runtimeProvided, "executedUniqueWithinWindow": executed, "dynamicExecutionsWithinWindow": hits, "coverageFraction": nil, "confidence": "hypothesis; local Ghidra window is not complete function coverage"})
		contracts = append(contracts, map[string]interface{}{"routineId": src.CPU + ":" + s.PC, "romSha256": src.ROM, "functionCandidate": s.Function, "instructions": ins, "incomingReferences": s.Incoming, "outgoingReferences": s.Outgoing, "inputs": []string{}, "outputs": []string{}, "registerEffects": []string{}, "memoryEffects": []string{}, "calls": []string{}, "cyclePolicy": "unknown", "unresolvedDependencies": []string{"independent CPU flags/register liveness", "complete function CFG", "runtime input/output and memory census", "bus and cycle equivalence"}, "reviewed": false, "readyForGeneration": false, "nativePromotionAllowed": false, "confidence": "hypothesis; static Ghidra data only"})
	}
	if len(rows) == 0 {
		return errors.New("no function-bearing Ghidra candidates")
	}
	digest := sha256.Sum256(raw)
	result := map[string]interface{}{"schema": "chq-native-evidence-bridge-v1", "sourceGhidraSHA256": fmt.Sprintf("%x", digest), "cpu": src.CPU, "romSha256": src.ROM, "coverageLedger": map[string]interface{}{"schema": "chq-native-coverage-ledger-v1", "routines": rows, "globalDenominatorKnown": false, "completionPercent": nil}, "contracts": contracts, "nativeSubstitution": false, "promotion": "BLOCKED"}
	if e := os.MkdirAll(filepath.Dir(out), 0755); e != nil {
		return e
	}
	b, e := json.MarshalIndent(result, "", "  ")
	if e != nil {
		return e
	}
	return os.WriteFile(out, append(b, '\n'), 0644)
}
func main() {
	input := flag.String("ghidra-zip", "", "Ghidra evidence ZIP")
	runtime := flag.String("runtime-census", "", "optional JSON array of CPU/PC/ROM SHA/hits")
	out := flag.String("out", "", "output JSON")
	flag.Parse()
	if *input == "" || *out == "" {
		fmt.Fprintln(os.Stderr, "usage: native-bridge --ghidra-zip evidence.zip --out bridge.json [--runtime-census census.json]")
		os.Exit(2)
	}
	if e := bridge(*input, *runtime, *out); e != nil {
		fmt.Fprintln(os.Stderr, e)
		os.Exit(1)
	}
}
