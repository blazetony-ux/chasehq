# ChaseHQ-Native v0.64.8.4 — Reliability & Autonomous-Research Foundation

This build is a reliability pass driven by failures found during real Workbench research.

## Fixes
- Live patch enforcement no longer depends on research-event tracing being enabled. `patch.freeze`, `replace` and `suppress` now intercept matching writes whenever active; hit counts and `last_pc` therefore become meaningful.
- Script Stop now performs an out-of-band emulator pause as well as setting the browser cancellation flag. `run-frames` detects a cleared `step_remaining` and exits as aborted instead of waiting for its original target.
- `run-frames` timeout diagnostics include last frame and remaining-step state.
- `frame.capture` defaults to millisecond filenames and adds a collision suffix rather than overwriting existing evidence. Named captures remain supported.
- Web/backend request log lines now include wall-clock timestamp and Web-host uptime. Browser diagnostics request timestamps include milliseconds.
- MSVC Debug baseline removes explicit `/Od` before target-local `/O2`, eliminating the recurring D9025 override warning while retaining `/Od` for `m68kmake`.
- SDL executable explicitly depends on and links the shared SDL target before post-build DLL staging.

## Regression scripts
- `research/scripts/regression/full-regression.chqscript` — broad deterministic Workbench/API regression.
- `research/scripts/regression/quick-smoke.chqscript` — short pre-research sanity check.

## Research focus
This release is intended to make the next track-mapping, steering-observability and autonomous-driving experiments trustworthy rather than adding speculative gameplay semantics.
