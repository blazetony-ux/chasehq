# v0.51.0 PROM compositor default

## Purpose

v0.51.0 is a substantive Chase H.Q. graphics-correction build. It promotes the PROM-driven mixer/compositor path to the default live renderer after deterministic validation showed that the previous REFERENCE/LEGACY paths were being overdrawn by the large map-817 / palette-197 sprite family while the PROM path preserved a coherent road, roadside scenery, HUD and player car.

## Evidence behind the change

The established frame-2352 checkpoint was used to compare the REFERENCE, PROM and LEGACY compositor outputs. At frame 2619, the PROM image differed from REFERENCE by tens of thousands of pixels while REFERENCE and LEGACY were nearly identical; visual and provenance evidence showed the PROM path preserving the expected road/player-car scene where the other paths were dominated by the map-817 / palette-197 sprite field.

A second deterministic run forced `--mixer prom` across the Stage 1 window and remained visually coherent. A scripted `stage1-gameplay` run was then extended into genuine gameplay and PROM remained coherent while the debug timer counted down consistently. These runs justify using PROM as the default while retaining the older paths for diagnosis and comparison.

## Behaviour changes

- Default CLI/UI mixer is now **PROM**.
- `--mixer reference` and `--mixer legacy` remain available for forensic comparison.
- The F-key/UI mixer cycle still exposes all three modes.
- Graphics diagnostics continue to emit explicit REFERENCE, PROM and LEGACY images.
- `prom_vs_legacy_different_pixels` is now calculated from the PROM image versus the LEGACY image regardless of which live mixer was selected; previously the counter accidentally compared the selected final framebuffer with LEGACY while being labelled PROM-vs-LEGACY.

## Curated gameplay checkpoint

The user-supplied live gameplay checkpoint is now bundled as:

`checkpoints/stage1-gameplay-2064.chqstate`

It complements the earlier frame-2352 graphics-forensics checkpoint and should be preferred when a diagnostic requires a state already inside live Stage 1 gameplay.

## Next graphics goal

Do not add generic compositor infrastructure merely because PROM is now default. Use the improved default renderer to identify remaining visible Chase H.Q.-specific priority, road, sprite or tile discrepancies during real gameplay, and fix those discrepancies from evidence.
