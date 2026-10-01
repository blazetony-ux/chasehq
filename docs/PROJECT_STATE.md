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

## Current candidate — v0.66.9.0-RC2.6.1

**Focus:** source-level Chase H.Q. sprite 4bpp plane-significance correction.

### Confirmed evidence
- RC2.5 session `20260930_175159`, run `008`, frame `2352`, Map `405`, palette bank `175` causally reproduced correct cloud shading by selecting palette entries according to 4-bit pen reversal.
- 16 visible Map-405 sprites used that target palette in the structured snapshot.
- Geometry, priority and ownership were not changed by the intervention.
- Snapshot reconstruction remained exact: `verifiedExact=true`, `mismatchedPixels=0`.
- Evidence bundle SHA-256: `343c14d710023fbb308312900dc453d2eed365409243242c634493aece42c6b2`.

### RC2.6 implementation
- One shared `src/sprite_gfx.h` decoder is used by live rendering and export/inspection paths.
- Physical source plane significance is now `0->bit3, 1->bit2, 2->bit1, 3->bit0`.
- `tests/sprite_gfx_tests.cpp` exhaustively checks all 16 source-plane combinations and transparency/geometry invariants.
- Existing sprite-ownership model is intentionally unchanged; bit reversal maps zero to zero and every nonzero nibble to nonzero.
- A packaged no-override prove-off script targets the canonical frame-2352 Map-405 scene.


### RC2.6.1 packaging/regression correction
- Emulator/rendering semantics are unchanged from RC2.6.
- Add explicitly version-labelled `regression-v06690-rc261-sprite-bitplane-significance.chqscript`.
- Include that regression before the Full Regression terminal COMPLETE marker.
- Retain the separate graphics prove-off for manual/structured Map-405 visual validation.

### Proof state
- Non-SDL configure/build: PASS.
- `sprite_gfx_tests`: PASS.
- `sprite_priority_tests`: PASS.
- Windows/SDL renderer prove-off: **PENDING**.
- RC2.6.1 focused regression + Full Regression: **PENDING**.

### Explicitly separate open tooling defect
The RC2.5 causal snapshot's `scene-no-sprites.png` is byte-identical to `final.png` while the aggregate sprite contribution is non-empty. Track/fix this as snapshot-forensics work after the decoder gate; do not mix it into the source-decoder proof.

RC2.5 consolidates the proven player-car sprite-ownership correction and the scripting/research reliability gaps exposed while starting the TC0100SCN sky/rowscroll investigation.

**Sprite correction is already Windows-proven on the RC2.4.4 working tree:** canonical frame 2352 reports body slot 82/map 475 ahead of shadow slot 81/map 575; `overlapPixels=689`, `frontOwnedOverlap=553`, `backVisibleOutsideFront=247`, `exportCoordinatesExact=true`. The focused ownership regression passed and the permanent Full Regression subsequently reached `=== ChaseHQ Full Regression: COMPLETE ===` in session `20260930_151202`, run `002` (118.824 s). The pristine RC2.4.4 control build independently reproduces the historical `cpu_bus_rom_tests` SEGFAULT, so that failure is pre-existing and unrelated to the sprite change.

RC2.5 additionally adds safe Research Script lifecycle support (`try ... finally ... end`, idempotent `timeline.reset`), schema/Script-Console allow-list parity checking, corrected diagnostic script attribution, current-session history as the default view, richer script-header guidance, and a packaged safe rowscroll raster-mapping probe. These new Workbench/script features require a fresh RC2.5 Windows regression gate before this package is promoted from candidate to proven.

# Historical v0.66.9.0-RC2.4.4 development state

**Historical RC2.5 baseline note:** RC2.5 carried the Windows-proven sprite-ownership correction and added scripting/reliability work. RC2.6 now supersedes it as the active candidate because the later Map-405 causal probe isolated a source-decoder significance defect.

RC2.4.3 same-turn structured snapshot discovery is now Windows-proven. The uploaded focused regression created `regression-rc243-snapshot-discovery` at frame 2065, immediately required it via `frame.snapshot.list require=...`, received the current-run entry, and completed with `status=PASS`; both normal and HUD-hidden self-comparisons had zero changed pixels.

