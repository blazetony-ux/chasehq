# Chase H.Q. Native v0.55.0 — Course Survey + Autopilot Diagnostics

## Course-follow survey assist
- Adds `--course-survey`, a one-switch long-run mode that holds the genuine accelerator input and enables course-follow, infinite time, auto/unlimited turbo, course-data logging and gameplay-state logging.
- Adds `--course-follow`, a live state-driven steering assist using the authoritative current course bank/record and proven signed `+0` horizontal-curvature channel. It deliberately does not infer steering from screenshots.
- `--course-follow-steer N` controls the signed steering magnitude (default 32); `--course-follow-deadzone N` controls the straight-road curvature deadzone (default 7).
- The initial controller is intentionally conservative: curvature sign selects steering sign, based on v0.54 experiments where ±32 kept the car INTERIOR through the tested bank-0 bends at ~200 km/h. This is a survey/debug controller, not yet a complete gameplay AI.

## Persistent survey assists
- `--infinite-time` continuously restores the starting BCD timer value at `$100200/$100201`; unlike v0.54 `--patch-when`, it is persistent rather than one-shot.
- `--unlimited-turbo` continuously restores authoritative turbos-left `$1003A2` to 3.
- `--auto-turbo` implies unlimited turbo and generates a genuine IOC3 bit-0 turbo pulse every 220 frames, preserving the normal game turbo path rather than forcing `$100212`.

## Grade / vertical profile
- Debug UI and gameplay CSV now expose `GRADE` plus the signed course `+1` vertical-profile command.
- `course_state.csv` adds `ch1_profile_signed` while retaining the raw byte.
- Positive/negative `+1` are proven opposite vertical-profile directions with zero neutral. The UI currently names the normalized positive direction `INCLINE` and negative `DECLINE`; absolute arcade/world sign should still be checked visually before treating that sign convention as hardware-authentic.

## Debug UI / structured logging
- F1 overlay now prioritises authoritative gameplay/course state: speed, distance, score, steering/curve, road zone/flags, grade, turbo/count, course bank/record and active survey assists.
- `gameplay_state.csv` adds `autopilot_target_signed`, `grade_state`, and `grade_profile_signed`.

## Branching-course direction
Stage/course mapping is now treated as a graph rather than a guaranteed single centreline. Future survey work must enumerate junction choices and rejoin points rather than assuming numeric bank order is a complete track.
