# Sprite bitplane-significance causal evidence

## Status

**Confirmed on RC2.5; source correction implemented in RC2.6 candidate.**

This evidence records the causal experiment that isolated the remaining Chase H.Q. sprite-colour problem to **4bpp plane significance**, not palette contents, sprite geometry, same-priority ownership, or compositor ordering.

## Evidence identity

- Build under test: `v0.66.9.0-RC2.5`
- Session: `20260930_175159`
- Run: `008`
- Script: `Sprite Bitplane-Significance Causal Probe`
- Target: Stage 1 frame `2352`, Map `405`, palette bank `175`
- Snapshot: `map405-mame-plane-order-simulation`
- Uploaded evidence bundle SHA-256: `343c14d710023fbb308312900dc453d2eed365409243242c634493aece42c6b2`
- Run result: `PASS`
- Structured reconstruction: `verifiedExact=true`, `mismatchedPixels=0`
- Visible Map-405/palette-175 sprites in the snapshot: `16`

The packaged images are copied directly from that evidence bundle:

- `rc25-map405-plane-order-simulation-final.png` — full causal-probe frame.
- `rc25-map405-slot-041.png` — one isolated Map-405 sprite asset showing coherent cloud shading after the simulated pen mapping.

## Hypothesis and intervention

RC2.5 decoded the four physical source planes with the correct physical bit addresses but assigned significance as `plane 0 -> pen bit 0`, ..., `plane 3 -> pen bit 3`.

MAME's Chase H.Q. 16x16x4bpp layout lists the physical plane offsets in the opposite significance order. The causal probe did **not** alter sprite geometry, map lookup, zoom, priority, traversal, ownership, or source ROM data. It changed only the palette entries addressed by the currently decoded Map-405 pens to simulate the palette selection that would result from reversing the 4-bit pen number.

Observed interventions:

| RC2.5 decoded pen | Original palette raw | Simulated correct target | Target raw |
| ---: | ---: | ---: | ---: |
| 2 | `0x6318` | 4 | `0x5294` |
| 4 | `0x5294` | 2 | `0x6318` |
| 6 | `0x4210` | 6 | `0x4210` |
| 8 | `0x0000` | 1 | `0x7B5C` |
| 10 | `0x4200` | 5 | `0x4A52` |
| 12 | `0x4000` | 3 | `0x5AD6` |

The complete relationship is 4-bit reversal:

`0->0, 1->8, 2->4, 3->12, 4->2, 5->10, 6->6, 7->14, 8->1, 9->9, A->5, B->D, C->3, D->B, E->7, F->F`.

## Result

The Map-405 cloud family became coherent multi-shade artwork while the existing sprite geometry and priority/ownership model remained unchanged. The structured snapshot still reconstructed the captured final frame exactly from its contribution layers.

This is causal evidence that the RC2.5 decoder's **plane significance** was wrong. The RC2.6 implementation therefore corrects significance at the shared low-level sprite decoder instead of adding a palette or compositor workaround.

## Corrected decode contract

For each 16x16 sprite tile:

- physical plane 0 contributes pen bit 3;
- physical plane 1 contributes pen bit 2;
- physical plane 2 contributes pen bit 1;
- physical plane 3 contributes pen bit 0.

The source bit-address formula and geometry remain unchanged.

## Regression contract

`tests/sprite_gfx_tests.cpp` exhaustively checks all 16 possible physical-plane masks against the expected 4-bit-reversed pen values and also checks geometry/transparency invariants. `Test-SpriteGfx.bat` is the focused Windows entry point.

The packaged `research/scripts/graphics/sprite-bitplane-significance-proveoff.chqscript` performs the post-fix visual/structured prove-off from the canonical frame-2352 checkpoint **without palette overrides**.

## Separate snapshot-tooling issue found in the same bundle

`scene-no-sprites.png` in this RC2.5 snapshot is byte-identical to `final.png` even though `layers/04_sprites.png` is non-empty. This is a separate forensic-export defect and is **not** evidence against the bitplane result. It remains an explicitly tracked tooling issue so the decoder correction can stay isolated.
