
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

## v0.66.9.0-RC1 Game Lab / tile actions

New scriptable actions: `game.experiment.status`, `game.turbo.stock`, `game.turbo.active`, `game.turbo.fire`, `game.turbo.timer.reset`, `game.turbo.remaining.set`, `game.speed.set`, `game.speed.freeze`, `game.speed.clear`, and `tile.inspect`. Game Lab actions are reversible and target confirmed Chase H.Q. state. `tile.inspect` currently covers TC0100SCN **RAM text characters** (256 x 8x8, 2bpp at tilemap character RAM `0xC06000`); background ROM tile inspection remains future work. See the machine-readable `research/schema/api-actions.json` for the exact live contract.


### Workbench convenience endpoints (v0.66.9.0-RC1)

The Game Lab UI is a convenience front end over the same script actions. `POST /api/v1/game/action` accepts an `action` plus action-specific fields and maps to the documented `game.*` actions. `POST /api/v1/tile/inspect` accepts `code`, `palette`, and `scale` and maps to `tile.inspect`. Automation should prefer the Research Script/API action names so CLI/script/UI behaviour stays aligned.

# ChaseHQ-Native Script API Reference — v0.66.5.1
> **Current proven release:** v0.66.8.0 (2026-09-29). Interfaces documented here correspond to the promoted RC7.4 runtime unless a section is explicitly historical.


**102 documented actions.** This file is generated from the same `$scriptActionSchemas` table used by the running Workbench.

> Maintenance rule: implementation + schema + documentation + regression must change together. Never infer or guess action names.

Runtime discovery: `api actions`, `api schema action=NAME`, `api schema.all`, or `GET /api/v1/schema/actions`.

## Generic memory-write tracing (v0.66.5.1)

`memory.trace.*` captures writer provenance at the `Bus::write` transaction boundary. It is game-agnostic and records matching writes even when the value does not change.

### `memory.trace.start`

**Example:** `api memory.trace.start cpu=A address=0x100303 length=1 width=8 limit=256`

Parameters: `cpu=A|B`, `address=HEX`, optional `length` (default 1), optional `width=8|16|32|any` (default `any`), and optional bounded `limit` (64..1000000, default 4096). A write matches when its transaction overlaps the selected range and, when specified, its write width matches.

Each event records frame, CPU, exact writer PC, write address, width, old value, new value, changed flag and trace-local write count.

### `memory.trace.status` / `memory.trace.tail` / `memory.trace.stop` / `memory.trace.clear`

Examples: `api memory.trace.status`, `api memory.trace.tail count=50`, `api memory.trace.stop`, `api memory.trace.clear`. `stop` retains events; `clear` removes events and resets the trace-local write count.


## Palette write tracing (v0.66.4)

Palette tracing is semantic TC0110PCR provenance, not a generic memory watch. It records the complete palette transaction after the low-byte commit and retains matching writes even when the raw value does not change. This is intended for exact game-code provenance such as the Chase H.Q. brake-lamp palette path.

### `palette.trace.start`

Start bounded tracing for one or more palette indices. Starting a new trace clears prior palette-trace events.

**Example:** `api palette.trace.start indices=1037,1053 limit=4096`

**Parameters:**

- `indices=comma-separated palette indices 0..4095 required`
- `limit=64..1000000 optional default 4096`

Each event records: frame, CPU, exact writer PC, palette index, bank, pen, old raw value, new raw value, changed flag, and cumulative write count.

### `palette.trace.status`

**Example:** `api palette.trace.status`

Returns whether capture is enabled, selected indices, buffered event count, and history limit.

### `palette.trace.tail`

**Example:** `api palette.trace.tail count=50`

**Parameters:** `count=N optional default 20`

Returns the most recent matching writes.

### `palette.trace.stop`

**Example:** `api palette.trace.stop`

Stops capture while retaining buffered events.

### `palette.trace.clear`

**Example:** `api palette.trace.clear`

Clears buffered palette-write events without changing the selected filter.

## `status`

Read machine/frame status

