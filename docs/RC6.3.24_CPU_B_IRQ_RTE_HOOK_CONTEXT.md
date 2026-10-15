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

# RC6.3.24 — CPU-B selected original-ROM IRQ/RTE hook-context verification

**Version:** `0.67.0.0-RC6.3.24`; `chqdev v0.5.43`. Source baseline: exact immutable RC6.3.23 candidate archive SHA-256 `9d7b33423e6c7f022059e77eca233f7cdae9b497ab5fedbd0eb60765ab75c55a`, Windows-qualified on 2026-10-15. Windows qualification distribution SHA-256 `3586f6fada3fc56384fb7a213f9c289a68b998427f22a3bf962ae81c28636e5b` was reported by `chqdev qualify --fresh`, not independently rehashed against the Windows dist ZIP.

## Research change (read-only, no new native execution)

Adds `M2-CPU-B-original-ROM-IRQ-RTE-hook-context` after the pre-existing `M2-CPU-B-ordered-original-ROM-stack-audit` and publishes `cpu-b-original-rom-irq-rte-hook-context.json`. Reuses original-ROM CPU-B selected instruction hook rows and their authenticated zero-drop cap, static Ghidra/ROM edge prerequisites and selected BSR/RTS verifier.

For each checkpoint, inspect every sampled CPU-B site `0x0474` (`0x48E7` handler code) and `0x04D6` (`0x4E73` RTE). Reject missing/repeated/orphan RTE, wrong opcode, user-mode SR at either sampled site, SP mismatch, different frame, inconsistent ordinal or intervening selected hook. Record chronological entry and return ordinals, frame, SP, SR, interval and whether the sampled nested call was active. Genuine prior `CHQ6323.zip` original-ROM evidence has **seven paired handler-entry/RTE instruction hooks in each of five checkpoint cases (35 total)**, with sampled SP `0x1003FA`, ordinal gap 14 and first entry ordinal 0. These observations are original-ROM, not synthetic. Mutation guards include wrong/missing RTE, bad handler opcode, wrong SP or SR and duplicated events.

**Non-certifications (fail closed):** PC=0x0474 is a sampled handler site, not independent proof of the hardware vector source; the filtered hook stream omits intermediate instructions and does **NOT** prove the exception-frame bytes, stack RAM bus transactions, complete IRQ delivery set, pre-interrupt PC/SR restoration, scheduler timing, multi-function composition or native substitution parity. `0x0518` is still runtime-unobserved, function CFG is incomplete, and `nativePromotionAllowed=false` / `completeFunctionVerified=false` remain mandatory.

The optional real-evidence integration test `TestRC6324RealWindowsCHQ6323IRQRTESample` sets `CHQ_RC6323_EVIDENCE_ZIP` to the authentic user-supplied archive. Its source data is never packaged and no ROMs are distributed. It checks Ghidra-derived ROM identity, all five original evidence files, 35 selected samples and the fail-closed maturity flags. The synthetic mutation fixture is explicitly separate from genuine execution.

## Windows acceptance handoff

After extracting RC6.3.24, execute from project root, single-line PowerShell:

`New-Item -ItemType Directory -Force 'C:\Projects\Evidence' | Out-Null; .\chqdev.bat build debug; if ($LASTEXITCODE -eq 0) { .\chqdev.bat native reconstruct --fresh --live-health --cpu-b-live --out C:\Projects\Evidence\CHQ6324 }`

Require `[PASS] M2-CPU-B-original-ROM-IRQ-RTE-hook-context` and `CHQDEV NATIVE RECONSTRUCT: PASS`; upload `C:\Projects\Evidence\CHQ6324.zip` and log, inspect the actual Ghidra/ROM evidence manifest and all five cases. On pass only, run `.\chqdev.bat qualify --fresh` on this exact source for 18/18 Windows CTests, focused and Full Workbench regressions and distribution identity. Keep RC6.3.23 immutable.

## Backlog

Improve exception stack bus observability and interrupted PC/SR provenance with IRQ vector attribution, investigate `0x0518` under controlled original-ROM entry states, close CPU-B function CFG and bounded native differential parity, then consider failure minimiser and guarded native composition; never conflate sampled hook parity with completed promotion.

Release hygiene: preserve CRLF/LF line-ending compatibility across Windows PowerShell/bat scripts, release guards and handover documentation.
