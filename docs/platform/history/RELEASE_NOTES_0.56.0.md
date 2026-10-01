# ChaseHQ-Native v0.56.0 — Adaptive Course Survey + Excursion Diagnostics

## Purpose
This build turns the first v0.55 long survey into a measurable refinement loop. It does not claim that the course follower is solved. Its job is to expose exactly where it fails and make each subsequent pass comparable.

## New in v0.56.0
- `--course-survey` now enables automatic evidence ZIP packaging. With an explicit `--logs` directory the ZIP is written beside that directory as `course_survey.zip`.
- Course follower adds configurable look-ahead (`--course-follow-lookahead`, default 2 records). It examines the current and upcoming records within the current bank and steers using the strongest curvature. It deliberately does not guess across a bank boundary.
- EDGE/OFF_ROAD activates a stronger recovery magnitude (`--course-follow-recovery-steer`, default 48). Because a signed left/right road displacement has not yet been proven, recovery retains the curvature sign rather than inventing a side-of-road signal.
- `gameplay_state.csv` now distinguishes injected raw and signed steering and records current curvature, look-ahead curvature and controller mode.
- `offroad_excursions.csv` records each OFF_ROAD episode with start/end frame, duration, course positions, bank/record and maximum speed.
- `course_survey_summary.txt` records INTERIOR/EDGE/OFF_ROAD frame totals, excursion count and controller parameters.
- Existing infinite-time, unlimited turbo, auto-turbo, grade, road-state and course-state diagnostics are retained.

## Pursuit / police-light investigation
The v0.55 survey proved that a long autonomous run can reach/catch the target, making pursuit-phase state a priority. v0.56 carries police beacon and target/enemy-car state as explicit reverse-engineering targets. `pursuit_probe.csv` records course position, score and a full-palette CRC every survey frame, while `pursuit_events.csv` marks score transitions with the same context. This gives us anchors around catches/progression for targeted provenance. It **does not label any unproven RAM/palette value as POLICE LIGHT**. The next evidence should be used to correlate catch/progression transitions with sprite/palette/memory activity before an authoritative UI field is added.

## Known limitation
The recovery controller knows INTERIOR/EDGE/OFF_ROAD but not which side of the road the car occupies. A proven signed lateral-error/side-of-road signal remains the key next control discovery.
