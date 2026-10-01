# ChaseHQ-Native v0.66.9.0-RC2.9

## RC2.9 consolidation candidate

RC2.9 starts from the Windows/SDL-proven RC2.8 baseline and promotes the later RC2.8 causal research into the product, knowledge base and canonical checkpoint corpus. The release adds a native Stage-1 special-target one-hit research toggle that preserves the authentic `0xA112 -> 0xA118` terminal damage path, a global SDL Pause/Break pause/resume shortcut, a responsive Dashboard layout with SDL controls adjacent to the live frame on wide screens, and a configurable long-run timeout for `control.run-frames`.

The target-health question is closed at **BEHAVIOUR UNDERSTOOD**: CPU-A `0x1002AE` is the special target remaining-hit counter; genuine damaging hits decrement it at PC `0xA112`; `0xFFFF` is the terminal/exhausted sentinel. A value of `0x0000` means one final damaging hit remains. The preferred causal anchor is `stage1-target-immediate-pre-damage-6333.chqstate`, with `stage1-target-first-damage-6334.chqstate` as its matching post-hit state.

RC2.9 requires its own Windows/SDL `Build-Debug.bat`, focused RC2.9 regression and Full Regression before promotion. Linux/static validation in the packaged candidate does not substitute for that Windows proof.

## Historical release introductions

# ChaseHQ-Native v0.66.9.0-RC2.6.1

## Current candidate: v0.66.9.0-RC2.6.1

RC2.6.1 is a packaging/regression-label correction to the RC2.6 focused graphics-correctness candidate. The emulator/source-decoder semantics are unchanged. RC2.6 itself was built on top of the RC2.5 source baseline. The RC2.5 frame-2352 / Map-405 causal probe proved that the four physical sprite bitplanes were assigned the wrong pen significance. RC2.6 corrects that in one shared live/export 4bpp decoder (`plane 0 -> pen bit 3`, `1 -> 2`, `2 -> 1`, `3 -> 0`), adds exhaustive native coverage for all 16 pen combinations, and packages a no-palette-override visual prove-off. See `RELEASE_NOTES_0.66.9.0-RC2.6.1.md`, `VALIDATION_0.66.9.0-RC2.6.1.md`, and `docs/evidence/sprite-bitplane-significance/README.md`.

RC2.5 remains the authoritative source baseline for the correction and carries the proven sprite-ownership/export-coordinate work plus safe Research Script lifecycle support (`try/finally`, `timeline.reset`), Script Console action/schema parity enforcement, current-session history defaulting, and corrected diagnostic script attribution.

RC2.4.4 raises the Research Script v2 expanded-command ceiling from 500 to 1000 so the now-503-command permanent Full Regression suite can run, fixes stale startup/research-session build metadata by deriving it from the project version, and introduces root `AGENTS.md` for Codex/agent workflow continuity. Native emulator/video output is unchanged from RC2.4.3.

RC2.4.3 is a Workbench snapshot-discovery correctness hotfix on top of the proven RC2.4 TC0100SCN Y-scroll correction. Two Windows RC2.4.2 bundles showed that `frame.snapshot` could create a valid run-owned snapshot while the immediately following `frame.snapshot.list` still returned only older snapshots. RC2.4.3 adds an explicit same-process recent-snapshot registry, invalidates all relevant discovery caches, and adds assertive `frame.snapshot.list require=NAME` regression coverage.

Native emulator/video output is unchanged from RC2.4.2. The RC2.4.2 layered capture also localized the visible sky pattern to the raw TC0100SCN background source, so sky dithering/banding is no longer treated as a compositor defect without contradictory reference evidence. See the historical `RELEASE_NOTES_0.66.9.0-RC2.4.4.md` and `VALIDATION_0.66.9.0-RC2.4.4.md` for that earlier gate; RC2.4.3 snapshot-discovery proof is carried forward.

**Historical v0.66.9.0 development note.** This was the first game-facing follow-up after the Forensic Timeline release; current release state is described at the top of this file. It adds reversible Chase H.Q.-specific turbo/speed experiments to the Workbench Game Lab, a TC0100SCN RAM text-character inspector with PNG/raw-pen export, and packages the reusable turbo/HUD investigation scripts that proved the current HUD behaviour.

Key additions: infinite turbo stock (`0x1003A2` freeze), infinite active turbo (`0x100414` timer freeze), current-speed set/freeze (`0x10041C`), improved Game Lab presentation/API parity, `tile.inspect`, reusable turbo/HUD scripts, and explicit timeline frame-boundary documentation.

# ChaseHQ-Native v0.66.8.0 — Forensic Timeline v1

