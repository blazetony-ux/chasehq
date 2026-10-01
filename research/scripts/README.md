## v0.66.9.0-RC1 reusable game-research scripts

The script library now carries the turbo/HUD timeline, exact tile-writer, text-plane neighbourhood, gameplay-to-HUD writer and tile-character inspection workflows, plus a reversible Game Lab turbo/speed demo and focused regression. These are retained because they generalize beyond a one-off chat probe.

# ChaseHQ Research Script Library — v0.65.0

The library is organised by intent so scripts are easy to distinguish before opening them.

- `regression/` — living authoritative smoke/regression coverage. **Every new Script Console syntax or scriptable Research API feature must update/add regression coverage in the same build.**
- `examples/` — small teaching examples.
- `graphics/` — sprite, palette, image comparison and deterministic A/B workflows.
- `track/` — track telemetry/surveys; longer surveys live in `track/surveys/`.
- `ioc/` — IOC/input research.
- `gameplay/` — gameplay registry/state research.
- `evidence/` — capture/state/evidence packaging helpers.
- `diagnostics/` — disassembly/log/low-level diagnosis examples.
- `scenarios/` — reproducible setup/priming scripts.
- `experimental/` — hypotheses/unstable experiments not yet promoted to stable categories.

See `SCRIPT_CATALOG.md` for the generated catalogue and `docs/platform/current/RESEARCH_SCRIPTING_0.65.0.md` for authoritative syntax.

Prefer API-backed query functions (`getregions()`, `getsprites()`, etc.) over hard-coded lists whenever the API can authoritatively provide the collection.

## v0.66.7.9 recovery / research workflow

- `recovery/` — short preflight/release-gate scripts. Run these before expensive regressions or after packaging/Workbench changes.
- `gameplay/turbo-state-causal-validation.chqscript` — revalidates already-confirmed turbo state from a canonical checkpoint.
- `graphics/turbo-hud-before-during-after.chqscript` — primary next-research script; captures and compares HUD source/composition around real turbo activation.
- `gameplay/turbo-state-writer-trace.chqscript` — exact producer PC tracing when visual localization shows it is needed.
- `graphics/canonical-checkpoint-visual-survey.chqscript` — broad visual baseline across all canonical checkpoints.
- `gameplay/target-endlevel-state-survey.chqscript` — next-stage evidence capture without prematurely naming target-health semantics.

Full Regression is the final release gate, not the first diagnostic step. See `docs/RELEASE_PROCESS.md`.

## v0.66.8.0 Forensic Timeline

`timeline/` contains the record-once/replay-many workflow. Forensic Timeline v1 saves a deterministic checkpoint every frame plus the complete bus-write stream, supports script import/seek and reuses normal inspection APIs against historical frames. See `docs/FORENSIC_TIMELINE.md`.

## v0.66.9.0-RC2
v0.66.9.0-RC2 adds reusable turbo-animation, player-car shadow/order, and focused sprite-order/speed-semantics regression workflows.

v0.66.9.0-RC2.2 adds a loop-based multi-scene same-priority sprite-order validation workflow and focused default-order regression.
