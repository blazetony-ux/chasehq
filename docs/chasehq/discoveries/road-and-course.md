# Discovery record — road geometry and course data

**Status:** core road geometry/course channels CONFIRMED; topology/branch selector OPEN

## Outcome
- Runtime course source: `0x109000–0x109FFF`.
- 16 banks of `0x100` bytes; 32 records of 8 bytes per bank.
- Record `+0`: signed horizontal curvature/control channel — CONFIRMED.
- Record `+1`: vertical road-profile control channel — CONFIRMED.
- Current bank: `0x1021BC & 0x0F`.
- Live road boundaries: `0x10A05C`.
- Player lateral coordinate: `0x10A044`.
- Derived road centre/width/signed and normalized centre error are valid research telemetry.

## How it was obtained
Long deterministic surveys captured course-bank progression and live road geometry. Course records were correlated with visible curvature/profile behaviour and road-generator state. Repeated bank numbers at different physical course contexts disproved the assumption that bank number alone uniquely identifies course location.

## Important finding
Observed bank reuse means Stage 1 should be modelled as contextual progression/graph data, not simply a linear list of bank IDs.

## Open questions
The authoritative branch/fork selector and branch-commit event remain unresolved. Evidence does not yet justify claiming the course is infinite; finite/contextual reuse remains plausible but unproven.
