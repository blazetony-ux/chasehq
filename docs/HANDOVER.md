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


RC2.6.1 is a packaging/regression-labelling patch over RC2.6. The sprite-decoder source correction is unchanged. The focused Script Console regression now follows the established version-specific convention as `research/scripts/regression/regression-v06690-rc261-sprite-bitplane-significance.chqscript`, and Full Regression includes it before the terminal COMPLETE marker.
## v0.66.9.0-RC2.6.1 active candidate — versioned sprite bitplane regression packaging

RC2.6 is built from the uploaded RC2.5 source tree after the `Sprite Bitplane-Significance Causal Probe` established the remaining sprite-colour error at Stage 1 frame 2352 / Map 405 / palette bank 175. The experiment changed only palette entries selected by the *currently decoded* pens; 16 visible Map-405 sprites became coherent shaded cloud artwork while geometry/priority stayed unchanged and the layered snapshot still reported `verifiedExact=true`, `mismatchedPixels=0`. Bundle SHA-256: `343c14d710023fbb308312900dc453d2eed365409243242c634493aece42c6b2`.

The implementation corrects the source model, not the symptom: `src/sprite_gfx.h` is now the shared live/export 16x16x4bpp decoder and assigns physical planes 0,1,2,3 to pen bits 3,2,1,0. `tests/sprite_gfx_tests.cpp` exhaustively asserts all 16 mappings plus geometry/transparency invariants. Existing descending traversal, first-nontransparent occupancy, black-pen opacity, priority masking and visible-coordinate logic are unchanged.

**Proof already completed in the available non-SDL environment:** `sprite_gfx_tests` PASS and `sprite_priority_tests` PASS.

**Immediate Windows task:** run `Build-Debug.bat`, `Test-SpriteGfx.bat`, `Test-SpritePriority.bat`, then `Regression - v0.66.9.0 RC2.6.1 Sprite Bitplane Significance`, then the packaged `Sprite Bitplane-Significance Native Prove-Off`, then `Regression - Sprite Ownership and Export Coordinates`, then Full Regression through `=== ChaseHQ Full Regression: COMPLETE ===`. Inspect player car, roadside sprites, target/end-level sprites and HUD before promotion.

The RC2.5 evidence also exposed a separate `scene-no-sprites.png` forensic-export defect (it equals `final.png` despite a non-empty sprite layer). It is tracked but intentionally not mixed into the RC2.6 decoder correction.

RC2.5 consolidates the proven player-car sprite-ownership correction and the scripting/research reliability gaps exposed while starting the TC0100SCN sky/rowscroll investigation.

**Sprite correction is already Windows-proven on the RC2.4.4 working tree:** canonical frame 2352 reports body slot 82/map 475 ahead of shadow slot 81/map 575; `overlapPixels=689`, `frontOwnedOverlap=553`, `backVisibleOutsideFront=247`, `exportCoordinatesExact=true`. The focused ownership regression passed and the permanent Full Regression subsequently reached `=== ChaseHQ Full Regression: COMPLETE ===` in session `20260930_151202`, run `002` (118.824 s). The pristine RC2.4.4 control build independently reproduces the historical `cpu_bus_rom_tests` SEGFAULT, so that failure is pre-existing and unrelated to the sprite change.

RC2.5 additionally adds safe Research Script lifecycle support (`try ... finally ... end`, idempotent `timeline.reset`), schema/Script-Console allow-list parity checking, corrected diagnostic script attribution, current-session history as the default view, richer script-header guidance, and a packaged safe rowscroll raster-mapping probe. These new Workbench/script features require a fresh RC2.5 Windows regression gate before this package is promoted from candidate to proven.

# Historical v0.66.9.0-RC2.4.4 handover

RC2.4.3 same-turn snapshot discovery is Windows-proven from the uploaded focused regression. The exact current-run snapshot was returned immediately by the assertive `frame.snapshot.list require=NAME` command and the run finished PASS.

The subsequent Full Regression did not execute because the permanent suite now expands to 503 commands, exceeding the historical 500-command Research Script v2 ceiling. RC2.4.4 raises that ceiling to 1000 in browser/server/schema/reference while retaining the existing source-size, source-line, loop-iteration and nesting limits.

The same diagnostic exposed stale `0.66.8.0` startup/research-session build metadata. RC2.4.4 derives that metadata from the project source version instead of hard-coding it.

Root `AGENTS.md` is now part of the package and should be read first by Codex/coding agents. Fast-changing status still belongs here and in PROJECT_STATE/ROADMAP.

