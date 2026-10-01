
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

## v0.66.9.0-RC1 action additions

The scripting language grammar is unchanged. New functionality is exposed through ordinary `api ACTION key=value` calls, notably the `game.*` and `tile.inspect` actions documented in the live API schema. Reusable turbo/HUD investigation scripts are now bundled under `research/scripts/graphics/` and `research/scripts/gameplay/`.

# Research Script v2 Language Reference — v0.66.5.1
> **Current proven release:** v0.66.8.0 (2026-09-29). Interfaces documented here correspond to the promoted RC7.4 runtime unless a section is explicitly historical.


Authoritative runtime discovery: `api script.schema` or `GET /api/v1/schema/script`. Static schema snapshot: `research/schema/script-language.json`.

## Statements

### `# comment`

Comment

### `set NAME = VALUE`

Set a variable

### `${NAME}`

Variable substitution

### `for ITEM in LIST ... end`

Iterate a list/query result

### `repeat N ... end`

Repeat a block; exposes ${repeat_index}

### `if status KEY=VALUE ... [else] ... end`

Conditional status check

### `if memory cpu=A address=HEX width=8|16|32 value=HEX ... end`

Conditional memory check

### `if capability NAME ... end`

Conditional capability check

### `wait MS`

Sleep 0..5000 ms

### `wait-until step-complete [TIMEOUT_MS]`

Wait for native frame-step completion

### `require capability NAME`

Require native capability

### `assert status KEY=VALUE`

Assert a status field

### `echo TEXT`

Write run output

### `api ACTION key=value ...`

Invoke a documented Script API action

### `include "path.chqscript"`

Include a saved library script

### `zip evidence latest|NAME`

ZIP one evidence capture

### `zip evidence-match PREFIX [BUNDLE_NAME]`

ZIP captures sharing a prefix

## Query/collection functions

- `getregions()`
- `getprofiles()`
- `getsprites()`
- `getpaletteentries(bank=N)`
- `getcheckpoints()`
- `getcapabilities()`
- `getmaps()`
- `getpalettes()`
- `getregistry()`
- `gettrackrecords()`
- `getevidence()`
- `getobjects()`
- `getaudiochannels()`
- `count()`
- `first()`
- `last()`
- `contains()`
- `unique()`

## Limits

- `sourceBytes`: 65536
- `sourceLines`: 1000
- `expandedCommands`: 1000

v0.66.9.0-RC2.4.4 raises the expanded-command ceiling from 500 to 1000 because the permanent Full Regression suite now expands to 503 commands. Loop-iteration and nesting limits are unchanged.
- `loopIterations`: 256
- `nestingDepth`: 4
- `waitUntilMs`: 30000

## SDL interactive prompt

The prompt is an API action used from scripts:

```text
api ui.prompt message=Hold_DOWN_until_the_brake_state_is_visible_then_press_ENTER pause=false top=true timeout=300000
api control.pause
```

`message` underscores render as spaces. Enter accepts; Escape cancels. Physical gameplay controls continue through the authentic IOC input path while the overlay is shown. Use `pause=false` when the condition must develop while the game is running.


## Palette-write tracing from Research Script (v0.66.4)

The Script language itself is unchanged; palette tracing is exposed through documented `api ACTION` calls. Use runtime discovery (`api schema action=palette.trace.start` or `api schema.all`) rather than guessing action names.

```text
api palette.trace.clear
api palette.trace.start indices=1037,1053 limit=256
api checkpoint.load file=stage1-gameplay-2064.chqstate run=false
api control.run-frames frames=5
api input.ioc.xor port=2 mask=20
api control.run-frames frames=8
api palette.trace.tail count=50
api palette.trace.stop
```

Palette trace events contain the exact writer PC and complete palette old/new values, which makes them suitable for follow-up disassembly/provenance work.

## Generic memory-write tracing from Research Script (v0.66.5.1)

The Script grammar is unchanged. The following first-class API actions are available through `api ACTION`: `memory.trace.start`, `memory.trace.status`, `memory.trace.tail`, `memory.trace.stop`, and `memory.trace.clear`. Example:

```text
api checkpoint.load file=stage1-gameplay-2064.chqstate run=false
api memory.trace.clear
api memory.trace.start cpu=A address=0x100303 length=1 width=any limit=256
api control.run-frames frames=5
api input.ioc.xor port=2 mask=0x20
api control.run-frames frames=6
api memory.trace.tail count=50
api memory.trace.stop
```

Use `api schema action=memory.trace.start` or `api schema.all` for the authoritative runtime contract.

## Scriptable shared Track View recorder (v0.66.5.1)

Track recording and SVG export are first-class Script API actions. They use the same server-side recorder as the Track View UI.

```text
api checkpoint.load file=stage1-driving-2352.chqstate run=false
api track.record.clear
api track.record.start
api track.record.sample
repeat 120
    api control.run-frames frames=30
    api track.record.sample
end
api track.record.stop
api track.record.status
api track.svg.export name=stage1-track-survey-3600
```

