# Post-v0.48 Conversation Completeness Audit

This document is the final consolidation audit of the design discussion that followed delivery of **v0.48.0 — Interactive Debugging Foundation** on 23 September 2026. Its purpose is to make the project repository, rather than chat history, the durable source of truth for the decisions and ideas from that discussion.

This is a design/state record, not a promise that every item will be implemented in the next release. The roadmap remains evidence-led and uses flexible semantic-style versioning.

## Chronology recovered from the discussion

### v0.48.0 delivery (~07:29 UTC)

The Interactive Debugging Foundation established continuous running by default, the F1 overlay, pause/step/fast-forward style controls, timer controls, quick checkpoint handling, screenshot/diagnostic/export facilities, live sprite inspection and the existing CLI path. The subsequent discussion was about turning that foundation into a reusable investigation platform rather than adding isolated debugging conveniences.

### Direct frame navigation (~07:52 UTC)

A request was made for the ability to go directly to a specific frame, including useful backward navigation.

The agreed direction was:
- forward seek executes deterministically to the requested frame;
- backward seek restores a suitable earlier checkpoint and deterministically replays to the requested frame;
- rolling checkpoints avoid having to replay from frame zero;
- example checkpoint cadences of **60 / 120 / 300 frames** were discussed as possibilities, not fixed policy;
- navigation should later generalise beyond frame number to useful investigation landmarks such as bookmarks, previous/next events, patches, collisions, checkpoints and particular writes/changes;
- a CLI/config equivalent such as `--goto-frame`/seek must exist rather than making navigation an SDL-only facility.

### Disk-backed checkpoints (~08:08 UTC)

The explicit preference was for rolling checkpoints to be **dropped to disk rather than retained only in memory**. This led to a two-level concept:
- disk-backed rolling checkpoints for the current investigation/session;
- a curated checkpoint corpus retained with project investigation material for known useful states.

### Formal roadmap acceptance (~08:12 UTC)

Disk-backed rolling checkpoints plus the curated checkpoint corpus were accepted as a formal roadmap direction, not merely a brainstorming possibility.

### Reproducible investigation infrastructure (~08:14 UTC onward)

The discussion broadened from frame seeking to infrastructure that removes repetitive troubleshooting work and makes experiments reproducible and resumable.

Recovered items:
- bookmarks for important frames/events;
- evidence-backed named game-state variables/concepts rather than repeatedly working only with raw addresses;
- baseline-versus-current state comparisons;
- restore/replay comparisons;
- **Repeat Last Experiment**: restore the same starting state, apply the same intervention, replay the same interval and collect the same evidence;
- one-click regression captures;
- state signatures for recognising equivalent machine/game states;
- reversible hypothesis toggles;
- compact/minimal evidence bundles containing the useful diagnostic material rather than indiscriminate huge logs;
- resumable investigation/session files using the working extension **`.chqsession`**.

A session was discussed as potentially retaining the starting checkpoint/scenario, target frame/event/bookmark, watches, trace settings, experiment/patch definitions, hypothesis toggles, selected object/known variable, relevant UI state and evidence/output configuration. The objective is to reopen an investigation later without rebuilding it from chat notes or shell history.

### Live Experiment Console (~08:28–08:33 UTC)

The ability to make controlled live changes while the game is running was strongly endorsed as a high-utility direction.

The intended progression is deliberately conservative:

**live parameters -> constrained runtime rules -> memory/ROM patches -> reusable presets**

Recovered requirements:
- interventions should be explicit and instantly reversible where practical;
- every change should be logged with timing/context;
- experiments should be savable in session/config form;
- useful experiments should be promotable to reusable presets/regression cases;
- deterministic CLI/config equivalents should be exportable/available;
- interactive UI and CLI/config automation must drive the same underlying experiment model;
- arbitrary unsafe runtime mutation is not the starting point: known variables and constrained, auditable rules come first.

### Shared debugging architecture

The discussion converged on a shared controller/API rather than separate implementations for every frontend. The working architectural concept is a **DebugController-style** layer coordinating navigation, checkpoints, known-state variables, experiments, bookmarks and evidence capture for:
- SDL interactive UI;
- CLI/config automation;
- possible later external/local Debug Studio tooling (for example localhost or named-pipe transport).

The preferred investigation pipeline is:

**Scenario -> Checkpoint/Navigation -> Experiment -> Observation -> Evidence**

Scenario setup remains separate from tracing/observation. Example scenario names already established in the project direction are `boot`, `service-mode`, `stage1-gameplay`, `stage1-driving` and `crash-test`. This same parameter-driven diagnostic philosophy is intended to extend to controls/input, audio, timing, sound CPU, device communication and other subsystems rather than spawning one-off diagnostic builds.

## Known-state / semantic registry

The debugger should progressively expose evidence-backed concepts such as race timer, stage/progress, player state, speed, lateral road position, collision state, gear and continue state when those mappings are established. Provisional mappings must remain visibly provisional; friendly names must not hide uncertainty.

