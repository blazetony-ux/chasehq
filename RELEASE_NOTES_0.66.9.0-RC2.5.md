# ChaseHQ-Native v0.66.9.0-RC2.5

RC2.5 is a coherent renderer-correctness and research-script reliability candidate on top of RC2.4.4.

## Proven renderer correction carried into this package

The player-car black under-car/shadow defect was isolated at canonical frame 2352. Body slot 82/map 475 and shadow slot 81/map 575 genuinely overlap; reversing the entire traversal changes thousands of unrelated pixels and making black transparent would destroy real sprite colour data.

RC2.5 preserves MAME-reference descending traversal but implements the missing first-nontransparent sprite occupancy rule. A nonzero-pen candidate reserves the sprite pixel even when its colour is blocked by background priority, so later sprite entries cannot overwrite it. Existing BG/road/text mask decisions remain intact.

The same change corrects individual-sprite forensic export Y coordinates to use the live renderer's visible-raster origin.

Windows proof already completed on the pre-package working tree:
- focused frame-2352 assertion: `overlapPixels=689`, `frontOwnedOverlap=553`, `backVisibleOutsideFront=247`, `exportCoordinatesExact=true`;
- `sprite_priority_tests`: PASS;
- `sprite_snapshot_assertions`: PASS;
- Full Regression: PASS, session `20260930_151202`, run `002`, terminal `=== ChaseHQ Full Regression: COMPLETE ===`;
- pristine RC2.4.4 independently reproduces the historical `cpu_bus_rom_tests` SEGFAULT, proving that failure predates and is unrelated to the sprite change.

## Research Script / Workbench reliability

- Add `try ... finally ... end` to the Workbench Research Script compiler for guaranteed cleanup after normal completion and API/assert/runtime failure; cancellation cleanup is attempted where practical and the original body error remains primary if cleanup also fails.
- Correct nested block variable expansion so loops/conditionals inside `try` blocks are compiled with their local variables rather than being substituted prematurely.
- Add idempotent `timeline.reset`. Script-initiated `timeline.load` captures the pre-load live state so `timeline.reset` can restore it after `timeline.fork-live`.
- Add bidirectional Script Console action/schema parity enforcement. This permanently incorporates the manual `frame.snapshot.assert-overlap` allow-list correction discovered during Windows validation.
- Correct diagnostic bundle source attribution so `fault.json` identifies the actual pasted/library script rather than stale selected-library state.
- Default Script Console history to the active session, while retaining explicit version/session/all-history filtering.
- Strengthen root `AGENTS.md` with safe experiment lifecycle, evidence identity, API exposure parity, pre-existing-failure proof, and manual-hotfix reconciliation rules.
- Package `TC0100SCN Rowscroll Raster-Mapping Probe` with meaningful metadata and fail-safe timeline cleanup.

## Regression/tooling additions

- `research/scripts/regression/regression-sprite-ownership.chqscript` remains the permanent canonical ownership/export-coordinate assertion and is inside Full Regression.
- Add `research/scripts/regression/regression-v06690-rc25-script-safety.chqscript` to exercise timeline seek/fork cleanup back to canonical frame 2064.
- `frame.snapshot.assert-overlap` has offline PowerShell parity through `scripts/Assert-SpriteSnapshotOverlap.ps1`.
- Release/workbench validators now check RC2.5 action parity, script-safety wiring, current-session history defaulting, diagnostic source attribution and permanent regression inclusion.

## Next graphics investigation

Two-frame evidence shows the visible sky/banding exists in raw `bg-bottom`, before final composition. Between frames 2352 and 2360, measured ~3/~7/~10-pixel horizontal movements correspond to rowscroll RAM changes of +3/+7/+10, with the vertical control changing by one. The remaining question is which rowscroll entries map to which visible raster lines. RC2.5 packages a safe counterfactual probe for that question; it does not claim the sky issue fixed.

## Candidate gate

The sprite correction is already proven, but RC2.5 introduces new Workbench/script semantics. Build this exact package on Windows, run the focused Script Safety regression, focused Sprite Ownership regression, and Full Regression before promoting RC2.5 from candidate to proven.
