# v0.41 Reusable Diagnostic Platform

v0.41 generalises the reverse-engineering workflow used by the memory and graphics tracers so future graphics, controls, timing and audio work can use the same reproducible tools.

## Scenarios

Scenarios establish a known game state; tracing presets only decide what is observed.

- `--scenario boot`
- `--scenario stage1-gameplay`
- `--scenario stage1-driving` (currently aliases the proven Stage 1 gameplay setup until native controls are wired)
- `--scenario service-mode` (reserved; deliberately does not invent undocumented service inputs)
- `--scenario crash-test` (Stage 1 setup plus an extended observation window; collision injection remains future work)
- `--scenario-list`
- `--scenario-describe NAME`

The Stage 1 scenario embeds the proven IOC pulse sequence, so a graphics investigation no longer needs thirteen `--pulse-ioc` arguments.

## Hardware-state captures

`--capture-frame N` writes `logs/capture_frame_N/` containing all mutable mapped bus regions, the TC0110PCR palette, a capture manifest and runtime summary. The format is intentionally useful for diffing and offline analysis.

This is **not yet a resumable CPU save-state**: Musashi CPU contexts/device-private latch state are not currently serialised. The distinction is recorded in every capture manifest rather than pretending the capability is complete.

## Generic event categories

`--trace-event NAME` maps common subsystem events onto the existing mature memory tracer:

- `ioc` -> `$400000-$400003`
- `palette` -> `$A00000-$A00007`
- `sprite` -> `$D00000-$D007FF`
- `road` -> `$800000-$801FFF`
- `sound` / `sound-command` -> `$820000-$820003`
- `cpu-control` -> `$800000-$800001`
- `irq` -> category marker for run manifests/summaries; IRQ-specific event records will be expanded when timing work begins

Use `--event-trace-from-frame`, `--event-trace-to-frame` and `--event-trace-max` to bound these traces. The existing generic tracer's PC, CPU, trigger, before/after and context options remain available alongside event categories.

## Input trace

`--input-trace` writes `logs/input_trace.csv`, recording the important IOC input ports, steering bytes and IOC read counters each frame. This is groundwork for deterministic control recording/replay and native keyboard/gamepad support.

## Run manifest

Every normal frontend run writes `logs/run_manifest.json` unless `--no-run-manifest` is supplied. It records build, scenario, command line, effective frame count, mixer, capture frames and event categories. Diagnostic bundles therefore remain self-describing when shared later.

## Master config

`--debug-config FILE` accepts key/value entries such as:

```text
scenario=stage1-gameplay
frames=1800
capture_frame=1800
graphics_preset=car
graphics_frame=1800
sprite_frame=1800
event=palette
event=sprite
input_trace=true
```

CLI options can still be layered on top.

## Maximal bundle

`--debug-everything` enables the broad sprite/video/raw-map/input forensic exports at the selected final frame. It is intentionally disk-heavy and intended for "capture everything likely to matter" runs.

## Design rule going forward

Controls, sound, sound-CPU communication, timing and future devices should use the same pattern: scenario + subsystem event categories + filters/triggers/context + machine-readable captures + manifests, rather than one-off diagnostic builds.

## v0.42 address-centric tracing

For new investigations prefer `--follow-address`, `--follow-read`, or `--follow-write` before constructing broad fixed-PC traces. The provenance engine attaches dynamic caller chains and exports a call graph automatically. Use `--fast-forward-to` to remove SDL rendering/pacing cost before a late forensic window while still executing all CPU frames.

See `docs/EXECUTION_PROVENANCE.md`.

## v0.43 additions

For final-video questions prefer `--pixel-provenance` / `--pixel-provenance-pair` over another broad CPU trace once the hardware sprite record is already established. Address provenance is now focused by default; request `--provenance-full-graph` only when a global graph/profile is actually needed. `--break-frame` provides an exact-frame forensic capture with rolling pre-break PC history, while `--auto-zip-logs` and `--batch` support unattended evidence collection.


## Burn-in findings through v0.51.2

Current confirmed tooling gaps: no deterministic CLI steering-value injection; checkpoint-relative exit/screenshot/performance accounting needs correction; provenance summary counts can disagree with valid callstack CSV/log evidence; memory-watch samples need explicit distinction from event-time writes; raster diagnostic PNGs cannot expose hardware lines 240..255; `run_phase.txt` finalisation and IOC log provenance wording need cleanup; large evidence bundles need a compact generated index. These should be fixed as a coherent debugger batch, not as one-off instrumentation releases.
