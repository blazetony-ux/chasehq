# Validation - v0.66.9.0-RC2.4.2

1. Build normally with `Build-Debug.bat`.
2. Start the Workbench with `Start-ChaseHQ.ps1 -Restart`.
3. Run `research/scripts/regression/regression-v06690-rc242-tc0100scn-snapshot-refresh.chqscript` from Script Console. Confirm the `frame.snapshot.list` output in that same run contains the newly created `regression-rc242-tc0100scn` snapshot.
4. Run `./Validate-FrameSnapshotRefresh.ps1`; it must print PASS.
5. Run `./Validate-TC0100SCN-Y.ps1`; it must retain the RC2.4 byte-exact frame-2065 PASS.
6. For the ongoing graphics investigation, run `research/scripts/graphics/tc0100scn-post-y-fix-layer-isolation.chqscript`, then inspect Raw BG0, Raw BG1, contribution layers and final-no-hud in Graphics Lab.