This semantic layer is also what allows sessions, experiments, bookmarks and later automation to become more durable than hard-coded raw addresses.

## Versioning / roadmap decision

The roadmap is directional rather than a rigid sequence of promised version numbers. Use normal semantic-style `0.minor.patch` numbering:
- patch releases can consolidate documentation and low-risk refinements;
- meaningful capabilities can receive a new minor version when warranted;
- evidence and implementation dependencies determine actual ordering.

## Relationship to multi-machine / multi-game architecture

The post-v0.48 retrieval strongly confirms the debugger/checkpoint/session/experiment discussion above. The broader multi-machine, render-abstraction and reusable-arcade-core direction is preserved separately in `MULTI_MACHINE_ARCHITECTURE.md` because that architectural thread spans the wider roadmap discussion and is not safely attributable solely to the post-07:29 v0.48 time window.

The important integration rule is nevertheless explicit: new debugger infrastructure should be **machine-aware, not machine-bound**. Generic navigation/checkpoint/session/experiment/evidence machinery belongs in reusable infrastructure; Chase H.Q.-specific named state, scenarios, semantic events, safe parameters and device inspectors belong behind machine/game registries or adapters.

## Completeness matrix

| Recovered topic | Canonical documentation |
| --- | --- |
| Forward goto/seek | `INTERACTIVE_DEBUGGER_DIRECTION.md`, `ROADMAP.md` |
| Backward checkpoint + deterministic replay | `INTERACTIVE_DEBUGGER_DIRECTION.md`, `ROADMAP.md` |
| 60/120/300-frame example cadence | `INTERACTIVE_DEBUGGER_DIRECTION.md`, this audit |
| Disk-backed rolling checkpoints | `INTERACTIVE_DEBUGGER_DIRECTION.md`, `ROADMAP.md` |
| Curated checkpoint corpus | `INTERACTIVE_DEBUGGER_DIRECTION.md`, `ROADMAP.md` |
| Bookmark/event/patch/collision/write navigation | `INTERACTIVE_DEBUGGER_DIRECTION.md`, this audit |
| Scenario/trace/experiment separation | `INTERACTIVE_DEBUGGER_DIRECTION.md`, this audit |
| `.chqsession` resumable investigations | `INTERACTIVE_DEBUGGER_DIRECTION.md`, `ROADMAP.md` |
| Named/evidence-backed game state | `INTERACTIVE_DEBUGGER_DIRECTION.md`, `ROADMAP.md` |
| Baseline/current and restore/replay comparison | `INTERACTIVE_DEBUGGER_DIRECTION.md`, `ROADMAP.md` |
| Repeat Last Experiment | `INTERACTIVE_DEBUGGER_DIRECTION.md`, `ROADMAP.md` |
| One-click regression capture | `INTERACTIVE_DEBUGGER_DIRECTION.md`, `ROADMAP.md` |
| State signatures | `INTERACTIVE_DEBUGGER_DIRECTION.md`, `ROADMAP.md` |
| Reversible hypothesis toggles | `INTERACTIVE_DEBUGGER_DIRECTION.md`, this audit |
| Minimal evidence bundles | `INTERACTIVE_DEBUGGER_DIRECTION.md`, `ROADMAP.md` |
| Live Experiment Console | `INTERACTIVE_DEBUGGER_DIRECTION.md`, `ROADMAP.md` |
| Parameters -> rules -> patches -> presets progression | `INTERACTIVE_DEBUGGER_DIRECTION.md`, this audit |
| Instant revert / logging / session save / CLI parity | `INTERACTIVE_DEBUGGER_DIRECTION.md`, this audit |
| Shared DebugController-style engine | `INTERACTIVE_DEBUGGER_DIRECTION.md`, this audit |
| Scenario -> Navigation -> Experiment -> Observation -> Evidence | `INTERACTIVE_DEBUGGER_DIRECTION.md`, this audit |
| Flexible semantic-style roadmap/versioning | `INTERACTIVE_DEBUGGER_DIRECTION.md`, `ROADMAP.md` |
| Machine-aware rather than Chase-H.Q.-bound debugger | `MULTI_MACHINE_ARCHITECTURE.md`, this audit |
| Reusable arcade/device/machine/game architecture | `MULTI_MACHINE_ARCHITECTURE.md`, `ROADMAP.md` |
| Semantic render / alternative asset direction | `MULTI_MACHINE_ARCHITECTURE.md`, `ROADMAP.md` |
| Avoid premature framework refactor | `MULTI_MACHINE_ARCHITECTURE.md`, `ROADMAP.md` |

## Preservation rule going forward

When a future chat produces an accepted architecture or workflow decision, update the relevant project design document and `ROADMAP.md` in the next build. Chat history should be useful context, but it should no longer be the only place where project-critical decisions live.
