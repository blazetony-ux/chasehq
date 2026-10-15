# RC4.2 CHQPACK graphics viewer

Workbench > CHQPACK Browser > load base .chqpack > Inspect graphics/obj-a.bin, graphics/obj-b.bin or graphics/tc0100scn.bin.

The viewer decodes the exact static graphics bytes selected from the base or compatible overlay using the same 16x16 4bpp OBJ bitplane significance and 8x8 packed-nibble TC0100SCN decoding as the native renderer. It displays a paginated atlas (first tile/count), grayscale pen or diagnostic pseudo-palette, nearest-neighbour zoom, provenance, and PNG export. The viewer works locally in the browser without ROMs or a running game. The display palettes are intentionally not claimed to be authentic TC0110PCR colours: these depend on runtime palette RAM. Sprite assembly, map relationships, zoom/flip and live palette are runtime-dependent and remain available through the existing Graphics Lab/sprite diagnostics, not in this static pack viewer.

Acceptance: open a valid pack, inspect each supported graphics region, compare pen pixels with native decoder, export PNG, load compatible overlay and verify changed pens/provenance. Windows `chqdev qualify --fresh` is still required before promotion.
