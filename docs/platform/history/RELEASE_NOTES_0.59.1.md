# Release Notes — v0.59.1

## Predictive mapping driver

- Added `predictive` and retained `legacy` course-follow controllers.
- Predictive controller uses weighted current-bank look-ahead, speed-scaled P correction, D damping and steering slew limiting.
- Added curve-aware target-speed control with accelerator lift and braking.
- Added structured telemetry for predicted curvature, P/D/error-rate, target speed and commanded controls.

## Collision-free mapping

Expanded `--no-collisions` from the single `$00A156` target shove to the complete currently proven response family: `$00A142`, `$00A156`, `$00A1BE`, `$00A1C4` lateral writes and `$00A200` internal-speed penalty. Detection/event/object/scoring state remains live. Added separate lateral/speed suppression counters and events.

## Runtime progress

Added `--progress-status N` for bounded-run percentage, effective FPS and ETA. Default is 300 frames; set `0` to disable. Fast-forward continues to use `--fast-forward-status N`.

## Compatibility

- Existing CHQSTATE v1 checkpoints remain unchanged and compatible.
- Existing Windows output layout is intentionally preserved.
- Existing course-follow tuning switches remain accepted.

## Deferred

Embedded checkpoint screenshots/metadata are documented as a planned CHQSTATE v2 feature, not silently introduced in this maintenance build. The current checkpoint format remains v1 to avoid risking deterministic restore compatibility while mapping/autopilot behaviour is being validated.
