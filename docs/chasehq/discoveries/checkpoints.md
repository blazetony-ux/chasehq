# Canonical research checkpoints

These states are reproducible starting points, not merely convenience saves. Experiments should name the checkpoint used and preserve its hash in evidence where possible.

| Checkpoint | Purpose |
|---|---|
| `stage1-gameplay-2064.chqstate` | general Stage-1 gameplay baseline |
| `stage1-driving-2352.chqstate` | deterministic driving/handling research |
| `stage1-target-post-final-hit-9548.chqstate` | user-confirmed decisive/final-hit aftermath while target enters scripted slowdown; authoritative bundled target-transition checkpoint |
| `stage1-end-level-9988.chqstate` | confirmed end-of-level screen/state |

Historical high-value frame-7060 target-approach and frame-9495 pre-final-hit states have been referenced in research but are not guaranteed to be bundled unless their checkpoint bytes are present.

Do not silently replace a canonical checkpoint with a different state using the same name. Superseding states should receive a new identity and document the relationship.

The previously referenced `stage1-target-near-destruction-9548.chqstate` name is not present in the current packaged corpus and must not be treated as canonical unless actual checkpoint bytes are supplied under a deliberately documented identity.
