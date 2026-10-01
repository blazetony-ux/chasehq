# Active candidate: v0.66.9.0-RC2.7

Authoritative source baseline: supplied v0.66.9.0-RC2.6.1. RC2.7 is a source/research candidate awaiting local Windows/SDL promotion, not a proven Windows release.

Implemented: shared TC0100SCN BG X correction with unchanged Y/origin/zero offsets; same-state legacy/corrected diagnostic mode; correct pre-sprite snapshot base; parsed authoritative history sorting without pre-filter truncation; server-side Current Session queries; full release stamping from src/version.h; bounded local cache and exact selected-run identity; wired SDL controls; live layer offset get/set/reset/assert; checkpoint-frame capture before advancement; requested-artifact completeness checks and CLI identity manifest.

Linux proof: TC0100SCN coordinate/raster/column-zero detection tests PASS; sprite 16-pen decoder PASS; sprite ownership/priority PASS; full CPU/bus/runtime test target compiled and PASS. JavaScript current-session/version/identity test PASS and frontend syntax check PASS. Windows CTest history/evidence tests and full Script Console regressions are bundled but not executed here.

Next gate: `Build-Debug.bat` stages local SDL/ROMs via `Prepare-Project.ps1` and `build-local.json`, compiles Windows/SDL and runs CTest. Then `Start-ChaseHQ.ps1 -Restart`, focused RC2.7 regression, preserved RC2.6.1 sprite regression, and Full Regression through its terminal COMPLETE marker. Inspect all four canonical old/corrected captures and require zero reconstruction mismatches before promotion.

Column scroll remains unsupported beyond the observed zero table and is exposed as such. Semantic TC0100SCN state and pixel-to-tile provenance remain stretch goals with an explicit unimplemented schema plan. No independent shadow backend or SHADOW VERIFIED claim is made.

References: `TC0100SCN_RC261_FINDINGS.md`, `NATIVE_RECONSTRUCTION.md`, `RELEASE_SCOPE_RC27.md`, `../VALIDATION_0.66.9.0-RC2.7.md`. Canonical post-final-hit 9548 is the packaged checkpoint filename; do not rename it to the older near-destruction label.

---
## Historical project material (earlier active labels are retained as history)

# v0.66.9.0-RC2.6.1 current candidate

## Immediate gate — v0.66.9.0-RC2.6.1

1. Windows build the exact RC2.6.1 tree and require `sprite_gfx_tests` plus existing ownership tests to pass.
2. Run `regression-v06690-rc261-sprite-bitplane-significance.chqscript` as the explicit build-specific focused regression.
3. Run `graphics/sprite-bitplane-significance-proveoff.chqscript` at canonical frame 2352. Require coherent Map-405 clouds with **no palette overrides** and exact structured-snapshot reconstruction.
4. Re-run sprite-ownership/export-coordinate regression to prove the pen correction did not disturb traversal, occupancy or geometry.
5. Survey representative sprite scenes: player car, roadside objects, target/final-hit state, end-level state; verify HUD/text remains unaffected.
6. Run Full Regression once and require the terminal COMPLETE marker.
7. Only after that proof, promote the decoder correction and resume the next graphics investigation.

### Follow-up tooling item
Fix and regress `scene-no-sprites.png`: the RC2.5 causal bundle showed that current export is identical to `final.png` despite an active sprite layer. Keep this separate from RC2.6's decoder change unless new evidence makes it blocking.

### Native-reconstruction maturity
Sprite pixel source interpretation advances from **MAPPED** to **BEHAVIOUR UNDERSTOOD / NATIVE-CANDIDATE**: plane addresses, significance, transparency and ownership interactions are now evidence-backed and isolated behind a shared semantic decoder. Future shadow/compare work should consume this shared contract rather than address-only assumptions.

RC2.5 consolidates the proven player-car sprite-ownership correction and the scripting/research reliability gaps exposed while starting the TC0100SCN sky/rowscroll investigation.

**Sprite correction is already Windows-proven on the RC2.4.4 working tree:** canonical frame 2352 reports body slot 82/map 475 ahead of shadow slot 81/map 575; `overlapPixels=689`, `frontOwnedOverlap=553`, `backVisibleOutsideFront=247`, `exportCoordinatesExact=true`. The focused ownership regression passed and the permanent Full Regression subsequently reached `=== ChaseHQ Full Regression: COMPLETE ===` in session `20260930_151202`, run `002` (118.824 s). The pristine RC2.4.4 control build independently reproduces the historical `cpu_bus_rom_tests` SEGFAULT, so that failure is pre-existing and unrelated to the sprite change.

