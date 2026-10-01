# v0.64.2 — Workbench Stability & Diagnostics

- Cooperative Script Console runner keeps the Web Workbench responsive during long scripts.
- Live script progress, Stop Script, Copy Output, Clear Output, and Download Output.
- Script Library selection auto-loads; explicit Load has visible status/error handling.
- Pause/Resume replaced by a single stateful Emulation toggle.
- Added Logs / Diagnostics tab with request timing and runtime/session log tailing.
- Browser refresh polling no longer overlaps itself; API requests have a bounded timeout.
- IOC preset labels changed to ASCII `pulse xN` form to avoid mojibake.
- Deterministic frame-run endpoint now pauses before sampling its start frame, removing the observed +1 race when invoked while the game was running.
- Carries forward v0.64.0 Research Workbench, Gameplay Registry, Track View, Experimental/Game Lab, image analysis, evidence tooling and Musashi disassembly.
