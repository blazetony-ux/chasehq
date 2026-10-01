# Chase H.Q. Native v0.51.1

v0.51.1 is a focused visible-area geometry patch built from v0.51.0.

## Why this patch exists

Runtime forensics established that Chase H.Q. writes the turbo HUD tile references and RAM-defined character data correctly, but the indicators are absent from the final framebuffer. Independent visual evidence also showed the scene/sky positioned suspiciously low.

MAME's Chase H.Q. machine configuration uses a 262-line raster with visible vertical range 16..255 (240 lines). v0.51.0 instead treated output Y=0..239 as hardware raster Y=0..239.

## Geometry correction

The 320x240 output is retained, but output Y=0..239 now maps to hardware raster Y=16..255 consistently for:

- TC0100SCN BG source/rowscroll Y
- TC0100SCN text source Y
- TC0150ROD source scanline selection
- runtime sprite screen Y
- debug sprite-list screen Y

The proven turbo text rows 29/30 occupy hardware Y=232..247, so this correction should place them at output Y=216..231 rather than clipping them at/below the old framebuffer boundary.

## Validation required

This is a real patch build, but Windows/SDL gameplay validation is still required. Do not consider the geometry correction fully proven solely because turbo indicators appear. Verify top HUD, sky/horizon, road, scenery, sprites/player car and bottom HUD remain mutually aligned across gameplay.

## Unchanged from v0.51.0

- PROM compositor remains the default.
- Both curated checkpoints are bundled.
- Existing diagnostics and evidence-bundle tooling are retained.

## Windows package / folder convention

Extract this patch with the project root named exactly `ChaseHQ-Native-v0.51.1-Visible-Area-Patch`. In the established workspace this is `C:\Projects\ChaseHQ-Native-v2-NoGit\ChaseHQ-Native-v0.51.1-Visible-Area-Patch`. Do not reuse `out\` from the earlier `ChaseHQ-Native-v0.51.0-Geometry-Test`; configure a fresh build tree from the v0.51.1 root. The local SDL3 source tree must be present under `SDL\` before configuring an SDL build.