RC2.5 additionally adds safe Research Script lifecycle support (`try ... finally ... end`, idempotent `timeline.reset`), schema/Script-Console allow-list parity checking, corrected diagnostic script attribution, current-session history as the default view, richer script-header guidance, and a packaged safe rowscroll raster-mapping probe. These new Workbench/script features require a fresh RC2.5 Windows regression gate before this package is promoted from candidate to proven.

# v0.66.9.0-RC2.5 immediate roadmap

1. **Windows release gate:** build/start RC2.5; run `Regression - v0.66.9.0 RC2.5 Script Safety`, `Regression - Sprite Ownership and Export Coordinates`, then `Full Regression Suite` to terminal COMPLETE.
2. **Workbench safety:** verify Script Console history defaults to the current session and `frame.snapshot.assert-overlap` / `timeline.reset` are both schema-visible and executable.
3. **Script cleanup semantics:** verify `try/finally` restores the canonical frame after timeline seek/fork; do not claim user-abort cleanup proven until exercised on Windows.
4. **Graphics next:** run the packaged `TC0100SCN Rowscroll Raster-Mapping Probe`. Existing two-frame evidence already shows raw `bg-bottom` sky motion follows rowscroll RAM; the remaining question is rowscroll-entry-to-visible-raster mapping.
5. **No new sprite-order work:** the frame-2352 player shadow ownership defect is closed unless new contradictory evidence appears.

# v0.66.9.0-RC2.4.4 immediate reliability + game-fidelity roadmap

1. **Windows gate:** build/start RC2.4.4 and run Full Regression. The permanent suite currently expands to 503 commands; the RC2.4.4 engine ceiling is 1000. Require the terminal `=== ChaseHQ Full Regression: COMPLETE ===`.
2. **Metadata sanity:** startup/research session build fields must report the source build (`0.66.9.0`) rather than stale `0.66.8.0`.
3. **Snapshot discovery:** considered Windows-proven from the uploaded RC2.4.3 focused regression; do not spend another cycle reproving it unless a fresh package changes that path.
4. **Graphics next:** return to player-car shadow/under-car ordering using the existing layered/per-sprite evidence workflow. Keep sky tuning separate unless direct reference evidence proves a source-layer mismatch.
5. **Agent workflow:** carry root `AGENTS.md` in every future package; add nested AGENTS files only when observed Codex behavior demonstrates a real need.

# v0.66.9.0-RC2.4.3 immediate reliability + game-fidelity roadmap

1. Prove same-turn snapshot discovery on Windows with `Regression - v0.66.9.0 RC2.4.3 Snapshot Discovery`; the `frame.snapshot.list require=regression-rc243-snapshot-discovery` command must make the run fail if the new snapshot is absent.
2. Run `Validate-FrameSnapshotRefresh.ps1` and retain the existing `Validate-TC0100SCN-Y.ps1` byte-exact frame-2065 gate.
3. Treat RC2.4.2's sky pattern as localized to the raw TC0100SCN background source, not the compositor. Do not tune renderer geometry merely to remove artwork/source dithering unless a direct reference comparison proves a mismatch.
4. Return substantive graphics work to the real remaining anomalies: player-car shadow/under-car ordering first, then independently proven roadside/building X placement/layer assignment or compositor discrepancies.
5. Keep BG presentation offsets neutral. Investigate TC0100SCN X-scroll/rowscroll semantics only with controlled evidence.

# v0.66.9.0-RC1 immediate game-fidelity roadmap

1. Prove the new TC0100SCN character inspector on Windows/SDL and directly compare normal turbo characters `0x7C..0x7F` with active characters `0x0C..0x0F`.
2. If those active characters decode correctly, close the turbo-count rendering issue as intentional game logic unless palette/compositor evidence contradicts it.
3. Use Game Lab turbo/speed controls for sustained deterministic experiments without repeated manual gameplay.
4. Move next to the remaining visible renderer issues: sky/top visible area, car-shadow priority, roadside/building X/Y placement and layer assignment.
5. Keep the timeline-first rule: use an existing `.chqtimeline` before replaying gameplay; fork-live only for causal experiments.

# Post-v0.66.8.0 roadmap — return to game fidelity

The v0.66.8.0 release gate is closed. Current proven baseline: **v0.66.8.0**.

Immediate priorities:

