# RC2.4.2 post-Y-fix raw background evidence

Source run: `bundle(20260929-234833).zip`, packaged script `TC0100SCN Post-Y-Fix Layer Isolation`, canonical Stage 1 frame 2065.

`bg-bottom-frame2065.png` is the raw `sources/bg-bottom.png` exported by the v2 structured frame snapshot. The visible sky dithering/banding pattern is already present in this raw TC0100SCN background source, before road, upper background, sprite, text/HUD or final compositor contributions.

The same snapshot's `reconstruction.json` reports exact final reconstruction (`verifiedExact=true`, `mismatchedPixels=0`). Therefore the visible sky pattern is not introduced by later composition in this capture. Treat it as source-layer presentation unless a direct authoritative reference comparison proves a specific rendering mismatch.
