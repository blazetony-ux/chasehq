# RC2.7 source handoff gate

Linux can produce the source candidate and run available native/JavaScript tests. Local `Build-Debug.bat` supplies SDL and ROMs via the preserved staging configuration. Source delivery and Windows promotion are separate claims. Run the bundled history/CLI-evidence CTest gates and focused/Full Regression locally before calling this candidate proven. See `../VALIDATION_0.66.9.0-RC2.7.md`.

---
## v0.66.9.0-RC2.4 candidate gate

RC2.4 requires Windows/SDL visual proof that canonical frame 2065 with neutral BG offsets matches the RC2.3 both-sign-proxy, then a title/attract + later Stage 1 spot-check, focused regression, and Full Regression. Package promotion is blocked until those pass.

## v0.66.9.0-RC1 candidate gate

This candidate must first pass `Regression - v0.66.9.0 Game Lab and Tile Inspector`, including PNG export on Windows PowerShell/System.Drawing. Only after the focused proof should Full Regression be run. v0.66.8.0 remains the proven fallback until both gates pass.

# Release / prove-off process

Current proven baseline: **v0.66.8.0**, promoted 2026-09-29 after focused bounded-history and Full Regression PASS.

This file is the canonical ChaseHQ-Native Windows development/release process. The detailed incident record is `docs/POSTMORTEM_0.66.7.8_RELEASE_PROVEOFF.md`. It exists because the v0.66.7.6-v0.66.7.8 prove-off exposed several avoidable packaging, validator, discovery and runner-lifecycle failures. Do not improvise around this process unless evidence shows the process itself is wrong.

## 1. Canonical build path

The normal Windows entry point is always:

```powershell
.\Build-Debug.bat
```

`Build-Debug.bat` runs project preparation, local dependency staging, static validation, CMake configuration/build and packaged tests. Do not insert an ad-hoc validator gate in front of it and do not bypass it with a one-off build command.

For a normal research launch use:

```powershell
.\Start-ChaseHQ.ps1 -Restart
```

## 2. Local dependencies and package policy

Developer handoff packages may carry the current `build-local.json` so the established local build continues immediately. The checked-in template remains `build-local.example.json`.

The handoff/research ZIP must not contain:

- copyrighted ROM contents;
- the full SDL source tree when `build-local.json` points to the local SDL source;
- stale `out` build products;
- transient evidence/session trees;
- `.bak`, `.pre-*.bak`, temporary patch helpers or editor debris.

A public/source release can omit machine-specific `build-local.json`; a developer handoff package may intentionally retain it. Never substitute bundled SDL for a working local dependency configuration without a specific reason.

## 3. Static validation requirements

Before a package is handed to Windows testing:

1. Parse every project-owned `.ps1` file with the PowerShell parser.
2. Run the Workbench validator under the same strict-mode/invocation path used by `Prepare-Project.ps1`.
3. Verify native, CMake, Workbench, health/OpenAPI, feature-manifest, API-schema, script-schema, handbook and knowledge build identities agree.
4. Validate `.chqscript` includes and required metadata.
5. Verify no debug backup files or common mojibake sentinels remain in project-owned text.
6. Verify the current Full Regression ends with `=== ChaseHQ Full Regression: COMPLETE ===` and contains all current permanent regressions.

A validator that has only been tested standalone is not considered proven. The real gate is the validator running through `Build-Debug.bat`.

## 4. Package validation before user handoff

A candidate ZIP should be checked after creation, not just before it:

- archive opens successfully;
- expected project root exists once;
- no `SDL/`, `out/` or ROM payload has leaked into the archive;
- `build-local.json` is present for the developer-handoff package when continuity requires it;
- canonical checkpoints, docs, schemas and scripts are present;
- `Start-ChaseHQWeb.ps1` parses;
- package size is consistent with a source/research package rather than an accidental dependency/build-output bundle.

Do not ask the user to discover packaging mistakes that can be detected from the ZIP itself.