RC2.4.4 Full Regression subsequently passed. The player-car shadow investigation then isolated and corrected sprite occupancy semantics; that correction is carried into RC2.5. The current RC2.5 gate is a fresh Windows build/start, focused Script Safety regression, focused Sprite Ownership regression, and Full Regression because RC2.5 changes Workbench/script lifecycle semantics.

# v0.66.9.0-RC2.4.3 handover

RC2.4.3 is a Workbench reliability hotfix; native TC0100SCN/video behavior is unchanged from RC2.4.2. RC2.4 remains the evidence-backed Y-scroll correction with neutral BG offsets.

Two RC2.4.2 Windows run bundles exposed that `frame.snapshot` successfully created a current-run structured snapshot, but the immediately following `frame.snapshot.list` omitted it and returned only older snapshots. The RC2.4.2 regression was non-assertive, so that failure incorrectly reported PASS.

RC2.4.3 records every newly completed snapshot in an explicit same-process registry, merges that registry into snapshot listing and named resolution, invalidates snapshot/root/session-root discovery caches, and adds `frame.snapshot.list require=NAME`. The current regression uses `require=` so a recurrence is a hard script failure.

The RC2.4.2 post-Y-fix capture localized the visible sky texture to raw `bg-bottom` source data before composition. The next graphics investigation is player-car shadow/under-car ordering, not further sky offset tuning.

Next gate: Windows build/start, run `Regression - v0.66.9.0 RC2.4.3 Snapshot Discovery`, `Validate-FrameSnapshotRefresh.ps1`, and `Validate-TC0100SCN-Y.ps1`. Then resume the car-shadow investigation.

## Active candidate — v0.66.9.0-RC1

Current proven baseline remains v0.66.8.0. v0.66.9.0-RC1 adds Chase H.Q. Game Lab turbo/speed controls and a TC0100SCN RAM text-character inspector, and packages the reusable turbo/HUD investigations. Run the focused Game Lab/tile regression before Full Regression.

# Current handover status — v0.66.8.0

Current proven baseline is v0.66.8.0. For active continuation use `CHATGPT_HANDOVER_PROMPT.md`; the remainder of this file is retained historical handover material.

# ChaseHQ-Native handover — v0.66.7.5

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

## Project priority
Chase H.Q. gameplay/render fidelity is the project. The Workbench exists to support that investigation. Do not allow non-blocking UI/tooling polish to become the main workstream.

## Mandatory workflow
**Investigate/remediate first, build second.** Before creating a new build: reproduce deterministically; query the documented API/schema; inspect logs/evidence/source; patch PowerShell/scripts locally where possible; validate the patch in-place; rebuild native code only when evidence requires it; package only after the change is proven as far as the available environment allows.

When the next step is a command, provide the exact one-line PowerShell command immediately. Do not invent API/script commands. Use `api actions`, `api schema action=NAME`, `api schema.all`, and `api script.schema`.

## Current confirmed game finding
Physical brake changes palette banks 64/65 pen 13 from `0x000E` to `0x18DE`. Player slot 45 / map 516 / palette 64 contains exactly 140 pen-13 pixels in the same 60x10 rear-window region affected by physical braking. `sprite.map.inspect slot=45` reports OBJ A, spritemap word base `0x8100`, byte base `0x10200`, `pen13=140`.

### Next substantive investigation
Raw OBJ-A provenance is now resolved: the corrected standalone decoder matches the native renderer/API and confirms exactly 140 pen-13 source pixels in map 516. Physical and scripted brake paths also match under genuine player gameplay. Use the new palette trace on indices 1037/1053 to identify the exact writer PC(s), disassemble the palette decision, and compare brake-only with brake during active turbo.

After that, return to turbo/HUD, target health/damage, end-level transition, and track branch/topology work.

## v0.66.3.1 reliability boundary
This hotfix is intended to close the current reliability detour, not start another tooling expansion. It fixes: restart port ownership, full-regression include ordering, compact session/run paths, run-owned artifacts/bundles, explicit build/session metadata, pasted-script provenance/name/purpose, shared selected-row history controls, thumbnail/path reliability, and script metadata/include validation.

New session layout:
```text
evidence/sessions/YYYYMMDD_HHMMSS/
  session.json
  native-*.log
  evidence/                 # non-script/manual session evidence
  runs/
    001/
      script.chqscript
      console-output.txt
      run-metadata.json
      artifacts/            # script-generated frames/snapshots/evidence/etc
      bundle-manifest.json
      bundle.zip
```

