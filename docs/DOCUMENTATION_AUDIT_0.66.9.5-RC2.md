> **AUTHORITATIVE CURRENT — v0.67.0.0-RC6.3.11 CPU-B IRQ-OWNERSHIP CORRECTIVE SOURCE CANDIDATE (2026-10-14).** RC6.3.10 actual Windows `native reconstruct --cpu-b-live` failed only at the CPU-B original-ROM baseline because 12 interrupt-vector/stack bus effects (four genuine IRQ entry groups) were wrongly tagged as BEQ.B data effects by stale `trace_pc`; its CPU-A health/copy, shared dispatcher and real CPU-B Ghidra/compiled ABI gates all PASS. RC6.3.11 uses Musashi interrupt ACK to assign each IRQ vector/stack transaction to a dedicated ordered IRQ stream, preserves strict zero-branch-data and original/native/fallback CPU/cycle/bus/machine parity, and adds real-ACK synthetic and missing/mutated IRQ evidence tests. **RC6.3.11 authentic Windows native CPU-B equivalence is PENDING**, as are complete-function/autonomous promotion and inherited Go Full Regression budget 1176>850. See `docs/RC6.3.11_IRQ_OWNERSHIP_CORRECTIVE.md`. Earlier status blocks remain historical.

# Documentation Audit - v0.66.9.5-RC2

Current release candidate: **v0.66.9.5-RC2**. This candidate is built directly from the exact fresh-Windows-qualified `v0.66.9.5-RC1.1` archive (SHA-256 `556aaee447fd0ebd616001139ae6f26572bad327325d48c105da6388905c44f1`). It is not promoted until the exact RC2 archive passes Windows `chqdev.bat qualify --fresh`.

`chqdev` is the **authoritative build/release** workflow and the de facto process. PowerShell/BAT wrappers are compatibility entry points. Promotion requires `qualify --fresh`, **zero tolerated CTest failures** (any CTest failure is fatal), deterministic exact-archive packaging/verification, focused regression, **Full Regression**, runtime/Workbench/mobile readiness and authoritative run-history verification. Shell compatibility does not override these gates. LF/CRLF are both supported where documented; release-owned Windows scripts remain ASCII-safe.

## Evidence incorporated from RC1.1 Runs 003-012

### Beacon producer and authentic sprite-pipeline latency

- Run003 showed a one-frame Force ON/OFF state change without a live visual change; this was initially treated as an implementation concern.
- Run004 identified final sprite-RAM slot-40 writes on CPU-A at PCs `0x40562/0x40564` into `0xD00140..0xD00147`; CPU-B did not write the descriptor.
- Run005 traced authentic map520/palette71 descriptor generation through CPU-A PCs `0x46BD4/0x46BD6` in the `0x1027xx` staging list. Force OFF suppressed this descriptor there, and a raw `101854=0000` / `108082=0000` freeze behaved identically to the helper.
- Runs006-007 closed the timing question: the intervention changes staging first, while the live `0xD00000` sprite list and framebuffer follow after the authentic staged pipeline delay. Force OFF disappears from presentation around frame6336 from a frame6333 start; Force ON from frame2064 becomes live around frame2068. Therefore RC1.1's coupled beacon implementation is causal and correct; the regression assumption, not the implementation, was wrong.
- The presentation model is now documented as `beacon state -> descriptor producer -> 0x1027xx staging -> D00000 sprite RAM -> renderer`.

### Semantic vehicle-family expansion

Runs008-010 established the following **confirmed** additions:

- `player_car.body`: maps **530, 536** (positive/incline render family; direct replacement succeeds where the old semantic resolver did not match).
- `player_car.shadow`: maps **598, 600** (co-located with 530/536; hiding removes exactly the visible under-car shadow pixels).
- `target_car`: map **20** / palette80 (small target-lateral intervention preserves map20 while shifting its rendered position).
- `police_beacon`: maps **519, 540, 617** and palette **78**, in addition to existing 518/520 and palette71. Force OFF removes these components after the measured presentation latency.

The semantic registry deliberately does **not** promote every visually similar ROM map discovered later; runtime causal evidence remains the promotion gate.

### Prototype vehicle atlas findings

- Run011 produced normalized side-by-side live-host sheets for confirmed player bodies, player shadows, target states and beacon components. It exposed an inspector ambiguity: while a presentation override was active, `sprite.inspect` retained authentic map/palette identity while geometry reflected the effective replacement.
- Run012 forced maps 450-620 through a common live host and proved the value of map-number adjacency. It found a visually coherent 30-map black-car artwork candidate set: `472,473,474,475,476,477,478,487,492,493,498,499,506,507,508,509,510,514,515,516,521,522,526,530,531,536,554,563,564,565`, plus a broad shadow-like block around 572-600.
- Those Run012 candidates are **ROM-visual candidates only** unless separately observed/causally tied to a live semantic object. The live-host prototype can also mislead on format, palette and clipping, so RC2 replaces it with a direct-ROM atlas workflow rather than treating it as semantic proof.

## RC2 implementation resulting from the evidence

- Direct ROM Sprite/Vehicle Atlas export from `b52-38.34` plus OBJ-A (`b52-34.5/35.7/36.9/37.11`) and OBJ-B (`b52-30.4/31.6/32.8/33.10`) graphics. It does not require a map to occur in live sprite RAM.
- Explicit `obj-a`, `obj-b64`, `obj-b32` or `all` interpretations; native-size raw-pen and palette-view PNGs; map/ROM provenance CSV; contact sheets; manifest; downloadable ZIP.
- Raw-pen geometry is authoritative for atlas presentation; runtime palette selection is a viewing aid only. Semantic identity is never inferred solely from visual similarity.
- `sprites list` / `sprite.inspect` now distinguish authentic `sourceMap/sourcePalette` from `effectiveMap/effectivePalette` and expose slot/map/asset override provenance.
- The confirmed semantic registry additions above are packaged in both authoritative semantic-object copies.
- The focused beacon gate is latency-aware and checks eventual live presentation rather than incorrectly requiring a next-frame framebuffer response.

## Remaining graphics/reconstruction work

- Add evidence-backed family clustering/similarity and runtime-evidence backlinks on top of the direct ROM atlas.
- Add richer palette-evidence views and automatic family contact-sheet grouping.
- Keep Run012 visual candidates as candidates until runtime observations or causal interventions promote them.
- Continue longer-term sprite upscale / high-resolution replacement and possible 3D-model exploration only after semantic/format provenance is mature enough to preserve authentic behaviour.

## RC2 qualification status

This document describes the RC2 source candidate. Portable build/test/package gates are necessary but not sufficient. Promotion requires the exact Windows RC2 archive to pass the authoritative `chqdev.bat qualify --fresh` flow, with zero tolerated CTest failures, deterministic archive verification, focused regression, Full Regression and run-history gates.
