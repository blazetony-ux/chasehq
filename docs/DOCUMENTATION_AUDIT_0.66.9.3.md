# Documentation audit - v0.66.9.3

Current release identity: **v0.66.9.3**. Direct promotion provenance: exact Windows-qualified `0.66.9.3-RC5` archive SHA-256 `19b7ed48edae98fd99f660ef56a949573050cabc2958fcdba2833a7888103961`.

## Qualification evidence incorporated

- Windows `chqdev.bat qualify --fresh`: PASS, 449873 ms.
- CTest: 8/8 PASS, including `sprite_layer_shadow_tests`, `cpu_bus_rom_tests` and repaired `cli_evidence_tests`.
- Deterministic package + exact verify: PASS; candidate SHA exactly `19b7ed48edae98fd99f660ef56a949573050cabc2958fcdba2833a7888103961`.
- Runtime restart/readiness: PASS.
- Focused run 001: PASS, 49 commands, 12085 ms; bundle verifier 17 files / zero errors; bundle SHA `e23091feaecba7b468b93f4b24ee8ae279b4eb16892cac54196b51527ba9e6c1`.
- All eight canonical placed-source shadow comparisons: 76,800 pixels each, zero pixel mismatches, zero owner mismatches.
- Full Regression run 002: PASS, 778 commands, 252451 ms; bundle verifier 2280 files / zero errors; bundle SHA `80471a6cd5d5ea1b4431bc7ad5b68f1c39f0012656fa87a18ccfe7c3d33922cf`; final safe player-gameplay frame 2064 paused.
- `promotable`: PASS.

## Current documentation reconciled

The audit updates the release/current docs, handover, roadmap, project state, native maturity, discovery records, machine-readable documentation/handbook, semantic objects, next-steps API data, audio capability wording, script catalog and final focused regression. Stale current claims corrected include: RC5 qualification pending; v0.66.9.2 still current; map181/pal75/77 player identity; Brake at 0x100303/P3:10; Turbo-HUD rendering still open; generic bit-based road-state wording; positive/negative grade provisional; placed sprite layer not yet SHADOW VERIFIED; and RC4-only audio build wording.

Historical RC/older-release documents and scripts remain historical provenance and are not rewritten to pretend the final release originally produced those findings.

## Final current state

`src/version.h`, `chqdev.json`, configured machine-readable build identities, current operational docs and final focused regression identify v0.66.9.3. `research/scripts/regression/regression-v06693.chqscript` is the final focused release regression; RC5 remains historical promotion evidence. `sprite-4bpp-decoder` and `sprite-placement-ownership` are SHADOW VERIFIED at the canonical corpus scope.

## Next work

Use v0.66.9.3 for diagnosis: police-beacon producer, target pre-copy producer, HUD producer provenance, collision object semantics, sound-command interpretation and Stage-1 graph/minimap. Rebuild only for a real blocker or coherent implementation milestone.

The permanent exact-archive release contract remains `chqdev.bat qualify --fresh`, zero tolerated CTest failures, deterministic exact archive verification, focused regression, Full Regression and `promotable`.

## Final-package identity rule

The embedded promotion SHA belongs to the exact Windows-qualified **RC5 promotion source**, not to the final v0.66.9.3 archive. The final release archive is regenerated after identity/documentation/focused-regression promotion and therefore receives its own manifest and SHA-256. Because those edits change archive identity, the exact final archive must receive one final `chqdev qualify --fresh` closure before it is treated as the immutable downloadable release; no diagnosis should modify the tree between packaging and that closure.

## Package-manifest promotion guard

The final audit also moved package status/source-provenance/feature prose out of RC-specific `chqdev` source constants into release-owned `chqdev.json` fields. `chqdev preflight` now validates a `package-metadata-contract`, preventing a final release archive from retaining stale candidate/pending-promotion wording.
