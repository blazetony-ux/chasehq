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

# RC6.3.25 - Bounded CPU-B exception-frame RAM and restored context

**Version**: `0.67.0.0-RC6.3.25`, `chqdev v0.5.44`. Immutable fully Windows-qualified RC6.3.24 source SHA-256 `cf1e9c75755c8ee13f8786c552bdf58e67ca930d86a2d5960c179745dd89b1d3`; reported Windows dist SHA-256 `78bc9b7fdaa00a55294f55099c18df82ed69e07b0d53e57b0bffded3308de014` (the dist ZIP itself was not uploaded). New RC6.3.25 local Linux source candidate **not Windows-qualified**.

## Instrumentation and evidence

In original-ROM CPU-B instruction hooks with `--cpu-b-edge-probe`, sample the six-byte stack RAM (saved SR.W at SP, saved PC.L at SP+2) at handler instruction PC `0x0474` and RTE instruction PC `0x04D6`. On the **immediately subsequent actual CPU-B instruction hook** record restored CPU PC, SR and SP. Capture ordinal, frame, sampled context and capped/drop state without altering mapped memory, execution or native dispatch. Files, per original-ROM five-case checkpoint:

- `branch-matrix/<case>/live-health/original/cpu-b-exception-frame-hooks.tsv`
- `branch-matrix/<case>/live-health/original/cpu-b-exception-frame-cap.json`

New M2 gate `M2-CPU-B-original-ROM-exception-frame-restoration` generates `cpu-b-original-rom-exception-frame-restoration.json`, gated by RC6.3.24's authentic selected handler/RTE chronology. It rejects changed memory snapshots, reordered/lost entries, bad next hook PC/SR, bad restored SP, saturated captures, and missing/incorrect original-ROM RTE edge evidence. The report's success flags cover **only sampled hook RAM bytes and immediately restored original-ROM CPU context**.

## Verification limitations

The prior authentic **`CHQ6324.zip`** has five real original-ROM/Ghidra cases, 7 handler/RTE pairs each (35 total) and 4/3 original RTE returns to `0x04D8`/`0x04DE` per case. The NEW frame peek values and actual restored CPU state were not recorded by RC6.3.24. Local tests use explicitly **synthetic** frame rows and negative mutations; genuine prior ROM/Ghidra replay MUST reject missing RC6.3.25 frame evidence. New Windows `CHQ6325.zip` is mandatory to verify the new state with authentic execution.

A RAM `bus.peek` **is not** the bus operations performed by the emulated CPU. This release does not prove original interrupt-stack write/read order, hardware vector number/ACK ownership, IRQ completeness, scheduler parity, entire CPU-B function CFG, branch `0x0518` execution or full-function native substitution. `nativePromotionAllowed=false`, `completeFunctionVerified=false`. Existing bounded one/two-instruction native execution is unchanged.

## One-run Windows acceptance

From extracted RC6.3.25 project root, one-line PowerShell:

`New-Item -ItemType Directory -Force 'C:\Projects\Evidence' | Out-Null; .\chqdev.bat build debug; if ($LASTEXITCODE -eq 0) { .\chqdev.bat native reconstruct --fresh --live-health --cpu-b-live --out C:\Projects\Evidence\CHQ6325 }`

Require `[PASS] M2-CPU-B-original-ROM-exception-frame-restoration` and `CHQDEV NATIVE RECONSTRUCT: PASS`; upload `CHQ6325.zip` and console log. Only then run ` .\chqdev.bat qualify --fresh ` and verify 18/18 Windows CTests, focused regression Run 001 and full regression Run 002. Keep RC6.3.24 immutable.

## Next investigation

Capture actual bus-ordered stack exception transitions and IRQ ACK/vector ownership, correlate preemption timing, investigate `0x0518`, complete function CFG/indirect targets and add first-divergence minimiser before guarded multi-routine native composition.

Release hygiene: preserve ASCII-compatible CRLF/LF line endings for Windows PowerShell and batch tooling; keep chqdev the authoritative build/release controller.
