# CLI Reference — v0.55.0 additions

- `--course-survey` — combined accelerator + course-follow + infinite-time + auto/unlimited-turbo + state/course logging mode.
- `--course-follow` — state-driven survey steering from current course `+0` curvature.
- `--course-follow-steer N` — steering magnitude, 1..96; default 32.
- `--course-follow-deadzone N` — curvature deadzone; default 7.
- `--infinite-time` — persistent race-timer hold at the checkpoint/start value.
- `--unlimited-turbo` — persistent turbos-left restoration to 3.
- `--auto-turbo` — unlimited turbo plus genuine IOC turbo pulses every 220 frames.
- `--gameplay-state-log` — includes autopilot target and signed grade/profile fields.
- `--course-data-log` — includes signed `ch1_profile_signed` plus raw course bytes.

All v0.54 tracing, provenance, checkpoint, steering, course-data, graphics and experiment switches remain available.
