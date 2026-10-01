# v0.64.0 — Research Workbench

This release promotes the browser research interface from a collection of panels into a tabbed workbench and turns recently proven Chase H.Q. gameplay semantics into live, inspectable registry data.

## Workbench UI
- Dedicated **Script Console** tab with library load/save, validation, Run, Run Selection, Run Current Line and a basic syntax-highlight preview.
- Dedicated **Gameplay Registry** tab showing live value, source, raw value and confidence.
- Dedicated **Track View** tab with an opt-in live SVG trajectory recorder derived from confirmed course curvature and live road geometry. It is intentionally labelled as a derived trajectory, not a complete authoritative branch/topology map.
- Dedicated **Experimental / Game Lab** tab for Chase H.Q.-profile controls and diagnostics while the generic Research API remains game-agnostic.
- Dedicated **Next Steps** tab containing only the high-level outstanding diagnosis areas.
- Dedicated **Evidence** tab.
- SDL Always On Top is now represented as a stateful toggle, matching Timer Hold.

## Research API / Script Console
- New native `disasm A|B ADDRESS [COUNT]` command backed by the existing Musashi disassembler.
- New script/API action `cpu.disassemble`.
- New deterministic `control.run-frames` action waits for the requested emulated frame count to complete.
- New `gameplay.registry`, `track.state`, `next-steps`, `image.regions`, `image.compare` and `image.diff` script actions.
- Image comparison operates locally on captured PNGs and reports changed pixels, percentage, mean RGB difference and bounding box. Optional diff PNG output is supported.
- Named Chase H.Q. image regions are profile data rather than generic-core assumptions.

## Chase H.Q. Game Lab
Known semantic IOC helpers are exposed for the currently proven/strong mappings:
- P3 `0x01` — Turbo — CONFIRMED
- P3 `0x02` — TILT trigger — CONFIRMED
- P3 `0x10` — Brake — HIGH CONFIDENCE
- P3 `0x20` — Accelerator — CONFIRMED

Semantic helpers sit on top of the generic IOC XOR/pulse primitives; raw IOC control remains available.

## Authoritative turbo findings carried into the registry
- CPU-A `0x100303` bit 0 — processed turbo input — CONFIRMED
- CPU-A `0x10040F` bit 1 — turbo-active control state — CONFIRMED
- CPU-A `0x1003A2` — turbos remaining — CONFIRMED
- CPU-A `0x100414` — active turbo frame counter — CONFIRMED
- Tested expiry threshold `0x00D2` / 210 frames in the canonical Stage-1 configuration — CONFIRMED for that configuration

## Validation
CPU/runtime build and unit tests pass with `CHASEHQ_BUILD_SDL=OFF`. The Windows SDL build still requires validation on the development machine because SDL3 source/runtime is intentionally not bundled in this source package.
