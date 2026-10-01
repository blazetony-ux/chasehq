# Active candidate: v0.66.9.0-RC2.7

Authoritative source baseline: supplied v0.66.9.0-RC2.6.1. RC2.7 is a source/research candidate awaiting local Windows/SDL promotion, not a proven Windows release.

Implemented: shared TC0100SCN BG X correction with unchanged Y/origin/zero offsets; same-state legacy/corrected diagnostic mode; correct pre-sprite snapshot base; parsed authoritative history sorting without pre-filter truncation; server-side Current Session queries; full release stamping from src/version.h; bounded local cache and exact selected-run identity; wired SDL controls; live layer offset get/set/reset/assert; checkpoint-frame capture before advancement; requested-artifact completeness checks and CLI identity manifest.

Linux proof: TC0100SCN coordinate/raster/column-zero detection tests PASS; sprite 16-pen decoder PASS; sprite ownership/priority PASS; full CPU/bus/runtime test target compiled and PASS. JavaScript current-session/version/identity test PASS and frontend syntax check PASS. Windows CTest history/evidence tests and full Script Console regressions are bundled but not executed here.

Next gate: `Build-Debug.bat` stages local SDL/ROMs via `Prepare-Project.ps1` and `build-local.json`, compiles Windows/SDL and runs CTest. Then `Start-ChaseHQ.ps1 -Restart`, focused RC2.7 regression, preserved RC2.6.1 sprite regression, and Full Regression through its terminal COMPLETE marker. Inspect all four canonical old/corrected captures and require zero reconstruction mismatches before promotion.

Column scroll remains unsupported beyond the observed zero table and is exposed as such. Semantic TC0100SCN state and pixel-to-tile provenance remain stretch goals with an explicit unimplemented schema plan. No independent shadow backend or SHADOW VERIFIED claim is made.

References: `TC0100SCN_RC261_FINDINGS.md`, `NATIVE_RECONSTRUCTION.md`, `RELEASE_SCOPE_RC27.md`, `../VALIDATION_0.66.9.0-RC2.7.md`. Canonical post-final-hit 9548 is the packaged checkpoint filename; do not rename it to the older near-destruction label.

---
## Historical project material (earlier active labels are retained as history)


# v0.66.9.0-RC2.6.1

- Packaging/regression-label correction only; renderer semantics remain RC2.6.
- Add `regression-v06690-rc261-sprite-bitplane-significance.chqscript` with exact RC2.6.1 regression metadata.
- Include the RC2.6.1 focused regression before Full Regression's terminal COMPLETE marker.
- Update current validation, handover, roadmap, script catalogue and handbook guidance to call the version-specific regression explicitly.
- Preserve `graphics/sprite-bitplane-significance-proveoff.chqscript` as the separate visual/structured no-override prove-off.

# v0.66.9.0-RC2.6

## v0.66.9.0-RC2.6

- Correct Chase H.Q. sprite 4bpp bitplane significance at the shared source decoder: physical planes 0,1,2,3 now contribute pen bits 3,2,1,0.
- Replace duplicated live/export pen decode logic with shared `src/sprite_gfx.h`.
- Add exhaustive `sprite_gfx_tests` coverage for all 16 source-plane combinations plus geometry/transparency invariants and `Test-SpriteGfx.bat`.
- Keep the historical `cpu_bus_rom_tests (SEGFAULT)` exception narrow while allowing the CTest total to grow: `Build-Debug.bat` now requires exactly one failed test but no longer hard-codes the total test count.
- Package `sprite-bitplane-significance-proveoff.chqscript` for a canonical frame-2352 no-palette-override structured visual gate.
- Carry the RC2.5 causal evidence into `docs/evidence/sprite-bitplane-significance/`, including bundle SHA-256 and representative output.
- Promote the finding into machine-readable knowledge, handbook source, feature manifest and sprite hardware/video reference docs.
- Preserve RC2.5 sprite traversal/first-nontransparent occupancy semantics; no palette/compositor workaround is introduced.
- Track the independently observed `scene-no-sprites.png` forensic-export defect separately; do not bundle that fix into the decoder correction.
- Non-SDL focused native tests pass in the handoff environment; fresh Windows/SDL focused proof and Full Regression remain mandatory before promotion.

- Correct Chase H.Q. same-priority sprite overlap semantics: preserve descending MAME-reference traversal, but reserve each pixel for the first nontransparent sprite candidate, including candidates blocked by background priority.
- Keep opaque black sprite pens opaque; do not solve the player shadow by making black transparent or globally reversing traversal.
- Correct individual-sprite forensic export Y coordinates to share the live renderer's visible-raster origin.
- Add `sprite-ownership.csv`, `frame.snapshot.assert-overlap`, offline PowerShell assertion parity, native synthetic ownership tests, and permanent canonical frame-2352 regression coverage.
- Add Research Script `try ... finally ... end` cleanup blocks and idempotent `timeline.reset` for script-initiated timeline transactions.
- Correct nested block variable expansion so loops/conditionals inside `try` blocks are compiled with their local variables rather than being substituted prematurely.
- Add schema/Script Console allow-list consistency enforcement so documented API actions cannot silently remain unreachable.
- Correct diagnostic bundle script attribution to the actual pasted/library source rather than stale selected-library state.
- Default Script Console run-history filtering to the active session while retaining version/session/all-history filters.
- Strengthen `AGENTS.md` with safe experiment lifecycle, evidence identity, pre-existing-failure proof, API exposure parity, and manual-hotfix reconciliation rules.
- Package a safe TC0100SCN rowscroll raster-mapping probe for the next graphics investigation.
- Windows proof carried into this candidate for the sprite correction: focused frame-2352 assertion and Full Regression PASS. Fresh Windows proof is still required for the new RC2.5 scripting/Workbench changes before promotion.

