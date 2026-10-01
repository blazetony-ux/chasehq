# ChaseHQ-Native v0.64.9 — Graphics Research & Autonomous Experimentation

## New
- Live Sprite Monitor tab in the Web Workbench with per-slot map/palette/position change highlighting and recent history.
- Structured Frame Snapshot API and Script Console actions: `frame.snapshot` and `frame.snapshot.list`.
- Snapshot captures an immutable PNG, semantic sprite CSV/tail and manifest as the foundation for offline composition/provenance work.
- Bundled experimental live-track mutation scripts (flat, strong left/right, synthetic slalom, restore).
- Bundled 1,800-frame and 3,600-frame autonomous attract-mode track survey scripts.
- Bundled graphics baseline snapshot script.

## Portability rule
Generic snapshot/monitor/experiment concepts belong in the research platform. Chase H.Q.-specific addresses remain in `games/chasehq/` or game-specific scripts; hardware-specific sprite/palette decode belongs under `hardware/taitoz/` as it is introduced.

## Still planned
- Normalized render-object model with priority, layer, bounding box and source provenance.
- Offline frame composer for reorder/recolour/hide/map substitutions.
- Render-pass/layer isolation captures.
- Palette provenance and sprite-region picking.
- Server-side long-running job runner and `run-until` predicates.
