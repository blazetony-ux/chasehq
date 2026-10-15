# Discovery record - collision response family

**Status:** mapped response writes CONFIRMED; structured event instrumentation available; complete interaction taxonomy remains open.

## Proven response writes

Player lateral `0x10A044`: PCs `$A142`, `$A156`, `$A1BE`, `$A1C4`. Internal/physics speed `0x10041C`: PC `$A200` writes exactly **3/4 of the attempted pre-response speed** (25% loss). The first mapped target-route shove was frame5664: ordinary lateral/speed updates were followed by `$A1BE` and `$A200`.

`collision.response.set enabled=false` suppresses only these proven response writes while preserving detection/event logic. Counters are session-cumulative unless reset; interpret experiment-local deltas.

## Run020 off-road interaction

On the deterministic full-right + accelerator route, off-road begins RIGHT at frame2103 and EXTREME at 2109. The first additional mapped physical response occurs on 2124->2125. In that frame PC `$9EC8` writes interaction type `0x10042E: 0000->0001`; related fields include `0x100424=03102048`, `0x100428=01400076`, `0x10042C=006A`, and PC `$A19A` writes `0x100420=0010A4C0`. `0x1002C6` remains zero, distinguishing this from the target damaging-hit class (`0x10042E=000A`).

Same-frame response suppression preserved detection but removed the mapped consequences. Authentic physics speed became `0x006E81C2`; suppressed became `0x009357AE`, exactly confirming the 0.75 response. The frame image was still identical at 2125, so physical-state response precedes visible divergence.

## Open

Treat type `0001` as a confirmed non-target physical object/scenery interaction class, but do not name the exact scenery object until `0x100420`/object record semantics are traced. Use structured collision events plus read provenance for the next classification work.
