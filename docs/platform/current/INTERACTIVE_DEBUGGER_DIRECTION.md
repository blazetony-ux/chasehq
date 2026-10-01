# Interactive Debugger Direction — post-v0.48 design record

This document preserves the design decisions made after the v0.48.0 Interactive Debugging Foundation was produced. It is intentionally a direction/architecture record rather than a rigid release schedule.

## Core principle

Build reusable investigation infrastructure once, then drive it with parameters, scenarios, checkpoints, sessions, presets and experiments. Avoid producing a succession of one-off diagnostic builds for individual hypotheses.

Interactive UI, CLI/config automation and future external tooling should converge on the same underlying debugging/experiment engine. The preferred workflow remains:

**interactive discovery → reproducible session/config → automated regression evidence**

## Deterministic navigation

Frame navigation is a formal roadmap capability.

- Forward navigation may deterministically execute to a requested frame.
- Backward navigation should restore the nearest suitable earlier checkpoint and deterministically replay to the target.
- Rolling checkpoints should make reverse navigation practical without replaying from frame zero.
- Checkpoints should preferentially be disk-backed so long investigations do not depend on large in-memory history.
- A curated checkpoint corpus may be retained with project investigation material for known useful states.
- Navigation should eventually generalise from frame numbers to bookmarks and semantic events: previous/next event, patch, collision, write/change, checkpoint or marked frame.
- CLI equivalents such as `--goto-frame` / seek-style commands are required; navigation must not be SDL-UI-only.

Checkpoint intervals such as 60/120/300 frames were discussed as useful examples, but interval policy remains configurable/experimental rather than fixed.

## Reproducible scenarios remain separate from observation

Scenario setup and tracing are distinct concerns. A scenario establishes a known game state; trace/debug presets decide what to observe; experiments decide what to change.

Examples of intended scenarios include:

- `--scenario boot`
- `--scenario service-mode`
- `--scenario stage1-gameplay`
- `--scenario stage1-driving`
- `--scenario crash-test`

This separation should be preserved as controls, audio, timing, sound CPU and device-communication investigations are added.

## Investigation sessions

A resumable investigation/session format (working extension `.chqsession`) is a formal direction item. A session should be able to capture enough information to reproduce an investigation, potentially including:

- starting checkpoint or scenario;
- target frame/event/bookmark;
- watches and trace settings;
- experiment/patch definitions and hypothesis toggles;
- selected sprite/known variable and relevant UI state;
- evidence/output configuration.

The goal is to resume an investigation later without reconstructing its setup from chat notes or command history.

## Bookmarks, state signatures and comparisons

Planned reusable capabilities:

- bookmarks for interesting frames/events;
- state signatures to recognise equivalent machine/game states;
- baseline-vs-current comparisons;
- restore/replay comparisons;
- one-click regression captures;
- repeat-last-experiment from the same start state and observation window;
- compact evidence bundles containing the minimum useful material for analysis.

## Known-state registry

Continue moving from raw addresses toward named, evidence-backed game-state concepts. The race timer is the first example. As reverse engineering establishes them confidently, expose concepts such as stage/progress, player state, speed, lateral road position, collision state, gear, continue state and related controls through the same registry/API.

Names must not conceal uncertainty: provisional mappings should remain clearly marked until evidence is strong enough to treat them as confirmed.

## Live Experiment Console

The interactive debugger should grow into a controlled Live Experiment Console. Capability should progress conservatively:

**live parameters → constrained runtime rules → memory/ROM patches → reusable presets**

Requirements:

- interventions are explicit and instantly reversible where practical;
- every change is logged with timing/context;
- experiments can be saved into a session/config;
- useful experiments can graduate into reusable presets/regression cases;
- CLI/config equivalents exist for deterministic reruns;
- the UI and CLI drive the same underlying experiment model rather than separate implementations.

Arbitrary unsafe runtime mutation is not the starting point. Prefer known variables and constrained, auditable interventions first.

## Debug-controller architecture

A shared internal DebugController-style API remains the preferred architecture for SDL UI, CLI/config automation and any future localhost/named-pipe Debug Studio. This should own or coordinate navigation, checkpoints, known-state variables, experiments, bookmarks and evidence capture.

## Versioning

Use flexible semantic-style `0.minor.patch` numbering. The roadmap is directional, not contractual. Patch releases may consolidate documentation and low-risk refinements; meaningful new capabilities can earn a new minor version whenever warranted by the evidence and implementation state.