**Status: PROVEN RELEASE.** Promoted from RC7.4 after the focused bounded-history regression and the complete Full Regression Suite both passed on Windows/SDL on 2026-09-29. The final runtime is unchanged from the proven RC7.4 candidate; this promotion updates the release/documentation state only.

v0.66.8.0 introduces the project's **record once, investigate repeatedly** workflow. Forensic Timeline captures deterministic per-frame CHQSTATE state plus the complete bus-write provenance stream, supports offline import/seek/inspection through the ordinary research APIs, can visually replay recorded states in SDL, can fork a historical frame back into LIVE execution, and can generate portable automated memory/writer/disassembly/graphics analyses without replaying gameplay.

The final RC7.x work also made long-running Workbench regression/history discovery safe: run-history responses are bounded and summary-first, browser output is bounded, thumbnails default off, and historical run filtering supports RC-aware versions plus PASS/FAIL/CANCELLED status.

**Release proof:**
- `Regression - v0.66.8.0 RC7.4 Bounded History Discovery` — PASS, 1.071 s, 2026-09-29.
- `Full Regression Suite` — PASS, 78.422 s, 2026-09-29, ending with the complete regression marker and no browser OOM.
- Earlier focused RC7 proof also passed portable automated timeline analysis and the real existing turbo forensic analysis without replaying gameplay.

Start with `docs/PROJECT_STATE.md`, `docs/ROADMAP.md`, `docs/FORENSIC_TIMELINE.md`, `docs/RELEASE_PROCESS.md`, and `docs/CHATGPT_HANDOVER_PROMPT.md`.

# ChaseHQ-Native v0.66.7.9 — Research Recovery / Release Discipline

**Status:** **PROVEN RELEASE**. Windows/SDL build, Fast Release Gate, v0.66.7.9 Research Recovery regression and the final Full Regression Suite all passed on 2026-09-28. Runtime/emulator semantics are carried forward from v0.66.7.8; this revision consolidates the proven runner/discovery fixes, documents the canonical release process, corrects stale turbo/HUD research framing, and adds reusable recovery/gameplay/graphics scripts so work can return to Chase H.Q. fidelity research.

Start with `docs/RELEASE_PROCESS.md` for build/prove-off discipline and `docs/RESEARCH_RECOVERY_PLAN.md` for the substantive next research sequence.

# ChaseHQ-Native v0.66.7 — Workbench Knowledge / Documentation Foundation

**Status:** implemented; Windows/SDL proof-off required. **Previous proven baseline:** v0.66.6.1.

See `docs/CHATGPT_HANDOVER_PROMPT.md` for the authoritative cross-conversation continuation context. v0.66.7 adds the Workbench Docs/Knowledge system, API/script documentation access and export, evidence-backed knowledge, individual-sprite Graphics Lab controls, and expanded API/scriptable SDL window controls.


## v0.66.6.1 layered graphics forensics

- Structured frame snapshots are now v2 reconstructable bundles with `final.png`, `final-no-hud.png`, `hud-only.png`, exact alpha contribution layers, raw source-layer renders, reconstruction metadata and semantic sprite evidence.
- Chase H.Q. HUD isolation currently maps to the TC0100SCN text layer and is suppressed at source rather than masked by screen coordinates.
- `image.compare` / `image.diff` accept `hud=normal|hidden|only` for deterministic HUD-aware comparisons.
- Graphics Lab can inspect saved snapshots, switch HUD modes, reconstruct contribution layers and toggle/solo/adjust opacity per layer.
- Same-build API/script schemas, Markdown references and regression coverage are included.

Adds exact TC0110PCR palette-write PC tracing, same-build API/Script documentation and regression coverage, correct whole-run duration metadata, descriptive bundle download names, and a release validation gate. Chase H.Q. remains the primary target; the immediate use is tracing brake-lamp palette entries 1037/1053 back to the game writer PC.


This build follows the project rule **investigate/remediate first, build second**. Non-blocking Workbench polish is backlog-only; the immediate substantive target after reliability validation is map 516 OBJ/spritemap decode provenance. See `docs/PROJECT_STATE.md`.

# ChaseHQ-Native v0.66.3 — Research Contracts, Run Artifacts & Sprite Provenance

v0.66.3 carries forward the v0.66.2 steering/driving controls and adds the research infrastructure needed for the next graphics/debugging pass:

- authoritative, queryable API and Script Language schemas with complete generated reference documentation;
- dedicated per-run artifact folders with exact script source, console output, generated/changed artifacts, metadata, manifest and automatic ZIP bundle;
- native `sprite.map.inspect` provenance for spritemap offsets, chunk tile codes and raw pen histograms;
- SDL-native interactive research prompts through `api ui.prompt ...`;
- regression coverage for the new contracts/provenance surface.

