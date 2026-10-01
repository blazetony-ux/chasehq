# Validation - v0.66.7.9 Research Recovery / Release Discipline

## Candidate gate

1. Extract the clean candidate alongside previous builds.
2. Run `Build-Debug.bat` only. Do not insert an alternate validation/build path.
3. Launch with `Start-ChaseHQ.ps1 -Restart` and require native API, Workbench and browser bootstrap READY.
4. Run **Recovery / Fast Release Gate**.
5. Run **Regression - v0.66.7.9 Research Recovery**.
6. Run **Full Regression Suite** once.
7. Upload the final Full Regression `bundle.zip` for evidence review.

## Expected release behavior

- No full SDL source tree is carried in the developer handoff ZIP; local SDL comes from `build-local.json`.
- No ROM payload, `out`, transient evidence, backup files or patch helpers are packaged.
- Every PowerShell file parses before handoff and the same validators pass through `Build-Debug.bat`.
- Script execution creates one parent run and one final bundle only.
- Transient predicates/preflight calls do not allocate incidental runs.
- Browser lifecycle ends RUNNING -> FINALIZING -> COMPLETE/DONE.
- Recent Script Runs and Graphics Lab can rediscover completed-run data without stalling the Workbench backend.

## Research recovery validation

The machine-readable turbo/HUD knowledge entry must state that processed turbo input, active state, remaining count and timer are confirmed, while HUD rendering/source/composition remains open. The new turbo and recovery scripts must appear in Script Console with metadata from their headers.

## Promotion rule

If any candidate requires a manual local source patch, do not promote that hand-edited tree. Fold the fix into a new clean candidate archive and run the gate against the clean candidate.

## Final proof result — 2026-09-28

**PASS / PROMOTED.** The clean v0.66.7.9 candidate completed the canonical Windows/SDL path successfully: build, launcher/browser bootstrap, Recovery / Fast Release Gate, v0.66.7.9 Research Recovery regression, and Full Regression Suite. Final full-regression metadata reported build `0.66.7.9`, `ok=true`, `status=PASS`, duration `169876 ms`, and the console ended with `=== ChaseHQ Full Regression: COMPLETE ===`.

v0.66.7.9 is therefore the current proven baseline. The next substantive task is the turbo HUD source/compositor investigation, not further release-process work.
