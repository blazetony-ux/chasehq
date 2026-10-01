# ChaseHQ-Native v0.66.9.0-RC2.2

## Purpose

Correct the native Chase H.Q. same-priority sprite traversal after the RC2/RC2.1 A/B evidence and authoritative MAME source review.

## Confirmed reference rule

Current MAME `src/mame/taito/taito_z_v.cpp`, function `chasehq_draw_sprites_16x16`, traverses sprite RAM from the final entry toward entry zero (`offs = bytes/2 - 4; offs >= 0; offs -= 4`). With ordinary transparent drawing, the later draw wins an equal-priority sprite/sprite overlap. Therefore lower-numbered Chase H.Q. sprite slots are drawn later and win equal-priority overlap.

Reference reviewed 2026-09-29: https://github.com/mamedev/mame/blob/master/src/mame/taito/taito_z_v.cpp

## Changes

- Native/default `sprite_tie_break` changed from `higher-slot` to `lower-slot`.
- `--sprite-tie-break higher-slot` remains available as a reversible forensic comparison.
- `sprite.order.inspect` continues to expose actual traversal and draw positions.
- Game Lab wording now identifies lower-slot ordering as the MAME-reference/native default.
- Added loop-based `graphics/same-priority-sprite-order-multiscene-validation.chqscript`.
- Added focused `regression-v06690-rc22-mame-sprite-order.chqscript` and included it in Full Regression.
- Project state, roadmap, handover, API/script documentation, knowledge and validation notes updated together.

## Evidence carried forward

The RC2.1 A/B captures showed that lower-slot ordering places slots 83/84 (the animated under-car effect) behind player-car slot 82 and produces the plausible player-car composition. The MAME source review independently confirms the same traversal direction.
