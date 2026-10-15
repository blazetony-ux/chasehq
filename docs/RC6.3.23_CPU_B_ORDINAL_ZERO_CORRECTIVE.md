> **RC6.3.33 authoritative current release metadata.** This document retains its historical content; current version is 0.67.0.0-RC6.3.33. See docs/RC6.3.33_GENUINE_SUBPROCESS_REPLAY_AND_ROLLBACK.md.

> Current development candidate: ChaseHQ-Native 0.67.0.0-RC6.3.32; original-ROM CPU-B writer results from RC6.3.31 are genuine but full function CFG/native promotion remain blocked. Check docs/RC6.3.32_CHECKPOINT_REPLAY_AND_FUNCTION_BOUNDARY_CONFIDENCE.md.

> CURRENT RELEASE 0.67.0.0-RC6.3.31: see `docs/RC6.3.31_AUTONOMOUS_FUNCTION_BLOCKERS_AND_DIAGNOSIS.md`; real Windows new writer capture PENDING, complete function native promotion BLOCKED; older sections historical.

> CURRENT BUILD: 0.67.0.0-RC6.3.30 — read-only BTST bit7 source-verification candidate; Windows ROM gate pending; historical body follows.

> **RC6.3.30 current source-candidate context (2026-10-15):** immutable RC6.3.29 Windows-qualified baseline; new 0x050E BTST byte evidence Windows execution pending, no whole-function native promotion. Historical discussion below.

> **Current authoritative version 0.67.0.0-RC6.3.29:** Read-only 0x0516 CPU-B BNE analyser corrective. Genuine CHQ6328 original-ROM 35/35 equal-clock immediate next-PC samples; Windows acceptance pending, 0x0518 unobserved, no native whole-function promotion. See docs/RC6.3.29_CPU_B_EQUAL_CYCLE_CORRECTIVE.md. Historical documentation below preserved.

> ChaseHQ-Native 0.67.0.0-RC6.3.28 current active source candidate; historic details retained below.

> RC6.3.28 current active source candidate (Windows qualification pending); historical details below.

> **Release provenance:** 0.67.0.0-RC6.3.27 current source candidate; this document preserves historical analysis and does not independently qualify new IRQ bus evidence.

> **AUTHORITATIVE CURRENT — ChaseHQ-Native 0.67.0.0-RC6.3.26 CPU-B DUAL-STACK EXCEPTION RETURN CORRECTIVE SOURCE CANDIDATE (2026-10-15).** Latest fully Windows-qualified baseline RC6.3.24, qualifier-reported dist SHA256 `78bc9b7fdaa00a55294f55099c18df82ed69e07b0d53e57b0bffded3308de014`. RC6.3.25 exact source SHA256 `64e62b77b118577cecfa2d0d4c3ed6eb57ccc456344f11e6a3229a09e0d525b8` is **FAILED Windows ROM acceptance**: handler `0x0474` and RTE `0x04D6` returned to CPU-B user SR `0x0004`, restored PC `0x04D8/0x04DE` correctly, but active A7 switched to USP `0x100800` rather than popped ISP `0x100400`; the original RC6.3.25 verifier incorrectly insisted on `SP+6` for active A7. RC6.3.26 instruments and verifies original-ROM USP and ISP independently, plus saved frame, post-RTE PC/SR and bank-selected active SP; fail-closed synthetic user/supervisor mutations PASS. Prior genuine CHQ6325 evidence supports diagnosis, NOT NEW dual-bank capture. **Fresh RC6.3.26 Windows ROM native and `qualify --fresh` remain PENDING.** `0x0518`, vector/bus timing, full function/native promotion remain BLOCKED. Evidence: `C:\Projects\Evidence\CHQ6326.zip`. See `docs/RC6.3.26_CPU_B_DUAL_STACK_BANK_CORRECTIVE.md`. Previous banners are historical.

> **AUTHORITATIVE CURRENT - ChaseHQ-Native 0.67.0.0-RC6.3.25 CPU-B EXCEPTION-FRAME RAM/POST-RTE RESTORATION SOURCE CANDIDATE (2026-10-15).** Latest fully Windows-qualified immutable baseline **RC6.3.24**, exact source ZIP SHA-256 `cf1e9c75755c8ee13f8786c552bdf58e67ca930d86a2d5960c179745dd89b1d3`; reported Windows dist SHA-256 `78bc9b7fdaa00a55294f55099c18df82ed69e07b0d53e57b0bffded3308de014` (Windows package bytes not independently uploaded). RC6.3.24 genuine native and 18/18 Windows CTests, focused 10 commands and Full Regression 1,153 commands PASS. RC6.3.25 adds **new read-only original-ROM CPU-B six-byte exception frame RAM peeks** at 0x0474/0x04D6 and post-RTE next-hook PC/SR/SP crosscheck. **Authentic CHQ6324 prerequisite 35/35 PASS; NEW frame data is synthetic in local tests and genuine Windows qualification PENDING.** Not a bus transaction trace, vector ACK, all IRQs, scheduler or native function promotion. `0x0518` remains unobserved; `--out C:\Projects\Evidence\CHQ6325`. See `docs/RC6.3.25_CPU_B_EXCEPTION_FRAME_RESTORATION.md`. Older banners below historical.