**Example:** `api status`

**Parameters:** none

## `capabilities`

List native debugger capabilities

**Example:** `api capabilities`

**Parameters:** none

## `memory.read`

Read one value

**Example:** `api memory.read cpu=A address=10A048 width=8`

**Parameters:**

- `cpu=A|B required`
- `address=HEX required`
- `width=8|16|32 optional default 16`

## `memory.read-range`

Read a contiguous byte range and return a compact hex dump

**Example:** `api memory.read-range cpu=A address=109000 length=512`

**Parameters:**

- `cpu=A|B required`
- `address=HEX required`
- `length=COUNT required, decimal or 0xHEX, max 65536`

## `memory.write`

Debugger-safe write; pauses machine and reports old/new values

**Example:** `api memory.write cpu=A address=10A04A width=16 value=0051 expect=0011`

**Parameters:**

- `cpu=A|B required`
- `address=HEX required`
- `width=8|16|32 optional default 16`
- `value=HEX required`
- `expect=HEX optional compare-before-write`

## `register.read`

Read one register or complete CPU register set

**Example:** `api register.read cpu=A name=D0`

**Parameters:**

- `cpu=A|B required`
- `name=D0..D7|A0..A7|PC|SR|SP optional`

## `register.write`

Write a CPU register while paused

**Example:** `api register.write cpu=A name=D0 value=00000001`

**Parameters:**

- `cpu=A|B required`
- `name=REGISTER required`
- `value=HEX required`

## `patch.freeze`

Immediately write and continuously freeze a value; returns patch ID

**Example:** `api patch.freeze cpu=A address=10A04A width=16 value=0051`

**Parameters:**

- `cpu=A|B required`
- `address=HEX required`
- `width=8|16|32 required`
- `value=HEX required`
- `expect=HEX optional`

## `patch.replace`

Replace matching game writes with a fixed value

**Example:** `api patch.replace cpu=A address=10A04A width=16 value=0051`

**Parameters:**

- `cpu=A|B required`
- `address=HEX required`
- `width=8|16|32 required`
- `value=HEX required`

## `patch.suppress`

Suppress writes and preserve current value

**Example:** `api patch.suppress cpu=A address=10A04A width=16`

**Parameters:**

- `cpu=A|B required`
- `address=HEX required`
- `width=8|16|32 required`

## `patch.list`

List active live patches

**Example:** `api patch.list`

**Parameters:** none

## `patch.remove`

Remove one live patch

**Example:** `api patch.remove id=1`

**Parameters:**

- `id=N required`

## `patch.clear`

Remove all live patches

**Example:** `api patch.clear`

**Parameters:** none

## `memory.export`

Export a contiguous debugger-safe byte range under the evidence root

**Example:** `api memory.export cpu=A address=109000 length=512 output=evidence\\stage1-track-109000.bin`

**Parameters:**

- `cpu=A|B required`
- `address=HEX + length=COUNT OR start=HEX + end=HEX`
- `output=relative evidence path optional`
- `name=basename optional legacy alias`
- `max 1 MiB`

## `cpu.disassemble`

Read-only disassembly

**Example:** `api cpu.disassemble cpu=A address=008120 count=64`

**Parameters:**

- `cpu=A|B optional default A`
- `address=HEX required`
- `count=N optional default 16`

## `control.run-frames`

Run exactly N emulated frames and pause; cancellation-aware

**Example:** `api control.run-frames frames=100`

**Parameters:**

- `frames=1..100000 required`

## `control.abort`

Out-of-band pause/cancel of active frame execution

**Example:** `api control.abort`

**Parameters:** none

## `checkpoint.load`

Load a packaged deterministic checkpoint

**Example:** `api checkpoint.load file=stage1-gameplay-2064.chqstate run=false`

**Parameters:**

- `file=packaged .chqstate required`
- `run=true|false optional default false`

## `checkpoint.save`

Save current deterministic state into packaged checkpoints and write metadata

**Example:** `api checkpoint.save file=stage1-gradient-test.chqstate role=research`

