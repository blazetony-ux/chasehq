> **CURRENT AUTHORITATIVE — ChaseHQ-Native 0.67.0.0-RC6.3.33 / chqdev v0.5.52 / REAL CHECKPOINT GENUINE NATIVE DIFFERENTIAL (2026-10-15).** Qualified immutable RC6.3.32 source SHA256 `3f72489f25b144cda5ca7d7416670b02e7f7fa02b70fc32811320943b75bccb6`; Windows dist SHA256 `cb4582d8abcac2f615de51a99065fb54f45d1f06cbed271135e146fe4f7e4c9e` reported by qualifying tool only. RC6.3.33 extends synthetic replay with independent **GENUINE ROM** checkpoint processes: original, real Ghidra-generated CPU-B two-opcode native substitution and forced rollback at frames 6337 and 6340. Each captures five semantic streams (CPU flags/registers/cycles, mapped R16 bus, IRQ events, machine state and raw bus). All signals must match; native commit and explicit fallback must be observed. Fresh high-frame replay verifies determinism, hashes pin checkpoint/executable/DLL. Unexpected divergence invokes genuine-subprocess prefix reduction and leaves a first-difference witness. An independent mutation of a COPIED trace is separately labelled CONTROLLED NEGATIVE, not genuine gameplay. No whole-function CFG/0x0518/producer coverage/native promotion. **Source candidate: fresh RC6.3.33 Windows ROM acceptance/Full Regression PENDING.** See `docs/RC6.3.33_GENUINE_SUBPROCESS_REPLAY_AND_ROLLBACK.md`. Previous current headers are historical.

> **AUTHORITATIVE CURRENT — ChaseHQ-Native 0.67.0.0-RC6.3.32 CHECKPOINT REPLAY REDUCTION AND CPU-B FUNCTION-CONFIDENCE (2026-10-15).** Exact immutable fully Windows-qualified RC6.3.31 source SHA256 `8ca40c81ba661a0a3274ce4203dd541fe34d39743cc914c2a63b07b037061cd4`; qualifier-reported Windows dist SHA256 `004f00ae527e5bacc8cb0791840808ff4e5e4da352c72fb2e37cd9ac8a6bfc76` separate and not independently hashed. Genuine CHQ6331 original-ROM evidence captures CPU-B PC 0x04A8 writing byte 0x80 at address 0x100802 seven times per case (35 total, zero bit7 transitions), then BTST at 0x050E and BNE at 0x0516→0x052C; 0x0518 remains unobserved. RC6.3.32 introduces actual paired original/native executable checkpoint replay via chqdev native minimize --plan JSON --out NEW_DIR: isolated ordered traces, executable/checkpoint hashes, bounded frame-prefix bisection, fresh confirmation reruns and fail-closed errors. The automatic native-reconstruct smoke uses **SYNTHETIC process fixtures only**; no full-function original/native gameplay replay certified locally. A genuine writer dependency report preserves incomplete producer history, function entry/CFG/exits/callers and scheduler uncertainty; nativePromotionAllowed=false. Windows acceptance CHQ6332.zip and fresh qualify PENDING; `chqdev v0.5.51`. See `docs/RC6.3.32_CHECKPOINT_REPLAY_AND_FUNCTION_BOUNDARY_CONFIDENCE.md`. Earlier banners below historical.

> **AUTHORITATIVE CURRENT — ChaseHQ-Native 0.67.0.0-RC6.3.31 MULTI-SOURCE FUNCTION FRONTIER AND DIAGNOSIS (2026-10-15).** Exact immutable fully Windows-qualified RC6.3.30 source ZIP SHA256 `24e7cf0fa32741f584d74908b7b707afc02797a699f9d40e1ab5c8dd32313a82`; Windows qualifier-reported `dist` SHA256 `51067d3a2dd9a8d370886619e18c6a0427c207d9f9517ece241e5ed374065ea1` separate, not independently hashed. Genuine CHQ6330.zip proves 35/35 sampled CPU-B 0x050E byte=0x80, bit7=1 -> SR.Z=0 -> 0x0516->0x052C; 0x0518 still unobserved. RC6.3.31 adds opt-in read-only CPU-A/B postcommit Bus::write witnesses overlapping 0x100802 (records zero honestly), plus automatic cross-case Ghidra/observed-edge function blocker dossier. New trace-witness CLI diagnoses the first differing original/native evidence row and shows SHA-pinned context, **not** verified deterministic scenario/input minimization. Fresh Windows writer capture `CHQ6331.zip` and `qualify --fresh` PENDING; local fixtures for NEW capture synthetic-only. Whole-function CFG, unobserved 0x0518, CPU-A/B scheduling and function-native promotion BLOCKED. `chqdev v0.5.50`. See `docs/RC6.3.31_AUTONOMOUS_FUNCTION_BLOCKERS_AND_DIAGNOSIS.md`. Older current labels below historical.

> **AUTHORITATIVE CURRENT — ChaseHQ-Native 0.67.0.0-RC6.3.30 CPU-B 0x050E BTST UPSTREAM SOURCE / 0x0516 BNE (2026-10-15).** Built from immutable fully Windows-qualified RC6.3.29 source ZIP SHA256 `087e973345581eb8603bba99ae69907a3403885449a21d4b9063e745eb094615`; Windows qualifier-reported `dist` SHA256 `4e2150384994fbb05876ff03b666fce6542cafc38ebdf1a2868ce1287bc3c6e8` is not independently byte-verified. Genuine CHQ6329.zip proves 35/35 original-ROM 0x050E instruction hooks immediately preceding 0x0516 BNE and 35 0x0516 Z-clear->0x052C branches (5 checkpoints). RC6.3.30 introduces **NEW original-ROM byte capture** from 0x100802 immediately before Ghidra `BTST.B #7,(0x00100802).L` at 0x050E; verifies bit7 against 0x0516 SR.Z, ordered hooks and actual BNE next PC. New-byte observations **synthetic-only in local fixtures**; fresh Windows ROM evidence `CHQ6330.zip` and `qualify --fresh` PENDING. Unmodified original evidence vs controlled data always distinct. `0x0518` remains unobserved, full CFG and native whole-function promotion BLOCKED. `chqdev v0.5.49`. Read `docs/RC6.3.30_CPU_B_BTST_UPSTREAM_BIT7_SOURCE.md`. Older banners below historical.

> **AUTHORITATIVE CURRENT — ChaseHQ-Native 0.67.0.0-RC6.3.29 CPU-B BNE EQUAL-CLOCK CORRECTIVE SOURCE CANDIDATE (2026-10-15).** Source comes from immutable RC6.3.28 ZIP SHA256 `18b2a4a5c32aa750e57eb525fc6b29697b7db77acca420268e084f7ce8a3ab4f` (Windows MSVC build PASS but native BNE decision audit FAIL on legitimate equal `cyclesB`). Genuine original-ROM `CHQ6328.zip` verified 1007/1007 and isolated corrected complete Ghidra/branch audit PASS: 35 real per-visit `0x0516` Z-clear BNE next-PCs `0x052C`, 35 equal-clock pairs, no `0x0518`. RC6.3.29 preserves strict lower-cycle rejection, opcode, flag, ordinal and original-ROM edge checks, plus new optional genuine-evidence regression. Latest fully Windows-qualified immutable baseline remains **RC6.3.27**, source SHA256 `deaf140c0ee129804785ee2c9026af240d7ee9d5806c77095eead93795932b39`; RC6.3.28 FAILED, RC6.3.29 Windows native+fresh qualify **PENDING**. New focused Run001 and 1153-command Full Regression required on Windows, one evidence ZIP `C:\Projects\Evidence\CHQ6329.zip`. Complete CPU-B function CFG, alternative `0x0518` execution, hardware timing and whole-function native promotion remain BLOCKED. See `docs/RC6.3.29_CPU_B_EQUAL_CYCLE_CORRECTIVE.md`. Older banners are historical.

> **AUTHORITATIVE CURRENT — ChaseHQ-Native 0.67.0.0-RC6.3.28 RC6.3.28 CPU-B ORIGINAL-ROM BNE CONDITION/IMMEDIATE NEXT-PC SOURCE CANDIDATE (2026-10-15).** Immutable fully Windows-qualified RC6.3.27 source SHA256 `deaf140c0ee129804785ee2c9026af240d7ee9d5806c77095eead93795932b39`; qualifier-reported Windows distribution SHA256 `828876f06979ce6f08b3970374e796abbc03b1980d561f7e09c32602ea5e5604` (actual Windows `dist` archive not independently supplied). Genuine `CHQ6327.zip` passed the previous IRQ bus gate and 18/18 Windows CTests; focused Run001 10 commands, Full Regression Run002 1,153 commands PASS. Genuine five-case original-ROM CPU-B 0x0516 BNE visits: 7 per case, 35 total, all SR.Z clear and observed aggregate successor 0x052C; **0x0518 was never visited**. RC6.3.28 independently records each BNE's original-ROM pre-branch SR/ordinal/frame and immediately succeeding CPU-B instruction PC, crosschecks original-ROM hook and Ghidra/edge evidence, and fails closed on mismatched flags/targets, dropped records or newly observed 0x0518 requiring separate review. **New per-visit successor capture is synthetic-only in local tests, real RC6.3.27 SR/edge prerequisites verified; fresh RC6.3.28 Windows ROM acceptance and `qualify --fresh` PENDING.** Function CFG/callers, 0x0518 runtime path, emulator cycle-exact IRQ timing, native function and autonomous promotion remain BLOCKED. One evidence ZIP: `C:\Projects\Evidence\CHQ6328.zip`. See `docs/RC6.3.28_CPU_B_BNE_CONDITION_AND_NEXT_PC.md`. Older banners retained as history.

