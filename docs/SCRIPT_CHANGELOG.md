
## RC2.7 public contracts

The full release label comes from `src/version.h`. These actions share native control paths and are exposed in the Script Console allow-list and authoritative schema. Layer offsets are presentation overrides, default zero, bounded -4096..4096, and do not change hardware RAM. `tc0100.mode` is an old/corrected diagnostic, not shadow verification.

- `layer.offsets.get` — Read live presentation offsets for all five layers. Parameters: none. Example: `api layer.offsets.get`.
- `layer.offsets.set` — Set reversible live presentation offsets; hardware state is unchanged. Parameters: layer=bg0|bg1|text|sprites|road|all required, x=-4096..4096 required, y=-4096..4096 required. Example: `api layer.offsets.set layer=bg0 x=0 y=4`.
- `layer.offsets.reset` — Reset presentation offsets to zero. Parameters: layer=bg0|bg1|text|sprites|road|all optional default all. Example: `api layer.offsets.reset layer=all`.
- `layer.offsets.assert` — Assert live layer offsets against expected values. Parameters: layer=bg0|bg1|text|sprites|road required, x=INTEGER required, y=INTEGER required. Example: `api layer.offsets.assert layer=bg0 x=0 y=4`.
- `tc0100.mode` — Select diagnostic old/new X transform; this is not independent shadow verification. Parameters: mode=legacy|corrected optional. Example: `api tc0100.mode mode=corrected`.
- `release.assert-version` — Require native status, Workbench and feature manifest to share the full release label. Parameters: none. Example: `api release.assert-version`.
- `frame.snapshot.assert-exact` — Require exact=true, verifiedExact=true and zero reconstruction mismatches. Parameters: name=NAME required, requireColumnZero=true|false optional default false. Example: `api frame.snapshot.assert-exact name=rc27-corrected`.
- `frame.snapshot.assert-scene-no-sprites` — Require exact reconstruction, visible sprite ownership and changed no-sprites pixels. Parameters: name=NAME required. Example: `api frame.snapshot.assert-scene-no-sprites name=rc27-corrected`.

Native CLI equivalents: `chqctl layer-offset get`, `chqctl layer-offset set bg0 0 4`, `chqctl layer-offset reset all`, `chqctl tc0100-mode legacy|corrected`. Existing startup `--layer-offset`/config `layer_offset` remain supported. Script API actions can also be submitted through the documented HTTP script endpoint; no new dedicated REST routes are claimed.

## v0.66.9.0-RC2.4.4

- Research Script v2 expanded-command ceiling raised from 500 to 1000 in both browser and server-side compilation paths.
- The current permanent Full Regression expands to 503 commands; this change restores the ability to run it without weakening the existing loop/nesting/source-size safeguards.
- Authoritative `research/schema/script-language.json` and `docs/SCRIPT_LANGUAGE_REFERENCE.md` report the same 1000-command limit.

## v0.66.9.0-RC2.4.3

- No Research Script grammar change.
- `api frame.snapshot.list` accepts optional `require=NAME` for assertive same-turn snapshot discovery.
- Added `regression-v06690-rc243-snapshot-discovery.chqscript`; unlike the superseded RC2.4.2 check, it fails if the newly captured snapshot is absent.
- The RC2.4.3 regression is included before the terminal Full Regression COMPLETE marker.

## v0.66.9.0-RC1

Bundled reusable investigations added: Turbo HUD Layer Sprite Correlation, Turbo HUD Exact Tile Writer Investigation, Turbo HUD Text Plane Neighbourhood Investigation, Turbo Gameplay To HUD Writer Path, Turbo HUD Tile Character Inspection, and a Game Lab turbo/speed demo. Added focused regression `Regression - v0.66.9.0 Game Lab and Tile Inspector`.

## 0.66.7.5
- No semantic changes. Completed a full live build-identity sweep after Windows smoke testing found stale v0.66.7.2 Workbench/health/OpenAPI branding in the v0.66.7.3 package.

## 0.66.7.3
- No semantic changes. Corrected stale Workbench/runtime build identity so snapshot manifests, health/frontend metadata and run-history defaults report 0.66.7.3 consistently.

## 0.66.7.2
- Corrected the current regression script from obsolete `checkpoint.load name=...` to documented `checkpoint.load file=... run=false`. No scripting-language syntax changes.

## 0.66.6.1

- No new script syntax. The bundled layered-frame regression is updated for the reconstruction hotfix; `frame.snapshot` now fails at capture time if exact reconstruction verification fails.

# Research Script changelog

## v0.66.6.1
- No grammar change.
- Upgraded `frame.snapshot` to the reconstructable v2 layered format.
- Added the `hud=` parameter to `image.compare` / `image.diff`.
- Added `regression-v06661-layered-frame-snapshot.chqscript` and included it in Full Regression.


