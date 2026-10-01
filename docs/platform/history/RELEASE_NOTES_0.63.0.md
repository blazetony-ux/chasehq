# ChaseHQ-Native v0.63.0 — Research Workflow

## Added

- Web SDL-frame preview with manual/low-frequency auto refresh.
- Canonical checkpoint list/load from the Web Workbench.
- Native game-timer freeze/resume API and Web controls.
- Native SDL always-on-top API and Web controls.
- Responsive single-column Workbench layout for snapped browser windows.
- Active-session log browser/tail.
- Quick buttons for known Stage-1 IOC scenario pulse values.
- Deterministic checkpoint-based IOC single-bit sweep with per-mask screenshots/events.
- PowerShell module helpers for checkpoints, IOC sweeps, timer freeze and always-on-top.

## Fixed

- Controller wizard steering-right false detection caused by accepting a large return-to-centre delta.
- Palette Restore Entry / Restore All now writes saved pre-override values back immediately rather than merely releasing the freeze.
- Palette UI now says **View bank** and explicitly states that the bank selector is inspection-only.

## Carried forward

- v0.62.6 resilient Web-host response handling.
- v0.62.x one-command launcher, evidence capture, handling tuning, Palette Lab, controller telemetry, HTTP/OpenAPI tooling and canonical checkpoints.

## Validation status

CPU/runtime-only Linux configure/build/test passes 1/1. `src/main.cpp` also passes a standalone C++20 syntax check with the project defines. Windows SDL, PowerShell HttpListener, `SDL_SetWindowAlwaysOnTop`, browser workflow endpoints and live IOC sweep require validation on the development machine.
