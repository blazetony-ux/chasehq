# Recovery / release-gate scripts

These scripts are deliberately short. Use them before a long Full Regression or after any packaging/Workbench change.

- `research-session-preflight.chqscript` is non-destructive and confirms build identity, core APIs, checkpoints, regions, current knowledge and gameplay registry access.
- `fast-release-gate.chqscript` exercises the specific surfaces that caused the v0.66.7.6-v0.66.7.8 prove-off churn: run discovery, docs search, checkpoint load, structured snapshot creation and post-run snapshot listing.

A Full Regression is the final gate, not the first diagnostic step. If either recovery script fails, fix that failure before spending time on Full Regression.
