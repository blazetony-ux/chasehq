# ChaseHQ-Native v0.57.0 — Closed-Loop Course Follower

## Purpose
v0.57 replaces v0.56's blind EDGE/OFF_ROAD recovery with a closed-loop controller based on the road-contact operands proven in the v0.56 lateral probes.

## Authoritative lateral geometry
- CPU A `0x10A044`: live car/road lateral coordinate. Normal per-frame writer observed at PC `0x008B84`; road/contact adjustments at `0x008698`/`0x0086A6`.
- CPU A `0x10A05C`: live 32-bit road boundary pair. Writer observed at PC `0x00812E`.
- CPU A `0x10A048`: established road flags / INTERIOR, EDGE, OFF_ROAD classification.
- The road-contact code around `0x0085FC–0x00864E` compares the lateral coordinate against the live boundaries with the known boundary margin. The bounds are dynamic, so the controller never targets a hard-coded centre.

## Closed-loop follower
`--course-follow` now combines:
1. existing course +0 curvature/look-ahead feed-forward;
2. live wrap-safe road-centre calculation from `0x10A05C`;
3. signed lateral error from `0x10A044` relative to that centre;
4. proportional centring correction with configurable gain/deadzone/cap.

v0.56's blind same-direction ±48 recovery is not used by the controller. The legacy option remains accepted for command-line compatibility.

New tuning options:
- `--course-follow-lateral-kp X` (default 0.006)
- `--course-follow-lateral-max N` (default 48)
- `--course-follow-lateral-deadzone N` (default 96)

## Telemetry
`gameplay_state.csv` now adds:
- `autopilot_feedforward`
- `autopilot_lateral_correction`
- `road_left_bound`, `road_centre`, `road_right_bound`
- `car_lateral`, `lateral_error`, `lateral_side`

`offroad_excursions.csv` adds start/max lateral-error evidence for every excursion.

## Pursuit work
The v0.56 pursuit probe/events are retained. Police beacon and target-car state are still forensic targets; v0.57 does not falsely label an unproven address as either state.

## Acceptance target
Compare against the v0.56 survey baseline: 28.1% INTERIOR / 71.3% OFF_ROAD, furthest observed course position about `0x416870`. The first v0.57 survey should materially improve road holding while retaining long-course progression.
