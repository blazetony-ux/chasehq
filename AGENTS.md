# ChaseHQ-Native — Agent Guide

Use this file as the durable operating contract for coding/research agents in this repository.
Keep **fast-changing project status out of this file**; read the current handover/state documents for that.

## Mission

The goal is accurate Chase H.Q. emulation and reverse engineering.
The Workbench, API, scripts, diagnostics, knowledge system, evidence capture, and automation exist to support that goal.
Do not let non-blocking tooling/UI polish displace a concrete gameplay or rendering investigation.

Prefer evidence-backed corrections over guesses, visual compensation, or hand-tuned offsets.

## Current sources of truth

Read only what the task needs, but use these as the authoritative map:

- `docs/HANDOVER.md` — current continuation point and next gate.
- `docs/PROJECT_STATE.md` — current confirmed project state.
- `docs/ROADMAP.md` — completed/outstanding/longer-term work.
- `docs/NEXT_BUILD_CHANGELOG.md` — pending coherent build changes when relevant.
- `docs/API_REFERENCE.md` and `research/schema/api-actions.json` — Research API.
- `docs/SCRIPT_LANGUAGE_REFERENCE.md` and `research/schema/script-language.json` — Research Script v2.
- `research/scripts/SCRIPT_CATALOG.md` — packaged script inventory.
- `docs/RELEASE_PROCESS.md` / `VALIDATION.md` — release and proof gates.

Do not infer the authoritative version from the folder name.
Do not assume the newest candidate is already the proven baseline.
If documents disagree, identify and fix the stale source instead of silently choosing one.

## Default engineering loop

Use this order unless the defect is already proven:

1. Reproduce deterministically.
2. Inspect existing evidence, logs, source, schemas, checkpoints, and prior runs.
3. Form a narrow falsifiable hypothesis.
4. Use existing diagnostics before adding tooling.
5. Prefer causal A/B experiments over visual intuition.
6. Patch only when evidence isolates a real defect or coherent tooling gap.
7. Add/update same-change regression coverage.
8. Validate the exact working tree.
9. Update project state/roadmap/handover/docs/knowledge as required.
10. Package last.

Do not cut a build merely because an experiment produced an interesting result.

## Investigation vs implementation

**Investigation mode:** read/search, use existing debugger/API/Workbench/chqctl surfaces, run packaged scripts, capture structured evidence, and use reversible runtime/presentation experiments. Avoid permanent source changes while the failure mechanism is still speculative.

**Implementation mode:** make the smallest coherent source/tooling correction after the evidence supports it. Preserve public-surface parity, add regression coverage, update schemas/docs, and validate before claiming success.

## Build and runtime rules

Do not assume the tree has been compiled.
Check for the expected runtime before runtime-dependent work.
Use repository build/launcher scripts rather than inventing a parallel build path.

Typical Windows commands:

```powershell
.\Build-Debug.bat
.\Start-ChaseHQ.ps1 -Restart
```

Static inspection or a non-Windows compile is not proof of a Windows/SDL/Workbench behavioural fix.
State exactly what was and was not run.

Do not require paid APIs, separately purchased credits, or external cloud services for the normal project workflow.

## Process/port safety

- Never terminate PID 4/System.
- Do not kill unrelated processes to free a port.
- Prefer repository restart/cleanup logic.
- When a stale ChaseHQ listener exists, identify the actual ChaseHQ process by command-line/provenance before terminating it.
- Prefer port-scoped cleanup over broad process-name cleanup.

## API and `.chqscript` rules

Never invent an API action, parameter, or script-language construct.
For an unfamiliar operation, inspect/query the authoritative surface first:

```text
api actions
api schema action=NAME
api schema.all
api script.schema
api script.functions
```

For Workbench Script Console workflows, use the current **Research Script v2** contract and prefer existing `api ACTION key=value` calls over new grammar.

The repository also contains older/other `.chqscript` usage under experiment/debugger tooling. Determine the intended runner before editing or executing a file; do not assume identical semantics from the extension alone.