See `BUILD_NOTES_0.66.3.txt`, `VALIDATION_0.66.3.md`, `docs/API_REFERENCE.md`, `docs/SCRIPT_LANGUAGE_REFERENCE.md`, and `docs/SCHEMA_POLICY.md`.

## v0.66.2 gameplay milestone

Authoritative player steering was revalidated and promoted into the live Gameplay Registry. SDL keyboard controls route through the arcade IOC path (Left/Right steering, Up accelerator, Down brake, Space turbo), and the Workbench can optionally drive steering from browser gamepad axis 0. The game retains its own nonlinear steering/handling response rather than patching lateral RAM. A permanent deterministic steering regression is included.

## v0.66.1.2 regression/resilience milestone

This preserved the v0.66.1 legacy-findings/API roll-up and v0.66.1.1 startup resilience work while closing Windows acceptance issues around runtime log aliases, failed-run UI state, history migration, regression completion/filtering and listener logging.

# ChaseHQ-Native v0.66.0.6

**Research Automation, Introspection & Audio Foundations**

See `BUILD_NOTES_0.66.0.txt` and `docs/platform/current/RESEARCH_SCRIPTING_0.66.0.md`.

# ChaseHQ-Native v0.65.1 — Live Graphics, Script Query & Experimental Profiles

> **Current scripting reference:** `docs/platform/current/RESEARCH_SCRIPTING_0.65.0.md`.

This release carries forward the v0.64.9 graphics/autonomous-research baseline and adds live SDL sprite experimentation, API-backed query functions, structured state snapshots, improved image regions, direct frame-snapshot image comparison, and a reorganised reusable script/regression library.

## Start

Use `Start-ChaseHQ.ps1` for the normal one-command gameplay/research session, or `Start-ChaseHQResearch.ps1` for parameter-heavy research launches.


## v0.65.1 experimental profile layer
- Adds **read-only, additive game-profile descriptors** under `games/<id>/profile.json`.
- Chase H.Q. remains the active/default/authoritative runtime; no established addresses or emulator paths were moved.
- Adds an **SCI metadata-only probe descriptor** for future cross-game quantification. It cannot activate or load SCI.
- New read-only API: `GET /api/v1/profiles` and `GET /api/v1/profile?game=sci`.
- New script API actions: `api profiles.list`, `api profile.get id=sci`, plus `getprofiles()` for dynamic scripts.
- This layer exists to validate/generalise tooling only when doing so benefits Chase H.Q.; it is not a second emulation target.

## v0.65.0 highlights

- Live SDL sprite inspect / map / palette / visibility presentation overrides.
- Sprite solo, hide and frame-driven flash visualization.
- `getregions()`, `getsprites()`, `getmaps()`, `getpalettes()`, `getpaletteentries()`, `getcheckpoints()`, `getcapabilities()`.
- Query functions are thin wrappers over the Research API and print their backing API result when called directly.
- `api state.snapshot` for coherent current research state.
- Improved image regions and structured `frame.snapshot` name resolution in image compare/diff.
- Reusable graphics/evidence scripts and living regression coverage.
- Script categories documented in `research/scripts/SCRIPT_CATALOG.md`.

See `docs/platform/current/RELEASE_NOTES_0.65.0.md` and `VALIDATION.md`.

## Command-line build workflow (v0.66.0.6)
Visual Studio IDE is no longer required for normal rebuilds. On Windows, double-click or run `Build-Debug.bat` for the normal development build. `Prepare-Project.bat` first unblocks project `.ps1` files and locates CMake from `tools\cmake`, PATH, or Visual Studio Build Tools. `Build-And-Run.bat` builds, tests, then launches the standard `Start-ChaseHQ.ps1` workflow. `Rebuild-All.bat` performs clean Debug + Release builds.

The project does not redistribute a CMake binary; `tools\cmake\bin\cmake.exe` is an optional portable location if you want to pin one locally.

## v0.66.5.1 Track Recorder scripting hotfix

- Shared server-side Track View recorder backs both browser controls and Research Script.
- Added start/stop/clear/status/sample actions plus deterministic SVG export.
- Empty SVG exports are rejected.
- Memory-trace documentation/regression now uses `width=any` when the producer transaction width is unknown.
- `Build-Debug.bat` remains the canonical entry point, still runs `Prepare-Project.bat` first to unblock PowerShell files, and still tolerates only the exact historical `cpu_bus_rom_tests (SEGFAULT)` signature.


### v0.66.7.5 built-in User & Research Handbook
The Workbench Docs / Knowledge tab now provides a user-facing handbook, complete schema-generated API and scripting references, project knowledge/glossary views, cross-domain search and full HTML/PDF handbook export. See `research/knowledge/handbook.json` and `docs/KNOWLEDGE_SYSTEM.md`.


