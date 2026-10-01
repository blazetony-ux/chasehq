# ChaseHQ-Native v0.62.0 — Live Research Workbench

This is the pickup candidate prepared while Windows validation is unavailable. It is based on v0.61.3 and deliberately concentrates on reversible research controls rather than speculative gameplay fixes.

## Implemented
- Native live handling control: `handling get`, `handling set CORNERING [SPEED_RETAIN]`, `handling reset`.
- Native live palette inspection/freeze/restore: 256 banks × 16 pens from the emulated TC0110PCR palette state. Research freezes are reapplied at the frame boundary and can all be cleared to return control to the game.
- Web **Driving Tuning** panel with immediate cornering/speed-retain changes and authentic reset.
- Web **Palette Lab** with live bank cycling, decoded colour swatches, entry selection, raw-word freeze, per-entry restore and restore-all.
- Web raw **Gamepad monitor** using the browser Gamepad API. No Chase H.Q. button mapping is guessed.
- Web **IOC held-state bridge** for confirmed masks, alongside deterministic IOC pulses.
- Self-documenting HTTP API: `/api/docs/`, `/api/v1/openapi.json`, `/api/health`.
- Versioned HTTP routes for handling tuning, palette inspection/override and held IOC state.
- PowerShell module functions for the same controls.
- Native `status` reports handling and palette research override state so evidence can identify non-authentic runs.

## Preserved from v0.61.x
Research session launcher, multi-instance launcher, Web Debugger, named snapshots, evidence capture, experiments, memory export/diff, annotations/workspaces, knowledge database, IOC pulses and existing CLI diagnostics remain present.

## Deliberately not guessed
- No hard-coded controller button mapping beyond the existing confirmed IOC mechanisms.
- No claim that palette selection/decoding is the cause of the colour mismatch.
- No arbitrary live sprite injection yet; existing sprite timeline/export and generic memory/patch tools remain available. Sprite Lab/injection remains a follow-on once Windows evidence confirms the desired path.
- SSE/native JSON/native high-volume bulk export remain roadmap work; the current localhost PowerShell bridge remains transitional.

## Validation status
CPU/runtime-only Linux build succeeds and the available CTest suite passes 1/1. Windows/MSVC+SDL, PowerShell HttpListener, browser Gamepad API, and live Web/API integration require validation on the development machine.
