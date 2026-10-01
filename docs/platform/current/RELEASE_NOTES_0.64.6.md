# ChaseHQ-Native v0.64.6 — Stable Research Workbench

This release packages the Windows-tested form of the v0.64 Research Workbench.

## Included fixes

- Proven Windows PowerShell compatibility for the concurrent embedded C# frontend.
- C# 5-compatible helper syntax.
- Concurrent request dispatch via `Task.Factory.StartNew(...)`.
- No `await` expressions inside `catch` clauses, matching the Windows PowerShell `Add-Type` compiler used in live testing.
- Manual Script Console cancellation is a neutral `STOPPED BY USER` state, not an error.
- Retains asynchronous browser-side Script Console execution, progress, Stop Script, Copy/Clear/Download Output, Logs/Diagnostics, stateful toggles, exact frame execution, image comparison/diffing, Gameplay Registry, Track View and Experimental IOC tooling.

## Live validation

The concurrent frontend, script loading, long-running scripts, cooperative stop, Copy Output, exact +60-frame run, frame capture, ROAD_AREA comparison and diff generation were all validated on the Windows development machine before this package was cut.
