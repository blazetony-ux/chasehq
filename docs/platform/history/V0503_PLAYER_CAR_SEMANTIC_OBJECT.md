# v0.50.3 Player-car semantic object

v0.50.3 promotes the first graphics-forensics result from raw sprite evidence into a named Chase H.Q. semantic object.

## Evidence-backed identity

The Stage 1 frame-2352 investigation established a player-car body/shadow pair that remains spatially linked while the raw sprite slots change.

Current body family:

- maps `472`, `514`, `516`
- palettes `64`, `65`

Current shadow family:

- maps `572`, `575`, `576`, `590`, `592`, `594`
- palette `70`

This is deliberately a **current proven family**, not a claim that every possible player-car animation/map has already been enumerated.

## CLI

All semantic-object functions are command-line driven as well as being available for future UI exposure.

- `--object-solo player_car` expands to the currently proven body/shadow selector set and changes presentation only.
- `--object-evidence player_car` writes `sprite_evidence/semantic_objects.csv` at every configured sprite-evidence sample.
- `--object-track player_car` enables the same semantic-object evidence without forcing solo rendering; it is the stable CLI name reserved for continued temporal tracking as object identity becomes richer.

`semantic_objects.csv` links the logical object back to the actual body/shadow sprite slots, maps and palettes observed on each frame, plus combined bounds and final visible-pixel count.

## Design rule

Raw sprite records remain authoritative evidence. Semantic object output is a higher-level interpretation layered on top and must continue to link back to the raw components that support it.