Pasted/ad-hoc Research Script v2 should normally include:

```text
# name: Meaningful investigation name
# purpose: What this proves or measures.
# category: appropriate-category
# target: Frame/checkpoint/subsystem under test.
# method: What is changed or measured.
# expected: Evidence that distinguishes the hypotheses.
# cleanup: State that must be restored even on failure.
```

### Safe experiment scripting

Treat emulator/debugger state as persistent across failed scripts. Never assume the previous script completed or cleaned up successfully.

For scripts that mutate runtime/research state:

- establish a known starting state explicitly;
- document target, method, expected evidence, and cleanup in the header;
- historical timeline state is read-only: fork to live state before memory writes;
- use idempotent cleanup/reset actions where available;
- use `try ... finally ... end` for timeline, sprite, patch, presentation, and other temporary mutations;
- cleanup must run on assertion/API/runtime failure and user abort where practical;
- cleanup failure must not hide the original experiment failure;
- restore temporary visual overrides, sprite overrides, timeline state, patches, and presentation changes unless the script explicitly documents otherwise.

Do not "fix" a failed experiment merely by rerunning it. Inspect the failure state first and determine whether the previous run left persistent state behind.

If a snapshot/run/evidence name is arbitrary, say so. Do not present a label invented for one experiment as an established feature.

## Evidence rules

Structured evidence is authoritative; screenshots are supporting evidence.

Prefer run-owned/session-owned outputs with machine-readable metadata, logs/CSV/JSON, checkpoints/timelines, layered snapshots, reconstruction metadata, and automatic ZIP bundles.

A requested run should be reproducible: preserve the checkpoint/scenario, frame range, inputs/patches, trace settings, output location, exit condition, and evidence bundle where supported.

Do not infer gameplay state from pixels when the underlying emulated/game state can be measured directly.

When reporting findings, distinguish observed fact, inference, code explanation, and untested assumption.
Never report PASS for a property the test did not actually assert.
Validate evidence-bundle identity before drawing conclusions: recorded script path/name, embedded script body, run/session metadata, and artifacts should describe the same run. If they disagree, report the metadata defect rather than attributing the failure to the stale name.

## Canonical checkpoints

Preserve these stable research references unless explicitly superseded:

- `checkpoints/stage1-gameplay-2064.chqstate`
- `checkpoints/stage1-driving-2352.chqstate`
- `checkpoints/stage1-target-post-final-hit-9548.chqstate`
- `checkpoints/stage1-end-level-9988.chqstate`

Preserve companion metadata. Do not silently overwrite canonical checkpoint files.

## Graphics/reverse-engineering rules

- Separate source-layer defects from compositor/priority defects.
- Inspect raw source layers before compensating with presentation offsets.
- Prefer deterministic frame comparisons.
- Preserve alpha/reconstruction metadata in layered snapshots.
- Use per-sprite provenance when aggregate sprite evidence is insufficient.
- Preserve HUD normal/hidden/only comparisons where relevant.
- Do not bake forensic layer offsets into defaults without hardware/reference evidence.

For sprite order, palette behaviour, TC0100SCN, TC0150ROD, road priority, or mixer behaviour, prefer a causal A/B experiment.

## Regression policy

Regressions are living tests, not demonstrations.

Any meaningful change to API/script behaviour, Workbench discovery/state, evidence ownership/bundling, renderer semantics, checkpoint determinism, or launcher/process behaviour needs focused regression coverage in the same change.

A regression must assert the actual property under test.
Logging output followed by `PASS` is not sufficient.
Do not accept a failure as "known" or "pre-existing" solely because a build/test script labels it that way. When practical, reproduce it against the last pristine proven baseline before classifying it as pre-existing.

When relevant:

1. run the focused build-specific regression first;
2. run `research/scripts/regression/full-regression.chqscript`;
3. preserve correct Full Regression ordering and its terminal COMPLETE marker;
4. treat unexpected `ERROR:` output or missing declared artifacts as failure.

## Documentation and public-surface parity

