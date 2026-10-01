# ChaseHQ-Native v0.64.6 — Windows Frontend Compile Fix

## Fix

- Fixed the concurrent Web Workbench frontend failing to compile under Windows PowerShell `Add-Type`.
- Replaced the C# 6 expression-bodied UTF-8 helper with C# 5-compatible block syntax.
- Keeps the v0.64.3 concurrent public frontend / serialized backend architecture unchanged.

## Why

Windows PowerShell's CodeDOM compiler rejected the expression-bodied method in the embedded C# frontend with `; expected`, preventing the Workbench from starting on port 37680.

## Validation target

Launch with `./Start-ChaseHQ.ps1`; the Web Workbench should start on 127.0.0.1:37680 and script-library load should remain responsive even while native polling is busy.