The following Full Regression attempt failed during script preflight, before test execution, with `Expanded script exceeds 500 commands`. The packaged suite expands to exactly 503 commands. RC2.4.4 therefore raises the Research Script v2 expanded-command ceiling to 1000 consistently across browser/server/schema/reference while retaining the other safety limits.

The same diagnostic bundle exposed stale startup metadata (`startup-session.json` reported build `0.66.8.0` while the live runtime correctly reported `0.66.9.0`). RC2.4.4 derives startup/research-session build metadata from `src/version.h` with a CMake fallback.

Root `AGENTS.md` is now packaged as the durable Codex/agent operating contract. Changing investigation state remains in HANDOVER/PROJECT_STATE/ROADMAP rather than AGENTS.md.

Immediate gate: build/start RC2.5 on Windows, run the focused Script Safety and Sprite Ownership regressions, then Full Regression. After that, resume the TC0100SCN sky/rowscroll raster-mapping investigation with the packaged safe probe.

# v0.66.9.0-RC2.4.3 current development state

**Proven baseline: v0.66.8.0. Candidate: v0.66.9.0-RC2.4.3.** RC2.4's evidence-backed TC0100SCN Y-scroll correction remains unchanged. Windows live gameplay showed the background building planes on the horizon with neutral BG offsets, and the deterministic frame-2065 validator remains the renderer gate.

RC2.4.2 attempted to repair immediate structured-snapshot discovery by invalidating discovery caches. Two Windows bundles disproved that fix: `frame.snapshot` created valid current-run snapshots (`regression-rc242-tc0100scn` and `tc0100scn-yfix-2065`), while the immediately following `frame.snapshot.list` returned only older RC2.3 snapshots. The RC2.4.2 regression was therefore a false positive because it printed the list but did not assert that the new name was present.

RC2.4.3 fixes the tooling gap with an explicit same-process recent-snapshot registry merged into list and resolution paths, invalidates all relevant discovery caches after capture, and extends `frame.snapshot.list` with optional `require=NAME`. The new RC2.4.3 regression uses that assertion so the same failure cannot report PASS.

The RC2.4.2 layered capture also localized the visible sky texture to `sources/bg-bottom.png` before compositor reconstruction. Exact reconstruction still verifies with zero mismatches. Treat that sky dithering/banding as source-layer presentation rather than an active compositor defect unless direct reference evidence later contradicts it. The next substantive graphics target is the player-car shadow/under-car ordering anomaly, followed by separately proven X/layer/compositor issues.

# v0.66.9.0-RC1 current development state

**Proven baseline: v0.66.8.0. Candidate: v0.66.9.0-RC1.** The project has returned to direct Chase H.Q. investigation. The turbo HUD path is now substantially proven: CPU-A `0x1003A2` is turbos remaining, `0x10040F` bit 1 is turbo active, `0x100414` is the active-duration counter, and the gameplay routine at `0x8460..0x84E6` consumes a turbo, sets active state and expires at `0x00D2` frames in the normal path. The HUD writer at `0x1C7C..0x1C8A` deliberately writes active character IDs `0x0C..0x0F`; the normal icon writer at `0x1CE6..0x1CF4` uses `0x7C..0x7F`; the expiry path at `0x1CC2..0x1CDA` clears the just-used 2x2 text-cell block.

v0.66.9.0-RC1 adds reversible Game Lab controls for infinite turbo stock, infinite active duration, turbo count/timer manipulation, and current-speed set/freeze at confirmed CPU-A `0x10041C`. It also adds `tile.inspect` for TC0100SCN RAM text characters, exporting raw 2bpp pens, palette-aware PNG and JSON metadata.

# Project state — v0.66.8.0 PROVEN RELEASE

**Current proven baseline: v0.66.8.0.** Promoted from RC7.4 after the final Windows/SDL Full Regression passed on 2026-09-29. The final promotion is documentation/package-state only; runtime semantics are the proven RC7.4 runtime.

