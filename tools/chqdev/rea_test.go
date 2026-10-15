package main

import (
	"encoding/json"
	"strings"
	"testing"
)

func TestREACrosswalkRequiresMappedROM(t *testing.T) {
	census := []byte("cpu\tpc\tinstructionHits\nCPU-A\t0x00A112\t9\nCPU-A\t0x04351C\t5\n")
	in := `{"schema":"chq-rea-static-export-v1","cpu":"A","romSha256":"` + strings.Repeat("a", 64) + `","imageBase":"0x100000","functions":[{"name":"target_health_candidate","start":"0x10A110","end":"0x10A120","confidence":"static-observed"}]}`
	out, e := crosswalkRea([]byte(in), census)
	if e != nil {
		t.Fatal(e)
	}
	if !strings.Contains(string(out), `"observedInstructionHits": 9`) {
		t.Fatal(string(out))
	}
	for _, bad := range []string{strings.Replace(in, "0x100000", "0x0", 1), strings.Replace(in, "0x10A120", "0x180002", 1), strings.Replace(in, "static-observed", "verified", 1)} {
		if _, e := crosswalkRea([]byte(bad), census); e == nil {
			t.Fatal("invalid mapping passed")
		}
	}
}

func TestREAObservedPCsSortedAndPerFunction(t *testing.T) {
	in := `{"schema":"chq-rea-static-export-v1","cpu":"A","romSha256":"` + strings.Repeat("c", 64) + `","imageBase":"0x100000","functions":[{"name":"routine_01","start":"0x1434EE","end":"0x143510","confidence":"hypothesis"}]}`
	csv := []byte("cpu\tpc\tinstructionHits\nCPU-A\t0x043500\t7\nCPU-B\t0x003500\t99\nCPU-A\t0x0434EE\t9\nCPU-A\t0x043510\t5\n")
	b, e := crosswalkRea([]byte(in), csv)
	if e != nil {
		t.Fatal(e)
	}
	var result struct {
		Matches []struct {
			ObservedPCs          []reaPC `json:"observedPCs"`
			ObservedInstructions uint64  `json:"observedInstructionHits"`
		} `json:"matches"`
	}
	if e = json.Unmarshal(b, &result); e != nil {
		t.Fatal(e)
	}
	if len(result.Matches) != 1 || result.Matches[0].ObservedInstructions != 16 || len(result.Matches[0].ObservedPCs) != 2 || result.Matches[0].ObservedPCs[0].PC != "0x0434EE" {
		t.Fatalf("unexpected observed PCs: %s", b)
	}
}
