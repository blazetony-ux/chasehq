# CLI additions — v0.56.0

- `--course-survey` — accelerator + adaptive course follower + infinite time + auto/unlimited turbo + gameplay/course logs + automatic ZIP.
- `--course-follow-lookahead N` — inspect 0..8 upcoming records in the current bank; default 2.
- `--course-follow-recovery-steer N` — steering magnitude used while EDGE/OFF_ROAD; 1..96, default 48.
- Existing `--course-follow-steer N` default 32 and `--course-follow-deadzone N` default 7 remain available.

New survey artifacts: `offroad_excursions.csv`, `course_survey_summary.txt`, `pursuit_probe.csv`, and `pursuit_events.csv`. The pursuit files are correlation/provenance aids; they do not yet claim an authoritative police-light bit. `gameplay_state.csv` adds `steering_injected_signed`, `autopilot_curve_now`, `autopilot_curve_ahead`, and `autopilot_mode`.

## v0.57 closed-loop additions
`--course-follow` now uses live lateral road geometry as feedback in addition to curvature look-ahead.

- `--course-follow-lateral-kp X` — proportional centring gain, default 0.006, range 0..0.05.
- `--course-follow-lateral-max N` — maximum absolute lateral correction, default 48, range 1..96.
- `--course-follow-lateral-deadzone N` — centre-error deadzone, default 96, range 0..4096.
- `--course-follow-recovery-steer N` remains accepted for v0.56 command compatibility but blind recovery is not used by the v0.57 closed-loop controller.
