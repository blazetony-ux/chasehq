# Diagnostic Evidence Policy (v0.48.4)

Chase H.Q. is the sole current development target. Diagnostics should make the emulator report what it already knows rather than forcing a human to infer facts from screenshots.

## Evidence hierarchy

1. Authoritative structured/machine-readable data (CSV/JSON/JSONL/raw maps).
2. Human-readable logs derived from the same state.
3. Screenshots and diagnostic images as supporting/correlation evidence.
4. Visual inference only when the underlying mechanism is not yet observable directly.

Raw hardware values must be retained alongside decoded/interpreted values. Where an interpretation is uncertain, label its confidence/provenance rather than presenting it as hardware fact.

## Graphics records we want directly

Per sprite/component where practical: frame and cycle context; RAM slot; raw attributes; graphics/spritemap code; raw coordinates; transformed screen coordinates; zoomed/rendered bounds; clipped bounds; visible-pixel bounds; flip; palette/pen; priority; draw/order information; component/group identity; and state changes/deltas.

Longer term, maintain pixel/compositor provenance: candidate layers/sprites, winning source, rejected sources and priority/mixer reason. This enables a "why is this pixel this colour?" query without reverse-engineering a screenshot.

Stable logical object identities are desirable above volatile RAM slots so a vehicle/object can be tracked through animation, code changes and slot reuse. Snapshot and change/event logging should coexist.

## Timeline correlation

Subsystem evidence should share frame plus finer timing/cycle context where available so CPU writes, RAM changes, sprite changes, mixer decisions, inputs, sound commands and other events can be correlated on one timeline.

## Diagnostic presentation

Presentation-only experiments must not alter emulated machine state. Planned sprite tools include hide, solo, highlight/track, solid black/white/checkerboard backgrounds, exact masks/silhouettes, outlines and diagnostic component colouring. Prefer generating masks/edges from renderer ownership data rather than image analysis.

Experiments should be classified as observation filters, presentation experiments, state experiments, or ROM/code experiments. They should be reversible and logged.

## Capture and evidence bundles

v0.48.4 adds deterministic final-frame PNG capture using emulated frame numbers. Screenshots support the structured evidence; they do not replace it. Evidence bundles should be self-describing: build, ROM hashes where available, scenario/checkpoint, frame range, experiment and trace configuration.

Future analysis should support regions of interest, change/delta logs, anomaly detection, deterministic baseline/current comparisons, and visual regression generated from the same replay.

Core rule: **do not make the human infer something the emulator can report directly.**

## Automatic evidence packaging (v0.52.0 requirement)
For multi-run diagnostic experiments, the supplied command/tool should normally perform the complete workflow: clean/create the experiment root, create isolated per-run directories, execute every case, preserve structured metadata/logs and screenshots, create a compact experiment index where practical, and automatically produce one final ZIP for upload/review. Manual evidence collection/compression should be the exception, not the normal workflow.
