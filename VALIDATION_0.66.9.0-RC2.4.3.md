# Validation - v0.66.9.0-RC2.4.3

1. Build normally with `Build-Debug.bat`.
2. Start the Workbench with `Start-ChaseHQ.ps1 -Restart`.
3. Run `research/scripts/regression/regression-v06690-rc243-snapshot-discovery.chqscript` from Script Console. The run must PASS; `frame.snapshot.list require=regression-rc243-snapshot-discovery` is assertive and fails the run if the new snapshot is absent.
4. Run `./Validate-FrameSnapshotRefresh.ps1`; it must print PASS.
5. Run `./Validate-TC0100SCN-Y.ps1`; it must retain the RC2.4 byte-exact frame-2065 PASS.
6. No additional sky-layer experiment is required from the RC2.4.2 capture: the pattern is already localized to the raw TC0100SCN background source. Do not classify it as a compositor defect; a direct authoritative reference comparison is required before treating the source-layer pattern itself as a native-renderer defect.
