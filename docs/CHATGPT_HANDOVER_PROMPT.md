# Active candidate: v0.66.9.0-RC2.7

Authoritative source baseline: supplied v0.66.9.0-RC2.6.1. RC2.7 is a source/research candidate awaiting local Windows/SDL promotion, not a proven Windows release.

Implemented: shared TC0100SCN BG X correction with unchanged Y/origin/zero offsets; same-state legacy/corrected diagnostic mode; correct pre-sprite snapshot base; parsed authoritative history sorting without pre-filter truncation; server-side Current Session queries; full release stamping from src/version.h; bounded local cache and exact selected-run identity; wired SDL controls; live layer offset get/set/reset/assert; checkpoint-frame capture before advancement; requested-artifact completeness checks and CLI identity manifest.

Linux proof: TC0100SCN coordinate/raster/column-zero detection tests PASS; sprite 16-pen decoder PASS; sprite ownership/priority PASS; full CPU/bus/runtime test target compiled and PASS. JavaScript current-session/version/identity test PASS and frontend syntax check PASS. Windows CTest history/evidence tests and full Script Console regressions are bundled but not executed here.

Next gate: `Build-Debug.bat` stages local SDL/ROMs via `Prepare-Project.ps1` and `build-local.json`, compiles Windows/SDL and runs CTest. Then `Start-ChaseHQ.ps1 -Restart`, focused RC2.7 regression, preserved RC2.6.1 sprite regression, and Full Regression through its terminal COMPLETE marker. Inspect all four canonical old/corrected captures and require zero reconstruction mismatches before promotion.

Column scroll remains unsupported beyond the observed zero table and is exposed as such. Semantic TC0100SCN state and pixel-to-tile provenance remain stretch goals with an explicit unimplemented schema plan. No independent shadow backend or SHADOW VERIFIED claim is made.

References: `TC0100SCN_RC261_FINDINGS.md`, `NATIVE_RECONSTRUCTION.md`, `RELEASE_SCOPE_RC27.md`, `../VALIDATION_0.66.9.0-RC2.7.md`. Canonical post-final-hit 9548 is the packaged checkpoint filename; do not rename it to the older near-destruction label.

---
## Historical project material (earlier active labels are retained as history)

# Authoritative continuation — ChaseHQ-Native

**Build:** v0.66.9.0-RC2.6.1 — Sprite Bitplane Significance regression-packaging candidate

## Immediate next task
On Windows, build the exact RC2.6.1 package. Run `Test-SpriteGfx.bat`, then `Test-SpritePriority.bat`, then `Regression - v0.66.9.0 RC2.6.1 Sprite Bitplane Significance`, then the packaged `Sprite Bitplane-Significance Native Prove-Off`. Inspect `map405-native-plane-order` and require coherent Map-405 clouds without palette overrides plus exact reconstruction. Then run `Regression - Sprite Ownership and Export Coordinates` and Full Regression through `=== ChaseHQ Full Regression: COMPLETE ===`.

The source correction is evidence-backed: RC2.5 session `20260930_175159`, run `008`, frame 2352 / Map 405 / palette 175, bundle SHA-256 `343c14d710023fbb308312900dc453d2eed365409243242c634493aece42c6b2`. The shared decoder now maps physical planes `0,1,2,3` to pen bits `3,2,1,0`; do not add a late palette/compositor bit-reversal workaround.

Non-SDL `sprite_gfx_tests` and `sprite_priority_tests` already pass in the packaged working tree. Windows/SDL proof is still required because the changed code participates in live rendering.

Keep the separate `scene-no-sprites.png` bug distinct: the RC2.5 causal snapshot exported it identical to `final.png` despite a non-empty sprite layer. It is a follow-up forensic-tooling issue, not part of the plane-significance correction.

Read root `AGENTS.md`, then `docs/HANDOVER.md`, `docs/PROJECT_STATE.md`, `docs/ROADMAP.md`, `RELEASE_NOTES_0.66.9.0-RC2.6.1.md`, and `VALIDATION_0.66.9.0-RC2.6.1.md` before changing the tree. Treat this package plus those docs as authoritative continuation.

---

# ChaseHQ-Native continuation handover

