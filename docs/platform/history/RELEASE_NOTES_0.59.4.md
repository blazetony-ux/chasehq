# v0.59.4 — Research Consolidation, Paged Debug UI & Target-Defeat Telemetry

v0.59.4 is a consolidation build. It carries forward v0.59.3 handling/profile work, adds the latest Stage-1 target-defeat findings to structured telemetry, improves the interactive debug overlay, and packages the available semantic checkpoints from the final-hit/end-level investigation.

## Debug UI

The F1 panel is now split into four pages to avoid crowding:

- DRIVING
- HANDLING
- TARGET
- SYSTEM

Use F11 / Shift+F11 to cycle forward/backward. The TARGET page adds live target movement (`0x10A092`), motion-control (`0x10A096`), defeat flags (`0x10018D`) and defeat timer (`0x1002D0`) alongside contact/interaction state.

## Structured target telemetry

`target_state.csv` now records:

- target movement magnitude `0x10A092`;
- target motion-control `0x10A096`;
- defeat flags `0x10018D`;
- defeat-mode candidate boolean (`bits 0+2`);
- defeat timer `0x1002D0`;
- existing target position, separation, contact, interaction and collision-suppression fields.

Candidate fields retain candidate-oriented names. `0x1002AE` is not labelled health.

## Bundled checkpoints

Added:

- `stage1-target-post-final-hit-9548.chqstate`
- `stage1-end-level-9988.chqstate`

Both were user-supplied semantic anchors from the same Stage-1 completion investigation. The locally generated 9495 pre-final-hit checkpoint is not bundled because its bytes were not in the supplied evidence package.

## Documentation

Added `docs/CONVERSATION_RESEARCH_SUMMARY_0.59.4.md`, a consolidated technical narrative of the project state and the evidence established throughout the current research thread. Added `docs/DEBUG_UI_0.59.4.md` for the paged overlay.

## Preserved behaviour

- known-good Windows output convention remains `out/build/x64-Debug/ChaseHQNative.exe`;
- v0.59.3 handling overrides/profile controller are retained;
- 1.15× remains the best tested mapping-oriented cornering scale, not a hardcoded default;
- `--no-collisions` remains limited to the five proven physical-response writes;
- no CHQSTATE format change is made in this build.
