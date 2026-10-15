# Discovery record - gameplay state

**Status:** core Stage-1 state substantially CONFIRMED; remaining producer/semantic questions listed below.

| State | Source / encoding | Confidence |
|---|---|---|
| Display speed | CPU-A `0x100400`, packed BCD | CONFIRMED |
| Internal/physics speed | CPU-A `0x10041C`, raw fixed/internal value | CONFIRMED |
| Authoritative score | CPU-A `0x100488`, packed-BCD-like score state | CONFIRMED |
| Immediate display score | CPU-A `0x100408`; tested `0x1234 -> 0x1230` from authoritative source | CONFIRMED |
| Distance | CPU-A `0x102FC0`, 24.8 fixed point | CONFIRMED |
| Player lateral | CPU-A `0x10A044` | CONFIRMED |
| Road-side state | CPU-A `0x10A048`: `01` ON, `03` RIGHT, `05` LEFT, `07` EXTREME | CONFIRMED |
| Processed turbo input | `0x100303 bit0` | CONFIRMED |
| Turbo active | `0x10040F bit1` | CONFIRMED |
| Turbos remaining | `0x1003A2` | CONFIRMED |
| Turbo timer | `0x100414` | CONFIRMED |
| Authentic Brake | IOC **P2:20** | CONFIRMED |
| Processed Brake/upstream state | CPU-A `0x100302`; forced `0x0014` reproduces braking physics + lamp path | CONFIRMED |
| Brake lamp | palette banks64/65 pen13 `0x000E -> 0x18DE` | CONFIRMED |
| Raw steering | signed 12-bit IOC; centre `0000`, right `0060`, left `0FA0` | CONFIRMED |
| Track grade | current record `+1`; positive INCLINE/uphill, negative DECLINE/downhill, zero level | CONFIRMED |
| Race timer | CPU-A `0x100200/0x100201`; authentic `01 -> 00` at PC `0x1AE0` leads TIME UP then CONTINUE | CONFIRMED |
| Collision interaction type | CPU-A `0x10042E`; target damage observed `000A`, off-road scenery/object collision `0001` | CONFIRMED at observed classes |
| Collision object reference | CPU-A `0x100420`; Run020 type0001 wrote `0x0010A4C0` | HIGH CONFIDENCE semantic pointer/reference |
| Target remaining hits | CPU-A `0x1002AE`; PC `0xA112` decrement, `FFFF` terminal sentinel | CONFIRMED |
| Target object record | CPU-A `0x10A080-0x10A0BF`, lateral `+4` at `0x10A084` | CONFIRMED |

## Exact road-side classifier on canonical straight geometry

With `roadLeft=0x7470`, `roadRight=0x86F0`, the decoded classifier is: `x<=0x7090` EXTREME, `0x7091..0x7190` LEFT, `0x7191..0x89D0` ON/INTERIOR, `0x89D1..0x8AD0` RIGHT, `x>=0x8AD1` EXTREME. This explains why merely crossing the visible road edge does not immediately set off-road state.

## Causal notes

Run018 proved grade sign by sustained controlled patches: negative grade increased speed/progress relative to neutral, positive grade reduced it. Run018 also proved `0x100488 -> 0x100408 -> text HUD`; forcing `0x100408` alone was sufficient for the exact HUD result. Run020 proved the timer is authentic gameplay state even though direct same-frame freezes do not immediately redraw the text HUD; its display path is tick/state driven.

The first mapped target physical response after initial contact occurs at frame5664: PC `0xA1BE` changes lateral state and PC `0xA200` applies exact 0.75 speed retention. On the deterministic right off-road path, interaction type `0001` first triggers the same mapped response at frame2125.

## Remaining gameplay-state work

The main open states are exact collision-object semantic typing, deeper timer/text producer state, top-speed clamp/target logic and broader later-stage state. The police-beacon primary lifecycle is behaviour-understood; use read provenance/structured events rather than screenshot inference for remaining work.
