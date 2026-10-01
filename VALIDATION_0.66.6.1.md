# v0.66.6.1 validation record

## Completed in the build environment

- **CPU/runtime CMake configure + build:** PASS using `-DCHASEHQ_BUILD_SDL=OFF`.
- **CTest:** PASS, 1/1 (`cpu_bus_rom_tests`).
- **Workbench embedded JavaScript syntax:** PASS with Node `--check` after extraction from `Start-ChaseHQWeb.ps1`.
- **Machine-readable JSON contracts:** PASS parsing for `research/schema/api-actions.json`, `research/schema/script-language.json`, and `research/feature-manifest.json`; all report build `0.66.6.1`.
- **Version/schema consistency checks:** current launchers, Web Workbench, native version header and machine-readable contracts identify v0.66.6.1; HTTP schema is v1.9.

## Environment limitation

The preparation environment does not contain the SDL3 source tree expected by this project and does not have PowerShell. Therefore it cannot perform the authoritative Windows SDL/native compilation or PowerShell parser/runtime validation. A full SDL configure correctly stops with the project's existing message requesting SDL3 source.

## Required dev-machine proof

1. Build the normal Windows SDL target using the existing project build path.
2. Start the SDL instance and Research Workbench using the normal launcher.
3. Run `research/scripts/regression/regression-v06661-layered-frame-snapshot.chqscript` (or the full regression suite).
4. Confirm the run reports success and inspect the generated v2 snapshot in **Graphics Lab > Structured Frame Snapshot**.
5. Confirm Normal, HUD hidden, HUD only and Layer reconstruction views render, and that the self-compare operations report zero differing pixels in normal/hidden/only modes.

This validation requirement is intentional: the new native graphics export lives in the SDL renderer and should not be marked proven until it has run against the real Windows/SDL build.
