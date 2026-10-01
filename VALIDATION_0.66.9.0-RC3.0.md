# Validation — v0.66.9.0-RC3.0

## Source/package gates

- Version/schema/feature/knowledge build identity must be `0.66.9.0-RC3.0`.
- All packaged JSON must parse.
- Linux no-SDL native build and available CTest suite must pass.
- New RC3.0 actions must be present in Workbench allow-list, machine-readable schema and API docs.
- Full Regression must include `regression-v06690-rc30-course-mapping-live-survey.chqscript`.

## Windows/SDL promotion gate

1. `./Build-Debug.bat` must succeed under the existing narrow CPU-bus SEGFAULT exception (sole exact known failure only).
2. Run `regression/regression-v06690-rc30-course-mapping-live-survey.chqscript`. It must complete, restore frame 2064 and produce a mapping dataset.
3. Verify Track View can configure/start/stop follower without emulator restart and shows road/player-line overlays.
4. Run Full Regression and require `=== ChaseHQ Full Regression: COMPLETE ===`.
5. Only then promote RC3.0 as Windows/SDL-proven.

## First research run after promotion

Run `track/surveys/stage1-course-mapping-shakedown-600.chqscript`. Inspect `track-map.json`, all CSVs, SVG, screenshots and topology trace before starting the 3600-frame survey.
