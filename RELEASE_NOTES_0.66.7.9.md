# ChaseHQ-Native v0.66.7.9 — Proven Release

Status: **PROVEN** on Windows/SDL, 2026-09-28.

## Release proof

- `Build-Debug.bat` completed through the canonical dependency/validation/build path.
- `Start-ChaseHQ.ps1 -Restart` reached native API, Workbench and browser bootstrap READY.
- `Recovery / Fast Release Gate` passed.
- `Regression - v0.66.7.9 Research Recovery` passed.
- `Full Regression Suite` passed with build `0.66.7.9`, `ok=true`, `status=PASS`; final marker `=== ChaseHQ Full Regression: COMPLETE ===`.

## What this release establishes

- Release/prove-off failures from the v0.66.7.6-v0.66.7.8 cycle are documented and converted into permanent validation/process rules.
- Script execution uses one parent run and one final bundle; transient predicate/preflight calls do not create incidental runs.
- Completed run history and structured snapshots are rediscoverable from disk without unbounded recursive scans.
- Developer handoff packages use `build-local.json` and exclude bundled SDL source, ROMs, `out`, transient evidence and patch/back-up files.
- Turbo gameplay state is already confirmed; active research now returns to the bottom-HUD turbo indicator source/compositor path.

## Next research

Run `research/scripts/graphics/turbo-hud-before-during-after.chqscript`, then use the causal-state and writer-trace scripts only as needed.