**Build:** v0.66.9.0-RC2.4.3 — Snapshot Discovery Correctness Hotfix  
**Package status:** **ACTIVE CANDIDATE** — native renderer unchanged from RC2.4.2; Windows proof required for the new assertive snapshot-discovery path.  
**Current proven baseline:** v0.66.8.0  

## v0.66.9.0-RC2.4.3 active candidate

RC2.4's TC0100SCN Y-scroll correction remains the current renderer candidate: BG0/BG1/TEXT source-Y applies the corrected control-word sign, historical BG presentation compensation is removed, and frame 2065 has a byte-exact validation target. RC2.4.1 fixed stale Workbench listener cleanup on restart.

RC2.4.2's attempted cache-only snapshot-discovery fix failed on Windows. Two uploaded run bundles prove that current-run snapshots were created successfully while the immediately following `frame.snapshot.list` returned only older RC2.3 snapshots. The RC2.4.2 regression was non-assertive and therefore reported PASS incorrectly.

RC2.4.3 adds an explicit same-process recent-snapshot registry, merges it into snapshot list/resolution, invalidates all relevant discovery caches after capture, and adds optional `frame.snapshot.list require=NAME`. The packaged `Regression - v0.66.9.0 RC2.4.3 Snapshot Discovery` uses that parameter and must fail if the just-created snapshot is not returned.

The RC2.4.2 layered capture also proved the visible sky texture exists in the raw TC0100SCN bottom/background source before compositor reconstruction. Do not chase that pattern as a compositor bug without contradictory reference evidence.

## Immediate continuation

On Windows: build/start RC2.4.3, run the packaged RC2.4.3 snapshot-discovery regression, then run `Validate-FrameSnapshotRefresh.ps1` and `Validate-TC0100SCN-Y.ps1`. Do not rerun the already-uploaded RC2.4.2 sky isolation simply to reproduce known evidence. Once the reliability gate passes, return to the player-car shadow/under-car sprite ordering investigation; keep sky tuning and speculative BG offsets out of that experiment.

## Authoritative continuation contract

Upload the latest project ZIP and provide this file to a new ChatGPT conversation. Treat the ZIP plus this prompt as authoritative. If an older conversation, memory, note, or assumption conflicts with packaged source/docs, prefer the packaged source and this prompt.

Continue ChaseHQ-Native as a native Chase H.Q. emulator/reimplementation and research platform. Be proactive: inspect evidence/source immediately; when the next step is a run, provide the exact one-line PowerShell command; batch independent experiments; avoid version churn; keep CLI/API/script/UI parity; and carry forward docs, checkpoints, regressions, roadmap, schemas, knowledge, and evidence conventions in every meaningful build.

## Historical platform milestones retained below

## What v0.66.7.6 consolidates
- Reworks **Docs / Knowledge** into a user-facing **User & Research Handbook** instead of exposing the knowledge-object catalogue as the primary interface.
- Five documentation modes: **Handbook**, **API Reference**, **Scripting Reference**, **Project Knowledge**, and **Glossary**.
- `research/knowledge/handbook.json` provides authored getting-started pages, Workbench guides and task-oriented research recipes.
- The complete API action reference is generated from the authoritative action schema at runtime, including descriptions, parameters and copyable `.chqscript` examples.
- The complete `.chqscript` language reference is generated from the authoritative script schema, including statements, query functions and parser limits.
- Search spans handbook pages, API actions, script-language constructs and project knowledge.
- Read-only docs/knowledge Research API and `.chqscript` access now includes `docs.handbook` and `docs.page` in addition to the existing knowledge actions.
- Full handbook HTML/PDF export is generated from the same handbook + API schema + script schema + project knowledge sources used by the Workbench; evidence mode remains `none|key|full`.
- Documentation evidence model with screenshots/artifacts and provenance metadata; initial proven layered-frame evidence is packaged under `docs/evidence/`.
- Authoritative `docs/CHATGPT_HANDOVER_PROMPT.md`, required to move with every revision.
- Graphics Lab individual-sprite forensic controls. Exact frame reconstruction still uses the aggregate sprite contribution layer; `sprites/sprites.json` plus cropped transparent per-sprite PNGs support hide/show/solo/opacity exploration without bloating each asset to a full 320×240 PNG.
- Workbench SDL controls backed by authoritative runtime/API actions: status, pause/resume/step, layered snapshot shortcut, show/hide, minimize/restore, fullscreen, integer scale 1..8, always-on-top.
- Runtime window state is reported from the native process and surfaced in the Workbench. Meaningful controls are API/scriptable; the Workbench is not the sole invocation path.

