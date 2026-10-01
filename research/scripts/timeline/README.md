# Forensic Timeline scripts

Forensic Timeline v1 records deterministic **per-frame `.chqstate` checkpoints** plus an **unfiltered bus-write provenance stream**. The intentionally simple first format favours correctness and seekability over compactness; delta compression can be introduced later without changing the script/API model.

Core workflow:

1. `capture-turbo-complete-cycle.chqscript` records the event once, indexes it using the confirmed turbo-active bit, then performs offline write/candidate ranking without mutating the recording.
2. `import-and-inspect-timeline.chqscript` shows that normal memory/register/sprite/palette/gameplay APIs work after `timeline.load`/`timeline.seek` because the selected recorded checkpoint is loaded into the paused emulator inspection surface.
3. `timeline-fork-live-example.chqscript` demonstrates counterfactual continuation from a recorded frame.

### What v1 captures

Every frame contains CPU-A/CPU-B architectural state and every mutable bus region saved by CHQSTATE: work RAM, shared RAM, tilemap/control RAM, sprite/object RAM, road RAM, palette state/provenance, IOC state and current stubbed sound bus registers. `writes.csv` records every non-internal bus write with frame, CPU, writer PC, address, width, old/new value and changed flag.

### Sprite parity

Sprite/object RAM is captured every frame. After seeking, the existing `sprites.list`, `sprite.inspect`, `sprite.map.inspect`, palette tools and layered `frame.snapshot` tooling operate on the selected recorded state, allowing per-sprite forensic assets to be generated later instead of storing PNGs for every sprite on every frame.

### Audio limitation

The current native emulator does **not** yet implement the real Chase H.Q. sound CPU/chips/PCM path; it exposes only `sound_stub`. Timeline v1 records that stub state but does **not** claim full audio fidelity. The timeline manifest advertises this explicitly. When native audio emulation lands, the timeline contract is designed to add sound CPU/chip state, events and PCM reference data.

### RC7 workflows

- `analyze-existing-turbo-forensic-timeline.chqscript` runs the complete unattended offline pipeline and writes all derived evidence into the current script run bundle.
- `replay-turbo-timeline-in-sdl.chqscript` replays the historical frame checkpoints through the SDL renderer at 30 fps. This is visual state playback, not CPU re-execution.
- `timeline.analyze.auto` combines memory ranking, writer grouping/disassembly, historical milestone snapshots and HUD_TURBO correlations.

## v0.66.9.0-RC2
Use existing timelines for same-frame graphics A/B tests such as sprite tie-break experiments; do not replay gameplay when the required state is already captured.

## RC2.5 safe timeline transactions

Research Script experiments should no longer assume the previous run unloaded its timeline. `api timeline.reset` is idempotent: call it before timeline mutation work and again in a `finally` block. For script-initiated `timeline.load`, Workbench captures the pre-load live checkpoint so a later reset can restore that live state even after `timeline.fork-live`. Historical timeline state remains read-only; fork live before memory/register writes.

```chqscript
api timeline.reset
try
    api timeline.load name=car-shadow-layer-investigation
    api timeline.seek frame=2352
    api timeline.fork-live
    # counterfactual writes/captures
finally
    api timeline.reset
end
```
