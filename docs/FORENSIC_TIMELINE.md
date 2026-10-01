## v0.66.9.0 frame-boundary convention

Timeline checkpoints and write events intentionally describe adjacent phases of an emulated frame. The runtime executes frame **N** while the bus/write tracer still tags writes with **N**; after execution the main frame counter increments and the resulting machine state is checkpointed as frame **N+1**. Therefore a write recorded as `frame=N` is normally observed in `timeline.seek frame=N+1`. The initial checkpoint is the pre-execution state at the recording start frame. Investigations should compare N/N+1 rather than assuming write-event labels and checkpoint labels are the same phase boundary.

This convention explains the turbo-HUD evidence: writes tagged frame 2065 first appear in checkpoint 2066. The timeline remains authoritative; the labels simply represent different boundaries.

# Forensic Timeline v1

**Release status:** proven in ChaseHQ-Native v0.66.8.0 on Windows/SDL (2026-09-29). The final RC7.4 Full Regression passed after bounded run-history discovery eliminated the prior browser OOM. The real historical turbo timeline has been analysed offline without replaying gameplay.

## Purpose

Forensic Timeline is a **record-once, investigate-many-times** subsystem. It captures a bounded gameplay interval as a seekable forensic corpus so later diagnosis can happen without replaying the event again.

v0.66.8.0 deliberately uses a simple, fidelity-first storage model: **one deterministic CHQSTATE checkpoint for every recorded frame**, plus an unfiltered bus-write provenance stream. The format can be delta-compressed later without changing the script/API contract.

## What is captured every frame

CHQSTATE already includes CPU-A and CPU-B architectural contexts, cycle/instruction counters, all mutable bus regions, palette state/provenance, IOC state and other deterministic bus state. Mutable memory includes the main/sub work RAM, shared RAM, tilemap/control RAM, sprite/object RAM, road RAM and the current `sound_stub` region.

In addition, Timeline v1 writes `writes.csv` for every non-internal bus write during capture:

- sequence number;
- emulated frame;
- CPU A/B;
- writer PC;
- address;
- width;
- old value;
- new value;
- changed flag.

This catches transient writes that could disappear before the next frame boundary.

## Sprite data

Sprite/object RAM is therefore preserved in every frame checkpoint. Loading/seeking a timeline loads the recorded checkpoint into the paused emulator inspection surface, so the normal sprite APIs (`sprites.list`, `sprite.inspect`, `sprite.map.inspect`), palette tooling and layered `frame.snapshot` can operate on a historical frame exactly as they do on a paused live state.

Per-sprite PNGs are intentionally generated **on demand** rather than duplicated for every sprite on every recorded frame.

## Audio status

The current native emulator does not yet implement the real Chase H.Q. sound CPU/chip/PCM path. It exposes only `sound_stub`. Timeline v1 captures that stub state because it is mutable bus state, but **does not claim full audio capture**. The timeline manifest advertises `audioRuntime=false` and `audioStub=true`.

When authentic audio emulation lands, this timeline contract is intended to grow to include sound-CPU state, chip writes, channel/sample events and a PCM reference stream without breaking existing timeline scripts.

## Recording

```chqscript
api timeline.record.start name=turbo-complete-cycle
# run the event
api timeline.record.stop
```

The current frame is captured immediately when recording begins. Every subsequently emulated frame is captured automatically until stop/finalization.

## Event-window indexing

A broad recording can be indexed after capture to an exact event interval using the complete write stream. For turbo:

```chqscript
api timeline.trim.event name=turbo-complete-cycle cpu=A address=0x10040F width=8 mask=02 pre=1 post=1
```

That finds the masked 0→1 and later 1→0 edges and creates a non-destructive event-window index. The authoritative recording is never trimmed or rewritten.

## Automated memory/writer analysis

```chqscript
api timeline.scan.memory name=turbo-complete-cycle
```

The scan groups every changed address in `writes.csv`, counts changed frames, records first/last frame, initial/final value and unique writer PCs, correlates changes to the event rise/fall, classifies simple temporal patterns and writes:

