# ChaseHQ-Native v0.66.9.0-RC2.8

RC2.8 carries forward the Windows-proven RC2.7 Run 009 baseline (Full Regression completed in 59.615 seconds and the build was marked saturated) and advances the graphics and research work from those findings.

## Changes

- Corrected the normal TC0100SCN text origin from `+23` to `+16` and moved text source-X calculation into a named helper.
- Implemented a separate flipped-text path with 512-pixel source mirroring and the distinct +23 offset and added coordinate tests for normal/flipped behavior.
- Added an independent sprite 4bpp SHADOW decoder and COMPARE helper. The shadow path is diagnostic and does not replace the live decoder or claim whole-frame SHADOW VERIFIED status.
- Added a deterministic regression script that captures and checks the Stage 1 driving and end-results checkpoints, including HUD/radar evidence and exact snapshot reconstruction.
- Corrected gameplay research classification: CPU-A `0x1002E8` is the end-results/intermission gate; `0x1002AE` is unconfirmed and no longer presented as target health.
- Updated the responsive Workbench layout, added an `Open in Evidence` action for selected runs, and refreshed the JSON-driven Next Steps items.
- Updated the native status version field and release metadata to RC2.8.

## Proof status

The included RC2.7 Windows/SDL Run 009 bundle establishes the starting baseline. RC2.8 native unit tests and packaging checks are recorded in `VALIDATION_0.66.9.0-RC2.8.md`. The RC2.8 Windows/SDL build and new Workbench regression still require execution with the project's local SDL/ROM staging configuration.
