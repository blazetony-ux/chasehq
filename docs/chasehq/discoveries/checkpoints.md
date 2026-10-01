# Canonical research checkpoints

These states are reproducible evidence anchors, not merely convenience saves. Experiments should name the checkpoint used and preserve its hash in evidence where practical.

| Checkpoint | Purpose / status |
|---|---|
| `stage1-driving-2352.chqstate` | Deterministic **attract-mode graphics/reference** state. Do not use it as a player-control gameplay baseline. |
| `stage1-gameplay-2064.chqstate` | Canonical live Stage-1 **player gameplay** baseline for controls, road/course, timer, compositor and gameplay-state work. |
| `stage1-target-immediate-pre-contact-5587.chqstate` | One frame before the first proven target physical-contact response in the recovered target encounter. Useful for collision/contact A/B work. |
| `stage1-target-first-physical-contact-5588.chqstate` | First proven target physical-contact response; PC `0xA13C` modifies target lateral state `0x10A084`. This is contact response, not the damage authority. |
| `stage1-target-immediate-pre-damage-6333.chqstate` | **Preferred target-health/damage research anchor.** One frame before a known authentic damaging hit. Use this for deterministic health/damage and one-hit-target experiments. |
| `stage1-target-first-damage-6334.chqstate` | First observed genuine target-damage event from the manual timeline: PC `0xA112` decrements confirmed counter `0x1002AE`; normal/terminal paths branch at `0xA124`/`0xA118`. |
| `stage1-target-post-final-hit-9548.chqstate` | Post-final-hit/defeat-transition state while the target is already in scripted slowdown. It is **not** a pre-hit or near-destruction-health checkpoint. |
| `stage1-end-level-9988.chqstate` | Confirmed Stage-1 end-level/intermission state. |

## Preferred target research path

For target damage work, start at `stage1-target-immediate-pre-damage-6333.chqstate`. The authentic next-frame damaging impact is deterministic and was used to causally confirm CPU-A `0x1002AE` as the target remaining-hit/damage counter. The earlier 5587/5588 pair remains useful for separating generic physical overlap/contact from genuine damage.

Historical frame-7060 target-approach and frame-9495 pre-final-hit states were referenced during earlier research, but their original checkpoint bytes were not retained in the distributable corpus. They are superseded for current target-health work by the recovered 6333/6334 pair.

Do not silently replace a canonical checkpoint with a different state using the same name. Superseding states should receive a new identity and document the relationship.

The historical name `stage1-target-near-destruction-9548.chqstate` was inaccurate. The packaged frame-9548 state is deliberately named `stage1-target-post-final-hit-9548.chqstate` because the decisive hit has already occurred.
