# Active candidate: v0.66.9.0-RC3.0

## Baseline

v0.66.9.0-RC2.9 is the authoritative Windows/SDL-proven working baseline. Its focused target-health/tooling regression and Full Regression passed; Dashboard and Experimental controls were manually verified. The physical Pause/Break key itself was not exercised because the test keyboard has no Pause key, but the shared pause/resume path is proven through Workbench controls.

## RC3.0 candidate scope

RC3.0 is the Course Mapping / Live Survey tooling release. It deliberately precedes lengthy Stage-1 surveys so that retained evidence is useful rather than immediately obsolete.

- Native course follower can be configured/started/stopped/reset live through debugger, Research API, Script Console and Workbench.
- Signed lateral target bias supports controlled structural branch coverage while preserving closed-loop road following.
- Track recorder v2 separates reconstructed road geometry from the player's driven trajectory and retains speed, steering, road state and follower telemetry.
- Mapping datasets export JSON, sample/trajectory/entity/surface CSVs and SVG.
- The confirmed Stage-1 target record is tracked semantically; other visible sprites remain unclassified candidate traffic/obstacle evidence.
- Raw course-record surface signatures are retained without prematurely naming asphalt/dirt materials.
- Bundled 600-frame shakedown, 3600-frame centre survey, positive/negative-bias branch surveys and a reusable current-state 1200-frame survey provide the next investigation workflow.
- Focused RC3.0 regression is permanently included in Full Regression.

## Validation state

Linux no-SDL compile/tests and package checks may be performed when producing the candidate, but **RC3.0 must not be called Windows/SDL-proven until the packaged `Build-Debug.bat`, focused RC3.0 regression and Full Regression pass on the user's Windows/SDL environment.**

## Immediate next task

After Windows prove-off, run `track/surveys/stage1-course-mapping-shakedown-600.chqscript`. Inspect its mapping dataset before committing to the 3600-frame and branch surveys. The goal is an authoritative Stage-1 directed course graph/minimap built from already-proven topology plus newly retained geometry/trajectory/entity/surface evidence.

- SDL pause convenience: Ctrl+P now mirrors Pause/Break for keyboards without a dedicated Pause key.
