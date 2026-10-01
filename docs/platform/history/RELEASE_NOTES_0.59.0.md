# ChaseHQ-Native v0.59.0 Release Notes

## Theme

Target / collision diagnostics and mapping-oriented consolidation.

## Added

- `--target-state-log`.
- `target_state.csv` with Stage-1 target record, separation, contact candidate, interaction type/timer/counter and suppression count.
- `collision_events.csv` with contact/timer/suppression transitions.
- Experimental `--no-collisions` mapping assist that suppresses the proven CPU-A `$00A156` lateral shove to `0x10A044` while preserving upstream game logic.
- Target/contact and interaction timer/type readouts in the F1 debug overlay.
- `--course-survey` automatically enables target-state logging.
- Consolidated `docs/PROJECT_STATE.md` and `docs/RESEARCH_FINDINGS_0.59.0.md`.

## Preserved

- v0.58.3 quiet fast-forward behaviour and build layout.
- v0.57 closed-loop course follower.
- Existing checkpoints and CHQSTATE v1 compatibility.
- Existing graphics/provenance/tracing workbench.

## Deliberately not claimed solved

- Universal collision disable across every traffic/scenery interaction.
- Target health/damage/destruction/caught semantics.
- Authoritative AIRBORNE/GROUNDED state.
- Branch/route selector at course forks.
- Whether the complete reachable course graph is finite/repeating/end-delimited.
- ROM instruction patching / same-write `patch-when` reliability.