## Current proven graphics state
v0.66.6.1 is proven. A layered snapshot contains `final.png`, `final-no-hud.png`, `hud-only.png`, exact contribution layers, raw source layers, palette state and reconstruction metadata. Native capture self-verifies exact reconstruction and requires `verifiedExact=true` / `mismatchedPixels=0`. The Windows regression independently confirmed exact pixel-for-pixel reconstruction.

## Current research priority after release prove-off
Turbo gameplay state and the bottom-HUD source chain are now confirmed: processed turbo input is CPU-A `0x100303` bit 0, active is `0x10040F` bit 1, turbos remaining is `0x1003A2`, timer is `0x100414`, and the normal/active TC0100SCN RAM characters decode coherently as intentional animation. The immediate open graphics gate is the RC2.4 TC0100SCN Y-scroll correction, followed by car-shadow priority, building X placement/layer assignment and any residual sky/top-visible-area mismatch.

## Canonical checkpoints
- `stage1-driving-2352.chqstate` — deterministic Stage 1 driving/reference state.
- `stage1-gameplay-2064.chqstate` — actual Stage 1 gameplay state.
- `stage1-target-post-final-hit-9548.chqstate` — user-confirmed post-final-hit slowdown/defeat-transition state; do not describe as pre-final-hit health.
- `stage1-end-level-9988.chqstate` — confirmed end-level screen.
Treat packaged checkpoint metadata as authoritative if filenames/roles evolve.

## Important confirmed findings / active leads
- Brake/rear-lamp visual change is a **palette transformation**, not a separate lamp sprite. Palette banks 64/65, pen 13, raw transition `0x000E -> 0x18DE`, player sprite slot 45 / map 516, 140 pen-13 source pixels, OBJ A provenance.
- Processed brake/input investigation around CPU-A `0x100303` has reusable generic memory-writer tracing; use `width=any` for discovery when producer transaction width is unknown.
- Shared Track Recorder is proven and scriptable; Stage 1 road/course mechanism chain includes `0x10A04E -> band logic -> 0x10A04A -> 0x10A076(+36) -> render-pointer table 0x4A602`, but exact physical semantics are not fully named.
- Turbo gameplay state and HUD character source/animation are confirmed; do not treat missing turbo indicators as an open renderer-source defect unless new contradictory evidence appears.

## Research API / scripting discipline
- `.chqscript` remains intentionally small. Prefer existing `api ACTION key=value` rather than inventing dedicated language syntax.
- Discover actions with `api actions`, `api schema action=NAME`, `api schema.all`, `api script.schema`.
- The v0.66.7.x handbook retains `docs.handbook` and `docs.page`; all docs/knowledge and SDL/window actions remain documented in `research/schema/api-actions.json`, while the language grammar remains in `research/schema/script-language.json`.
- Any API/scripting change must update implementation + schema + human docs + regression in the same build.

## Documentation / knowledge discipline
- `research/knowledge/handbook.json` is the authored user-manual/navigation source; `research/knowledge/documentation.json` remains the evidence-backed project knowledge/glossary source.
- Entries carry identity, kind/category, status/confidence, introduced/updated versions, tags, relationships, docs/scripts/API references and optional evidence.
- Confirmed findings should cite evidence; provisional/open findings must not be presented as confirmed.
- Do not silently delete old conclusions: use supersession/history metadata when a later finding replaces them.
- User-visible tooling, API/script behaviour, or established research changes must update the knowledge source before the revision is documentation-complete.
- `docs/CHATGPT_HANDOVER_PROMPT.md` must match the current build and proof status. Release validation should flag stale handover metadata.

## Release/prove-off lessons now made permanent
- `Build-Debug.bat` is the only canonical build entry point; its strict-mode validator path is the path that must be tested.
- Developer handoff ZIPs use `build-local.json` for local ROM/SDL staging and must not accidentally bundle the full SDL source tree, `out`, ROM payloads or transient evidence.
- Parse every PowerShell file before handoff and validate the final ZIP itself.
- Run history is disk-authoritative; Graphics Lab must rediscover completed-run snapshots without unbounded recursive scanning.
- One script execution owns one run directory and one final bundle; transient predicates do not allocate runs and `Compress-Archive` must not run after every command.
- Runner UI lifecycle is RUNNING -> FINALIZING -> COMPLETE/DONE; stop the ticker before finalization.
- Use cheap gates before Full Regression. Full Regression is the final expensive gate, not the first diagnostic action.
- If a candidate needs a manual local patch, fold that patch into a clean candidate package and prove the clean package; never promote the hand-edited tree.

