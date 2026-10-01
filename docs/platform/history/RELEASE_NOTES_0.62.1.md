# ChaseHQ-Native v0.62.1 — Simplified Launcher

This is a workflow/polish update to the v0.62 Live Research Workbench. It does not intentionally change Chase H.Q. emulation behaviour.

## Added

- New canonical `Start-ChaseHQ.ps1` one-command launcher.
- Default launch goes directly to the `Gameplay` checkpoint.
- Named `Gameplay`, `Boot`, `Attract`, `TargetPostFinalHit`, and `StageEnd` modes.
- Automatic native-API readiness wait.
- Automatic Web Workbench launch and readiness wait.
- Automatic browser opening.
- `.chq/active-session.json` process/session record.
- `Start-ChaseHQ.ps1 -Stop` to stop the recorded emulator/web session.
- Clear semantic mapping of `stage1-driving-2352` to attract-mode reference state.

## Fixed

- Research session manifest build string now reports `0.62.1` instead of the stale `0.61.2` value.
- Package root name follows the build version rather than a documentation-only suffix.

## Retained

`Start-ChaseHQResearch.ps1` and `Start-ChaseHQWeb.ps1` remain supported as advanced/manual launchers.
