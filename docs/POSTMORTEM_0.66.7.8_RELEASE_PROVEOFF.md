# v0.66.7.8 release prove-off postmortem

## Purpose

This is a technical record of the v0.66.7.6-v0.66.7.8 prove-off failures and the controls added so the same classes of mistake are not rediscovered manually during a future release. It is not a substitute for the current process; `docs/RELEASE_PROCESS.md` is the canonical workflow.

## What happened

### 1. Developer-local dependency continuity was broken

A candidate omitted the working `build-local.json`, even though the established Windows workflow relied on it to stage local ROM and SDL sources. A later package also carried the complete SDL source tree unnecessarily.

**Prevention:** developer handoff packages intentionally retain the working `build-local.json`; package validation rejects accidental SDL/out/ROM/session payloads.

### 2. A new validator failed under the real strict-mode build path

An ASCII check assumed a pipeline result always exposed `.Count`. Under `Set-StrictMode -Version Latest`, the same validator failed when invoked through `Prepare-Project.ps1`.

**Prevention:** the canonical proof is the validator running through `Build-Debug.bat`, not a standalone test. Collection expressions that may yield zero/one item must be explicitly array-wrapped where `.Count` is used.

### 3. A Workbench PowerShell parse error reached a candidate ZIP

`Get-StructuredFrameSnapshotRoots()` was handed off with a missing closing brace, so the Workbench process exited before startup logging.

**Prevention:** parse every project-owned `.ps1` before handoff and parse the extracted final archive again with `tools/Test-ReleaseArchive.ps1`.

### 4. Live build identity drifted

Launcher/Workbench metadata retained stale version literals even when the native build had advanced.

**Prevention:** native/CMake/Workbench/health/OpenAPI/schema/handbook/knowledge identity consistency remains release-blocking validation.

### 5. Cross-version discovery blocked the Workbench backend

Initial run/snapshot persistence discovery recursively walked accumulated evidence trees. `/api/v1/script/runs` could monopolize the single-request backend long enough for unrelated requests to hit the 8-second bridge timeout.

**Prevention:** discovery follows known `sessions/<session>/runs/<run>` shapes, reads existing manifests instead of recursively enumerating artifacts, and uses bounded caching/refresh-on-miss.

### 6. Full Regression rebuilt `bundle.zip` after individual commands

Browser-side command expansion called the script endpoint command-by-command, while the server finalized/ZIPped on each request. As artifacts grew, `Compress-Archive` repeatedly ran and Full Regression slowed dramatically.

Predicate/preflight requests could also allocate unrelated tiny runs.

**Prevention:** one script execution owns one parent run. Command requests append only; transient queries allocate no run; `bundle-manifest.json` and `bundle.zip` are generated once at terminal PASS/FAIL/CANCELLED finalization.

### 7. Runner UI could remain visibly RUNNING after backend completion

The 250 ms ticker continued updating status during finalization/history recording and could overwrite a COMPLETE state.

**Prevention:** stop the ticker when command execution ends, show FINALIZING while the final bundle/history is committed, and publish COMPLETE/DONE only afterwards.

### 8. A validator then falsely rejected the correct lifecycle

The lifecycle validator used a multiline-sensitive regex without single-line mode and rejected correct source ordering.

**Prevention:** validator assertions must themselves be exercised through the canonical build path before packaging. Prefer structural/simple assertions over fragile broad regex where practical.

## Process change

The prove-off sequence is now intentionally staged:

1. `Build-Debug.bat`
2. `Start-ChaseHQ.ps1 -Restart`
3. endpoint/bootstrap health
4. `Recovery / Fast Release Gate`
5. focused regression for the changed subsystem
6. Full Regression once
7. archive validation of the exact candidate intended for promotion

If a local patch is needed at any point, it must be folded back into a clean candidate package before final proof.

## Research impact / recovery

The release work did not materially advance Chase H.Q. gameplay fidelity. To prevent further tooling drift, v0.66.7.9 adds a specific research-recovery plan and scripts. The first substantive task is not generic turbo-state discovery: turbo input, active state, remaining count and timer are already confirmed. The next question is the bottom-HUD turbo indicator source/render/compositor path.
