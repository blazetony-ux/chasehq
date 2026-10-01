# RC2.4.2 same-turn snapshot discovery failure

Windows evidence supplied on 2026-09-30 from:

- `bundle(20260929-234727).zip` — `Regression - v0.66.9.0 RC2.4.2 TC0100SCN / Snapshot Refresh`
- `bundle(20260929-234833).zip` — `TC0100SCN Post-Y-Fix Layer Isolation`

Both runs successfully created a structured snapshot inside the current run artifact tree, then immediately executed `frame.snapshot.list`. In both cases the returned list contained only older RC2.3 snapshots and omitted the snapshot just created.

Observed current-run names:

- `regression-rc242-tc0100scn` at frame 2065
- `tc0100scn-yfix-2065` at frame 2065

The RC2.4.2 regression still reported PASS because its list command was diagnostic-only and did not assert that `regression-rc242-tc0100scn` was present. This is the direct evidence for the RC2.4.3 recent-snapshot registry and `frame.snapshot.list require=NAME` contract.

Do not classify RC2.4.2 as proof of immediate snapshot discovery.
