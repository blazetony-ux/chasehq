# Multi-machine / multi-game architecture direction

This document preserves the longer-term architecture discussion around making ChaseHQ-Native useful beyond a single Chase H.Q. executable. It is a direction record, not a commitment to refactor working code prematurely.

## Goal

Chase H.Q. remains the reference target and the project should continue to become a correct, useful Chase H.Q. implementation first. At the same time, new infrastructure should avoid unnecessary Chase-H.Q.-specific assumptions when a clean reusable boundary is already understood.

The long-term objective is a reusable native arcade framework in which related and eventually different hardware/games can be brought up by combining reusable devices, machine/board definitions, ROM/game descriptions and game-specific knowledge instead of cloning the whole Chase H.Q. implementation.

Potential future targets discussed include other arcade games such as OutRun and Super Hang-On. They are architectural examples, not near-term compatibility promises: their hardware differs and would require their own adapters/devices and reverse-engineering work.

## Intended layering

1. **Arcade/runtime core**
   - execution/scheduling primitives;
   - lifecycle and frame control;
   - save/checkpoint plumbing;
   - input/event plumbing;
   - common diagnostics, tracing, sessions and experiment interfaces.

2. **Reusable hardware/device implementations**
   - CPU cores and wrappers;
   - buses/address maps;
   - RAM/ROM/shared-memory devices;
   - interrupt/timer/watchdog facilities;
   - graphics devices (tilemaps, roads, sprites/motion objects, palettes/priority);
   - sound CPUs/chips and inter-device communication;
   - input/I/O devices.

3. **Machine/board definitions**
   - instantiate devices;
   - describe clocks and relationships;
   - map address spaces;
   - connect interrupts, I/O and communication paths;
   - define video composition and timing appropriate to that board.

4. **Game/ROM definitions**
   - ROM regions/files, loading/interleaving and validation;
   - game/region/revision metadata;
   - game-specific inputs/configuration where required;
   - no redistribution of copyrighted commercial ROM/assets.

5. **Semantic game knowledge**
   - evidence-backed named state such as race timer, stage/player state and known transitions;
   - scenarios, bookmarks and investigation presets;
   - game-specific experiment definitions and regression expectations.

6. **Presentation / render abstraction**
   - preserve original hardware-faithful rendering as the reference path;
   - progressively expose semantic render objects where evidence supports them;
   - allow alternative renderers/asset packs without changing game logic or CPU-visible machine state.

## Asset and renderer direction

A longer-term concept discussed was:

`Arcade Core -> semantic object identification -> Render Abstraction Layer -> renderer -> assets`

The renderer side could eventually support:
- original/hardware-faithful output;
- replacement artwork;
- higher-resolution presentation;
- potentially 3D/alternative presentation.

An intermediate native asset representation can sit between hardware-specific extraction/adapters and presentation. Hardware-specific adapters should translate the source game's tile/sprite/road concepts into reusable semantic/native forms rather than forcing every renderer to understand each original board directly.

This is deliberately future-facing. Hardware-faithful Chase H.Q. remains the correctness baseline. Alternative assets/rendering must not become a shortcut around understanding the original hardware.

Commercial game assets/ROMs are not to be bundled with the project; user-supplied/local assets and legally distributable replacements are the intended model.

## Debugger architecture must be machine-aware, not machine-bound

The v0.48+ debugger work should be reusable across machines even though Chase H.Q. is currently its only client.

Generic concepts:
- pause/run/step/seek;
- checkpoint index and deterministic replay;
- memory/register inspection;
- trace/event streams;
- bookmarks;
- sessions;
- state signatures;
- experiments and reversible patches;
- evidence/regression capture.

Machine/game-specific concepts should be supplied through registries/adapters:
- named variables/state;
- scenarios;
- semantic events;
- safe experiment parameters;
- device-specific inspectors;
- game-specific presets.

The intended flow remains:

`Scenario -> checkpoint/navigation -> experiment -> observation -> evidence`

but none of those infrastructure stages should require Chase H.Q. to be hard-coded into the core controller.

## Migration strategy: extract only when justified

Do **not** stop Chase H.Q. development for a speculative framework rewrite.

Use an incremental rule:
1. implement/understand the subsystem against Chase H.Q.;
2. establish evidence and stable behaviour;
3. identify the genuinely reusable boundary;
4. extract/refactor behind that boundary when it improves current work or when a second machine actually needs it;
5. keep regression tests proving Chase H.Q. behaviour did not change.

A second hardware target is a useful test of an abstraction. It should expose assumptions rather than cause a pre-emptive attempt to design every arcade board in advance.

## Near-term design rules

- Avoid new global Chase-H.Q.-specific assumptions in generic debugger/runtime code when a simple machine interface is available.
- Keep ROM layout and address-map knowledge out of generic CPU/runtime components.
- Prefer device interfaces and explicit machine wiring over cross-subsystem reach-through.
- Keep semantic names/evidence in machine/game registries rather than pretending they are generic hardware facts.
- Preserve CLI/config parity with interactive debugging so automation remains portable.
- Keep original rendering and machine behaviour as the reference for regression testing.
- Do not rename/refactor stable working code solely for architectural purity.

## Candidate future architecture work

These are candidates, not assigned releases:
- formal machine/device interfaces around current Chase H.Q. components;
- declarative/structured ROM-set descriptions;
- explicit reusable address-map/bus construction;
- generic debugger target interface and device-inspector registration;
- hardware-family extraction where multiple Taito devices can genuinely share implementations;
- semantic render-object/native-asset representation;
- hardware-specific import/extraction adapters;
- renderer/asset-pack interface;
- a deliberately chosen second-machine bring-up used to validate abstractions.

## Success criterion

The architecture is succeeding when adding another supported machine increasingly means **describing and wiring its hardware plus supplying the missing device implementations and game knowledge**, rather than copying Chase H.Q.-specific runtime/debugger/rendering code and editing it in place.