> **AUTHORITATIVE CURRENT — ChaseHQ-Native 0.67.0.0-RC6.3.27 CPU-B ORIGINAL-ROM IRQ VECTOR AND ORDERED STACK BUS SOURCE CANDIDATE (2026-10-15).** Exact source baseline RC6.3.26 SHA256 `aee45c18268daa006f13a0bebaed43f61e9dbd804a63cf1d34b426ae12650386` is fully Windows qualified: genuine native reconstruction PASS, 18/18 CTests, focused 10-command and Full Regression 1153-command PASS; Windows qualifier-reported dist hash `7cb0ebcfe2ef5076e058a2b1f0d82f29f244fa45cc476f16f6f3561072eff6cf` (actual dist ZIP not independently provided). RC6.3.27 uses actual Musashi ACK and CPU-B bus read/write callbacks to record a bounded original-ROM interrupt-entry vector read and PC/SR stack writes, closes at the first handler instruction, and fail-closed crosschecks independent genuine RC6.3.26 exception-frame/USP/ISP snapshots. **NEW bus capture is synthetic-only in local tests; fresh RC6.3.27 Windows ROM acceptance and qualify --fresh are PENDING.** External IRQ ownership, timing, full IRQ scheduling, unobserved 0x0518, full function and native promotion remain BLOCKED. Evidence `C:\Projects\Evidence\CHQ6327.zip`. See `docs/RC6.3.27_CPU_B_IRQ_VECTOR_BUS_ORDER.md`. Older banners below historical.

> **AUTHORITATIVE CURRENT — ChaseHQ-Native 0.67.0.0-RC6.3.26 CPU-B DUAL-STACK EXCEPTION RETURN CORRECTIVE SOURCE CANDIDATE (2026-10-15).** Latest fully Windows-qualified baseline RC6.3.24, qualifier-reported dist SHA256 `78bc9b7fdaa00a55294f55099c18df82ed69e07b0d53e57b0bffded3308de014`. RC6.3.25 exact source SHA256 `64e62b77b118577cecfa2d0d4c3ed6eb57ccc456344f11e6a3229a09e0d525b8` is **FAILED Windows ROM acceptance**: handler `0x0474` and RTE `0x04D6` returned to CPU-B user SR `0x0004`, restored PC `0x04D8/0x04DE` correctly, but active A7 switched to USP `0x100800` rather than popped ISP `0x100400`; the original RC6.3.25 verifier incorrectly insisted on `SP+6` for active A7. RC6.3.26 instruments and verifies original-ROM USP and ISP independently, plus saved frame, post-RTE PC/SR and bank-selected active SP; fail-closed synthetic user/supervisor mutations PASS. Prior genuine CHQ6325 evidence supports diagnosis, NOT NEW dual-bank capture. **Fresh RC6.3.26 Windows ROM native and `qualify --fresh` remain PENDING.** `0x0518`, vector/bus timing, full function/native promotion remain BLOCKED. Evidence: `C:\Projects\Evidence\CHQ6326.zip`. See `docs/RC6.3.26_CPU_B_DUAL_STACK_BANK_CORRECTIVE.md`. Previous banners are historical.

> **AUTHORITATIVE CURRENT - ChaseHQ-Native 0.67.0.0-RC6.3.25 CPU-B EXCEPTION-FRAME RAM/POST-RTE RESTORATION SOURCE CANDIDATE (2026-10-15).** Latest fully Windows-qualified immutable baseline **RC6.3.24**, exact source ZIP SHA-256 `cf1e9c75755c8ee13f8786c552bdf58e67ca930d86a2d5960c179745dd89b1d3`; reported Windows dist SHA-256 `78bc9b7fdaa00a55294f55099c18df82ed69e07b0d53e57b0bffded3308de014` (Windows package bytes not independently uploaded). RC6.3.24 genuine native and 18/18 Windows CTests, focused 10 commands and Full Regression 1,153 commands PASS. RC6.3.25 adds **new read-only original-ROM CPU-B six-byte exception frame RAM peeks** at 0x0474/0x04D6 and post-RTE next-hook PC/SR/SP crosscheck. **Authentic CHQ6324 prerequisite 35/35 PASS; NEW frame data is synthetic in local tests and genuine Windows qualification PENDING.** Not a bus transaction trace, vector ACK, all IRQs, scheduler or native function promotion. `0x0518` remains unobserved; `--out C:\Projects\Evidence\CHQ6325`. See `docs/RC6.3.25_CPU_B_EXCEPTION_FRAME_RESTORATION.md`. Older banners below historical.

> **AUTHORITATIVE CURRENT — ChaseHQ-Native 0.67.0.0-RC6.3.24 ORIGINAL-ROM CPU-B IRQ/RTE SELECTED-HOOK CONTEXT SOURCE CANDIDATE (2026-10-15).** Latest fully Windows-qualified immutable baseline **RC6.3.23**, exact source ZIP SHA-256 `9d7b33423e6c7f022059e77eca233f7cdae9b497ab5fedbd0eb60765ab75c55a`, qualifier-reported Windows distribution SHA-256 `3586f6fada3fc56384fb7a213f9c289a68b998427f22a3bf962ae81c28636e5b` (distribution bytes not independently uploaded). RC6.3.23 Windows native, 18/18 CTest and Full Regression 1153/1153 PASS. RC6.3.24 adds fail-closed `M2-CPU-B-original-ROM-IRQ-RTE-hook-context`: authentic CHQ6323.zip selected CPU-B 0x0474 handler-entry and 0x04D6 RTE observations, 7 sampled pairs/case across five cases (35), supervisor-mode, opcode, frame, order and sampled SP consistency. Actual exception-frame bytes/bus transactions, vector ownership, interrupted SR/PC, all IRQs, scheduler and native function promotion NOT VERIFIED. **RC6.3.24 ROM-backed Windows acceptance and `qualify --fresh` PENDING.** Evidence output `C:\Projects\Evidence\CHQ6324.zip`; `0x0518` remains unobserved. See `docs/RC6.3.24_CPU_B_IRQ_RTE_HOOK_CONTEXT.md`. Older status banners below are historical.

> **AUTHORITATIVE CURRENT — ChaseHQ-Native 0.67.0.0-RC6.3.23 ORIGINAL-ROM CPU-B ORDINAL-ZERO CORRECTIVE SOURCE CANDIDATE (2026-10-15).** Exact immutable RC6.3.22 source archive SHA-256 `c17673a958446875f047bfd1dceab5675ff5e81084949578615edc9929c3684e` passed local tests and Windows build but **FAILED genuine Windows native acceptance** in `M2-CPU-B-ordered-original-ROM-stack-audit`: first authentic selected hook has zero-based ordinal `0`, incorrectly rejected by `ordinal == 0`. RC6.3.23 removes only this incorrect prohibition while retaining strict increasing ordinals, transition cap, original opcode, sampled SP/return, IRQ separation and fail-closed blockers. Authentic failed `CHQ6322.zip` five-case ROM+Ghidra integration replay now PASS; duplicate-zero and malformed-order tests reject. **Latest fully Windows-qualified baseline RC6.3.21**, Windows qualifier SHA-256 `fbfc6c19131a3a10a28de60449a9716111179fe03cc4285f2f970a0e8646ea3a` (not independently byte-verified). RC6.3.23 Windows native acceptance and `qualify --fresh` PENDING; `--out C:\Projects\Evidence\CHQ6323`. Unobserved `0x0518`, full CPU-B CFG/IRQ stack parity and native promotion BLOCKED. See `docs/RC6.3.23_CPU_B_ORDINAL_ZERO_CORRECTIVE.md`. Earlier banners below are historical.

> **AUTHORITATIVE CURRENT — ChaseHQ-Native 0.67.0.0-RC6.3.22 CPU-B ORDERED HOOK/STACK SOURCE CANDIDATE (2026-10-15).** Exact immutable RC6.3.21 source SHA-256 `bdfbe71f911b2e283132410b3c22fb4159ab044baabb00a4298af3d76d93cd54`; latest Windows fresh-qualified RC6.3.21 release archive SHA-256 `fbfc6c19131a3a10a28de60449a9716111179fe03cc4285f2f970a0e8646ea3a` as recorded by the qualifier (Windows `dist` bytes not independently supplied). Genuine RC6.3.21 ROM/Ghidra native reconstruction PASS; Windows 18/18 CTests, focused 10 commands and Full Regression 1,153 commands PASS. RC6.3.22 instruments *original-ROM* CPU-B instruction hooks to record chronological selected PC visits with ordinal/cycles/SP/SR and marks IRQ vector/RTE sites separately; checks per-checkpoint actual BSR/RTS timeline with sampled stack-pointer pairing. **The selection is filtered and does not establish exhaustive IRQ/scheduling/bus parity, complete CFG, 0x0518 execution or promotable function.** Windows native acceptance and `qualify --fresh` PENDING. Evidence `--out C:\Projects\Evidence\CHQ6322`. See `docs/RC6.3.22_CPU_B_ORDERED_HOOK_STACK_EVIDENCE.md`. Older release banners below are historical.

