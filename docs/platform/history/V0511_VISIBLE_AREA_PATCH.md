# v0.51.1 visible-area patch

This patch promotes the v0.51.0 geometry experiment into a versioned patch build for runtime validation.

## Evidence basis

Chase H.Q. uses a 262-line raster with visible Y range 16..255 (240 lines). The native renderer previously treated framebuffer Y=0..239 as hardware raster Y=0..239.

The patch maps output Y=0..239 to hardware Y=16..255 consistently across BG/text, road and sprites.

The change is motivated by two correlated faults:

1. the scene/sky appears vertically low;
2. turbo HUD tile references are written into TC0100SCN rows 29/30, but the indicators are clipped/missing.

The patch is intentionally narrow: no speculative gameplay-state UI additions are included yet.
