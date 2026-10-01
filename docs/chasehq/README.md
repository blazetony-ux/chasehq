# Chase H.Q. research documentation

This tree documents the **subject being researched**, not the Research Workbench itself.

## Structure
- `discoveries/` — current evidence-led discovery records. Each records starting state, method, observations/interventions, evidence, conclusion, confidence and remaining questions.
- `reference/` — consolidated technical references such as the memory map, sprite hardware and video pipeline.
- `history/` — older release-era research narratives retained for provenance. They are evidence history, not the preferred current summary.

## Confidence vocabulary
- **CONFIRMED** — repeated evidence and/or causal intervention supports the interpretation.
- **PROBABLE** — strong structural/correlation evidence, not yet fully causal/generalised.
- **HYPOTHESIS** — plausible interpretation awaiting stronger evidence.
- **REJECTED** — a tested interpretation contradicted by evidence.
- **OPEN** — observed behaviour/address whose meaning remains unresolved.

## Current discovery index
1. `discoveries/gameplay-state.md` — authoritative/near-authoritative gameplay values and unresolved states.
2. `discoveries/stage1-target-contact.md` — Stage-1 target identity, contact chain and rejected health interpretation.
3. `discoveries/road-and-course.md` — road geometry, course store, bank reuse and route questions.
4. `discoveries/handling-and-cornering.md` — movement coefficient path and experimental cornering override.
5. `discoveries/collision-response.md` — proven collision response writes and scoped suppression.
6. `discoveries/sprite-and-video.md` — sprite format/path, compositor and graphics evidence.
7. `discoveries/checkpoints.md` — canonical reproducible states and what each is for.
8. `discoveries/open-investigations.md` — immediate unresolved Chase H.Q. questions.

A discovery record should be updated when evidence changes. Do not silently promote a hypothesis to CONFIRMED.

## Current investigation
v0.66.9.0-RC2 focuses on same-priority sprite ordering for the player car/under-car effect and exposes deterministic A/B controls using existing timelines.

### RC2.2 sprite traversal finding
MAME's Chase H.Q. renderer (`chasehq_draw_sprites_16x16`) traverses sprite RAM from high entries to low entries. RC2.2 adopted that descending traversal but incorrectly retained later-candidate overwrite semantics and therefore inferred that lower-numbered slots win overlaps. RC2.5 adds the missing first-nontransparent occupancy rule; `higher-slot` remains a reversible diagnostic traversal mode.
