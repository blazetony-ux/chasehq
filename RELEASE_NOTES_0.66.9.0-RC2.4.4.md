# ChaseHQ-Native v0.66.9.0-RC2.4.4

RC2.4.4 is a Workbench/regression-capacity and evidence-metadata hotfix on top of RC2.4.3. Native emulator/video rendering is unchanged.

## Windows evidence carried forward
The focused RC2.4.3 snapshot-discovery regression is now Windows-proven. The uploaded run created `regression-rc243-snapshot-discovery` at frame 2065 and the immediately following assertive `frame.snapshot.list require=regression-rc243-snapshot-discovery` returned that exact current-run snapshot. Both self-comparisons reported zero changed pixels.

The subsequent Full Regression attempt did not execute: preflight failed with `Expanded script exceeds 500 commands`. Static expansion of the packaged suite is 503 commands, so adding the RC2.4.3 regression pushed the permanent suite three commands beyond the historical ceiling.

## Fixes
- Raise the Research Script v2 expanded-command ceiling from 500 to 1000 in both browser and server-side script compilation paths.
- Keep the 64 KiB source, 1000 source-line, 256-loop-iteration and nesting-depth limits unchanged.
- Update the authoritative script-language schema/reference to the same 1000-command limit.
- Derive startup/research-session `build` metadata from `src/version.h` (with CMake fallback) instead of the stale hard-coded `0.66.8.0` value exposed in the uploaded diagnostic bundle.
- Add root `AGENTS.md` as the durable agent/Codex operating contract agreed for the project.
- Promote same-turn structured snapshot discovery from candidate to Windows-proven in project knowledge/state.

## Validation target
Build/start on Windows, run Full Regression, and confirm it reaches `=== ChaseHQ Full Regression: COMPLETE ===`. The focused RC2.4.3 snapshot-discovery regression does not need to be rerun merely to reproduce the already-uploaded PASS unless validating a fresh package.
