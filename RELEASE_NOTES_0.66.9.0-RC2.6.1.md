# ChaseHQ-Native v0.66.9.0-RC2.6.1 — Versioned Sprite Regression Packaging Fix

**Status: DEVELOPMENT CANDIDATE — same emulator/source-decoder semantics as RC2.6; packaging/regression labelling corrected. Windows/SDL proof remains required before promotion.**

RC2.6.1 does not change the RC2.6 sprite-decoder implementation. It corrects the handoff package so the focused regression follows the established build-specific naming and discovery convention used by earlier RC candidates.

## Packaging / regression correction

- Add `research/scripts/regression/regression-v06690-rc261-sprite-bitplane-significance.chqscript`.
- Label it explicitly as **Regression - v0.66.9.0 RC2.6.1 Sprite Bitplane Significance**.
- Give it `regression-version: 0.66.9.0-RC2.6.1` and `regression-scope: current` metadata.
- Include it in `full-regression.chqscript` before the terminal `=== ChaseHQ Full Regression: COMPLETE ===` marker.
- Update the regression README and script catalogue so the current focused regression is visible by version.
- Retain `graphics/sprite-bitplane-significance-proveoff.chqscript` as the separate visual/structured no-palette-override prove-off.

## Renderer semantics

Unchanged from RC2.6: live rendering and export/inspection share the corrected 4bpp decoder where physical planes 0,1,2,3 contribute pen bits 3,2,1,0. `Test-SpriteGfx.bat` remains the exhaustive all-16-pen native assertion.

## Why RC2.6.1 exists

RC2.6 had already been issued as a versioned ZIP. Project release discipline forbids silently replacing that archive under the same version/hash, so this packaging correction is issued as RC2.6.1.

## Remaining Windows/SDL gate

1. `Build-Debug.bat`;
2. `Test-SpriteGfx.bat`;
3. `Test-SpritePriority.bat`;
4. run **Regression - v0.66.9.0 RC2.6.1 Sprite Bitplane Significance**;
5. run **Sprite Bitplane-Significance Native Prove-Off** and inspect `map405-native-plane-order`;
6. run **Regression - Sprite Ownership and Export Coordinates**;
7. run Full Regression through its terminal COMPLETE marker.
