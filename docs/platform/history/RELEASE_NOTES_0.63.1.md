# ChaseHQ-Native v0.63.1 - Workflow Patch

Patch release based on live Windows testing of v0.63.0. No intentional game/emulation behavior changes except the timer-hold defect fix.

## Corrected
- IOC sweep PowerShell variable shadowing (`$port` vs `$Port`).
- Timer hold captures current timer instead of holding zero and causing TIME UP.
- Timer UI changed to one toggle.
- Checkpoint/dropdown mojibake-prone separators replaced with ASCII-safe labels.

## Added
- Safe multi-command Research Script Console.
- Evidence Viewer, state reload, screenshot preview, automatic ZIP creation and ZIP download.
