# Native reconstruction

A fully native Chase H.Q. implementation without CPU/hardware emulation is a credible long-term destination. Immediate work remains accurate emulation and causal reverse engineering. Prefer semantic state, algorithms and documented data structures over address-only knowledge; keep rendering/research interfaces usable independently of emulated CPUs.

| Maturity | Evidence required |
|---|---|
| UNKNOWN | No established observation |
| OBSERVED | Reproducible state or output observed |
| MAPPED | Addresses/data structures and ownership mapped |
| BEHAVIOUR UNDERSTOOD | Rules explain controlled experiments |
| NATIVE-CANDIDATE | Sufficient model to attempt independent reproduction |
| SHADOW VERIFIED | Independent native results match authoritative original execution under differential tests |
| REPLACEABLE | Coverage and failure/fallback criteria justify switching |
| NATIVE | Native backend is used with documented parity |

Knowing an address does not establish behaviour. Unit tests of one implementation do not establish shadow parity. Promote only on recorded evidence, with scope and limitations.

Future subsystem backends: original/emulated, native experimental, shadow/compare. In shadow mode original execution remains authoritative; the independent implementation computes the same result and machine-readable differential evidence reports divergence. Candidate early experiments include composition/mixing, road, sprite, background/tile rendering, HUD and input translation. Gameplay physics needs substantially stronger semantic knowledge.

Machine-readable state: `research/knowledge/native-subsystems.json`. RC2.7's old/corrected TC0100SCN switch is a diagnostic branch in one renderer, not an independent shadow backend. RC2.8 adds a separate sprite 4bpp SHADOW decoder and COMPARE utility; deterministic fixture parity is tested, but no whole-frame SHADOW VERIFIED claim is made. RC2.7's TC0100SCN BG correction is Windows-proven; RC2.8's text +16 origin and flipped-text path still require Windows visual proof.
