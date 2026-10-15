# Documentation Audit - v0.66.9.5-RC3

Current release candidate: **v0.66.9.5-RC3**. RC3 is the deliberate **final native-promotion tooling pass** built directly on the exact `v0.66.9.5-RC2` candidate archive SHA-256 `3aaaebf55cac7cfa9150754556db42438083b52c2965fd51ce36cc5f2bdca0f7`. RC2 itself remains an unpromoted candidate until Windows qualification; RC3 supersedes it for further qualification rather than claiming RC2 promotion.

`chqdev` remains the de facto and authoritative build/release process. Promotion requires exact-archive Windows `chqdev.bat qualify --fresh`, zero tolerated CTest failures, deterministic archive packaging/verification, runtime readiness, focused regression, Full Regression, authoritative run-history and `promotable` PASS.

## Purpose: final tooling pass before native implementation

This release intentionally closes the broad tooling phase. After RC3 qualification, **tooling freeze** is the default policy: no new general Workbench/script/atlas features unless an active native implementation exposes a concrete blocker. The next milestone is native subsystem replacement, starting with sprites.

The promotion ladder remains:

`UNKNOWN -> OBSERVED -> MAPPED -> BEHAVIOUR UNDERSTOOD -> NATIVE-CANDIDATE -> SHADOW VERIFIED -> REPLACEABLE -> NATIVE`

RC3 adds the missing fail-closed machinery between **REPLACEABLE** and **NATIVE**.

## Native Promotion Framework

RC3 standardises one promotion workflow for independently reconstructed subsystems:

1. **Original** - authoritative/emulated oracle.
2. **Shadow** - Original remains displayed while the independent implementation is evaluated against the exact same live state.
3. **Native** - independent output is displayed; explicit compatibility fallback may still exist for research-only controls.
4. **Native-only** - Original fallback is hard-disabled. Unsupported states are counted as `native_only_blocked` rather than silently calling Original.
5. **Promotion corpus** - deterministic Stage 1 / Stage 2 coverage.
6. **Fail-closed assertion** - no mismatches, no blocked Native-only frames, no Original fallback.
7. Only then may the subsystem move from REPLACEABLE to **NATIVE**.

New Script/API surfaces:

- `native.progress`
- `native.promotion.status subsystem=sprites`
- `native.promotion.reset subsystem=sprites`
- `native.promotion.assert subsystem=sprites minShadowFrames=N minNativeOnlyFrames=N`
- `native.layer.capture subsystem=sprites name=...`
- `backend.sprite.set mode=native-only`

The framework is generic at the API/Workbench/maturity-model level. Only the sprite subsystem currently implements an independent Native-only backend; other subsystems report their current maturity and remain unsupported for promotion assertions until an independent implementation exists.

## Sprite Native-only hard-disable gate

`Native` and `Native-only` are intentionally different:

- `Native` may explicitly fall back to Original for slot-specific forensic presentation controls that the independent renderer does not yet implement.
- `Native-only` **never** falls back. If those controls are active, the frame is counted as `native_only_blocked` and Original is not called.

Backend status now exposes:

- `original_frames`
- `native_frames`
- `native_only_frames`
- `native_only_blocked`
- `original_fallbacks`
- `original_avg_us`
- `native_avg_us`
- `shadow_compare_avg_us`
- existing Shadow compare / mismatch / first-divergence counters

This makes the independence claim measurable instead of relying on appearance.

## First-divergence and same-state evidence

On the **first live Shadow mismatch**, the Video subsystem automatically writes a `native_promotion/first-sprite-mismatch_f<frame>` evidence directory containing:

- `original.png`
- `native.png`
- `diff.png`
- `manifest.json`

`native.layer.capture` exposes the same Original/Native/diff capture manually at the exact paused state without advancing emulation. This is the cross-backend state-locking primitive for sprite work.

## Canonical sprite promotion corpus

`research/scripts/graphics/native-promotion-sprites.chqscript` is the intended final sprite promotion run. It covers deterministic moving windows from:

- Stage 1 driving 2352
- Stage 1 gameplay 2064
- pre-contact 5587
- first physical contact 5588
- pre-damage 6333
- first damage 6334
- post-final-hit 9548
- end-level 9988
- Stage 2 gameplay 7500
- Stage 2 enemy 10514

The script first accumulates clean live Shadow comparisons, then repeats the same canonical corpus in Native-only mode and finishes with a fail-closed `native.promotion.assert`. It restores Original / authentic state in `finally`.

The release-focused RC3 regression performs a shorter Stage 1 + Stage 2 prove-off of the same contracts. Full Regression includes RC1.1, RC2 and RC3 tails.

## Workbench Native Progress matrix

Graphics Lab -> Native Reconstruction Lab now includes:

- **Native-only** backend control;
- manual **Original/Native diff capture**;
- a live **Native Progress** matrix populated from `research/knowledge/native-subsystems.json` plus live promotion counters;
- a fail-closed **Assert sprite promotion gate** control.

The static maturity model remains evidence-backed. Runtime counters do not silently rewrite project knowledge or promote a subsystem automatically.

## Performance telemetry boundary

RC3 records average microseconds for the live Original sprite draw, Native sprite draw and same-state Shadow compare. This is diagnostic telemetry only; no performance threshold is currently a promotion requirement. Correctness/independence remain the gates.

## Carried-forward RC2 findings

RC3 retains all RC2 work unchanged:

- direct-ROM Sprite / Vehicle Atlas from `b52-38.34` + OBJ-A/OBJ-B graphics;
- source/effective sprite inspection;
- player body maps 530/536;
- player shadows 598/600;
- target map 20;
- beacon maps 519/540/617 and palette 78;
- measured beacon descriptor staging -> live sprite pipeline latency;
- Run012 visual-only neighbours remain candidates, not silently confirmed semantic identities.

RC3 also retains RC1.1 fixes: semantic asset snapshot parity, map509, 32-bit fixed-point speed freeze, target-contact structured events, stale asset telemetry cleanup and Workbench backend bridge parity.

## Portability boundary

The Native Promotion Framework (maturity model, progress API, assertions, evidence concept, Workbench matrix) is intended to be reusable across games/hardware. The current sprite implementation is Taito Z / Chase H.Q. specific. The ROM Atlas remains explicitly `hardware=taito_z` / Chase H.Q. ROM-layout specific until another profile/adapter implements equivalent decoders.

## Next milestone after qualification

**No more broad tooling expansion.** The next project milestone is:

> Run the exact RC3 sprite promotion corpus on Windows/SDL. If Shadow remains exact and Native-only completes the canonical corpus with zero blocked frames and zero Original fallback, promote the sprite subsystem from REPLACEABLE to NATIVE and make the independent sprite renderer the first real native replacement milestone.

After sprites, implement an independent TC0100SCN text/HUD Shadow path using this same framework.

## Broad-tooling freeze

After exact-archive Windows qualification of **v0.66.9.5-RC3**, general research-tool expansion is frozen. New tooling is justified only by a concrete blocker encountered while implementing/promoting a native subsystem. The immediate native milestone is the sprite promotion corpus, followed by TC0100SCN text/HUD Shadow.
