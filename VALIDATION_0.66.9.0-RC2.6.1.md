# Validation — v0.66.9.0-RC2.6.1

## Purpose

Validate the exact RC2.6.1 handoff package. Emulator semantics are unchanged from RC2.6; this patch adds the missing build-specific focused regression and integrates it into Full Regression.

## 1. Static/package gate

```powershell
.\Validate-Release.ps1 -SkipBuild
```

Require terminal `READY TO PACKAGE: YES` and no unexpected failures.

## 2. Canonical Windows build and native guards

```powershell
.\Build-Debug.bat; if ($LASTEXITCODE -eq 0) { .\Test-SpriteGfx.bat }; if ($LASTEXITCODE -eq 0) { .\Test-SpritePriority.bat }
```

Require the sprite 4bpp all-16-pen test and sprite ownership native test to pass. Only the narrowly documented historical `cpu_bus_rom_tests (SEGFAULT)` signature may be tolerated by the existing build gate.

## 3. Version-specific focused regression

Run **Regression - v0.66.9.0 RC2.6.1 Sprite Bitplane Significance**:

`research/scripts/regression/regression-v06690-rc261-sprite-bitplane-significance.chqscript`

Require:

- canonical frame 2352 is loaded;
- Map 405 inspection succeeds;
- `regression-rc261-map405-bitplane` is created and immediately returned by `frame.snapshot.list require=...`;
- the self-comparison completes exactly;
- no palette override is required or left active;
- the terminal RC2.6.1 regression COMPLETE marker is reached.

The exhaustive pen-significance property itself is asserted by `Test-SpriteGfx.bat`; this Script Console regression proves the corrected decoder remains integrated with the canonical renderer/snapshot path.

## 4. Visual/structured prove-off

Run **Sprite Bitplane-Significance Native Prove-Off** (`research/scripts/graphics/sprite-bitplane-significance-proveoff.chqscript`). Inspect `map405-native-plane-order` and require coherent Map-405 cloud shading without palette overrides plus exact structured-snapshot reconstruction.

## 5. Ownership / geometry regression

Run **Regression - Sprite Ownership and Export Coordinates** and retain the established ownership/export-coordinate contract.

## 6. Full Regression

Run **Full Regression Suite** once the focused gate passes. The RC2.6.1 regression must execute before the terminal marker and the run must reach exactly:

`=== ChaseHQ Full Regression: COMPLETE ===`

## 7. Separate known issue

`scene-no-sprites.png` remains a separately tracked snapshot-forensics defect and is intentionally not changed by RC2.6.1.

## Promotion rule

RC2.6.1 may be promoted only after the exact packaged tree passes the Windows/SDL focused regression, visual prove-off, ownership regression and Full Regression.
