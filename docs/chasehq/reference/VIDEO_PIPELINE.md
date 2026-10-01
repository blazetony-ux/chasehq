# Chase H.Q. Native Video Pipeline Diagnostics

## Current pipeline

`TC0100SCN bottom BG -> TC0150ROD road A/B -> TC0100SCN upper BG -> sprites -> text -> final framebuffer`. Sprite/road arbitration may use the current B52 PROM-mask reconstruction or the retained legacy mask path.

## v0.36 frame bundle

Each `graphics_frame_N` directory includes:

- `00_final_prom.png` — normal current output before debug visualisation.
- `10..14_*_only.png` — each hardware source independently.
- `20..23_stage_*.png` — incremental composition through sprites before text.
- `24_composite_legacy_no_prom.png` — same frame through the legacy mask path.
- `30_layer_mask.png`, `31_prom_address_bits.png`, `32_prom_address_output.png`, `33_road_signal_probe.png`.
- `34_prom_vs_legacy_difference.png` — pixels whose final RGB differs between current PROM and legacy paths.
- `40_sprite_owner.png` — logical sprite slot that owns each final accepted sprite pixel.
- `41_sprite_overlap_count.png` — number of non-transparent sprite candidates touching each pixel.
- `42_sprite_bounding_boxes.png` and `sprite_screen_placement.csv`.
- `sprite_screen_isolated/slot_NNN.png` — every active sprite at actual screen position.
- `diagnostics_summary.txt` — difference count, overlap maximum, layer-mask histogram and visible pixels by slot.
- with raw-map export: binary layer/PROM/owner/coverage maps and `pixel_pipeline.csv`.

## Interpretation order

1. Check `13_sprites_only.png`. If bad, focus on live zoom/anchoring/positioning despite correct unscaled object reconstruction.
2. If sprites-only is good, compare `22_stage_bottom_road_upper.png` and `23_stage_before_text.png` plus per-slot blocked counts.
3. If stage 23 is plausible but `00_final_prom.png` differs unexpectedly from `24_composite_legacy_no_prom.png`, inspect PROM address/output maps and difference image.
4. Use owner/overlap maps to identify which logical slot creates any visible corruption and then correlate that slot back to v0.35 sprite manifests/raw RAM.

## v0.38 priority/palette evidence layer
The frame bundle now carries overlap-pair analysis, current-layer rejection density, mask-candidate ranking, full TC0110PCR palette tables, raw/effective PROM dumps, per-frame PROM usage, address-bit activity, video-register snapshots and controlled suppression/priority experiments. These sit downstream of the already-validated sprite artwork path and are intended to isolate final composition faults without another instrumentation build.


## v0.38 reference compositor

The preferred validation path is now: bottom TC0100SCN (priority 0) -> upper TC0100SCN (priority 1) -> TC0150ROD (priority 1/2) -> text (priority 4) -> sprites back-to-front using masks `0xf0` for sprite priority 0 and `0xfc` for priority 1. This path is deliberately separate from the experimental B52 PROM reconstruction. Diagnostic captures export both paths and a pixel-difference image.

Normal palette conversion is `xBBBBBGGGGGRRRRR`; raw `$0000` is black. Synthetic fallback colours are diagnostic-only and must not be used to infer object identity or masking behaviour.


## v0.51.1/v0.51.2 geometry status

The final framebuffer is 320x240 but corresponds to hardware raster Y=16..255. v0.51.1 moved tilemap/text sampling, road scanlines and sprite coordinates into that visible-raster coordinate system; this restored the three turbo HUD indicators that had been clipped at the bottom and improved top-of-screen geometry.

Historical note: v0.51.2 introduced BG presentation offsets while the underlying TC0100SCN Y-scroll interpretation was still uncertain. RC2.3 RAM/layout and four-way A/B evidence later isolated a Y-scroll sign error. RC2.4 corrects that hardware-coordinate calculation and returns BG0/BG1 defaults to `0:0`. `--layer-offset` remains presentation-only and must not be treated as emulated register state.

## v0.66.9.0-RC2.4.2/RC2.4.3 source-layer localization

The deterministic post-Y-fix frame-2065 v2 snapshot localizes the visible sky dithering/banding to raw `sources/bg-bottom.png` before road, upper-BG, sprite, text or final compositor contribution. Exact snapshot reconstruction remains the authority for composition (`verifiedExact=true`, `mismatchedPixels=0`). Do not change TC0100SCN presentation offsets or compositor rules merely to remove that source-layer texture; reopen it as a renderer defect only if a direct reference comparison proves a specific mismatch.

The frame-2352 causal test isolates true shadow slot 81/map 575 over body 82/map 475. RC2.5 reserves nontransparent sprite occupancy while retaining descending traversal and opaque black; it also aligns individual exports with visible-raster coordinates. See ../../SPRITE_OWNERSHIP.md for implementation and proof gates.
## RC2.6 sprite source-decode correction

Sprite source decoding is upstream of palette lookup, priority masks and composition. RC2.6 makes the 16x16x4bpp pen decoder shared between live rendering and export/inspection tooling and corrects plane significance to `physical 0->pen bit 3, 1->2, 2->1, 3->0`. The RC2.5 Map-405 causal probe established this by changing only which palette entries the already-decoded pens selected; coherent cloud shading appeared without geometry or priority changes. Therefore late palette substitution, global colour remapping or compositor-side nibble reversal are explicitly rejected as production fixes.

The corrected source pen then follows the existing pipeline unchanged: pen-0 transparency/occupancy decision -> palette lookup -> priority/background acceptance -> sprite ownership -> later composition. Because 4-bit reversal maps only zero to zero, RC2.5 first-nontransparent occupancy semantics remain valid and are still regression-gated.

