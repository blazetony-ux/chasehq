# Validation - v0.66.7.7

Status: RELEASE CANDIDATE - awaiting Windows/SDL proof.

## Reason for this revision

v0.66.7.6 Full Regression passed, but interactive prove-off found two post-run discovery defects:

1. completed server-side runs and their bundle ZIPs could exist on disk while Recent Script Runs omitted them because the UI depended on browser localStorage;
2. frame snapshots captured under a run artifact root were no longer visible in Graphics Lab after the run ended.

## Fixes

- authoritative run catalogue scans `evidence/sessions/*/runs/*/run-metadata.json`;
- Workbench merges that catalogue with local history;
- `/api/v1/script/runs` and `api script.runs` expose the catalogue;
- structured snapshot listing scans active and completed-run `frame-snapshots` roots;
- snapshot manifest/asset resolution uses the same discovery path; newest duplicate name wins;
- Windows PowerShell strict-mode validator `.Count` checks are array-safe.

## Windows proof required

1. `Build-Debug.bat` passes via the canonical documented process.
2. Start Workbench/SDL normally.
3. Recent Script Runs shows the prior/current completed Full Regression from disk after a reload/restart.
4. Graphics Lab lists and opens a snapshot from a completed run, including normal/HUD-hidden/HUD-only/layers/individual-sprites modes.
5. Run `Regression - v0.66.7.7 Run History / Snapshot Discovery`.
6. Run Full Regression.
