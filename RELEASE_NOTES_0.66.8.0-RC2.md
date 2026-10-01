# ChaseHQ-Native v0.66.8.0 RC2

RC2 is a focused integration hotfix over RC1. Native Forensic Timeline v1 semantics are unchanged.

## Fixed

- Script Library now auto-loads the selected script at initial discovery and after category/version/scope/search filtering, preventing stale editor text from being run under a different visible selection. Frontend bootstrap requires a real selected script load before READY.
- Long `control.run-frames` operations are progress-aware instead of failing at a fixed 15-second threshold. Browser/front-proxy operation budgets were widened to permit heavy per-frame timeline capture.
- Turbo capture advances its 240-frame tail as four 60-frame chunks for clearer progress and additional resilience.
- Restart browser bootstrap uses a unique query URL, clears stale frontend-error diagnostics for the new Workbench instance and waits up to 20 seconds.
- Build-Debug now owns one coherent `[1/6]..[6/6]` sequence; Prepare-Project prints subordinate unnumbered steps.
- Added a focused RC2 long-run regression and a recovery script for the already-captured RC1 turbo timeline.

## Important evidence recovered from RC1

The failed browser run did not lose the turbo recording. The uploaded RC1 dataset is complete: frames 2064..2319 (256 frame checkpoints), 711,374 bus-write records, turbo-active rises at frame 2065 and falls at frame 2275. Do not replay gameplay simply to recreate this event.