## Core project working rules
1. Gameplay/render fidelity is the project; tooling exists to unlock and prove fidelity.
2. Prefer authoritative emulated/game state over screenshot inference.
3. Reproduce from checkpoints/scenarios; collect structured evidence; package one portable bundle per run.
4. Keep generic research infrastructure game-agnostic; Chase H.Q.-specific addresses/actions belong in game/profile knowledge.
5. Preserve CLI/API/config parity for diagnostics. UI-only capabilities are not acceptable for meaningful research operations.
6. When the next action is a run, give the exact copy/paste PowerShell command immediately.
7. Push the current build as far as practical before another release; cut a new build for genuine fixes/coherent tooling milestones, not tiny changes.
8. Every release carries forward current docs, roadmap/project state, schemas, scripts/regressions and canonical checkpoints.
9. Never bundle copyrighted ROM/audio data.

## Repository landmarks
- `docs/PROJECT_STATE.md` — current authoritative state.
- `docs/HANDOVER.md` — human handover and project continuity rules.
- `docs/CHATGPT_HANDOVER_PROMPT.md` — this authoritative cross-conversation prompt.
- `docs/ROADMAP.md` — priorities / longer-term direction.
- `docs/API_REFERENCE.md`, `docs/API_CHANGELOG.md` — Research API.
- `docs/SCRIPT_LANGUAGE_REFERENCE.md`, `docs/SCRIPT_CHANGELOG.md` — scripting.
- `research/schema/` — machine-readable contracts.
- `research/knowledge/` — machine-readable research/documentation knowledge.
- `research/scripts/` — experiments, diagnostics, surveys and regressions.
- `docs/evidence/` — curated documentation evidence, not transient run output.
- `checkpoints/` — canonical save states and metadata.

## Longer-term direction
Continue developing reusable visual/audio/input/road research infrastructure, authoritative gameplay-state discovery, complete stage/game fidelity, richer asset explorers, higher-resolution sprite replacement packs, and potential hybrid 3D presentation while retaining an original/pixel-authentic canonical mode. Enhancement rendering must remain separate from authoritative gameplay simulation.

## Required response behaviour in the new conversation
Inspect the uploaded ZIP/source/evidence rather than relying on memory. State the current build/proof status and immediate next action. Do not resurrect superseded tasks from older chats unless the packaged roadmap/state says they remain open. When the next step is a run, provide the exact command. When changing code, update the respective API/script/knowledge/handover/release documentation and regression coverage in the same build.

## v0.66.7.2 hotfix reason
The first v0.66.7 Windows prove-off confirmed the new Docs/Knowledge APIs and SDL status/control actions up to checkpoint loading. The regression itself then failed because it used `checkpoint.load name=stage1-driving-2352.chqstate`; the live/documented contract requires `file=...`. v0.66.7.2 corrects the regression and also fixes visible per-sprite label mojibake by using an ASCII separator. No API semantics changed; the remaining gate is to rerun the corrected regression through snapshot and docs export.


## v0.66.7.2 documentation integration hotfix
The v0.66.7.1 Windows regression passed the substantive Docs/Knowledge, SDL control and per-sprite snapshot checks, but review found two integration defects: `docs.handover` was decoded with the host default encoding when invoked through `.chqscript`, producing mojibake for UTF-8 punctuation; and `docs.export` used the session-level export directory captured at Workbench startup, so script-generated documentation exports did not follow the active run artifact root and were omitted from that run bundle. v0.66.7.2 forces UTF-8 for handover reads and resolves the documentation export directory dynamically from the current evidence root, preserving normal Workbench exports while making script-run exports portable in their run bundle. No API action names or `.chqscript` syntax changed.


