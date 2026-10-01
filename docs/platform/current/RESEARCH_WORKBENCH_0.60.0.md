# v0.60.0 Live Research Workbench

## Purpose

The Workbench is the first implementation of the project's generic **observe -> intervene -> compare -> rewind -> repeat** architecture. It intentionally uses generic memory/event primitives rather than Chase H.Q.-specific one-off diagnostics, so the same model can later accept input/IOC, audio, device, timing, tilemap, road and other adapters.

## Opening and execution control

Press **F12** during the SDL frontend. Opening the Workbench:

1. enables the research event layer;
2. pauses emulation;
3. saves `logs/workbench/pause_baseline.chqstate`;
4. arms a non-breaking sprite-RAM write watch so semantic sprite changes can recover recent writer PCs.

Controls while the Workbench owns focus:

| Key | Action |
|---|---|
| F12 / Esc | close Workbench |
| Space | run / pause |
| F3 | execute one CPU-A instruction while paused |
| Shift+F3 | execute one CPU-B instruction while paused |
| F5 | replace the temporary pause baseline with the current state |
| F6 | run 1 frame, remain in paused experiment mode |
| F7 | run 5 frames |
| F8 | run 30 frames |
| F9 | clear active live patches and restore the temporary pause baseline |
| F2 | screenshot |
| F10 | export interactive evidence bundle |
| Delete | clear active live patches |
| Shift+Delete | clear research watches |
| Ctrl+Delete | clear research-event history |

The CPU-local F3 step is a research operation: it executes a single instruction on one 68000 and does **not** advance whole-machine frame timing, IRQ cadence, or the peer CPU. Use frame stepping when whole-machine timing matters.

## Live intervention editor

`Tab` selects ADDRESS, VALUE, WIDTH, CPU or MODE. Hexadecimal keys edit address/value. Left/right changes WIDTH, CPU or MODE. Enter applies the selected operation.

Modes:

- **WRITE ONCE** — directly alter mutable emulated memory now.
- **FREEZE** — write the requested value now and replace future exact matching writes with that value.
- **SUPPRESS** — preserve the current value by suppressing future exact matching writes.
- **WATCH WRITE** — record matching writes and pause at the next safe frame boundary.
- **WATCH CHANGE** — as above, but only when the value actually changes.

All active patches display interception count and last writer information in the Workbench. Patches are research state; they are not silently promoted into source fixes.

## Event model

The current memory adapter publishes a rolling event history containing:

- frame;
- event kind;
- CPU/address space;
- writer/reader PC;
- address and width;
- old/new value;
- changed flag.

Capture is dormant unless the Research Workbench is enabled. This is the first adapter on a deliberately broader event architecture intended for future input, audio, device and rendering producers.

## Sprite-change provenance

Every rendered frame compares the decoded sprite slots with the previous frame. Events are classified as:

- movement-only; or
- **semantic**: map, palette, priority, shape, visibility, creation/destruction.

The Workbench surfaces recent semantic changes and looks backward through recent sprite-RAM events to attach the most recent writer PC for that slot's 8-byte sprite record.

This is deliberately generic. Immediate applications include the enemy hit bar, target hit feedback, turbo indicators, police lights, billboard/object identification, later-stage assets and compositor debugging.

## Rewind / trial workflow

A practical test now looks like:

```text
F12                     pause + save baseline
select 10A096 / 16-bit
WRITE 01AA
F7                      run five frames
inspect target/sprite/event state
F9                      restore identical baseline
WRITE 0180
F7
compare
```

No source edit or rebuild is required while discovering the causal intervention.

## Current v0.60 boundaries

This release establishes the live substrate, not the finished debugger studio. Important current limits are explicit:

- WATCH breakpoints stop at the next **safe frame boundary**, not midway through an emulated frame.
- FREEZE/SUPPRESS currently match CPU space + exact start address + exact access width.
- trial outcome scoring/diff ranking is still manual in the UI; automatic A/B/N comparison is next-layer work.
- sprite provenance currently correlates against recent sprite-RAM writes rather than a full source-read dependency graph.
- the generic event bus currently has a memory adapter; audio/input/device adapters are architectural follow-ons.
- event formatting remains synchronous; the planned profiler/async event writer is still needed for very heavy research sessions.

These constraints are intentional: v0.60.0 first proves the high-value live pause/patch/run/rewind workflow before layering on automated causality and batch experimentation.
