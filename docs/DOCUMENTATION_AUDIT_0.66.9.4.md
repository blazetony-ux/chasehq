# Documentation audit - v0.66.9.4

Current release identity: **v0.66.9.4**. Direct promotion provenance: exact Windows-qualified `v0.66.9.4-RC3.1.3` archive SHA-256 `48534c0bd3d8e4a40987e65089b07046535976675ca2e36d0ebf3a6d9584383e`. Immutable source lineage remains promoted v0.66.9.3 SHA-256 `6da88889dc8214c6058644e44e4d29f6a2117c9f0e92d99c8d906c280fc3462e`.

## Qualification evidence incorporated

- Windows `chqdev.bat qualify --fresh`: PASS, 533940 ms.
- CTest: 9/9 PASS, including `sprite_layer_shadow_tests`, `hud_layout_tests`, `sprite_snapshot_assertions`, `cpu_bus_rom_tests` and `cli_evidence_tests`.
- Deterministic package + exact verify: PASS; promotion-source candidate SHA exactly `48534c0bd3d8e4a40987e65089b07046535976675ca2e36d0ebf3a6d9584383e`.
- Runtime startup/readiness: PASS; a transient native-status 504 was recovered by the lightweight retry exactly as designed.
- Focused run 001: PASS, 54 commands; finalization COMPLETE; bundle verifier 337 files / zero errors. It proves visible cross-format Stage-2 yellow-target rendering on the semantic player-car body, persistence through right/left steering state changes, live SPEED suppression after 120 accelerator-driven frames, Native sprite shadow parity and authentic cleanup.
- Full Regression run 002: PASS, 832 commands; finalization COMPLETE; bundle verifier 2608 files / zero errors.
- Authoritative run-history late gate: PASS.
- `promotable`: PASS.

## Promotion delta

The final v0.66.9.4 promotion changes release identity, current documentation, machine-readable build identity, focused-regression wiring and final-package metadata only. It intentionally does **not** change emulation/gameplay/rendering behavior from the exact qualified RC3.1.3 promotion source. RC3.1.3 remains packaged as historical promotion evidence; `research/scripts/regression/regression-v06694.chqscript` is the final focused release regression.

## Final current state

- Native sprite backend: **REPLACEABLE**, with Original retained as oracle/default.
- Cross-format semantic vehicle replacement: working; `player_car.body` can render the Stage-2 yellow target (OBJ-B64 map190) while preserving authentic gameplay ownership/state and remaining active through known player steering body-map changes. The current proof deliberately keeps the same rear-view target artwork while steering; directional replacement-family mapping is future work.
- Semantic HUD layout/show/hide: working; SPEED suppression includes its live dynamic value/unit destination region and remains hidden while authentic TC0100SCN character RAM continues updating.
- TC0100SCN charset export: complete 256-slot live atlas/raw export; four-state census established 220 observed invariant and 36 state-dependent slots.
- Stage-2 checkpoints: integrated (`stage2-gameplay-7500`, `stage2-enemy-10514`).
- chqdev v0.5.8: lightweight startup readiness, regression START/COMPLETE/FAILED console reporting, and authoritative history as a late fail-closed qualification gate.

## Immediate next work

Use v0.66.9.4 as the stable investigation baseline rather than cutting another build. Begin exploiting the tooling already shipped: semantic asset/object mapping and vehicle-family discovery; click/region graphics provenance for billboard identification; Text/Font Lab static-vs-dynamic glyph work; and Native Reconstruction Lab routine-contract / first-divergence work toward the first shadowed 68000 gameplay routine. Rebuild only for a genuine tooling blocker or a coherent implementation milestone.

## Authoritative release process

`chqdev` is the de facto and authoritative build/release process. Fresh Windows qualification is `chqdev qualify --fresh`; there are zero tolerated CTest failures and any CTest failure is fatal. Packaging and verification are deterministic exact-archive gates, followed by focused regression, Full Regression, authoritative run-history and promotable. PowerShell/BAT and other shell entry points are compatibility surfaces only.

## Final-package identity rule

The promotion SHA above belongs to the exact Windows-qualified **RC3.1.3 promotion source**, not to the regenerated final v0.66.9.4 archive. Because final identity/documentation/focused-regression promotion changes archive bytes, the exact final archive must receive one final `chqdev.bat qualify --fresh` closure before it is treated as the immutable downloadable v0.66.9.4 release. No source edits should occur between final packaging and that closure.