When public behaviour changes, update the authoritative sources in the same change as applicable:

- API/script schemas and references;
- API/script changelogs;
- script catalog and feature manifest;
- Workbench/User Handbook knowledge sources;
- `docs/PROJECT_STATE.md`;
- `docs/ROADMAP.md`;
- handover/continuation docs;
- release and validation notes.

Do not leave current-build catalogs/generated docs carrying stale version labels for a surface you changed.

Prefer CLI/config/API parity for diagnostics. UI-only research capabilities are incomplete when a practical scriptable equivalent should exist.
When adding or changing a Research API action, verify every exposure layer: schema, dispatcher/implementation, Script Console allow-list, documentation, and regression coverage. Prefer an automated schema/allow-list consistency check so a documented action cannot remain unreachable.

## Release discipline

Push the current build as far as practical before creating another release.
A new build should represent a genuine emulator correction, meaningful rendering/gameplay improvement, coherent tooling/reliability change, or deliberate documentation/knowledge consolidation checkpoint.

Packaging is the final step, not the discovery step.
Validate the exact tree first, then package, then validate packaged contents.
Before packaging, reconcile any manual/local hotfixes used during validation back into the authoritative working tree, regression coverage, schemas, and documentation. A runtime PASS obtained with an unrecorded manual edit is not yet a releasable tree.
Never silently replace an already-issued versioned ZIP while keeping the same version/hash.

## Preserve cumulative capabilities

Do not regress established research infrastructure while fixing the current issue. Preserve, where relevant:

- native debugger / `chqctl` control;
- Workbench Research API and Script Console;
- CLI/config diagnostics;
- scenarios and canonical checkpoints;
- structured evidence and bundling;
- Forensic Timeline/history analysis;
- searchable docs/knowledge;
- Graphics Lab layered/per-sprite inspection;
- regression coverage.

Before building a new diagnostic, search for an existing API action, script, helper, viewer, or tracer that already answers the question.

## User interaction

When the next step is a run, give the exact copy/paste **one-line PowerShell command immediately**.
Batch independent experiments when safe; avoid many tiny rebuild/run cycles.
If evidence is already uploaded, inspect it before requesting another run.
Do not claim something was built, run, tested, validated, uploaded, or proven unless it actually was.

## Destructive/external actions

Normal repository work may read/edit project files, run project-local build/tests/diagnostics, and use required localhost services.

Ask before deleting user data/evidence outside disposable outputs, overwriting ROMs/canonical checkpoints, force-killing a process not clearly owned by ChaseHQ, installing software or changing machine-wide settings, pushing/publishing externally, or using credentials/secrets/paid services.

ROMs are user-supplied runtime inputs. Do not redistribute them in release packages.

## Definition of done

Match proof to the claim:

- source-only claim → static/code evidence may suffice;
- native behaviour → native runtime/test proof;
- Windows launcher/Workbench behaviour → Windows launcher/Workbench proof;
- rendering → deterministic visual/structured evidence plus relevant state/provenance;
- API/script change → schema parity plus focused regression;
- release → release validation and package integrity.

If the required proof cannot be run in the current environment, state the remaining gate explicitly.

---

This guide was introduced on the v0.66.9.0-RC2.4.3 project line. Keep it short and durable; put changing investigation status in the handover/state/roadmap documents.

## Split-environment source delivery

`Build-Debug.bat` and `Build-Release.bat` call `Prepare-Project.ps1`, which stages user-owned SDL and ROM inputs from `build-local.json`/environment settings. Their absence in the Linux source workspace does not block authorised source edits, available native tests, documentation or a clearly labelled candidate package. Preserve the staging scripts and local configuration. Never claim Windows/SDL or complete Script Console proof from Linux tests; keep the local promotion gate explicit. No user needs to repeat authorisation for routine source work already requested.

Release identity is authoritative in `src/version.h`; CMake, launchers and Workbench consume it. Keep feature/schema/knowledge catalogs in sync. Old/corrected renderer diagnostics are not independent shadow verification.