1. Use the existing turbo forensic timeline to continue the **turbo HUD/source/compositor** investigation, automatically correlating known turbo state with text/tile/palette/sprite changes and writer routines.
2. Reuse timeline analysis for the canonical target/final-hit and end-level checkpoints to map causal state transitions without repeatedly replaying gameplay.
3. Continue authoritative gameplay-state discovery and expose useful confirmed state consistently in UI/API/logs: score, road state, turbo remaining, police light, incline/decline and related Stage 1 state.
4. Continue graphics fidelity work: missing HUD turbo indicators, sky/top visible-area behaviour, car shadow/layer priority, building offsets/layer assignment, sprite provenance and layered reconstruction.
5. Expand visual-regression use of deterministic layered snapshots and per-sprite evidence.
6. Longer-term: sprite upscale/high-resolution replacement experiments, possible 3D-model substitution research, whole-course map/minimap work, cross-game comparative reverse engineering and authentic audio implementation.

Process rule: use the proven v0.66.8.0 tooling aggressively first; cut a new build only for a genuine fix, meaningful gameplay/rendering improvement, or coherent tooling gap exposed by evidence.

# Historical v0.66.8.0 qualification roadmap

## Completed priority: prove Forensic Timeline v1

- [ ] Windows build/startup.
- [ ] Focused timeline regression.
- [ ] Capture one real turbo event into `turbo-complete-cycle.chqtimeline`.
- [ ] Trim the broad capture to exact event edges using `0x10040F` bit 1.
- [ ] Run offline whole-write-stream ranking.
- [ ] Restart/import the same recording and inspect memory/register/sprites/palette/gameplay without replaying gameplay.
- [ ] Fork one recorded frame back to LIVE and confirm counterfactual stepping/writes work.
- [ ] Full Regression once after focused proof.

## Follow-on once v1 is proven

- keyframe+delta compression while preserving the v1 script/API contract;
- richer offline temporal signatures and control-vs-experiment comparison;
- Workbench timeline scrubber;
- authentic audio event/PCM capture only after the native sound runtime exists.

---

# Immediate roadmap — v0.66.7.9

## Release/recovery gate — COMPLETE

- [x] Built through `Build-Debug.bat`.
- [x] Launched with `Start-ChaseHQ.ps1 -Restart`; native API, Workbench and browser bootstrap reached READY.
- [x] `Recovery / Fast Release Gate` passed.
- [x] `Regression - v0.66.7.9 Research Recovery` passed.
- [x] `Full Regression Suite` passed against the clean v0.66.7.9 candidate.
- [x] Final promotion is from the clean packaged candidate, not a hand-edited RC.

## Substantive research priority

Turbo gameplay state is already confirmed. The next task is to localize the **bottom turbo HUD rendering path**, using the packaged causal-state and before/during/after HUD scripts. After that, trace visual producer/writer paths only where needed, then move to target health/defeat/end-level transition and track branch/topology naming. See `docs/RESEARCH_RECOVERY_PLAN.md`.

## Release discipline

`docs/RELEASE_PROCESS.md` is now canonical. It records the one-run/one-bundle lifecycle, bounded/cached run/snapshot discovery, developer handoff packaging policy, strict-mode/parser validation, staged prove-off gates and the rule that Full Regression is the final expensive gate rather than the first diagnostic step.

---

# Immediate roadmap — v0.66.7.7

## v0.66.6.1 layered graphics forensics

- Reconstructable frame snapshots are now first-class evidence: exact alpha contribution layers plus raw hardware-source renders.
- HUD capture modes are Normal / HUD Hidden / HUD Only, implemented by suppressing the identified text/HUD source rather than masking coordinates.
- Graphics Lab can inspect the saved snapshot, reconstruct contribution layers, toggle/solo layers and vary opacity.
- Image compare/diff accepts HUD selection for deterministic scene comparisons without HUD noise.
- Next research use: turbo/HUD indicator investigation, sprite/compositor priority anomalies, brake-lamp visual causality, and repeatable frame comparisons.
- Continue to evolve the snapshot metadata as additional authoritative mixer/palette/source state becomes available; do not break v2 consumers without a schema bump.

## v0.66.5.1 memory trace / Track View / Workbench polish

