# ChaseHQ-Native v0.66.8.0-RC6

Focused Forensic Timeline compatibility fix.

- Fixes `timeline.scan.memory` against legacy/damaged RC2 timelines whose `writes.csv` was rewritten by PowerShell `Export-Csv` and therefore contains quoted numeric fields.
- Streaming parser now accepts both original unquoted native CSV and quoted legacy CSV without `Import-Csv`.
- `timeline.scan.memory` now accepts `trigger`/`stop` aliases as well as canonical `triggerFrame`/`stopFrame`, while the packaged recovery script uses the canonical names.
- Adds permanent RC6 legacy timeline CSV compatibility regression.
- No gameplay replay is required; the existing turbo recording remains the investigation source.