**Parameters:**

- `file=packaged filename required`
- `name=display name optional`
- `role=role optional`
- `description=text optional`

## `frame.snapshot`

Capture a reconstructable layered frame snapshot. v0.66.6.1 self-verifies the alpha-over reconstruction and fails capture on any pixel mismatch; `reconstruction.json` exposes `verifiedExact` and `mismatchedPixels`. In v0.66.7, v2 additionally writes `scene-no-sprites.png`, `sprites/sprites.json`, and cropped transparent `sprites/slot_NNN.png` forensic assets for visible sprites, while retaining the aggregate sprite contribution as the authoritative exact-reconstruction layer. The per-sprite assets are intended for Graphics Lab hide/show, solo, opacity and provenance inspection. The snapshot also writes `final.png`, `final-no-hud.png`, `hud-only.png`, exact alpha contribution layers under `layers/`, raw hardware-source renders under `sources/`, a complete `palette.csv`, and `manifest.json`.

**Example:** `api frame.snapshot name=stage1-bad-foreground`

**Parameters:**

- `name=optional snapshot name`

The HUD model is source-aware: Chase H.Q. currently maps HUD isolation to the TC0100SCN text layer. No fixed screen rectangle is masked. `reconstruction.json` records compositor selection, layer order and the files required to rebuild the captured final frame.

**Workbench HTTP v1.9 snapshot routes:** `GET /api/v1/frame/snapshots`, `GET /api/v1/frame/snapshot/{name}/manifest`, and `GET /api/v1/frame/snapshot/{name}/asset/{path}`. The manifest route returns both `manifest.json` and parsed reconstruction metadata; the asset route serves the snapshot PNG/JSON/CSV files used by Graphics Lab.

## `frame.snapshot.list`

List structured frame snapshots. RC2.4.3 adds an optional assertion for same-turn discovery.

**Example:** `api frame.snapshot.list require=stage1-study`

**Parameters:**

- `require=NAME` optional; throw unless exactly one snapshot named `NAME` is present in the returned list.

Omit `require` to retain the normal non-assertive listing behavior.

## `regions.list`

Return authoritative image regions as structured JSON

**Example:** `api regions.list`

**Parameters:** none

## `profiles.list`

List packaged game-profile descriptors; metadata only and does not switch runtime

**Example:** `api profiles.list`

**Parameters:** none

## `profile.get`

Read one packaged game profile and whether it is active/activatable

**Example:** `api profile.get id=sci`

**Parameters:**

- `id=profile id optional default chasehq`

## `sprites.list`

Return current live composed sprite records as structured JSON

**Example:** `api sprites.list`

**Parameters:** none

## `sprites.maps`

Return unique live sprite map IDs

**Example:** `api sprites.maps`

**Parameters:** none

## `sprites.palettes`

Return unique live sprite palette IDs

**Example:** `api sprites.palettes`

**Parameters:** none

## `sprite.inspect`

Inspect one currently active sprite slot

**Example:** `api sprite.inspect slot=44`

**Parameters:**

- `slot=N required`

## `sprite.map.inspect`

Inspect raw spritemap/chunk provenance and raw pen histogram for one active sprite slot

**Example:** `api sprite.map.inspect slot=45`

**Parameters:**

- `slot=N required`

## `schema.all`

Return every Script API action contract from the authoritative in-process schema

**Example:** `api schema.all`

**Parameters:** none

## `script.schema`

Return the authoritative Research Script v2 grammar and limits

**Example:** `api script.schema`

**Parameters:** none

## `ui.prompt`

Show an SDL-native prompt overlay and wait for Enter/Escape while physical gameplay keys remain live

**Example:** `api ui.prompt message=Hold_DOWN_then_press_ENTER pause=true top=true`

**Parameters:**

- `message=TEXT required; underscores render as spaces`
- `pause=true|false optional default true`
- `top=true|false optional default true`
- `timeout=1000..3600000 optional default 300000`

## `sprite.override`

Presentation-only live SDL sprite override; omitted fields retain original values

