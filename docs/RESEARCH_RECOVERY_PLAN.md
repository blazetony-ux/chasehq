## v0.66.9.0-RC1 note

Recovery/release infrastructure is no longer the active task. The current candidate is driven by concrete Chase H.Q. research needs: tile inspection and reversible turbo/speed experiments. Use timeline evidence first and avoid replaying already-captured events.

# Research recovery after v0.66.8.0

The v0.66.8.0 platform/release gate is complete. Use this plan as historical/reusable guidance, but the current immediate direction is substantive game research described in `ROADMAP.md`.

# Research recovery plan after v0.66.7.8 release hardening

The recent prove-off work strengthened the platform but consumed time that should now be redirected into Chase H.Q. research. This plan deliberately uses the tooling already built instead of adding another large tooling tranche.

## Important correction to the current research framing

Turbo gameplay state is **not** unknown. Current authoritative project sources already mark the following as confirmed:

- processed turbo input: CPU-A `0x100303` bit 0;
- turbo active: CPU-A `0x10040F` bit 1;
- turbos remaining: CPU-A `0x1003A2` 16-bit count;
- turbo active-duration counter: CPU-A `0x100414` 16-bit frame counter;
- tested Stage 1 expiry threshold: `0x00D2` / 210 emulated frames;
- IOC Port 3 `0x01` is turbo and `0x20` is accelerator in the confirmed Stage 1 mapping.

The open problem is therefore **the HUD/rendering representation and path**, not discovery of the basic gameplay state. Older/current text that described turbo-active/turbos-remaining as unresolved was stale and is corrected in this build.

## Priority 1 - turbo/HUD source localization

Run these in order:

1. `recovery/research-session-preflight.chqscript`
2. `gameplay/turbo-state-causal-validation.chqscript`
3. `graphics/turbo-hud-before-during-after.chqscript`
4. `gameplay/turbo-state-writer-trace.chqscript` only if writer provenance is needed after visual localization

The graphics script produces reconstructable snapshots before/during/after a genuine turbo activation and compares `HUD_TURBO` in Normal, HUD-only and HUD-hidden modes.

Interpretation:

- HUD-only changes: the TC0100SCN text/HUD source is participating; inspect source tiles/characters and their writer path.
- Normal changes but HUD-only does not: investigate sprite/road/compositor contribution rather than the text source.
- No expected change in HUD-only despite confirmed gameplay-state transition: inspect the text RAM/tile/character-data producer path and historical visible-area evidence.

Historical documentation records that turbo HUD tile references existed in TC0100SCN rows 29/30 and that a visible-area correction previously made the three indicators visible. Treat that as an important historical lead to revalidate against the current renderer, not as permission to assume the current fault is identical.

## Priority 2 - authoritative writer/provenance path

`gameplay/turbo-state-writer-trace.chqscript` captures exact writer PCs for:

- processed turbo input;
- turbos remaining;
- turbo active + timer range.

Use `cpu.disassemble` on the returned writer PCs and connect the producer/consumer path to the HUD source only after the visual source is localized.

Do not create a new semantic field for values that are already confirmed in `games/chasehq/gameplay-registry.json`.

## Priority 3 - target health / defeat / end-level transition

Use `gameplay/target-endlevel-state-survey.chqscript` to compare the canonical near-destruction and end-level checkpoints. It intentionally does not assign a target-health meaning to unknown values; it captures coherent state and visual evidence first.

Once candidate state changes are narrowed, use generic memory writer tracing to find producer PCs and promote only reproduced semantics.

## Priority 4 - track branch/topology naming

The track recorder and known Stage 1 chain are already proven. Continue naming physical semantics and branch/topology only after the turbo/HUD and target/end-level investigations have concrete evidence.

## Priority 5 - enhancement work after fidelity blockers

Sprite upscale/replacement packs and possible hybrid 3D presentation remain valid longer-term goals, but they should stay separate from the original/pixel-authentic canonical renderer and should not displace current fidelity research.

## Useful broad evidence script

`graphics/canonical-checkpoint-visual-survey.chqscript` captures layered snapshots and coherent state from all four canonical checkpoints. Use it after renderer changes or when a visual regression needs to be checked across more than one gameplay state.

## Current recovery/research direction
The platform is back in game-facing research. Prefer existing `.chqtimeline` recordings for graphics diagnosis; only reproduce gameplay when the required state was not captured. The current target is the player-car/under-car-effect same-priority sprite ordering.

> RC2.2 note: Chase H.Q. native same-priority sprite ordering defaults to `lower-slot`, matching MAME reverse sprite-RAM traversal; `higher-slot` remains an explicit diagnostic override.
