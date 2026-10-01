# Validation — v0.66.9.0-RC2.6

## Purpose

Prove the source-level Chase H.Q. sprite 4bpp plane-significance correction while retaining RC2.5 sprite ownership, geometry, scripting and Workbench behaviour.

RC2.6 must not be promoted from candidate on documentation or the Linux/non-SDL unit-test result alone. The renderer change requires a fresh Windows/SDL build and structured visual proof.

## 1. Static/package gate

From the RC2.6 source root:

```powershell
.\Validate-Release.ps1 -SkipBuild
```

Require terminal `READY TO PACKAGE: YES` and no unexpected failures.

## 2. Canonical Windows build

```powershell
.\Build-Debug.bat
```

Requirements:

- native/SDL compile succeeds;
- `sprite_gfx_tests` passes;
- `sprite_priority_tests` passes;
- only the precisely documented historical `cpu_bus_rom_tests (SEGFAULT)` signature may be tolerated by the existing build gate;
- no new test failure is acceptable.

Then run the focused low-level decoder test explicitly:

```powershell
.\Test-SpriteGfx.bat
```

Expected terminal line:

`PASS Chase H.Q. sprite 4bpp plane significance (all 16 pens)`

Also retain the RC2.5 ownership guard:

```powershell
.\Test-SpritePriority.bat
```

## 3. Start the exact candidate

Use the normal project launcher from the RC2.6 tree and confirm the SDL runtime/Workbench report native build `0.66.9.0` and that evidence paths belong to the current RC2.6 working tree/session.

## 4. Focused visual/structured prove-off

Run the packaged script **Sprite Bitplane-Significance Native Prove-Off** (`research/scripts/graphics/sprite-bitplane-significance-proveoff.chqscript`).

Required observations from `map405-native-plane-order`:

- frame 2352 / Map 405 loads successfully from the canonical checkpoint;
- cloud sprites are coherent multi-shade artwork without any palette override;
- sprite geometry/placement is not altered by the fix;
- `reconstruction.json` reports `verifiedExact=true` and `mismatchedPixels=0`;
- per-sprite Map-405 assets agree with the aggregate sprite layer's pen interpretation.

Do **not** rerun the RC2.5 palette-remap causal script as fixed-build validation: on RC2.6 that intervention would intentionally remap already-correct pens.

## 5. Ownership / geometry regression

Run **Regression - Sprite Ownership and Export Coordinates**.

Require the existing canonical overlap assertions to remain nonzero and `exportCoordinatesExact=true`. The exact overlap counts may be compared with RC2.5 evidence, but the key contract is that the bitplane correction must not reintroduce lower-slot overwrite, black-transparency hacks or export-Y drift.

## 6. Canonical sprite-scene survey

Using existing packaged checkpoints/scripts, inspect at minimum:

- Stage 1 frame 2352 player car + cloud family;
- roadside/environment sprites;
- `stage1-target-post-final-hit-9548.chqstate` target/enemy presentation;
- `stage1-end-level-9988.chqstate` end-level presentation;
- HUD/text remains unaffected by the sprite-source decode correction.

Capture structured snapshots when a discrepancy is found. Do not tune palette, offsets or compositor state to mask a source-decoder error.

## 7. Full Regression

Run **Full Regression Suite** once after the focused prove-off passes.

Requirements:

- no unexpected `ERROR:` output;
- all permanent regressions complete;
- final command/output reaches exactly the terminal marker:

`=== ChaseHQ Full Regression: COMPLETE ===`

## 8. Known separate issue

The RC2.5 causal evidence showed `scene-no-sprites.png` byte-identical to `final.png` despite a non-empty sprite contribution layer. Treat this as a separate snapshot-forensics defect. It is not a reason to alter the decoder fix during RC2.6 validation.

## Promotion rule

RC2.6 may be promoted only after the Windows/SDL focused prove-off and Full Regression pass on the exact packaged tree. Update project-state/proof metadata with the resulting session/run identities before any proven-release promotion.
