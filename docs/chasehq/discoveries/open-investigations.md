# Immediate Chase H.Q. investigations

This is intentionally short-term and game-specific.

1. Validate v0.63.0 workflow controls on Windows/SDL, especially timer freeze, always-on-top, checkpoint loading, frame preview and IOC sweep.
2. Resolve colour accuracy: palette contents vs bank selection vs RGB/bit decoding vs final output conversion.
3. Extend v0.66.2 browser/native controller mapping beyond steering to accelerator, brake, turbo, start and credit.
4. Tune/test live cornering override while preserving authentic/default state separately.
5. Confirm remaining authoritative gameplay telemetry: accel/brake state, target damage/health/defeated/caught and end-level transitions.
6. Diagnose missing turbo HUD indicators through state -> graphics -> palette -> priority/compositor path.
7. Use near-destruction/end-level checkpoints to document target defeat and Stage-1 completion transitions.
8. Convert confirmed discoveries into deterministic regression tests.
9. Add Sprite Lab manipulation/substitution after the v0.63.0 live workflow is validated.