Run IDs are sequential within the already timestamped session. Build and session are stored explicitly in metadata; never infer version from folder names.

## Script naming/provenance
Ad-hoc scripts supplied in chat should include:
```text
# name: Meaningful investigation name
# purpose: What this script is proving or measuring.
# category: graphics-research
```
Unchanged saved scripts are `LIBRARY`; edited loaded scripts are `MODIFIED`; pasted/freeform scripts are `PASTED`.

## Release-quality gate added after v0.66.3.1 validation failure
The v0.66.3.1 packaging incident exposed a validator bug that should have been caught before a ZIP was handed over: `Validate-Workbench.ps1` used a double-quoted regex containing `$sessionPath`, so StrictMode attempted to expand an undefined variable. The local correction was proven by rerunning the validator successfully.

Going forward, packaging is the final step, never the discovery step. A candidate build is not ready to package until the exact working tree has passed the following gate:

1. **Static validation**
   - parse/StrictMode-check every PowerShell script without executing project-specific side effects;
   - JavaScript syntax check;
   - all `.chqscript` includes resolve;
   - required script metadata is present;
   - version consistency is authoritative and does not depend on folder-name regexes;
   - API/schema/docs remain synchronized;
   - validator/source strings are checked for accidental PowerShell interpolation such as `$name` inside double-quoted regex literals;
   - calculate representative/worst-case generated artifact paths against the actual project-root length and reject unsafe layouts before runtime.

2. **Local workflow smoke tests on the exact tree**
   - `Build-Debug.bat`;
   - clean start;
   - `Start-ChaseHQ.ps1 -Restart`, including port-owner cleanup and reacquisition;
   - one unchanged library-script run;
   - one modified-library-script run;
   - one pasted/ad-hoc run with `# name:` and `# purpose:` metadata;
   - one `frame.snapshot` run with its `final.png` physically present;
   - inline thumbnail and Artifact Viewer both resolve that same canonical file;
   - per-run metadata and `bundle.zip` exist and report the correct build/session/provenance.

3. **Regression gate**
   - run the build-specific reliability regression first;
   - run Full Regression;
   - require the final COMPLETE marker to be the final regression command/output as designed;
   - fail on unexpected `ERROR:` output;
   - fail when a regression-declared artifact is missing even if the producing command otherwise returned success.

4. **Package last**
   - package only the already validated tree;
   - rerun static/release validation against the packaged contents;
   - preferably extract the ZIP to a fresh temporary directory and run validation there so packaging/path/omission errors are caught before handover.

A dedicated `Validate-Release.ps1` should be added in the next build and become the mandatory packaging gate. Its successful terminal state should be an explicit `READY TO PACKAGE: YES`; otherwise packaging must stop with a precise reason.

### One-source-of-truth rule
Do not infer authoritative state from paths or UI text when a real source exists. In particular:
- build version comes from the authoritative build/version definition and stored run metadata, never directory-name parsing;
- session/run identity comes from explicit metadata, never path regexes;
- API documentation/schema/regression contracts derive from the same authoritative action definitions;
- script provenance is explicit (`LIBRARY`, `MODIFIED`, `PASTED`, etc.), not guessed from text similarity or previously selected scripts;
- artifact URLs/viewer links and artifact writes must use the same canonical path mapping.

### Change-development rule
For a native/API/tooling change, require implementation + schema + documentation + regression + one live smoke test + handover update before considering it complete. For PowerShell/UI-only work, avoid rebuilding the native executable unless the change actually requires it.

## v0.66.5.1 Track Recorder scripting hotfix

- Shared server-side Track View recorder backs both browser controls and Research Script.
- Added start/stop/clear/status/sample actions plus deterministic SVG export.
- Empty SVG exports are rejected.
- Memory-trace documentation/regression now uses `width=any` when the producer transaction width is unknown.
- `Build-Debug.bat` remains the canonical entry point, still runs `Prepare-Project.bat` first to unblock PowerShell files, and still tolerates only the exact historical `cpu_bus_rom_tests (SEGFAULT)` signature.

## v0.66.7 continuation contract
`docs/CHATGPT_HANDOVER_PROMPT.md` is now the authoritative cross-conversation continuation prompt. Ship it with every build and use it alongside the latest ZIP when starting a new ChatGPT conversation. Its build/proof status must be kept current. v0.66.7 adds the Workbench Docs/Knowledge foundation, API/script access, HTML/PDF export, documentation evidence, per-sprite Graphics Lab controls and expanded SDL window/runtime controls.

