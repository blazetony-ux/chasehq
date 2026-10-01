# v0.62.5 — Stop Session Fix

## Fixed

- `Start-ChaseHQ.ps1 -Stop` no longer assumes a nullable integer exposes `.Value` after PowerShell parameter binding.
- Stop logic now normalises PID values loaded from `active-session.json` and tolerates missing/null/stale PIDs.
- Session manifest build marker updated to 0.62.5.

## Scope

Launcher/tooling fix only. No emulation behaviour changed. The native executable may still report its underlying native-core version.
