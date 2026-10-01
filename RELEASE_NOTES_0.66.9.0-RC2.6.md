# ChaseHQ-Native v0.66.9.0-RC2.6 — Sprite Bitplane Significance Correction

**Status: DEVELOPMENT CANDIDATE — source correction implemented and native focused tests pass in the non-SDL build environment; fresh Windows/SDL proof is required before promotion.**

RC2.6 is a focused game-facing correction on top of the RC2.5 source baseline. RC2.5's frame-2352 Map-405 causal experiment proved that the native sprite decoder used the correct physical source-plane addresses but assigned their 4bpp significance in reverse. RC2.6 corrects that at the shared source decoder rather than compensating in palette or compositor code.

## Renderer correction

- Add shared `src/sprite_gfx.h` as the single 16x16x4bpp sprite-pixel decode contract used by both live rendering and export/inspection tooling.
- Physical plane 0 contributes pen bit 3, plane 1 -> bit 2, plane 2 -> bit 1, plane 3 -> bit 0.
- Preserve existing tile addressing, geometry, zoom, flip, sprite traversal, first-nontransparent ownership, priority masks and palette contents.
- Preserve transparency semantics: pen 0 remains 0 and every nonzero nibble remains nonzero under the corrected significance mapping.
- Remove duplicated live/export significance logic so map inspection, per-sprite exports and runtime rendering cannot drift silently.

## Causal evidence carried into the build

The RC2.5 `Sprite Bitplane-Significance Causal Probe` ran in session `20260930_175159`, run `008`, at Stage 1 frame `2352`, targeting Map `405` / palette bank `175`. It changed only the palette entries selected by the currently decoded pens to simulate the MAME-consistent 4-bit reversal. Sixteen visible Map-405 sprites became coherent shaded cloud artwork while geometry and priority remained unchanged. The structured snapshot remained exact with `verifiedExact=true` and `mismatchedPixels=0`.

The uploaded evidence bundle SHA-256 is:

`343c14d710023fbb308312900dc453d2eed365409243242c634493aece42c6b2`

A curated evidence report and representative images are packaged under `docs/evidence/sprite-bitplane-significance/`.

## Regression / prove-off additions

- `tests/sprite_gfx_tests.cpp` exhaustively checks all 16 physical-plane combinations against the corrected pen significance and verifies geometry/transparency invariants.
- `Test-SpriteGfx.bat` is the focused Windows test entry point.
- Keep the historical `cpu_bus_rom_tests (SEGFAULT)` exception narrow while allowing the CTest total to grow: `Build-Debug.bat` now requires exactly one failed test but no longer hard-codes the total test count.
- `research/scripts/graphics/sprite-bitplane-significance-proveoff.chqscript` captures the canonical frame-2352 Map-405 scene with **no palette overrides**.
- Existing `Test-SpritePriority.bat` and `regression-sprite-ownership.chqscript` remain mandatory because sprite pens feed occupancy/visibility even though 4-bit reversal preserves zero-vs-nonzero status.

## Documentation / knowledge

- Promote sprite bitplane significance to a confirmed evidence-backed graphics finding.
- Update the sprite hardware/video pipeline reference and sprite discovery status.
- Update project state, roadmap, handover, handbook source, script catalog and feature manifest.
- Correct the top-level README/current-candidate pointer to RC2.6.

## Separate known tooling issue

The RC2.5 causal snapshot exposed that `scene-no-sprites.png` is byte-identical to `final.png` despite a non-empty aggregate sprite layer. That is logged as a separate forensic-export defect and is intentionally **not** mixed into this source-decoder correction.

## Proof status in this packaged handoff

Completed in the available non-SDL environment:

- configure with `CHASEHQ_BUILD_SDL=OFF`;
- compile `sprite_gfx_tests` and `sprite_priority_tests`;
- both focused native tests PASS.

Still required on the user's Windows/SDL development machine before promotion:

1. `Build-Debug.bat`;
2. `Test-SpriteGfx.bat`;
3. `Test-SpritePriority.bat`;
4. run `Sprite Bitplane-Significance Native Prove-Off` and inspect `map405-native-plane-order`;
5. run `Regression - Sprite Ownership and Export Coordinates`;
6. run `Full Regression Suite` through its terminal COMPLETE marker;
7. perform canonical sprite-scene visual checks (player car, roadside sprites, target/enemy, end-level/HUD).