**Example:** `api sprite.override slot=44 map=182`

**Parameters:**

- `slot=N required`
- `map=N optional`
- `palette=N optional`
- `visible=true|false optional`

## `sprite.override.clear`

Clear all live sprite presentation overrides

**Example:** `api sprite.override.clear`

**Parameters:** none

## `sprite.visual`

Live SDL sprite isolation/highlight mode

**Example:** `api sprite.visual slot=44 mode=flash`

**Parameters:**

- `slot=N required`
- `mode=solo|hide|flash required`

## `sprite.visual.clear`

Clear sprite solo/hide/flash presentation mode

**Example:** `api sprite.visual.clear`

**Parameters:** none

## `palette.entries`

Return the 16 palette entry indices for a bank

**Example:** `api palette.entries bank=75`

**Parameters:**

- `bank=0..255 required`

## `state.snapshot`

Capture coherent structured research state from the current paused instant

**Example:** `api state.snapshot include=sprites,regions,gameplay,track`

**Parameters:**

- `include=sprites,regions,gameplay,track optional`

## `input.ports`

Read current IOC/input port state

**Example:** `api input.ports`

**Parameters:** none

## `events.tail`

Return recent research events

**Example:** `api events.tail count=50`

**Parameters:**

- `count=N optional default 100`

## `sprites.tail`

Return recent semantic sprite-change events

**Example:** `api sprites.tail count=100`

**Parameters:**

- `count=N optional default 100`

## `control.pause`

Pause emulation

**Example:** `api control.pause`

**Parameters:** none

## `control.resume`

Resume emulation

**Example:** `api control.resume`

**Parameters:** none

## `control.step-frame`

Advance paused emulation by N frames

**Example:** `api control.step-frame frames=1`

**Parameters:**

- `frames=N optional default 1`

## `control.step-instruction`

Step one CPU instruction

**Example:** `api control.step-instruction cpu=A`

**Parameters:**

- `cpu=A|B optional default A`

## `timer.hold`

Freeze or resume the game timer helper

**Example:** `api timer.hold enabled=true`

**Parameters:**

- `enabled=true|false optional default true`

## `window.always-on-top`

Toggle SDL always-on-top

**Example:** `api window.always-on-top enabled=true`

**Parameters:**

- `enabled=true|false optional default true`

## `tuning.handling.get`

Read handling override plus instruction-time turn/table/speed/lateral coefficient telemetry when valid

**Example:** `api tuning.handling.get`

**Parameters:** none

## `tuning.handling.set`

Set live handling multipliers

**Example:** `api tuning.handling.set cornering=1.15 speedRetain=1.0`

**Parameters:**

- `cornering=NUMBER optional default 1`
- `speedRetain=NUMBER optional default 1`

## `tuning.handling.reset`

Restore authentic handling values

**Example:** `api tuning.handling.reset`

**Parameters:** none

## `collision.state`

Read evidence-scoped collision-response suppression state and counters; detection remains live

**Example:** `api collision.state`

**Parameters:** none

## `collision.response.get`

Read whether authentic collision response writes are enabled

**Example:** `api collision.response.get`

**Parameters:** none

## `collision.response.set`

Enable/disable only proven collision response writes while preserving detection/event logic

**Example:** `api collision.response.set enabled=false`

**Parameters:**

- `enabled=true|false required`

## `collision.response.reset`

Restore authentic collision response

**Example:** `api collision.response.reset`

**Parameters:** none

## `input.steering.get`

Read raw IOC steering injection (signed 12-bit encoding, centre=0000)

**Example:** `api input.steering.get`

**Parameters:** none

## `input.steering.set`

Set raw IOC steering injection; use 0000 centre, 0060 right, 0FA0 left for proven full steering

**Example:** `api input.steering.set value=0060`

**Parameters:**

- `value=HEX required`

## `palette.bank`

Inspect one 16-pen palette bank

**Example:** `api palette.bank bank=75`

**Parameters:**

- `bank=N required`

## `palette.entry.set`

Freeze one live palette entry