Forensic Timeline v1 is now proven for the intended workflow: capture once, reopen and analyse offline, generate portable current-run forensic artifacts, replay recorded states in SDL, and fork a selected historical frame back into LIVE execution. The real historical turbo recording has been recovered and analysed without replaying gameplay.

Final release evidence: bounded-history regression PASS (1.071 s); Full Regression PASS (78.422 s, session 20260929_195302/run 002); prior RC7 portable-analysis and real turbo automated-analysis focused gates PASS.

The immediate project focus can return to Chase H.Q. itself. Tooling/release-process work should now be driven by concrete game-fidelity investigations rather than speculative platform expansion.

# v0.66.8.0 development record

## Finalized feature state

**Forensic Timeline v1 is proven in v0.66.8.0.** This is the first substantial post-v0.66.7.9 tooling feature and is intended to reduce repeated gameplay reproduction. The implementation records each frame as a deterministic CHQSTATE and records every bus write with frame/CPU/PC/address/width/old/new provenance.

A timeline can be loaded and sought through `.chqscript`; because seek loads the recorded checkpoint into the paused native runtime, existing read-side APIs inspect the historical state directly, including memory, CPU-A/B registers, gameplay registry, full sprite/object RAM, palettes, road/tilemap state and structured frame capture. `timeline.fork-live` leaves the selected historical checkpoint loaded and returns to LIVE paused execution.

The first automated event workflow is turbo: record a broad deterministic window, trim it to one frame before the confirmed turbo-active bit rises through one frame after it falls, then rank every changed bus address and writer PC offline.

**Audio limitation:** the current emulator still has only `sound_stub`; Forensic Timeline v1 captures that stub memory but cannot provide authentic sound CPU/chip/PCM data until the native audio runtime exists. This limitation is explicit in the manifest and audio capability descriptor.

## Immediate prove-off

1. Build with `Build-Debug.bat`.
2. Launch with `Start-ChaseHQ.ps1 -Restart`.
3. Run `Regression - v0.66.8.0 Forensic Timeline v1`.
4. Run `Capture Turbo Complete-Cycle Forensic Timeline`.
5. Verify the generated `.chqtimeline` can be imported and inspected after restarting the Workbench.
6. Verify `timeline.scan.memory` produces ranked CSV/JSON without replaying gameplay.
7. Only after focused proof, run Full Regression once.

---

# Project state — v0.66.7.9

## Current status

This build is a **research-recovery / release-discipline consolidation**. It carries forward the v0.66.7.8 runtime/Workbench fixes and turns the failures found during that prove-off into explicit process rules, fast gates and reusable research scripts. No new emulator/gameplay semantic claim is introduced by the platform changes in this revision.

**Release proof:** PROVEN on Windows/SDL on 2026-09-28. `Build-Debug.bat`, browser bootstrap, Recovery / Fast Release Gate, `Regression - v0.66.7.9 Research Recovery`, and the final `Full Regression Suite` all passed. The final full-regression metadata reported build `0.66.7.9`, `ok=true`, `status=PASS` and ended with `=== ChaseHQ Full Regression: COMPLETE ===`. v0.66.7.9 was the previous proven baseline; v0.66.8.0 supersedes it.

The v0.66.7.6-v0.66.7.8 prove-off exposed and corrected: missing local config continuity in one package, accidental full SDL bundling, a strict-mode validator bug, an unparsed Workbench brace defect, stale build identity, unbounded cross-version run/snapshot discovery, repeated per-command `bundle.zip` compression, stray transient predicate runs, and a UI ticker that could overwrite COMPLETE during finalization. `docs/RELEASE_PROCESS.md` records the prevention rules.

## Corrected turbo research state

The previous knowledge entry incorrectly framed turbo-active/turbos-remaining discovery as open. Current authoritative sources already confirm processed turbo input `0x100303` bit 0, active flag `0x10040F` bit 1, remaining count `0x1003A2`, timer `0x100414`, and the tested 210-frame expiry threshold. The open problem is the bottom-HUD visual/source/compositor path. The machine-readable knowledge entry and recovery plan are corrected accordingly.