> **AUTHORITATIVE CURRENT — ChaseHQ-Native 0.67.0.0-RC6.3.21 CPU-B NESTED CALL/RETURN SOURCE CANDIDATE (2026-10-15).** Exact immutable 0.67.0.0-RC6.3.20 source SHA-256 `f52d6abd6b2062050313a8e4bd4370311b3d39ebf956aae3510ee81da35b1a33`; latest fresh Windows-qualified 0.67.0.0-RC6.3.20 release archive SHA-256 `37ff80b5063a687765468c8dc2bde6d3b5f9c32e004408631e233b9eb6bdaab1` (reported by the Windows qualifier, actual `dist` ZIP not provided). RC6.3.20 native ROM/Ghidra reconstruct PASS, 18/18 Windows CTests, focused Run 001 ten commands and Full Regression Run 002 1,153 commands PASS. RC6.3.21 extends original-ROM CPU-B call-return research to three nested BSR instructions (`0x4F70/0x4F74/0x4F78`), nested RTS (`0x5062`) and outer RTS (`0x4F7C`), with fresh Ghidra provenance and five original-ROM scenario graphs, seven counted edges per case. **Aggregated edge counts do not prove temporal stack pairing, complete function closure or native replacement**. CPU-B `0x0518` runtime branch remains unseen; no native promotion. RC6.3.21 Windows native acceptance and `qualify --fresh` PENDING. Evidence `--out C:\Projects\Evidence\CHQ6321`. See `docs/RC6.3.21_CPU_B_NESTED_CALL_RETURN_AUDIT.md`. Older document banners below are retained as history.

> **AUTHORITATIVE CURRENT — ChaseHQ-Native 0.67.0.0-RC6.3.20 CORRECTIVE SOURCE CANDIDATE (2026-10-15).** Latest fully Windows-qualified baseline 0.67.0.0-RC6.3.18 SHA-256 `7978d2e4bdac825e81ef9540694b0ccb54edcd5d04c342b2472198dc25500099`. Exact immutable failed 0.67.0.0-RC6.3.19 source archive SHA-256 `0900d52cb41bea82203695517c708e4a399dab971db878ae85f465c0e099dde0`. RC6.3.19 Windows/MSVC build PASS, but `M2-CPU-B-original-ROM-call-return-audit` **FAIL** due solely to string-format comparison of Ghidra `0x000516` vs runtime-canonical `0x00000516` for post-return CPU-B `BTST` at `0x050E`. RC6.3.20 changes the comparison to strict numeric 32-bit equality and adds independent replay of the actual CHQ6319 Windows Ghidra JSON and all five original-ROM traces (PASS after correction), plus negative guards. Full enclosing function CFG, native substitution composition, unobserved `0x0518` path, timing parity and autonomous promotion remain **UNPROVEN/BLOCKED**. Windows RC6.3.20 native acceptance/`qualify --fresh` PENDING. One ZIP: `--out C:\Projects\Evidence\CHQ6320`. See `docs/RC6.3.20_REAL_GHIDRA_ADDRESS_CORRECTIVE.md`. Older banners are historical.

> **AUTHORITATIVE CURRENT — ChaseHQ-Native 0.67.0.0-RC6.3.19 SOURCE CANDIDATE (2026-10-15).** Immutable RC6.3.18 original authoritative source SHA-256 `7968f432939be3a16de942d46afb8caeeffddba39df3a32c2689c05469530369`; fresh Windows qualified archive SHA-256 `7978d2e4bdac825e81ef9540694b0ccb54edcd5d04c342b2472198dc25500099` (recorded by user Windows qualify), CHQ6318 ROM/Ghidra native reconstruction PASS; 18/18 Windows CTest, focused Run001 10 commands, Full Regression Run002 1,153 commands PASS. RC6.3.19 adds **authentic CPU-B observed JSR→callee→RTS→postreturn provenance** across five original-ROM checkpoints; Ghidra exports new CPU-B 0x050C/0x050E/0x08AC statics and the unobserved 0x0518 branch path (static only). Local replay uses authentic CPU-B TSV and a clearly synthetic static Ghidra double. 0x0518 is NOT runtime proven; no complete function CFG, nested-call closure, timing/composition verification, minimiser or autonomous promotion. RC6.3.19 Windows ROM/Ghidra acceptance is PENDING. One evidence ZIP via `--out C:\Projects\Evidence\CHQ6319`. See `docs/RC6.3.19_CPU_B_ORIGINAL_CALL_RETURN_AND_UNOBSERVED_BNE.md`; older banners are historical.

> **AUTHORITATIVE CURRENT — ChaseHQ-Native 0.67.0.0-RC6.3.18 SOURCE CANDIDATE (2026-10-15).** Exact RC6.3.17 fresh Windows qualification PASS: 18/18 CTest, focused Run001 10 commands, Full Regression Run002 1,153 commands, deterministic Windows release ZIP SHA-256 `6fc40389db4bccd47aef40b7a2c874c43517e2db45b2ac29ced4e72c3f8c1d79`. Genuine CHQ6317 bounded CPU-A/B native/rollback evidence PASS; CPU-B observed research frontier 5,234 edges PASS. RC6.3.18 adds **five extra real-ROM Ghidra continuation instruction sites** (0x04E0/0x04E6/0x04EC/0x0516/0x052C), statically checks lengths/flows and crosswalks with five genuine original-ROM checkpoint cases; entire function CFG, BNE fallthrough, generated whole-function C++, minimiser, composition and native promotion remain BLOCKED. Local static Ghidra cannot run here; RC6.3.18 genuine Windows execution is PENDING. One evidence ZIP: `--out C:\Projects\Evidence\CHQ6318`. See `docs/RC6.3.18_CPU_B_GHIDRA_CONTINUATION_CROSSWALK.md`. Prior banners are historical.

> **AUTHORITATIVE CURRENT — ChaseHQ-Native 0.67.0.0-RC6.3.17 SOURCE CANDIDATE (2026-10-15).** Exact RC6.3.16 Windows-qualified baseline SHA-256 `79c94b0090009b5f5dd29dc2feb543d5d758c072919bb428a761342f732a646d` passed 18/18 Windows CTest, focused 10-command Run001 and Full Regression 1153-command Run002; RC6.3.16 genuine CPU-A/B bounded native/rollback evidence PASS. RC6.3.17 adds a read-only two-hop CPU-B *observed* original-ROM function frontier and heuristic, risk-labelled candidate research ranking (NOT measured cycle savings). Complete enclosing function, automatic minimiser, independent composition gate, native whole-function promotion and RC6.3.17 Windows acceptance remain PENDING/BLOCKED. Next proof: `--out C:\Projects\Evidence\CHQ6317`. See `docs/RC6.3.17_OBSERVED_FUNCTION_FRONTIER.md`. Prior release status banners below are historical.

> **AUTHORITATIVE CURRENT - v0.67.0.0-RC6.3.16 (2026-10-15).** RC6.3.15 genuine Windows Debug and original/native/rollback CPU-A/B bounded reconstruction PASS, verified evidence 959/959. RC6.3.16 SOURCE CANDIDATE adds five-checkpoint authentic CPU-B pair predecessor/successor/BEQ taken+fallthrough and IRQ diversion edge audit, final cross-stage candidate-portfolio reconciliation, and repairs the stale Full Regression 850-limit warning (runtime qualification still pending). CPU-A/B complete-function promotion, automatic composition and minimiser remain BLOCKED. Next Windows proof: `--out C:\Projects\Evidence\CHQ6316`. See `docs/RC6.3.16_BOUNDED_EDGE_AND_PORTFOLIO_RECONCILIATION.md`. Older banners below are historical.

> **AUTHORITATIVE CURRENT - v0.67.0.0-RC6.3.15 (2026-10-14).** RC6.3.15 SOURCE CANDIDATE (2026-10-14): actual RC6.3.14 Windows DEBUG and bounded genuine CPU-B BEQ original/native/fault PASS (17,892 native commits); TST.W+BEQ.B pair original/native/fault PASS (8,945 commits). RC6.3.15 adds Ghidra-authenticated CPU-A/B function-boundary dossiers (NOT a complete-function proof), CPU-B TST mapped R16 versus BEQ zero-data/IRQ ACK-owned attribution, first-divergence capture on pair mismatches, safe atomic single-ZIP evidence under C:\Projects\Evidence\CHQ6315, revised Full Regression structural budget 1176..1536 without removing any commands, and corrected Workbench Next Steps and Brainstorm P0/P1 roadmap. Native execution code unchanged. New Windows qualification PENDING. Whole-function generation/verification and autonomous promotion remain BLOCKED. See `docs/RC6.3.15_FUNCTION_DOSSIER_AND_EVIDENCE.md`. Historic banners below are not current.

> **AUTHORITATIVE CURRENT — v0.67.0.0-RC6.3.14 CPU-B DUAL-CANDIDATE DISPATCH CORRECTIVE (2026-10-14).** Actual RC6.3.13 Windows build PASS and Ghidra pair ABI PASS, but BEQ live acceptance BLOCKED in the Go verifier: 22 valid inactive pair original-ROM events were rejected despite 17,892 single-BEQ native commits. RC6.3.14 corrects only the verified audit/proof contract, adds inverse strict pair-mode audits and mutation regressions; **Windows RC6.3.14 genuine BEQ+pair live three-mode parity PENDING**. No whole-function or autonomous promotion; inherited Go Full Regression structural count 1,176 >850 remains FAIL. See `docs/RC6.3.14_CPU_B_DISPATCH_AUDIT_CORRECTIVE.md`. Older CURRENT banners are historical.

> **CURRENT RELEASE — v0.67.0.0-RC6.3.13 CPU-B TWO-INSTRUCTION LIVE SOURCE CANDIDATE (2026-10-14).** RC6.3.12 original-ROM Ghidra generated pair ABI on Windows PASS; RC6.3.13 opt-in live pair ROM/native/forced-rollback gate pending authentic Windows qualification. Full Go structural budget 1176 > 850 remains FAIL, no function/autonomous promotion. See `docs/RC6.3.13_LIVE_CPU_B_PAIR.md`. Older sections historical.

