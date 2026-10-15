> **AUTHORITATIVE CURRENT — v0.67.0.0-RC6.3.14 CPU-B DUAL-CANDIDATE DISPATCH CORRECTIVE (2026-10-14).** Actual RC6.3.13 Windows build PASS and Ghidra pair ABI PASS, but BEQ live acceptance BLOCKED in the Go verifier: 22 valid inactive pair original-ROM events were rejected despite 17,892 single-BEQ native commits. RC6.3.14 corrects only the verified audit/proof contract, adds inverse strict pair-mode audits and mutation regressions; **Windows RC6.3.14 genuine BEQ+pair live three-mode parity PENDING**. No whole-function or autonomous promotion; inherited Go Full Regression structural count 1,176 >850 remains FAIL. See `docs/RC6.3.14_CPU_B_DISPATCH_AUDIT_CORRECTIVE.md`. Older CURRENT banners are historical.

> **AUTHORITATIVE CURRENT — v0.67.0.0-RC6.3.13 CPU-B TWO-INSTRUCTION LIVE SOURCE CANDIDATE (2026-10-14).** RC6.3.12 real Windows generated TST.W+BEQ pair ABI 4/4 PASS from short output path, with previous RC6.3.11 authentic single BEQ live parity PASS. RC6.3.13 introduces guarded actual native TST.W+BEQ replacement and original/generated/forced-ROM rollback differential, mapped R16, IRQ and full machine evidence. Local 68000 tests PASS; **actual RC6.3.13 Windows live pair acceptance PENDING**. Full function/autonomous M1–M7 unproven; inherited Full Go regression 1176>850 still FAIL. See `docs/RC6.3.13_LIVE_CPU_B_PAIR.md`. Older CURRENT statements are retained only for historical provenance.

> **AUTHORITATIVE PROMOTED BASELINE — v0.67.0.0-RC4.6 (2026-10-10).** Windows `chqdev.bat qualify --fresh` reports **11/11 stages PASS** for exact frozen ZIP SHA-256 `9e76a107e19f3e28a54a8a8b3bdaed630511e02f1da3e3e7ae274c07d863580f`. User explicitly authorised promotion following focused, full, and successful headless Ghidra/REA crosswalk evidence. This is a **post-qualification documentation overlay**; it does not change the immutable tested archive. See `docs/RELEASE_VERIFICATION_RC46.md`, `promotion/PROMOTION_RECORD.json`, and `docs/REA_GHIDRA_NEXT_SCOPE.md`. Earlier RC4.6 “candidate/unqualified” statements below are **historical build-time notes, not current release status**.

> **RC4.4 candidate:** `v0.67.0.0-RC4.4` — Windows qualification pending; chqdev authoritative, original CPU execution retained. Full scope: `docs/NATIVE_PROJECTION_RC44.md`. Release guard: LF/CRLF parity.

# Native reconstruction

## v0.66.9.4-RC3 semantic replacement milestone

RC3 builds on RC2's qualified executable sprite backend by making replacement format-aware. A Native replacement may decode its target map using OBJ-A, OBJ-B64 or OBJ-B32 independently of the source descriptor's zoom-X/format, while retaining the source placement box. `Original` remains the default/oracle. `Shadow` displays Original while running the independent placed-source renderer and accumulating comparison counters/first-divergence metadata. `Native` displays the independent renderer and is explicitly experimental until exact-archive Windows RC2 qualification. The existence of the switch does not by itself promote the subsystem beyond **SHADOW VERIFIED**.

Global sprite-map substitution is a presentation-only asset experiment layered on the same sprite path. It replaces an active source map with another existing OBJ map (and optional palette) wherever that source map appears; game sprite RAM and gameplay objects are not mutated. This is the bridge toward later semantic vehicle/billboard replacement packs.

The next native-tooling priorities are semantic sprite-family discovery/coverage, click-to-identify source mapping, arbitrary PNG/RGBA replacement with anchors/hot reload, and first-divergence/routine-contract tooling for bounded 68000 gameplay rewrites.

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

Machine-readable state: `research/knowledge/native-subsystems.json`. The final v0.66.9.3 release promotes both the independent `sprite-4bpp-decoder` and the independent `sprite-placement-ownership`/placed-source renderer to **SHADOW VERIFIED** for the canonical eight-checkpoint corpus. The Windows focused RC5 promotion regression compared 76,800 placed-source pixels per checkpoint and reported zero pixel and owner mismatches at 2352, 2064, 5587, 5588, 6333, 6334, 9548 and 9988. This remains a shadow implementation only: the original/emulated renderer is authoritative, no default replacement backend is enabled, and REPLACEABLE requires explicit backend switching plus documented coverage/fallback criteria. TC0100SCN, TC0150 road, compositor and HUD still lack equivalent independent full-subsystem shadow verification.