## New reusable scripts

- `research/scripts/recovery/research-session-preflight.chqscript`
- `research/scripts/recovery/fast-release-gate.chqscript`
- `research/scripts/gameplay/turbo-state-causal-validation.chqscript`
- `research/scripts/gameplay/turbo-state-writer-trace.chqscript`
- `research/scripts/graphics/turbo-hud-before-during-after.chqscript`
- `research/scripts/graphics/canonical-checkpoint-visual-survey.chqscript`
- `research/scripts/gameplay/target-endlevel-state-survey.chqscript`

The immediate goal now that release proof is complete is to use these scripts, not to expand platform tooling again.

---

# Project state — v0.66.7.7

## v0.66.6.1 layered graphics forensics

- Structured frame snapshots are now v2 reconstructable bundles with `final.png`, `final-no-hud.png`, `hud-only.png`, exact alpha contribution layers, raw source-layer renders, reconstruction metadata and semantic sprite evidence.
- Chase H.Q. HUD isolation currently maps to the TC0100SCN text layer and is suppressed at source rather than masked by screen coordinates.
- `image.compare` / `image.diff` accept `hud=normal|hidden|only` for deterministic HUD-aware comparisons.
- Graphics Lab can inspect saved snapshots, switch HUD modes, reconstruct contribution layers and toggle/solo/adjust opacity per layer.
- Same-build API/script schemas, Markdown references and regression coverage are included.

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

Chase H.Q. remains the authoritative target. This hotfix is intentionally limited to research-workflow reliability; it is not a new Workbench expansion cycle.

## Working rule: investigate first, build second

Before cutting a new build, exhaust practical remedial and investigatory work in the current tree. Reproduce deterministically, use the documented API/script schemas, inspect logs/evidence/source, patch PowerShell/scripts locally where possible, and validate the patch in-place. Rebuild native code only when evidence shows the native implementation must change. Package a new ZIP only after the fix/feature is proven as far as the available environment allows.

Tooling defects that do not block the current Chase H.Q. investigation go to the backlog. They must not displace gameplay/render work. New tooling should be the smallest addition that unlocks the current investigation and should ship with schema/docs/regression coverage.

## Immediate gameplay/render focus

The brake-input question is resolved. Physical braking changes palette banks 64/65 pen 13 from `0x000E` to `0x18DE`. Player slot 45 / map 516 / palette 64 contains exactly 140 pen-13 pixels in the same 60x10 rear-window region changed by physical braking. `sprite.map.inspect slot=45` reports map 516 from OBJ A, spritemap word base `0x8100`, byte base `0x10200`, with `pen13=140`. Raw OBJ-A provenance is resolved: the corrected standalone decoder matches the native renderer/API and confirms all 140 pen-13 pixels in source tile data. Physical and scripted brake inputs match under the player-gameplay checkpoint. The next substantive investigation is to trace palette entries 1037/1053 to their exact writer PC(s), then disassemble the brake/turbo palette decision.

After that, return to turbo/HUD, target health/damage, end-level state and track topology.

## v0.66.3.1 reliability fixes

- session directories are short `YYYYMMDD_HHMMSS` identifiers; launcher mode/ports belong in session metadata, not folder names;
- the obsolete `web-evidence` layer is removed; session-owned web/research evidence lives under `session/evidence`;
- Script Console runs live under `session/runs/NNN` using short sequential run IDs;
- each run owns exact source, console output, artifacts, run metadata, bundle manifest and `bundle.zip`;
- run metadata stores build/session explicitly; UI history does not infer build versions from folder-name regexes;
- pasted/ad-hoc scripts can identify themselves with `# name:`, `# purpose:` and `# category:`; provenance distinguishes LIBRARY, MODIFIED and PASTED;
- history actions are shared controls for the actively selected row, not repeated on every row;
- `-Restart` falls back to terminating the processes actually holding ChaseHQ ports if recorded PIDs do not release them;
- validator checks script includes, required metadata, version consistency and the canonical session/run layout.

## Research script handover convention