> **AUTHORITATIVE CURRENT — v0.67.0.0-RC6.3.12 CPU-B TWO-INSTRUCTION GENERATED ABI SOURCE CANDIDATE (2026-10-14).** RC6.3.11 authentic Windows single CPU-B BEQ original/native/fault PASS (17,892 commits, exact CPU/IRQ/machine parity; 869/869 evidence hashes). RC6.3.12 adds separate original-Ghidra TST.W 0x4D8 + BEQ 0x4DE pair C++ ABI from authenticated ROM extension words and 17,903 consecutive observed edges; **pair is offline only, NOT yet a live native replacement**. All CPU-A/B enclosing function boundaries, autonomous M1–M7 and full Go budget 1176>850 remain open. See `docs/RC6.3.12_CPU_B_TWO_INSTRUCTION_ABI.md`. Earlier status paragraphs remain historical.

> **AUTHORITATIVE CURRENT — v0.67.0.0-RC6.3.11 CPU-B IRQ-OWNERSHIP CORRECTIVE SOURCE CANDIDATE (2026-10-14).** RC6.3.10 actual Windows `native reconstruct --cpu-b-live` failed only at the CPU-B original-ROM baseline because 12 interrupt-vector/stack bus effects (four genuine IRQ entry groups) were wrongly tagged as BEQ.B data effects by stale `trace_pc`; its CPU-A health/copy, shared dispatcher and real CPU-B Ghidra/compiled ABI gates all PASS. RC6.3.11 uses Musashi interrupt ACK to assign each IRQ vector/stack transaction to a dedicated ordered IRQ stream, preserves strict zero-branch-data and original/native/fallback CPU/cycle/bus/machine parity, and adds real-ACK synthetic and missing/mutated IRQ evidence tests. **RC6.3.11 authentic Windows native CPU-B equivalence is PENDING**, as are complete-function/autonomous promotion and inherited Go Full Regression budget 1176>850. See `docs/RC6.3.11_IRQ_OWNERSHIP_CORRECTIVE.md`. Earlier status blocks remain historical.

> **AUTHORITATIVE CURRENT — v0.67.0.0-RC6.3.10 CPU-B GENUINE GENERATED LIVE BEQ SOURCE CANDIDATE (2026-10-14).** RC6.3.9 actual Windows CPU-B World-ROM CRC/SHA, five Ghidra opcode sites, and independently compiled generated BEQ C++ ABI all PASS (20261014-193243 evidence; 788/788 SHA entries valid). RC6.3.10 adds opt-in **CPU-B live BEQ 0x0004DE** shared-dispatch substitution and genuine original/generated-native/forced-rollback comparisons, with 68000 timeslice/IRQ guard and complete CPU state, cycle, zero mapped data-bus and full machine endstate parity. **RC6.3.10 Windows real-ROM live parity is PENDING**; synthetic Musashi and local tests do not establish game-ROM equivalence. No complete function boundaries, CPU-B auto-promotion or unrestricted M1–M7 signoff. See `docs/RC6.3.10_LIVE_CPU_B_BEQ.md`. Historical sections below remain snapshots.

> **AUTHORITATIVE CURRENT — v0.67.0.0-RC6.3.9 CPU-B ROM/GHIDRA GENERATED ABI (2026-10-14).** RC6.3.8 Windows original/native/fallback shared CPU-A dispatch PASS with authentic 5,234 CPU-B transition rows. RC6.3.9 adds guarded original CPU-B World ROM CRC/SHA verification, five-PC Ghidra static provenance, and compiled BEQ branch C++ shared ABI candidate; **not yet run against real CPU-B Ghidra on Windows**, no CPU-B live native substitution, M6–M7 autonomous promotion BLOCKED. See `docs/RC6.3.9_GENUINE_CPU_B_GHIDRA_GENERATOR.md`. Historical sections retained.

> **AUTHORITATIVE CURRENT - v0.67.0.0-RC6.3.6 SECOND GHIDRA-GENERATED COPY BLOCK (2026-10-14).** RC6.3.5 Windows authentic caller-to-health-tail and CPU-B original census PASS. RC6.3.6 compiles a second Ghidra-derived CPU-A 16-instruction copy library and adds opt-in ROM-backed original/native/forced-rollback parity; 15/15 local CPU tests, Windows native proof PENDING. No full-function or CPU-B native promotion. See `docs/RC6.3.6_SECOND_LIVE_BLOCK.md`. Historical sections below remain as recorded.

> **AUTHORITATIVE CURRENT — v0.67.0.0-RC6.3.4 OBSERVED HEALTH FUNCTION BOUNDARY / CPU-B CANDIDATE PORTFOLIO SOURCE CANDIDATE (2026-10-13).** RC6.3.3 real Windows 5-case controlled original/native/forced-rollback health matrix PASS with 3 distinct authentic original-ROM paths, 9/9 candidate sites, five native commits, five verified ROM fallbacks; 508/508 evidence hashes independently verified. RC6.3.4 adds real 68000 stacked return/entry captures, independent ROM RTS and observed caller audit, and CPU-B original-ROM PC census into a persistent candidate portfolio; **actual Windows RC6.3.4 execution and release qualification PENDING**. Observed boundary does not independently certify all callers or a complete function CFG. CPU-B observed PCs are not functions. No generalized autonomous promotion. RC6.2.6 steering instruction baseline remains signed off. See `docs/RC6.3.4_BOUNDARY_PORTFOLIO.md`. All older CURRENT headings below are historical.

> **AUTHORITATIVE CURRENT — v0.67.0.0-RC6.3.3 CONTROLLED HEALTH BRANCH-MATRIX DEVELOPMENT CANDIDATE (2026-10-13).** RC6.3.2 real Windows bounded CPU-A health-function live differential PASS: one generated multi-instruction commit and authentic forced rollback with exact CPU/bus/mapped state; manifest 198/198 verified. RC6.3.3 adds original-ROM instruction-hook path capture and controlled entry-state 5-case x original/native/rollback differential; real Windows matrix and release qualification **PENDING**. Controlled branch coverage cannot be presented as natural gameplay or independently reviewed full-function coverage. No automatic CPU-B promotion or autonomous multi-routine orchestration. RC6.2.6 steering instruction baseline remains signed off. See `docs/RC6.3.3_BRANCH_MATRIX.md`. All older CURRENT headings below are historical.

> **AUTHORITATIVE CURRENT — v0.67.0.0-RC6.3.2 TRACE-BOUNDARY CORRECTIVE SOURCE CANDIDATE (2026-10-13).** Actual Windows RC6.3.1 evidence showed one generated CPU-A target-health function commit with exact CPU-state and mapped end-state parity but an extra caller W32 0x102DE0 attributed to the function. RC6.3.2 fixes post-native-commit bus PC attribution and adds a synthetic Musashi original/native/forced-rollback regression which fails against RC6.3.1 and passes with the correction; full game-ROM Windows differential and release qualification are **PENDING**. No RC6.3 general-function promotion or CPU-B autonomy. RC6.2.6 bounded steering M1-M7 signoff retained. See `docs/RC6.3.2_TRACE_BOUNDARY.md`. All older CURRENT statements below are historical.

> **AUTHORITATIVE CURRENT — RC6.3.1 PHASE 2 SOURCE CANDIDATE (2026-10-13).** Generated Ghidra target-health function C++ shared library, experimental opt-in live CPU-A entry substitution and original/native/forced-rollback replay with fail-closed CPU-state/bus/endstate gates. **Linux source CTests and offline ABI fixtures are not Windows live proof**; function boundary provisional, generalized CPU-A/B autonomous M1–M7 unproven. RC6.2.6 bounded steering instruction signoff remains the foundation. See `docs/RC6.3.1_LIVE_FUNCTION.md`. All previous CURRENT headings below are historical snapshots.

> **AUTHORITATIVE CURRENT — 0.67.0.0-RC6.3.1 DEVELOPMENT SOURCE CANDIDATE (2026-10-13).** RC6.2.6 bounded CPU-A steering instruction M1–M7 technical acceptance SIGNED_OFF from real Windows evidence: 240 native replacements, 240 identical CPU return states, ordered bus/mapped-state parity, genuine forced rollback. RC6.3 adds three real Ghidra-derived multi-instruction generated C++ candidates including target-health branching; offline fixtures PASS but **RC6.3 whole-function autonomous M1–M7 and Windows qualification are NOT SIGNED OFF**. See `docs/RC6.3_AUTONOMOUS_RECONSTRUCTION.md` and `RC6_STATUS.md`. Older CURRENT headings below are historical snapshots only.

**CURRENT RELEASE CANDIDATE: 0.67.0.0-RC6.2.6.** Real opt-in CPU-A `0x820C` guarded generated-library native instruction replacement and original/native/rollback full-state acceptance; Windows and ROM-backed proof pending. Enclosing Ghidra function still unreviewed. See `RC6_STATUS.md` and `docs/RC6.2.6_M1_M7_ACCEPTANCE.md`. All prior “current” paragraphs below are **historical only**.

> **AUTHORITATIVE CURRENT STATUS — 0.67.0.0-RC6.2.2 SOURCE CANDIDATE (2026-10-12).** RC6.2.1 Windows qualification PASS (SHA256 `c8422141d9d079ce522f0b1dc12a95527c5d80e9055b4c874fb356493b46174b`); actual CPU-A Ghidra steering export `0x820C` PASS. RC6.2.2 adds self-contained, fail-closed M1–M7 **acceptance evaluation**, not automatically proven unrestricted native replacement. Full acceptance and Windows qualification must be tested; see `docs/RC6.2.2_M1_M7_ACCEPTANCE.md` and `RC6_STATUS.md`. Prior CURRENT/unqualified statements below are preserved as **historical snapshots, not current release status**.