**Example:** `api palette.entry.set index=1209 raw=7FFF`

**Parameters:**

- `index=N required`
- `raw=HEX required`

## `palette.entry.restore`

Return one palette entry to game control

**Example:** `api palette.entry.restore index=1209`

**Parameters:**

- `index=N required`

## `palette.overrides.clear`

Clear all palette freezes

**Example:** `api palette.overrides.clear`

**Parameters:** none

## `input.ioc.xor`

Set held IOC XOR mask

**Example:** `api input.ioc.xor port=3 mask=20`

**Parameters:**

- `port=N required`
- `mask=HEX required`

## `input.ioc.clear`

Clear held IOC XOR masks

**Example:** `api input.ioc.clear`

**Parameters:** none

## `input.ioc.pulse`

Pulse IOC mask for emulated frames

**Example:** `api input.ioc.pulse port=3 mask=20 frames=3`

**Parameters:**

- `port=N required`
- `mask=HEX required`
- `frames=N optional default 3`

## `input.ioc.sweep`

Run deterministic IOC mask sweep and collect evidence

**Example:** `api input.ioc.sweep checkpoint=stage1-gameplay-2064.chqstate port=3 masks=01,02,04`

**Parameters:**

- `checkpoint=FILE optional`
- `port=N optional default 3`
- `masks=comma list optional`
- `pulseFrames=N optional`
- `observeFrames=N optional`

## `checkpoint.list`

List packaged checkpoints

**Example:** `api checkpoint.list`

**Parameters:** none

## `logs.list`

List active-session logs

**Example:** `api logs.list`

**Parameters:** none

## `logs.tail`

Tail a log using the exact identifier returned by logs.list

**Example:** `api logs.tail file=@runtime/web-listener.log lines=80`

**Parameters:**

- `file=RELATIVE required`
- `lines=N optional default 80`

## `evidence.capture`

Capture checkpoint/events/sprites/screenshot evidence bundle

**Example:** `api evidence.capture name=experiment-a`

**Parameters:**

- `name=NAME optional`

## `evidence.list`

List evidence captures

**Example:** `api evidence.list`

**Parameters:** none

## `evidence.view`

Read evidence manifest

**Example:** `api evidence.view name=experiment-a`

**Parameters:**

- `name=NAME optional latest`

## `evidence.load`

Load captured evidence checkpoint paused

**Example:** `api evidence.load name=experiment-a`

**Parameters:**

- `name=NAME optional latest`

## `evidence.zip`

Create ZIP for one evidence capture

**Example:** `api evidence.zip name=latest`

**Parameters:**

- `name=NAME|latest optional`

## `frame.capture`

Capture current SDL frame PNG

**Example:** `api frame.capture name=control`

**Parameters:**

- `name=NAME optional`

## `image.compare`

Compare two captures/snapshots and return pixel-difference metrics. Layered snapshots support source-aware HUD selection.

**Example:** `api image.compare a=control b=test region=PLAYER_CAR hud=hidden`

**Parameters:**

- `a=NAME required`
- `b=NAME required`
- `region=NAME optional default FULL`
- `hud=normal|hidden|only optional default normal`

`hud=hidden` (alias `exclude`) resolves `final-no-hud.png`; `hud=only` resolves `hud-only.png`. These modes require v2 layered snapshots.

## `image.diff`

Compare two images and save a grayscale diff PNG, with the same HUD selection rules as `image.compare`.

**Example:** `api image.diff a=control b=test region=FULL hud=hidden output=diff`

**Parameters:**

- `a=NAME required`
- `b=NAME required`
- `region=NAME optional default FULL`
- `output=NAME optional default image-diff`
- `hud=normal|hidden|only optional default normal`

## `image.regions`

Return raw named image-region document

**Example:** `api image.regions`

**Parameters:** none

## `gameplay.registry`

Return live evidence-backed gameplay registry

**Example:** `api gameplay.registry`

**Parameters:** none

## `track.state`

Return current authoritative track/road state

**Example:** `api track.state`

