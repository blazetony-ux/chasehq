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

> **AUTHORITATIVE CURRENT — ChaseHQ-Native 0.67.0.0-RC6.3.23 ORIGINAL-ROM CPU-B ORDINAL-ZERO CORRECTIVE SOURCE CANDIDATE (2026-10-15).** Exact immutable RC6.3.22 source archive SHA-256 `c17673a958446875f047bfd1dceab5675ff5e81084949578615edc9929c3684e` passed local tests and Windows build but **FAILED genuine Windows native acceptance** in `M2-CPU-B-ordered-original-ROM-stack-audit`: first authentic selected hook has zero-based ordinal `0`, incorrectly rejected by `ordinal == 0`. RC6.3.23 removes only this incorrect prohibition while retaining strict increasing ordinals, transition cap, original opcode, sampled SP/return, IRQ separation and fail-closed blockers. Authentic failed `CHQ6322.zip` five-case ROM+Ghidra integration replay now PASS; duplicate-zero and malformed-order tests reject. **Latest fully Windows-qualified baseline RC6.3.21**, Windows qualifier SHA-256 `fbfc6c19131a3a10a28de60449a9716111179fe03cc4285f2f970a0e8646ea3a` (not independently byte-verified). RC6.3.23 Windows native acceptance and `qualify --fresh` PENDING; `--out C:\Projects\Evidence\CHQ6323`. Unobserved `0x0518`, full CPU-B CFG/IRQ stack parity and native promotion BLOCKED. See `docs/RC6.3.23_CPU_B_ORDINAL_ZERO_CORRECTIVE.md`. Earlier banners below are historical.

# RC6.3.22 — CPU-B sampled chronological call/return evidence

**Release** `v0.67.0.0-RC6.3.22`, `chqdev v0.5.41`. Baseline: immutable RC6.3.21 source ZIP SHA-256 `bdfbe71f911b2e283132410b3c22fb4159ab044baabb00a4298af3d76d93cd54` and Windows qualifier-reported release ZIP SHA-256 `fbfc6c19131a3a10a28de60449a9716111179fe03cc4285f2f970a0e8646ea3a`. RC6.3.21 has fully passed native ROM/Ghidra evidence, fresh 18/18 Windows CTest, focused 10 commands and Full Regression 1,153 commands.

## Substantive implementation

RC6.3.21's `cpu-b-observed-edges.tsv` contained aggregate visit counts **without time order**. RC6.3.22 now additionally records the true CPU-B instruction-hook execution order from the ROM oracle at **selected PC sites**, into `cpu-b-ordered-hook-events.tsv`. Each real hook row captures ordinal (counting all CPU-B hooks, not just selected), frame, cumulative CPU-B cycles, PC, opcode, SP and SR. This is an opt-in read-only observation when `--cpu-b-edge-probe` is active, and cannot change emulated RAM, registers, or timing decisions. The report's sibling `cpu-b-ordered-hook-cap.json` records how many filtered events were saved/dropped and the total CPU-B transitions, making saturation or truncation fail closed.

The newly required `M2-CPU-B-ordered-original-ROM-stack-audit` gate reads the **five genuine, freshly captured ROM execution samples** (natural, bypass, nonterminal, terminal, overflow). Its `cpu-b-ordered-original-rom-stack-audit.json` proves only the sampled chronological chain of four nested invocations per case: `0x08A6 JSR -> 0x4F70 BSR -> 0x4FB2 -> 0x5062 RTS -> 0x4F74 BSR -> 0x4FB2 -> 0x5062 RTS -> 0x4F78 BSR -> 0x4FB2 -> 0x5062 RTS -> 0x4F7C RTS -> 0x08AC`. It verifies order, original opcodes, sampled SP restoration, strict ordinal ordering and sample-count reconciliation against the existing genuine Ghidra/edge audit.

Instruction hooks `0x0474` and `0x04D6` are labelled IRQ-vector/RTE observations and **never counted as normal subroutine pushes/pops**. This is not complete IRQ exception-stack provenance or bus transaction parity; intervening instructions that are outside the selected set are not exhaustively captured. The `0x0518` alternative is explicitly absent/untested in original checkpoint execution; seeing that branch without additional classification fails this bounded gate. No complete function or automated promotion is permitted.

## Testing boundaries

- Local Go structural and mutation tests include wrong return order, changed opcode, wrong stack pointer, duplicated ordinal, cap overflow and unexpected alternate branch. The local chronological fixtures are **SYNTHETIC**; they do **not** claim ROM temporal evidence before Windows execution.
- Independent tests combine unchanged genuine RC6.3.21 original-ROM **aggregate** edge TSVs with explicitly synthetic chronological fixtures. Linux CPU-only CTest, build, portable preflight, deterministic package and windows tooling are local gates.
- **Only Windows** `chqdev native reconstruct --fresh --live-health --cpu-b-live` establishes the five actual CPU-B *ordered* traces and checks them against the genuine original ROM. Do not promote based on fixture replay.

## Windows handoff

From the extracted RC6.3.22 project root run this single PowerShell line:

`New-Item -ItemType Directory -Force 'C:\Projects\Evidence' | Out-Null; .\chqdev.bat build debug; if ($LASTEXITCODE -eq 0) { .\chqdev.bat native reconstruct --fresh --live-health --cpu-b-live --out C:\Projects\Evidence\CHQ6322 }`

Inspect the complete `C:\Projects\Evidence\CHQ6322.zip` and console log. If Windows native reconstruction genuinely PASS, run `.\chqdev.bat qualify --fresh` to check Windows tests, deterministic packaging and both Workbench regressions.

## Unproven and intentionally blocked

Complete CPU-B function entry/callers/CFG/exits, full stack memory/IRQ/RTE scheduling and ordered bus transactions, all indirect targets, unobserved alternate `0x0518`, whole-machine native/original/fallback multi-function composition, automatic counterexample minimisation, and autonomous native promotion. Backlog documented in Workbench Next Steps and Roadmap.

Release hygiene: Preserve CRLF/LF handling for both Windows and Linux; documentation and scripts must not regress LF/CRLF parsing.
