# RC4.4 documentation and qualification audit

# RC4.4: bounded CPU-A projection shadow / optional REA evidence crosswalk

**Candidate:** `v0.67.0.0-RC4.4`, based on the qualified-but-not-promoted RC4.3.2 source. **Not promoted.** Windows exact-archive `chqdev.bat qualify --fresh` remains mandatory; Linux CPU-only evidence cannot qualify Windows SDL/Workbench. `chqdev` is the authoritative build/release process; BAT/PowerShell compatibility wrappers must not contain a parallel release workflow. Preserve LF/CRLF safety and zero tolerated CTest failures.

## Research closure through Run 029 (2026-10-10)

- Runs 018-022: verified CPU-A target projection lower inclusive `-0x180`, upper exclusive `+0x8000`; cache `0x101A80`, reference `0x101A00`, entry `0x434EE`, table choice `0x54100 + sign_extend16(((delta arithmetic >> 1) & 0xFFF8))`, lookup entry `0x4351C`. Original object-positive guard applies.
- Runs 023-025: target `0x10A080` longitudinal depth changes projected scale/Y and perspective-dependent screen X; `0x10A084` lateral changes X only at fixed depth, with quantisation sensitive to integer arithmetic. Writes to prepared sprite words by PCs `0x43828/0x4382A` are observed, **not** yet fully reproduced by native geometry.
- Run 026: failed solely from a stale checkpoint name; restored safely. Run 027: same cache/producer path at pre-contact, hit and post-final-hit; frame 2064 lacks the later projection stages. Actual post-final-hit checkpoint is `stage1-target-post-final-hit-9548.chqstate`.
- Runs 028-029: map 19 shares the controlled coordinate-projection path. Map 19 ignored changes at frame 9551 because of staging delay, **not** because it was unconnected; in the matched near-minus-0x400 run, software prepared X=210 at frame 9549, hardware copier `0x40540` wrote slot 54 X=210/map 19 at frame 9551, and rendered map 19 X=210 Y=127 size 81x81 at frame 9553. Slot identity varies. Four-frame time difference is specific to that replay, not universal.

## Implemented candidate scope

1. `src/projection_shadow.{h,cpp}` independently calculates original-68000-style clipping and table record selection. Opt-in `ChaseHQRuntime --native-projection-shadow` samples the original CPU-A instruction PC path (`0x434EE`, `0x4351C`, `0x40F5A`), reads cache/reference without writes, compares acceptance and A2 record pointers, and emits `native-projection-shadow.json` with matched, mismatched and unresolved counts. Disabled by default. No original instruction skipped, no geometry replacement, no side-effect mutation, no universal shadow parity asserted. Pure reference tests are `projection_shadow_tests`; real-case runtime comparisons require operator ROM/CHQPACK inputs and representative gameplay states.
2. Optional `chqdev rea crosswalk --static <reviewed-static.json> --census <rom-execution-pc-counts.tsv> --out <crosswalk.json>` imports an explicit ROM SHA256, image base and candidate function ranges, then intersects static findings with original observed CPU-PC counts. The initial known target is CPU-A `0xA112`; no REA installation, network access, Ghidra project mutation or ROM upload is performed. **Direct REA/Ghidra raw 68000 ROM integration on Windows remains unimplemented and blocked by current upstream Windows PE-only support**; an explicit reviewed export is required, and all function boundaries retain stated uncertainty.
3. `tools/sprite_pipeline_correlate.py` is an optional stdlib-only offline model that cross-references prepared/hardware/presented events by original map, X and temporal order, without fixing a slot; `research/evidence/projection-pipeline-run029.json` provides the explicitly transcribed Run 029 example and a unit fixture. Ambiguous same-map/same-X ownership, complete Workbench automatic export, and full native sprite projection remain deferred.

## Native maturity and limits

- CPU-A target **clip/index**: independently implemented **SHADOW CANDIDATE**, offline reference fixtures verified; live multi-checkpoint **SHADOW VERIFIED** maturity **NOT** claimed; `Replace` remains forbidden.
- Depth-to-size/Y and lateral fixed-point arithmetic: **NATIVE-CANDIDATE research only**, not independently implemented or shadow verified. Prepared/hardware correlator is a research aid, not a generic emulator rewrite.
- REA: first **optional adapter crosswalk proof of concept** implemented; native analysis provider and raw 68000 import integration pending.
- Current baseline RC4.3.2: Windows `qualify --fresh` previously passed on user's machine (13/13 CTest and full regression), but that archive is not promoted. RC4.4 evidence is separate and must not inherit qualification.
- Existing Go `TestFullRegressionConsolidatedCommandBudget` (<850 structural expanded commands, currently ~1176) is an **open failing Go test**; do not delete unique coverage or raise its threshold to disguise the failure. Windows qualification must be run on the exact final RC4.4 ZIP and independently report its own gates.

## Reproduction / release gates

```
# Optional CPU-only observability using locally owned content, e.g. from a checkpoint-capable environment
ChaseHQRuntime --content-pack <user-pack.chqpack> --native-projection-shadow --frames 120 --logs <evidence-folder>
# Optional offline crosswalk, after reviewing raw ROM base/sha independently
.\chqdev.bat rea crosswalk --static <static-export.json> --census <rom-execution-pc-counts.tsv> --out <crosswalk.json>
# Authoritative release qualification from a fresh extraction of this EXACT ZIP on Windows
.\chqdev.bat qualify --fresh
```

Qualification contract: doctor and trust repair, clean Debug build, **zero failed CTests**, deterministic repeated package/verify, runtime readiness, focused regression, Full Regression, authoritative history, promotable. Source/SDL/ROM/content are user-owned and not bundled. A failure leaves candidate **unqualified**. Do not promote without explicit approval. Preserve release-process and governance checks for LF/CRLF.