- `timeline-analysis/<timeline>/memory-write-candidates.csv`
- `timeline-analysis/<timeline>/memory-byte-candidates.csv`
- `timeline-analysis/<timeline>/ranked-candidates.json`
- `timeline-analysis/<timeline>/writer-groups.json`
- `timeline-analysis/<timeline>/report.html`

The scan uses the manifest event automatically when `triggerFrame`/`stopFrame` are omitted.

## Import / inspection parity

```chqscript
api timeline.load name=turbo-complete-cycle
api timeline.frames
api timeline.seek frame=2117

api memory.read cpu=A address=0x10040F width=8
api register.read cpu=A
api gameplay.registry
api sprites.list
api palette.bank bank=64
api state.snapshot include=sprites,regions,gameplay,track
```

`timeline.seek` loads the selected recorded CHQSTATE into the paused native runtime. This deliberately reuses the existing inspection APIs instead of creating parallel `timeline.memory.*`, `timeline.sprite.*`, etc. commands.

`status` exposes `source=timeline` while a recording is imported.

## Fork back to live

```chqscript
api timeline.fork-live
```

The selected recorded checkpoint remains loaded but Timeline mode is released. The machine is paused in LIVE mode and normal writes/patches/stepping can be used for a counterfactual experiment.

`timeline.unload` instead restores the live checkpoint saved immediately before the timeline was loaded.

## File layout

```text
name.chqtimeline/
    manifest.json
    frames.csv
    writes.csv
    frames/
        frame_00002064.chqstate
        frame_00002065.chqstate
        ...
    # immutable recording only; derived analysis belongs to the current run bundle
```

## Current limitations / next iteration

- Storage is fidelity-first and can be large because every frame is a complete CHQSTATE. Delta/keyframe compression is planned after Windows proof.
- CPU-A/B are captured; authentic sound CPU/chip state is unavailable because native audio emulation is not yet implemented.
- The v1 automated scan is write/provenance based. Additional offline signature analyzers (counters, bit lifetimes, control-vs-experiment comparison, graphics-memory correlation) can be layered on the immutable recording later without replaying gameplay.
- Workbench timeline scrubber/waveform-style UI is a follow-on presentation layer. Script/API parity is the first acceptance criterion.

## Imported timeline execution semantics

An imported timeline is **read-only by default**. Ordinary read-side APIs operate against the selected recorded frame, including memory/register reads, Gameplay Registry, sprite inspection, palette inspection, screenshots and layered snapshots. `control.step-frame` and `control.run-frames` advance the timeline cursor instead of executing CPUs. Instruction stepping, resume, memory/register writes and patches are rejected until `timeline.fork-live` is used.

`timeline.fork-live` deliberately leaves the selected frame loaded, exits timeline mode and returns a paused live emulator state for counterfactual experiments. `timeline.unload` instead restores the live state that existed immediately before import.

## Automated whole-event analysis

`timeline.scan.memory` analyses the complete captured bus-write stream offline. It now emits both transaction-level and byte-address-level candidate tables, ranks changes near the recorded trigger/expiry frames, groups all changed addresses by writer PC, and generates a self-contained HTML report:

- `analysis/memory-write-candidates.csv`
- `analysis/memory-byte-candidates.csv`
- `analysis/ranked-candidates.json`
- `analysis/writer-groups.json`
- `analysis/report.html`

This means later investigations do not need to know an address in advance. The capture contains every frame checkpoint plus every normal CPU bus write for the bounded event; new scripts can repeatedly query and re-rank the same immutable evidence without replaying gameplay.

`timeline.trim.event` accepts an 8/16/32-bit event field even when the producing CPU write is wider and begins at an earlier address. It extracts the requested big-endian subfield from overlapping writes before detecting rising/falling edges.

## RC2 resilience corrections after first Windows capture

