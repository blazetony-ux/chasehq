# v0.61.0 Research Infrastructure

This candidate deliberately builds infrastructure while v0.60.3 remains the unvalidated baseline.

## Added now
- Deterministic live IOC pulse: `chqctl input pulse PORT MASK FRAMES`. Expiry is based on emulated frames, not wall time.
- Per-session evidence namespace and `session.json` manifest in `Start-ChaseHQResearch.ps1`; emulator `--logs` is routed into that session.
- Multi-instance launcher `Start-ChaseHQMultiResearch.ps1` with automatic consecutive API ports and A/B-style session names.
- `Watch-ChaseHQEvent.ps1`: first event/action automation controller. A memory watch can trigger checkpoint, screenshot, pause, or another command. Event provenance remains in the research event buffer.
- `Start-ChaseHQWeb.ps1`: dependency-free localhost HTTP research browser/bridge over chqctl. Initial live panels: status, IOC ports, recent research events, semantic sprite changes, pause/resume.

## Architecture rule
The emulator/debug API is authoritative. chqctl, PowerShell automation, SDL debugger and browser are clients. New gameplay/asset discoveries should be exposed once through structured telemetry rather than reimplemented per UI.

## Next expansion
Native structured JSON responses; push/SSE event subscriptions; in-process event/action rules; track telemetry/recorder; decoded asset endpoints; experiment manifests/diffs; Native Oracle consumers.
