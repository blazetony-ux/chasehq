# Active candidate: v0.66.9.0-RC2.7

Authoritative source baseline: supplied v0.66.9.0-RC2.6.1. RC2.7 is a source/research candidate awaiting local Windows/SDL promotion, not a proven Windows release.

Implemented: shared TC0100SCN BG X correction with unchanged Y/origin/zero offsets; same-state legacy/corrected diagnostic mode; correct pre-sprite snapshot base; parsed authoritative history sorting without pre-filter truncation; server-side Current Session queries; full release stamping from src/version.h; bounded local cache and exact selected-run identity; wired SDL controls; live layer offset get/set/reset/assert; checkpoint-frame capture before advancement; requested-artifact completeness checks and CLI identity manifest.

Linux proof: TC0100SCN coordinate/raster/column-zero detection tests PASS; sprite 16-pen decoder PASS; sprite ownership/priority PASS; full CPU/bus/runtime test target compiled and PASS. JavaScript current-session/version/identity test PASS and frontend syntax check PASS. Windows CTest history/evidence tests and full Script Console regressions are bundled but not executed here.

Next gate: `Build-Debug.bat` stages local SDL/ROMs via `Prepare-Project.ps1` and `build-local.json`, compiles Windows/SDL and runs CTest. Then `Start-ChaseHQ.ps1 -Restart`, focused RC2.7 regression, preserved RC2.6.1 sprite regression, and Full Regression through its terminal COMPLETE marker. Inspect all four canonical old/corrected captures and require zero reconstruction mismatches before promotion.

Column scroll remains unsupported beyond the observed zero table and is exposed as such. Semantic TC0100SCN state and pixel-to-tile provenance remain stretch goals with an explicit unimplemented schema plan. No independent shadow backend or SHADOW VERIFIED claim is made.

References: `TC0100SCN_RC261_FINDINGS.md`, `NATIVE_RECONSTRUCTION.md`, `RELEASE_SCOPE_RC27.md`, `../VALIDATION_0.66.9.0-RC2.7.md`. Canonical post-final-hit 9548 is the packaged checkpoint filename; do not rename it to the older near-destruction label.


See `docs/RELEASE_SCOPE_RC27.md` for every approved item.