**Parameters:** none

## `next-steps`

Return packaged high-level research next steps

**Example:** `api next-steps`

**Parameters:** none

## `experiment.begin`

Start a grouped research experiment/evidence manifest

**Example:** `api experiment.begin name=player-car-ab`

**Parameters:**

- `name=NAME optional`

## `experiment.capture`

Capture screenshot plus structured state into the active experiment

**Example:** `api experiment.capture label=control`

**Parameters:**

- `label=NAME optional`
- `include=sprites,regions,gameplay,track optional`

## `experiment.status`

Return the active experiment manifest or inactive state

**Example:** `api experiment.status`

**Parameters:** none

## `experiment.end`

Finalize and close the active experiment manifest

**Example:** `api experiment.end`

**Parameters:** none

## `platform.info`

Return build, script engine, active profile and research-platform identity

**Example:** `api platform.info`

**Parameters:** none

## `features.list`

Return machine-readable feature/regression manifest

**Example:** `api features.list`

**Parameters:** none

## `script.functions`

List supported Script Console query/collection functions

**Example:** `api script.functions`

**Parameters:** none

## `registry.list`

Return gameplay registry items as a flat structured list

**Example:** `api registry.list`

**Parameters:** none

## `track.records`

Read track records around the current authoritative record

**Example:** `api track.records before=2 after=8`

**Parameters:**

- `before=0..64 optional default 4`
- `after=0..64 optional default 12`

## `objects.list`

Return semantic-object registry candidates and evidence confidence

**Example:** `api objects.list`

**Parameters:** none

## `audio.capabilities`

Describe planned/current audio research capabilities without mutating audio runtime

**Example:** `api audio.capabilities`

**Parameters:** none

## `audio.channels`

Return audio channel/source descriptors from the audio research profile

**Example:** `api audio.channels`

**Parameters:** none

## `audio.events`

Return current audio-event foundation status; runtime capture is intentionally not enabled yet

**Example:** `api audio.events`

**Parameters:** none

## `audio.waveform.plan`

Return waveform-viewer/capture implementation contract for future validated audio tooling

**Example:** `api audio.waveform.plan`

**Parameters:** none

## `audio.export.plan`

Return planned sample/music export contract and evidence formats

**Example:** `api audio.export.plan`

**Parameters:** none

## `audio.inject.plan`

Return guarded runtime audio-injection design; does not inject audio

**Example:** `api audio.inject.plan`

**Parameters:** none

## `evidence.names`

Return structured evidence-session directory list

**Example:** `api evidence.names`

**Parameters:** none

## `schema`

Describe scriptable API actions and parameter contracts

**Example:** `api schema action=memory.export`

**Parameters:**

- `action=ACTION optional`


## Shared Track View recorder and SVG export (v0.66.5.1)

The Track View recorder is now server-side shared state. The browser Track View buttons and Research Script API operate on the same sample set, so a survey can be recorded and exported without the Track View tab being open.

```text
api track.record.clear
api track.record.start
api track.record.sample
api track.record.status
api track.record.stop
api track.svg.export name=stage1-track-survey-3600
```

`track.record.sample` appends one deterministic sample of the authoritative `track.state` data and derives the same 2D reconstruction coordinates used by Track View. `track.record.status` returns enabled state, sample/segment counts, first/last frame, spatial bounds and recorder capacity. `track.svg.export` writes a genuine vector SVG under `evidence/track-exports` and refuses to export fewer than two samples.

The recorder retains up to 5000 samples per Workbench process. `track.record.stop` does not clear samples. `track.record.clear` clears them explicitly.

### Memory trace transaction-width clarification (v0.66.5.1)

`memory.trace.start width=...` filters the width of the originating CPU/bus write transaction, not the width of the individual byte being observed. A byte address can therefore change as part of a 16- or 32-bit write. When the producer width is not already known, use `width=any`; address/range overlap is still enforced independently.

```text
api memory.trace.start cpu=A address=0x100303 length=1 width=any limit=256
```

