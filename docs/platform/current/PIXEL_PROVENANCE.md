# v0.43 final-pixel provenance

`--pixel-provenance X:Y` traces a selected final video pixel through all non-zero sprite candidates in hardware sprite RAM. For each candidate the export records raw sprite words, object bank, spritemap code, source coordinates/pen, TC0110PCR palette bank/entry/raw value, reference priority bitmap and primask decision, candidate PROM address/output/mask, winner-after-draw-order, final recorded owner and final ARGB.

`--pixel-provenance-pair A:B` automatically chooses overlapping pixels for the requested sprite pair. `--gfx-trace-preset car` now enables pair 44:45 provenance automatically. `--pixel-provenance-max N` limits auto-selected overlap pixels.

Outputs live in `logs/graphics_frame_N/`: `pixel_provenance.csv`, `pixel_provenance.txt`, `pixel_provenance_summary.txt`. A reconstructed winner mismatch is evidence of a renderer/provenance-model discrepancy and should be investigated before changing game-side sprite generation.

## v0.44 selection and experiment improvements

v0.44 makes explicit `--gfx-trace-pixel` and `--gfx-trace-region` selections authoritative for final-pixel provenance. Presets may still provide default regions, but an explicit location is no longer replaced by the pair auto-selector. When pair auto-selection is used without an explicit location, overlapping pixels are ranked for information value: zero-vs-nonzero palette differences, different palette values, different source pens and different palette banks are preferred. The top ranked candidates are exported in `interesting_pixels.csv`.

Controlled renderer experiments are deliberately separate from normal rendering. `--experiment-slot-zero-transparent`, `--experiment-palette-zero-transparent`, `--experiment-sprite-pair-order`, `--experiment-sprite-order`, `--experiment-layer-order`, and `--experiment-layer-matrix` generate comparison images under `experiments_v044/` without changing the default renderer.
