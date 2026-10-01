# Documentation index — v0.66.9.0-RC2.9

**Current source candidate:** RC2.9. **Latest Windows/SDL-proven baseline:** RC2.8. RC2.9 promotes the causally confirmed target-health/damage mechanism, bundles the 5587/5588 and preferred 6333/6334 target checkpoint pairs, adds One-hit Target Kill, SDL Pause/Break pause, responsive Dashboard SDL controls, and a configurable run-frames timeout. RC2.9 still requires its own Windows focused regression and Full Regression before promotion.

Start with `PROJECT_STATE.md`, `ROADMAP.md`, `HANDOVER.md`, `API_REFERENCE.md`, and `SCRIPT_LANGUAGE_REFERENCE.md`.

## Historical candidate notes

## v0.66.9.0-RC2.4 active candidate

RC2.4 corrects TC0100SCN BG0/BG1/TEXT Y-scroll source-coordinate sign and restores neutral BG presentation offsets. No API or script-language surface changes. Windows/SDL canonical-frame and regression proof are pending; proven baseline remains v0.66.8.0.

## v0.66.9.0-RC1 development candidate

Historical RC1 note: at that point v0.66.8.0 was the proven baseline and the candidate added Game Lab/tile-inspector work. This section is retained for release history; current RC2.9 state is described above.

# ChaseHQ-Native documentation

**Current proven baseline: v0.66.8.0 — Forensic Timeline v1 (2026-09-29).**

Start with `PROJECT_STATE.md` for current status, `ROADMAP.md` for next research priorities, `FORENSIC_TIMELINE.md` for record-once forensic workflows, `API_REFERENCE.md` / `SCRIPT_LANGUAGE_REFERENCE.md` for interfaces, `RELEASE_PROCESS.md` for release discipline, and `CHATGPT_HANDOVER_PROMPT.md` for cross-conversation continuation. Historical documents remain preserved for provenance.

# Documentation map

The documentation is deliberately split into three bodies. Do not mix them again.

## `docs/chasehq/` — what we know about Chase H.Q.
Game/hardware observations, confirmed/rejected hypotheses, memory maps, graphics findings, canonical checkpoints and reproducible discovery records. A reader interested in *how Chase H.Q. works* should start here.

## `docs/platform/` — documentation for the research tool
ChaseHQ-Native Research Workbench/API implementation, CLI/API behaviour, architecture, release notes and build validation. A reader interested in *how to operate or develop the tool* should start here.

## `docs/methods/` — reusable reverse-engineering methodology
Game-independent research recipes distilled from successful and failed Chase H.Q. investigations. These are intended to transfer to other games/hardware.

`PROJECT_STATE.md` and `ROADMAP.md` remain at this level as short project-wide navigation documents. Historical material is retained under the relevant `history/` folders for provenance.

## Built-in handbook
From v0.66.7.5 the Workbench Docs / Knowledge tab is a real user manual, not just a machine-readable knowledge browser. Authored content lives in `research/knowledge/handbook.json`; the complete API and scripting references are generated from `research/schema/`, while evidence-backed project findings/glossary remain in `research/knowledge/documentation.json`.


## v0.66.7.6 consolidation
The handbook platform now indexes Project Knowledge article bodies in both Workbench and server-side search, exports the full article model, suppresses empty reference sections, and groups Project Knowledge by proof/status. Release validation checks the consolidated documentation path.

## Current process documents

- `RELEASE_PROCESS.md` — canonical build, packaging, staged prove-off, one-run/one-bundle and promotion rules.
- `RESEARCH_RECOVERY_PLAN.md` — immediate post-release Chase H.Q. research sequence, beginning with turbo/HUD source localization.
- `CHATGPT_HANDOVER_PROMPT.md` — authoritative cross-conversation continuation state.
- `POSTMORTEM_0.66.7.8_RELEASE_PROVEOFF.md` — technical record of the failures that motivated the current release discipline.


Release evidence summary: `RELEASE_PROOF_0.66.8.0.md`.

## Current candidate
v0.66.9.0-RC2; see project state, roadmap, API/script changelogs and release notes for the synchronized research state.