The first real turbo recording exposed an integration timeout rather than a recorder failure. Per-frame full-state capture is intentionally much slower than ordinary execution; the old Script API bridge used a fixed 15-second `control.run-frames` wait and could return an error while the native emulator was still making valid forward progress. RC2 changes frame waits to be **progress-aware**: a long operation may continue while frames are advancing and is treated as stalled only when no frame progress occurs for the stall window. Browser/front-proxy long-operation budgets are widened accordingly.

The packaged turbo capture also advances its long tail in four 60-frame chunks. This is not required for correctness, but it gives the script runner bounded progress points and clearer evidence.

The first failed browser run did not destroy its native recording. The uploaded RC1 evidence contained a complete 256-frame timeline (2064..2319) with 711,374 write records; the authoritative turbo-active bit rose at frame 2065 and fell at frame 2275. `timeline/recover-existing-turbo-recording.chqscript` exists specifically to continue from such a recording without replaying gameplay.

Script Console selection is also now synchronized with the editor: initial library discovery auto-loads the selected script, category/version/scope/search filtering auto-loads a newly selected result, and frontend bootstrap does not report READY unless a library script has actually loaded. This prevents the stale-editor case where changing a filter visually selected a different script but Run executed the previously loaded text.

## RC3 streaming / immutable-analysis rule

The first real 711,374-write turbo recording exposed a serious performance flaw in the RC2 offline analyser: `timeline.trim.event` used full-file `Import-Csv`, accumulated PowerShell objects with `$rows +=`, imported the same 34 MB write stream again, then destructively rewrote the recording. On Windows PowerShell this exceeded the 360-second backend limit.

RC3 changes the contract:

- the authoritative `.chqtimeline` is immutable after recording finalization;
- `timeline.trim.event` is a compatibility name for **event-window indexing**, not destructive trimming;
- event detection uses one sequential streaming pass and stops after the matching falling edge;
- the discovered view is stored under `analysis/events/` and mirrored to `analysis/event-window.json`;
- `timeline.scan.memory` streams the write log and automatically limits aggregation to that indexed event window when present;
- no full write CSV is loaded into a PowerShell object array.

This preserves the original full capture so later investigations can define different windows without replaying gameplay.

## RC7 portable automated analysis and SDL playback

RC6 proved that the historical turbo capture can be analysed offline without replaying gameplay. RC7 makes derived analysis a property of the **current script run**, not the source recording. `timeline.scan.memory` and `timeline.trim.event` now write under the current run's `artifacts/timeline-analysis/<timeline>/` tree and record source hashes/provenance, so the `.chqtimeline` remains immutable and the resulting evidence travels in `bundle.zip`.

`timeline.analyze.auto` performs the higher-level unattended pipeline:

1. streaming temporal candidate ranking;
2. known semantic reference tagging for the confirmed turbo fields;
3. top-candidate writer-PC grouping;
4. automatic disassembly around unique high-value writer PCs;
5. historical milestone state capture at before/active/mid/expiry/after frames;
6. structured layered frame snapshots for those milestones;
7. `HUD_TURBO` comparison in normal, HUD-only and HUD-hidden modes;
8. portable JSON/CSV/HTML outputs in the current run bundle.

Recorded events are also visually replayable in SDL. `timeline.load`, `timeline.seek`, `timeline.next`, `control.step-frame` and `control.run-frames` already load historical CHQSTATE frames into the native renderer. RC7 adds paced convenience playback:

```chqscript
api timeline.play name=turbo-complete-cycle from=2064 to=2276 fps=30
```

This is **recorded-state playback**, not CPU re-execution: the SDL renderer displays each historical frame checkpoint in sequence. The timeline remains read-only and loaded at the final frame; use `timeline.unload` to restore the pre-import live state or `timeline.fork-live` to continue as a live paused machine. Authentic audio playback is still unavailable because the native sound runtime remains `sound_stub`.

## Graphics A/B use in v0.66.9.0-RC2
Captured timelines can be used with `sprite.order.set` to render the exact same historical machine state under higher-slot-wins and descending traversal with first-nontransparent ownership compositor experiments. This avoids replaying gameplay while testing sprite tie-break hypotheses.

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
