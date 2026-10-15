package main

import (
	"archive/zip"
	"encoding/json"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func sampleZIP(t *testing.T) string {
	t.Helper()
	p := filepath.Join(t.TempDir(), "sample.zip")
	f, e := os.Create(p)
	if e != nil {
		t.Fatal(e)
	}
	z := zip.NewWriter(f)
	w, e := z.Create("evidence/analysis/chq-ghidra-findings.json")
	if e != nil {
		t.Fatal(e)
	}
	_, e = w.Write([]byte(`{"schema":"chq-ghidra-static-findings-v1","cpu":"A","rawRomSha256":"` + strings.Repeat("a", 64) + `","sites":[{"pc":"0x00820C","functionCandidate":{"name":"FUN_000081c4","entry":"0x0081C4","bodyMin":"0x0081C4","bodyMaxInclusive":"0x008211"},"nearbyInstructions":[{"pc":"0x00820C","text":"move.w (A0),(-0x7d00,A5)","length":4}]}]}`))
	if e != nil {
		t.Fatal(e)
	}
	if e = z.Close(); e != nil {
		t.Fatal(e)
	}
	if e = f.Close(); e != nil {
		t.Fatal(e)
	}
	return p
}
func TestBridgeFailClosedAndLiveCensus(t *testing.T) {
	z := sampleZIP(t)
	out := filepath.Join(t.TempDir(), "bridge.json")
	if e := bridge(z, "", out); e != nil {
		t.Fatal(e)
	}
	var x struct {
		Promotion string `json:"promotion"`
		Contracts []struct {
			Ready      bool `json:"readyForGeneration"`
			Promotable bool `json:"nativePromotionAllowed"`
		} `json:"contracts"`
		Coverage struct {
			Rows []struct {
				Hits int `json:"dynamicExecutionsWithinWindow"`
			} `json:"routines"`
		} `json:"coverageLedger"`
	}
	b, _ := os.ReadFile(out)
	if e := json.Unmarshal(b, &x); e != nil {
		t.Fatal(e)
	}
	if x.Promotion != "BLOCKED" || len(x.Contracts) != 1 || x.Contracts[0].Ready || x.Contracts[0].Promotable {
		t.Fatalf("unsafe promotion: %+v", x)
	}
	census := filepath.Join(t.TempDir(), "census.json")
	os.WriteFile(census, []byte(`[{"cpu":"A","pc":"0x00820C","romSha256":"`+strings.Repeat("a", 64)+`","hits":7}]`), 0644)
	if e := bridge(z, census, out); e != nil {
		t.Fatal(e)
	}
	b, _ = os.ReadFile(out)
	if e := json.Unmarshal(b, &x); e != nil {
		t.Fatal(e)
	}
	if x.Coverage.Rows[0].Hits != 7 {
		t.Fatalf("expected 7 matched hits, got %d", x.Coverage.Rows[0].Hits)
	}
	os.WriteFile(census, []byte(`[{"cpu":"B","pc":"0x00820C","romSha256":"`+strings.Repeat("a", 64)+`","hits":7}]`), 0644)
	if e := bridge(z, census, out); e == nil {
		t.Fatal("cross-CPU runtime evidence was accepted")
	}
}
