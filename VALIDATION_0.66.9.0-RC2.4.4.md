# Validation - v0.66.9.0-RC2.4.4

1. Build normally with `Build-Debug.bat`.
2. Start the Workbench with `Start-ChaseHQ.ps1 -Restart`.
3. Open Script Console and run `Full Regression Suite`.
4. The suite must pass and end with `=== ChaseHQ Full Regression: COMPLETE ===`; RC2.4.4 raises the expanded-command ceiling to 1000 because the current suite expands to 503 commands.
5. Inspect the active session/startup metadata and confirm `build=0.66.9.0`, not the stale historical `0.66.8.0`.
6. The already-uploaded RC2.4.3 focused snapshot-discovery run is accepted evidence for same-turn snapshot discovery: it created the current-run snapshot, `require=NAME` returned it immediately, and both image self-comparisons were identical.
7. Native TC0100SCN/video output is unchanged from RC2.4.3; `Validate-TC0100SCN-Y.ps1` remains the renderer gate if a fresh renderer validation is required.
