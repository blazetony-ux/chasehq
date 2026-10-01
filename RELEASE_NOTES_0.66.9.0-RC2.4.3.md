# ChaseHQ-Native v0.66.9.0-RC2.4.3

RC2.4.3 is a Workbench snapshot-discovery correctness hotfix on top of RC2.4.2. Native emulator/video output is unchanged from RC2.4.2.

## Why this patch exists
The first Windows RC2.4.2 regression bundle proved that `frame.snapshot` created the requested snapshot, but the immediately following `frame.snapshot.list` still returned only older snapshots. The RC2.4.2 regression itself was also non-assertive, so the run was marked PASS despite the missing list entry.

## Fix
`New-StructuredFrameSnapshot` now records every newly created structured snapshot in an explicit same-process registry before invalidating disk-discovery caches. Snapshot listing merges this registry with cached/on-disk discovery, and named snapshot resolution consults it before scanning disk. The known evidence-session-root cache is also invalidated when the snapshot tree changes.

`frame.snapshot.list` gains an optional `require=NAME` parameter. When supplied, the action throws unless exactly one snapshot with that name is present in the returned list. Existing no-argument behaviour is unchanged.

## Regression coverage
Added `research/scripts/regression/regression-v06690-rc243-snapshot-discovery.chqscript`. It creates `regression-rc243-snapshot-discovery`, immediately executes `frame.snapshot.list require=regression-rc243-snapshot-discovery`, and therefore cannot report PASS if the same-turn discovery bug remains.

`Validate-FrameSnapshotRefresh.ps1` remains the HTTP-level validation gate.

## Graphics investigation result carried forward
The RC2.4.2 post-Y-fix layered capture localized the visible sky pattern to the raw TC0100SCN bottom/background source rather than the compositor. Because the pattern is already present in the raw TC0100SCN background source, it is not classified as a compositor corruption. A direct authoritative reference comparison is required before treating the source-layer pattern itself as a renderer defect. Remaining active graphics work returns to genuine mismatches such as player-car shadow/under-car ordering and any independently proven geometry/compositor differences.