> **CURRENT RC6.1 DEVELOPMENT — v0.67.0.0-RC6.2.1.1; native steering shadow opt-in, Windows qualification outstanding.** See `RC6_STATUS.md`; older status below is historical.

> **RC6 DEVELOPMENT STATUS (not qualified):** v0.67.0.0-RC6. RC5.3 sections below are historical. See `RC6_STATUS.md` at release root.

> **RC6 DEVELOPMENT SOURCE CANDIDATE — NOT WINDOWS-QUALIFIED.** See [RC6_STATUS.md](RC6_STATUS.md). Historical RC5.3 status sections below are retained for provenance and must not be read as the current RC6 status.

> **CURRENT: v0.67.0.0-RC5.3 native sprite-clear guard corrective DEVELOPMENT CANDIDATE (UNQUALIFIED), 2026-10-11.** RC5 remains Windows-qualified, immutable SHA-256 `47def65d75056fb765d5b60dfe5a4c4a21bdcd3214ebb84920dd62a0d8b37ddf`. RC5.1 original shadow corpus passed (96/96 sprites, 24/24 class-6 road). RC5.2 Windows substitution failed: all 85 native attempts were guard-rejected (0 actual skips); final CSV export crashed. RC5.3 fixes the instruction-proven ROM branch fingerprint, uses entry-latched records for the destination guard, adds observed values, and repairs PS5.1 CSV. RC5.3 native equivalence and Windows qualification remain PENDING. See `docs/RC5.3_NATIVE_CLEAR_SUBSTITUTION.md`.

> **CURRENT: v0.67.0.0-RC5.3 native sprite-clear SUBSTITUTION DEVELOPMENT CANDIDATE (UNQUALIFIED), 2026-10-11.** The original Windows-qualified RC5 parent SHA-256 `47def65d75056fb765d5b60dfe5a4c4a21bdcd3214ebb84920dd62a0d8b37ddf` remains immutable. RC5.1 headless live-shadow corpus PASS 12/12 cases (96/96 sprite transfers and 24/24 class-6 road generator comparisons); RC5.3 adds an experimental, opt-in native substitution of the trailing sprite-clear DBRA loop ONLY. Bulk copier and final RTS remain original ROM; normal emulation is default. RC5.3 Windows native A/B equivalence and exact-archive qualification pending. See `docs/RC5.3_NATIVE_CLEAR_SUBSTITUTION.md`; all older release-status headings below are historical.

> **RC5 SOURCE CANDIDATE (`0.67.0.0-RC5`), NOT WINDOWS-QUALIFIED.** Original RC4.7 source archive supplied by user; RC5 integrates offline native C++ sprite/road candidates and standalone synthetic Windows preview, updates live Workbench Next Steps. No live SHADOW/COMPARE or native replacement is implemented. Follow `docs/RC5_NATIVE_PREVIEW_AND_SHADOW.md` and exact Windows `chqdev.bat qualify --fresh` gate before promotion. Older version status sections below are historical.

> **RC4.7 current source candidate `0.67.0.0-RC4.7`; qualified promoted baseline remains RC4.6.** See `docs/REA_GHIDRA_RC47.md`. Windows `chqdev.bat qualify --fresh` pending.

> **RC4.7 CURRENT SOURCE CANDIDATE — NOT WINDOWS-QUALIFIED.** Authoritative promoted parent RC4.6 SHA-256 `9e76a107e19f3e28a54a8a8b3bdaed630511e02f1da3e3e7ae274c07d863580f`. RC4.7 scope and acceptance criteria were documented before source implementation in `docs/REA_GHIDRA_RC47.md`. The original RC4.6 ZIP remains immutable. Exact RC4.7 Windows `chqdev.bat qualify --fresh` remains mandatory. Older release candidate paragraphs below are historical context.

> **AUTHORITATIVE PROMOTED BASELINE — v0.67.0.0-RC4.6 (2026-10-10).** Windows `chqdev.bat qualify --fresh` reports **11/11 stages PASS** for exact frozen ZIP SHA-256 `9e76a107e19f3e28a54a8a8b3bdaed630511e02f1da3e3e7ae274c07d863580f`. User explicitly authorised promotion following focused, full, and successful headless Ghidra/REA crosswalk evidence. This is a **post-qualification documentation overlay**; it does not change the immutable tested archive. See `docs/RELEASE_VERIFICATION_RC46.md`, `promotion/PROMOTION_RECORD.json`, and `docs/REA_GHIDRA_NEXT_SCOPE.md`. Earlier RC4.6 “candidate/unqualified” statements below are **historical build-time notes, not current release status**.

## Current state after Windows qualification and completed Ghidra analysis

- **Promoted source baseline:** `0.67.0.0-RC4.6`, exact ZIP SHA-256 shown above, immutable. Prior baseline RC4.5 remains retained for rollback.
- **Windows authority:** user-pasted `qualification.json` (`chqdev 0.5.13`) reports 11/11 PASS. Focused Run 001: 13 commands PASS; Full Regression Run 002: 1,153 commands PASS. Evidence integrity checks PASS.
- **Ghidra / REA proof:** Ghidra 12.1.4 headless processed a matching 524,288-byte CPU-A image, Motorola 68000 big-endian, base `0x000000`, and emitted eight static instruction-site windows. Run 035 correlated three genuine checkpoint ROM-census TSVs against those Ghidra exports; 19 commands PASS, three Workbench `ghidra-*` crosswalk reports.
- **Health routine:** Ghidra **unreviewed function candidate** `0x00A10C–0x00A129` (exclusive end `0x00A12A`), contains authentic `0x00A112` decrement; in damage checkpoint, seven distinct PCs executed once. Ownership/boundaries remain `hypothesis`, not verified.
- **Known RC4.6 limits:** `ghidra.results` findings-directory resolution is incorrect; Java/Jython preflight and background-job log capture are incomplete; `ghidra.import` initially produced zero reports when prior census inputs were absent (manual `rea.crosswalk` subsequently succeeded); REA/Ghidra controls are cramped in the Native Lab column. `ghidra.wait`, live Workbench ROM census, direct upstream REA 68000 provider, Capstone, complete native projection arithmetic remain future work. The existing Go structural command budget >850 is separate from passing Windows Full Regression.
- **Next UI priority:** dedicated **top-level Static Analysis / REA tab**, full-width and responsive, with crosswalk/function explorer, Ghidra job controls, evidence, confidence, ROM identity and runtime-PC crosslinks. See `docs/REA_GHIDRA_NEXT_SCOPE.md`.

## RC4.6 Ghidra integration — historical source-candidate description

Promoted immutable parent: RC4.5 SHA-256 `443ffe623a38de4813691c436e05d6f92d9e113a8abae0219824fcc77e9aae11`. RC4.6 adds an optional asynchronous local headless 68000 Ghidra workflow to the Research API, Script Console, and Native Reconstruction Lab. It verifies a user-owned CPU-A ROM fingerprint, runs packaged scripts in a separate process, exports unreviewed static candidates and XREFs, and correlates with previously captured genuine ROM census TSVs. The live SDL emulator is not restarted; no REA agent or native replacement is claimed. See `docs/GHIDRA_WORKBENCH_RC46.md`. Fresh Windows qualification and genuine headless Ghidra execution are pending. Historical RC4.5 candidate lines below remain provenance, not current status.


> **AUTHORITATIVE PROMOTED BASELINE — v0.67.0.0-RC4.4 (2026-10-10).** Exact Windows-qualified archive SHA-256: `8a8b715646ee07619891ea137425bd24272d5df2b84c60170105a164ec3d8912`. User-authorised promotion; all eleven stages in Windows `out/chqdev/qualification.json` are PASS, including `promotable`, focused and full regressions, and run-history. The immutable qualified archive is unchanged. Full provenance: `promotion/PROMOTION_RECORD.json` in the separate promotion supplement; historical candidate notes below describe earlier states and are retained.

> **Current development candidate:** `v0.67.0.0-RC4.4`. Windows exact-archive qualification pending. See `docs/NATIVE_PROJECTION_RC44.md` for present scope and limitations.

## RC4.3.2 — authoritative history CTest signature corrective (unqualified)

The exact RC4.3.1 Windows `chqdev.bat qualify --fresh` log passed configuration and debug compilation, then failed CTest 10/13, `authoritative_history_tests`, with `Authoritative history function absent`. The production `Get-AuthoritativeScriptRuns` function is present, but its declaration changed in RC4.3.1 from `()` to `([string]$onlySession='')`. The PowerShell test's regex still matched only `()` and falsely reported absence. Correct the extractor to allow parameters and whitespace, **without dropping** its 320-run ordering/pagination/version tests. Extend Windows CTest coverage to exercise the new current-session filtering path. Add an independent Go regression guard against repeating this signature drift. Do not change production execution, the existing run-history performance fix, or Full Regression assertions. 0 tolerated CTest failures; this candidate is NOT promoted until the final exact ZIP passes Windows qualification. The historical Full Regression expanded-command budget failure remains unresolved.

**Mandatory release rule:** a production PowerShell function signature change MUST be accompanied by updating and executing every PowerShell CTest that introspects it; passing Linux Go tests alone is not Windows CTest qualification. Preserve the exact fresh-extracted archive qualification gate.

## RC4.3.1 Workbench readiness release blocker — NON-NEGOTIABLE

