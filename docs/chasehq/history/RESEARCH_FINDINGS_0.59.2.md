# Research findings — v0.59.2

## Handling/cornering path
Evidence from v0.59.1 traces places the normal movement handling transform around CPU-A `$008A76-$008AB4`. Internal speed in D2 is multiplied by coefficients selected from the ROM table around `$00AD46`. Observed forward coefficients include `$0100`, `$00FE`, `$00F4`; the paired lateral coefficient rises with turn magnitude. The resulting movement state feeds the normal player lateral update culminating at `$008B84 -> $10A044`.

v0.59.2 captures these values directly in the instruction hook but does not yet claim a universal tyre-grip scalar. The next causal test is to override a coefficient after read/before multiply and compare centre holding at identical course position/speed/steering.

## Mapping/autopilot direction
The mapping target line is the authoritative road centre derived from `$10A05C`; car lateral position remains `$10A044`. Road width is derived from the boundary separation and centre error is normalized by half-width. Learned course-profile data should supply feed-forward steering/throttle, while live road-centre geometry provides correction and recovery.
