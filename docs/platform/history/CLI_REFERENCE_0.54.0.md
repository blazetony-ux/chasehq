# CLI reference additions/changes — v0.54.0

This snapshot records the v0.54.0 diagnostic additions. Run `ChaseHQNative.exe --help` for the complete inherited CLI.

```text
--gameplay-state-log          write authoritative known gameplay_state.csv (signed steering + road state)
--course-data-log             export 0x109000 course banks and live course position/channel state
--steering-signed VALUE       signed 12-bit steering (-2048..2047)
--steering-signed-at F:VALUE  signed steering for one frame
--steering-signed-range A:B:VALUE signed steering for an inclusive frame range
--follow-address A[:B]        follow reads+writes with dynamic caller chain
--follow-read A[:B]           add read provenance range; may be combined with --follow-write
--follow-write A[:B]          add write provenance range; may be combined with --follow-read
```

Course outputs: `course_raw.csv`, `course_banks.csv`, `course_state.csv`.
