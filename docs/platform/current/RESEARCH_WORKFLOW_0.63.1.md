# v0.63.1 Research Workflow Patch

This patch rolls up defects and usability findings from live Windows testing of v0.63.0.

## Fixes

- IOC sweep now keeps `debugApiPort` (normally 37600) separate from the IOC hardware port (normally 3). The original PowerShell route used `$port`, which is case-insensitively the same variable name as the script parameter `$Port`, causing `chqctl` to connect to TCP port 3.
- Timer hold now snapshots bytes at `0x100200` and `0x100201` before enabling the hold, matching the already-correct SDL debug-UI behavior.
- Timer control is presented as a single stateful toggle.
- Checkpoint labels use ASCII-safe separators to avoid mojibake in Windows/browser combinations.

## Script Console

`POST /api/v1/script` accepts a bounded safe research script. It uses the same allowlist as the single-command Web bridge, ignores blank lines and `#` comments, supports validation-only mode and supports a bridge-only `wait MS` command from 0 to 5000 ms. It is not a shell.

## Evidence Viewer

The Workbench lists evidence captures under the active session evidence root. A capture can be inspected, its screenshot previewed, its saved state loaded paused, and its self-contained ZIP created/downloaded. New captures automatically create their own ZIP so active files such as the session `bus.log` are never part of the archive.
