# Discovery record — Stage-1 target contact and damage

**Status:** target identity/contact CONFIRMED; target health/damage BEHAVIOUR UNDERSTOOD.

## Target object

- object record: `0x10A080-0x10A0BF`
- longitudinal position: `0x10A080`
- lateral/response field used by physical contact: `0x10A084`
- target motion: `0x10A092`
- defeated setpoint/floor: `0x10A096`

Physical overlap/contact at frame 5588 reaches the `0x9C46 -> 0xA12A` response path and modifies target `0x10A084`; it does **not** by itself modify the confirmed damage counter. `0x10A089` pulses can occur without that physical contact and must not be used as a hit/damage flag.

## Genuine damage

Canonical frame 6334 is a proven damaging target impact:

- PC `0x9EC8`: interaction type `0x10042E = 0x000A`
- PC `0xA094`: response timer `0x1002C6 = 0x003C`
- PC `0xA112`: `subq.w #1, 0x1002AE`
- normal/nonterminal result: PC `0xA124` updates `0x1002D8` (observed `0x0155`)
- terminal underflow: PC `0xA118` writes `0x1002D8 = 0x1400`

## Superseded interpretation

Earlier releases rejected `0x1002AE` as health because post-defeat observations and insufficiently controlled interventions conflated terminal/phase behavior with live target state. The frame-6333 causal A/B proof supersedes that interpretation. `0x1002AE` is now CONFIRMED as the special target remaining-hit/damage counter. Zero means one final damaging hit remains; the next authentic decrement produces `FFFF` and enters defeat.

See `target-health-and-defeat.md` for the complete causal proof and downstream state machine.