## v0.66.9.0-RC2.4.4

- Windows evidence: RC2.4.3 assertive same-turn snapshot discovery passed.
- Full Regression preflight then failed before execution because the permanent suite expands to 503 commands against the historical 500-command ceiling.
- Raise expanded-command capacity to 1000 consistently across client/server/schema/reference.
- Replace stale hard-coded `0.66.8.0` startup/research-session metadata with project-version derivation.
- Add root `AGENTS.md` for Codex/agent operating rules and carry it in future packages.
- Next Windows gate: Full Regression must reach its terminal COMPLETE marker.

# v0.66.9.0-RC1 implemented candidate

- Game Lab: infinite turbo stock, infinite active duration, timer reset, count set, turbo fire, current speed set/freeze/clear.
- Game Lab presentation: grouped cards, more button spacing/padding, live state readouts and clearer reversible-experiment descriptions.
- TC0100SCN RAM character inspector with raw pens, palette-aware PNG and JSON metadata.
- Reusable turbo/HUD scripts promoted from chat experiments into the packaged script library.
- Timeline write/checkpoint frame-boundary semantics documented.
- Focused regression added; Windows/SDL proof pending.

# Next development changelog — after v0.66.8.0

Current proven baseline: **v0.66.8.0 (2026-09-29)**.

Do not create a successor merely for release/process polish. Accumulate changes until there is a coherent game-fidelity improvement, emulator fix, or evidence-driven research-tool gap.

Candidate next work:
- turbo HUD/source/compositor causal investigation using the existing timeline;
- target final-hit/end-level timeline analysis;
- authoritative gameplay-state expansion (score, on/off-road, turbo remaining, police light, incline/decline);
- graphics alignment/layer-priority fixes and deterministic visual-regression cases;
- optional timeline scrubber/browse UI only if it materially improves ongoing investigations;
- authentic audio runtime remains a longer-term subsystem.

Every future build must update API/script docs, knowledge, handover, project state, roadmap, regression coverage, and package validation when its surface changes.

## v0.66.9.0-RC2
- Correct `sprite.map.inspect map=N`.
- Add live same-priority sprite-order inspect/set and Game Lab UI.
- Clarify HUD vs internal speed and preserve run state for Game Lab writes/freezes.
- Make tile-inspector outputs frame-safe.
- Fail scripts on unhandled native `ERR`.
- Package reusable turbo-animation and car-shadow/order scripts.

## v0.66.9.0-RC2.2
- Correct native Chase H.Q. equal-priority sprite traversal to MAME-reference descending traversal. (Historical note: RC2.2 still used overwrite semantics; RC2.5 later adds the missing first-nontransparent occupancy rule.)
- Retain higher-slot override for forensic A/B work.
- Add loop-based multi-scene proof and regression coverage.


## v0.66.9.0-RC2.3
- Build-gate-only correction: move the RC2.2 sprite-order regression include before the Full Regression terminal COMPLETE marker.
- No runtime/API/script-language behaviour changes.


## v0.66.9.0-RC2.4
- Correct TC0100SCN BG0/BG1/TEXT Y-scroll source-coordinate sign.
- Remove historical BG0 `0:-4` / BG1 `0:-20` default presentation compensation; defaults are now neutral.
- Keep explicit layer offsets as forensic-only overrides.
- Add shared source-Y helper and native regression for the proven `CTRL3/4=0x01E0` wrapped coordinate.
- Document the RC2.3 A/B proof and mark the turbo-character rendering question resolved as intentional animation.
- No API/schema/script-language changes.

## v0.66.9.0-RC2.4.1
- Launcher-only hotfix: when Windows HTTP.sys/PID 4 owns the configured Workbench port, restart cleanup identifies the actual stale `Start-ChaseHQWeb.ps1` host by `-HttpPort` across sibling build folders rather than limiting search to the current build path.
- Native emulator/video/API semantics unchanged.

## v0.66.9.0-RC2.4.2
- Attempted immediate structured-snapshot discovery repair by invalidating snapshot/root caches after capture.
- Bundled deterministic post-Y-fix layer isolation and a current-build regression.
- Windows evidence subsequently showed the cache-only fix was insufficient and the regression was non-assertive; superseded by RC2.4.3.

## v0.66.9.0-RC2.4.3
- Add an explicit same-process recent structured-snapshot registry merged into list and named-resolution paths.
- Invalidate structured-snapshot, snapshot-root, and known-session-root caches after capture.
- Add optional `frame.snapshot.list require=NAME`; it throws if the requested new snapshot is not returned.
- Add assertive RC2.4.3 regression and include it before the terminal Full Regression COMPLETE marker.
- Reclassify the observed sky pattern as raw TC0100SCN source-layer presentation rather than a compositor target based on the RC2.4.2 layered evidence.