## v0.66.7.3 metadata hotfix
The v0.66.7.2 regression passed its functional and documentation-integration checks, but evidence review found stale `0.66.7.1` build literals in the Workbench host, including snapshot `manifest.json`, health/frontend metadata and UI run-history defaults. v0.66.7.3 synchronizes those runtime/Workbench build identifiers with the native/package version. No Research API semantics or `.chqscript` syntax changed.
## v0.66.7.5 identity-sweep hotfix
A Windows smoke test showed native v0.66.7.3 with Workbench v0.66.7.2. v0.66.7.5 removes the remaining live stale Workbench/runtime version literals and adds release-validator checks for Workbench header, health bridge and OpenAPI identity. Historical changelog references are not rewritten. No API or `.chqscript` semantics changed.


## v0.66.7.6 consolidation notes
- Folded the locally proven handbook UI/search/export fixes into one clean release candidate instead of carrying ad-hoc patch state.
- Article-body search is indexed in the compact docs index and server-side docs/knowledge search.
- Standalone handbook export renders Project Knowledge article sections and omits empty reference headings.
- Project Knowledge landing is grouped and clickable by status.
- Release validators cover duplicate handbook handlers, article/schema integrity, ASCII-safe Workbench source, owned-text mojibake sentinels and debug-backup exclusion.
- Local `.pre-*.bak` files and temporary patch helpers are intentionally excluded from the release package.

## v0.66.7.7 persistence/discovery hotfix
- v0.66.7.6 Full Regression completed successfully (`ok=true`) but interactive proof found that browser-local history could omit a valid completed server run.
- Recent Script Runs now merges server-discovered `run-metadata.json` records with local history.
- Added `GET /api/v1/script/runs` and `api script.runs`.
- Graphics Lab now scans/resolves snapshots inside completed run artifact roots as well as the active snapshot root.
- No emulator/gameplay semantics changed.


## v0.66.7.8 discovery-performance RC
- Fixes v0.66.7.7 cross-version discovery blocking the single-request Workbench backend.
- Run discovery now scans only the known session/run directory shape instead of recursive trees, reads artifact lists from bundle-manifest.json, and caches results briefly.
- Structured snapshot root/list discovery is cached briefly while preserving refresh-on-miss for named snapshots.
- This RC intentionally excludes the bundled SDL source tree; build-local.json remains the local dependency pointer.


## v0.66.7.8 RC2 final-gate cleanup
- Permanently fixes the missing closing brace in `Get-StructuredFrameSnapshotRoots()` that caused `Start-ChaseHQWeb.ps1` to fail parsing in the first v0.66.7.8 RC package.
- Synchronizes launcher health metadata from stale `0.66.7.4` to `0.66.7.8`.
- Keeps the focused discovery/performance implementation unchanged; the remaining gate is one final Full Regression Suite against this corrected package.

## v0.66.7.8 RC3 runner-finalization prove-off
RC2 exposed a script-run packaging defect during Full Regression: every command request rebuilt the parent `bundle.zip`, and predicate/preflight requests could allocate incidental runs. RC3 changes the browser/server contract so command requests append to one RUNNING run, transient query/predicate requests do not allocate runs, and PASS/FAIL/CANCELLED finalization creates the manifest and ZIP once. Prove this with the dedicated run-finalization regression, then run Full Regression; do not promote v0.66.7.8 until repeated Compress-Archive activity is absent and one authoritative run/bundle is produced.
## v0.66.7.8 RC4 runner lifecycle UI correction
RC3 proved the one-run/one-bundle backend lifecycle, but UI review showed the 250 ms status ticker could overwrite the visible COMPLETE state during finalization/history recording. RC4 clears the ticker before finalization, reports FINALIZING while the single bundle/history entry is committed, and sets DONE/COMPLETE only after success. Final runner context reports executed commands rather than a branch-inclusive denominator.


## v0.66.8.0 RC2 first Windows feedback
- RC1 native Forensic Timeline regression passed twice; duplicate execution was traced to Script Library UI selection/filtering not auto-loading the newly selected script text.
- RC1 turbo recording browser run timed out at the old 15-second bridge limit while native execution was still progressing. The recording itself continued and is complete: frames 2064..2319, 256 frame checkpoints, 711,374 writes; turbo active rises at 2065 and falls at 2275.
- RC2 fixes script auto-load synchronization, uses progress-aware long-run waits and wider long-operation proxy/browser budgets, chunks the turbo tail into 4x60 frames, uses a unique browser bootstrap URL, clears stale frontend-error diagnostics on web startup, extends bootstrap wait to 20 seconds, and normalizes Build-Debug numbering.
- Do not ask the user to replay turbo. After RC2 starts, use `Timeline / Recover Existing Turbo Forensic Timeline`; cross-version timeline discovery can find the RC1 recording in the sibling evidence tree.

