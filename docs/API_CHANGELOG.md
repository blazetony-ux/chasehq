
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

# v0.66.9.0-RC2.5

- Add `timeline.reset`: idempotent cleanup for script-initiated timeline work. `timeline.load` captures the pre-load live checkpoint; `timeline.reset` restores it after `timeline.fork-live` and succeeds harmlessly when no transaction is active.
- Add `frame.snapshot.assert-overlap` to the Script Console allow-list as well as its existing implementation/schema, and enforce bidirectional schema/allow-list parity at Workbench startup.
- `script.runs` now exposes `currentSession`; the Workbench uses it to default history to the active session while preserving explicit version/session/all-history filtering.
- Diagnostic bundles attribute `scriptPath` to the actual classified script source rather than stale library selection state.
- No native debugger transport names are removed; legacy sprite-order mode names remain accepted.

## v0.66.9.0-RC2.4.3

- `frame.snapshot.list` adds optional `require=NAME`. When present, the action throws unless exactly one snapshot with that name is present in the list returned for the current request.
- Existing `api frame.snapshot.list` behavior remains unchanged when `require` is omitted.
- Snapshot capture/list/resolution now uses an explicit same-process recent-snapshot registry in addition to disk discovery, closing the same-run stale-list gap exposed by RC2.4.2 Windows evidence.
- No new HTTP route and no Research Script grammar change.

## v0.66.9.0-RC1

- Added Chase H.Q.-specific Game Lab actions for turbo stock, turbo active-duration, turbo fire/timer/count and current-speed set/freeze/clear.
- Added `tile.inspect` for TC0100SCN RAM text-character raw-pen/palette-aware PNG inspection.
- Added localhost Workbench endpoints `/api/v1/game/action` and `/api/v1/tile/inspect`; Research Script remains the authoritative automation surface.
- All new UI controls route through the same action helpers; no UI-only state mutation path was introduced.

## 0.66.7.5
- No semantic changes. Completed a full live build-identity sweep after Windows smoke testing found stale v0.66.7.2 Workbench/health/OpenAPI branding in the v0.66.7.3 package.

## 0.66.7.3
- No semantic changes. Corrected stale Workbench/runtime build identity so snapshot manifests, health/frontend metadata and run-history defaults report 0.66.7.3 consistently.

## 0.66.7.2
- No Research API semantic changes. Corrected the packaged v0.66.7 regression to use the documented `checkpoint.load file=...` parameter so the API/sprite/docs-export proof can complete.

## 0.66.6.1

- `frame.snapshot` v2 reconstruction is now self-verifying. Exported compositor finals are normalised to opaque RGBA and contribution layers encode changed pixels as opaque replacements, fixing lossless reconstruction when a renderer write carries alpha 0.
- `reconstruction.json` now reports `verifiedExact` and `mismatchedPixels`; capture fails rather than claiming success if reconstruction is not exact.

# API changelog

## v0.66.6.1
- `frame.snapshot` now creates reconstructable v2 layered snapshots: exact transparent contribution layers, raw source layers, `reconstruction.json`, and normal/HUD-hidden/HUD-only final variants.
- `image.compare` and `image.diff` add `hud=normal|hidden|only` (`exclude` accepted as hidden) for source-aware HUD comparisons on layered snapshots.
- Added snapshot manifest/asset HTTP routes for the Graphics Lab viewer.
- Native debugger advertises `graphics.layered_snapshot` and accepts `graphics snapshot PATH`.


## v0.66.5.1
- Added shared Track View recorder actions: `track.record.start`, `.stop`, `.clear`, `.status`, `.sample`.
- Added `track.svg.export` for deterministic SVG export from scripts.
- Clarified `memory.trace.start width=` as originating transaction width; `width=any` is the recommended discovery mode for sub-byte targets.


## v0.66.5

- Added generic bounded `memory.trace.start/status/tail/stop/clear` writer-provenance actions with CPU/range/width filtering and exact write-time PC capture.
- Added same-build schema and regression coverage.


## v0.66.4

- Added `palette.trace.start`, `palette.trace.status`, `palette.trace.tail`, `palette.trace.stop`, and `palette.trace.clear`.
- Palette trace events report frame, CPU, writer PC, palette index/bank/pen, complete old/new TC0110PCR raw values, changed flag and write count.
- Trace history is bounded (`limit=64..1000000`, default 4096) and selected by palette index.
- `api capabilities` now advertises `palette.trace`.
- Added same-build regression coverage in `regression-v0664-palette-trace.chqscript`.

## v0.66.3.1

- Reliability hotfix: no Script API action removals.
- REST Script Console run submission now records explicit run name, purpose, source type/path, build and session metadata.
- Run/session identity is stored metadata; it is not inferred from version-folder names.

## 0.66.3
Added `sprite.map.inspect`, `schema.all`, `script.schema`, `ui.prompt`, `/api/v1/schema/actions`, and `/api/v1/schema/script`. Script execution now emits an automatic per-run artifact folder and ZIP bundle.

## 0.66.7
- Added `docs.*` and `knowledge.*` read-only/search/export actions.
- Added SDL window status/fullscreen/scale/show/hide/minimize/restore actions; existing always-on-top remains supported.
- Extended structured frame snapshot artifacts with individual-sprite forensic metadata/assets and a no-sprites scene base.
- HTTP schema/OpenAPI/Workbench controls updated with the same contract.