> **AUTHORITATIVE CURRENT — ChaseHQ-Native 0.67.0.0-RC6.3.24 ORIGINAL-ROM CPU-B IRQ/RTE SELECTED-HOOK CONTEXT SOURCE CANDIDATE (2026-10-15).** Latest fully Windows-qualified immutable baseline **RC6.3.23**, exact source ZIP SHA-256 `9d7b33423e6c7f022059e77eca233f7cdae9b497ab5fedbd0eb60765ab75c55a`, qualifier-reported Windows distribution SHA-256 `3586f6fada3fc56384fb7a213f9c289a68b998427f22a3bf962ae81c28636e5b` (distribution bytes not independently uploaded). RC6.3.23 Windows native, 18/18 CTest and Full Regression 1153/1153 PASS. RC6.3.24 adds fail-closed `M2-CPU-B-original-ROM-IRQ-RTE-hook-context`: authentic CHQ6323.zip selected CPU-B 0x0474 handler-entry and 0x04D6 RTE observations, 7 sampled pairs/case across five cases (35), supervisor-mode, opcode, frame, order and sampled SP consistency. Actual exception-frame bytes/bus transactions, vector ownership, interrupted SR/PC, all IRQs, scheduler and native function promotion NOT VERIFIED. **RC6.3.24 ROM-backed Windows acceptance and `qualify --fresh` PENDING.** Evidence output `C:\Projects\Evidence\CHQ6324.zip`; `0x0518` remains unobserved. See `docs/RC6.3.24_CPU_B_IRQ_RTE_HOOK_CONTEXT.md`. Older status banners below are historical.

# RC6.3.23 — CPU-B ordered-hook ordinal-zero corrective

**Release:** `v0.67.0.0-RC6.3.23`, `chqdev v0.5.42`. Baseline is immutable RC6.3.22 source SHA-256 `c17673a958446875f047bfd1dceab5675ff5e81084949578615edc9929c3684e`. Last fully Windows-qualified baseline: RC6.3.21 (qualifier-reported Windows archive SHA-256 `fbfc6c19131a3a10a28de60449a9716111179fe03cc4285f2f970a0e8646ea3a`). RC6.3.22 is **NOT qualified**: actual Windows ROM-backed audit failed on natural event row one after successful MSVC build.

## Root cause and exact correction

The genuine original-ROM `cpu-b-ordered-hook-events.tsv` first event in each of five natural, bypass, nonterminal, terminal and overflow cases has ordinal `0`. The original CPU-B hook records the first selected PC before the `cpuBTransitions` count advances. The RC6.3.22 Go analyser erroneously included `ordinal == 0` in the invalid-row predicate despite the C++ recorder's zero-based chronology. **RC6.3.23 removes only that erroneous check**. The existing `ordinal > cpuBTransitions`, strictly increasing subsequent ordinal, monotonic cycle, opcode-fingerprint, header/schema, size, drop/saturation and selected nested-RTS pairing guards remain active. No C++ runtime execution, substitution, IRQ or bus logic is changed.

## Genuine Windows evidence and regression

The actual user-provided failed `CHQ6322.zip` contains five authentic original-ROM ordered-hook TSVs and caps (`100` selected events/case, first ordinal `0`, dropped `0`), CPU-B Ghidra static output, and five original-ROM edge tables. The optional Go integration test `TestRC6323RealWindowsCHQ6322OrderedStackIntegration`, enabled by `CHQ_RC6322_EVIDENCE_ZIP`, reads these actual artifacts without bundling ROMs and replays **the entire** M2 ordered audit, including Ghidra/ROM prerequisites. It derives original ROM SHA-256 directly from the Ghidra provenance record, verifies four selected nested sequences and 16 ordered return pairs in every case, and requires all full-function/IRQ stack/native promotion flags to remain blocked. `TestRC6323ZeroBasedOrdinalIsValidButRepeatedZeroIsNot` also confirms the synthetic zero-first ordinal succeeds, duplicate zero after first is rejected, and earlier positive-ordinal fixture remains valid. Original mutation guards for wrong opcode/order/SP/alternate branch remain intact.

**Evidence distinction:** genuine uploaded Windows traces establish sampled original-ROM chronology, not exhaustive instruction coverage or original-vs-generated function-level parity. Local replay is not a new live Windows acceptance. Promotion still blocked for full CPU-B CFG/callers/exits, indirect targets, IRQ/RTE stack bus and scheduler timing, alternate `0x0518`, automatic failure minimiser and multi-routine native composition.

## Windows handoff

From extracted RC6.3.23 project root, single-line PowerShell:

`New-Item -ItemType Directory -Force 'C:\Projects\Evidence' | Out-Null; .\chqdev.bat build debug; if ($LASTEXITCODE -eq 0) { .\chqdev.bat native reconstruct --fresh --live-health --cpu-b-live --out C:\Projects\Evidence\CHQ6323 }`

Expected `[PASS] M2-CPU-B-ordered-original-ROM-stack-audit`, plus complete `CHQDEV NATIVE RECONSTRUCT: PASS`. Only when this is proved on **RC6.3.23** run ` .\chqdev.bat qualify --fresh ` for Windows CTest, exact package, focused Workbench regression, Full Regression and promotion gate. Store the new authoritative evidence and actual run IDs. Keep RC6.3.22 and RC6.3.21 immutable; do not relabel their archives.

Release hygiene: Preserve CRLF/LF compatibility for the Windows shell, scripts and documentation; retain the chqdev portable release guard.
