# Discovery record — Stage-1 target identity and contact chain

**Status:** target identity/contact chain CONFIRMED; health semantics OPEN

## Starting state
Canonical Stage-1 encounter/checkpoint research around frames 7060–7085 and later near-final-hit states.

## Method
1. Correlate visible pursued vehicle with object records and semantic sprite activity.
2. Compare target-approach/contact frames.
3. Perform causal A/B intervention by moving the candidate object longitudinally away.
4. Observe whether interaction type, score award and response path disappear while object processing remains alive.
5. Trace writes during the contact frame and follow physical response into player state.

## Outcome
- Target object record: `0x10A080–0x10A0BF`.
- Target longitudinal position: `0x10A080`.
- Investigated visual assembly: maps `319/320/321/322`, palette `152`.
- `0x10A089` bit `0x20`: strong contact/interaction candidate.
- `0x10042E`: interaction/event type; type 9 observed in the investigated contact.
- `0x1002C6`: initialised to `0x003C` in the type-9 path; strong interaction-response timer candidate.
- Contact path ultimately reaches the player lateral shove around CPU-A `$00A12A` / `$00A156`.

## Rejected interpretation
`0x1002AE = target health` is **REJECTED**. Intervention showed that forcing it near zero changes downstream phase/counter behaviour without eliminating the encounter/score event. Keep it described as an interaction/phase counter candidate until stronger evidence exists.

## Why this matters
This is a model discovery record: the causal intervention was more informative than simple correlation. The reusable procedure is extracted in `../../methods/recipes/discover-damage-or-health.md` and `../../methods/recipes/controlled-a-b-intervention.md`.