- Added generic memory-write provenance tracing with exact CPU/PC/address/width/old/new/change/count events and Script/API contracts.
- Added the Stage 1 `0x100303` processed-input writer investigation script and same-build regression coverage.
- Removed the low-value Script Console Syntax Preview panel.
- Track View now auto-fits as the path grows, offers an explicit Fit Track control, and can save the current vector view as SVG.
- Build-Debug/Build-Release still run `Prepare-Project.bat` first (including PowerShell unblocking), but tolerate only the exact historical `cpu_bus_rom_tests (SEGFAULT)` 1-of-1 CTest signature; all other test failures remain blocking.
- Restart cleanup never terminates PID 4/System and attempts to stop the stale `Start-ChaseHQWeb.ps1` host when HTTP.sys owns the listener.


## v0.66.4 palette trace / release reliability

- Palette-write provenance is now first-class: selected TC0110PCR entries can be traced with frame, CPU, exact writer PC, index/bank/pen, complete old/new raw values, changed flag and write count.
- Script API actions: `palette.trace.start`, `palette.trace.status`, `palette.trace.tail`, `palette.trace.stop`, `palette.trace.clear`.
- The immediate target is Chase H.Q. brake-lamp entries 1037/1053.
- Workbench run duration now preserves the first request's start time across a multi-command run.
- Run ZIPs remain physically `bundle.zip` but download with a descriptive session/run/script filename.
- Release packaging is gated by `Validate-Release.ps1`.

1. Validate v0.66.4 on Windows with `Validate-Release.ps1`, restart smoke, the v0.66.4 palette-trace regression, then Full Regression.
2. Use `palette.trace.start indices=1037,1053` around the proven Stage 1 brake transition and capture exact writer PC(s).
3. Disassemble the writer routine and compare brake-only versus brake-during-turbo conditions.
4. Promote any newly proven brake-lamp/control state into the gameplay registry only after the write-decision path is authoritative.
5. Continue gameplay work: turbo/HUD indicators, target health/damage, end-level transition and track branch/topology.
6. Keep API/script documentation release-blocking and synchronized with the authoritative schema.
7. Default workflow remains: investigate/remediate locally -> validate -> native rebuild only if required -> package only after proof.

## v0.66.5.1 Track Recorder scripting hotfix

- Shared server-side Track View recorder backs both browser controls and Research Script.
- Added start/stop/clear/status/sample actions plus deterministic SVG export.
- Empty SVG exports are rejected.
- Memory-trace documentation/regression now uses `width=any` when the producer transaction width is unknown.
- `Build-Debug.bat` remains the canonical entry point, still runs `Prepare-Project.bat` first to unblock PowerShell files, and still tolerates only the exact historical `cpu_bus_rom_tests (SEGFAULT)` signature.

## v0.66.7 immediate gate
- [ ] Windows SDL Debug build succeeds.
- [ ] Docs/Knowledge tab loads/searches/filter entries and evidence.
- [ ] `docs.*` / `knowledge.*` actions work from `.chqscript`.
- [ ] HTML export works; PDF export works when Edge is present or reports the missing capability clearly.
- [ ] Graphics Lab individual-sprite mode loads cropped sprite assets and supports hide/show/solo/opacity.
- [ ] Workbench SDL controls and `window.*` API actions report/change native state correctly.
- [ ] Authoritative `CHATGPT_HANDOVER_PROMPT.md` matches package version/proof status.
- [ ] Full/permanent regression remains green.

After this gate, return to turbo/HUD investigation and use the new knowledge/evidence workflow for findings as they are proven.


## v0.66.7.3 build identity correction
The v0.66.7.2 Windows regression passed functionality, UTF-8 handover delivery, portable docs export, individual-sprite capture, and exact layered reconstruction. Evidence review found stale `0.66.7.1` identifiers in Workbench-host metadata. v0.66.7.3 synchronizes current build identity across native/runtime/Workbench snapshot and frontend metadata; no API or script semantics change.
## v0.66.7.5 prove-off gate
Before resuming turbo/HUD research, prove that native SDL title, Workbench header, `/api/health`, OpenAPI metadata, snapshot manifest and run metadata all report v0.66.7.5 consistently, then run the current v0.66.7.5 regression.


## Documentation platform milestone
The v0.66.7.4 knowledge/documentation foundation is proven. v0.66.7.5 reworks the user-facing Docs / Knowledge tab into a readable User & Research Handbook with authored guides, complete schema-generated API and `.chqscript` references, grouped cross-domain search, copyable examples, richer project-knowledge pages, and full handbook HTML/PDF export. `research/knowledge/handbook.json` is now carried and versioned alongside `research/knowledge/documentation.json`. v0.66.7.5 awaits Windows/Workbench regression proof.