Scripts supplied for interactive research should start with meaningful metadata, for example:

```text
# name: Brake Lamp Map 516 OBJ Provenance
# purpose: Trace the 140 pen-13 pixels in player map 516 back to spritemap chunks and raw OBJ graphics.
# category: graphics-research
```

Do not invent API action names or parameters. Use `api actions`, `api schema action=NAME`, `api schema.all` and `api script.schema`; implementation + schema + docs + regression must move together.

## v0.66.5.1 Track Recorder scripting hotfix

- Shared server-side Track View recorder backs both browser controls and Research Script.
- Added start/stop/clear/status/sample actions plus deterministic SVG export.
- Empty SVG exports are rejected.
- Memory-trace documentation/regression now uses `width=any` when the producer transaction width is unknown.
- `Build-Debug.bat` remains the canonical entry point, still runs `Prepare-Project.bat` first to unblock PowerShell files, and still tolerates only the exact historical `cpu_bus_rom_tests (SEGFAULT)` signature.

## v0.66.7.2 — Workbench Knowledge / Documentation Foundation Hotfix
Status: **IMPLEMENTED — AWAITING WINDOWS/SDL PROOF-OFF**. Previous proven baseline: **v0.66.6.1**.

This revision adds the machine-readable Workbench Docs/Knowledge system, evidence-backed documentation/export, authoritative ChatGPT handover prompt, individual-sprite Graphics Lab forensic controls, and API/scriptable SDL window controls. After proof-off, resume the turbo/turbos-remaining/missing-HUD investigation using HUD-only/HUD-hidden/layered/per-sprite evidence and promote confirmed findings into the knowledge source.

## v0.66.7.2 hotfix
The first Windows v0.66.7 regression proved Docs/Knowledge discovery/search/handover and non-destructive SDL window controls, but the regression script stopped at checkpoint load because it used the obsolete `name=` parameter. The documented/current `checkpoint.load` contract requires `file=`. v0.66.7.2 corrects the regression to `api checkpoint.load file=stage1-driving-2352.chqstate run=false`. It also replaces the Unicode middle-dot separator in per-sprite Graphics Lab labels with an ASCII `|` separator to avoid the visible mojibake middle-dot artifact observed in the Windows Workbench. No Research API semantics changed.


## v0.66.7.2 documentation integration hotfix
The v0.66.7.1 Windows regression passed the substantive Docs/Knowledge, SDL control and per-sprite snapshot checks, but review found two integration defects: `docs.handover` was decoded with the host default encoding when invoked through `.chqscript`, producing mojibake for UTF-8 punctuation; and `docs.export` used the session-level export directory captured at Workbench startup, so script-generated documentation exports did not follow the active run artifact root and were omitted from that run bundle. v0.66.7.2 forces UTF-8 for handover reads and resolves the documentation export directory dynamically from the current evidence root, preserving normal Workbench exports while making script-run exports portable in their run bundle. No API action names or `.chqscript` syntax changed.


## v0.66.7.3 build identity correction
The v0.66.7.2 Windows regression passed functionality, UTF-8 handover delivery, portable docs export, individual-sprite capture, and exact layered reconstruction. Evidence review found stale `0.66.7.1` identifiers in Workbench-host metadata. v0.66.7.3 synchronizes current build identity across native/runtime/Workbench snapshot and frontend metadata; no API or script semantics change.
## v0.66.7.5 full build-identity sweep
Windows smoke testing showed the SDL/native title at 0.66.7.3 while the Workbench still displayed 0.66.7.2. The cause was a second set of live version literals in `Start-ChaseHQWeb.ps1` (header, health bridge, OpenAPI metadata/API landing page, checkpoint-save description and startup text). v0.66.7.5 synchronizes all live Workbench/runtime identities and adds validator coverage so current native, Workbench, health and OpenAPI versions must match. Historical changelog references remain untouched. No API or script semantics changed.


