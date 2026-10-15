# Open investigations - v0.66.9.4-RC1

Settled items should not be reopened without contradictory evidence. Sprite record/placement semantics, placed-source shadow parity, target health/damage, post-defeat handoff, Turbo HUD selection, Brake/lamp mechanism, score display path, exact road-side bands, physical grade sign and race-timer semantics are closed at their documented scope.

Priority open work:

1. **Police beacon:** closed at primary lifecycle scope. CPU-A `0x101854` is the authoritative visible/deployment phase; RC1 adds an explicit experimental override. Reopen only for contradictory evidence or a new replacement-backend requirement.
2. **Target pre-copy provenance:** trace object state into the prepared sprite record upstream of the generic 0x40500-0x40576 bulk copier.
3. **HUD/text producer provenance:** resolve the tick-driven timer text writer and exact score/Turbo cell producer chains.
4. **Collision object semantics:** classify interaction type/object-reference data, especially Run020 type 0001 / `0x100420` object reference.
5. **Audio:** interpret command-interface traffic, then map/implement sound CPU/chips/mixer/PCM. Current support is command-event instrumentation only.
6. **Stage-1 graph/minimap:** convert proven selector/fork topology into an authoritative directed graph and position overlay.
7. **Residual graphics fidelity:** TC0100SCN flipped text/origin and remaining palette/compositor edge cases only where reproducible.
8. **Native progression:** move the SHADOW VERIFIED placed sprite-source layer toward an explicit experimental replacement backend and REPLACEABLE criteria; then shadow TC0100SCN/compositor/road.
9. **Coverage expansion:** later stages/transitions and full-game deterministic regression.
