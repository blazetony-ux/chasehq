# ChaseHQ-Native v0.66.7.8 validation

Purpose: prove the v0.66.7.7 cross-version run/snapshot discovery fixes without reintroducing Workbench backend starvation.

1. Run `Build-Debug.bat`.
2. Launch with `Start-ChaseHQ.ps1 -Restart`.
3. Confirm `/api/v1/status`, `/api/v1/script/runs`, and `/api/v1/frame/snapshots` respond normally.
4. Confirm the previous v0.66.7.6 Full Regression appears in Recent Script Runs.
5. Confirm `regression-v06676-docs-consolidation` appears in Graphics Lab.
6. Run `Regression - v0.66.7.8 Discovery Performance`, then Full Regression.

Packaging: SDL source tree is intentionally excluded; local SDL/ROM dependency paths are supplied by build-local.json / local dependency staging.


## RC2 final gate
The focused `Regression - v0.66.7.8 Discovery Performance` has passed on Windows. RC2 includes the permanent Workbench parser-brace correction and launcher health identity fix. Run `Full Regression Suite` once against RC2; a clean bundle is the remaining release gate.

## RC3 - script run finalization / bundle performance fix

Full Regression prove-off of RC2 exposed a runner integration defect: each per-command POST to `/api/v1/script` rewrote `bundle-manifest.json` and recompressed `bundle.zip`. Long scripts therefore rebuilt the same growing ZIP hundreds of times. Predicate/preflight API calls could also create incidental tiny runs.

RC3 changes the run lifecycle so a browser script owns one server run directory for its whole execution. Command responses append output/artifacts and maintain RUNNING metadata without packaging. Preflight/query and predicate requests use transient execution and do not allocate runs. On PASS, FAIL or CANCELLED, the browser explicitly finalizes the parent run once; finalization writes the bundle manifest and creates exactly one `bundle.zip`.

Required prove-off:
- a multi-command regression creates one run directory only;
- no `bundle.zip` exists/rebuilds between individual commands;
- one bundle is created at finalization;
- `if memory` / `if capability` / query preflight do not create extra run directories;
- Full Regression completes without repeated Compress-Archive windows.
## RC4 - script runner completion-state fix

The focused RC3 run-finalization regression passed and proved one authoritative parent run / one final bundle. UI review then found the 250 ms RUNNING ticker could overwrite the visible COMPLETE state while final bundle/history work was still finishing, leaving `Script: RUNNING...` beside `Runner: DONE`. RC4 clears the ticker immediately after command execution, reports a distinct `FINALIZING` phase while the single run bundle/history record is committed, and only publishes `DONE` / `COMPLETE` after finalization succeeds. The final runner badge reports executed commands rather than an inflated branch-inclusive total.

RC4 release gate: build/launch normally, run `Regression - v0.66.7.8 Run Finalization`, confirm the UI ends at `Script: COMPLETE` and `Runner: DONE`, then run `Full Regression Suite` once.



## RC5 validator correction
The RC4 Workbench validator incorrectly used multiline-sensitive regex assertions without singleline mode, causing a false build failure even though the runner ordering was correct. RC5 changes only those validation expressions to `(?s)` singleline matching; runtime semantics are unchanged.
