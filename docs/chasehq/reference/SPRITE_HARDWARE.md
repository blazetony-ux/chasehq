# Chase H.Q. Sprite Hardware Reference

## Confirmed working model (v0.35+)

Sprite RAM is `$D00000-$D007FF`, consumed back-to-front as 8-byte entries. W0 contains Zoom Y and Y; W1 contains priority, palette and Zoom X; W2 contains Flip Y, Flip X and X; W3 contains the 11-bit object number. Chase H.Q. applies a +7 Y offset, then anchors chunks at `y + (128 - zoomY)`.

Zoom X selects object format: 128x128 uses OBJ-A with 8x8 chunks; 64x128 uses OBJ-B with 4x8 chunks; 32x128 uses OBJ-B with 2x8 chunks. B52-38 spritemap entries are interpreted as 16-bit words and the effective graphics tile is `raw & 0x3fff`. Each OBJ graphics region is 0x200000 bytes = 16384 16x16x4bpp tiles.

### RC2.6 4bpp source-plane significance
For each 16x16 tile, the physical source-plane bit blocks remain at offsets `0, 16, 32, 48` within each 64-bit row group. Their **pen significance** is high-to-low: physical plane 0 -> pen bit 3, plane 1 -> bit 2, plane 2 -> bit 1, plane 3 -> bit 0. RC2.5 had the physical addresses right but weighted them low-to-high. The frame-2352 Map-405 causal palette simulation independently produced the exact 4-bit-reversal signature and coherent cloud shading, so RC2.6 centralizes the corrected mapping in `src/sprite_gfx.h`.

Equivalent old-pen transform: `0->0, 1->8, 2->4, 3->12, 4->2, 5->10, 6->6, 7->14, 8->1, 9->9, A->5, B->D, C->3, D->B, E->7, F->F`. This preserves pen-0 transparency and nonzero occupancy while changing palette selection for affected source pixels.

## Diagnostic invariants

A coherent `assembled_unscaled.png` proves ROM decode + spritemap lookup + object traversal for that object. A coherent exported zoomed object but bad `13_sprites_only.png` points at live screen anchoring/placement. A coherent sprites-only frame but bad staged composition points at layer masking, overlap or priority. A difference confined to `34_prom_vs_legacy_difference.png` focuses investigation on PROM wiring/polarity rather than object decode.

## v0.36 per-slot measurements

`sprite_screen_placement.csv` records raw words, renderer coordinates, format/map base, palette/priority/flips, PROM address/raw output/block mask, candidate visible pixels, pixels accepted against the pre-sprite BG/road mask, blocked pixels, final owner pixels and estimated loss to later sprite overlap. `sprite_screen_isolated/slot_NNN.png` renders each active logical sprite at its actual screen coordinates independently of other layers.

## v0.38 mask/overlap analysis
Exact non-transparent sprite-pair overlap is now measured for every capture. High-overlap or colour-uniform objects are ranked as mask/control candidates, but the ranking never alters normal emulation and must not be treated as a confirmed object type. Controlled suppression and forced-priority images are generated for the highest-ranked candidates so the visual consequence of each hypothesis can be evaluated without recompiling.
