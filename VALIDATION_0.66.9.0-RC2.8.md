# RC2.8 validation

| Check | Result |
|---|---|
| TC0100SCN geometry tests, including text `+16`, control scroll and explicit flipped coordinates | PASS (GCC direct build/run) |
| Sprite 4bpp live vs independent SHADOW decoder parity test | PASS (GCC direct build/run; 1,024 pixels across four deterministic fixture tiles) |
| Sprite priority tests | PASS (GCC direct build/run) |
| Full runtime tests | Not run (CMake unavailable in this Linux environment) |
| Workbench JavaScript syntax and JSON payload validation | PASS (Node syntax check; Python validates every packaged JSON file) |
| Script catalog/full-suite wiring for RC2.8 regressions | PASS (all .chqscript files catalogued; focused checks included before COMPLETE) |
| PowerShell release validator and RC2.8 radar/HUD/end-state script | Pending Windows |
| Full Windows/SDL build using `Build-Debug.bat` with local `build-local.json` staging | Pending Windows |
| Full Regression terminal COMPLETE marker | Pending Windows |

RC2.7 baseline evidence: Run 009 completed successfully in 59.615 seconds on Windows/SDL, then the build was marked saturated. Its evidence bundle is the user's `bundle(20261001-105533).zip`; completion checkpoint and screenshot are embedded in that bundle. RC2.8 is a new candidate and does not inherit RC2.7's proof status.
