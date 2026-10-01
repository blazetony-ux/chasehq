# ChaseHQ-Native v0.66.9.0-RC1

Development candidate based on the proven v0.66.8.0 release.

## Game Lab
- Grouped/padded Experimental / Game Lab presentation with live turbo and speed readouts.
- Reversible API/UI controls for infinite turbo stock (`0x1003A2`), infinite active duration (`0x100414`), turbo fire/timer/count, and current speed set/freeze/clear (`0x10041C`).
- Speed control is explicitly current-speed manipulation, not yet claimed as a proven top-speed limiter.

## TC0100SCN character inspector
- New `tile.inspect` action for TC0100SCN RAM text characters (256 x 8x8, 2bpp).
- Exports raw pen matrix, palette-aware PNG, JSON metadata and source address.
- Graphics/Workbench inspector includes presets for turbo normal `0x7C..0x7F` and active `0x0C..0x0F` characters.
- Background ROM tile inspection is not yet implemented and is documented as future scope.

## Turbo/HUD research promoted into the build
- Bundled reusable scripts for layer/sprite correlation, exact tile writer, text-plane neighbourhood, gameplay-to-HUD writer path and tile-character inspection.
- Current evidence proves the active turbo HUD uses hard-coded `0x0C..0x0F`, normal icons use `0x7C..0x7F`, and expiry clears the consumed icon.

## Timeline documentation
- Documents the exact frame-boundary convention: bus writes tagged N occur during execution of N and are reflected in checkpoint N+1.

## Validation
Run `Regression - v0.66.9.0 Game Lab and Tile Inspector` first. Run Full Regression only after that passes. v0.66.8.0 remains the proven baseline until Windows/SDL proof completes.
