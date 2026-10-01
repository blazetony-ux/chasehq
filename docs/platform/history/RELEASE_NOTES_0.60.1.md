# v0.60.1 — Live Research API

v0.60.1 turns the v0.60 Research Workbench into a remotely scriptable *local* debugging surface.

## Added

- `chqctl.exe`, a copy/paste-friendly console client.
- Localhost-only debug server in the SDL frontend, default `127.0.0.1:37600`.
- `--debug-api-port N` and `--no-debug-api`.
- Live pause/resume, controlled frame stepping and CPU-local single-instruction stepping.
- Baseline save/restore from the console.
- Live CPU-A/B 8/16/32-bit RAM read/write.
- Live FREEZE and SUPPRESS interventions plus patch inspection/clear.
- Live Research Event Bus watches that can be added/cleared after startup.
- `trace` aliases for dynamically creating memory-event traces without relaunching.
- Console queries/export for research events and semantic sprite changes.
- Runtime checkpoint save/load and screenshot commands from `chqctl`.
- Optional interactive `chqctl shell`.

## Intent

The primary productivity gain is the ability to receive a one-line command during an investigation, paste it into a second console, and immediately change what the already-running emulator observes or does. This is the first external-control layer; future audio, IOC/input, device, provenance and experiment adapters should be exposed through the same controller rather than creating independent tooling.
