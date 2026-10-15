> **RC6.3.33 authoritative current release metadata.** This document retains its historical content; current version is 0.67.0.0-RC6.3.33. See docs/RC6.3.33_GENUINE_SUBPROCESS_REPLAY_AND_ROLLBACK.md.

> Current development candidate: ChaseHQ-Native 0.67.0.0-RC6.3.32; original-ROM CPU-B writer results from RC6.3.31 are genuine but full function CFG/native promotion remain blocked. Check docs/RC6.3.32_CHECKPOINT_REPLAY_AND_FUNCTION_BOUNDARY_CONFIDENCE.md.

> CURRENT RELEASE 0.67.0.0-RC6.3.31: see `docs/RC6.3.31_AUTONOMOUS_FUNCTION_BLOCKERS_AND_DIAGNOSIS.md`; real Windows new writer capture PENDING, complete function native promotion BLOCKED; older sections historical.

> CURRENT BUILD: 0.67.0.0-RC6.3.30 — read-only BTST bit7 source-verification candidate; Windows ROM gate pending; historical body follows.

> **RC6.3.30 current source-candidate context (2026-10-15):** immutable RC6.3.29 Windows-qualified baseline; new 0x050E BTST byte evidence Windows execution pending, no whole-function native promotion. Historical discussion below.

> **Current authoritative version 0.67.0.0-RC6.3.29:** Read-only 0x0516 CPU-B BNE analyser corrective. Genuine CHQ6328 original-ROM 35/35 equal-clock immediate next-PC samples; Windows acceptance pending, 0x0518 unobserved, no native whole-function promotion. See docs/RC6.3.29_CPU_B_EQUAL_CYCLE_CORRECTIVE.md. Historical documentation below preserved.

> ChaseHQ-Native 0.67.0.0-RC6.3.28 current active source candidate; historic details retained below.

> RC6.3.28 current active source candidate (Windows qualification pending); historical details below.

# RC6.3.27 — Original-ROM CPU-B IRQ vector and ordered exception stack bus evidence

**Candidate:** `0.67.0.0-RC6.3.27` / `chqdev v0.5.46` (2026-10-15). **Baseline:** immutable and fully Windows-qualified RC6.3.26 source SHA256 `aee45c18268daa006f13a0bebaed43f61e9dbd804a63cf1d34b426ae12650386`. Windows qualifying dist hash `7cb0ebcfe2ef5076e058a2b1f0d82f29f244fa45cc476f16f6f3561072eff6cf` recorded by Windows (archive bytes not provided). RC6.3.26 native reconstruction PASS, Windows 18/18 CTests, focused Run 001 10 commands, Full Regression Run 002 1,153 commands, session `20261015_155233`, three evidence ZIPs independently hashed 4,176/4,176.

## New scoped, read-only functionality

RC6.3.26 already proved 35 authentic sampled interrupt-frame restorations across natural, bypass, nonterminal, terminal and overflow checkpoints, including saved SR/PC and mode-dependent active SP, USP and ISP. However its handler/RTE hooks are memory *peeks* and are not themselves evidence of actual exception bus read/write order.

RC6.3.27 introduces a **separate original-ROM CPU-B bus observer** that activates only on the real Musashi interrupt ACK callback while the original-ROM edge probe is enabled. It records the ACK and the ordered original-ROM Bus::read/write transactions until the next CPU-B instruction hook (handler entry). Exports `cpu-b-original-irq-delivery-bus.tsv` (sample, frame, kind, address, value, width) and `cpu-b-original-irq-delivery-cap.json` (schema, bounded count, drops, pending, transition count) in every original checkpoint case; cap rejects missing/dropped/incomplete data.

`M2-CPU-B-original-ROM-IRQ-vector-and-stack-bus-order` checks for each sampled frame exactly: ACK from Musashi (level 4, interrupted PC), autovector table read at 0x70 of handler 0x0474 (width 4), interrupted PC write at ISP+2 (width 4), interrupted SR write at ISP (width 2), first CPU-B handler instruction 0x0474. Frame numbers/order/multiplicity agree with the preceding genuine RC6.3.26 six-byte saved frame and USP/ISP restoration proof and original CPU-B ROM hook boundaries. Reject reordered/invented/missing events, different widths, PC, SR, vector, stack addresses, missing handler or mismatching sample/transition cap.

**Important:** These are Musashi-emulated bus callbacks, not external hardware captures. The selected vector read proves what the emulator accessed, not physical IRQ source ownership or real bus cycle timing. Five checkpoint windows and one handler are not an exhaustive IRQ/scheduler proof. Nothing certifies complete function CFG, the unobserved 0x0518 branch, compositional parity or autonomous promotion. `nativePromotionAllowed=false` is explicit and mandatory. Existing bounded CPU-A/B native and rollback paths are untouched.

## Provenance distinctions and validation

Previous authentic RC6.3.26 Windows evidence is available for validating frame and interrupt prerequisites (and RC6.3.26 independent native pair IRQ streams contain real ordered IRQ transactions). The NEW standalone original-ROM hook capture is only **synthetic in local Go mutation tests**, not falsely presented as a genuine current release trace. Linux native 15/15 CTests, full Go suite, portable preflight, cross-platform tool compilation and deterministic manifest/package are local gates. Fresh Windows ROM `native reconstruct --fresh --live-health --cpu-b-live` and `qualify --fresh` remain required before promotion.

## Windows acceptance (single-line PowerShell)

`New-Item -ItemType Directory -Force 'C:\Projects\Evidence' | Out-Null; .\chqdev.bat build debug; if ($LASTEXITCODE -eq 0) { .\chqdev.bat native reconstruct --fresh --live-health --cpu-b-live --out C:\Projects\Evidence\CHQ6327 }`

Expected evidence: `C:\Projects\Evidence\CHQ6327.zip`. Verify all five original-ROM cases, ACK/vector/PC/SR/handler order, cap schema/dropped=0, new JSON report `cpu-b-original-rom-irq-vector-stack-bus-order.json`, Full Regression and exact source/archive qualification. Then `.\chqdev.bat qualify --fresh`. RC6.3.26 stays the latest fully qualified baseline until accepted.

Packaging and Windows qualification scripts preserve ASCII-compatible CRLF and LF line endings.