Script-history enumeration MUST NOT run during desktop/mobile browser startup or be awaited before core status/frame hydration. A release that makes Workbench health or status unresponsive due to evidence scans is unqualified even when CTest and packaging pass. Large bundle manifests MUST NOT be reparsed on each history-list request; preserve authoritative bundles and persist compact summaries at finalization. Current-session listings MUST avoid cross-release directory discovery. Maintain static Go regression guards, inspect Windows readiness on the actual release archive, and never increase timeouts merely to mask serialized history requests. See `docs/WORKBENCH_READINESS_RC431.md`.

## v0.67.0.0-RC4.3.2 CURRENT DEVELOPMENT CANDIDATE — NOT QUALIFIED

This source candidate extends v0.67.0.0-RC4.2.3. Its authoritative new feature and limitation inventory is `docs/NATIVE_MIGRATION_RC43.md`. The prior RC4.2.3 Windows debug compile was PASS, but it does **not** qualify RC4.3. Required before promotion: exact-archive Windows `chqdev.bat qualify --fresh` including strict CTest, focused and full regressions, deterministic package verification and readiness. The known pre-existing Full Regression Go structural budget check remains FAIL (1,170+ commands versus <850); do not delete unique assertions or relax that gate to conceal the failure. New code paths MUST keep native replacement disabled by default. User-controlled ROM, SDL and content paths stay outside release ZIPs. Fresh release setup MUST find the existing stable `build-local.json` or provide a precise doctor error. The bundled Windows chqdev executables MUST be present, tested and accompanied by matching release metadata.

## RC4.2.3 mandatory fresh-extraction dependency contract

A newly extracted RC MUST support `chqdev.bat qualify --fresh` without manually copying ROMs, SDL or build-local.json. chqdev discovers configuration in order: release-local `build-local.json`, parent `build-local.json` (preferred stable shared location), then a valid sibling ChaseHQ-Native release configuration. Environment variables CHASEHQ_ROM_SOURCE and CHASEHQ_SDL_SOURCE override configured sources. User-owned ROMs and SDL MUST NOT be bundled. `doctor` MUST fail, not warn, when neither configured nor staged dependencies exist. The package MUST include the Windows chqdev executable, and archive tests MUST verify it. All candidate handovers MUST disclose incomplete qualification, including the existing regression command-budget failure. Fresh Windows qualification remains required before promotion. NEVER weaken regression coverage or gates to conceal a failure.

> Historical unqualified RC4.2.3-line candidate: **v0.67.0.0-RC4.2.3.1** — CHQPACK Browser, overlay inspection and provenance. Windows qualification pending.

> Current development candidate: **v0.67.0.0-RC4** — ROM/CHQPACK source switching and RC3.1 release consistency corrections.

> Current development candidate: **v0.67.0.0-RC3** — CHQPACK runtime content boundary; ROM-free runtime after deterministic import; RC2.1 native rendering retained.

> Exact source baseline: **v0.67.0.0-RC2.1** SHA-256 `0199faf2333ebbe25e7c23df2c570d0f5f35c9179b8ea53b43983136f3885fb6`. See `docs/DOCUMENTATION_AUDIT_0.67.0.0-RC3.md`.

> Historical qualified candidate: **v0.67.0.0-RC1.1** — native sprites default; text/HUD Shadow zero-diff bring-up; native sprite forensic overlap-coverage corrective.

> **Historical candidate: v0.66.9.5-RC3.** This is the final planned tooling pass before native replacement work. Baseline: exact v0.66.9.5-RC2 SHA-256 `3aaaebf55cac7cfa9150754556db42438083b52c2965fd51ce36cc5f2bdca0f7`. After Windows qualification, do not broaden tooling unless native implementation is blocked. First implementation milestone: prove the sprite subsystem in Native-only mode across the canonical corpus with Original hard-disabled/fallback-free, then promote it to NATIVE and begin TC0100SCN text/HUD Shadow work.

> Historical candidate: **v0.66.9.5-RC2**. See `docs/DOCUMENTATION_AUDIT_0.66.9.5-RC2.md` for Runs003-012 evidence and RC2 changes.

> Current development candidate: **v0.66.9.5-RC1.1**, a narrow Workbench web-bridge hotfix built from the exact Windows-qualified **v0.66.9.5-RC1** archive SHA-256 `26bda3287a3256fe8bf2c46aaaafba68546c02d2e1922465abcbacb0a62d262e`. RC1.1 changes no intended emulation behaviour and requires exact-archive Windows `chqdev qualify --fresh` before promotion.

> Current release: **v0.66.9.4**. Promoted from exact Windows-qualified **v0.66.9.4-RC3.1.3** SHA-256 `48534c0bd3d8e4a40987e65089b07046535976675ca2e36d0ebf3a6d9584383e`. The final archive is regenerated for release identity/docs/focused-regression wiring only and requires one exact-final-archive `chqdev qualify --fresh` closure before immutable publication.

> Current development candidate: **v0.66.9.4-RC3.1.2**. RC3.1.2 keeps the proven cross-format yellow-target renderer but binds it to semantic `player_car.body` across steering/orientation maps, expands SPEED suppression to the complete live destination block, and retains fail-closed proof plus chqdev readiness/console UX. Exact-archive Windows `chqdev qualify --fresh` remains required.

> Current development candidate: **v0.66.9.4-RC2**. Source lineage remains the immutable promoted **v0.66.9.3** baseline (SHA-256 `6da88889dc8214c6058644e44e4d29f6a2117c9f0e92d99c8d906c280fc3462e`); **v0.66.9.4-RC1** (`82d0ee720b40f248624e6c3b2b532ecbef83a68c80a45c68e70556a49b1b1a3a`) passed fresh Windows qualification. RC2 adds the Native Reconstruction Lab foundation, Original/Shadow/Native sprite backend switching, global existing-sprite map substitution, complete live TC0100SCN charset export, Stage-2 checkpoint corpus expansion and the HUD Layout responsive-width fix. RC2 still requires exact-archive Windows `chqdev qualify --fresh` before promotion.

> Current release: **v0.66.9.3**. Promotion provenance is the exact Windows-qualified `v0.66.9.3-RC5` archive SHA-256 `19b7ed48edae98fd99f660ef56a949573050cabc2958fcdba2833a7888103961`. That candidate passed 8/8 CTest, deterministic package/verify, runtime readiness, focused regression, Full Regression and `promotable`; the final promotion changes release identity/documentation/focused-regression wiring and adds a release-package metadata guard only; it does not intentionally change emulation/gameplay behaviour.


# Current release / Windows portability contract (mandatory)

Current release: `0.66.9.3`; direct qualification provenance is exact Windows-qualified `0.66.9.3-RC5` SHA-256 `19b7ed48edae98fd99f660ef56a949573050cabc2958fcdba2833a7888103961`. `chqdev` is the **de facto and authoritative build/release process** and normal Windows runtime lifecycle entry point. The canonical fresh Windows prove-off is `chqdev qualify --fresh`; zero tolerated CTest failures, deterministic exact-archive identity, focused regression and Full Regression are mandatory. Implement new validation, packaging, qualification, trust, runtime lifecycle and regression orchestration in `tools/chqdev`; do not add authoritative behavior only to PowerShell/BAT.

Every future release must preserve deterministic package bytes, native Windows `Zone.Identifier` repair/verification, Windows-script ASCII/U+00B7 checks, LF/CRLF/CR contract normalization, exact script-catalog parity, current-document identity, immutable archive verification and machine-readable qualification state. Manual `Unblock-File` must never become a required release/runtime step again.

# ChaseHQ-Native — Agent Guide

Use this file as the durable operating contract for coding/research agents in this repository.

## RC3.4.9 clean CTest / Windows portability hardening

RC3.4.9 is built from the exact promoted RC3.4.8 archive SHA-256 `ea24ce76ba1555e23b997a0e925ba85a9c489da6a569c04a8e374df661395990`. Windows diagnosis proved the long-standing `cpu_bus_rom_tests` crash had two independent portability causes: sixteen large stack-local `chq::Runtime` instances exhausted the MSVC test process stack, and eight `std::ifstream` lifetimes remained open across `std::filesystem::remove/remove_all`, which Linux permits but Windows rejects.

The permanent fix heap-allocates those sixteen Runtime instances and explicitly closes all eight read streams before cleanup. `chqdev` v0.4.2 removes the historical `cpu_bus_rom_tests` exception completely: **any CTest failure is fatal**. The canonical fresh qualification remains `chqdev qualify --fresh`; Windows promotion now requires genuine **7/7 CTest PASS**, focused regression PASS, Full Regression PASS, and final promotable PASS. Compatibility `Build-Debug.bat`/`Build-Release.bat` are fail-closed as well.

## RC3.4.8 chqdev lifecycle / fresh qualification contract (mandatory)

`tools/chqdev` is the de facto/authoritative build-release controller and the normal Windows runtime lifecycle entry point. Keep its Go implementation standard-library-only unless a dependency is explicitly justified. Windows `preflight` must natively repair project-owned `Zone.Identifier` streams and verify zero remain; manual `Unblock-File` is not a supported normal step. `chqdev runtime start|stop|restart` owns lifecycle. The PowerShell Workbench host may remain an internal compatibility bridge in this milestone, invoked only through chqdev with `ExecutionPolicy Bypass`. Every release must preserve Windows-script ASCII/U+00B7 and LF/CRLF/CR guards, deterministic package bytes, exact catalog parity and current-document identity.
Keep **fast-changing project status out of this file**; read the current handover/state documents for that.

## Mission

The goal is accurate Chase H.Q. emulation and reverse engineering.
The Workbench, API, scripts, diagnostics, knowledge system, evidence capture, and automation exist to support that goal.
Do not let non-blocking tooling/UI polish displace a concrete gameplay or rendering investigation.

