# ChaseHQ-Native v0.66.9.0-RC2.1

RC2.1 is a packaging/build-gate hotfix for RC2. It contains no emulator/runtime behaviour changes.

## Fix

- Added the required `# regression-version:` and `# regression-scope:` metadata to `research/scripts/regression/regression-v06690-rc2-sprite-order-speed-semantics.chqscript`.
- Re-ran an equivalent of the packaged Workbench script-metadata gate across every `.chqscript` before packaging.
- RC2 gameplay, Game Lab, speed semantics, sprite-order tooling, documentation, API/schema and research functionality are otherwise unchanged.

## Why this hotfix exists

RC2 was packaged before its own `Validate-Workbench.ps1` metadata gate was exercised on the final archive tree. Windows correctly stopped the build at preparation time. RC2.1 corrects the metadata and treats the build gate as a release-blocking check.
