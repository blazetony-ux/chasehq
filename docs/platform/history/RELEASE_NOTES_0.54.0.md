# Chase H.Q. Native v0.54.0 — Gameplay State + Course Diagnostics

## Consolidated gameplay-state diagnostics
- `gameplay_state.csv` now reports authoritative SPEED, SPEED INTERNAL, DISTANCE, TIMER, SCORE, TURBO, TURBOS LEFT and IOC accelerator/brake state.
- Steering is separated into injected raw IOC value, game raw 12-bit value, signed 12-bit value, and signed processed value. New `--steering-signed`, `--steering-signed-at`, and `--steering-signed-range` accept intuitive signed 12-bit values while the original raw controls remain available.
- Adds authoritative road state from CPU-A `$10A048`: INTERIOR, EDGE, or OFF_ROAD, while preserving raw flags.
- Adds a brake-lamp state alongside the two underlying palette samples.

## Course decoder foundation
- New `--course-data-log` exports `course_raw.csv`, `course_banks.csv`, and per-frame `course_state.csv`.
- Source store `$109000-$109FFF` is exported as 16 banks × 32 records × 8 bytes.
- Record byte +0 is labelled as signed horizontal curvature/trajectory control, based on the traced signed expansion/integration path.
- Bytes +1 and +3 remain raw/unnamed pending semantic proof.
- Live output includes course position `$10080E`, current bank/record and authoritative road-zone state.
- The exporter intentionally distinguishes source/world trajectory information from later perspective-projected road X.

## Reliability
- Repeated `--follow-read` and `--follow-write` options now combine access modes instead of the last option silently disabling the other mode.
- Gameplay/course CSV streams are explicitly flushed and closed before runtime teardown/evidence packaging.
- Curated checkpoint metadata now identifies the attract checkpoint separately from the player-gameplay checkpoint.

## Still experimental / not claimed
- Course bytes +1/+3 are not yet called elevation or width.
- Complete bank traversal and physical X/Y scale/orientation are not yet hardware-proven.
- HUD minimap and deterministic course-aware autoplay are roadmap features, not part of this release.
