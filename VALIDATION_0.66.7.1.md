# Validation — v0.66.7.2

Status: IMPLEMENTED — awaiting corrected Windows/SDL prove-off.

## Reason for hotfix
The first v0.66.7 Windows run successfully exercised Docs/Knowledge schema/index/search/glossary/handover and SDL window status/fullscreen/scale/restore/show, then stopped because the regression used `api checkpoint.load name=stage1-driving-2352.chqstate`. The current API contract requires `file=...`. The Workbench screenshot also exposed `Â·` mojibake in individual-sprite labels.

## Fixes
- Regression now uses `api checkpoint.load file=stage1-driving-2352.chqstate run=false`.
- Sprite-control labels use ASCII `|` separators.
- Regression is versioned/current as v0.66.7.2 and full-regression wiring follows it.
- No Research API semantics and no `.chqscript` syntax changed.

## Remaining Windows gate
Build with `Build-Debug.bat`, launch the standard SDL + Workbench pair, run `Regression - v0.66.7.2 Workbench Knowledge / SDL / Sprite Controls`, and return the evidence bundle. Success must reach per-sprite `frame.snapshot`, snapshot listing, HTML docs export, and final window status.

## Validation performed in packaging environment
- CPU/non-SDL configure: PASS.
- Native build: PASS.
- CTest: 1/1 PASS (`cpu_bus_rom_tests`).
- Windows/SDL build and corrected current regression remain the required external prove-off.
