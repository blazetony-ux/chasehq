# ChaseHQ-Native v0.64.8.3

Corrective Script Console lifecycle fix over v0.64.8.2.

- Script completion is now committed immediately after the final command.
- The RUNNING ticker is stopped before any post-run dashboard/evidence refresh.
- Run/selection/current-line controls are re-enabled immediately on completion.
- Post-run refresh/evidence housekeeping runs asynchronously and can no longer leave the console stuck at RUNNING 4/4.
- Carries forward v0.64.8.2 intervention APIs, Track View colouring, UTF-8 fixes and clipboard fallback.
