# Sprite masking / control-object reverse engineering

## Why this exists
v0.36 showed that logical sprite artwork can be reconstructed correctly while the final live composition still contains suspicious coverage relationships. In particular, strongly overlapping sprite pairs can resemble mask/control objects. v0.38 therefore measures those relationships directly rather than identifying masks by eye or by colour alone.

## v0.38 outputs
- `sprite_overlap_pairs.csv`: exact non-transparent-pixel intersection for every pair.
- `sprite_mask_candidates.csv`: heuristic ranking using best-pair overlap, lost overlap and colour uniformity. The score is **not** emulation logic.
- `45_mask_candidate_owner.png`: spatial overlay of the top-ranked candidates.
- `experiments/suppress_slot_NNN.png`: full frame with one candidate omitted.
- `experiments/force_slot_NNN_priority0.png` / `priority1.png`: full frame with only the candidate priority input changed.
- `sprite_screen_isolated/slot_NNN.png`: isolated current decoded sprite.

## Interpretation rules
A sprite should only be promoted from “mask candidate” to a hardware mask/control object if multiple independent observations support it. A flat-looking palette is insufficient because palette selection itself is still under investigation. Exact overlap with another object, repeated use in the same role, PROM behaviour and reference-hardware evidence are stronger indicators.

## Current status
Sprite RAM generation, basic object format selection, spritemap lookup, graphics-bank selection and decoded artwork are substantially validated. Mask/control semantics and sprite-to-sprite interaction remain open.


## v0.38 correction to the slot-44 mask hypothesis

Slot 44 overlaps slot 45 intentionally, but v0.37 showed that its apparent solid-green appearance was contaminated by the synthetic palette fallback. v0.38 therefore does not classify slot 44 as a mask/control sprite. It is treated as an ordinary sprite until raw attributes, priority behaviour, or reference output prove otherwise.