Use the packaged archive validator before handoff:

```powershell
.\tools\Test-ReleaseArchive.ps1 -ZipPath .\ChaseHQ-Native-vX.Y.Z.zip -DeveloperHandoff
```

It expands the archive into a temporary directory, rejects bundled SDL/out/ROM/session debris, parses every PowerShell file and runs `Validate-Release.ps1 -SkipBuild` against the extracted copy.

## 5. Prove-off order

Use progressively more expensive gates. Do not start with Full Regression.

### Gate A - build

```powershell
.\Build-Debug.bat
```

If this fails, stop. Fix the candidate and create a new candidate package; do not make the failed hand-edited tree the release artifact.

### Gate B - launch / browser bootstrap

```powershell
.\Start-ChaseHQ.ps1 -Restart
```

Require native Research API ready, Workbench ready and browser frontend bootstrap READY.

### Gate C - quick endpoint health

The root page, `/api/v1/status`, `/api/v1/script/runs` and `/api/v1/frame/snapshots` must return within the Workbench bridge timeout. Expensive discovery must use bounded known directory shapes and/or caching; never recursively walk accumulated evidence trees in a request path.

### Gate D - fast release gate

Run:

```text
Recovery / Fast Release Gate
```

This checks the exact high-risk surfaces that previously consumed most prove-off time: run history, docs search, checkpoint load, layered snapshot creation and snapshot rediscovery.

### Gate E - focused regression for the changed subsystem

Run the regression that directly proves the current change. Examples include run-finalization, discovery performance, layered-frame reconstruction or memory tracing.

### Gate F - Full Regression

Only after Gates A-E are clean, run `Full Regression Suite` once against the exact package intended for promotion.

## 6. Script-run lifecycle contract

One user script run owns exactly one run directory.

The browser/server contract is:

1. allocate/open one RUNNING run;
2. execute expanded commands into that run;
3. transient preflight/predicate queries do not allocate independent runs;
4. do not rebuild `bundle.zip` after each command;
5. stop the RUNNING ticker when command execution finishes;
6. show FINALIZING while metadata/history/manifest/ZIP are committed;
7. generate `bundle-manifest.json` and `bundle.zip` once at PASS/FAIL/CANCELLED terminal finalization;
8. only then show DONE / COMPLETE and return controls to idle.

If `Compress-Archive` repeatedly appears while individual commands are still executing, stop the run: the lifecycle contract is broken.

## 7. Run history and Graphics Lab persistence contract

Browser `localStorage` is not authoritative. Completed server-side run metadata is authoritative and must be rediscovered from the known `evidence/sessions/<session>/runs/<run>` shape.

Graphics Lab must discover structured snapshots both from the active snapshot root and from completed run artifact roots. Discovery must remain bounded and cached enough that old accumulated evidence cannot starve the single-request Workbench backend.

## 8. Evidence bundle rule

A completed important run should be portable from its one `bundle.zip`. The bundle should contain the exact script source, console output, run metadata, manifest and all run-owned artifacts. Run bundles are supporting evidence; authoritative conclusions still belong in project knowledge/docs after review.

## 9. Versioning / hotfix discipline

Do not cut a new version for every observation. Prefer one coherent fix package. However, never call a candidate proven after manual local patching unless that patch has been folded back into a clean candidate ZIP and the final gate is run against that clean package.

Version identity changes must be swept across every live identity surface in the same change and checked by validation. Historical release/changelog text is not rewritten merely to make string searches clean.

## 10. Research resumes after release work

Release tooling is not the project goal. Once a candidate is proven, return immediately to gameplay/render fidelity research. The current substantive sequence is documented in `docs/RESEARCH_RECOVERY_PLAN.md`.

## RC2 focused-gate note
For v0.66.9.0-RC2, run the dedicated sprite-order/speed-semantics regression before any Full Regression. A/B graphics work should use captured timelines rather than repeated gameplay.
