# Script Runner Resilience - v0.66.0.7

The Workbench script runner now uses an explicit lifecycle: `IDLE -> PREPARING -> RUNNING/VALIDATING -> DONE`, with `FAILED`, `CANCELLED`, and `STALLED` escape states. UI controls are restored from a `finally` path. Preflight queries/includes are bounded and cancellation-aware.

## Fault containment

- Startup self-test verifies required DOM controls/helpers.
- `window.error` and `unhandledrejection` are recorded to `@runtime/web-frontend-errors.log`.
- Fault records include build, runner phase/context, script path, stack, and last API timing record.
- A watchdog marks a runner `STALLED` after 30 seconds without progress and exposes Stop/Force reset recovery.
- Background Workbench refresh remains suppressed while a script job is active.

## Launcher recovery

`Start-ChaseHQ.ps1 -Restart` stops the recorded session, waits for native/public/private ports to release, and starts cleanly. `-HealthCheck` summarizes the recorded session; `-Diagnostics` additionally tails the four runtime logs.

## Regression matrix

Every future runner change should preserve explicit tests for zero dynamic queries, one query, many queries, include loading, cancellation, timeout/malformed response handling, and restart/history recovery.