## v0.66.7.2 documentation integration hotfix
The v0.66.7.1 Windows regression passed the substantive Docs/Knowledge, SDL control and per-sprite snapshot checks, but review found two integration defects: `docs.handover` was decoded with the host default encoding when invoked through `.chqscript`, producing mojibake for UTF-8 punctuation; and `docs.export` used the session-level export directory captured at Workbench startup, so script-generated documentation exports did not follow the active run artifact root and were omitted from that run bundle. v0.66.7.2 forces UTF-8 for handover reads and resolves the documentation export directory dynamically from the current evidence root, preserving normal Workbench exports while making script-run exports portable in their run bundle. No API action names or `.chqscript` syntax changed.


## 0.66.7.5
- Added read-only `docs.handbook` and `docs.page`.
- Full `docs.export` without a filter now emits the User & Research Handbook rather than only the short knowledge catalogue.
- Workbench API Reference is generated from the authoritative action schema.


## 0.66.7.7
- Added `GET /api/v1/script/runs`.
- Added Script API action `script.runs`.
- Extended frame-snapshot listing/resolution to completed run artifact roots.


### v0.66.7.8
`script.runs` keeps the same contract but uses shallow known-layout discovery plus bundle-manifest artifact enumeration and short-lived caching to avoid blocking the Workbench listener. No action names or parameters changed.

## v0.66.7.8 RC3
- `/api/v1/script` browser-run transport now supports internal `transient`, `finalizeRun`, and `runStatus` request fields.
- Normal command chunks append to one RUNNING run without creating a ZIP.
- `transient=true` executes preflight/predicate queries without allocating run metadata or bundles.
- `finalizeRun=true` finalizes an existing run as PASS/FAIL/CANCELLED and creates its manifest/ZIP exactly once.
- Public `.chqscript` action names and language syntax are unchanged.

## v0.66.7.9

- No new Research API actions or parameter semantics.
- Build identity advanced to v0.66.7.9 for the Research Recovery / Release Discipline candidate.
- New packaged `.chqscript` workflows use the existing documented API surface only.

## v0.66.8.0

- Added Forensic Timeline v1 API/script surface for per-frame CHQSTATE capture, full bus-write provenance, import/seek, event trim, offline memory/writer ranking, fork-live and restore.
- `status` now exposes `source=live|timeline`, `timeline_recording` and `timeline_loaded`.
- Native capabilities now advertise `timeline.record`, `timeline.load`, `timeline.seek`, `timeline.fork` and `timeline.write_history`.
- Audio capability metadata explicitly states that Timeline v1 captures only the current sound stub; real sound CPU/chip/PCM capture awaits native audio implementation.

### v0.66.8.0 timeline source semantics
- Imported timelines are read-only until `timeline.fork-live`.
- Existing frame-step/run controls move the timeline cursor while a timeline source is loaded.
- `timeline.scan.memory` exports transaction candidates, byte-address candidates, writer-PC groups and an HTML analysis report.
- `timeline.trim.event` can detect an event field contained within a wider bus write.

## v0.66.8.0 RC7

- Added `timeline.play` for paced SDL recorded-state playback.
- Added `timeline.analyze.auto` for portable current-run automated forensic analysis, writer disassembly, milestone snapshots and HUD_TURBO correlation.
- `timeline.scan.memory` derived evidence now belongs to the current run artifact tree instead of mutating/writing into the source timeline.
- No new `.chqscript` grammar; new capabilities use existing `api ACTION key=value` syntax.

## v0.66.8.0 final promotion

- Forensic Timeline v1 and RC7 portable automated analysis/playback are proven on Windows/SDL.
- `script.runs` discovery is bounded/paginated and summary-first; supports limit/offset/status/version/session filters.
- Large browser command responses are display-capped while authoritative output remains in run evidence.
- No API removals were made during final promotion from RC7.4.

## v0.66.9.0-RC2
- Added `sprite.order.inspect` and `sprite.order.set`.
- `sprite.map.inspect` now accepts `map=N` as well as `slot=N`.
- Added Game Lab aliases `game.sprite.order.inspect` / `game.sprite.order.set`.
- Clarified `game.speed.set/freeze` as raw internal/physics speed (`0x10041C`) and preserved caller run/pause state.
- `tile.inspect` artifacts are frame-safe.

## v0.66.9.0-RC2.2
No API action names changed. The default value of sprite ordering changed: `sprite.order.inspect` now starts from `lower-slot`, matching the MAME Chase H.Q. reverse sprite-RAM traversal. Explicit `sprite.order.set mode=higher-slot|lower-slot` behaviour is unchanged.
## v0.66.9.0-RC2.5 sprite correctness

- Preserve order-control names/traversal; correct first-nontransparent sprite occupancy independently of layer masks. Add explicit ownership semantics to order inspection.
- Snapshots export sprite-stage owner/coverage CSV and corrected individual visible-raster coordinates.
- Add `frame.snapshot.assert-overlap` with offline PowerShell parity and focused regression. No language grammar or release-version change.
