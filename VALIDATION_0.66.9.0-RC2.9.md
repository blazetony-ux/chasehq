# Validation — v0.66.9.0-RC2.9

## Candidate validation completed in this packaging environment

RC2.9 was recovered from the preserved source working tree after the earlier staging/package interruption. The stale package produced by that interrupted attempt was discarded and is not the release artifact.

### Static/source consistency

- **PASS — 36/36 final static consistency checks after the Windows preparation correction.**
- Version reports `0.66.9.0-RC2.9`.
- Native one-hit target path, CLI/config/debugger/script/Experimental-tab parity present.
- Pause/Break is the SDL pause/resume shortcut; it does not alias F10.
- SDL title exposes PAUSED/RUNNING state.
- Dashboard uses the wide-screen 7/5 SDL-preview/control layout with responsive collapse below 1200 px.
- `control.run-frames` timeout contract is configurable up to 3,600,000 ms with RC2.9 default 900,000 ms; transport budgets are 3,700 s at the front proxy and 3,700,000 ms in the browser so the maximum contract is reachable.
- Focused RC2.9 regression is present and included in Full Regression.
- All four new target checkpoints and metadata are present, with frame 6333 documented as the preferred target-damage anchor.
- Gameplay Registry/knowledge/docs describe `0x1002AE` as the causally confirmed target remaining-hit/damage counter.
- All 43 shipped JSON files checked outside generated build/evidence directories parsed successfully.
- Current documentation contains no stale F10-fallback or rejected/UNKNOWN `0x1002AE` target-health wording.

### Native build/tests available in this environment

- **PASS — Linux native RC2.9 build.**
- **PASS — CTest 4/4:**
  - `tc0100scn_geometry_tests`
  - `sprite_priority_tests`
  - `sprite_gfx_tests`
  - `cpu_bus_rom_tests`
- **PASS — direct runtime test**, including `[test] RC2.9 one-hit target option parsing`.
- **PASS — `tests/workbench_history_tests.js`.**
- **PASS — `tests/workbench_sdl_controls_tests.js`.**

These checks prove the portable/native source and JavaScript portions that can execute here. This environment does **not** provide Windows PowerShell/SDL runtime prove-off, and ROMs are intentionally not bundled.

## Required Windows/SDL promotion gate

RC2.8 remains the current Windows/SDL-proven validation baseline until RC2.9 completes this gate:

1. Run `Build-Debug.bat` using the normal local SDL/ROM staging.
2. Run `research/scripts/regression/regression-v06690-rc29-target-health-tooling.chqscript`.
3. Verify one-hit mode from `stage1-target-immediate-pre-damage-6333.chqstate` produces `0x1002AE=FFFF`, enters the authentic terminal/defeat path, and cleans up with one-hit mode OFF.
4. Verify Pause/Break toggles pause/resume directly in the SDL window and the SDL title reflects PAUSED/RUNNING.
5. Verify Dashboard SDL controls sit beside the live preview at wide desktop widths and collapse below it on narrower widths.
6. Run **Full Regression** to COMPLETE with no unexpected ERROR/FAIL.

Do not promote RC2.9 to Windows/SDL-proven until that evidence is captured.

## Windows preparation correction

The first distributed pre-proof RC2.9 archive failed during `Prepare-Project` because `Validate-Workbench.ps1` still expected the historical `Wait-ChqPaused(... timeoutMs=300000 ...)` signature even though the shipped Workbench implementation used the RC2.9 900000 ms progress-aware default. The corrected archive updates the validator and adds explicit RC2.9 guards for checkpoint, one-hit target, SDL UX, timeout and documentation parity. The earlier archive/hash is withdrawn.

## Windows build-gate correction discovered during prove-off

The first Windows `Build-Debug.bat` run after the preparation fix compiled successfully but reported two CTest failures: the documented `cpu_bus_rom_tests (SEGFAULT)` plus `cli_evidence_tests`, whose manifest assertion still expected `0.66.9.0-RC2.7`. The wrapper then incorrectly returned success because `%ERRORLEVEL%` was expanded before the two `findstr` commands inside the parenthesized failure block.

The corrected candidate therefore requires all of the following before handoff:

- `tests/cli_evidence_tests.ps1` must assert build identity `0.66.9.0-RC2.9`;
- Debug and Release wrappers must capture the `findstr` status with delayed `!ERRORLEVEL!`;
- the tolerated historical CPU segfault path is legal only when CTest reports exactly **one** failed test;
- an additional failing CTest must return a non-zero build status.

This correction has not yet been Windows-proven; rerun `Build-Debug.bat` against the replacement archive.
