> **RC6.3.33 authoritative current release metadata.** This document retains its historical content; current version is 0.67.0.0-RC6.3.33. See docs/RC6.3.33_GENUINE_SUBPROCESS_REPLAY_AND_ROLLBACK.md.

> Current development candidate: ChaseHQ-Native 0.67.0.0-RC6.3.32; original-ROM CPU-B writer results from RC6.3.31 are genuine but full function CFG/native promotion remain blocked. Check docs/RC6.3.32_CHECKPOINT_REPLAY_AND_FUNCTION_BOUNDARY_CONFIDENCE.md.

> CURRENT RELEASE 0.67.0.0-RC6.3.31: see `docs/RC6.3.31_AUTONOMOUS_FUNCTION_BLOCKERS_AND_DIAGNOSIS.md`; real Windows new writer capture PENDING, complete function native promotion BLOCKED; older sections historical.

> CURRENT BUILD: 0.67.0.0-RC6.3.30 — read-only BTST bit7 source-verification candidate; Windows ROM gate pending; historical body follows.

> **RC6.3.30 current source-candidate context (2026-10-15):** immutable RC6.3.29 Windows-qualified baseline; new 0x050E BTST byte evidence Windows execution pending, no whole-function native promotion. Historical discussion below.

> **Current authoritative version 0.67.0.0-RC6.3.29:** Read-only 0x0516 CPU-B BNE analyser corrective. Genuine CHQ6328 original-ROM 35/35 equal-clock immediate next-PC samples; Windows acceptance pending, 0x0518 unobserved, no native whole-function promotion. See docs/RC6.3.29_CPU_B_EQUAL_CYCLE_CORRECTIVE.md. Historical documentation below preserved.

# RC6.3.28 — original-ROM CPU-B BNE at 0x0516: flags and immediate next PC

**Candidate:** 0.67.0.0-RC6.3.28, chqdev v0.5.47. **Fully Windows-qualified immutable baseline:** RC6.3.27 source SHA256 `deaf140c0ee129804785ee2c9026af240d7ee9d5806c77095eead93795932b39`; Windows `dist` SHA256 `828876f06979ce6f08b3970374e796abbc03b1980d561f7e09c32602ea5e5604` reported by chqdev (archive bytes not independently supplied). Genuine RC6.3.27 native reconstruction, 18/18 Windows CTests, focused ten and Full Regression 1153 commands PASS, session 20261015_164508.

## Real previous evidence and unresolved 0x0518

Actual genuine `CHQ6327.zip` original-ROM CPU-B ordered hook and aggregate edge traces show precisely **seven 0x0516 BNE.B visits in each of natural, bypass, nonterminal, terminal and overflow** (35 total). Original opcode is `0x6614`; every observed saved CPU status register has `SR.Z=0` (sampled raw SR includes 0x0000 and 0x0008). Aggregate observed successor `0x052C` occurs seven times per case; the not-taken fallthrough `0x0518` occurs **zero** times. Existing Ghidra statics identify that alternative, not its execution. This is a real branch-reachability gap, not license to report both paths proven.

## New read-only capture / strict audit

`src/runtime.cpp` now captures every CPU-B original-ROM 0x0516 instruction entry with frame, transition ordinal, `cyclesB` and the actual prebranch `SR`; at the **immediately next genuine CPU-B instruction hook**, it attaches the actual next PC and frame/cycle values. The stream is bounded and opt-in only under `--cpu-b-edge-probe`; no CPU registers or RAM are changed. Each original checkpoint case exports `cpu-b-original-0516-bne-decisions.tsv` and `cpu-b-original-0516-bne-cap.json`. Cap schema `chq-rc6328-cpu-b-0516-bne-cap-v1`, exact hits/recorded, dropped=0, no pending, matching CPU transitions required.

`M2-CPU-B-original-ROM-0516-BNE-condition-and-next-PC` crosschecks this new chronological per-visit evidence with independent original-ROM selected hook SR/opcode/ordinal/frame/cycles, original ROM edge counts and the authenticated Ghidra call-return/continuation preconditions. For `BNE.B 0x6614`, `SR.Z=0` demands next-PC 0x052C; `SR.Z=1` demands 0x0518. Any inconsistent flags/next-PC, missing visit, dropped event, duplicate, truncated/altered cap, unexpected interrupt diversion or newly exercised 0x0518 **fails closed**; a new authentic branch needs its own path-specific contract and Ghidra/ABI investigation. Full CFG, unseen indirect caller set, native promotion, hardware interrupt timing and scheduler parity remain BLOCKED (`nativePromotionAllowed=false`).

## Provenance labels and local tests

Previous genuine RC6.3.27 Ghidra/edge/hook evidence is replayed exactly; **the new next-PC TSV rows are SYNTHETIC in the local integration test**, seeded explicitly to validate the analyser without pretending the new Windows observation has happened. Mutation tests reject incorrect Z flag, PC, opcode, order, timing, duplicate ordinals, missing streams and dropped/pending caps. Linux native CTest / Go suite / Windows crossbuild / deterministic package must PASS. Newly instrumented C++ capture is not independently proven until the Windows original-ROM run.

## Acceptance

One-line PowerShell from extracted RC6.3.28 root:

`New-Item -ItemType Directory -Force 'C:\Projects\Evidence' | Out-Null; .\chqdev.bat build debug; if ($LASTEXITCODE -eq 0) { .\chqdev.bat native reconstruct --fresh --live-health --cpu-b-live --out C:\Projects\Evidence\CHQ6328 }`

Expect `[PASS] M2-CPU-B-original-ROM-0516-BNE-condition-and-next-PC`, `CHQDEV NATIVE RECONSTRUCT: PASS` and `C:\Projects\Evidence\CHQ6328.zip`. Only then run `.\chqdev.bat qualify --fresh`. Confirm 18/18 Windows CTests, focused regression, Full Regression and evidence manifest. RC6.3.27 remains fully qualified until fresh RC6.3.28 acceptance; do not relabel immutable archives.

Release process preserves ASCII and **LF/CRLF** conventions for Windows compatibility; package manifest, `chqdev qualify --fresh` and immutable evidence are the release authority.
