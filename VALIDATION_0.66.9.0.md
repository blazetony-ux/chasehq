# Validation v0.66.9.0-RC2

Candidate scope: sprite-order diagnostics/A-B controls, sprite-map lookup correctness, Game Lab speed semantics/run-state preservation, frame-safe tile exports, script ERR handling, documentation/script consolidation.

Local static/non-SDL validation performed before packaging:
- C++ configure/build
- native unit tests
- PowerShell parse/static Workbench validation
- browser JavaScript syntax
- JSON/schema consistency
- script library/catalog consistency
- package hygiene and ZIP integrity

Windows/SDL prove-off:
1. `Build-Debug.bat`
2. `Start-ChaseHQ.ps1 -Restart`
3. Run `Regression - v0.66.9.0 RC2 Sprite Order and Speed Semantics`
4. If PASS, use `Player Car Sprite Order A/B` against the captured car-shadow timeline.
5. Full Regression only after focused proof.
