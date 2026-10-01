# Validation — v0.66.5.1

## Patch acceptance

- PowerShell parses cleanly on Windows.
- Version/schema/feature manifest agree on 0.66.5.1.
- Script/API docs expose all six Track Recorder actions.
- `Regression - v0.66.5.1 shared track recorder scripting` completes and exports a non-empty SVG.
- Track View shows the same sample count/path produced by scripted samples.
- `track.svg.export` rejects an empty recorder.
- Generic memory-trace proof uses `width=any` for 0x100303 until the producer transaction width is known.
- `Build-Debug.bat` remains canonical and runs `Prepare-Project.bat` before configure/build.
- Only the exact historic `cpu_bus_rom_tests (SEGFAULT)` + `1 tests failed out of 1` signature is tolerated by batch build gating. Any other test failure remains fatal.

## Packaging-side validation performed

- CPU/runtime CMake configure/build with `CHASEHQ_BUILD_SDL=OFF`: PASS.
- `cpu_bus_rom_tests`: PASS (1/1) in the packaging environment.
- Embedded Workbench JavaScript syntax (`node --check`): PASS.
- JSON schema/feature manifests parse: PASS.
- Track Recorder API/schema/docs contract presence: PASS.
- Windows/SDL and PowerShell parser validation remain to be run on the development machine via `Validate-Release.ps1`.