## v0.66.5.1
- Research Script can now drive the same Track View recorder used by the browser.
- Added deterministic `track.record.sample` and scripted SVG export.
- Added v0.66.5.1 track-recorder regression and a full 3600-frame scripted SVG survey.


## v0.66.5

- Exposed generic `memory.trace.*` actions to Research Script v2 and added `regression-v0665-memory-trace.chqscript`.
- Script Console Syntax Preview was removed; validation, schema help and execution remain unchanged.


## v0.66.4

- No grammar change.
- Added five documented API actions for palette-write tracing: `palette.trace.start/status/tail/stop/clear`.
- Added `regression-v0664-palette-trace.chqscript` and included it in Full Regression.
- Run metadata duration now accumulates across the complete multi-command browser run instead of being overwritten by the final command request.
- Run bundles keep physical `bundle.zip` storage but expose descriptive download names derived from session, run ID and script name.

## v0.66.3.1

- Reliability hotfix: no Script API action removals.
- REST Script Console run submission now records explicit run name, purpose, source type/path, build and session metadata.
- Run/session identity is stored metadata; it is not inferred from version-folder names.

## 0.66.3
No existing syntax removed. Added authoritative grammar introspection and SDL-native interactive research workflow through `ui.prompt`.

## 0.66.7
- No new script grammar.
- Added documented `api ...` actions for Docs/Knowledge and expanded SDL window controls.
- Added v0.66.7 regression coverage for schema discovery, knowledge search/handover and non-destructive window state queries/control round-trips.


## v0.66.7.2 documentation integration hotfix
The v0.66.7.1 Windows regression passed the substantive Docs/Knowledge, SDL control and per-sprite snapshot checks, but review found two integration defects: `docs.handover` was decoded with the host default encoding when invoked through `.chqscript`, producing mojibake for UTF-8 punctuation; and `docs.export` used the session-level export directory captured at Workbench startup, so script-generated documentation exports did not follow the active run artifact root and were omitted from that run bundle. v0.66.7.2 forces UTF-8 for handover reads and resolves the documentation export directory dynamically from the current evidence root, preserving normal Workbench exports while making script-run exports portable in their run bundle. No API action names or `.chqscript` syntax changed.


## 0.66.7.5
- No new language syntax.
- Added script-visible `docs.handbook` and `docs.page` API actions.
- Added Workbench Scripting Reference generated from the authoritative script-language schema, with copyable examples.


## 0.66.7.7
- No language grammar change.
- Added Script API action `script.runs` for authoritative completed-run discovery.

## v0.66.7.9

- No new Research API actions or parameter semantics.
- Build identity advanced to v0.66.7.9 for the Research Recovery / Release Discipline candidate.
- New packaged `.chqscript` workflows use the existing documented API surface only.

## v0.66.8.0

- Added script-first Forensic Timeline v1 actions: record/start/stop, list/load/seek/navigation, event trim, offline write-memory ranking, fork-live and unload.
- Existing inspection actions are deliberately reused after timeline seek; no duplicate timeline-specific memory/sprite/read syntax is required.
- Added `research/scripts/timeline/` workflows and permanent `regression-v06680-forensic-timeline.chqscript`.

### v0.66.8.0 forensic source parity
- Existing read-only Research Script actions can operate on the state selected by `timeline.load` / `timeline.seek`.
- Existing `control.step-frame` / `control.run-frames` advance recorded frames when source=timeline.
- Mutating operations require `timeline.fork-live` first.

## v0.66.8.0 RC7

- Added `timeline.play` for paced SDL recorded-state playback.
- Added `timeline.analyze.auto` for portable current-run automated forensic analysis, writer disassembly, milestone snapshots and HUD_TURBO correlation.
- `timeline.scan.memory` derived evidence now belongs to the current run artifact tree instead of mutating/writing into the source timeline.
- No new `.chqscript` grammar; new capabilities use existing `api ACTION key=value` syntax.

## v0.66.8.0 final promotion

- No grammar change during final promotion.
- Timeline automation and playback scripts are proven on Windows/SDL.
- Run-history discovery used by scripts is bounded/summary-first and supports filtering.
- RC7.4 bounded-history regression is retained as permanent coverage.

## v0.66.9.0-RC2
- Unhandled API output beginning with `ERR` now fails the command/run when Stop on error is enabled.
- Added reusable turbo animation and player-car sprite-order scripts.
- Added focused RC2 regression coverage.

## v0.66.9.0-RC2.2
- Added loop-based `graphics/same-priority-sprite-order-multiscene-validation.chqscript`.
- Added `regression-v06690-rc22-mame-sprite-order.chqscript` and Full Regression coverage.


## v0.66.9.0-RC2.3
- Corrected Full Regression integration ordering so the RC2.2 sprite-order regression executes before the suite's terminal COMPLETE marker. No `.chqscript` grammar change.
