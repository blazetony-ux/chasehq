# ChaseHQ-Native v0.66.8.0 RC3

RC3 is a focused hotfix for offline Forensic Timeline analysis after the real 711,374-write turbo recording exposed a >360 s Windows PowerShell timeout in RC2.

Changes:
- `timeline.trim.event` now performs streaming edge detection and stops after the matching falling edge.
- The command is now non-destructive: it writes an event-window view under `analysis/` and does not delete checkpoints or rewrite the authoritative `frames.csv` / `writes.csv`.
- `timeline.scan.memory` now streams the CSV and aggregates only changed writes, automatically using the indexed event window when present.
- Added a focused RC3 streaming-analysis regression and permanent validator guards against reintroducing full `Import-Csv` / destructive trim behaviour.
- The existing RC1 `turbo-complete-cycle` recording remains the dataset to use; do not replay gameplay.
