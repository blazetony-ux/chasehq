# Course Mapping / Live Survey Guide — v0.66.9.0-RC3.0

RC3.0 turns the pre-existing native course follower into a first-class live research tool so long course surveys can be run entirely from Workbench/Script Console without restarting the emulator.

## Evidence model

The mapper deliberately separates **what is known** from **what merely looks plausible**.

- Road geometry: left edge, centre, right edge, page/selector, record, curvature, course position and authoritative road-state/lateral values.
- Player driven line: authoritative lateral position/error plus a separate projected map trajectory. The SVG trajectory is a visual projection; the raw lateral fields remain the authority.
- Confirmed Stage-1 target: object record `0x10A080-0x10A0BF` is emitted as `CONFIRMED_TARGET_CAR` with longitudinal/lateral/speed/health context.
- Other visible sprites: emitted as `UNCLASSIFIED_DYNAMIC_OBJECT` with slot/map/palette/screen bounds/priority/visible-pixel evidence. Do not call them AI cars or obstacles until their semantics are proven.
- Surface candidates: composite raw signatures retain course-record bytes/selectors plus representative TC0150ROD control/body/gfx scanline state. Material is intentionally `UNCLASSIFIED` until screenshots/behaviour prove asphalt, dirt, or another surface.

## Mapping export

`api track.map.export name=NAME` writes a reconstructable directory under `evidence/track-maps/NAME/`:

- `track-map.json` — master dataset including complete sampled state.
- `track-samples.csv` — road/course/player/follower/surface telemetry.
- `car-trajectory.csv` — player line, speed, steering, road state and follower outputs.
- `track-entities.csv` — confirmed target row plus unclassified visible-sprite census.
- `track-surfaces.csv` — grouped raw surface signatures and representative road-render state.
- `track-map.svg` — road edges/centreline plus player trajectory visualisation.
- `README.txt` — dataset semantics.

## Recommended scripts

1. `track/surveys/stage1-course-mapping-shakedown-600.chqscript` — first prove-off after a new build.
2. `track/surveys/stage1-course-mapping-3600.chqscript` — main centre-follow Stage-1 survey.
3. `track/surveys/stage1-course-mapping-negative-bias.chqscript` and `...positive-bias...` — controlled structural branch coverage. These are **not** semantic left/right labels.
4. `track/surveys/current-state-course-mapping-1200.chqscript` — reusable later-stage/current-checkpoint survey. It intentionally does not load a Stage-1 checkpoint.

## Workbench workflow

From Track View, configure the controller, apply a signed bias if required, and use **Start clean survey**. This enables the native follower, holds the timer, suppresses only the already-proven collision response while preserving detection, clears/starts the shared recorder, and keeps the emulator in the same process. **Stop / restore** restores the prior timer/collision state and pre-existing follower-enabled state.

For deterministic evidence, prefer Script Console scripts. `track.record.sample` returns compact output during scripts while retaining the full recorder dataset server-side, avoiding quadratic console/evidence growth in long surveys.

## Research discipline

Do not re-prove the Stage-1 selector mechanism merely to build the map: it is already behaviour-understood. Use the new survey evidence to attach geometry, visual landmarks, trajectory, entities and candidate surface semantics to the proven topology. Promote AI-car, obstacle or material labels only after independent evidence supports them.