## v0.66.7.5 documentation UX
The v0.66.7.4 knowledge/documentation foundation is proven. v0.66.7.5 reworks the user-facing Docs / Knowledge tab into a readable User & Research Handbook with authored guides, complete schema-generated API and `.chqscript` references, grouped cross-domain search, copyable examples, richer project-knowledge pages, and full handbook HTML/PDF export. `research/knowledge/handbook.json` is now carried and versioned alongside `research/knowledge/documentation.json`. v0.66.7.5 awaits Windows/Workbench regression proof.


## v0.66.7.6 Docs / Knowledge consolidation release candidate

- Consolidates the locally proven v0.66.7.5 handbook fixes into one release candidate.
- Project Knowledge entries now carry human-readable article bodies with discovery, significance, confirmed values where applicable, proof method, reproduction steps and open questions.
- Unified Workbench search and `docs.search` / `knowledge.search` include article-body text and documentation/script/API references.
- Full handbook HTML export includes the same article content and suppresses empty API/script/checkpoint/docs sections.
- Project Knowledge landing page groups clickable entries by confirmed, provisional, open/planned, implemented-awaiting-proof and historical/superseded status.
- `Start-ChaseHQWeb.ps1` remains ASCII-safe for Windows PowerShell 5.1; release validation checks this and scans owned project text for common mojibake markers.
- Release validation now requires one `showHandbookPage` implementation, article/schema hygiene, search/export wiring and absence of debug backup files.
- No emulator/gameplay semantics changed. Windows/SDL prove-off is still required before marking v0.66.7.6 proven.
- After prove-off, resume the bottom-HUD turbo rendering/source investigation using the already-confirmed turbo gameplay state plus HUD-only, layered and per-sprite evidence.


## v0.66.7.7 release candidate

The v0.66.7.6 Windows full regression passed, but prove-off exposed two persistence/discovery defects: a completed run could exist on disk with a valid bundle while being absent from browser-local Recent Script Runs, and structured snapshots stored inside completed run artifact roots disappeared from Graphics Lab after the run ended. v0.66.7.7 fixes both by making on-disk run metadata authoritative for history merge and by scanning/resolving completed-run snapshot roots. No emulator/gameplay semantics changed.


### v0.66.7.8 RC
Cross-version run/snapshot discovery performance hotfix: avoid recursive evidence scans that can monopolize the Workbench backend; cache discovery results briefly. Full SDL source is no longer bundled in the RC package.


### v0.66.7.8 RC2
Focused discovery/performance regression is proven on Windows. RC2 permanently carries the Workbench parser-brace correction and synchronizes launcher health build identity to 0.66.7.8. Remaining release gate: final Full Regression Suite against RC2.

### v0.66.7.8 RC3 runner finalization correction
RC2 Full Regression exposed repeated per-command bundle compression and incidental predicate/preflight runs. RC3 defers `bundle-manifest.json` + `bundle.zip` generation until one explicit terminal finalization (PASS/FAIL/CANCELLED), while transient preflight/predicate calls bypass run allocation. This must be proven on Windows before v0.66.7.8 is promoted from RC.
### v0.66.7.8 RC4 runner lifecycle UI correction
The RC3 run-finalization regression passed with one authoritative run and one final bundle, but the browser ticker could continue writing `Script: RUNNING...` while finalization/history recording completed, even after the runner badge had reached DONE. RC4 stops that ticker before finalization, exposes a dedicated FINALIZING phase, publishes DONE/COMPLETE only after finalization succeeds, and reports executed-command count rather than branch-inclusive totals. Remaining gate: focused run-finalization UI prove-off, then one Full Regression Suite.


## v0.66.8.0 RC2 first-capture corrections

The first Windows Forensic Timeline run proved the native recorder was functioning but exposed a fixed 15-second Script API wait limit. The browser reported failure at frame 2220 while native execution continued to the requested frame 2319. The resulting recording is complete (256 frames / 711,374 writes) and captures the turbo-active rise at frame 2065 and fall at 2275, so gameplay does not need to be reproduced. RC2 makes long frame waits progress-aware, chunks the packaged turbo tail into 60-frame operations, fixes Script Library auto-load/filter synchronization, prevents stale frontend errors from poisoning a later launcher bootstrap, and cleans up Build-Debug progress numbering.

