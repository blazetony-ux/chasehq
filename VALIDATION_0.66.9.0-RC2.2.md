# Validation — v0.66.9.0-RC2.2

Candidate validation target for the MAME-reference Chase H.Q. sprite traversal correction.

## Static/local checks

- Entire `.chqscript` metadata set: required metadata checked.
- Regression metadata: `regression-version` and `regression-scope` checked.
- JSON sources/schema: parse validation required.
- C++ build/runtime test gate: required before packaging.
- ZIP/package exclusion rules: ROM payload, SDL source, build output and transient evidence excluded.

## Windows focused prove-off

1. `Build-Debug.bat` must pass.
2. Start via `Start-ChaseHQ.ps1 -Restart`.
3. Run `Regression - v0.66.9.0 RC2.2 MAME Sprite Order Default`.
4. Require initial `sprite.order.inspect` to report `tie_break=lower-slot` / descending traversal.
5. Verify higher-slot override works, then lower-slot restore works.
6. Run `Same Priority Sprite Order Multi-Scene Validation` when the captured car-shadow timeline is present.
7. Only after focused proof passes, run Full Regression.
