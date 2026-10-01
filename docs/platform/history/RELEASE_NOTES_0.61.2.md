# v0.61.2 — HTTP Automation API

This candidate promotes the localhost Web Debugger bridge into a scriptable research automation surface while preserving the native debugger API as the authoritative control plane.

## Added
- Versioned `/api/v1` HTTP surface with structured JSON responses for status, capabilities, IOC ports, events, sprites and individual memory reads.
- HTTP POST controls for pause/resume, frame/instruction stepping and deterministic IOC pulses.
- Restricted generic research command endpoint.
- Conservative live binary RAM-range download endpoint (up to 1 MiB through debugger-safe byte reads).
- Evidence capture endpoint producing a manifest, named snapshot, screenshot, event CSV and sprite CSV.
- `tools/powershell/ChaseHQResearch.psm1` wrapper around `Invoke-RestMethod` / `Invoke-WebRequest`.
- API smoke-test example.
- Multi-instance launcher can optionally start one Web/API bridge per emulator and writes `instances.json` for automation discovery.

## Deliberately deferred
- Native bulk RAM/VRAM/tilemap/sprite binary export endpoints. The current RAM bridge proves the workflow but is not intended for high-volume dumping.
- Native JSON protocol and push event stream/SSE.
- Decoded sprite/tilemap/asset downloads until authoritative decoders/export contracts are exposed by the core.

No intended gameplay/emulation behaviour changes.
