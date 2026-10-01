# ChaseHQ-Native v0.66.9.0-RC3.0 release candidate

RC3.0 is a meaningful Course Mapping / Live Survey tooling release built from the Windows/SDL-proven RC2.9 baseline. It intentionally precedes long course surveys.

## Added

- Live native course-follower runtime command and Workbench/API/Script Console parity: status/configure/start/stop/reset.
- Signed `course-follow-lateral-bias` startup/config/live option for controlled branch surveys.
- Reversible `course.survey.start/stop` orchestration with timer hold, proven collision-response suppression and state restoration.
- Track Recorder v2 with road edges/centre, authoritative lateral state, separate projected player driven line, speed/steering/follower telemetry, confirmed Stage-1 target state and unclassified visible-sprite census.
- Raw surface-signature evidence from course-record selectors plus representative TC0150ROD renderer control/body/gfx state, intentionally not named asphalt/dirt until proven.
- Reconstructable mapping export: `track-map.json`, `track-samples.csv`, `car-trajectory.csv`, `track-entities.csv`, `track-surfaces.csv`, `track-map.svg`.
- Updated Track View with live follower controls, survey start/stop, road/player overlays and map export.
- New survey scripts: 600-frame shakedown, 3600-frame centre survey, negative/positive-bias branch surveys, plus a reusable 1200-frame current-state survey for later stages/checkpoints.
- Permanent RC3.0 focused regression wired into Full Regression.
- Long script sampling returns compact sample results while retaining full shared recorder state, avoiding quadratic Script Console evidence growth.

## Semantic discipline

Only the already-confirmed Stage-1 special-target object is exported as a semantic car. Other live sprites remain `UNCLASSIFIED_DYNAMIC_OBJECT`. Surface signatures are `UNCLASSIFIED` evidence candidates until screenshots/behaviour prove material semantics. Structural branch A/B remains unlabeled as left/right.

## Promotion

This package is a source candidate until Windows `Build-Debug.bat`, the focused RC3.0 regression and Full Regression pass.

- SDL pause convenience: Ctrl+P now mirrors Pause/Break for keyboards without a dedicated Pause key.

## Candidate correction — Track View tab routing

The first Windows-proven mapping regression exposed a Workbench presentation defect: the Track View button still routed to the historical panel title `Live SVG Track View` after the RC3.0 panel was renamed `Course Mapping / Live Survey`. The mapping backend, recorder v2 and export path were working, but the browser hid the renamed panel and therefore displayed an empty Track View.

The corrected RC3.0 candidate aligns the tab group with `Course Mapping / Live Survey` and adds a `Validate-Workbench.ps1` guard so a future panel-title/tab-routing mismatch fails validation before packaging.
