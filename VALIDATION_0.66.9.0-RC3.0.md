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

## Candidate correction gate — Track View visibility

A focused RC3.0 Windows regression on 2026-10-01 passed and produced a valid 3-sample `chq-course-map-v1` mapping dataset, proving the backend mapping path. The same run exposed a browser-only tab-routing mismatch: the Track View tab still referenced the old `Live SVG Track View` panel title while the panel is now `Course Mapping / Live Survey`.

The corrected candidate must additionally prove:

- selecting **Track View** visibly renders the **Course Mapping / Live Survey** panel;
- the panel can retrieve the already-recorded samples from `/api/v1/track/record` after a focused regression;
- road/player overlays and telemetry render rather than an empty tab;
- `Validate-Workbench.ps1` rejects reintroduction of the stale `track:['Live SVG Track View']` routing entry.

## Replacement-candidate gate — combined bridge and UI corrections

The candidate with SHA-256 `6b209d84945a64f6d33435926a0fdb4a8fbcab7f86e60b4b04b878f8a00f56b1` is withdrawn. Its Track View route was corrected, but a packaging merge reintroduced `Set-CourseFollowConfig($args)`, so the focused regression failed at `course.follow.configure` with the PowerShell `System.Object[]`/`ContainsKey` error and recorded zero track samples.

The replacement must prove both corrections in the same archive:

- `Set-CourseFollowConfig($config)` and `$config.ContainsKey($key)` are present; the `$args` form is absent;
- Track View routes to `Course Mapping / Live Survey`; the old `Live SVG Track View` route is absent;
- focused RC3.0 regression completes and records/exports mapping samples;
- Track View visibly renders those samples;
- Full Regression completes before promotion.

