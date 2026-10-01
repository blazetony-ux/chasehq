# v0.51.2 BG alignment and layer-offset workbench

## Scope

v0.51.2 is a BG-vertical-alignment and diagnostic layer-offset patch built directly on v0.51.1. It does not replace the PROM compositor, alter checkpoint format, or claim final geometry parity.

## Evidence motivating the patch

v0.51.1 corrected the hardware visible-area origin to Y=16..255. Windows/SDL validation immediately made the three previously missing turbo indicators visible at the lower-left HUD and improved the top/sky presentation, confirming that the global crop/origin error was real. Follow-up layer captures showed text, sprites, and road substantially coherent while BG output retained an approximately 24-pixel empty/black strip before useful sky content.

v0.51.2 therefore leaves text/sprite/road geometry unchanged and applies a default presentation correction of BG0/BG1 Y=-24. Because this value still needs live A/B validation, the same mechanism is exposed as a parameter rather than hard-wiring further speculative releases.

## New diagnostic control

`--layer-offset LAYER:X:Y` is repeatable and accepts `bg0`, `bg1`, `text`, `sprites`, `road`, or `all`. Negative Y moves the selected layer upward. Master debug configs accept the same syntax as `layer_offset=...`. Active values are emitted into run metadata.

Defaults in this build:

```text
bg0     0,-24
bg1     0,-24
text    0,0
sprites 0,0
road    0,0
```

The offsets are presentation-only and do not mutate emulated hardware state.

## Validation target

Use `stage1-gameplay-2064.chqstate` first. Success means the BG sky/cloud/scenery relationship improves while the v0.51.1 gains remain intact: turbo icons visible, top HUD stable, road/player-car alignment unchanged, and no new clipping. Use `--layer-offset bg0:0:N --layer-offset bg1:0:N` to sweep candidate Y positions without rebuilding.

## Project-state updates carried in documentation

This release records the authoritative gameplay-state findings for score, speed, distance, turbo active/count, brake lamps and IOC control mappings, plus the current debugger limitations discovered during the v0.51.0/v0.51.1 burn-in. Road-contact state remains unresolved pending deterministic steering injection.

## Build validation

CPU-only configure/build/tests pass in the development container. Windows/SDL geometry validation remains required.
