# Research Findings Consolidation — v0.59.0

## Confidence convention

- **Proven**: repeated trace evidence and/or causal intervention supports the interpretation.
- **Strong candidate**: tightly correlated and structurally plausible, but not yet generalised across contexts.
- **Open**: address/behaviour observed, semantic meaning unresolved.

## Target identity and proximity

The Stage-1 pursued object is represented by the 0x40-byte object record at `0x10A080`. Its longitudinal position updates once per frame from CPU-A `$004F9E`. A causal A/B intervention moving `0x10A080` farther away removed the exceptional interaction path, type-9 selection, `0x1002AE` decrement and +50,000 score award while leaving the object processing alive. This establishes `0x10A080` as the relevant pursued/target object for this encounter.

Sprite evidence across frames 7060–7085 identifies the visible encounter object as a four-map assembly `319/320/321/322`, palette `152`. It grows/moves toward the player through the approach and changes assembly/scaling behaviour at the proven contact frame.

## Contact / interaction chain

At frame 7084 in the canonical run:

1. Player longitudinal state around `0x10A040` and target longitudinal state `0x10A080` differ by about `0x2D` (45), below the proximity threshold around `0x100`.
2. The `0x009Cxx` path enters interaction handling around `$009E2A`.
3. Event/type `9` is selected and `0x10042E` becomes 9.
4. Object-local `0x10A089` bit `0x20` is asserted by CPU-A `$00A036` — **strong contact candidate**.
5. The type-9 path explicitly initialises `0x1002C6` to `0x003C` (60) at `$00A094` — **strong interaction/collision response timer candidate**.
6. The score-award source at `0x100186` receives `0x00050000`; the packed-BCD accumulator at `0x100488–0x10048B` consumes it and the sampled score subsequently changes `0x14 -> 0x19`.
7. CPU-A `$009C46` calls the response routine around `$00A12A`.
8. `$00A156` executes `ADD.W D5,$0004(A3)` with `A3=$10A040`, producing the physical lateral shove at `0x10A044`.

## `0x1002AE`

`0x1002AE` was initially tempting to label target health. Intervention disproved that simple interpretation. In the frame-7084 interaction it can decrement twice; forcing it near zero causes underflow/different downstream state while the encounter and score event still occur. It remains an interaction/phase counter candidate and must not be presented as health.

## Collision-free mapping experiment implemented in v0.59.0

`--no-collisions` suppresses only the proven CPU-A `$00A156 -> 0x10A044` lateral response write. The intention is to let course surveys retain collision/contact telemetry while preventing this specific response from pushing the autonomous car away from its line.

This option is **experimental and narrow**. It does not yet claim to suppress speed loss, spin, scenery impacts, every traffic type, target-specific secondary effects, off-road penalties or airborne/landing physics.

## Airborne state objective

Authoritative AIRBORNE/GROUNDED state has not yet been identified. The next investigation should search the player object/physics state and distinguish:

- grounded vs airborne flag/state,
- vertical position/height,
- vertical velocity,
- take-off and landing transitions.

The mapper should ultimately record airborne zones as course landmarks without disabling them when collision-free survey mode is active.

## Course topology objective

The course is not adequately represented by a single bank sequence because banks are revisited and routes branch. The next mapping milestone is to identify the state/write that commits a branch choice, create a checkpoint shortly before that decision, then drive the same state left/right and compare course context/bank progression. A complete Stage-1 map should be a graph with fork and rejoin nodes.
