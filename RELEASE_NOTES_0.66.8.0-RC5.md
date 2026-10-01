# ChaseHQ-Native v0.66.8.0-RC5

Focused startup/recovery correction after RC4 Windows prove-off.

- Restores the RC3-proven Workbench source as the base and keeps the turbo recovery non-destructive.
- The recovery script no longer calls `timeline.trim.event` on the legacy RC1 capture; it uses the already-proven turbo event frames 2065..2275 and one-frame context directly.
- `timeline.scan.memory` bounds its streaming pass from explicit trigger/stop/window arguments even when no event index exists.
- Corrects stale `0.66.7.4` build identity in `Start-ChaseHQResearch.ps1` startup/session records.
- Workbench child stdout/stderr are now redirected to `.chq/web-stdout.log` and `.chq/web-stderr.log` so an early process failure is no longer silent.
- Adds `.chq/web-startup.log` before the private backend listener is opened and records backend/type/front-proxy startup phases and exceptions.
- Launcher Web readiness deadline increased from 15 to 30 seconds to allow first-run Add-Type/JIT startup.
- Support/diagnostic bundles now carry `web-startup.log`.

The original turbo gameplay must not be replayed for this correction.