## v0.66.7 — Documentation/Knowledge and SDL window control

New read-only documentation actions: `docs.index`, `docs.search`, `docs.get`, `docs.glossary`, `docs.handover`, `docs.export`, plus `knowledge.list`, `knowledge.get`, `knowledge.search`, `knowledge.related`. These are callable from the HTTP/Workbench bridge and through `.chqscript` using `api ...`.

SDL/runtime control parity is expanded with `window.status`, `window.fullscreen`, `window.scale`, `window.show`, `window.hide`, `window.minimize`, `window.restore`, alongside existing `window.always-on-top`. Scale accepts 1..8 and leaves fullscreen. Window/status output is native authoritative state.

Layered frame snapshots now also emit `scene-no-sprites.png` and `sprites/sprites.json` plus cropped transparent per-visible-sprite assets. These are exploratory forensic controls; exact reconstruction continues to use the aggregate sprite contribution layer recorded in `reconstruction.json`.


## Documentation handbook actions (v0.66.7.5)
- `docs.handbook` — returns the complete authored user-facing handbook tree and page blocks. Example: `api docs.handbook`.
- `docs.page id=PAGE` — returns one handbook page and its parent section. Example: `api docs.page id=graphics-lab-guide`.
- `docs.export` with no `id`/`query` now exports the **full User & Research Handbook**, combining authored handbook content, the generated API action reference, generated scripting-language reference, project knowledge/glossary, and selected evidence.

The Workbench API Reference view is generated directly from the authoritative action schema, so parameters/examples shown to users should match `api schema.all` / `api schema action=NAME`.


## Documentation search/export consolidation (v0.66.7.6)
No action names or parameters change. `docs.search` / `knowledge.search` now include the structured Project Knowledge article body plus API/script/docs references in their searchable corpus. `docs.export` emits the same article sections used by the Workbench and omits empty reference headings.


## Run-history / completed-run snapshot discovery (v0.66.7.7)

`GET /api/v1/script/runs` returns up to 100 completed script runs discovered from authoritative on-disk `run-metadata.json` files. The Workbench merges this server list with browser-local history so a valid completed run is not lost merely because localStorage missed the completion event.

The Script API mirrors this as `api script.runs`.

`GET /api/v1/frame/snapshots`, `frame.snapshot.list`, and the existing frame-snapshot manifest/asset routes now also discover snapshots inside completed run artifact roots. When the same snapshot name exists in more than one location, the newest matching snapshot is resolved.


### v0.66.7.8
`script.runs` keeps the same contract but uses shallow known-layout discovery plus bundle-manifest artifact enumeration and short-lived caching to avoid blocking the Workbench listener. No action names or parameters changed.

## Forensic Timeline v1 (v0.66.8.0)

Forensic Timeline records a bounded gameplay interval once and makes it reusable through the same inspection APIs used for a paused live machine. See `docs/FORENSIC_TIMELINE.md`.

- `timeline.record.start name=NAME` — start per-frame deterministic capture in the current run artifacts; current frame is captured immediately.
- `timeline.record.stop` — finalize the active timeline.
- `timeline.status` — native recording/import/source status.
- `timeline.list` — discover timelines from active/completed runs and packaged research data.
- `timeline.load name=NAME|path=PATH` — import and load the first frame into paused inspection mode.
- `timeline.seek frame=N` — load an exact recorded frame.
- `timeline.next` / `timeline.prev` / `timeline.frames` — navigate/index the recording.
- `timeline.play name=NAME from=N to=N fps=1..60` — paced recorded-state playback through the SDL renderer; this loads historical frame checkpoints in sequence and does not re-execute CPUs.
- `timeline.trim.event ...` — find a masked rising/falling event window with configurable pre/post frames and persist it as a non-destructive analysis view; the master recording is left intact.
- `timeline.scan.memory ...` — offline whole-write-stream candidate/writer ranking; exports portable CSV/JSON/HTML into the current script run artifacts while leaving the source timeline immutable.
- `timeline.analyze.auto ...` — full unattended pipeline: portable memory ranking, known-reference tagging, writer-PC disassembly, historical milestone state/layered snapshots and HUD_TURBO graphics correlation.
- `timeline.fork-live` — keep the selected historical checkpoint loaded and return to LIVE paused execution.
- `timeline.unload` — restore the live state that existed before import.