## v0.66.8.0 RC3 offline-analysis performance correction

RC2 successfully proved the 160-frame long timeline run, but `Recover Existing Turbo Forensic Timeline` then timed out after 360 seconds inside `timeline.trim.event` against the valid RC1 turbo recording (711,374 writes). Do not ask the user to replay turbo. RC3 fixes the analyser itself: streaming event detection, streaming memory analysis, and immutable/non-destructive event windows. The master timeline must never be deleted or rewritten merely to select an investigation interval.

## v0.66.8.0 RC6 legacy timeline compatibility note
RC5 exposed two compatibility defects in offline recovery: the recovery script used `trigger=`/`stop=` while the documented action expected `triggerFrame=`/`stopFrame=`, and the RC2-destructively-rewritten `writes.csv` contains quoted CSV fields (`"2064"`) whereas the streaming parser assumed native unquoted fields. RC6 accepts both parameter spellings, uses canonical names in the packaged recovery script, strips optional CSV quotes during streaming parsing, and gives explicit trigger/stop bounds precedence over any stale event-window index. Do not replay turbo gameplay; reuse the existing recording.

## v0.66.8.0 RC7 portable-analysis note
RC6 proved offline recovery of the existing turbo capture. RC7 moves derived timeline evidence out of the source recording and into the active script run, tags known turbo reference fields, automatically disassembles high-value writer PCs, captures before/active/mid/expiry/after historical states and layered frames, correlates HUD_TURBO in normal/HUD-only/HUD-hidden modes, and adds paced SDL recorded-state playback. Timeline playback is visual historical-state playback; CPU re-execution requires `timeline.fork-live`. Authentic audio is still unavailable while the runtime exposes only `sound_stub`.

## RC7.3 Workbench browser-memory/history hotfix
- Script Console thumbnails default OFF.
- Artifact presentation defaults to a compact file-type count summary; opt-in previews are bounded/lazy-loaded.
- Live browser output retains only the newest 120 DOM blocks; complete authoritative output remains in run logs/bundles.
- Recent Script Runs derives RC-aware labels from run paths where available (for example 0.66.8.0-RC7.2) and adds PASS/FAIL/CANCELLED filtering.
- These are Workbench UI/resilience changes only; emulator semantics remain the RC7.2 baseline.


Release evidence summary: `RELEASE_PROOF_0.66.8.0.md`.

## v0.66.9.0-RC2 authoritative continuation
Immediate next task: prove `Regression - v0.66.9.0 RC2 Sprite Order and Speed Semantics`, then run `Player Car Sprite Order A/B` against the already captured car-shadow timeline. Current speed terminology: HUD/display speed is packed BCD at `0x100400`; internal/physics speed is raw `0x10041C`. Do not expect the two numeric representations to match.

### Current candidate note — v0.66.9.0-RC2.2
RC2.2 supersedes RC2.1. The player-car/under-car A/B evidence and current MAME `chasehq_draw_sprites_16x16` both support reverse sprite-RAM traversal, so the native same-priority default is now `lower-slot`. Keep `higher-slot` only as a reversible forensic comparison. Run the focused RC2.2 sprite-order regression before Full Regression.

> RC2.2 note: Chase H.Q. native same-priority sprite ordering defaults to `lower-slot`, matching MAME reverse sprite-RAM traversal; `higher-slot` remains an explicit diagnostic override.


## RC2.4.2 CURRENT STATE
- Package: v0.66.9.0-RC2.4.2.
- TC0100SCN Y-scroll correction from RC2.4 remains the current renderer behavior.
- Workbench structured snapshot capture now invalidates discovery caches immediately; a `frame.snapshot.list` issued in the same script run must include the newly created snapshot.
- The ad-hoc name `post-y-fix-sky` was only an evidence label, not a built-in feature. The packaged workflow is now `research/scripts/graphics/tc0100scn-post-y-fix-layer-isolation.chqscript`.
- New regression: `research/scripts/regression/regression-v06690-rc242-tc0100scn-snapshot-refresh.chqscript`.
