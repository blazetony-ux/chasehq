# Canonical checkpoints

The checkpoint corpus is intentionally semantic: keep states that anchor reproducible claims.

## Core

- `stage1-gameplay-2064.chqstate` — early player gameplay; general controls/gameplay research.
- `stage1-driving-2352.chqstate` — autonomous attract/driving graphics/road reference; **not** a player-control state.

## Target/contact/damage

- `stage1-target-immediate-pre-contact-5587.chqstate` — one frame before the first proven physical target contact in the recovered contact timeline. Useful for collision-response A/B work.
- `stage1-target-first-physical-contact-5588.chqstate` — first proven physical contact response; this contact by itself is not the damage-authority event.
- `stage1-target-immediate-pre-damage-6333.chqstate` — **preferred target-health/damage anchor**. One frame before a known authentic damaging impact. Use this for deterministic target-health and one-hit experiments.
- `stage1-target-first-damage-6334.chqstate` — matching first genuine damaging-hit state. PC `0xA112` decrements confirmed remaining-hit counter `0x1002AE`.
- `stage1-target-post-final-hit-9548.chqstate` — post-final-hit transition while the defeated target is entering scripted slowdown. Do not describe as near-destruction/pre-hit health.
- `stage1-end-level-9988.chqstate` — confirmed Stage-1 end/intermission reference.

CHQSTATE v1 remains the restore format. The 6333/6334 pair supersedes older lost/non-bundled 7060/9495 references for target-damage causality.