Prefer evidence-backed corrections over guesses, visual compensation, or hand-tuned offsets.

## Current sources of truth

Read only what the task needs, but use these as the authoritative map:

- `docs/HANDOVER.md` — current continuation point and next gate.
- `docs/PROJECT_STATE.md` — current confirmed project state.
- `docs/ROADMAP.md` — completed/outstanding/longer-term work.
- `docs/NEXT_BUILD_CHANGELOG.md` — pending coherent build changes when relevant.
- `docs/API_REFERENCE.md` and `research/schema/api-actions.json` — Research API.
- `docs/SCRIPT_LANGUAGE_REFERENCE.md` and `research/schema/script-language.json` — Research Script v2.
- `research/scripts/SCRIPT_CATALOG.md` — packaged script inventory.
- `docs/RELEASE_PROCESS.md` / `VALIDATION.md` — release and proof gates.

Do not infer the authoritative version from the folder name.
Do not assume the newest candidate is already the proven baseline.
If documents disagree, identify and fix the stale source instead of silently choosing one.

## Default engineering loop

Use this order unless the defect is already proven:

1. Reproduce deterministically.
2. Inspect existing evidence, logs, source, schemas, checkpoints, and prior runs.
3. Form a narrow falsifiable hypothesis.
4. Use existing diagnostics before adding tooling.
5. Prefer causal A/B experiments over visual intuition.
6. Patch only when evidence isolates a real defect or coherent tooling gap.
7. Add/update same-change regression coverage.
8. Validate the exact working tree.
9. Update project state/roadmap/handover/docs/knowledge as required.
10. Package last.

Do not cut a build merely because an experiment produced an interesting result.

## Investigation vs implementation

**Investigation mode:** read/search, use existing debugger/API/Workbench/chqctl surfaces, run packaged scripts, capture structured evidence, and use reversible runtime/presentation experiments. Avoid permanent source changes while the failure mechanism is still speculative.

**Implementation mode:** make the smallest coherent source/tooling correction after the evidence supports it. Preserve public-surface parity, add regression coverage, update schemas/docs, and validate before claiming success.

## Build and runtime rules

Do not assume the tree has been compiled.
Check for the expected runtime before runtime-dependent work.
Use repository build/launcher scripts rather than inventing a parallel build path.

Canonical Windows commands:

```powershell
.\chqdev.bat qualify --fresh
```

Direct `Build-Debug.bat` / `Start-ChaseHQ.ps1` use is compatibility/debugging only. Do not require manual PowerShell unblocking; `chqdev preflight` owns Windows trust normalization.

Static inspection or a non-Windows compile is not proof of a Windows/SDL/Workbench behavioural fix.
State exactly what was and was not run.

Do not require paid APIs, separately purchased credits, or external cloud services for the normal project workflow.

## Process/port safety

- Never terminate PID 4/System.
- Do not kill unrelated processes to free a port.
- Prefer repository restart/cleanup logic.
- When a stale ChaseHQ listener exists, identify the actual ChaseHQ process by command-line/provenance before terminating it.
- Prefer port-scoped cleanup over broad process-name cleanup.

## API and `.chqscript` rules

Never invent an API action, parameter, or script-language construct.
For an unfamiliar operation, inspect/query the authoritative surface first:

```text
api actions
api schema action=NAME
api schema.all
api script.schema
api script.functions
```

For Workbench Script Console workflows, use the current **Research Script v2** contract and prefer existing `api ACTION key=value` calls over new grammar.

The repository also contains older/other `.chqscript` usage under experiment/debugger tooling. Determine the intended runner before editing or executing a file; do not assume identical semantics from the extension alone.

Pasted/ad-hoc Research Script v2 should normally include:

```text
# name: Meaningful investigation name
# purpose: What this proves or measures.
# category: appropriate-category
# target: Frame/checkpoint/subsystem under test.
# method: What is changed or measured.
# expected: Evidence that distinguishes the hypotheses.
# cleanup: State that must be restored even on failure.
```

### Safe experiment scripting

Treat emulator/debugger state as persistent across failed scripts. Never assume the previous script completed or cleaned up successfully.

For scripts that mutate runtime/research state:

- establish a known starting state explicitly;
- document target, method, expected evidence, and cleanup in the header;
- historical timeline state is read-only: fork to live state before memory writes;
- use idempotent cleanup/reset actions where available;
- use `try ... finally ... end` for timeline, sprite, patch, presentation, and other temporary mutations;
- cleanup must run on assertion/API/runtime failure and user abort where practical;
- cleanup failure must not hide the original experiment failure;
- restore temporary visual overrides, sprite overrides, timeline state, patches, and presentation changes unless the script explicitly documents otherwise.

Do not "fix" a failed experiment merely by rerunning it. Inspect the failure state first and determine whether the previous run left persistent state behind.

If a snapshot/run/evidence name is arbitrary, say so. Do not present a label invented for one experiment as an established feature.

## Evidence rules

Structured evidence is authoritative; screenshots are supporting evidence.

Prefer run-owned/session-owned outputs with machine-readable metadata, logs/CSV/JSON, checkpoints/timelines, layered snapshots, reconstruction metadata, and automatic ZIP bundles.

A requested run should be reproducible: preserve the checkpoint/scenario, frame range, inputs/patches, trace settings, output location, exit condition, and evidence bundle where supported.

Do not infer gameplay state from pixels when the underlying emulated/game state can be measured directly.

When reporting findings, distinguish observed fact, inference, code explanation, and untested assumption.
Never report PASS for a property the test did not actually assert.
Validate evidence-bundle identity before drawing conclusions: recorded script path/name, embedded script body, run/session metadata, and artifacts should describe the same run. If they disagree, report the metadata defect rather than attributing the failure to the stale name.

## Canonical checkpoints

Preserve these stable research references unless explicitly superseded:

- `checkpoints/stage1-gameplay-2064.chqstate` — canonical player-gameplay state.
- `checkpoints/stage1-driving-2352.chqstate` — canonical attract/graphics reference.
- `checkpoints/stage1-target-immediate-pre-contact-5587.chqstate` — one frame before the first proven target physical-contact response.
- `checkpoints/stage1-target-first-physical-contact-5588.chqstate` — matching physical-contact state.
- `checkpoints/stage1-target-immediate-pre-damage-6333.chqstate` — **preferred target-health/damage research anchor**, one frame before a causally proven damaging hit.
- `checkpoints/stage1-target-first-damage-6334.chqstate` — matching genuine damaging-hit state.
- `checkpoints/stage1-target-post-final-hit-9548.chqstate` — post-final-hit/defeat-transition reference (not a near-destruction health state).
- `checkpoints/stage1-end-level-9988.chqstate` — confirmed end-level/intermission reference.

Preserve companion metadata. Do not silently overwrite canonical checkpoint files.

## Graphics/reverse-engineering rules

- Separate source-layer defects from compositor/priority defects.
- Inspect raw source layers before compensating with presentation offsets.
- Prefer deterministic frame comparisons.
- Preserve alpha/reconstruction metadata in layered snapshots.
- Use per-sprite provenance when aggregate sprite evidence is insufficient.
- Preserve HUD normal/hidden/only comparisons where relevant.
- Do not bake forensic layer offsets into defaults without hardware/reference evidence.

For sprite order, palette behaviour, TC0100SCN, TC0150ROD, road priority, or mixer behaviour, prefer a causal A/B experiment.

## Regression policy

Regressions are living tests, not demonstrations.

Any meaningful change to API/script behaviour, Workbench discovery/state, evidence ownership/bundling, renderer semantics, checkpoint determinism, or launcher/process behaviour needs focused regression coverage in the same change.

A regression must assert the actual property under test.
Logging output followed by `PASS` is not sufficient.
Do not accept a failure as "known" or "pre-existing" solely because a build/test script labels it that way. When practical, reproduce it against the last pristine proven baseline before classifying it as pre-existing.

When relevant:

1. run the focused build-specific regression first;
2. run `research/scripts/regression/full-regression.chqscript`;
3. preserve correct Full Regression ordering and its terminal COMPLETE marker;
4. treat unexpected `ERROR:` output or missing declared artifacts as failure.

## Documentation and public-surface parity

When public behaviour changes, update the authoritative sources in the same change as applicable:

- API/script schemas and references;
- API/script changelogs;
- script catalog and feature manifest;
- Workbench/User Handbook knowledge sources;
- `docs/PROJECT_STATE.md`;
- `docs/ROADMAP.md`;
- handover/continuation docs;
- release and validation notes.

Do not leave current-build catalogs/generated docs carrying stale version labels for a surface you changed.

Prefer CLI/config/API parity for diagnostics. UI-only research capabilities are incomplete when a practical scriptable equivalent should exist.
When adding or changing a Research API action, verify every exposure layer: schema, dispatcher/implementation, Script Console allow-list, documentation, and regression coverage. Prefer an automated schema/allow-list consistency check so a documented action cannot remain unreachable.

## Release discipline

Push the current build as far as practical before creating another release.
A new build should represent a genuine emulator correction, meaningful rendering/gameplay improvement, coherent tooling/reliability change, or deliberate documentation/knowledge consolidation checkpoint.

Packaging is the final step, not the discovery step.
Validate the exact tree first, then package, then validate packaged contents.
Before packaging, reconcile any manual/local hotfixes used during validation back into the authoritative working tree, regression coverage, schemas, and documentation. A runtime PASS obtained with an unrecorded manual edit is not yet a releasable tree.
Never silently replace an already-issued versioned ZIP while keeping the same version/hash.

## Preserve cumulative capabilities

Do not regress established research infrastructure while fixing the current issue. Preserve, where relevant:

