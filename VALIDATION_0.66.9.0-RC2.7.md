# RC2.7 validation

| Check | Result |
|---|---|
| TC0100SCN geometry / signed X / modulo512 / rowscroll raster isolation / displacement / Y preservation / column zero detector | PASS, GCC on Linux |
| sprite_gfx_tests, all16 pens | PASS, GCC on Linux |
| sprite_priority_tests | PASS, GCC on Linux |
| cpu_bus_rom_tests (full runtime_tests target with generated Musashi core) | PASS, GCC on Linux, exit0 |
| Workbench Current Session/full version/run identity Node test | PASS |
| Workbench SDL bindings/toggle state/snapshot route Node test | PASS; simulated transport, not GUI proof |
| Main.cpp and changed video methods C++ syntax | PASS, GCC using actual project declarations; no SDL linking |
| Browser JavaScript syntax | PASS, node --check |
| PowerShell parser/StrictMode and authoritative_history_tests | Not run: PowerShell unavailable here; bundled local gate |
| Windows/SDL build and cli_evidence_tests | Not run here; Build-Debug stages local inputs and runs CTest |
| Focused RC2.7 and Full Regression | Not run here; require local Workbench/native runtime |
| Canonical visual old/corrected inspection | Pending local run |

The historical Windows cpu_bus_rom_tests SEGFAULT remains a platform-specific known baseline issue; this Linux PASS does not establish that it has been fixed on Windows. Existing build scripts retain their narrowly documented handling.

Local order: Build-Debug.bat → Start-ChaseHQ.ps1 -Restart → focused RC2.7 script → preserved RC2.6.1 sprite script → Full Regression COMPLETE. Keep bundles from each run. Require exact=true,verifiedExact=true,mismatchedPixels=0 and inspect road/background/sprite/HUD relationships. No screenshot preference alone proves hardware accuracy.

Source-only candidate delivery is authorised. A Windows promotion report should record the actual full build label, CTest results, session/run keys, manifests, canonical comparisons and unresolved defects before replacing this proof status.
