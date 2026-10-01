# ChaseHQ-Native v0.66.9.0-RC2

## Purpose
Focused graphics/gameplay research candidate built from the proven v0.66.9.0-RC1 base.

## Changes
- Fixes `sprite.map.inspect map=N`: map-based inspection now resolves the currently active matching sprite slot(s) instead of silently treating the requested map as slot 0.
- Adds runtime same-priority sprite ordering inspection and reversible A/B control:
  - `sprite.order.inspect`
  - `sprite.order.set mode=higher-slot|lower-slot`
  - Game Lab aliases and controls.
- Adds a Game Lab Sprite Ordering card for the player-car/under-car-effect investigation.
- Corrects speed terminology in Game Lab:
  - `0x100400` = packed-BCD HUD/display speed.
  - `0x10041C` = raw internal/physics speed.
  These values are intentionally separate and are not expected to numerically match.
- `Set once`, speed freeze, turbo stock and turbo active Game Lab mutations now preserve the prior RUNNING/PAUSED state. Low-level `memory.write` remains debugger-safe and pauses by design.
- TC0100SCN `tile.inspect` output filenames now include the current frame so repeated historical inspections do not overwrite earlier animation phases.
- Unhandled native `ERR ...` results now fail `.chqscript` execution when Stop on error is enabled.
- Adds reusable turbo HUD animation and player-car sprite-order investigation scripts.
- Adds focused regression `Regression - v0.66.9.0 RC2 Sprite Order and Speed Semantics`.

## Current research state
Turbo HUD animation is confirmed game behaviour: normal 7C-7F characters are replaced by animated RAM characters 0C-0F, sourced from a three-frame table at 0x33A1A/0x33A5A/0x33A9A and cleared on expiry.

The current player-car issue is now framed as a same-priority sprite-order investigation. At the canonical driving frame, player car slot 82/map 475 and under-car effect slots 83/84 share priority 0. RC2 makes the renderer tie-break directly testable against the already captured timeline.

## Validation target
Run `Regression - v0.66.9.0 RC2 Sprite Order and Speed Semantics` before Full Regression.