### v0.66.7.6 Docs / Knowledge consolidation
Consolidates article-body search, human-readable Project Knowledge articles, article-aware standalone export, grouped knowledge navigation and release-hardening validation. Windows/SDL prove-off remains required before this release candidate is marked proven.

### v0.66.7.7 Run-history / snapshot persistence hotfix

- Recent Script Runs now merges authoritative on-disk `run-metadata.json` records with browser-local history, so completed runs remain visible even if the browser did not persist local history.
- `GET /api/v1/script/runs` and `api script.runs` expose the same authoritative completed-run catalogue.
- Graphics Lab structured snapshot discovery now includes snapshots stored under completed run artifact roots (`evidence/sessions/<session>/runs/<run>/artifacts/frame-snapshots`).
- Snapshot manifest/asset endpoints resolve those completed-run snapshots by name, preferring the newest match when names repeat.


### v0.66.7.8 RC2
Cross-version run/snapshot discovery performance hotfix: avoid recursive evidence scans that can monopolize the Workbench backend; cache discovery results briefly. Full SDL source is no longer bundled in the RC package.
### v0.66.7.8 RC4
RC4 fixes the Script Console completion lifecycle discovered during RC3 prove-off: the RUNNING ticker is stopped before finalization, a distinct FINALIZING state is shown while the one authoritative bundle/history entry is committed, and COMPLETE/DONE is published only after finalization succeeds.



## RC5 validator correction
The RC4 Workbench validator incorrectly used multiline-sensitive regex assertions without singleline mode, causing a false build failure even though the runner ordering was correct. RC5 changes only those validation expressions to `(?s)` singleline matching; runtime semantics are unchanged.

### v0.66.8.0 RC3

- `timeline.trim.event` is retained for compatibility but is now **non-destructive**: it creates an indexed event-window view under `analysis/` instead of deleting frame checkpoints or rewriting `writes.csv`.
- Event-edge discovery streams `writes.csv` line-by-line and stops as soon as the matching falling edge is found; it no longer `Import-Csv`s the full write stream or grows a PowerShell array with `$rows +=`.
- `timeline.scan.memory` now streams and aggregates changed writes rather than loading the full CSV into memory. When `analysis/event-window.json` exists, it automatically scopes analysis to that event window.
- The original `.chqtimeline` remains immutable and reusable for future investigations.

### RC7.3 Workbench UI resilience
The Script Console defaults artifact thumbnails off, summarizes artifact counts by type, bounds opt-in previews and live output DOM, and adds RC-aware Version plus PASS/FAIL/CANCELLED filters to Recent Script Runs. Complete output and artifacts remain authoritative in the run bundle.


Release evidence summary: `docs/RELEASE_PROOF_0.66.8.0.md`.

## v0.66.9.0-RC2 research candidate
RC2 adds corrected sprite-map lookup, live same-priority sprite-order A/B controls, clearer HUD-vs-internal speed semantics, run-state-preserving Game Lab mutations, frame-safe tile-inspector exports, and stricter script error handling. See `RELEASE_NOTES_0.66.9.0-RC2.md`.

## RC2.2 candidate note
Use `v0.66.9.0-RC2.2` for Windows proof. RC2.2 supersedes RC2.1 by changing the native Chase H.Q. same-priority sprite default to `lower-slot`, matching MAME's reverse sprite-RAM traversal; `higher-slot` remains a reversible diagnostic mode.

### v0.66.9.0-RC2.2 candidate
Chase H.Q. same-priority sprite traversal now defaults to **lower-slot wins**, matching the current MAME `chasehq_draw_sprites_16x16` reverse sprite-RAM traversal. The higher-slot mode remains available for controlled forensic comparisons.


### v0.66.9.0-RC2.3 build-gate correction
RC2.3 supersedes RC2.2. The RC2.2 sprite-order regression had been appended after the Full Regression `COMPLETE` marker, violating the canonical build gate. RC2.3 moves that include before the final marker. Runtime/emulator behaviour is unchanged from RC2.2.
### RC2.4.1 launcher hotfix
`Start-ChaseHQ.ps1 -Restart` now cleans up a stale Web Workbench from an older extracted ChaseHQ build when it owns the requested HTTP port through HTTP.sys. Cleanup is port-scoped and never terminates PID 4/System.


## RC2.4.2
See `RELEASE_NOTES_0.66.9.0-RC2.4.2.md`. This hotfix makes newly captured structured snapshots immediately discoverable and adds the packaged TC0100SCN post-fix investigation/regression scripts.
