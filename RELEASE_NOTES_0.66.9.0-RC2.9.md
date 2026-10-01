# ChaseHQ-Native v0.66.9.0-RC2.9 release candidate

RC2.9 consolidates the RC2.8 saturation research into code, canonical checkpoints and documentation.

## Added / changed

- Confirmed target remaining-hit counter `0x1002AE` and authentic `A112/A124/A118` damage/terminal paths promoted everywhere.
- Four target research checkpoints bundled: contact pair 5587/5588 and preferred damage pair 6333/6334.
- Experimental **One-hit Target Kill**, default OFF, preserving the authentic terminal underflow path.
- Native/script/UI parity: startup `--one-hit-target`, debugger `target one-hit ...`, script `game.target.one-hit`, Experimental-tab toggle.
- SDL Pause/Break toggles pause/resume globally; paused state appears in the SDL title.
- Dashboard live frame and SDL controls share a wide-screen row and collapse responsively.
- `control.run-frames` timeout is configurable, default 900000 ms; the front-proxy/browser long-operation budgets are 3700 s / 3700000 ms so the documented 3600000 ms maximum is not truncated by transport timeouts.
- Gameplay Registry, Next Steps, semantic objects, handbook, API/script docs and handover updated.

## Validation boundary

The source is based on Windows/SDL-proven RC2.8, but RC2.9 requires its own Windows build, focused regression and Full Regression before promotion.

## Replacement archive note

An earlier pre-proof RC2.9 archive was withdrawn after Windows `Prepare-Project` correctly exposed a stale validator expectation for the old 300000 ms wait. The corrected archive aligns the validator with the RC2.9 progress-aware contract and retains the widened 3700-second/3700000-ms transport budgets. Do not use the earlier RC2.9 ZIP/hash.

## Windows build-gate correction

A subsequent Windows build exposed two additional stale release-gate defects before RC2.9 promotion: `tests/cli_evidence_tests.ps1` still expected the historical RC2.7 evidence build identity, and the Debug/Release batch wrappers captured `ERRORLEVEL` with `%ERRORLEVEL%` inside a parenthesized block instead of delayed `!ERRORLEVEL!`. The latter could incorrectly classify multiple failing CTests as the one tolerated historical CPU segfault. RC2.9 now asserts evidence identity `0.66.9.0-RC2.9`, uses delayed error-level capture, and permits the historical CPU segfault only when CTest reports exactly one failed test.