## v0.66.7.2 prove-off note
The initial v0.66.7 Windows run reached Docs/Knowledge and SDL/window API checks successfully, then stopped because the packaged regression called `checkpoint.load name=...`; the authoritative API requires `file=...`. v0.66.7.2 fixes that regression call and the per-sprite label encoding artifact (the mojibake middle-dot artifact). Re-run the v0.66.7.2 regression before promoting this build to proven.


## v0.66.7.2 documentation integration hotfix
The v0.66.7.1 Windows regression passed the substantive Docs/Knowledge, SDL control and per-sprite snapshot checks, but review found two integration defects: `docs.handover` was decoded with the host default encoding when invoked through `.chqscript`, producing mojibake for UTF-8 punctuation; and `docs.export` used the session-level export directory captured at Workbench startup, so script-generated documentation exports did not follow the active run artifact root and were omitted from that run bundle. v0.66.7.2 forces UTF-8 for handover reads and resolves the documentation export directory dynamically from the current evidence root, preserving normal Workbench exports while making script-run exports portable in their run bundle. No API action names or `.chqscript` syntax changed.


## v0.66.7.3 build identity correction
The v0.66.7.2 Windows regression passed functionality, UTF-8 handover delivery, portable docs export, individual-sprite capture, and exact layered reconstruction. Evidence review found stale `0.66.7.1` identifiers in Workbench-host metadata. v0.66.7.3 synchronizes current build identity across native/runtime/Workbench snapshot and frontend metadata; no API or script semantics change.
## v0.66.7.5 identity sweep
A Windows smoke run of v0.66.7.3 exposed that the native SDL title reported 0.66.7.3 while the Workbench header still reported 0.66.7.2. v0.66.7.5 fixes all remaining live Workbench/runtime identity literals and strengthens validation to detect this class of mismatch before packaging. Historical version notes are intentionally preserved.


## v0.66.7.5 handover note
The v0.66.7.4 knowledge/documentation foundation is proven. v0.66.7.5 reworks the user-facing Docs / Knowledge tab into a readable User & Research Handbook with authored guides, complete schema-generated API and `.chqscript` references, grouped cross-domain search, copyable examples, richer project-knowledge pages, and full handbook HTML/PDF export. `research/knowledge/handbook.json` is now carried and versioned alongside `research/knowledge/documentation.json`. v0.66.7.5 awaits Windows/Workbench regression proof.


## v0.66.7.9 release / research recovery handover

The canonical build/prove-off process is now `docs/RELEASE_PROCESS.md`. Follow it exactly. The v0.66.7.6-v0.66.7.8 cycle exposed packaging, parser/strict-mode, version identity, discovery performance, run finalization and runner-UI lifecycle defects; those are now explicit release contracts rather than tribal knowledge.

The substantive research plan is `docs/RESEARCH_RECOVERY_PLAN.md`. Turbo gameplay state is already confirmed; focus on the bottom-HUD rendering/source path. Use the packaged recovery and turbo/HUD scripts before adding new tooling.

## v0.66.9.0-RC2 handover delta
Current immediate task: run the RC2 focused regression, then use the existing `car-shadow-layer-investigation` timeline for higher-slot/lower-slot A/B rendering. Do not replay gameplay unless the recording is unavailable.

## RC2.2 continuation note
The player-car “shadow above car” investigation resolved to same-priority sprite traversal. Current MAME Chase H.Q. code iterates sprite RAM from the highest entry down to zero; lower slots are visited later but cannot overwrite occupied pixels. RC2.2 makes `lower-slot` the native default. Keep `higher-slot` only as a forensic A/B override.


## RC2.3 continuation note
RC2.3 supersedes RC2.2 solely because RC2.2 appended its focused regression after the Full Regression COMPLETE marker. The include is now before the terminal marker. No renderer/API/script behaviour changed from RC2.2.


## RC2.4.2 continuation note
The current build is v0.66.9.0-RC2.4.2. The TC0100SCN Y-scroll fix from RC2.4 is retained. `frame.snapshot.list` previously could omit a snapshot created immediately beforehand because the Workbench cached structured-snapshot discovery for five seconds; capture now invalidates both snapshot and root caches. Use `research/scripts/graphics/tc0100scn-post-y-fix-layer-isolation.chqscript` for the next deterministic sky/banding localization step.
