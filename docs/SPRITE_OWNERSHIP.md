# Sprite ownership correction (v0.66.9.0-RC2.5)

The RC2.2 conclusion that descending traversal means lower-slot ownership was incorrect. MAME's Chase H.Q. driver traverses highest entry to lowest, but `prio_zoom_transpen` / `prio_transpen` mark every nontransparent candidate occupied, even when its colour is blocked by background priority. Later entries cannot replace it. See MAME `src/mame/taito/taito_z_v.cpp`, `src/emu/drawgfx.cpp` and `src/emu/drawgfxt.ipp`.

The native correction keeps descending traversal and uses a separate per-pass occupancy surface. Existing BG/road/text mask decisions remain unchanged. Pen 0 does not occupy a pixel; nonzero pens with RGB black remain opaque. Sprite owner means the entry whose colour was accepted; a layer-blocked candidate occupies the pixel but is not a visible owner. Coverage counts all nonzero-pen candidates including occluded/blocked candidates and saturates at 65535. Occupancy resets for every sprite pass.

Legacy API/CLI/config names remain compatible: `lower-slot` selects descending traversal (first higher entry retains ownership); `higher-slot` selects ascending traversal (first lower entry retains ownership). They no longer describe the winner. `sprite.order.inspect` explicitly reports traversal and first-nontransparent ownership. Both modes apply identical occupancy rules. This is not a global switch to the historical higher-slot overwrite mode.

Individual sprite export and live rendering share the visible-raster Y transform, including the 16-line subtraction and presentation offset once. Individual assets still represent original RAM sprites independently of live solo/hide/override experiments; aggregate layers represent the selected presentation. Source PNG alpha is authoritative for coordinate comparisons.

## Evidence and tests

Pre-fix causal evidence: session `20260930_131005`, run `003`, frame 2352. Hiding shadow slot 81/map 575 under descending traversal restores 948 pixels. Hiding effects 83/84 removes side effects but not central occlusion. Palette 70 pens 4/8 are written black. Body is slot 82/map 475. See `evidence/investigations/player-shadow-review/report.md` for prior evidence and the original interpretation error.

- `Test-SpritePriority.bat`: native synthetic nontransparent/transparent overlap, black pens, layer-blocked occupancy, reverse traversal, coverage saturation, visible-raster geometry.
- `tests/sprite_snapshot_assertions.ps1`: positive fixture and negative controls for wrong ownership, missing overlap coverage and 16-line export misalignment.
- `research/scripts/regression/regression-sprite-ownership.chqscript`: canonical checkpoint at frame 2352, aggregate snapshot and independent live-solo source renders of 82 and 81; asserts ownership/coverage and exact export placement, with nonzero uncovered shadow visibility. Included before Full Regression's terminal COMPLETE marker.

Structured snapshots additionally export `sprite-ownership.csv` (`x,y,owner,coverage`, 320x240 rows; owner 65535 = no accepted sprite). This is sprite-stage ownership, before any later text composition in non-reference modes. `frame.snapshot.assert-overlap` compares both individual PNGs at their advertised coordinates with corresponding solo runtime source renders across the entire screen, then rejects any overlap owned by the back sprite or missing coverage. It requires actual front-owned overlap and actual back visibility outside the front. It does not treat completion of screenshot capture as rendering proof.

Windows proof is complete for the sprite correction on the pre-package RC2.4.4 working tree: the focused canonical assertion returned `overlapPixels=689`, `frontOwnedOverlap=553`, `backVisibleOutsideFront=247`, `exportCoordinatesExact=true`, and Full Regression reached its terminal COMPLETE marker. RC2.5 packages that proven correction together with new script/workbench reliability changes; therefore RC2.5 itself still requires a fresh Windows package gate. The pre-fix TC0100SCN whole-frame hash validator includes sprites and must not be used as a golden image for changed sprite behavior; retain its background-specific proof separately.
## RC2.6 bitplane-significance interaction

RC2.6 changes which nonzero 4bpp pen number a sprite source pixel produces, but does not change the ownership contract above. Four-bit reversal maps `0 -> 0` and every value `1..15` to another nonzero value, so transparent-vs-nontransparent occupancy is invariant. Descending traversal, first-nontransparent reservation, background blocking, black-pen opacity and visible-coordinate rules must therefore remain unchanged. `Test-SpritePriority.bat` and the canonical ownership regression stay mandatory after the decoder correction.

The source-decoder proof lives in `docs/evidence/sprite-bitplane-significance/README.md`; do not reinterpret palette intervention from that experiment as an ownership rule.

