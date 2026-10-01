# ChaseHQ-Native v0.66.9.0-RC2.4.2

RC2.4.2 is a focused Workbench/research-pack hotfix on top of RC2.4.1. The emulator/renderer is unchanged from RC2.4.1.

## Structured snapshot discovery
A structured snapshot created by `frame.snapshot` could be absent from an immediately following `frame.snapshot.list` or Graphics Lab refresh because the Workbench retained snapshot/root discovery caches for up to five/ten seconds. `New-StructuredFrameSnapshot` now invalidates both caches as soon as the manifest is committed.

## Packaged investigation workflow
Added `research/scripts/graphics/tc0100scn-post-y-fix-layer-isolation.chqscript`. The earlier `post-y-fix-sky` wording was merely an arbitrary snapshot evidence name, not a built-in command or existing project object.

## Regression coverage
Added `research/scripts/regression/regression-v06690-rc242-tc0100scn-snapshot-refresh.chqscript` and `Validate-FrameSnapshotRefresh.ps1`. The existing `Validate-TC0100SCN-Y.ps1` remains the byte-exact visual gate for the RC2.4 Y-scroll fix.

No native C++, Research API contract, or `.chqscript` grammar change is introduced by RC2.4.2.
