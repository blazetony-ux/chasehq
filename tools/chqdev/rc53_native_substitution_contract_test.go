package main

import (
 "crypto/sha256"
 "encoding/hex"
 "encoding/json"
 "os"
 "path/filepath"
 "strings"
 "testing"
)

func TestRC53NativeClearSafetyContract(t *testing.T) {
 root:=filepath.Join("..","..")
 get:=func(path string)string{b,e:=os.ReadFile(filepath.Join(root,filepath.FromSlash(path)));if e!=nil{t.Fatal(e)};return string(b)}
 cpp:=get("src/runtime.cpp")
 for _,s:=range []string{"CHQ_NATIVE_SPRITE_CLEAR_EXPERIMENT_ACK","YES_UNQUALIFIED","CHQ_NATIVE_SPRITE_TRANSFER_SHADOW","process_native_clear_instruction", "m68ki_remaining_cycles", "native_clear_disabled_", "native_clear_rollback_", "native_clear_report_json"}{if !strings.Contains(cpp,s){t.Errorf("source missing native guard %s",s)}}
 header:=get("src/native_sprite_clear.cpp")
 for _,s:=range []string{"rom-fingerprint","bulk-input-output-mismatch","clear-out-of-range","destination-pointer","dbra-count-register", "emulated_cycles=34u*count+4u"}{if !strings.Contains(header,s){t.Errorf("independent native plan missing %s",s)}}
 ps:=get("tools/Run-RC5.3-Native-Clear-Equivalence.ps1")
 for _,s:=range []string{"foreach ($entry in $loaded)","Nine", "Get-FileHash", "--rom-census", "nativeLoopSkips", "RAM PARITY MISMATCH", "CPU STATE/TIMING MISMATCH", "OBSERVED_12_CASE_EQUIVALENCE"}{if !strings.Contains(ps,s){t.Errorf("missing corpus check %s",s)}}
 // RC5.2 repeated the WRONG synthetic DBRA displacement and lost all
 // instruction-time destination context. Pin the actual Run 005 -6 branch.
 if !strings.Contains(header, "peek(0x40172u,2)!=0xFFFAu") {t.Fatal("wrong ROM DBRA displacement: must use real 0xFFFA") }
 if strings.Contains(header, "peek(0x40172u,2)!=0xFFF8u") {t.Fatal("obsolete RC5.2 ROM fingerprint guard") }
 for _,needle:=range []string{"active_sprite_records()", "active_sprite_stage_pointer()", "lastGuardObservation", "last_expected_a1_", "last_current_records_"}{if !strings.Contains(cpp,needle){t.Errorf("instruction-time guard diagnostic missing: %s",needle)}}
 if !strings.Contains(ps,"$results.ToArray() | Select-Object") {t.Fatal("RC5.2 PowerShell 5.1 CSV List<T> crash has not been corrected") }
 if strings.Contains(ps,"@($results) | Export-Csv") {t.Fatal("RC5.2 PowerShell 5.1 CSV crash pattern retained") }
 // Musashi fetches the redirected RTS in the SAME hook iteration. Require
 // full native output verification before mutating 68000 architectural state.
 comparison:=strings.Index(cpp,"live_candidate_shadow_.instruction(false,0x40174")
 redirect:=strings.Index(cpp,"m68k_set_reg(M68K_REG_PC,plan.return_pc)")
 if comparison<0||redirect<0||comparison>=redirect{t.Fatal("native sprite clear must compare full RAM BEFORE redirecting Musashi PC")}
 if strings.Contains(cpp,"native_clear_in_flight_"){t.Fatal("obsolete deferred 0x40174 hook must not be used")}
 if strings.Contains(ps,"@(Get-Content -LiteralPath $manifestPath") {t.Fatal("PowerShell 5.1 root-array parsing regression")}
 for _,ch:=range []byte(ps){if ch>127{t.Fatal("non ASCII PowerShell 5.1 script")}}
}

func TestRC53AllHarvestedCheckpointsIntegrity(t *testing.T) {
 root:=filepath.Join("..","..")
 type entry struct{File string `json:"file"`;Sha string `json:"sha256"`;Bytes int64 `json:"bytes"`;Run string `json:"source_run"`}
 b,e:=os.ReadFile(filepath.Join(root,"shadow-harvest","manifest.json"));if e!=nil{t.Fatal(e)}
 var rows []entry;if e=json.Unmarshal(b,&rows);e!=nil{t.Fatal(e)}
 if len(rows)!=9{t.Fatalf("expected exactly 9 original checkpoints, got %d",len(rows))}
 for _,r:=range rows {
  if r.Run==""||r.File==""||r.Sha==""{t.Fatalf("missing provenance: %+v",r)}
  b,e=os.ReadFile(filepath.Join(root,"shadow-harvest","checkpoints",r.File));if e!=nil{t.Fatal(e)}
  if int64(len(b))!=r.Bytes{t.Fatalf("size mismatch: %s",r.File)}
  h:=sha256.Sum256(b);if hex.EncodeToString(h[:])!=r.Sha{t.Fatalf("SHA mismatch: %s",r.File)}
 }
}