## v0.66.7.6 release-candidate gate
- [ ] `Validate-Release.ps1` passes on Windows.
- [ ] Debug native build/tests complete; only the exact documented historical `cpu_bus_rom_tests (SEGFAULT)` signature may be tolerated.
- [ ] Docs / Knowledge loads without startup errors and all five documentation modes work.
- [ ] UI search finds article-only phrases; `docs.search` and `knowledge.search` return the same intended entries.
- [ ] Full HTML export contains article bodies and no empty reference sections.
- [ ] Final PDF is regenerated from the same post-fix export path and spot-checked.
- [ ] ChatGPT handover display/copy/download returns the complete current prompt.
- [ ] SDL pause/resume/step and show/hide/minimize/restore/fullscreen/scale/always-on-top controls are exercised with API parity.
- [ ] Graphics Lab layered reconstruction remains exact and individual-sprite hide/show/solo/opacity/cropped PNG metadata paths work.
- [ ] Major Workbench tabs receive one smoke pass and the full regression completes.
- [ ] Final package contains canonical checkpoints and no ROM/audio/debug-backup files.

After this gate, return to turbo/HUD gameplay research rather than expanding documentation tooling further.


## v0.66.7.7 prove-off gate

- Confirm Recent Script Runs shows completed on-disk runs, including Full Regression Suite, after page reload/restart.
- Confirm Graphics Lab lists and opens `regression-v06676-docs-consolidation` (or another completed-run snapshot) after the originating run has finished.
- Run `Regression - v0.66.7.7 Run History / Snapshot Discovery`, then Full Regression.


### v0.66.7.8 RC2
Cross-version run/snapshot discovery performance hotfix: avoid recursive evidence scans that can monopolize the Workbench backend; cache discovery results briefly. Full SDL source is no longer bundled in the RC package.

## v0.66.8.0 RC6 legacy timeline compatibility note
RC5 exposed two compatibility defects in offline recovery: the recovery script used `trigger=`/`stop=` while the documented action expected `triggerFrame=`/`stopFrame=`, and the RC2-destructively-rewritten `writes.csv` contains quoted CSV fields (`"2064"`) whereas the streaming parser assumed native unquoted fields. RC6 accepts both parameter spellings, uses canonical names in the packaged recovery script, strips optional CSV quotes during streaming parsing, and gives explicit trigger/stop bounds precedence over any stale event-window index. Do not replay turbo gameplay; reuse the existing recording.

## Immediate graphics/gameplay work after v0.66.9.0-RC2
1. Prove the sprite-order A/B control on the existing car-shadow timeline.
2. If descending traversal fixes the under-car effect, trace authentic hardware/MAME sprite traversal and promote the correct default.
3. If not, continue into priority-mask/PROM tie-breaking with the same captured timeline.
4. Identify the actual top-speed clamp/target path; do not conflate it with raw internal speed `0x10041C`.

### RC2.1 build-gate correction
- [x] Repair mandatory metadata on the v0.66.9.0 RC2 regression script.
- [ ] Prove RC2 gameplay/sprite-order/speed tooling on Windows/SDL after the clean build gate.

### Completed in v0.66.9.0-RC2.2
- [x] Historical RC2.2 conclusion superseded: frame-2352 focused regression proves first-nontransparent occupancy; see SPRITE_OWNERSHIP.md.
- [x] Make descending traversal with first-nontransparent ownership the native default while retaining A/B override.
- [x] Add loop-based multi-scene order validation and focused regression coverage.


### RC2.3 release-gate correction
- [x] Move the RC2.2 sprite-order regression before the Full Regression terminal COMPLETE marker.
- [x] Pre-package verify script includes, metadata, JSON, version identity, package exclusions, and the exact final-marker contract.
- [ ] Run `Build-Debug.bat` on Windows, then the focused RC2.2 sprite-order regression.

### v0.66.9.0-RC2.4.1 launcher hotfix
- Fixed cross-version `-Restart` cleanup for HTTP.sys-owned Workbench ports: stale `Start-ChaseHQWeb.ps1` hosts are now matched by requested `-HttpPort` / backend port rather than only the current extracted build path.


### RC2.4.2 completed
- [x] Fix same-turn `frame.snapshot` -> `frame.snapshot.list` stale-cache discovery.
- [x] Bundle a deterministic post-TC0100SCN-Y-fix layer-isolation script.
- [x] Add current-build regression coverage for the corrected graphics/snapshot path.
