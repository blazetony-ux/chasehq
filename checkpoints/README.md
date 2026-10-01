# Canonical checkpoints

The checkpoint corpus is intentionally small and semantic. Keep states only when they provide a reproducible research anchor.

- `stage1-gameplay-2064.chqstate` — early player-gameplay checkpoint used for general gameplay and long-run reproduction.
- `stage1-driving-2352.chqstate` — autonomous driving / road-state research checkpoint.
- `stage1-target-post-final-hit-9548.chqstate` — user-confirmed Stage-1 target state after the decisive hit, while the target is entering its scripted slowdown. Do not describe this as pre-final-hit or remaining-health state.
- `stage1-end-level-9988.chqstate` — user-confirmed Stage-1 end-of-level screen.

Important known-but-not-bundled states:

- `stage1-target-approach-7060.chqstate` — validated in prior working trees; target active/approaching and useful for collision/pursuit experiments. Its validated bytes were not present in the v0.59.3 source package used to create v0.59.4.
- `stage1-target-pre-final-hit-9495.chqstate` — generated during final-hit tracing, but not contained in the uploaded evidence bundle. Add only when the actual checkpoint file is supplied.
- `stage1-fork1-approach-XXXX.chqstate` — planned after the route-commit frame is proven.

CHQSTATE v1 remains the current restore format. A future v2 should add optional metadata and embedded PNG preview while retaining v1 compatibility.
