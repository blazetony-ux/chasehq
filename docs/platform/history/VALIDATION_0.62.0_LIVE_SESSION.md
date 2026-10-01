# v0.62.0 live Windows validation observations

Observed on the Windows development machine before the v0.62.1 launcher-only update:

- Core SDL/emulation boots and continues through attract/game screens.
- Native debug API listens successfully on `127.0.0.1:37600`.
- `Start-ChaseHQWeb.ps1` serves the Web Debugger on `127.0.0.1:37680` and communicates with the native API.
- Web UI reports live frame/IOC/sprite information.
- Live handling override transport is functional: the browser successfully changed `cornering_scale` from `1` to `1.5` and reported `override=1`; gameplay-effect validation remains outstanding.
- `Start-ChaseHQResearch.ps1 -Checkpoint` successfully loads checkpoints.
- `stage1-driving-2352.chqstate` is an attract-mode/autonomous reference state, not player gameplay.
- `stage1-gameplay-2064.chqstate` successfully restores a Stage 1 player-gameplay state.
- Minor Web UI UTF-8/mojibake text remains to be fixed.

These observations validate transport/startup behaviour only where stated; they do not imply that all v0.62 Workbench functions have been tested.