While `source=timeline`, ordinary read-side actions such as `memory.read`, `register.read`, `gameplay.registry`, `sprites.list`, palette inspection and `state.snapshot` operate on the selected recorded checkpoint. Timeline v1 captures complete sprite/object RAM. Authentic audio capture remains unavailable because the native runtime currently has only `sound_stub`; this limitation is explicit in the timeline manifest and audio capability descriptor.

## v0.66.9.0-RC2 additions
- `sprite.map.inspect slot=N|map=N`
- `sprite.order.inspect`
- `sprite.order.set mode=higher-slot|lower-slot`
- `game.sprite.order.inspect`
- `game.sprite.order.set mode=higher-slot|lower-slot`

### Speed semantics
`game.speed.set` and `game.speed.freeze` target CPU-A `0x10041C`, the raw internal/physics speed. The visible HUD speed is the separate packed-BCD field at `0x100400`; the numeric values are not expected to match. Game Lab mutations preserve the prior RUNNING/PAUSED state, while generic `memory.write` intentionally pauses for debugger safety.

> RC2.2 note: Chase H.Q. native same-priority sprite ordering defaults to `lower-slot`, matching MAME reverse sprite-RAM traversal; `higher-slot` remains an explicit diagnostic override.
## v0.66.9.0-RC2.5 sprite ownership assertions

`api frame.snapshot.assert-overlap name=normal front=82 back=81 frontSolo=body backSolo=shadow`

All five parameters are required. Names resolve existing snapshots. The action throws unless frames agree, both exported sprites exactly match their separate solo runtime source renders at the exported coordinates, overlapping source pixels have coverage >=2 and are never owned by the back sprite, front-owned overlap is nonzero, and the back sprite remains visible outside the front. It returns frame/map IDs and overlap/visibility counts. It performs no emulation steps or presentation changes. Offline CLI equivalent: dot-source `scripts/Assert-SpriteSnapshotOverlap.ps1`, then call `Assert-SpriteSnapshotOverlap -Snapshot DIR -Front 82 -Back 81 -FrontSolo DIR -BackSolo DIR`.

Snapshots add `sprite-ownership.csv` (x,y,owner,coverage; 65535=no sprite-stage owner). Coverage counts blocked/occluded nonzero pens; owner records accepted color. Individual sprite asset coordinates now use the same visible Y=16 origin as live rendering.

Legacy `sprite.order.set` mode names remain accepted: `lower-slot` selects descending traversal, `higher-slot` ascending. First nontransparent candidates retain occupancy in both modes, including layer-blocked candidates. `sprite.order.inspect` now reports `ownership=first-nontransparent occupancy=includes-layer-blocked`. RC2.2's inference that descending traversal means lower-slot wins was incorrect. See [SPRITE_OWNERSHIP.md](SPRITE_OWNERSHIP.md).

## v0.66.9.0-RC2.5 lifecycle additions

### `timeline.reset`

`api timeline.reset`

Idempotent cleanup for script-initiated timeline work. The first `timeline.load` in a transaction captures the pre-load live state. `timeline.reset` unloads a read-only timeline or restores that captured live checkpoint after `timeline.fork-live`, then clears the transaction. If no transaction/timeline is active it succeeds without changing state. Use it before and in `finally` for mutation experiments.

### `script.runs` current-session metadata

`api script.runs` continues to support bounded `limit`, `offset`, `status`, `version`, and `session` filters. The response now includes `currentSession`, which the Workbench uses to default Script Console history to the active session; users can still select all sessions or filter by build/version.

### API exposure parity

Workbench startup now validates both directions between the Script Console allow-list and the action schema. A schema action missing from the executable allow-list (or vice versa) is a startup error rather than a silently unreachable documented command.
