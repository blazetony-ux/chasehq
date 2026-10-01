# Validation — v0.66.7.3

Purpose: correct stale Workbench/runtime build-identity metadata discovered while reviewing the successful v0.66.7.2 regression bundle.

## Proven by v0.66.7.2 Windows evidence
- Docs/Knowledge API and search/glossary/handover paths execute successfully.
- Handover text is clean UTF-8.
- HTML docs export is stored inside the active run artifact root and included in the portable run bundle.
- SDL window status/control actions execute successfully.
- Stage 1 checkpoint loads.
- Per-sprite snapshot exports 83 individual forensic sprite assets.
- Layer reconstruction reports `exact=true`, `verifiedExact=true`, `mismatchedPixels=0`.

## v0.66.7.3 correction
- Synchronize Workbench/runtime build literals with native/package version, including snapshot `manifest.json`, `/api/health`, frontend diagnostics/readiness, script history/current-build filters and defaults.
- No Research API semantics or `.chqscript` syntax changed.

## Remaining gate
Run `Regression - v0.66.7.3 Workbench Knowledge / SDL / Sprite Controls` on Windows and confirm snapshot `manifest.json` reports build `0.66.7.3` as well as the already-proven checks above.