## v0.66.8.0 RC3 offline-analysis correction

RC2's long-run timeline recording regression passed on Windows (161 frames / 438,415 writes in 18.372 s), proving the progress-aware runner fix. The subsequent recovery of the existing RC1 turbo timeline exposed a separate offline-analysis defect: `timeline.trim.event` spent more than 360 seconds processing a 711,374-write / ~34 MB CSV and timed out. The bottleneck was PowerShell `Import-Csv` plus `$rows +=` object accumulation and a second full import/rewrite pass.

RC3 replaces event detection and memory analysis with streaming readers. Event extraction is now non-destructive: the complete 2064–2319 turbo recording remains authoritative, while the 2064–2276 turbo-active investigation range is represented by an analysis/event-window index. This matches the project's capture-once/investigate-repeatedly goal.

## v0.66.8.0 RC6 legacy timeline compatibility note
RC5 exposed two compatibility defects in offline recovery: the recovery script used `trigger=`/`stop=` while the documented action expected `triggerFrame=`/`stopFrame=`, and the RC2-destructively-rewritten `writes.csv` contains quoted CSV fields (`"2064"`) whereas the streaming parser assumed native unquoted fields. RC6 accepts both parameter spellings, uses canonical names in the packaged recovery script, strips optional CSV quotes during streaming parsing, and gives explicit trigger/stop bounds precedence over any stale event-window index. Do not replay turbo gameplay; reuse the existing recording.


Release evidence summary: `RELEASE_PROOF_0.66.8.0.md`.

## v0.66.9.0-RC2 current candidate
- Proven v0.66.8.0 timeline/portable-analysis foundation remains the baseline.
- Turbo HUD animation mechanism is confirmed game logic, not a missing-indicator renderer defect.
- Current graphics target: player car slot 82/map 475 versus under-car effect slots 83/84 at priority 0.
- RC2 exposes the runtime same-priority tie-break for deterministic higher-slot/lower-slot A/B rendering against the existing car-shadow timeline.
- Game Lab now distinguishes packed-BCD HUD speed (`0x100400`) from raw internal/physics speed (`0x10041C`).

## v0.66.9.0 RC2.1 packaging correction
RC2.1 is a metadata-only handoff correction to RC2. The RC2 sprite-order/speed-semantics regression now carries the mandatory regression-version and regression-scope headers so the canonical Windows build gate can complete. No runtime behaviour changed.

## v0.66.9.0-RC2.2 — Chase H.Q. sprite traversal correction
- Correction: RC2.1 A/B captures did not prove RC2.2 ownership semantics; the later causal test identifies missing sprite occupancy (see SPRITE_OWNERSHIP.md).
- Authoritative MAME review independently confirms `chasehq_draw_sprites_16x16` traverses sprite RAM from the highest entry down to zero. Lower-numbered slots are therefore drawn later and win equal-priority overlap.
- Native/default sprite tie-break is now `lower-slot`; `higher-slot` remains a reversible research override.
- Reference: MAME `src/mame/taito/taito_z_v.cpp` (`chasehq_draw_sprites_16x16`), reviewed 2026-09-29.


## v0.66.9.0-RC2.3 build-gate correction
RC2.3 fixes the RC2.2 Full Regression integration error: the new MAME sprite-order regression is included before the suite's terminal `COMPLETE` marker. No runtime behaviour changed. Historical note: RC2.2/RC2.3 had descending traversal but still allowed later sprite candidates to overwrite earlier ones; RC2.5 adds the missing first-nontransparent occupancy rule.


### v0.66.9.0-RC2.4.2 snapshot-discovery hotfix
- RC2.4 corrected TC0100SCN Y-scroll remains unchanged.
- Newly captured structured frame snapshots now invalidate Workbench snapshot/root discovery caches immediately.
- Added packaged `tc0100scn-post-y-fix-layer-isolation.chqscript` so the current sky-band investigation has a named deterministic workflow rather than an unexplained ad-hoc snapshot label.
- Added current-build regression coverage and an assertive snapshot-refresh validator.
