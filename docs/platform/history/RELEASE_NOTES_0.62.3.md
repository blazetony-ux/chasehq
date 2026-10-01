# v0.62.3 Web Host Reliability Fix

## Fixes
- Web host now binds both `127.0.0.1` and `localhost` loopback URLs.
- One-command launcher redirects Web host stdout/stderr to `.chq/web-stdout.log` and `.chq/web-stderr.log`.
- Launcher now verifies both `/api/v1/status` and `/` before reporting READY.
- Launcher verifies the Web host process remains alive after readiness.
- A Web host startup failure now returns its captured logs instead of falsely reporting READY.
- Web/API self-reported version updated to 0.62.3.

## Validation status
Native Windows/SDL gameplay and checkpoint loading were observed working in v0.62.2. This Web-host fix requires Windows validation.
