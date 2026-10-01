# Research Findings — v0.59.1

## Course and mapping state

The runtime course source remains `0x109000–0x109FFF`: 16 banks of `0x100` bytes, 32 records of 8 bytes per bank. Record `+0` is the proven signed horizontal curvature/control channel and record `+1` the proven vertical road-profile control channel. Current bank is `0x1021BC & 0x0F`. Live road boundaries are at `0x10A05C`; player lateral position is `0x10A044`.

Long surveys have observed the bank sequence `0 -> 1 -> 3 -> 7 -> 15 -> 0 -> 1 -> 2 -> 3 -> 4 -> 5 -> 6 -> 7 -> 8 -> 9...`. Bank reuse means a bank number is not a unique physical-course node. The course must be represented as a graph with contextual route/branch state. The authoritative branch selector remains unresolved.

No evidence yet proves the track is infinite. Course position continued increasing through the long frame-12001 survey; current evidence is more consistent with finite/reused course data assembled through contextual progression than with procedural infinite generation, but that remains an inference until the reachable graph/end/repeat behaviour is enumerated.

## Predictive course follower

v0.59.1 adds a new default predictive controller while preserving the legacy v0.57 controller for A/B testing.

Predictive inputs:
- current and upcoming `+0` course curvature records within the current bank;
- live road centre derived from `0x10A05C`;
- player lateral coordinate `0x10A044`;
- lateral error and frame-to-frame error rate;
- displayed speed at `0x100400`.

Predictive outputs/behaviour:
- weighted look-ahead curvature feed-forward;
- speed-scaled proportional centring term;
- derivative damping term;
- steering slew-rate limiting;
- curve-severity target speed with accelerator lift/braking.

Cross-bank look-ahead is intentionally not guessed. Once the route selector is proven, future controllers can predict safely across branch/bank boundaries.

## Collision-response family

The longer v0.59.0 collision survey proved that the original single-PC suppression was incomplete. The response family contains four proven lateral-response writes to player coordinate `0x10A044`:

- `$00A142` — subtract lateral displacement;
- `$00A156` — add lateral displacement;
- `$00A1BE` — alternate subtract path;
- `$00A1C4` — alternate add path.

A separate proven speed consequence occurs at `$00A200`. The game loads internal speed from `0x10041C`, computes 3/4 of the previous value, then stores it back: a 25% speed loss.

v0.59.1 `--no-collisions` suppresses those five response writes while preserving upstream detection, target/object state, interaction timers and scoring/event logic. This provides a cleaner mapping intervention while retaining evidence of where collisions would have occurred.

The interaction timer at `0x1002C6` is explicitly initialised to `0x003C` in the investigated type-9 path and counts down. It is a strong interaction/collision-response timer candidate, but not yet proven universal across every object type.

## Stage-1 target/pursuit state carried forward

- target/pursuit object record: `0x10A080–0x10A0BF`;
- target longitudinal position: `0x10A080`;
- object-local status byte: `0x10A089`;
- bit `0x20`: strong contact/interaction candidate;
- Stage-1 visual identity in investigated encounter: maps `319–322`, palette `152`;
- interaction/event type: `0x10042E`;
- interaction timer: `0x1002C6`;
- `0x1002AE` remains an interaction/phase counter candidate and must not be labelled target health.

## Open authoritative-state targets

Priority discoveries remain: AIRBORNE/GROUNDED plus vertical position/velocity/landing; route/fork selector and branch-commit event; target health/damage/hit/defeated/caught; IN SIGHT; siren/police-light state; remaining collision types if any bypass the five proven response writes.

## Tooling limitations carried forward

- frame-sampled telemetry can lag event-time writes;
- broad `--trace-mem-change` is not a substitute for explicit write tracing for known causality;
- ROM patch reporting and CPU instruction-fetch visibility are inconsistent;
- current `--patch-when` does not provide reliable same-write interception semantics.
