# Discovery record - sprite and video path

**Status:** sprite record/decode/placement/ownership SHADOW VERIFIED for the canonical placed-source corpus; compositor/tile/road work remains open.

## Confirmed raw sprite format

Raw sprite RAM is `$D00000-$D007FF`, 256 entries x 8 bytes, words big-endian.

- W0: Y bits 8..0; Zoom Y = bits15..9 + 1. Native Y uses `(W0&0x1ff)+7` wrap; logical visible placement includes the proven bottom-anchor/visible-origin transform.
- W1: bit15 priority; bits14..7 palette; Zoom X = bits6..0 + 1.
- W2: bit15 Flip Y; bit14 Flip X; bits8..0 X with hardware wrap.
- W3: bits10..0 spritemap/object number; map0 inactive.
- Traversal is descending slot order; first non-transparent source pixel owns equal-priority overlap.
- Spritemap entries use the established object-size/bank rules and independent 4bpp physical-plane significance 0,1,2,3 -> pen bits 3,2,1,0.

## Independent shadow verification

The v0.66.9.3 promotion carries an independent placed sprite-source renderer that parses raw sprite RAM, raw spritemap bytes, raw OBJ graphics, palette, zoom, flips, clipping and ownership without calling the authoritative placement/decode helpers. The Windows RC5 focused promotion regression compared the independent and authoritative placed-source layers at canonical frames 2352, 2064, 5587, 5588, 6333, 6334, 9548 and 9988. Every frame compared 76,800 pixels and reported **0 pixel mismatches / 0 owner mismatches**.

Therefore both `sprite-4bpp-decoder` and `sprite-placement-ownership` are **SHADOW VERIFIED** at this corpus/scope. They are not yet REPLACEABLE: original/emulated remains authoritative and no default replacement backend exists.

## Confirmed semantic sprite families

Player body maps: `472,475,476,508,514,516,522`, palettes `64/65`. Player shadow maps: `572,575,576,590,592,594,596`, palette `70`. Historical map181/pal75/77 fragments are explicitly rejected as player identity.

Pursuit police-beacon presentation/lifecycle is behaviour-understood at current scope: maps `517/518/519/520/601/602` participate in deployment/roof-side animation, steady deployed is map `520`, CPU-A `0x101854` is the visible/deployment phase, and steady palette state is driven through `0x101858`. Target presentation is one family maps `13/18/19`, palettes `80/81`, causally tied to object record `0x10A080-0x10A0BF`; changing target lateral `0x10A084` first displaces its sprite on the third emulated frame.

The player brake lamp is a palette transformation, not separate geometry: banks64/65 pen13 `000E -> 18DE`, affecting the existing body source.

## Remaining visual work

Do not continue generic sprite-format probing. Remaining high-value graphics work is targeted: target pre-copy producer provenance, deeper TC0100SCN text/HUD writers and flipped-origin edge cases, compositor priority/PROM completeness, then independent tile/compositor/road shadow backends.