This makes long track surveys reproducible without browser polling. The packaged example is `research/scripts/track/surveys/stage1-track-survey-3600-scripted-svg.chqscript`.

For memory tracing, remember that `width` describes the originating write transaction. Use `width=any` if a target byte may be modified by a wider write.


## Reconstructable layered frame capture and HUD-aware comparison (v0.66.6.1)

The grammar remains unchanged; these are API actions available from Research Script.

```text
api checkpoint.load file=stage1-driving-2352.chqstate run=false
api frame.snapshot name=layered-baseline
api control.run-frames frames=30
api frame.snapshot name=layered-test
api image.compare a=layered-baseline b=layered-test region=FULL hud=hidden
api image.diff a=layered-baseline b=layered-test region=FULL hud=only output=hud-only-diff
```

`frame.snapshot` v2 self-verifies reconstruction in v0.66.6.1 and fails if any reconstructed pixel differs from `final.png`. It writes exact transparent contribution layers under `layers/`, raw source renders under `sources/`, and `reconstruction.json`. `hud=hidden` suppresses the Chase H.Q. TC0100SCN text/HUD source; it is not a fixed rectangular crop. `hud=only` compares only the pixels whose final output is influenced by that HUD source.

## v0.66.7 API-facing additions
No new language syntax is introduced. Use the existing `api ...` command for documentation/knowledge and SDL controls, for example:

```text
api docs.search query=turbo
api docs.handover
api docs.export format=html evidence=key output=project-handbook
api window.status
api window.fullscreen enabled=false
api window.scale scale=3
api window.always-on-top enabled=true
```

This preserves the small deterministic language while maintaining Workbench/API/script parity.


## Built-in scripting reference (v0.66.7.5)
The Workbench Docs / Knowledge tab now contains a complete Scripting Reference generated from the authoritative script-language schema. It lists every supported statement, query function and parser limit, with copyable examples. No new core language syntax is introduced in v0.66.7.5; the new handbook is exposed through the existing generic API statement:

```text
api docs.handbook
api docs.page id=api-overview
api schema.all
api script.schema
```


## v0.66.7.6 documentation consolidation
No `.chqscript` grammar changes. Existing `api docs.search`, `api knowledge.search`, `api docs.get` and `api docs.export` actions expose the richer article search/export behaviour through the unchanged generic `api ACTION key=value` statement.


## v0.66.7.7 persistence/discovery addition

No grammar change. The generic `api` statement gains the documented action `script.runs`, which lists authoritative completed script runs from on-disk run metadata.

## Native API error propagation
As of v0.66.9.0-RC2, an `api ...` command whose native result begins with `ERR` is treated as a script failure. With **Stop on error** enabled, execution stops and the run is finalized FAIL. This closes the older gap where a command could print `ERR` inside an otherwise green PASS.

> RC2.2 note: Chase H.Q. native same-priority sprite ordering defaults to `lower-slot`, matching MAME reverse sprite-RAM traversal; `higher-slot` remains an explicit diagnostic override.
## v0.66.9.0-RC2.5 sprite correctness

Grammar is unchanged. `frame.snapshot.assert-overlap name=NAME front=SLOT back=SLOT frontSolo=NAME backSolo=NAME` is an assertive API action; failure stops a normal stop-on-error script. Full Regression includes `regression-sprite-ownership.chqscript`. Legacy order mode names select traversal, not winning slot: descending `lower-slot` now reserves each nontransparent pixel for its first candidate. See the API reference and `docs/SPRITE_OWNERSHIP.md`.

## v0.66.9.0-RC2.5 safe experiment lifecycle

Research Script v2 adds `try ... finally ... end` in the Workbench compiler. The `finally` block runs after the body on success and on API/assert/runtime failure; on user cancellation it is attempted where practical. If both body and cleanup fail, the body error remains the primary failure and cleanup errors are reported separately.

Use `api timeline.reset` as the idempotent timeline cleanup primitive. A script-initiated `timeline.load` captures the pre-load live state; after `timeline.fork-live`, `timeline.reset` restores that state. Calling it with no active transaction succeeds without changing the machine.

Mutation scripts should include `name`, `purpose`, `category`, `target`, `method`, `expected`, and `cleanup` header fields, establish known preconditions, and place temporary timeline/sprite/patch/presentation cleanup in `finally`.

Example:

```text
# name: Safe timeline experiment
# purpose: Demonstrate cleanup after a counterfactual fork.
# category: example
# target: recorded frame 2352
# method: load, seek and fork inside try
# expected: experiment runs at frame 2352
# cleanup: timeline.reset restores the pre-load live state

api timeline.reset
try
    api timeline.load name=car-shadow-layer-investigation
    api timeline.seek frame=2352
    api timeline.fork-live
    # experiment commands
finally
    api timeline.reset
end
```
