> **RC6.3.33 authoritative current release metadata.** This document retains its historical content; current version is 0.67.0.0-RC6.3.33. See docs/RC6.3.33_GENUINE_SUBPROCESS_REPLAY_AND_ROLLBACK.md.

> Current development candidate: ChaseHQ-Native 0.67.0.0-RC6.3.32; original-ROM CPU-B writer results from RC6.3.31 are genuine but full function CFG/native promotion remain blocked. Check docs/RC6.3.32_CHECKPOINT_REPLAY_AND_FUNCTION_BOUNDARY_CONFIDENCE.md.

> CURRENT RELEASE 0.67.0.0-RC6.3.31: see `docs/RC6.3.31_AUTONOMOUS_FUNCTION_BLOCKERS_AND_DIAGNOSIS.md`; real Windows new writer capture PENDING, complete function native promotion BLOCKED; older sections historical.

> CURRENT BUILD: 0.67.0.0-RC6.3.30 — read-only BTST bit7 source-verification candidate; Windows ROM gate pending; historical body follows.

> **RC6.3.30 current source-candidate context (2026-10-15):** immutable RC6.3.29 Windows-qualified baseline; new 0x050E BTST byte evidence Windows execution pending, no whole-function native promotion. Historical discussion below.

> **Current authoritative version 0.67.0.0-RC6.3.29:** Read-only 0x0516 CPU-B BNE analyser corrective. Genuine CHQ6328 original-ROM 35/35 equal-clock immediate next-PC samples; Windows acceptance pending, 0x0518 unobserved, no native whole-function promotion. See docs/RC6.3.29_CPU_B_EQUAL_CYCLE_CORRECTIVE.md. Historical documentation below preserved.

> ChaseHQ-Native 0.67.0.0-RC6.3.28 current active source candidate; historic details retained below.

> RC6.3.28 current active source candidate (Windows qualification pending); historical details below.

> **Release provenance:** 0.67.0.0-RC6.3.27 current source candidate; this document preserves historical analysis and does not independently qualify new IRQ bus evidence.

# RC6.3.26 — Corrective dual-stack CPU-B exception-frame restoration

**Release:** `0.67.0.0-RC6.3.26` / `chqdev v0.5.45`. Baseline is exact immutable RC6.3.25 source SHA256 `64e62b77b118577cecfa2d0d4c3ed6eb57ccc456344f11e6a3229a09e0d525b8`, which built successfully on Windows but **FAILED the new M2 original-ROM exception-frame restoration gate**. Latest fully Windows-qualified baseline: RC6.3.24 (Windows distribution hash reported by qualify `78bc9b7fdaa00a55294f55099c18df82ed69e07b0d53e57b0bffded3308de014`, dist bytes not separately uploaded). RC6.3.26 remains **UNQUALIFIED** until genuine Windows ROM replay and `qualify --fresh` pass.

## Genuine RC6.3.25 failure and 68000 register-bank correction

RC6.3.25 `CHQ6325.zip` contained genuine five-case ROM/Ghidra and CPU-B exception-frame snapshots (seven handler/RTE/resume triples each). At the natural first RTE ordinal 14: pre-RTE active A7 `0x1003FA`, saved SR `0x0004` (supervisor S=0), saved PC `0x0004D8`, post-RTE SR `0x0004`, post-RTE PC `0x0004D8` and active A7 `0x100800`. The frame's 6-byte supervisor-pop expectation is `ISP 0x100400`, **not active SP**, which selects USP when S is clear. All five original-ROM cases showed the consistent user-mode switch. RC6.3.25 wrongly compared `post.activeSP == pre.activeSP + 6`. Changing that to skip the SP check altogether would silently weaken an existing corrupted-SP negative test, so it is **not accepted**.

RC6.3.26 updates read-only original-ROM instruction-hook recording to sample **active SP, USP and ISP** independently at `0x0474` handler, `0x04D6` RTE and the immediately next CPU-B hook. Hook export schema is bumped to `chq-rc6326-cpu-b-exception-frame-cap-v2` and ten-column TSV, so old v1 evidence cannot masquerade as new bank observations. The fail-closed M2 analyser checks: selected authentic chronology, opcodes, frame-byte stability, pre-RTE active SP equals ISP, preserved USP from entry through RTE and restoration, popped ISP = pre-RTE ISP+6, saved PC/SR equals immediately resumed PC/SR, and `post.activeSP == post.USP` for S=0 **or** `post.activeSP == post.ISP` for S=1. Supervisor and user-mode synthetic positive tests plus corrupted USP, ISP, active SP, PC/SR, missing hooks and dropped evidence all reject as expected. The new report retains `nativePromotionAllowed=false`, and explicitly records sampled bank-switch versus prohibited bus/vector/IRQ scheduler parity claims.

## Evidence provenance and non-certifications

The RC6.3.25 Windows ZIP is **real** but lacks USP/ISP observations. New v2 bank-aware tests are **synthetic until fresh RC6.3.26 Windows execution**. Nothing establishes hardware vector ACK source, ordered 68000 exception-frame bus reads/writes, all IRQ/scheduler paths, complete CPU-B function CFG/caller/return set, unobserved `0x0518` branch or autonomous native promotion. The native replacement code remains unchanged. Keep the latest qualified RC6.3.24 and failed RC6.3.25 source archives immutable and distinct.

## Windows acceptance, one-line PowerShell

From RC6.3.26 extracted project root:

`New-Item -ItemType Directory -Force 'C:\Projects\Evidence' | Out-Null; .\chqdev.bat build debug; if ($LASTEXITCODE -eq 0) { .\chqdev.bat native reconstruct --fresh --live-health --cpu-b-live --out C:\Projects\Evidence\CHQ6326 }`

Require `[PASS] M2-CPU-B-original-ROM-exception-frame-restoration` and `CHQDEV NATIVE RECONSTRUCT: PASS` on this **exact build**, upload `CHQ6326.zip` and console output and verify v2 per-case USP/ISP provenance. Only then run `.\chqdev.bat qualify --fresh` for 18/18 Windows CTests, deterministic package, Workbench focused and Full Regression and authoritative run-history. Output remains at `C:\Projects\Evidence`, with no redundant `-SHORT` suffix.

Release hygiene: preserve ASCII-compatible CRLF/LF line endings for Windows PowerShell and batch tooling; chqdev remains authoritative.
