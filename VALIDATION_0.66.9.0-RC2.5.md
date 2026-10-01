# Validation - v0.66.9.0-RC2.5

RC2.5 packages a Windows-proven sprite correction plus new Workbench/Research Script lifecycle changes. The package itself therefore requires a fresh Windows gate.

1. Run `Build-Debug.bat` from the RC2.5 root. `BUILD COMPILE: PASS` is required. The exact historical `cpu_bus_rom_tests (SEGFAULT)` signature is allowed because it was independently reproduced against pristine RC2.4.4; any different test failure is not allowed.
2. Run `Test-SpritePriority.bat`; native synthetic ownership/coverage/visible-raster tests must pass.
3. Start with `Start-ChaseHQ.ps1 -Restart`. Workbench startup must succeed; schema/Script Console action parity checks must not throw.
4. In Script Console run `Regression - v0.66.9.0 RC2.5 Script Safety`. It must reach frame 2352 inside the timeline fork, execute `finally`, restore frame 2064 through `timeline.reset`, and end with `=== v0.66.9.0 RC2.5 Script Safety: COMPLETE ===`.
5. Run `Regression - Sprite Ownership and Export Coordinates`. Require frame 2352, front slot 82/map 475, back slot 81/map 575, nonzero overlap, nonzero front-owned overlap, nonzero shadow visibility outside the body, `exportCoordinatesExact=true`, and terminal `=== Sprite ownership and export coordinate assertions COMPLETE ===`.
6. Run `Full Regression Suite`. It must reach `=== ChaseHQ Full Regression: COMPLETE ===`; both RC2.5 script-safety and sprite-ownership regressions must execute before that marker.
7. Open Script Console history after a fresh Workbench start. It should default to the active session; switching to All sessions/current build must remain possible.
8. Verify `api schema action=timeline.reset` and `api schema action=frame.snapshot.assert-overlap` are both documented and executable through Script Console.
9. Run `Validate-Release.ps1` and `Validate-Workbench.ps1` if not already invoked by the standard build/release flow.
10. Only after the release gate passes, run `graphics/tc0100scn-rowscroll-raster-mapping.chqscript` to resume the sky/rowscroll investigation. Its outcome is research evidence, not a release-pass criterion.
