# v0.61.1 Web Debugger Foundation

This candidate advances the v0.61 browser bridge toward a real debugger workstation without changing emulated gameplay behaviour.

## Added
- Web debugger controls: pause/resume, frame step, CPU-A/B instruction step.
- Live CPU register panels.
- Memory inspector with `why mem` provenance query and browser-local pinned values.
- IOC port view plus deterministic live pulse controls.
- Research event and semantic sprite timelines.
- Watch/patch inspection.
- Named snapshot save/restore/list controls.
- Restricted research command console (no shell execution; allowlisted debugger command families; shell metacharacters rejected).
- One-click evidence capture skeleton: named snapshot, screenshot, event CSV, sprite CSV.
- Lightweight rolling browser history graph for event activity.
- Machine-readable `research/knowledge/` skeleton for symbols, gameplay variables, memory map, routines, assets and hypotheses, with explicit hypothesis/probable/confirmed states.

## Architectural direction
The browser is now treated as the future primary research/debug workstation. The emulator/debug API remains authoritative. Features required by the browser should be added to the shared API/telemetry model rather than implemented as browser-only game knowledge.

## Not yet claimed
- Historical frame cursor/time travel (needs native rolling state/history).
- Native JSON protocol and SSE/WebSocket push stream.
- Persistent workspace layouts/pins.
- Full asset decoding/browser.
- Track plot/recorder.
- Cross-linked symbol/provenance knowledge UI.
- Headless runtime mode.
