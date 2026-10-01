# Discovery record — sprite and video path

**Status:** core sprite format/path largely CONFIRMED; remaining compositor/palette accuracy work OPEN

## Confirmed sprite RAM format
Raw sprite RAM: `$D00000-$D007FF`, 8-byte entries.

- W0: bits 15..9 Zoom Y; bits 8..0 Y.
- W1: bit 15 priority; bits 14..7 palette bank; bits 6..0 Zoom X.
- W2: bit 15 Flip Y; bit 14 Flip X; bits 8..0 X.
- W3: bits 10..0 spritemap/object number.
- Sprite list consumption is back-to-front.
- B52-38 spritemap entries are little-endian 16-bit words; effective graphics tile index `raw_code & 0x3fff`.

## Proven player-car path examples
- `$1027E8-$1027EF`: slot 44 software staging entry.
- `$1027F0-$1027F7`: slot 45 primary-car staging entry.
- `$D00160-$D00167`: slot 44 hardware sprite RAM destination.

## Rejected interpretation
Global zero-colour transparency is REJECTED: investigated slot-44 palette entry 70/pen 4 is deliberately written `$0000`.

## RC2.6 confirmed sprite-decoder finding
The remaining broad sprite-colour error was isolated causally on RC2.5 at Stage 1 frame 2352 / Map 405 / palette bank 175. The physical source-plane addresses were already correct, but their 4bpp significance was reversed. Physical planes 0,1,2,3 must contribute pen bits 3,2,1,0. A palette-only simulation of that four-bit reversal produced coherent shaded Map-405 cloud artwork without changing geometry, priority, ownership, or ROM data; the structured snapshot remained exact. RC2.6 implements the significance in the shared live/export source decoder. See `../../evidence/sprite-bitplane-significance/README.md`.

This result does **not** authorize palette tuning as a renderer fix: the palette intervention was a causal simulation of a source-decoder hypothesis. RC2.6 removes the need for that intervention by correcting the decoded pen itself.

## Current unresolved visual work
Fresh Windows/SDL prove-off is required across representative sprite scenes after the decoder change. Missing turbo indicators and any residual HUD issues remain separate compositor/text-layer investigations. The `scene-no-sprites.png` forensic base also has a separately tracked export bug discovered in the RC2.5 causal bundle.

## Reference
See `../reference/SPRITE_HARDWARE.md`, `SPRITE_FORENSICS.md`, `PRIORITY_PROM.md` and `VIDEO_PIPELINE.md` for the detailed evidence chain.


## 2026-09-25 live Palette Lab intervention

**Starting state:** canonical Stage-1 gameplay checkpoint (`stage1-gameplay-2064.chqstate`).

**Method:** inspect TC0110PCR bank 0 in the Web Workbench, select a non-zero palette entry, replace its raw value with `$0000`, and observe the SDL output without restarting.

**Observation:** the top HUD/text colours visibly changed after the live entry override. This confirms the Workbench can causally alter a palette entry that contributes to the rendered frame.

**Tool defect found:** in v0.62.x, `Restore entry` only released the freeze and did not immediately write the pre-override raw value back, so the HUD remained altered until a later game write/restart. v0.63.0 stores the original raw value at first override and restores it explicitly.

**What this does not prove:** it does not yet identify the root cause of the broader colour-accuracy problem, and changing the browser's viewed bank does not force a different hardware palette bank.

**Reusable method:** controlled one-entry palette intervention is a valid way to distinguish whether a visible element is using a suspected palette entry; always restore the exact original value and repeat from a canonical checkpoint.