- native debugger / `chqctl` control;
- Workbench Research API and Script Console;
- CLI/config diagnostics;
- scenarios and canonical checkpoints;
- structured evidence and bundling;
- Forensic Timeline/history analysis;
- searchable docs/knowledge;
- Graphics Lab layered/per-sprite inspection;
- regression coverage.

Before building a new diagnostic, search for an existing API action, script, helper, viewer, or tracer that already answers the question.

## User interaction

When the next step is a run, give the exact copy/paste **one-line PowerShell command immediately**.
Batch independent experiments when safe; avoid many tiny rebuild/run cycles.
If evidence is already uploaded, inspect it before requesting another run.
Do not claim something was built, run, tested, validated, uploaded, or proven unless it actually was.

## Destructive/external actions

Normal repository work may read/edit project files, run project-local build/tests/diagnostics, and use required localhost services.

Ask before deleting user data/evidence outside disposable outputs, overwriting ROMs/canonical checkpoints, force-killing a process not clearly owned by ChaseHQ, installing software or changing machine-wide settings, pushing/publishing externally, or using credentials/secrets/paid services.

ROMs are user-supplied runtime inputs. Do not redistribute them in release packages.

## Definition of done

Match proof to the claim:

- source-only claim → static/code evidence may suffice;
- native behaviour → native runtime/test proof;
- Windows launcher/Workbench behaviour → Windows launcher/Workbench proof;
- rendering → deterministic visual/structured evidence plus relevant state/provenance;
- API/script change → schema parity plus focused regression;
- release → release validation and package integrity.

If the required proof cannot be run in the current environment, state the remaining gate explicitly.

---

This guide was introduced on the v0.66.9.0-RC2.4.3 project line. Keep it short and durable; put changing investigation status in the handover/state/roadmap documents.

## Split-environment source delivery

`chqdev build debug|release` is the authoritative Windows staging/build path and consumes the local ROM/SDL configuration. `Build-Debug.bat` / `Build-Release.bat` / `Prepare-Project.ps1` remain compatibility/fallback helpers. Their absence in the Linux source workspace does not block authorised source edits, available native tests, documentation or a clearly labelled candidate package. Preserve the staging scripts and local configuration. Never claim Windows/SDL or complete Script Console proof from Linux tests; keep the local promotion gate explicit. No user needs to repeat authorisation for routine source work already requested.

Release identity is authoritative in `src/version.h`; CMake, launchers and Workbench consume it. Keep feature/schema/knowledge catalogs in sync. Old/corrected renderer diagnostics are not independent shadow verification.

## Course mapping / survey discipline (RC3.0+)

Use the live `course.follow.*` / `course.survey.*` API surfaces and bundled mapping scripts before inventing launch-only survey flows. Long course surveys should retain road geometry and the player's driven line as separate data. Preserve generic visible sprites as `UNCLASSIFIED_DYNAMIC_OBJECT` until evidence proves traffic/obstacle semantics. Preserve raw course-record + TC0150ROD surface signatures as unclassified evidence until screenshots/behaviour justify material names. Structural fork candidates remain A/B until physical left/right semantics are proven. Prefer the 600-frame shakedown before any 3600+ frame evidence run.


## RC3.1 survey-data rule

Before long course surveys, keep stable surface segmentation separate from volatile renderer evidence. `surfaceStableSignature`/surface segments are conservative candidates only; preserve full TC0150 evidence separately. Preserve raw sprite rows and any heuristic `UNCLASSIFIED_OBJECT_CANDIDATE` track IDs together. Do not merge touching sprite rectangles into one semantic object merely from screen proximity; repeated perspective scenery can form dense chains. Candidate IDs are not semantic AI-car/obstacle proof and are screen-space until authoritative object/world coordinates are mapped. The final RC3.0 `$config` bridge fix, Track View route fix and Ctrl+P pause fallback are release invariants and must be carried forward.

### PowerShell variable safety

PowerShell variable names are case-insensitive. Do not use automatic/read-only/reserved names such as `$PID`, `$args`, `$Matches`, `$PROFILE`, `$Host`, `$Error`, `$input`, `$MyInvocation`, `$PSScriptRoot`, `$PSCommandPath`, `$PSVersionTable`, `$PSBoundParameters`, `$PSItem`/`$_`, `$foreach`, `$switch`, `$this`, or equivalent case variants as ordinary local variables or function parameters. Use descriptive names instead. The release validator AST-scans packaged `.ps1` files for these collisions; do not bypass that gate. Intentional preference assignments such as `$ErrorActionPreference='Stop'` are allowed, but preference names should not be repurposed as parameters.

### RC3.1 cumulative course-mapping prove-off fixes

Carry forward the locally proven RC3.1 fixes together: `surfaceSegmentsCsv` vs numeric `surfaceSegments`; production OrderedDictionary-aware `Prop()`; non-vacuous object-candidate coverage; same-frame Track Recorder suppression; no Track View auto-sampling during Script Console runs; and distinct `$nativeLaunchArgs` in `Start-ChaseHQResearch.ps1` to avoid duplicate launch arguments. The local 600-frame shakedown proved exactly 41 unique 15-frame samples, all ON_ROAD, with full candidate coverage.


### RC3.1 long-survey signed-16 correction

The first 3600-frame centre survey reached frame 3399 with 89 unique samples, all ON_ROAD, before a Workbench-only signed conversion failed on raw `0xFEF7` (65271), which semantically represents -265. Track-state road/lateral 16-bit deltas must use the explicit `Signed16` two's-complement helper rather than direct `[int16]` range casts. The corrected candidate includes a PowerShell runtime regression and static guard for this exact class of failure.

## Historical RC3.4.4 release/document contract (rules carried forward)

At RC3.4.4 the then-current candidate was `0.66.9.0-RC3.4.4`. The release/document safeguards introduced there remain mandatory for the current `0.66.9.3` line and future candidates:

- Preserve the Windows-script ASCII/U+00B7 fail-closed gate.
- Normalize CRLF and CR to LF before line-oriented release/document regex matching.
- Add positive LF, CRLF and CR self-test fixtures plus independent portable coverage for any new text contract.
- Keep the current-documentation matrix synchronized with `src/version.h`; do not hand off a ZIP if any authoritative current document still identifies an older candidate.
- Keep `## Authoritative continuation` and `## Immediate next task` in the ChatGPT handover.
- Historical note: RC3.4.4 used version-specific qualification wrappers; that policy is superseded. Do not reintroduce them. The only canonical Windows qualification command is `.\chqdev.bat qualify --fresh`.
- Never promote a mutable tree; build/test/promote one exact immutable archive SHA.


RC3.4.8 trust rule: Windows preflight includes a real `Zone.Identifier` create/detect/remove self-test before normalizing the project tree.


## RC3.4.8 canonical fresh qualification

RC3.4.8 makes `chqdev qualify --fresh` the canonical fresh-build/release prove-off command. It is fail-closed and records each stage in the `chqdev-qualification-v2` state: generated-state clean, doctor/preflight, Debug build, structural CTest classification, deterministic package + exact verification, Windows runtime restart/readiness, focused release regression, Full Regression, and final promotable status. On non-Windows hosts the same switch proves the portable clean build/test/package stages and records Windows runtime/regression gates as pending.

Full Regression is no longer posted wholesale to the reduced server-side script parser. `chqdev` v0.4 contains a Go-side Script Console runner that expands packaged `include` files locally and executes `set`, variable substitution, `for`, `repeat`, `if/else/end`, `try/finally`, query helpers, predicates, waits and ordinary API/assert commands through the authoritative Workbench run API while preserving one run ID and finalization status. The RC3.4.6 `Unexpected end at line 59` failure is therefore regression-locked as a controller defect that must not recur.

Release-root hygiene rule: ship one `CHANGELOG.md`, one current `VALIDATION.md`, and generic `PACKAGE_MANIFEST.json` / `PACKAGE_MANIFEST.txt`; do not ship version-specific `Qualify-*.bat` wrappers or historical root release-note/validation/manifest files. `chqdev preflight` enforces this.

Future release documentation and automation MUST treat `chqdev qualify --fresh` as the normal fresh Windows candidate qualification entry point. Individual `preflight`, `build`, `test`, `package`, `runtime`, and `regression` commands remain supported for diagnosis and development, but users should not need to manually reproduce the release sequence step-by-step. PowerShell/BAT release tooling remains compatibility/fallback only.

## RC3.4.8 Go-runner parameter-default parity and failure diagnostics

RC3.4.8 fixes the RC3.4.7 Windows `chqdev qualify --fresh` controller defect in which Research Script `# param ... default=...` metadata was parsed but not seeded into Go-runner query resolution/compilation. Parameter defaults are now authoritative runner state and are regression-tested through a real HTTP-backed parameterized script. Release scripts must not be weakened to accommodate controller parity defects.

`chqdev` remains the **de facto and authoritative build/release process** and `chqdev qualify --fresh` remains the canonical fresh qualification command. Regression failures must preserve the precise nested command error together with session ID, run ID, run path and bundle path so top-level qualification never collapses an actionable error into a generic stage failure.



## v0.66.9.1 permanent release-governance contract

`docs/RELEASE_GOVERNANCE.md` is mandatory for every future release. Human docs, handbook and machine-readable documentation must be updated together. `chqdev preflight` must fail if process-facing docs omit `chqdev qualify --fresh`, zero tolerated CTest failures / any failure fatal, deterministic exact-archive proof, focused + Full Regression, or PowerShell/BAT compatibility-only status; it must also reject stale claims that legacy shell commands are canonical. Historical docs remain archival evidence and are not rewritten to masquerade as current instructions.


Current development candidate: v0.66.9.4-RC3.
