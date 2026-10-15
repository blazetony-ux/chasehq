> **CURRENT AUTHORITATIVE — ChaseHQ-Native 0.67.0.0-RC6.3.33 / chqdev v0.5.52 / REAL CHECKPOINT GENUINE NATIVE DIFFERENTIAL (2026-10-15).** Qualified immutable RC6.3.32 source SHA256 `3f72489f25b144cda5ca7d7416670b02e7f7fa02b70fc32811320943b75bccb6`; Windows dist SHA256 `cb4582d8abcac2f615de51a99065fb54f45d1f06cbed271135e146fe4f7e4c9e` reported by qualifying tool only. RC6.3.33 extends synthetic replay with independent **GENUINE ROM** checkpoint processes: original, real Ghidra-generated CPU-B two-opcode native substitution and forced rollback at frames 6337 and 6340. Each captures five semantic streams (CPU flags/registers/cycles, mapped R16 bus, IRQ events, machine state and raw bus). All signals must match; native commit and explicit fallback must be observed. Fresh high-frame replay verifies determinism, hashes pin checkpoint/executable/DLL. Unexpected divergence invokes genuine-subprocess prefix reduction and leaves a first-difference witness. An independent mutation of a COPIED trace is separately labelled CONTROLLED NEGATIVE, not genuine gameplay. No whole-function CFG/0x0518/producer coverage/native promotion. **Source candidate: fresh RC6.3.33 Windows ROM acceptance/Full Regression PENDING.** See `docs/RC6.3.33_GENUINE_SUBPROCESS_REPLAY_AND_ROLLBACK.md`. Previous current headers are historical.

> **AUTHORITATIVE CURRENT — ChaseHQ-Native 0.67.0.0-RC6.3.32 CHECKPOINT REPLAY REDUCTION AND CPU-B FUNCTION-CONFIDENCE (2026-10-15).** Exact immutable fully Windows-qualified RC6.3.31 source SHA256 `8ca40c81ba661a0a3274ce4203dd541fe34d39743cc914c2a63b07b037061cd4`; qualifier-reported Windows dist SHA256 `004f00ae527e5bacc8cb0791840808ff4e5e4da352c72fb2e37cd9ac8a6bfc76` separate and not independently hashed. Genuine CHQ6331 original-ROM evidence captures CPU-B PC 0x04A8 writing byte 0x80 at address 0x100802 seven times per case (35 total, zero bit7 transitions), then BTST at 0x050E and BNE at 0x0516→0x052C; 0x0518 remains unobserved. RC6.3.32 introduces actual paired original/native executable checkpoint replay via chqdev native minimize --plan JSON --out NEW_DIR: isolated ordered traces, executable/checkpoint hashes, bounded frame-prefix bisection, fresh confirmation reruns and fail-closed errors. The automatic native-reconstruct smoke uses **SYNTHETIC process fixtures only**; no full-function original/native gameplay replay certified locally. A genuine writer dependency report preserves incomplete producer history, function entry/CFG/exits/callers and scheduler uncertainty; nativePromotionAllowed=false. Windows acceptance CHQ6332.zip and fresh qualify PENDING; `chqdev v0.5.51`. See `docs/RC6.3.32_CHECKPOINT_REPLAY_AND_FUNCTION_BOUNDARY_CONFIDENCE.md`. Earlier banners below historical.

> **AUTHORITATIVE CURRENT — ChaseHQ-Native 0.67.0.0-RC6.3.31 MULTI-SOURCE FUNCTION FRONTIER AND DIAGNOSIS (2026-10-15).** Exact immutable fully Windows-qualified RC6.3.30 source ZIP SHA256 `24e7cf0fa32741f584d74908b7b707afc02797a699f9d40e1ab5c8dd32313a82`; Windows qualifier-reported `dist` SHA256 `51067d3a2dd9a8d370886619e18c6a0427c207d9f9517ece241e5ed374065ea1` separate, not independently hashed. Genuine CHQ6330.zip proves 35/35 sampled CPU-B 0x050E byte=0x80, bit7=1 -> SR.Z=0 -> 0x0516->0x052C; 0x0518 still unobserved. RC6.3.31 adds opt-in read-only CPU-A/B postcommit Bus::write witnesses overlapping 0x100802 (records zero honestly), plus automatic cross-case Ghidra/observed-edge function blocker dossier. New trace-witness CLI diagnoses the first differing original/native evidence row and shows SHA-pinned context, **not** verified deterministic scenario/input minimization. Fresh Windows writer capture `CHQ6331.zip` and `qualify --fresh` PENDING; local fixtures for NEW capture synthetic-only. Whole-function CFG, unobserved 0x0518, CPU-A/B scheduling and function-native promotion BLOCKED. `chqdev v0.5.50`. See `docs/RC6.3.31_AUTONOMOUS_FUNCTION_BLOCKERS_AND_DIAGNOSIS.md`. Older current labels below historical.

> **AUTHORITATIVE CURRENT — ChaseHQ-Native 0.67.0.0-RC6.3.30 CPU-B 0x050E BTST UPSTREAM SOURCE / 0x0516 BNE (2026-10-15).** Built from immutable fully Windows-qualified RC6.3.29 source ZIP SHA256 `087e973345581eb8603bba99ae69907a3403885449a21d4b9063e745eb094615`; Windows qualifier-reported `dist` SHA256 `4e2150384994fbb05876ff03b666fce6542cafc38ebdf1a2868ce1287bc3c6e8` is not independently byte-verified. Genuine CHQ6329.zip proves 35/35 original-ROM 0x050E instruction hooks immediately preceding 0x0516 BNE and 35 0x0516 Z-clear->0x052C branches (5 checkpoints). RC6.3.30 introduces **NEW original-ROM byte capture** from 0x100802 immediately before Ghidra `BTST.B #7,(0x00100802).L` at 0x050E; verifies bit7 against 0x0516 SR.Z, ordered hooks and actual BNE next PC. New-byte observations **synthetic-only in local fixtures**; fresh Windows ROM evidence `CHQ6330.zip` and `qualify --fresh` PENDING. Unmodified original evidence vs controlled data always distinct. `0x0518` remains unobserved, full CFG and native whole-function promotion BLOCKED. `chqdev v0.5.49`. Read `docs/RC6.3.30_CPU_B_BTST_UPSTREAM_BIT7_SOURCE.md`. Older banners below historical.

> **AUTHORITATIVE CURRENT — ChaseHQ-Native 0.67.0.0-RC6.3.29 CPU-B BNE EQUAL-CLOCK CORRECTIVE SOURCE CANDIDATE (2026-10-15).** Source comes from immutable RC6.3.28 ZIP SHA256 `18b2a4a5c32aa750e57eb525fc6b29697b7db77acca420268e084f7ce8a3ab4f` (Windows MSVC build PASS but native BNE decision audit FAIL on legitimate equal `cyclesB`). Genuine original-ROM `CHQ6328.zip` verified 1007/1007 and isolated corrected complete Ghidra/branch audit PASS: 35 real per-visit `0x0516` Z-clear BNE next-PCs `0x052C`, 35 equal-clock pairs, no `0x0518`. RC6.3.29 preserves strict lower-cycle rejection, opcode, flag, ordinal and original-ROM edge checks, plus new optional genuine-evidence regression. Latest fully Windows-qualified immutable baseline remains **RC6.3.27**, source SHA256 `deaf140c0ee129804785ee2c9026af240d7ee9d5806c77095eead93795932b39`; RC6.3.28 FAILED, RC6.3.29 Windows native+fresh qualify **PENDING**. New focused Run001 and 1153-command Full Regression required on Windows, one evidence ZIP `C:\Projects\Evidence\CHQ6329.zip`. Complete CPU-B function CFG, alternative `0x0518` execution, hardware timing and whole-function native promotion remain BLOCKED. See `docs/RC6.3.29_CPU_B_EQUAL_CYCLE_CORRECTIVE.md`. Older banners are historical.

> **AUTHORITATIVE CURRENT — ChaseHQ-Native 0.67.0.0-RC6.3.28 RC6.3.28 CPU-B ORIGINAL-ROM BNE CONDITION/IMMEDIATE NEXT-PC SOURCE CANDIDATE (2026-10-15).** Immutable fully Windows-qualified RC6.3.27 source SHA256 `deaf140c0ee129804785ee2c9026af240d7ee9d5806c77095eead93795932b39`; qualifier-reported Windows distribution SHA256 `828876f06979ce6f08b3970374e796abbc03b1980d561f7e09c32602ea5e5604` (actual Windows `dist` archive not independently supplied). Genuine `CHQ6327.zip` passed the previous IRQ bus gate and 18/18 Windows CTests; focused Run001 10 commands, Full Regression Run002 1,153 commands PASS. Genuine five-case original-ROM CPU-B 0x0516 BNE visits: 7 per case, 35 total, all SR.Z clear and observed aggregate successor 0x052C; **0x0518 was never visited**. RC6.3.28 independently records each BNE's original-ROM pre-branch SR/ordinal/frame and immediately succeeding CPU-B instruction PC, crosschecks original-ROM hook and Ghidra/edge evidence, and fails closed on mismatched flags/targets, dropped records or newly observed 0x0518 requiring separate review. **New per-visit successor capture is synthetic-only in local tests, real RC6.3.27 SR/edge prerequisites verified; fresh RC6.3.28 Windows ROM acceptance and `qualify --fresh` PENDING.** Function CFG/callers, 0x0518 runtime path, emulator cycle-exact IRQ timing, native function and autonomous promotion remain BLOCKED. One evidence ZIP: `C:\Projects\Evidence\CHQ6328.zip`. See `docs/RC6.3.28_CPU_B_BNE_CONDITION_AND_NEXT_PC.md`. Older banners retained as history.

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

> **AUTHORITATIVE CURRENT — v0.67.0.0-RC6.3.13 CPU-B TWO-INSTRUCTION LIVE SOURCE CANDIDATE (2026-10-14).** RC6.3.12 real Windows generated TST.W+BEQ pair ABI 4/4 PASS from short output path, with previous RC6.3.11 authentic single BEQ live parity PASS. RC6.3.13 introduces guarded actual native TST.W+BEQ replacement and original/generated/forced-ROM rollback differential, mapped R16, IRQ and full machine evidence. Local 68000 tests PASS; **actual RC6.3.13 Windows live pair acceptance PENDING**. Full function/autonomous M1–M7 unproven; inherited Full Go regression 1176>850 still FAIL. See `docs/RC6.3.13_LIVE_CPU_B_PAIR.md`. Older CURRENT statements are retained only for historical provenance.

> **AUTHORITATIVE CURRENT — v0.67.0.0-RC6.3.12 CPU-B TWO-INSTRUCTION GENERATED ABI SOURCE CANDIDATE (2026-10-14).** RC6.3.11 authentic Windows single CPU-B BEQ original/native/fault PASS (17,892 commits, exact CPU/IRQ/machine parity; 869/869 evidence hashes). RC6.3.12 adds separate original-Ghidra TST.W 0x4D8 + BEQ 0x4DE pair C++ ABI from authenticated ROM extension words and 17,903 consecutive observed edges; **pair is offline only, NOT yet a live native replacement**. All CPU-A/B enclosing function boundaries, autonomous M1–M7 and full Go budget 1176>850 remain open. See `docs/RC6.3.12_CPU_B_TWO_INSTRUCTION_ABI.md`. Earlier status paragraphs remain historical.

> **AUTHORITATIVE CURRENT — v0.67.0.0-RC6.3.11 CPU-B IRQ-OWNERSHIP CORRECTIVE SOURCE CANDIDATE (2026-10-14).** RC6.3.10 actual Windows `native reconstruct --cpu-b-live` failed only at the CPU-B original-ROM baseline because 12 interrupt-vector/stack bus effects (four genuine IRQ entry groups) were wrongly tagged as BEQ.B data effects by stale `trace_pc`; its CPU-A health/copy, shared dispatcher and real CPU-B Ghidra/compiled ABI gates all PASS. RC6.3.11 uses Musashi interrupt ACK to assign each IRQ vector/stack transaction to a dedicated ordered IRQ stream, preserves strict zero-branch-data and original/native/fallback CPU/cycle/bus/machine parity, and adds real-ACK synthetic and missing/mutated IRQ evidence tests. **RC6.3.11 authentic Windows native CPU-B equivalence is PENDING**, as are complete-function/autonomous promotion and inherited Go Full Regression budget 1176>850. See `docs/RC6.3.11_IRQ_OWNERSHIP_CORRECTIVE.md`. Earlier status blocks remain historical.

> **AUTHORITATIVE CURRENT — v0.67.0.0-RC6.3.10 CPU-B GENUINE GENERATED LIVE BEQ SOURCE CANDIDATE (2026-10-14).** RC6.3.9 actual Windows CPU-B World-ROM CRC/SHA, five Ghidra opcode sites, and independently compiled generated BEQ C++ ABI all PASS (20261014-193243 evidence; 788/788 SHA entries valid). RC6.3.10 adds opt-in **CPU-B live BEQ 0x0004DE** shared-dispatch substitution and genuine original/generated-native/forced-rollback comparisons, with 68000 timeslice/IRQ guard and complete CPU state, cycle, zero mapped data-bus and full machine endstate parity. **RC6.3.10 Windows real-ROM live parity is PENDING**; synthetic Musashi and local tests do not establish game-ROM equivalence. No complete function boundaries, CPU-B auto-promotion or unrestricted M1–M7 signoff. See `docs/RC6.3.10_LIVE_CPU_B_BEQ.md`. Historical sections below remain snapshots.

> **AUTHORITATIVE CURRENT — v0.67.0.0-RC6.3.9 CPU-B ROM/GHIDRA GENERATED ABI (2026-10-14).** RC6.3.8 Windows original/native/fallback shared CPU-A dispatch PASS with authentic 5,234 CPU-B transition rows. RC6.3.9 adds guarded original CPU-B World ROM CRC/SHA verification, five-PC Ghidra static provenance, and compiled BEQ branch C++ shared ABI candidate; **not yet run against real CPU-B Ghidra on Windows**, no CPU-B live native substitution, M6–M7 autonomous promotion BLOCKED. See `docs/RC6.3.9_GENUINE_CPU_B_GHIDRA_GENERATOR.md`. Historical sections retained.

## 0.67.0.0-RC6.3.9 — CPU-B original-ROM Ghidra and generated C++ ABI

RC6.3.9: authentic CPU-B ROM CRC/SHA-verified headless Ghidra discovery and original-runtime-hot-PC crosswalk; generates and independently compiles a guarded BEQ C++ DLL from real Ghidra static instruction evidence when run on Windows with original ROM/Ghidra. OFFLINE CPU-B generated ABI ONLY; no CPU-B live native commits, CPU-B original/native/forced rollback equivalence or M1–M7 autonomous promotion. Existing RC6.3.8 Windows CPU-A shared dispatch and CPU-B runtime edge proof preserved; new Windows CPU-B Ghidra run PENDING.

The optional `chqdev native reconstruct --fresh --live-health --cpu-b-discover` appends a fail-closed original ROM CRC/SHA, authentic headless Ghidra five-site opcode/edge correlation, and independent compiled CPU-B BEQ branch DLL/ABI gate to the previously qualified shared-dispatch workflow. Source package does not bundle copyrighted ROM bytes or generated fake Ghidra findings; raw CPU-B reconstructed ROM is deleted after headless analysis and excluded from ZIP evidence. It must not be reported as CPU-B live native replacement. Fresh Windows gate PENDING. Inherited full Go suite structural 1176>850 budget failure remains open.

## 0.67.0.0-RC6.3.8 — CPU-B census/edge lookup corrective

RC6.3.8 source candidate from RC6.3.7: real Windows CPU-A health and copy live replay retained; correct six- vs eight-digit CPU-B census/edge PC key mismatch without weakening strict evidence gate. RC6.3.7 Windows uploaded evidence independently re-audited after correction, all five ranked CPU-B ROM PCs correlate. Changed Windows build/live acceptance PENDING; CPU-B native substitution and autonomous M1-M7 promotion BLOCKED.

Implementation: `rc637ReadObservedEdges` returns canonical 24-bit `B:0x%06X` keys consistent with `readNativeTsv`, preserving exact edge counts, opcode fingerprints, duplicate rejection, and absent-edge failure. New `native_dispatch_rc638_test.go` runs full acceptance against Windows-format census/edge fixtures and optionally the genuine RC6.3.7 evidence ZIP, with negative mutants and old-bug red test.

Acceptance on Windows: `\.\chqdev.bat build debug; if ($LASTEXITCODE -eq 0) { \.\chqdev.bat native reconstruct --fresh --live-health }`. New runtime proof pending; Full Go regression inherited 1176 > 850 budget blocker not waived.

> **AUTHORITATIVE CURRENT - v0.67.0.0-RC6.3.6 SECOND GHIDRA-GENERATED COPY BLOCK (2026-10-14).** RC6.3.5 Windows authentic caller-to-health-tail and CPU-B original census PASS. RC6.3.6 compiles a second Ghidra-derived CPU-A 16-instruction copy library and adds opt-in ROM-backed original/native/forced-rollback parity; 15/15 local CPU tests, Windows native proof PENDING. No full-function or CPU-B native promotion. See `docs/RC6.3.6_SECOND_LIVE_BLOCK.md`. Historical sections below remain as recorded.

> **AUTHORITATIVE CURRENT — v0.67.0.0-RC6.3.4 OBSERVED HEALTH FUNCTION BOUNDARY / CPU-B CANDIDATE PORTFOLIO SOURCE CANDIDATE (2026-10-13).** RC6.3.3 real Windows 5-case controlled original/native/forced-rollback health matrix PASS with 3 distinct authentic original-ROM paths, 9/9 candidate sites, five native commits, five verified ROM fallbacks; 508/508 evidence hashes independently verified. RC6.3.4 adds real 68000 stacked return/entry captures, independent ROM RTS and observed caller audit, and CPU-B original-ROM PC census into a persistent candidate portfolio; **actual Windows RC6.3.4 execution and release qualification PENDING**. Observed boundary does not independently certify all callers or a complete function CFG. CPU-B observed PCs are not functions. No generalized autonomous promotion. RC6.2.6 steering instruction baseline remains signed off. See `docs/RC6.3.4_BOUNDARY_PORTFOLIO.md`. All older CURRENT headings below are historical.

> **AUTHORITATIVE CURRENT — v0.67.0.0-RC6.3.3 CONTROLLED HEALTH BRANCH-MATRIX DEVELOPMENT CANDIDATE (2026-10-13).** RC6.3.2 real Windows bounded CPU-A health-function live differential PASS: one generated multi-instruction commit and authentic forced rollback with exact CPU/bus/mapped state; manifest 198/198 verified. RC6.3.3 adds original-ROM instruction-hook path capture and controlled entry-state 5-case x original/native/rollback differential; real Windows matrix and release qualification **PENDING**. Controlled branch coverage cannot be presented as natural gameplay or independently reviewed full-function coverage. No automatic CPU-B promotion or autonomous multi-routine orchestration. RC6.2.6 steering instruction baseline remains signed off. See `docs/RC6.3.3_BRANCH_MATRIX.md`. All older CURRENT headings below are historical.

> **AUTHORITATIVE CURRENT — v0.67.0.0-RC6.3.2 TRACE-BOUNDARY CORRECTIVE SOURCE CANDIDATE (2026-10-13).** Actual Windows RC6.3.1 evidence showed one generated CPU-A target-health function commit with exact CPU-state and mapped end-state parity but an extra caller W32 0x102DE0 attributed to the function. RC6.3.2 fixes post-native-commit bus PC attribution and adds a synthetic Musashi original/native/forced-rollback regression which fails against RC6.3.1 and passes with the correction; full game-ROM Windows differential and release qualification are **PENDING**. No RC6.3 general-function promotion or CPU-B autonomy. RC6.2.6 bounded steering M1-M7 signoff retained. See `docs/RC6.3.2_TRACE_BOUNDARY.md`. All older CURRENT statements below are historical.

> **AUTHORITATIVE CURRENT — RC6.3.1 PHASE 2 SOURCE CANDIDATE (2026-10-13).** Generated Ghidra target-health function C++ shared library, experimental opt-in live CPU-A entry substitution and original/native/forced-rollback replay with fail-closed CPU-state/bus/endstate gates. **Linux source CTests and offline ABI fixtures are not Windows live proof**; function boundary provisional, generalized CPU-A/B autonomous M1–M7 unproven. RC6.2.6 bounded steering instruction signoff remains the foundation. See `docs/RC6.3.1_LIVE_FUNCTION.md`. All previous CURRENT headings below are historical snapshots.

> **AUTHORITATIVE CURRENT — 0.67.0.0-RC6.3.1 DEVELOPMENT SOURCE CANDIDATE (2026-10-13).** RC6.2.6 bounded CPU-A steering instruction M1–M7 technical acceptance SIGNED_OFF from real Windows evidence: 240 native replacements, 240 identical CPU return states, ordered bus/mapped-state parity, genuine forced rollback. RC6.3 adds three real Ghidra-derived multi-instruction generated C++ candidates including target-health branching; offline fixtures PASS but **RC6.3 whole-function autonomous M1–M7 and Windows qualification are NOT SIGNED OFF**. See `docs/RC6.3_AUTONOMOUS_RECONSTRUCTION.md` and `RC6_STATUS.md`. Older CURRENT headings below are historical snapshots only.

## RC6.2.6 — Native 0x8210 boundary and fail-closed replay correction

- Real Windows RC6.2.5 evidence (native-accept-20261013-185540.zip) proved 240 successful instruction replacements but zero native CPU return-state records. RC6.2.6 captures actual registers, SR, PC, RAM and Musashi-cycle state immediately after native MOVE.W completion at the equivalent 0x8210 boundary; it also resolves pending scalar shadow comparison there.
- Forced corrupt-output probe is now rejected **before** bus read/write; ROM fallback retains the original instruction's observable transaction order (not a speculative write followed by undo).
- Ghidra local-window ledger canonicalises PC keys identically to the authoritative census and emits observed counts rather than zero for 0x820C.
- An instruction-level proof does NOT sign off the entire Ghidra function or generalized RC6.3 autonomous reconstruction. Windows qualification and real ROM-backed M1–M7 acceptance remain mandatory.

## 0.67.0.0-RC6.2.6 (evidence-driven runtime coverage and steering sign extension)

- Corrected M2 census parser canonicalisation. RC6.2.4 actually recorded 240 genuine CPU-A steering hits; prefix-case mismatch previously reported zero.
- Matched 68000 `EXT.W D1` low-byte sign extension, including raw `0x0FE0` indexing `-32`, in both native steering shadow and generated-library runtime substitution.
- Added actual five-interval ROM-evidence regression, malformed TSV fail-closed cases, and incremented release/tool identities.
- Windows fresh qualification and ROM-backed M1–M7 acceptance remain pending; RC6.3 deferred.

## 0.67.0.0-RC6.2.4 (Windows compile corrective candidate)

- Corrected `src/runtime.cpp` Windows SDK/Musashi include ordering. The SDK `FLOAT128` typedef must be parsed before Musashi's `FLOAT128` preprocessor feature definition.
- Defined `NOMINMAX` before loading Windows headers to preserve C++ `std::min` and `std::max`, and isolated the colliding `EXCEPTION_ILLEGAL_INSTRUCTION` macro before importing Musashi.
- Added a Go regression gate preventing reintroduction of these Windows header-order and macro errors, and recompiled bundled chqdev tooling.
- No change to the instruction-scoped native replacement implementation or M1–M7 acceptance criteria. Windows MSVC rebuild and real ROM-backed acceptance remain pending.
- RC6.2.3 archive remains an immutable failed Windows source candidate; this fix is a new release version.

**CURRENT RELEASE CANDIDATE: 0.67.0.0-RC6.2.6.** Real opt-in CPU-A `0x820C` guarded generated-library native instruction replacement and original/native/rollback full-state acceptance; Windows and ROM-backed proof pending. Enclosing Ghidra function still unreviewed. See `RC6_STATUS.md` and `docs/RC6.2.6_M1_M7_ACCEPTANCE.md`. All prior “current” paragraphs below are **historical only**.

> **AUTHORITATIVE CURRENT STATUS — 0.67.0.0-RC6.2.2 SOURCE CANDIDATE (2026-10-12).** RC6.2.1 Windows qualification PASS (SHA256 `c8422141d9d079ce522f0b1dc12a95527c5d80e9055b4c874fb356493b46174b`); actual CPU-A Ghidra steering export `0x820C` PASS. RC6.2.2 adds self-contained, fail-closed M1–M7 **acceptance evaluation**, not automatically proven unrestricted native replacement. Full acceptance and Windows qualification must be tested; see `docs/RC6.2.2_M1_M7_ACCEPTANCE.md` and `RC6_STATUS.md`. Prior CURRENT/unqualified statements below are preserved as **historical snapshots, not current release status**.

> **CURRENT RC6.1 DEVELOPMENT — v0.67.0.0-RC6.2.1.1; native steering shadow opt-in, Windows qualification outstanding.** See `RC6_STATUS.md`; older status below is historical.

> **RC6 DEVELOPMENT STATUS (not qualified):** v0.67.0.0-RC6. RC5.3 sections below are historical. See `RC6_STATUS.md` at release root.

> **CURRENT: v0.67.0.0-RC5.3 native sprite-clear SUBSTITUTION DEVELOPMENT CANDIDATE (UNQUALIFIED), 2026-10-11.** The original Windows-qualified RC5 parent SHA-256 `47def65d75056fb765d5b60dfe5a4c4a21bdcd3214ebb84920dd62a0d8b37ddf` remains immutable. RC5.1 headless live-shadow corpus PASS 12/12 cases (96/96 sprite transfers and 24/24 class-6 road generator comparisons); RC5.3 adds an experimental, opt-in native substitution of the trailing sprite-clear DBRA loop ONLY. Bulk copier and final RTS remain original ROM; normal emulation is default. RC5.3 Windows native A/B equivalence and exact-archive qualification pending. See `docs/RC5.3_NATIVE_CLEAR_SUBSTITUTION.md`; all older release-status headings below are historical.

## v0.67.0.0-RC5 — source candidate, NOT Windows-qualified (2026-10-10)

- Native C++ candidate modules: independent sprite transfer and CPU-B 104-entry road mask, compiled into the existing runtime library; exact offline captured evidence comparison PASS (6144 + 832 bytes).
- Read-only, environment-opt-in original ROM entry/return SHADOW hooks and JSON evidence summary; live Windows checkpoint proof PENDING, no native replacement.
- Self-contained synthetic Windows GUI preview, actual Workbench Next Steps JSON refreshed to reflect verified RC4.7 research findings, focused RC5 regression catalogued, and documentation synchronized with chqdev preflight.
- Linux CPU-only 13/13 CTest PASS, Go preview tests and PE cross-build PASS. `chqdev` Full Regression structural-budget test remains FAIL (1176 versus <850) and exact Windows `chqdev.bat qualify --fresh` was NOT RUN. Do not promote.

> **RC5 SOURCE CANDIDATE (`0.67.0.0-RC5`), NOT WINDOWS-QUALIFIED.** Original RC4.7 source archive supplied by user; RC5 integrates offline native C++ sprite/road candidates and standalone synthetic Windows preview, updates live Workbench Next Steps. No live SHADOW/COMPARE or native replacement is implemented. Follow `docs/RC5_NATIVE_PREVIEW_AND_SHADOW.md` and exact Windows `chqdev.bat qualify --fresh` gate before promotion. Older version status sections below are historical.

## RC4.7 — Static Analysis / REA tooling candidate

See `docs/REA_GHIDRA_RC47.md` and `docs/DOCUMENTATION_AUDIT_0.67.0.0-RC4.7.md`. **Current candidate RC4.7; promoted parent RC4.6. Windows qualification pending.** Dedicated Workbench REA workspace, bounded `ghidra.wait`, Ghidra run evidence attachment, corrected result path, Java/Jython preflight and flow-edge export. No Capstone library is shipped in this candidate; source vendoring is blocked pending a verified upstream archive. No native emulator CPU replacement.

> **AUTHORITATIVE PROMOTED BASELINE — v0.67.0.0-RC4.6 (2026-10-10).** Windows `chqdev.bat qualify --fresh` reports **11/11 stages PASS** for exact frozen ZIP SHA-256 `9e76a107e19f3e28a54a8a8b3bdaed630511e02f1da3e3e7ae274c07d863580f`. User explicitly authorised promotion following focused, full, and successful headless Ghidra/REA crosswalk evidence. This is a **post-qualification documentation overlay**; it does not change the immutable tested archive. See `docs/RELEASE_VERIFICATION_RC46.md`, `promotion/PROMOTION_RECORD.json`, and `docs/REA_GHIDRA_NEXT_SCOPE.md`. Earlier RC4.6 “candidate/unqualified” statements below are **historical build-time notes, not current release status**.

## RC4.6 Ghidra integration — historical source-candidate description

Promoted immutable parent: RC4.5 SHA-256 `443ffe623a38de4813691c436e05d6f92d9e113a8abae0219824fcc77e9aae11`. RC4.6 adds an optional asynchronous local headless 68000 Ghidra workflow to the Research API, Script Console, and Native Reconstruction Lab. It verifies a user-owned CPU-A ROM fingerprint, runs packaged scripts in a separate process, exports unreviewed static candidates and XREFs, and correlates with previously captured genuine ROM census TSVs. The live SDL emulator is not restarted; no REA agent or native replacement is claimed. See `docs/GHIDRA_WORKBENCH_RC46.md`. Fresh Windows qualification and genuine headless Ghidra execution are pending. Historical RC4.5 candidate lines below remain provenance, not current status.


> **AUTHORITATIVE PROMOTED BASELINE — v0.67.0.0-RC4.4 (2026-10-10).** Exact Windows-qualified archive SHA-256: `8a8b715646ee07619891ea137425bd24272d5df2b84c60170105a164ec3d8912`. User-authorised promotion; all eleven stages in Windows `out/chqdev/qualification.json` are PASS, including `promotable`, focused and full regressions, and run-history. The immutable qualified archive is unchanged. Full provenance: `promotion/PROMOTION_RECORD.json` in the separate promotion supplement; historical candidate notes below describe earlier states and are retained.

> **Current development candidate:** `v0.67.0.0-RC4.4`. Windows exact-archive qualification pending. See `docs/NATIVE_PROJECTION_RC44.md` for present scope and limitations.

## RC4.3.2 — authoritative history CTest signature corrective (unqualified)

The exact RC4.3.1 Windows `chqdev.bat qualify --fresh` log passed configuration and debug compilation, then failed CTest 10/13, `authoritative_history_tests`, with `Authoritative history function absent`. The production `Get-AuthoritativeScriptRuns` function is present, but its declaration changed in RC4.3.1 from `()` to `([string]$onlySession='')`. The PowerShell test's regex still matched only `()` and falsely reported absence. Correct the extractor to allow parameters and whitespace, **without dropping** its 320-run ordering/pagination/version tests. Extend Windows CTest coverage to exercise the new current-session filtering path. Add an independent Go regression guard against repeating this signature drift. Do not change production execution, the existing run-history performance fix, or Full Regression assertions. 0 tolerated CTest failures; this candidate is NOT promoted until the final exact ZIP passes Windows qualification. The historical Full Regression expanded-command budget failure remains unresolved.

## v0.67.0.0-RC4.3.2 — Workbench startup/history responsiveness corrective

- Fixed RC4.3 Windows runtime-restart readiness regression where 23-29s global history scans blocked 10s health/status requests. History now loads on demand; startup is not held hostage by historical evidence enumeration.
- Current-session run listings are scoped; artifact counts recorded in run metadata at finalization. Existing bundles/metadata stay intact, and unknown old artifact totals are labelled unindexed.
- Added permanent native `chqdev` regression guard for startup isolation and history-list constraints.
- Existing Go command-budget failure and Windows full qualification remain outstanding.

## v0.67.0.0-RC4.3.2 CURRENT DEVELOPMENT CANDIDATE — NOT QUALIFIED

This source candidate extends v0.67.0.0-RC4.2.3. Its authoritative new feature and limitation inventory is `docs/NATIVE_MIGRATION_RC43.md`. The prior RC4.2.3 Windows debug compile was PASS, but it does **not** qualify RC4.3. Required before promotion: exact-archive Windows `chqdev.bat qualify --fresh` including strict CTest, focused and full regressions, deterministic package verification and readiness. The known pre-existing Full Regression Go structural budget check remains FAIL (1,170+ commands versus <850); do not delete unique assertions or relax that gate to conceal the failure. New code paths MUST keep native replacement disabled by default. User-controlled ROM, SDL and content paths stay outside release ZIPs. Fresh release setup MUST find the existing stable `build-local.json` or provide a precise doctor error. The bundled Windows chqdev executables MUST be present, tested and accompanied by matching release metadata.

## v0.67.0.0-RC4.2.3 — regression timing diagnostics

- Added per-executed-command Go runner wall-clock timing and grouped JSON sidecar, retained on successful and failed runs.
- No regression commands removed; existing coverage and command-budget gate unchanged.
- Windows qualification pending.

## v0.67.0.0-RC4.2.3 (candidate)

- Audit Full Regression for repeated scenario bodies. Remove automatic RC2.1 and RC3.2 BG Shadow reruns, which are identical to the retained current BG Shadow regression.
- Preserve historical scripts and all distinct checks; add explicit coverage-equivalence documentation.
- Keep the <850 structural command budget and full Windows qualification gate unchanged.

> Current development candidate: **v0.67.0.0-RC4.2.3** — CHQPACK sprite/tile atlas and PNG export; Windows qualification pending.

> Current development candidate: **v0.67.0.0-RC4.2.3** — CHQPACK Browser, overlay inspection and provenance. Windows qualification pending.

> Current development candidate: **v0.67.0.0-RC4** — ROM/CHQPACK source switching and RC3.1 release consistency corrections.

# v0.67.0.0-RC3

- Added deterministic `chqimport` World-ROM importer and `CHQPACK1` content format.
- Added `--content-pack` runtime loading for CPU, graphics, road, spritemap and PROM content with no loose-ROM-directory dependency.
- Retained `--roms` as the oracle/reference path.
- Added CHQPACK CRC integrity validation, provenance metadata and round-trip unit coverage.
- Updated one-command/research launchers to accept a content pack.

> Current development candidate: **v0.67.0.0-RC2.1** — Native sprites + Native TC0100SCN text/HUD; TC0100SCN BG Shadow pen-0 parity corrective.

> Exact source baseline: qualified **v0.67.0.0-RC1.1** SHA-256 `ea4ebaafe07cd1c15b64c07ed8e92e32fd0e39cdcc360e31dd7b09c676b77f1d`. RC2 keeps the tooling freeze and moves visible rendering forward. See `docs/DOCUMENTATION_AUDIT_0.67.0.0-RC2.1.md`.
## v0.67.0.0-RC2.1 — TC0100SCN BG opaque pen-0 corrective

- Windows RC2 focused Run001 and Full Regression Run002 both completed PASS, but the new BG Shadow telemetry exposed a deterministic renderer mismatch rather than a qualification-infrastructure failure.
- **Physical BG0 compared cleanly throughout**: focused `228/228` and Full Regression final focused section `125/125` comparisons with zero BG0 mismatches.
- **Physical BG1 mismatched on every compared sample** when it was selected as the opaque bottom plane. First divergence was `original_pixel=0x00000000` versus `native_pixel=0xff000000` with matching coverage at the bottom-screen pen-0 area.
- The captured Stage-2 reconstruction reports `tileControl6=0x0028`; bit 3 therefore selects physical BG1 as the bottom/opaque TC0100SCN plane.
- Source inspection localized the defect exactly: the authoritative runtime returns pixel value `0` for palette pen 0 while still setting bottom-layer priority/coverage; the independent renderer instead converted pen 0 through TC0110PCR and produced opaque black.
- RC2.1 makes independent BG palette conversion return zero for pen 0 while retaining coverage on opaque passes, and adds a unit regression for the exact physical-BG1 opaque-pen-0 contract. No sprite, text/HUD, road, gameplay, script-engine or general-tooling behaviour is intentionally changed.


> Historical qualified candidate: **v0.67.0.0-RC1.1** — native sprites default; text/HUD Shadow zero-diff bring-up; native sprite forensic overlap-coverage corrective.

## v0.67.0.0-RC1.1 — Native sprite forensic overlap-coverage corrective

- Windows RC1 focused regression passed with Native sprites active and TC0100SCN text Shadow at 171 compared frames / 0 mismatch frames.
- Full Regression isolated a forensic-only regression: Native sprite output and final owners matched exactly, but every occupied pixel reported `coverage=1`, losing overlap density required by `frame.snapshot.assert-overlap`.
- Independent native sprite rendering now counts every in-bounds non-zero candidate before first-nontransparent occupancy, preserving Original coverage values while leaving pixels and winning owners unchanged.
- Added focused Native overlap assertion and unit coverage so this cannot regress silently.
- Tooling freeze remains in force; no unrelated tooling expansion.


> Historical candidate: **v0.66.9.5-RC3**.

## v0.66.9.5-RC3 - Native Promotion Framework candidate

- Fail-closed Sprite Native-only backend mode.
- Generic native promotion status/reset/assert/capture APIs and Native Progress Workbench matrix.
- Automatic first-divergence Original/Native/diff evidence.
- Original/Native/Shadow timing, fallback and blocked-frame counters.
- Canonical 10-checkpoint sprite promotion corpus across Stage 1 and Stage 2.
- Explicit tooling freeze after RC3 qualification; next target is sprite promotion to NATIVE, then TC0100SCN text/HUD Shadow.

> Historical candidate: **v0.66.9.5-RC2**. See `docs/DOCUMENTATION_AUDIT_0.66.9.5-RC2.md` for Runs003-012 evidence and RC2 changes.

## v0.66.9.5-RC2 - candidate

- ROM-backed Sprite/Vehicle Atlas with raw-pen and palette views, provenance CSV, contact sheets and ZIP export.
- Expanded confirmed player/target/beacon semantic families from RC1.1 Runs008-010.
- Source/effective sprite inspection fixes the Run011 presentation-override identity ambiguity.
- Beacon regression now models the authentic staged descriptor-to-live presentation delay proven by Runs004-007.

# v0.66.9.5-RC1.1 - Workbench Native Reconstruction Lab bridge hotfix

- Fixes the Native Reconstruction Lab Original / Shadow / Native / Reset-verification buttons returning `Command not allowed by web bridge`.
- Root cause: the UI correctly issued native `backend sprites ...` commands, while the safe raw-command bridge omitted the `backend` first-token allow-list entry even though the Research API/script actions were present and fully qualified.
- Adds a static parity regression that enumerates literal Workbench `cmd(...)` first verbs and fails if any UI command is not admitted by the safe bridge.
- No emulation, renderer, checkpoint, ROM-decoder, snapshot, speed, beacon or collision behaviour is intentionally changed from the exact Windows-qualified v0.66.9.5-RC1 source.

> Current development candidate: **v0.66.9.5-RC1.1** from exact final Windows-qualified **v0.66.9.4** SHA-256 `a50b3c571c845b16d3a3f9250144c345d25025bb516458f9f2dedf10885e9a40`. RC1 is evidence-driven from Runs005-010 and requires exact-archive Windows `chqdev qualify --fresh` before promotion.

## v0.66.9.5-RC1.1

- Fix structured snapshot semantic sprite replacement parity and reset asset visibility telemetry.
- Add `frame.snapshot.parity` and `sprite.asset-resolve`.
- Add map509 to semantic player body coverage.
- Fix 32-bit fixed-point speed set/freeze.
- Couple beacon phase/request forcing (Windows presentation prove-off pending).
- Add structured target-contact events and gameplay-aware Timeline ranking.
- Add focused RC1 regression and Full Regression tail wiring.

> Current release: **v0.66.9.4**. Promoted from exact Windows-qualified **v0.66.9.4-RC3.1.3** SHA-256 `48534c0bd3d8e4a40987e65089b07046535976675ca2e36d0ebf3a6d9584383e`. The final archive is regenerated for release identity/docs/focused-regression wiring only and requires one exact-final-archive `chqdev qualify --fresh` closure before immutable publication.

## v0.66.9.4

- Promoted from exact Windows-qualified v0.66.9.4-RC3.1.3 SHA-256 `48534c0bd3d8e4a40987e65089b07046535976675ca2e36d0ebf3a6d9584383e`.
- Native sprite backend is REPLACEABLE; Original/Shadow/Native switching and Stage-1/Stage-2 parity evidence carry forward.
- Format-aware semantic player-car replacement supports OBJ-A -> OBJ-B64 yellow-target rendering and persists across known steering body-map changes.
- Semantic HUD show/hide/reposition includes live SPEED value/unit suppression under acceleration.
- Complete TC0100SCN charset export, Stage-2 checkpoints, police-beacon override, same-format map substitution and extensive graphics/evidence tooling carry forward.
- chqdev v0.5.8 provides lightweight startup readiness, explicit regression progress markers and late authoritative run-history gating.
- Final promotion changes identity/docs/focused-regression/package metadata only; no intentional behavior change from RC3.1.3.

## v0.66.9.4-RC3.1.3

- chqdev v0.5.8 removes authoritative run-history enumeration from startup readiness; Workbench health, exact native identity and mobile pairing are the lightweight startup gate.
- `qualify --fresh` runs the expensive authoritative history endpoint only after focused + Full Regression, with a 30-second cold-scan budget, and promotion remains fail-closed on that result.
- Qualification console output reports readiness component status/elapsed time and the exact failed component during retry.
- RC3.1.2 semantic yellow-target steering persistence and live SPEED suppression are carried forward unchanged and remain in the RC3.1.3 focused regression.
- Windows exact-archive `chqdev qualify --fresh` is still required before promotion.

> Current release: **v0.66.9.4**. Promoted from exact Windows-qualified **v0.66.9.4-RC3.1.3** SHA-256 `48534c0bd3d8e4a40987e65089b07046535976675ca2e36d0ebf3a6d9584383e`. The final archive is regenerated for release identity/docs/focused-regression wiring only and requires one exact-final-archive `chqdev qualify --fresh` closure before immutable publication.

- RC3.1.2 binds the yellow-target replacement to semantic `player_car.body` across the proven player body-map family and expands SPEED suppression to source rows 4–5, columns 32–41; focused proof now steers both directions and captures before assertions.

- RC3.1.1 corrects the focused-regression contract: Research Script v2 `assert status` supports equality only, so release assertions use boolean status fields `sprite_asset_visible=1` and `hud_speed_region_empty=1`; native banner now reports chqdev v0.5.7 consistently.
> Current development candidate: **v0.66.9.4-RC3.1.2**. RC3.1.2 keeps the proven cross-format yellow-target renderer but binds it to semantic `player_car.body` across steering/orientation maps, expands SPEED suppression to the complete live destination block, and retains fail-closed proof plus chqdev readiness/console UX. Exact-archive Windows `chqdev qualify --fresh` remains required.



## v0.66.9.4-RC3.1.2

- Semantic player-body replacement now follows the complete proven body-map family instead of only map516, preventing authentic-player fallback during steering.
- SPEED semantic destination coverage expands from cols36..41 to cols32..41 after live acceleration evidence proved digits use the omitted cells.
- Focused regression adds right/left steering persistence checks and captures HUD evidence before fail-closed assertions.

## v0.66.9.4-RC3.1.1

- Corrects RC3 cross-format semantic replacement by fitting target-map non-transparent intrinsic bounds to the authentic source content box instead of inheriting only the source descriptor canvas.
- Corrects SPEED semantic suppression to destination text cells at source rows 4..5, columns 36..41; dynamic character RAM remains authentic underneath.
- Adds fail-closed focused assertions for visible replacement pixels and zero hidden-SPEED-region pixels after accelerator-driven live updates.
- chqdev v0.5.7 retries transient runtime readiness and prints regression START/COMPLETE/FAILED lines during `qualify --fresh`.

> Current development candidate: **v0.66.9.4-RC3.1.2**. Source lineage remains the immutable promoted **v0.66.9.3** baseline (SHA-256 `6da88889dc8214c6058644e44e4d29f6a2117c9f0e92d99c8d906c280fc3462e`); **v0.66.9.4-RC2** (`50a8fb6d2c4be5fdafdea5b3804d6f2e1f1e5e62685160d3dc6f6d65237a6f41`) passed fresh Windows qualification. RC3.1.2 preserves the proven cross-format target renderer, makes player replacement semantic across steering/orientation body maps, and completes dynamic SPEED destination suppression. Exact-archive Windows `chqdev qualify --fresh` remains required before promotion.

## v0.66.9.4-RC3

- Added format-aware native sprite asset replacement with explicit OBJ-A / OBJ-B64 / OBJ-B32 target format, palette, flip and X/Y alignment controls.
- Added Workbench one-click `Player -> Stage 2 yellow target` proof using player map 516 and target map 190/palette 73 through Native rendering.
- Fixed semantic SPEED hiding to include the authentic 6x2 dynamic character block at TC0100SCN source rows 4..5, columns 32..37; live character rewrites no longer reappear while Speed is hidden.
- Added Script/API actions `sprite.asset-replace` and `sprite.asset-replace.clear` with evidence/status accounting.
- Added focused RC3 regression and retained RC2 Native/Shadow, Stage-2, charset export, HUD layout and map-substitution coverage.


## v0.66.9.4-RC2

- Added Native Reconstruction Lab foundation and live sprite `Original | Shadow | Native` backend selector.
- Added live Shadow comparison counters / first-divergence metadata and experimental Native placed-sprite rendering.
- Added global presentation-only existing-sprite map substitution with optional palette override.
- Added complete live TC0100SCN 256-character charset atlas/glyph/raw/usage export.
- Added conservative Stage-2 gameplay/enemy-present checkpoints and cross-stage sprite shadow regression.
- Fixed Experimental HUD Layout card width/responsive clipping observed in RC1.
- Preserved RC1 police-beacon/HUD controls and immutable v0.66.9.3 release lineage.

## v0.66.9.4-RC1

- Added evidence-backed experimental police-beacon override: Authentic / Force ON / Force OFF over CPU-A `0x101854`; Force ON preserves the original steady palette-flash path and checkpoint load resets the intervention to Authentic.
- Added presentation-only semantic TC0100SCN HUD controls for High Score, Time, Distance, Score, Speed, Turbo markers and Stage/Credit, with signed pixel offsets plus Authentic / Minimal / No HUD presets.
- Added native command, Script API and Experimental/Game Lab parity for beacon/HUD controls.
- Layered frame reconstruction metadata now records semantic HUD presentation state.
- Added `hud_layout_tests`, runtime beacon lifecycle coverage, Workbench static RC1 checks and `regression-v06694-rc1.chqscript`; Full Regression includes the new focused contract.
- Carried forward the v0.66.9.3 SHADOW VERIFIED independent sprite 4bpp decoder and placed sprite-source layer.
- Candidate baseline is exact promoted v0.66.9.3 SHA-256 `6da88889dc8214c6058644e44e4d29f6a2117c9f0e92d99c8d906c280fc3462e`; Windows exact-archive qualification is still required before promotion.

# v0.66.9.3 - Promoted research-capability release

- Promoted from exact Windows-qualified `v0.66.9.3-RC5` SHA-256 `19b7ed48edae98fd99f660ef56a949573050cabc2958fcdba2833a7888103961` after `chqdev.bat qualify --fresh` PASS: 8/8 CTest, deterministic package/exact verification, runtime readiness, focused run 001 PASS, Full Regression run 002 PASS and `promotable` PASS.
- Focused evidence bundle SHA-256 `e23091feaecba7b468b93f4b24ee8ae279b4eb16892cac54196b51527ba9e6c1` verified 17 files with zero errors; all eight canonical `sprite.layer-shadow.compare` checks reported zero pixel and owner mismatches.
- Full Regression bundle SHA-256 `80471a6cd5d5ea1b4431bc7ad5b68f1c39f0012656fa87a18ccfe7c3d33922cf` verified 2280 files with zero errors and restored the safe player-gameplay frame 2064 paused state.
- Promotes `sprite-placement-ownership` to **SHADOW VERIFIED** for the canonical placed-source corpus.
- Final release identity/documentation/focused-regression wiring plus a release-package metadata guard; no intentional emulation/gameplay change from RC5. Package status/source/features are now release-owned in `chqdev.json`, and preflight rejects stale candidate/pending-promotion manifest wording.
- Adds final focused regression `research/scripts/regression/regression-v06693.chqscript`; RC5 remains packaged as historical qualification provenance.
- Reconciles current documentation/knowledge/next-steps with RC3 Runs 003-020 and the successful RC5 prove-off, removing stale player-car, Brake, Turbo-HUD, road-state and sprite-shadow wording.

> Current release: **v0.66.9.3**.

# v0.66.9.3-RC5 - Qualification identity repair

- Carries the RC4 capability-promotion feature set unchanged.
- Fixes the first Windows RC4 qualification failure: `tests/cli_evidence_tests.ps1` no longer hard-codes stale build identity `0.66.9.2`; CMake injects the authoritative current release identity.
- Adds portable preflight guards for Windows CTest evidence identity wiring, CMake project-version alignment, focused-regression identity and machine-readable build identity.
- `chqdev` advances to v0.5.6.
- Candidate-time gate: Windows qualification was required before promotion. **Historical outcome:** exact RC5 SHA-256 `19b7ed48edae98fd99f660ef56a949573050cabc2958fcdba2833a7888103961` subsequently passed 8/8 CTest, deterministic package/verify, runtime readiness, focused run 001, Full Regression run 002 and `promotable`; it is the direct promotion provenance for final v0.66.9.3.

> Historical development candidate: **v0.66.9.3-RC5**; subsequently qualified and promoted to v0.66.9.3.

# v0.66.9.3-RC4 - RC4 capability promotion

- Independent placed sprite-layer shadow/compare from raw sprite RAM/spritemap/gfx/palette.
- Bounded memory read provenance (`read|write|rw`).
- Structured collision-response and sound-command-interface events.
- Corrected/promoted player, target, beacon, Turbo, Brake, score, road-side, incline/decline and timer knowledge from RC3 Runs 003-020.
- Alpha/occupancy-aware image compare, IOC sweep state capture, strict signed-12-bit steering validation, corrected P2:20 Brake semantic control.
- Historical outcome: RC4 itself was **not promoted** because its first Windows qualification exposed a stale CLI-evidence identity assertion. RC5 repaired that release-test contract without changing the RC4 capability set and then passed the complete qualification path.

> Historical development candidate: **v0.66.9.3-RC4**; superseded by qualified RC5 and final v0.66.9.3.

# ChaseHQ-Native changelog

## v0.66.9.3-RC3 - Tooling stabilization before diagnosis-heavy work (development candidate)

- Keeps final Windows-qualified v0.66.9.2 SHA-256 `f3c730e5bc41886189c11e23d4998967f205b085fa172a9c30fc812330227b26` as the immutable gameplay/emulation baseline; no intentional game-behaviour change.
- Closes the RC2 authoritative source-line gap end-to-end: browser and Go runners transport the resolved-source line to the server, including nested `for`/`repeat`/`if`/`try` constructs.
- Records client-side `wait` operations as first-class authoritative commands instead of counting them while omitting them from `console-output.txt` / `resolved-commands.txt`.
- Preserves both original authored `script.chqscript` and include-expanded `resolved-script.chqscript`, with per-command `resolved-command-map.jsonl` provenance.
- Makes evidence bundles cross-platform: manifest/index paths and ZIP entry names use `/`, generated authoritative metadata is UTF-8 without BOM, and bundle creation rejects backslash ZIP entries.
- Upgrades the standalone evidence index to `chq-run-bundle-index-v2` with clickable file links and path/category filtering.
- Corrects Full Regression growth accounting to recursive structural commands. Qualified v0.66.9.2 has a 936-command structural maximum (919 actually executed on Windows); RC3 has 746 structural commands / 603 top-level items, a ~20.3% structural reduction with the explicit capability-equivalence map retained.
- Adds HTTP-backed Go coverage for nested source-line transport and authoritative client-only wait evidence. RC1 and RC2 tooling probes remain packaged as historical reproduction scripts; RC3 is the current focused tooling regression.
- `chqdev` advances to v0.5.5. RC3 is deliberately intended to remain the working build through the next diagnosis phase; promotion qualification stays deferred until the tooling surface has survived real diagnostic use.

## v0.66.9.3-RC2 - Tooling hardening repair pass (development candidate)

- Keeps final Windows-qualified v0.66.9.2 SHA-256 `f3c730e5bc41886189c11e23d4998967f205b085fa172a9c30fc812330227b26` as the immutable gameplay/emulation baseline; no intentional game-behaviour change.
- Repairs the first real RC1 Windows Workbench failure: evidence-index byte totals no longer use `Measure-Object -Property bytes` on ordered dictionaries.
- Authoritative run finalization now fails closed: packaging/index/hash/ZIP errors produce `FINALIZATION_FAILED`, preserve the requested final status/error, invalidate run-history caches and remove incomplete generated bundle artifacts.
- Browser Script Console finalization now sends the authoritative executed command total, including client-side `wait` commands, matching Go-runner count semantics.
- Nested tuple/repeat/if/try compilation retains authored top-level source line numbers instead of renumbering sliced blocks from line 1.
- `corpus.run` now emits parsed `result` objects for JSON-producing nested actions and uses `raw` only for non-JSON output.
- Corrects stale script-schema/handbook wording that still described tuple iteration and explicit runtime queries as future work.
- Supersedes `regression-v06693-rc1.chqscript` with `regression-v06693-rc2.chqscript`; RC1 remains packaged as historical evidence. The RC2 focused run includes a client-side wait so browser finalization count parity is exercised by real Workbench use.
- `chqdev` advances to v0.5.4. Promotion qualification remains intentionally deferred until the tooling-hardening cycle is considered frozen.


## v0.66.9.3-RC1 - Research Tooling Hardening (development candidate)

- Consolidated Full Regression by capability rather than historical release lineage. RC1 initially reported **602 top-level compiled items** versus 919 executed commands in the qualified v0.66.9.2 run; RC3 later corrected that mixed metric to the true recursive structural comparison (**936 baseline -> 746 RC3**). Historical release scripts remain packaged/cataloged and an explicit coverage map records where every removed auto-run contract is retained. A Go budget regression prevents duplicate-growth returning.
- Converted historical Workbench static tests from stale release-identity/auto-inclusion assertions into version-agnostic carry-forward contract tests; substantive feature coverage remains active without forcing obsolete release scripts back into Full Regression.

- Baseline: exact final Windows-qualified v0.66.9.2 SHA-256 `f3c730e5bc41886189c11e23d4998967f205b085fa172a9c30fc812330227b26`.
- Qualification is intentionally deferred while this RC is used to push tooling gaps as far as practical; it is not yet promotable.
- Adds read-only native/HTTP/Script/Workbench sprite 4bpp shadow comparison using the independent decoder.
- Adds explicit execution-time `query.runtime`, bounded fixed-arity tuple loops, checkpoint `corpus.run`, and a configurable 2048-command default safety limit.
- Adds per-file SHA-256 + byte-length authoritative bundle manifests, generated JSON/HTML bundle indexes, verification reports, and direct Recent Script Runs -> Evidence viewing.
- Consolidates repeated historical release-tail Full Regression execution into a current capability contract while preserving all unique coverage; historical scripts remain available. See `docs/REGRESSION_COVERAGE_MAP.md`.
- No intentional emulator/game-behaviour changes.

# ChaseHQ-Native Changelog
This is the single canonical human-readable release history shipped at the project root. Historical per-version BUILD_NOTES, HOTFIX_NOTES and RELEASE_NOTES files were consolidated in v0.66.9.2-RC2. Current qualification policy lives in `VALIDATION.md`; package identity lives in `PACKAGE_MANIFEST.json` / `PACKAGE_MANIFEST.txt` when generated by `chqdev`.

## v0.66.9.2

- Promotes the Windows-qualified v0.66.9.2-RC4 feature set. Qualified RC4 archive SHA-256: `45f4538841a438e951d0a88a09a7a428eca17452675ada5be9619884c67b7d8d`.
- RC4 Windows `chqdev qualify --fresh` passed 7/7 CTest, deterministic package/exact verification, runtime readiness, focused run 001, Full Regression run 002 and final `promotable`.
- Post-qualification bundle inspection confirmed the RC4 evidence-integrity repair: complete authored `script.chqscript` and authoritative non-zero command counts are preserved.
- Final promotion changes release/config/documentation identity and adds `regression-v06692.chqscript`; no intentional emulation/gameplay behaviour change.
- Stage-1 track-record Runs 019-029 remain closed: +4 road processing, +5 palette transition, +6 environment transition, +7 polymorphic feature parameter; route-indicator semantics are only claimed for the proven 0xEB family.
- Release-root hygiene remains consolidated: one `CHANGELOG.md`, one current `VALIDATION.md`, generic manifests, no version-specific qualification wrappers.
- Next native milestone after exact-final qualification: whole-frame independent sprite 4bpp SHADOW/COMPARE, aiming to move `sprite-4bpp-decoder` from NATIVE-CANDIDATE to SHADOW VERIFIED.

## v0.66.9.2-RC4

- Supersedes RC3 despite RC3 completing Windows `chqdev qualify --fresh` with 7/7 CTest PASS, runtime readiness PASS, focused run 001 PASS, Full Regression run 002 PASS and final promotable PASS.
- Post-qualification inspection of those two authoritative bundles found a provenance defect: `script.chqscript` contained only the first executed command (focused: 1 line instead of the authored script; Full: only the BEGIN echo), while `run-metadata.json` did not persist the real command count and Recent Script Runs therefore showed 0 commands.
- chqdev v0.5.2 carries the complete authored source as `runSource` when creating the authoritative server run, while continuing to execute resolved commands one at a time for Go Script Console parity.
- Workbench run metadata now accumulates resolved command counts and finalization accepts the Go runner's authoritative total, so `run-metadata.json` and Recent Script Runs report the real count.
- Adds Go integration coverage proving full authored-source preservation and final command-count propagation, plus RC4 static regression and permanent Full Regression wiring.
- Carries RC3 frontend-bootstrap readiness, track semantics, evidence isolation, release-root hygiene and all gameplay/emulation behaviour unchanged.

## v0.66.9.2-RC3

- Supersedes RC2 after its fresh Windows qualification passed clean build/package and **7/7 CTest**, but failed the runtime-restart gate because the browser frontend bootstrap self-test did not report ready within the launcher timeout.
- RC2 support evidence showed the native API and Workbench server were already responsive and the browser was actively issuing successful API requests; no frontend exception was recorded. The ready acknowledgement was simply placed after heavyweight startup hydration (script library/history/dashboard refresh), making readiness timing-dependent.
- Frontend bootstrap is now split into two phases: a small fail-closed core self-test (DOM/script-runner validation + `/api/health` + nonzero script discovery) immediately posts `/api/v1/frontend-ready`; slower library/history/pairing/dashboard hydration continues afterwards.
- Hydration failures are still logged through the frontend fault channel and surface as DEGRADED, but they no longer create a false startup failure after core browser readiness has been proven.
- Adds a static regression that requires the ready acknowledgement to occur before `loadScriptLibrary`, authoritative run-history refresh and final dashboard hydration.
- Extends the launcher frontend-bootstrap safety deadline from 20 to 60 seconds so a cold Windows/browser start has headroom even though core readiness is now acknowledged before heavyweight hydration; regression coverage locks the 60-second contract.
- Carries RC2 track semantics, evidence isolation, release-root hygiene and canonical `\.\chqdev.bat qualify --fresh` workflow unchanged.

## v0.66.9.2-RC2

- Supersedes RC1 before Windows promotion; no RC1 qualification is carried forward.
- Removes version-specific `Qualify-*.bat` and legacy `tools/Invoke-RC*Qualification.ps1` wrappers. The only canonical fresh Windows promotion command is `.\chqdev.bat qualify --fresh`.
- Consolidates root-level BUILD/HOTFIX/RELEASE note history into this `CHANGELOG.md`.
- Removes historical version-specific validation files and historical package manifests from the release root. `VALIDATION.md` is the current contract; only the current generic package manifests are generated.
- Adds chqdev v0.5.1 release-root hygiene preflight so obsolete note/validation/manifest/wrapper files cannot creep back into packaged candidates.
- Corrects the v0.66.9.1 baseline archive SHA-256 to `36c40833f4a077fd0545e64b52585ef2c6ef6ed414ae31832092f5150caf6a4`.
- Carries the RC1 Stage-1 track-record semantics, evidence isolation, session metadata and checkpoint-context work unchanged.

## Historical release notes archive

The sections below preserve the former root-level release/build/hotfix notes. Source filenames are retained as headings for provenance.

<details>
<summary>RELEASE_NOTES_0.66.9.2-RC1.md</summary>

```text
# ChaseHQ-Native v0.66.9.2-RC1 release notes

Baseline: exact promoted v0.66.9.1 archive SHA-256 `36c40833f4a077fd0545e64b52585ef2c6ef6ed414ae31832092f5150caf6a4`.

This RC consolidates the completed Stage-1 track-record investigation and evidence/session correctness fixes. It does not intentionally alter authentic emulation/gameplay behaviour.

### Included

- Evidence-backed Track View/API names for record offsets +2 through +7, with deprecated legacy aliases retained.
- Full course-record identity separated from road-processing/material-candidate identity.
- Proven type-dependent `+7` route-indicator interpretation for the `0xEB` family without global overclaiming.
- Legacy-trace run isolation/freshness guard.
- Dynamic launcher session build identity.
- Diagnostic live-state/startup-phase separation and structured diagnostic-state metadata.
- Explicit attract-mode vs player-gameplay checkpoint context.
- Documentation, machine-readable knowledge, focused regression and Full Regression updates.

Windows promotion is pending. Run `chqdev qualify --fresh` on the exact frozen archive; zero tolerated CTest failures remain mandatory.
```

</details>

<details>
<summary>RELEASE_NOTES_0.66.9.1.md</summary>

```text
# ChaseHQ-Native v0.66.9.1

## Purpose

Stable release-process/documentation consolidation from promoted `0.66.9.0-RC3.4.9` SHA-256 `30ec017edebb9d221dfed16d090ffd6678b645cff6df8c5445a9ef298efe0ebf`. No intentional emulation/gameplay behavior change.

## Changes

- Deep audit of current human docs, quick-start/local setup, platform operational notes, handbook and machine-readable documentation knowledge.
- Permanent `docs/RELEASE_GOVERNANCE.md`.
- chqdev v0.5.0 preflight release-process contract and legacy-canonical-claim rejection.
- Canonical fresh Windows promotion remains `chqdev qualify --fresh`.
- Zero tolerated CTest failures; any CTest failure is fatal.
- Deterministic exact-archive identity, focused regression, Full Regression and `promotable` PASS remain mandatory.
- PowerShell/BAT release tooling remains compatibility/fallback only.

## Research continuation

After promotion, resume Stage-1 road-material causal reconstruction.
```

</details>

<details>
<summary>RELEASE_NOTES_0.66.9.0-RC3.4.md</summary>

```text
# ChaseHQ-Native v0.66.9.0-RC3.4

RC3.4 closes two previously agreed mobile-companion gaps on the definitive RC3.3 baseline.

- Adds a desktop Workbench Mobile Console Pairing panel with a fully offline QR encoder.
- Adds local-only `/api/v1/mobile/pairing`; the session token is never sent to an external QR service and is not exposed by the pairing descriptor to remote clients.
- Remote QR entry captures the existing random token into HttpOnly `chq_token` and uses a history-replacing transition to clean `/console`.
- Adds mobile Recent Script Runs/history using the same authoritative `/api/v1/script/runs` source as desktop, with current/all-session scope, status filtering, paging/load-more, build/session identity and selected-run console output.
- Adds RC3.4 static/QR regression, focused `.chqscript` regression and RC3.4 qualification launcher while retaining all RC3.3 provenance/fail-closed rules.

No gameplay/emulation behavior is intentionally changed.
```

</details>

<details>
<summary>RELEASE_NOTES_0.66.9.0-RC3.4.9.md</summary>

```text
# ChaseHQ-Native v0.66.9.0-RC3.4.9

RC3.4.9 is a narrow release-engineering cleanup derived from the exact promoted RC3.4.8 archive (`ea24ce76ba1555e23b997a0e925ba85a9c489da6a569c04a8e374df661395990`).

## Fixes

- Fixes the historical Windows `cpu_bus_rom_tests` SEGFAULT by moving sixteen large `chq::Runtime` test fixtures from stack storage to heap-backed `std::unique_ptr`.
- Fixes the subsequently exposed Windows file-in-use failures by explicitly closing six `tf`, one `pf` and one `cf` input stream before `std::filesystem::remove/remove_all`.
- Removes `knownCTestFailure` from `chqdev.json` and the Go config model.
- Removes the special-case PASS path from `chqdev test`; any CTest failure is now fatal.
- Makes compatibility `Build-Debug.bat` and `Build-Release.bat` fail on any CTest failure as well.
- Adds RC3.4.9 static/release regression guards and permanent Full Regression wiring.

## Release gate

The canonical Windows prove-off remains `chqdev qualify --fresh`. RC3.4.9 promotion requires CTest **7/7 PASS**, deterministic package verification, runtime readiness, focused regression PASS, Full Regression PASS and final promotable PASS.
```

</details>

<details>
<summary>RELEASE_NOTES_0.66.9.0-RC3.4.8.md</summary>

```text
# ChaseHQ-Native v0.66.9.0-RC3.4.8 Release Notes

## Scope

RC3.4.8 is a chqdev runner-parity hardening release built from the exact RC3.4.7 candidate SHA `991cd12e8ac7f05d394017cc6b6d1dd968d4c678f79b36d925ae6ec5aebf085c`. It does not intentionally change Chase H.Q. emulation/gameplay behavior.

## RC3.4.7 Windows finding

`chqdev qualify --fresh` proved trust, clean build, CTest policy, deterministic packaging, exact SHA reproduction and runtime readiness on Windows, then failed the focused regression. The same focused regression and Full Regression both passed when run through Workbench. The failed authoritative run bundle showed execution stopped after `checkpoint.load` and before `echo RC347_PARAMETER_FRAMES=${frames}`.

Root cause: the Go regression runner parsed `# param ... default=4` metadata but called query resolution and script compilation with empty/default-less variable state. `${frames}` therefore remained unresolved. This was a chqdev parity defect, not an emulator or regression-script failure.

## chqdev v0.4.1

- Seeds Go-runner query resolution and compilation from `scriptDefaultVars(source)`.
- Preserves parameter defaults through include/query/control-flow compilation.
- Adds an HTTP-backed Go integration regression proving a parameterized script emits `echo frames=4` and `api control.run-frames frames=4`, with no `${frames}` leakage.
- Upgrades regression runner identity to `go-script-console-v2`.
- Regression failures now expose the exact nested error, session ID, run ID, run path and expected bundle path.
- `qualify --fresh` propagates the nested regression failure detail instead of reducing it to `focused regression failed` or `Full Regression failed`.
- Adds RC3.4.8 focused regression and permanent Full Regression wiring.

## Process policy

`chqdev qualify --fresh` remains the de facto and authoritative fresh-build/release prove-off. Script Console parity defects must be fixed in chqdev and regression-locked; release scripts must not be weakened to accommodate controller bugs.
```

</details>

<details>
<summary>RELEASE_NOTES_0.66.9.0-RC3.4.7.md</summary>

```text
# ChaseHQ-Native v0.66.9.0-RC3.4.7 Release Notes

## Scope

RC3.4.7 is an infrastructure hardening release built from the Windows-proven RC3.4.6 candidate. It does not intentionally change Chase H.Q. emulation/gameplay behavior.

## chqdev v0.4

- Adds `chqdev qualify --fresh` as the canonical one-command fresh Windows qualification sequence.
- Adds a Go-side Research Script v2 release runner for Full Regression orchestration.
- Recursively expands packaged `include` files locally.
- Supports release-script `set`, `${var}`, `for`, `repeat`, `if/else/end`, `try/finally`, query helpers and predicates while ordinary commands remain authoritative Workbench `/api/v1/script` commands.
- Preserves a single authoritative run ID across command-by-command execution and finalizes PASS/FAIL explicitly.
- Fixes the RC3.4.6 Full Regression orchestration defect where posting the whole script to the reduced server parser failed with `Unexpected end at line 59`.
- `qualify --fresh` cleans generated `out/` and `dist/`, proves source/build/test/package, restarts/checks runtime on Windows, then runs focused and Full Regression before recording `promotable=PASS`.

## Process policy

`chqdev qualify --fresh` is the de facto fresh-build/release prove-off entry point going forward. Individual commands remain diagnostic surfaces. New release orchestration logic belongs in chqdev first; PowerShell/BAT release qualification remains compatibility/fallback only.
```

</details>

<details>
<summary>RELEASE_NOTES_0.66.9.0-RC3.4.6.md</summary>

```text
# ChaseHQ-Native v0.66.9.0-RC3.4.6

RC3.4.6 is a release-process solidification build. It carries the RC3.4 feature work and RC3.4.1-RC3.4.5 release guards forward unchanged while closing the remaining Windows trust/runtime-launch gap.

## chqdev v0.3

- Adds native Windows `Zone.Identifier`/Mark-of-the-Web scanning and removal.
- `chqdev preflight` automatically repairs project-owned trust state before source validation and fails if any marked files remain.
- Adds explicit `chqdev trust` and diagnostic `chqdev trust --check`.
- Adds `chqdev runtime start|stop|restart` as the normal lifecycle interface.
- Start/restart runs a readiness check after the Workbench host is launched.
- The Workbench host itself remains PowerShell-backed in RC3.4.6; chqdev invokes it internally with `-NoProfile -ExecutionPolicy Bypass` after trust normalization. This is a compatibility bridge behind the stable chqdev interface.
- Manual `Unblock-File` is no longer part of the supported workflow.

## Release discipline

`chqdev` remains the de facto/authoritative build-release controller. New release/runtime-controller logic belongs in `tools/chqdev`, not solely in PowerShell/BAT. Deterministic packaging, exact archive verification, Windows-script ASCII/U+00B7, LF/CRLF/CR, handover, exact script-catalog and current-document guards remain mandatory.

## Regression

`research/scripts/regression/regression-v06690-rc346.chqscript` is permanently included in Full Regression.
```

</details>

<details>
<summary>RELEASE_NOTES_0.66.9.0-RC3.4.5.md</summary>

```text
# ChaseHQ-Native v0.66.9.0-RC3.4.5

RC3.4.5 promotes `chqdev` v0.2 from a parity experiment to the **de facto and authoritative build/release process**.

The release keeps the RC3.4 QR/mobile-history functionality and RC3.4.1-RC3.4.4 release safeguards unchanged. New infrastructure: deterministic/reproducible ZIP packaging, SHA sidecars, runtime readiness checks, focused/Full Regression orchestration over the Workbench HTTP API, and persistent qualification state.

RC3.4.4 proved chqdev's Windows preflight/build/test/package/verify path. It also exposed non-deterministic ZIP metadata because Linux and Windows created different SHA-256 archives from the same authoritative source. RC3.4.5 fixes that in the controller itself and makes cross-host SHA parity a required proof.

Legacy PowerShell/BAT release scripts remain compatibility/fallback only. New release behavior must be implemented and tested in chqdev.
```

</details>

<details>
<summary>RELEASE_NOTES_0.66.9.0-RC3.4.4.md</summary>

```text
# ChaseHQ-Native v0.66.9.0-RC3.4.4

RC3.4.4 pauses substantive game research to introduce **chqdev v0.1**, the first cross-platform ChaseHQ-Native build/release controller.

`chqdev` is written in Go using only the standard library and is bundled for Windows/Linux amd64/arm64. It provides native doctor/preflight/build/test/package/verify/qualify/status/version commands, plus structured `--json` output.

Portable preflight reimplements the permanent version, JSON/BOM, Windows-script ASCII, LF/CRLF/CR handover, exact script-catalog, current-document and backup-file gates without PowerShell. Windows build orchestration can stage local ROM/SDL sources, call CMake directly and perform the launcher argument smoke. Linux v0.1 is CPU-only. Packaging creates a manifest-owned ZIP and independently re-extracts/verifies it.

RC3.4.4 is a **parity release**. It does not remove PowerShell/BAT qualification yet; the exact archive must pass chqdev and the existing Windows qualification/runtime/regression path before promotion. The RC3.4 QR/mobile-history functionality and all RC3.4.1-RC3.4.3 release guards are carried forward unchanged.
```

</details>

<details>
<summary>RELEASE_NOTES_0.66.9.0-RC3.4.3.md</summary>

```text
# ChaseHQ-Native v0.66.9.0-RC3.4.3

RC3.4.3 is a release-hardening hotfix over the locally Windows-proven RC3.4.2 working tree. It exists specifically to end the repeated handover-validator portability loop without changing gameplay/emulation or the RC3.4 QR/mobile-history feature behavior.

## Fixes and permanent prevention

- Incorporates the locally proven RC3.4.2 repair: handover contract text is normalized from CRLF/CR to LF before line-oriented regex matching.
- Adds reusable `Normalize-ReleaseContractText` and requires `Test-ChatGptHandoverContract` to consume normalized text.
- The PowerShell source audit now positively proves **LF, CRLF and CR** valid fixtures, while still rejecting missing-continuation, missing-next-task and stale-version fixtures.
- `tests/workbench_rc342_tests.js` carries the proven CRLF repair forward and is extended to CR-only input.
- Adds independent `tests/workbench_rc343_tests.js` to prove the newline matrix, actual frozen handover, current qualification wiring, focused regression and current-documentation matrix.
- `Validate-WindowsPreflight.ps1` adds a named `RC3.4.3 text-contract newline invariance` gate.
- Current authoritative documentation is version-locked to v0.66.9.0-RC3.4.3; release-guard docs explicitly state the LF/CRLF/CR rule so future sessions do not rediscover it.
- RC3.4.1 ASCII/U+00B7 fail-closed checks, `[EXPECTED FAIL]` negative-fixture UX, full child-output diagnostics, immutable-archive qualification and exact catalog parity remain mandatory.
- Adds canonical RC3.4.3 qualifier and focused release regression, permanently included by Full Regression.

## Non-negotiable future rule

Any new or changed text-contract validator that uses line anchors MUST normalize CRLF and CR to LF and MUST ship positive LF/CRLF/CR tests in both the Windows PowerShell source audit and an independent portable regression before a candidate ZIP is handed off.
```

</details>

<details>
<summary>RELEASE_NOTES_0.66.9.0-RC3.4.2.md</summary>

```text
# ChaseHQ-Native v0.66.9.0-RC3.4.2

RC3.4.2 is a release-contract hardening hotfix over the locally Windows-proven RC3.4.1 source. It carries the RC3.4 QR pairing/mobile authoritative history feature set and RC3.4.1 ASCII/qualification diagnostics unchanged.

## Fixes

- Incorporates the locally proven RC3.4.1 handover repair: `docs/CHATGPT_HANDOVER_PROMPT.md` contains the current build marker plus explicit `## Authoritative continuation` and `## Immediate next task` sections.
- Replaces the handover's brittle one-off string check with generic `Test-ChatGptHandoverContract`, driven by the authoritative version in `src/version.h`.
- Adds a positive/negative validator self-test covering a valid handover, missing continuation section, missing next-task section and stale build version. Any failure is release-blocking.
- Adds `tests/workbench_rc342_tests.js`, an independent portable regression that checks the actual frozen handover plus independent good/bad fixtures.
- Makes RC3.4.2 handover-contract wiring a named `Validate-Release.ps1` release gate.
- Carries forward the RC3.4.1 all-Windows-script ASCII fail-closed policy, U+00B7 negative fixture, `[EXPECTED FAIL]` qualification UX and complete child stdout/stderr diagnostics.
- Adds canonical RC3.4.2 qualifier and focused release regression, permanently included by Full Regression.
- Tightens script-catalog validation to exact 1:1 Markdown table parity; prose mentions no longer satisfy coverage. Three historically packaged scripts that were only mentioned in prose now have real catalog rows.

No gameplay/emulation or intended QR/mobile-history behavior is changed.
```

</details>

<details>
<summary>RELEASE_NOTES_0.66.9.0-RC3.4.1.md</summary>

```text
# ChaseHQ-Native v0.66.9.0-RC3.4.1

RC3.4.1 is a release-hardening hotfix over RC3.4. It carries the QR pairing and mobile authoritative history feature set unchanged.

- Incorporates the locally proven RC3.4 repair: the seven U+00B7 middle-dot characters accidentally embedded in `Start-ChaseHQWeb.ps1` are replaced with ASCII separators.
- Expands the Windows PowerShell/source-code-page gate from a Workbench-specific check to every project-owned Windows script surface (`.ps1`, `.psm1`, `.psd1`, `.bat`, `.cmd`), allowing only an optional UTF-8 BOM before otherwise ASCII content.
- Non-ASCII failures identify file, line, column and Unicode code point.
- The Windows preflight creates an intentional U+00B7 fixture on every run and proves the policy rejects it.
- `tests/workbench_rc341_tests.js` independently scans the final source tree for the same invariant so a candidate cannot rely on a single validator implementation.
- The qualification negative child-gate fixture now renders `[EXPECTED FAIL]`; ordinary `[FAIL]` is reserved for real release failures.
- Child gate execution captures complete stdout/stderr and exit status so native stderr cannot collapse an actionable validation failure into a generic `NativeCommandError`.
- Adds RC3.4.1 canonical qualifier and focused release regression, permanently included by Full Regression.

No gameplay/emulation or intended QR/mobile-history behavior is changed.
```

</details>

<details>
<summary>RELEASE_NOTES_0.66.9.0-RC3.3.md</summary>

```text
# ChaseHQ-Native v0.66.9.0-RC3.3

RC3.3 is the definitive release-engineering hardening release built from the final locally patched and internally verified RC3.2 source surface. It deliberately avoids unrelated emulator changes.

## What changed

The release lifecycle now has one authoritative Windows entry point, `Qualify-RC3.3.bat`, backed by `tools/Invoke-RC33Qualification.ps1`. The BAT launcher deliberately uses `-ExecutionPolicy Bypass` so downloaded/extracted source cannot prompt for every nested project script. It fails closed when a child gate fails or reports `READY TO PACKAGE: NO`, packages only the manifest-owned surface, independently extracts and hashes the exact archive, and records candidate identity/gates in machine-readable `qualification.json`. A negative self-test proves the orchestration layer cannot convert a child failure into a parent PASS.

`regression-v06690-rc33.chqscript` is the permanent version-specific whole-release runtime regression and is included in Full Regression.

## Promotion rule

A source ZIP is not a proven Windows release. Promotion requires the same exact immutable ZIP to pass clean `Build-Debug.bat`, launcher/runtime READY smoke, RC3.3 focused regression, authenticated second-client `/console` proof, and Full Regression. Repacking or editing after qualification creates a different candidate and invalidates prior promotion evidence.

## RC3.3 qualification-controller corrections

The definitive candidate regression-guards relative `-ProjectRoot .` handling, resets child `$LASTEXITCODE`, requires the RC3.3 regression to be present in the exact script catalog, and returns a clean PENDING/HANDOFF state after automated Windows build success instead of throwing merely because interactive runtime/LAN gates remain outstanding.

## Windows source-tree proof before definitive packaging

On 2026-10-03 the locally corrected RC3.3 source tree passed the real Windows `Build-Debug.bat` path: aggregate source/preflight validation reached `READY TO PACKAGE: YES`, local ROM staging validated 19/19, MSVC/SDL Debug configure and compilation passed, the real research-launcher argument/process smoke passed, and `cli_evidence_tests` passed. CTest had only the explicitly tolerated historical `cpu_bus_rom_tests (SEGFAULT)` failure, so the wrapper ended `BUILD SUCCESSFUL WITH KNOWN TEST ISSUE`.

The Windows-proven handoff ZIP used as the definitive packaging source has SHA-256 `57675ea1c8c8ca84e5963c8547da30454bf4c89f907552b2a8d4c1f18f53ea65`. Before repackaging its previous RC3.3 manifest had exactly one mismatch: the intentionally repaired `Validate-Release.ps1` CLI-evidence identity literal. RC3.3 now permanently checks that the validator's expected CLI build identity matches the authoritative native identity from `src/version.h`, so this stale-version failure class cannot rely on manual string synchronization alone.

This Windows source-tree proof does **not** promote the definitive archive. Exact-archive build/runtime/LAN/focused/Full Regression evidence must still belong to the same final immutable ZIP SHA.
```

</details>

<details>
<summary>RELEASE_NOTES_0.66.9.0-RC3.2.md</summary>

```text
# ChaseHQ-Native v0.66.9.0-RC3.2

RC3.2 closes the tooling gaps uncovered during the Stage-1 road-material investigation and makes long reusable investigations easier to drive from both desktop and mobile.

## Added

- Additive `.chqscript` runtime parameters using metadata such as `# param frames type=select values=300,600,900,1200,2400,3600 default=1200`.
- Workbench parameter controls and resolved-command preview; parameter values seed ordinary `${name}` substitution.
- Research API action `script.run file=... key=value` for parameterized library execution.
- Run metadata records resolved parameter values and resolved command lines.
- Run-local `legacytrace` artifact collection with source path, size and SHA-256 provenance.
- Run-local checkpoint evidence: saves copy `.chqstate` + metadata into `artifacts/checkpoints`; loads append requested/resolved path + frame evidence.
- Default one-command LAN/mobile Workbench exposure with automatic LAN IPv4 discovery, per-session token generation, one-time HTTP URL-ACL + Private-profile firewall setup, ready-to-use LAN and `/console` URLs, and explicit `-LocalOnly` opt-out. Native debugger/private backend remain loopback-only.
- Stage-1 type-6 road-material causal-path documentation while preserving the semantic boundary: observed dirt-like behaviour is not promoted to a global material enum.

## Compatibility

Legacy scripts without `# param` declarations execute with the existing variable/loop semantics. Existing dedicated 600/1200/3600 survey scripts are retained; the new parameterized survey is additive.

## Promotion status

Linux no-SDL compilation/CTest and portable static tests are release-package gates here. Windows PowerShell parsing, SDL runtime, Workbench live execution and Full Regression remain required on the exact archive before RC3.2 can replace the last fully Windows/SDL-proven baseline.

## Checkpoint packaging note

The archive carries the eight canonical checkpoint binaries inherited from RC3.1. The newer October 2 `stage1-page4-material-pre-4704` and `stage1-selector5-page0-8664` states are not included because their real raw bytes were not available to this packaging environment; no placeholders are fabricated. See `docs/RC3.2_CHECKPOINT_IMPORT_STATUS.md`.

## Corrected replacement package

The first RC3.2 archive exposed a Windows preparation-gate defect before compilation: `Prepare-Project.ps1` enables strict mode and invokes `Validate-Workbench.ps1`, whose same-frame recorder check accidentally used a double-quoted search literal containing `$prev`. PowerShell therefore tried to read an undefined validator variable instead of searching for the literal source text. The corrected package uses a non-interpolating literal and adds a release guard for this regression. **All earlier RC3.2 archive hashes are withdrawn.**

## Corrected Windows preflight

The final RC3.2 candidate hardens the Windows package gate after two exact-archive preparation failures were found before compilation. `Start-ChaseHQWeb.ps1` is again ASCII-safe for Windows PowerShell 5.1, the strict-mode `$prev` validator literal is non-interpolating, and a new aggregate `Validate-WindowsPreflight.ps1` reports parser/encoding/automatic-variable/build-gate/version/RC3.2 plumbing problems together before the deeper Workbench validator runs. The earlier RC3.2 hashes `aee39d...` and `c1636f...` are withdrawn.

## Windows preflight consolidation correction

The aggregate preflight initially overreached by treating legal `$null = ...` assignments as protected-variable writes. The corrected RC3.2 package uses PowerShell's runtime variable `ScopedItemOptions` and blocks only actual `Constant`/`ReadOnly` collisions. `$null` sinks are explicitly accepted and the rule self-tests both accepted and rejected names. The duplicate rule was removed from the Workbench validator, the release validator now executes the same preflight, and source validation runs before dependency staging.
Previous RC3.2 aggregate-preflight candidate SHA-256 `58aafb2f6c70e772853e543b81683e8d0b8f2b7f8c2a3dbc29f4d588d773ebf6`: WITHDRAWN after the `$null` false-positive was proven on Windows.

## Windows launcher argument-vector correction

A subsequent exact-archive Windows launch exposed a launcher handoff defect after a successful build: `Start-ChaseHQResearch.ps1` constructed and printed the authoritative `$nativeLaunchArgs` vector, but built the `Start-Process -ArgumentList` string from the optional `$EmulatorArgs` parameter instead. With the normal one-command launcher that parameter is empty, so Windows PowerShell rejected a null/empty `ArgumentList` before the emulator started. The corrected package derives `$argLine` from `$nativeLaunchArgs`, and the Windows preflight, Workbench validator, release validator, RC3.1 carried-forward test and RC3.2 static regression all assert the executed-argument contract. All earlier RC3.2 archive hashes are withdrawn.
## LAN/mobile default and release-process correction

Windows live testing proved that the original RC3.2 LAN path attempted to bind a non-loopback `HttpListener` without provisioning the required HTTP.sys URL reservation, producing `HttpListenerException: Access is denied`. The corrected candidate makes LAN/mobile the normal `Start-ChaseHQ.ps1` mode, automatically resolves the LAN IPv4 address, generates the token, performs one-time elevated URL-ACL + Private-profile firewall setup when required, prints usable desktop/mobile URLs, and runs an authenticated host-side LAN `/api/health` + `/console` smoke before reporting READY. `-LocalOnly` is the explicit opt-out.

RC3.2 now also ships `regression-v06690-rc32.chqscript` as the obvious whole-release focused regression. Release validation requires that script, Full Regression wiring, the LAN prerequisite helper and default LAN launcher contract. Narrow `regression-v06690-rc32-script-parameters.chqscript` remains available as a feature regression but no longer stands in for a release regression.


RC3.2 LAN ACL correction: the elevated prerequisite phase must never use an elevated HttpListener bind as evidence that the normal user owns the URL namespace. It now ensures and verifies the exact `http://<resolved-lan-ip>:37680/` URL ACL for the original user, then the parent non-elevated process performs the final bind proof.

## Final regression-fixture self-containment correction

Final Windows prove-off exposed one stale permanent-regression assumption: the RC2.5 script-safety test loaded `car-shadow-layer-investigation`, a research timeline that is not guaranteed to exist in a clean release session. The regression now records its own minimal deterministic timeline fixture, then proves the same load/seek/fork/finally-reset behavior against live frame 2064. Release validation now rejects literal named-timeline dependencies that are not created by the same regression script.

This source change supersedes the pre-correction exact candidate SHA-256 `f3f2816ea9a22609e1fb77e4ace758c534311b34b6601efb38042fe467959601`.
```

</details>

<details>
<summary>RELEASE_NOTES_0.66.9.0-RC3.1.md</summary>

```text
# ChaseHQ-Native v0.66.9.0-RC3.1

## Course Mapping Survey Quality / Object Tracking

RC3.1 is a focused refinement of RC3.0 before committing to long Stage-1 course surveys. The final RC3.0 focused Windows regression proved the live course-follow/survey backend and visible Track View. That short run also exposed two evidence-quality problems worth fixing first: volatile TC0150 scanline state fragmented one logical opening road section into multiple apparent surfaces, and the raw visible-sprite census produced many rows without conservative temporal candidate tracking.

### Carried-forward RC3.0 fixes

These are release invariants in RC3.1:

- `Set-CourseFollowConfig($config)` / `$config.ContainsKey($key)`; the broken PowerShell automatic `$args` form is rejected by validation.
- Track View routes to `Course Mapping / Live Survey`; the historical `Live SVG Track View` route is rejected by validation.
- Pause/Break remains the primary SDL pause/resume shortcut and Ctrl+P remains the secondary fallback for compact keyboards. Ctrl+P was manually proven on Windows during RC3.0 prove-off.
- Live `course.follow.*`, `course.survey.*`, Track Recorder and mapping export remain available from Workbench/API/Script Console without restarting the emulator.

### Track Recorder v3

- `surfaceStableSignature` is now a conservative course-record candidate key and is used for surface candidate counts/segments.
- Full representative TC0150 renderer state remains in `roadRenderSignature` as raw evidence but no longer fragments stable surface counts.
- `surfaceRenderCandidateSignature` retains a conservative masked TC0150 candidate signature separately from the raw words.
- Recorder status now reports stable surface candidates, contiguous surface segments, raw renderer variants, raw sprite rows, object-candidate rows and object-candidate tracks.
- Road geometry and player driven trajectory remain distinct outputs.

### Heuristic object candidate tracking

- Every visible sprite row is still preserved as `UNCLASSIFIED_DYNAMIC_OBJECT`.
- Road-relevant visible sprite observations become separate candidate rows rather than being merged merely because their rectangles touch. The RC3.0 sample showed repeated perspective-scaled scenery can form dense visual chains.
- Candidate rows receive temporary temporal IDs across adjacent samples only when map + palette identity and plausible nearest screen position support continuity.
- Candidate tracks are explicitly `UNCLASSIFIED_OBJECT_CANDIDATE` / `HEURISTIC_SCREENSPACE`; they are not semantic AI-car or obstacle proof and are not authoritative world/course coordinates.
- The proven target record is emitted as `CONFIRMED_TARGET_CAR` only under explicit Stage-1 checkpoint context; otherwise it remains `UNCLASSIFIED_TARGET_RECORD` so later-stage surveys do not inherit Stage-1 semantics automatically.

### Mapping export v2

`track.map.export` now adds:

- `track-map-summary.json`
- `track-object-candidates.csv`
- `track-surface-segments.csv`

while retaining:

- `track-map.json`
- `track-samples.csv`
- `car-trajectory.csv`
- `track-entities.csv`
- `track-surfaces.csv`
- `track-map.svg`

The master JSON schema is now `chq-course-map-v2`; export response schema is `chq-course-map-export-v2`.

### Survey scripts

Updated/current scripts include the 600-frame shakedown, 3600-frame Stage-1 centre survey, signed-bias structural branch surveys, generic current-state survey, and new `current-state-material-object-survey-1200.chqscript` for later-stage/current-checkpoint reconnaissance.

### Regression and validation

New permanent focused regression:

`research/scripts/regression/regression-v06690-rc31-course-mapping-quality.chqscript`

It is included in Full Regression after the RC3.0 live-survey regression. Static validation additionally guards the stable-surface separation and all final RC3.0 fixes.

RC3.1 is not Windows/SDL-proven until the exact packaged candidate passes Build-Debug, the focused RC3.1 regression, visual/data inspection of the 600-frame shakedown, and Full Regression.


### Portable prehandoff validation

Before packaging, all 44 pre-manifest source JSON files parse; the Workbench history, SDL-controls and RC3.1 course-mapping static Node tests pass; the Linux no-SDL build succeeds; and Linux CTest is 4/4 PASS. Windows/SDL and PowerShell execution remain explicit promotion gates.

### Corrected RC3.1 candidate after first Windows focused-regression attempt

The first RC3.1 package is withdrawn. Windows startup itself succeeded after the prior duplicate-hashtable-key correction, but the focused RC3.1 mapping-quality regression exposed a PowerShell automatic-variable collision on the second `track.record.sample`: `Assign-TrackObjectIds` assigned a previous candidate ID to `$pid`. PowerShell variable names are case-insensitive and `$PID` is a built-in read-only automatic variable, so the assignment failed with `Cannot overwrite variable PID because it is read-only or constant.`

The corrected candidate renames that local to `$previousTrackId`. `Validate-Workbench.ps1` and `tests/workbench_course_mapping_tests.js` now explicitly reject any `$pid = ...` assignment in the Workbench source, because ordinary PowerShell AST parsing validates syntax but does not diagnose writes to read-only automatic variables. All previously corrected RC3.0/RC3.1 invariants remain mandatory and are carried forward unchanged.

### Corrected RC3.1 candidate after object-candidate enrichment proof

The first fully running RC3.1 focused regression completed and proved the stable-surface split, but inspection of its exported `track-object-candidates.csv` exposed a data-quality defect: 131 raw visible-sprite rows collapsed to only three candidate rows, and those rows retained only `trackId` / tracking metadata while map, palette, bounds and screen-position evidence were null.

The corrected candidate now captures `New-TrackSpriteCandidates` output into an explicit array before temporal ID assignment, passes that array through a dedicated `Get-TrackObjectCandidates` helper, and builds tracked candidate objects with explicit evidence fields rather than reflective property copying. `track.map.export` now enforces a runtime coverage invariant: under the conservative one-eligible-sprite -> one-candidate policy, candidate-row count must equal the count of eligible visible sprite rows, and every exported candidate must retain map/palette/bounds/position/pixel evidence. Recorder/export summaries expose `eligibleObjectCandidateSourceRowCount` and `objectCandidateCoverage`.

A new Windows PowerShell regression (`tests/workbench_object_candidate_tests.ps1`) isolates the actual packaged helper functions from `Start-ChaseHQWeb.ps1`, feeds mock sprite observations through two samples, and verifies array preservation, evidence fields and temporal ID reuse.

PowerShell safety is also broadened beyond the previously found `$args` and `$PID` defects. `Validate-Workbench.ps1` now AST-scans every packaged `.ps1` and rejects unsafe case-insensitive assignments/parameters that collide with PowerShell automatic/read-only/reserved variables. Existing accidental locals named `$args`, `$Matches`/`$matches`, and `$PROFILE`/`$profile` were renamed to descriptive non-reserved names. Preference variables remain legal for intentional assignments such as `$ErrorActionPreference='Stop'`, but may not be repurposed as parameters.

### Consolidated locally proven fixes before long course survey

The locally patched Windows RC3.1 prove-off exposed and resolved several Workbench/data-quality issues before the 3600-frame survey. The consolidated candidate now carries all of them together:

- `track.map.export` uses `surfaceSegmentsCsv` for the CSV path while `surfaceSegments` remains the numeric segment count, avoiding the PowerShell duplicate-hashtable-key parser failure.
- `Prop()` now reads `System.Collections.IDictionary` / ordered-dictionary rows case-insensitively before falling back to normal PSObject properties. This matches the production visible-sprite representation and prevents vacuous zero-candidate coverage.
- object-candidate regression inputs now use the production ordered-dictionary shape as well as preserving map/palette/bounds/centre/pixel evidence and temporal IDs.
- Track Recorder sampling is idempotent for a repeated emulator frame: a same-frame request returns the existing last sample with `duplicateFrameSuppressed=true` instead of mutating reconstructed X/Y or object tracking.
- entering Track View while a Script Console run is active no longer injects an extra recorder sample.
- `Start-ChaseHQResearch.ps1` now uses a distinct `$nativeLaunchArgs` local instead of case-insensitively aliasing the `EmulatorArgs` parameter as `$emulatorArgs`; this removes the duplicated ROM/API/log/checkpoint argument block observed in launcher output.

Local Windows evidence before repackaging: `Build-Debug.bat` passed with only the documented historical `cpu_bus_rom_tests (SEGFAULT)` exception; the focused RC3.1 regression passed with 131/131 eligible object candidates and 1.0 non-vacuous coverage; and the 600-frame shakedown passed with exactly 41 unique 15-frame samples, full object-candidate coverage, all recorded samples ON_ROAD, and stable course-surface segmentation separated from raw TC0150 render variants. The exact rebuilt archive still requires its normal final package validation/promotion gates.


### Corrected RC3.1 candidate after first 3600-frame survey attempt

The first long Stage-1 centre survey on the locally patched Windows RC3.1 tree ran from frame 2064 to 3399 and retained 89 unique 15-frame samples before failing in Workbench track-state arithmetic. All 89 recorded samples remained `ON_ROAD`; the run crossed the first observed topology transition from page 0 to page 1, retained full object-candidate coverage (6986/6986), and preserved four stable course-record surface signatures separately from 89 raw renderer variants.

The failure was deterministic: `Get-LiveTrackState` used direct `[int16]` casts on masked 16-bit deltas. At frame 3399 a legitimate negative lateral error was represented as unsigned `0xFEF7` / decimal 65271, and Windows PowerShell rejected `[int16]65271` instead of reinterpreting it as -265. RC3.1 now routes both road-width delta and lateral-error conversion through the existing `Signed16` helper, which performs explicit two's-complement reinterpretation. A new `tests/workbench_track_signed16_tests.ps1` regression checks boundary values including `0x8000`, `0xFEF7` and `0xFFFF`, and `Validate-Workbench.ps1` rejects masked direct `[int16]` reinterpret casts.
```

</details>

<details>
<summary>RELEASE_NOTES_0.66.9.0-RC3.0.md</summary>

```text
# ChaseHQ-Native v0.66.9.0-RC3.0 release candidate

RC3.0 is a meaningful Course Mapping / Live Survey tooling release built from the Windows/SDL-proven RC2.9 baseline. It intentionally precedes long course surveys.

## Added

- Live native course-follower runtime command and Workbench/API/Script Console parity: status/configure/start/stop/reset.
- Signed `course-follow-lateral-bias` startup/config/live option for controlled branch surveys.
- Reversible `course.survey.start/stop` orchestration with timer hold, proven collision-response suppression and state restoration.
- Track Recorder v2 with road edges/centre, authoritative lateral state, separate projected player driven line, speed/steering/follower telemetry, confirmed Stage-1 target state and unclassified visible-sprite census.
- Raw surface-signature evidence from course-record selectors plus representative TC0150ROD renderer control/body/gfx state, intentionally not named asphalt/dirt until proven.
- Reconstructable mapping export: `track-map.json`, `track-samples.csv`, `car-trajectory.csv`, `track-entities.csv`, `track-surfaces.csv`, `track-map.svg`.
- Updated Track View with live follower controls, survey start/stop, road/player overlays and map export.
- New survey scripts: 600-frame shakedown, 3600-frame centre survey, negative/positive-bias branch surveys, plus a reusable 1200-frame current-state survey for later stages/checkpoints.
- Permanent RC3.0 focused regression wired into Full Regression.
- Long script sampling returns compact sample results while retaining full shared recorder state, avoiding quadratic Script Console evidence growth.

## Semantic discipline

Only the already-confirmed Stage-1 special-target object is exported as a semantic car. Other live sprites remain `UNCLASSIFIED_DYNAMIC_OBJECT`. Surface signatures are `UNCLASSIFIED` evidence candidates until screenshots/behaviour prove material semantics. Structural branch A/B remains unlabeled as left/right.

## Promotion

This package is a source candidate until Windows `Build-Debug.bat`, the focused RC3.0 regression and Full Regression pass.

- SDL pause convenience: Ctrl+P now mirrors Pause/Break for keyboards without a dedicated Pause key.

## Candidate correction — Track View tab routing

The first Windows-proven mapping regression exposed a Workbench presentation defect: the Track View button still routed to the historical panel title `Live SVG Track View` after the RC3.0 panel was renamed `Course Mapping / Live Survey`. The mapping backend, recorder v2 and export path were working, but the browser hid the renamed panel and therefore displayed an empty Track View.

The corrected RC3.0 candidate aligns the tab group with `Course Mapping / Live Survey` and adds a `Validate-Workbench.ps1` guard so a future panel-title/tab-routing mismatch fails validation before packaging.

## Candidate correction — combined live-config + Track View fix

A later Windows diagnostic exposed a packaging merge regression: the candidate that fixed Track View routing had accidentally reintroduced the earlier PowerShell bridge signature `Set-CourseFollowConfig($args)`. Because `$args` is PowerShell's automatic argument array, `course.follow.configure` failed with `System.Object[] does not contain a method named 'ContainsKey'` before the survey could start.

This replacement candidate contains **both** corrections simultaneously:

- `Set-CourseFollowConfig($config)` with all configuration lookups performed against the explicit hashtable;
- `track:['Course Mapping / Live Survey']` routing to the renamed Track View panel.

`Validate-Workbench.ps1` now rejects either regression independently. The prior candidate whose archive SHA-256 was `6b209d84945a64f6d33435926a0fdb4a8fbcab7f86e60b4b04b878f8a00f56b1` is withdrawn.
```

</details>

<details>
<summary>RELEASE_NOTES_0.66.9.0-RC2.md</summary>

```text
# ChaseHQ-Native v0.66.9.0-RC2

## Purpose
Focused graphics/gameplay research candidate built from the proven v0.66.9.0-RC1 base.

## Changes
- Fixes `sprite.map.inspect map=N`: map-based inspection now resolves the currently active matching sprite slot(s) instead of silently treating the requested map as slot 0.
- Adds runtime same-priority sprite ordering inspection and reversible A/B control:
  - `sprite.order.inspect`
  - `sprite.order.set mode=higher-slot|lower-slot`
  - Game Lab aliases and controls.
- Adds a Game Lab Sprite Ordering card for the player-car/under-car-effect investigation.
- Corrects speed terminology in Game Lab:
  - `0x100400` = packed-BCD HUD/display speed.
  - `0x10041C` = raw internal/physics speed.
  These values are intentionally separate and are not expected to numerically match.
- `Set once`, speed freeze, turbo stock and turbo active Game Lab mutations now preserve the prior RUNNING/PAUSED state. Low-level `memory.write` remains debugger-safe and pauses by design.
- TC0100SCN `tile.inspect` output filenames now include the current frame so repeated historical inspections do not overwrite earlier animation phases.
- Unhandled native `ERR ...` results now fail `.chqscript` execution when Stop on error is enabled.
- Adds reusable turbo HUD animation and player-car sprite-order investigation scripts.
- Adds focused regression `Regression - v0.66.9.0 RC2 Sprite Order and Speed Semantics`.

## Current research state
Turbo HUD animation is confirmed game behaviour: normal 7C-7F characters are replaced by animated RAM characters 0C-0F, sourced from a three-frame table at 0x33A1A/0x33A5A/0x33A9A and cleared on expiry.

The current player-car issue is now framed as a same-priority sprite-order investigation. At the canonical driving frame, player car slot 82/map 475 and under-car effect slots 83/84 share priority 0. RC2 makes the renderer tie-break directly testable against the already captured timeline.

## Validation target
Run `Regression - v0.66.9.0 RC2 Sprite Order and Speed Semantics` before Full Regression.
```

</details>

<details>
<summary>RELEASE_NOTES_0.66.9.0-RC2.9.md</summary>

```text
# ChaseHQ-Native v0.66.9.0-RC2.9 release candidate

RC2.9 consolidates the RC2.8 saturation research into code, canonical checkpoints and documentation.

## Added / changed

- Confirmed target remaining-hit counter `0x1002AE` and authentic `A112/A124/A118` damage/terminal paths promoted everywhere.
- Four target research checkpoints bundled: contact pair 5587/5588 and preferred damage pair 6333/6334.
- Experimental **One-hit Target Kill**, default OFF, preserving the authentic terminal underflow path.
- Native/script/UI parity: startup `--one-hit-target`, debugger `target one-hit ...`, script `game.target.one-hit`, Experimental-tab toggle.
- SDL Pause/Break toggles pause/resume globally; paused state appears in the SDL title.
- Dashboard live frame and SDL controls share a wide-screen row and collapse responsively.
- `control.run-frames` timeout is configurable, default 900000 ms; the front-proxy/browser long-operation budgets are 3700 s / 3700000 ms so the documented 3600000 ms maximum is not truncated by transport timeouts.
- Gameplay Registry, Next Steps, semantic objects, handbook, API/script docs and handover updated.

## Validation boundary

The source is based on Windows/SDL-proven RC2.8, but RC2.9 requires its own Windows build, focused regression and Full Regression before promotion.

## Replacement archive note

An earlier pre-proof RC2.9 archive was withdrawn after Windows `Prepare-Project` correctly exposed a stale validator expectation for the old 300000 ms wait. The corrected archive aligns the validator with the RC2.9 progress-aware contract and retains the widened 3700-second/3700000-ms transport budgets. Do not use the earlier RC2.9 ZIP/hash.

## Windows build-gate correction

A subsequent Windows build exposed two additional stale release-gate defects before RC2.9 promotion: `tests/cli_evidence_tests.ps1` still expected the historical RC2.7 evidence build identity, and the Debug/Release batch wrappers captured `ERRORLEVEL` with `%ERRORLEVEL%` inside a parenthesized block instead of delayed `!ERRORLEVEL!`. The latter could incorrectly classify multiple failing CTests as the one tolerated historical CPU segfault. RC2.9 now asserts evidence identity `0.66.9.0-RC2.9`, uses delayed error-level capture, and permits the historical CPU segfault only when CTest reports exactly one failed test.
```

</details>

<details>
<summary>RELEASE_NOTES_0.66.9.0-RC2.8.md</summary>

```text
# ChaseHQ-Native v0.66.9.0-RC2.8

RC2.8 carries forward the Windows-proven RC2.7 Run 009 baseline (Full Regression completed in 59.615 seconds and the build was marked saturated) and advances the graphics and research work from those findings.

## Changes

- Corrected the normal TC0100SCN text origin from `+23` to `+16` and moved text source-X calculation into a named helper.
- Implemented a separate flipped-text path with 512-pixel source mirroring and the distinct +23 offset and added coordinate tests for normal/flipped behavior.
- Added an independent sprite 4bpp SHADOW decoder and COMPARE helper. The shadow path is diagnostic and does not replace the live decoder or claim whole-frame SHADOW VERIFIED status.
- Added a deterministic regression script that captures and checks the Stage 1 driving and end-results checkpoints, including HUD/radar evidence and exact snapshot reconstruction.
- Corrected gameplay research classification: CPU-A `0x1002E8` is the end-results/intermission gate; `0x1002AE` is unconfirmed and no longer presented as target health.
- Updated the responsive Workbench layout, added an `Open in Evidence` action for selected runs, and refreshed the JSON-driven Next Steps items.
- Updated the native status version field and release metadata to RC2.8.

## Proof status

The included RC2.7 Windows/SDL Run 009 bundle establishes the starting baseline. RC2.8 native unit tests and packaging checks are recorded in `VALIDATION_0.66.9.0-RC2.8.md`. The RC2.8 Windows/SDL build and new Workbench regression still require execution with the project's local SDL/ROM staging configuration.
```

</details>

<details>
<summary>RELEASE_NOTES_0.66.9.0-RC2.7.md</summary>

```text
# Active candidate: v0.66.9.0-RC2.7

Authoritative source baseline: supplied v0.66.9.0-RC2.6.1. RC2.7 is a source/research candidate awaiting local Windows/SDL promotion, not a proven Windows release.

Implemented: shared TC0100SCN BG X correction with unchanged Y/origin/zero offsets; same-state legacy/corrected diagnostic mode; correct pre-sprite snapshot base; parsed authoritative history sorting without pre-filter truncation; server-side Current Session queries; full release stamping from src/version.h; bounded local cache and exact selected-run identity; wired SDL controls; live layer offset get/set/reset/assert; checkpoint-frame capture before advancement; requested-artifact completeness checks and CLI identity manifest.

Linux proof: TC0100SCN coordinate/raster/column-zero detection tests PASS; sprite 16-pen decoder PASS; sprite ownership/priority PASS; full CPU/bus/runtime test target compiled and PASS. JavaScript current-session/version/identity test PASS and frontend syntax check PASS. Windows CTest history/evidence tests and full Script Console regressions are bundled but not executed here.

Next gate: `Build-Debug.bat` stages local SDL/ROMs via `Prepare-Project.ps1` and `build-local.json`, compiles Windows/SDL and runs CTest. Then `Start-ChaseHQ.ps1 -Restart`, focused RC2.7 regression, preserved RC2.6.1 sprite regression, and Full Regression through its terminal COMPLETE marker. Inspect all four canonical old/corrected captures and require zero reconstruction mismatches before promotion.

Column scroll remains unsupported beyond the observed zero table and is exposed as such. Semantic TC0100SCN state and pixel-to-tile provenance remain stretch goals with an explicit unimplemented schema plan. No independent shadow backend or SHADOW VERIFIED claim is made.

References: `TC0100SCN_RC261_FINDINGS.md`, `NATIVE_RECONSTRUCTION.md`, `RELEASE_SCOPE_RC27.md`, `../VALIDATION_0.66.9.0-RC2.7.md`. Canonical post-final-hit 9548 is the packaged checkpoint filename; do not rename it to the older near-destruction label.


See `docs/RELEASE_SCOPE_RC27.md` for every approved item.
```

</details>

<details>
<summary>RELEASE_NOTES_0.66.9.0-RC2.6.md</summary>

```text
# ChaseHQ-Native v0.66.9.0-RC2.6 — Sprite Bitplane Significance Correction

**Status: DEVELOPMENT CANDIDATE — source correction implemented and native focused tests pass in the non-SDL build environment; fresh Windows/SDL proof is required before promotion.**

RC2.6 is a focused game-facing correction on top of the RC2.5 source baseline. RC2.5's frame-2352 Map-405 causal experiment proved that the native sprite decoder used the correct physical source-plane addresses but assigned their 4bpp significance in reverse. RC2.6 corrects that at the shared source decoder rather than compensating in palette or compositor code.

## Renderer correction

- Add shared `src/sprite_gfx.h` as the single 16x16x4bpp sprite-pixel decode contract used by both live rendering and export/inspection tooling.
- Physical plane 0 contributes pen bit 3, plane 1 -> bit 2, plane 2 -> bit 1, plane 3 -> bit 0.
- Preserve existing tile addressing, geometry, zoom, flip, sprite traversal, first-nontransparent ownership, priority masks and palette contents.
- Preserve transparency semantics: pen 0 remains 0 and every nonzero nibble remains nonzero under the corrected significance mapping.
- Remove duplicated live/export significance logic so map inspection, per-sprite exports and runtime rendering cannot drift silently.

## Causal evidence carried into the build

The RC2.5 `Sprite Bitplane-Significance Causal Probe` ran in session `20260930_175159`, run `008`, at Stage 1 frame `2352`, targeting Map `405` / palette bank `175`. It changed only the palette entries selected by the currently decoded pens to simulate the MAME-consistent 4-bit reversal. Sixteen visible Map-405 sprites became coherent shaded cloud artwork while geometry and priority remained unchanged. The structured snapshot remained exact with `verifiedExact=true` and `mismatchedPixels=0`.

The uploaded evidence bundle SHA-256 is:

`343c14d710023fbb308312900dc453d2eed365409243242c634493aece42c6b2`

A curated evidence report and representative images are packaged under `docs/evidence/sprite-bitplane-significance/`.

## Regression / prove-off additions

- `tests/sprite_gfx_tests.cpp` exhaustively checks all 16 physical-plane combinations against the corrected pen significance and verifies geometry/transparency invariants.
- `Test-SpriteGfx.bat` is the focused Windows test entry point.
- Keep the historical `cpu_bus_rom_tests (SEGFAULT)` exception narrow while allowing the CTest total to grow: `Build-Debug.bat` now requires exactly one failed test but no longer hard-codes the total test count.
- `research/scripts/graphics/sprite-bitplane-significance-proveoff.chqscript` captures the canonical frame-2352 Map-405 scene with **no palette overrides**.
- Existing `Test-SpritePriority.bat` and `regression-sprite-ownership.chqscript` remain mandatory because sprite pens feed occupancy/visibility even though 4-bit reversal preserves zero-vs-nonzero status.

## Documentation / knowledge

- Promote sprite bitplane significance to a confirmed evidence-backed graphics finding.
- Update the sprite hardware/video pipeline reference and sprite discovery status.
- Update project state, roadmap, handover, handbook source, script catalog and feature manifest.
- Correct the top-level README/current-candidate pointer to RC2.6.

## Separate known tooling issue

The RC2.5 causal snapshot exposed that `scene-no-sprites.png` is byte-identical to `final.png` despite a non-empty aggregate sprite layer. That is logged as a separate forensic-export defect and is intentionally **not** mixed into this source-decoder correction.

## Proof status in this packaged handoff

Completed in the available non-SDL environment:

- configure with `CHASEHQ_BUILD_SDL=OFF`;
- compile `sprite_gfx_tests` and `sprite_priority_tests`;
- both focused native tests PASS.

Still required on the user's Windows/SDL development machine before promotion:

1. `Build-Debug.bat`;
2. `Test-SpriteGfx.bat`;
3. `Test-SpritePriority.bat`;
4. run `Sprite Bitplane-Significance Native Prove-Off` and inspect `map405-native-plane-order`;
5. run `Regression - Sprite Ownership and Export Coordinates`;
6. run `Full Regression Suite` through its terminal COMPLETE marker;
7. perform canonical sprite-scene visual checks (player car, roadside sprites, target/enemy, end-level/HUD).
```

</details>

<details>
<summary>RELEASE_NOTES_0.66.9.0-RC2.6.1.md</summary>

```text
# ChaseHQ-Native v0.66.9.0-RC2.6.1 — Versioned Sprite Regression Packaging Fix

**Status: DEVELOPMENT CANDIDATE — same emulator/source-decoder semantics as RC2.6; packaging/regression labelling corrected. Windows/SDL proof remains required before promotion.**

RC2.6.1 does not change the RC2.6 sprite-decoder implementation. It corrects the handoff package so the focused regression follows the established build-specific naming and discovery convention used by earlier RC candidates.

## Packaging / regression correction

- Add `research/scripts/regression/regression-v06690-rc261-sprite-bitplane-significance.chqscript`.
- Label it explicitly as **Regression - v0.66.9.0 RC2.6.1 Sprite Bitplane Significance**.
- Give it `regression-version: 0.66.9.0-RC2.6.1` and `regression-scope: current` metadata.
- Include it in `full-regression.chqscript` before the terminal `=== ChaseHQ Full Regression: COMPLETE ===` marker.
- Update the regression README and script catalogue so the current focused regression is visible by version.
- Retain `graphics/sprite-bitplane-significance-proveoff.chqscript` as the separate visual/structured no-palette-override prove-off.

## Renderer semantics

Unchanged from RC2.6: live rendering and export/inspection share the corrected 4bpp decoder where physical planes 0,1,2,3 contribute pen bits 3,2,1,0. `Test-SpriteGfx.bat` remains the exhaustive all-16-pen native assertion.

## Why RC2.6.1 exists

RC2.6 had already been issued as a versioned ZIP. Project release discipline forbids silently replacing that archive under the same version/hash, so this packaging correction is issued as RC2.6.1.

## Remaining Windows/SDL gate

1. `Build-Debug.bat`;
2. `Test-SpriteGfx.bat`;
3. `Test-SpritePriority.bat`;
4. run **Regression - v0.66.9.0 RC2.6.1 Sprite Bitplane Significance**;
5. run **Sprite Bitplane-Significance Native Prove-Off** and inspect `map405-native-plane-order`;
6. run **Regression - Sprite Ownership and Export Coordinates**;
7. run Full Regression through its terminal COMPLETE marker.
```

</details>

<details>
<summary>RELEASE_NOTES_0.66.9.0-RC2.5.md</summary>

```text
# ChaseHQ-Native v0.66.9.0-RC2.5

RC2.5 is a coherent renderer-correctness and research-script reliability candidate on top of RC2.4.4.

## Proven renderer correction carried into this package

The player-car black under-car/shadow defect was isolated at canonical frame 2352. Body slot 82/map 475 and shadow slot 81/map 575 genuinely overlap; reversing the entire traversal changes thousands of unrelated pixels and making black transparent would destroy real sprite colour data.

RC2.5 preserves MAME-reference descending traversal but implements the missing first-nontransparent sprite occupancy rule. A nonzero-pen candidate reserves the sprite pixel even when its colour is blocked by background priority, so later sprite entries cannot overwrite it. Existing BG/road/text mask decisions remain intact.

The same change corrects individual-sprite forensic export Y coordinates to use the live renderer's visible-raster origin.

Windows proof already completed on the pre-package working tree:
- focused frame-2352 assertion: `overlapPixels=689`, `frontOwnedOverlap=553`, `backVisibleOutsideFront=247`, `exportCoordinatesExact=true`;
- `sprite_priority_tests`: PASS;
- `sprite_snapshot_assertions`: PASS;
- Full Regression: PASS, session `20260930_151202`, run `002`, terminal `=== ChaseHQ Full Regression: COMPLETE ===`;
- pristine RC2.4.4 independently reproduces the historical `cpu_bus_rom_tests` SEGFAULT, proving that failure predates and is unrelated to the sprite change.

## Research Script / Workbench reliability

- Add `try ... finally ... end` to the Workbench Research Script compiler for guaranteed cleanup after normal completion and API/assert/runtime failure; cancellation cleanup is attempted where practical and the original body error remains primary if cleanup also fails.
- Correct nested block variable expansion so loops/conditionals inside `try` blocks are compiled with their local variables rather than being substituted prematurely.
- Add idempotent `timeline.reset`. Script-initiated `timeline.load` captures the pre-load live state so `timeline.reset` can restore it after `timeline.fork-live`.
- Add bidirectional Script Console action/schema parity enforcement. This permanently incorporates the manual `frame.snapshot.assert-overlap` allow-list correction discovered during Windows validation.
- Correct diagnostic bundle source attribution so `fault.json` identifies the actual pasted/library script rather than stale selected-library state.
- Default Script Console history to the active session, while retaining explicit version/session/all-history filtering.
- Strengthen root `AGENTS.md` with safe experiment lifecycle, evidence identity, API exposure parity, pre-existing-failure proof, and manual-hotfix reconciliation rules.
- Package `TC0100SCN Rowscroll Raster-Mapping Probe` with meaningful metadata and fail-safe timeline cleanup.

## Regression/tooling additions

- `research/scripts/regression/regression-sprite-ownership.chqscript` remains the permanent canonical ownership/export-coordinate assertion and is inside Full Regression.
- Add `research/scripts/regression/regression-v06690-rc25-script-safety.chqscript` to exercise timeline seek/fork cleanup back to canonical frame 2064.
- `frame.snapshot.assert-overlap` has offline PowerShell parity through `scripts/Assert-SpriteSnapshotOverlap.ps1`.
- Release/workbench validators now check RC2.5 action parity, script-safety wiring, current-session history defaulting, diagnostic source attribution and permanent regression inclusion.

## Next graphics investigation

Two-frame evidence shows the visible sky/banding exists in raw `bg-bottom`, before final composition. Between frames 2352 and 2360, measured ~3/~7/~10-pixel horizontal movements correspond to rowscroll RAM changes of +3/+7/+10, with the vertical control changing by one. The remaining question is which rowscroll entries map to which visible raster lines. RC2.5 packages a safe counterfactual probe for that question; it does not claim the sky issue fixed.

## Candidate gate

The sprite correction is already proven, but RC2.5 introduces new Workbench/script semantics. Build this exact package on Windows, run the focused Script Safety regression, focused Sprite Ownership regression, and Full Regression before promoting RC2.5 from candidate to proven.
```

</details>

<details>
<summary>RELEASE_NOTES_0.66.9.0-RC2.4.md</summary>

```text
# ChaseHQ-Native v0.66.9.0-RC2.4

RC2.4 fixes a real TC0100SCN renderer error exposed by the RC2.3 RAM-layout and controlled layer-offset experiments.

## TC0100SCN Y-scroll fix

The renderer previously converted control Y to a negative `scrolly`, then subtracted that value again while calculating source Y. That inverted the intended sign. With Chase H.Q. `CTRL3/CTRL4 = 0x01E0`, this displaced the background planes by 64 source lines modulo the 512-line tilemap and led to long-lived presentation-offset compensation.

RC2.4 uses one shared `tc0100scn_source_y()` helper for BG0, BG1 and TEXT and for geometry tracing. The historical BG0 `0:-4` and BG1 `0:-20` defaults are removed; both now default to `0:0`. `--layer-offset` remains available for explicit forensic experiments.

The RC2.3 A/B evidence is especially strong: the `-64` sign-proxy moved the dark and pale building planes from the lower road/foreground region to the skyline/horizon. At the captured `0x01E0` control value, that proxy is exactly equivalent to the corrected renderer with neutral offsets.

## Regression

A native geometry regression now asserts the wrapped source-Y result for the proven `0x01E0` case. CPU-only CMake build and `cpu_bus_rom_tests` pass locally. Windows/SDL visual and full-regression proof remain the release gate.

## Compatibility

No API, schema or `.chqscript` grammar changes. Evidence metadata continues to report presentation offsets; their defaults are now neutral.

Focused Windows visual gate: `./Validate-TC0100SCN-Y.ps1` captures canonical frame 2065 with neutral defaults and requires byte-exact equality with the packaged RC2.3 both-sign-proxy reference.
```

</details>

<details>
<summary>RELEASE_NOTES_0.66.9.0-RC2.4.4.md</summary>

```text
# ChaseHQ-Native v0.66.9.0-RC2.4.4

RC2.4.4 is a Workbench/regression-capacity and evidence-metadata hotfix on top of RC2.4.3. Native emulator/video rendering is unchanged.

## Windows evidence carried forward
The focused RC2.4.3 snapshot-discovery regression is now Windows-proven. The uploaded run created `regression-rc243-snapshot-discovery` at frame 2065 and the immediately following assertive `frame.snapshot.list require=regression-rc243-snapshot-discovery` returned that exact current-run snapshot. Both self-comparisons reported zero changed pixels.

The subsequent Full Regression attempt did not execute: preflight failed with `Expanded script exceeds 500 commands`. Static expansion of the packaged suite is 503 commands, so adding the RC2.4.3 regression pushed the permanent suite three commands beyond the historical ceiling.

## Fixes
- Raise the Research Script v2 expanded-command ceiling from 500 to 1000 in both browser and server-side script compilation paths.
- Keep the 64 KiB source, 1000 source-line, 256-loop-iteration and nesting-depth limits unchanged.
- Update the authoritative script-language schema/reference to the same 1000-command limit.
- Derive startup/research-session `build` metadata from `src/version.h` (with CMake fallback) instead of the stale hard-coded `0.66.8.0` value exposed in the uploaded diagnostic bundle.
- Add root `AGENTS.md` as the durable agent/Codex operating contract agreed for the project.
- Promote same-turn structured snapshot discovery from candidate to Windows-proven in project knowledge/state.

## Validation target
Build/start on Windows, run Full Regression, and confirm it reaches `=== ChaseHQ Full Regression: COMPLETE ===`. The focused RC2.4.3 snapshot-discovery regression does not need to be rerun merely to reproduce the already-uploaded PASS unless validating a fresh package.
```

</details>

<details>
<summary>RELEASE_NOTES_0.66.9.0-RC2.4.3.md</summary>

```text
# ChaseHQ-Native v0.66.9.0-RC2.4.3

RC2.4.3 is a Workbench snapshot-discovery correctness hotfix on top of RC2.4.2. Native emulator/video output is unchanged from RC2.4.2.

## Why this patch exists
The first Windows RC2.4.2 regression bundle proved that `frame.snapshot` created the requested snapshot, but the immediately following `frame.snapshot.list` still returned only older snapshots. The RC2.4.2 regression itself was also non-assertive, so the run was marked PASS despite the missing list entry.

## Fix
`New-StructuredFrameSnapshot` now records every newly created structured snapshot in an explicit same-process registry before invalidating disk-discovery caches. Snapshot listing merges this registry with cached/on-disk discovery, and named snapshot resolution consults it before scanning disk. The known evidence-session-root cache is also invalidated when the snapshot tree changes.

`frame.snapshot.list` gains an optional `require=NAME` parameter. When supplied, the action throws unless exactly one snapshot with that name is present in the returned list. Existing no-argument behaviour is unchanged.

## Regression coverage
Added `research/scripts/regression/regression-v06690-rc243-snapshot-discovery.chqscript`. It creates `regression-rc243-snapshot-discovery`, immediately executes `frame.snapshot.list require=regression-rc243-snapshot-discovery`, and therefore cannot report PASS if the same-turn discovery bug remains.

`Validate-FrameSnapshotRefresh.ps1` remains the HTTP-level validation gate.

## Graphics investigation result carried forward
The RC2.4.2 post-Y-fix layered capture localized the visible sky pattern to the raw TC0100SCN bottom/background source rather than the compositor. Because the pattern is already present in the raw TC0100SCN background source, it is not classified as a compositor corruption. A direct authoritative reference comparison is required before treating the source-layer pattern itself as a renderer defect. Remaining active graphics work returns to genuine mismatches such as player-car shadow/under-car ordering and any independently proven geometry/compositor differences.
```

</details>

<details>
<summary>RELEASE_NOTES_0.66.9.0-RC2.4.2.md</summary>

```text
# ChaseHQ-Native v0.66.9.0-RC2.4.2

RC2.4.2 is a focused Workbench/research-pack hotfix on top of RC2.4.1. The emulator/renderer is unchanged from RC2.4.1.

## Structured snapshot discovery
A structured snapshot created by `frame.snapshot` could be absent from an immediately following `frame.snapshot.list` or Graphics Lab refresh because the Workbench retained snapshot/root discovery caches for up to five/ten seconds. `New-StructuredFrameSnapshot` now invalidates both caches as soon as the manifest is committed.

## Packaged investigation workflow
Added `research/scripts/graphics/tc0100scn-post-y-fix-layer-isolation.chqscript`. The earlier `post-y-fix-sky` wording was merely an arbitrary snapshot evidence name, not a built-in command or existing project object.

## Regression coverage
Added `research/scripts/regression/regression-v06690-rc242-tc0100scn-snapshot-refresh.chqscript` and `Validate-FrameSnapshotRefresh.ps1`. The existing `Validate-TC0100SCN-Y.ps1` remains the byte-exact visual gate for the RC2.4 Y-scroll fix.

No native C++, Research API contract, or `.chqscript` grammar change is introduced by RC2.4.2.
```

</details>

<details>
<summary>RELEASE_NOTES_0.66.9.0-RC2.4.1.md</summary>

```text
# ChaseHQ-Native v0.66.9.0-RC2.4.1

RC2.4.1 is a launcher hotfix for `Start-ChaseHQ.ps1 -Restart` when a previous ChaseHQ Web Workbench from another extracted build still owns the standard HTTP port through HTTP.sys.

## Restart cleanup fix

`Get-NetTCPConnection` reports PowerShell `HttpListener` listeners as PID 4/System. RC2.4 correctly refused to terminate PID 4, but its fallback stale-host search was restricted to the current build folder. A Workbench left running from RC2.3 or another adjacent build could therefore keep port 37680 open and cause `-Restart` to time out.

The stale-host cleanup is now **port-scoped rather than build-folder-scoped**. It identifies `Start-ChaseHQWeb.ps1` processes by their `-HttpPort` argument and stops only hosts whose public port or private backend port overlaps the ports requested by the new session. Other ChaseHQ Workbench instances using different ports are left alone.

No emulator, TC0100SCN renderer, Research API, schema or `.chqscript` behavior changed from RC2.4.
```

</details>

<details>
<summary>RELEASE_NOTES_0.66.9.0-RC2.2.md</summary>

```text
# ChaseHQ-Native v0.66.9.0-RC2.2

## Purpose

Correct the native Chase H.Q. same-priority sprite traversal after the RC2/RC2.1 A/B evidence and authoritative MAME source review.

## Confirmed reference rule

Current MAME `src/mame/taito/taito_z_v.cpp`, function `chasehq_draw_sprites_16x16`, traverses sprite RAM from the final entry toward entry zero (`offs = bytes/2 - 4; offs >= 0; offs -= 4`). With ordinary transparent drawing, the later draw wins an equal-priority sprite/sprite overlap. Therefore lower-numbered Chase H.Q. sprite slots are drawn later and win equal-priority overlap.

Reference reviewed 2026-09-29: https://github.com/mamedev/mame/blob/master/src/mame/taito/taito_z_v.cpp

## Changes

- Native/default `sprite_tie_break` changed from `higher-slot` to `lower-slot`.
- `--sprite-tie-break higher-slot` remains available as a reversible forensic comparison.
- `sprite.order.inspect` continues to expose actual traversal and draw positions.
- Game Lab wording now identifies lower-slot ordering as the MAME-reference/native default.
- Added loop-based `graphics/same-priority-sprite-order-multiscene-validation.chqscript`.
- Added focused `regression-v06690-rc22-mame-sprite-order.chqscript` and included it in Full Regression.
- Project state, roadmap, handover, API/script documentation, knowledge and validation notes updated together.

## Evidence carried forward

The RC2.1 A/B captures showed that lower-slot ordering places slots 83/84 (the animated under-car effect) behind player-car slot 82 and produces the plausible player-car composition. The MAME source review independently confirms the same traversal direction.
```

</details>

<details>
<summary>RELEASE_NOTES_0.66.9.0-RC2.1.md</summary>

```text
# ChaseHQ-Native v0.66.9.0-RC2.1

RC2.1 is a packaging/build-gate hotfix for RC2. It contains no emulator/runtime behaviour changes.

## Fix

- Added the required `# regression-version:` and `# regression-scope:` metadata to `research/scripts/regression/regression-v06690-rc2-sprite-order-speed-semantics.chqscript`.
- Re-ran an equivalent of the packaged Workbench script-metadata gate across every `.chqscript` before packaging.
- RC2 gameplay, Game Lab, speed semantics, sprite-order tooling, documentation, API/schema and research functionality are otherwise unchanged.

## Why this hotfix exists

RC2 was packaged before its own `Validate-Workbench.ps1` metadata gate was exercised on the final archive tree. Windows correctly stopped the build at preparation time. RC2.1 corrects the metadata and treats the build gate as a release-blocking check.
```

</details>

<details>
<summary>RELEASE_NOTES_0.66.9.0-RC1.md</summary>

```text
# ChaseHQ-Native v0.66.9.0-RC1

Development candidate based on the proven v0.66.8.0 release.

## Game Lab
- Grouped/padded Experimental / Game Lab presentation with live turbo and speed readouts.
- Reversible API/UI controls for infinite turbo stock (`0x1003A2`), infinite active duration (`0x100414`), turbo fire/timer/count, and current speed set/freeze/clear (`0x10041C`).
- Speed control is explicitly current-speed manipulation, not yet claimed as a proven top-speed limiter.

## TC0100SCN character inspector
- New `tile.inspect` action for TC0100SCN RAM text characters (256 x 8x8, 2bpp).
- Exports raw pen matrix, palette-aware PNG, JSON metadata and source address.
- Graphics/Workbench inspector includes presets for turbo normal `0x7C..0x7F` and active `0x0C..0x0F` characters.
- Background ROM tile inspection is not yet implemented and is documented as future scope.

## Turbo/HUD research promoted into the build
- Bundled reusable scripts for layer/sprite correlation, exact tile writer, text-plane neighbourhood, gameplay-to-HUD writer path and tile-character inspection.
- Current evidence proves the active turbo HUD uses hard-coded `0x0C..0x0F`, normal icons use `0x7C..0x7F`, and expiry clears the consumed icon.

## Timeline documentation
- Documents the exact frame-boundary convention: bus writes tagged N occur during execution of N and are reflected in checkpoint N+1.

## Validation
Run `Regression - v0.66.9.0 Game Lab and Tile Inspector` first. Run Full Regression only after that passes. v0.66.8.0 remains the proven baseline until Windows/SDL proof completes.
```

</details>

<details>
<summary>RELEASE_NOTES_0.66.8.0.md</summary>

```text
# ChaseHQ-Native v0.66.8.0 — Forensic Timeline v1

**Status: PROVEN RELEASE — 2026-09-29**

This release is the promoted RC7.4 runtime with documentation/release metadata updated to final. No emulator/runtime semantics changed during promotion.

## Major capabilities

- Forensic Timeline v1: deterministic per-frame CHQSTATE capture plus complete bus-write provenance.
- Read-only historical inspection through the same memory/register/gameplay/sprite/palette surfaces used for paused LIVE state.
- Recorded-state SDL playback with `timeline.play`; `timeline.fork-live` restores a selected historical state into LIVE execution for causal experiments.
- Immutable source timelines with derived event windows/analyses written into the current run artifact tree.
- Streaming large-write analysis with ranked byte/address candidates, writer groups and automated writer disassembly.
- Portable automated timeline analysis including before/active/mid/expiry/after historical milestones and HUD/graphics correlation.
- Legacy quoted-CSV compatibility for the first turbo capture; no gameplay replay is required to reuse it.
- Progress-aware long timeline recording waits.
- Script-library autoload/bootstrap fixes and improved Workbench startup diagnostics.
- Bounded, summary-first run-history discovery and bounded browser output to prevent Full Regression browser OOM.
- Script history version/status filtering, including RC-aware labels and PASS/FAIL/CANCELLED filtering.
- Thumbnails default OFF; large artifact sets are summarized rather than eagerly rendered.

## Proven Windows/SDL gates

- RC7 portable automated timeline-analysis focused regression: PASS.
- Real `Analyze Existing Turbo Forensic Timeline`: PASS, producing portable candidate/writer/disassembly/milestone/graphics analysis without replaying gameplay.
- `Regression - v0.66.8.0 RC7.4 Bounded History Discovery`: PASS, 1.071 s.
- Final `Full Regression Suite`: PASS, 78.422 s, session `20260929_195302`, run `002`.

## Known limitation

Authentic Chase H.Q. audio remains unavailable because the native runtime still exposes `sound_stub`. Timeline manifests state this explicitly; no release claim is made for real sound CPU/chip/PCM capture or playback.

## Next substantive direction

The release gate is closed. Development can return to game fidelity/reverse-engineering work, using the timeline system to reduce repeated manual gameplay reproduction. Immediate research priorities include the turbo HUD/source-compositor path, broader authoritative gameplay-state discovery, target/end-level causal analysis, and graphics/rendering fidelity.


Release evidence summary: `docs/RELEASE_PROOF_0.66.8.0.md`.
```

</details>

<details>
<summary>RELEASE_NOTES_0.66.8.0-RC7.md</summary>

```text
# v0.66.8.0-RC7 — Portable Automated Timeline Analysis

- Derived timeline analysis now writes into the current run artifact tree; source `.chqtimeline` recordings remain immutable.
- `source.json` records source path and SHA-256 provenance for manifest/write stream.
- Candidate ranking now rewards event-edge bounded state and tags confirmed turbo reference addresses.
- New `timeline.analyze.auto` performs writer-PC disassembly, historical milestone state capture, layered snapshots and HUD_TURBO graphics correlation.
- New `timeline.play` performs paced SDL playback of recorded frame checkpoints. This is state playback, not CPU re-execution.
- New `Analyze Existing Turbo Forensic Timeline` and `Replay Turbo Timeline in SDL` scripts.
- New permanent RC7 focused regression wired into Full Regression.
- No gameplay replay is required for the existing turbo investigation.
```

</details>

<details>
<summary>RELEASE_NOTES_0.66.8.0-RC7.4.md</summary>

```text
# ChaseHQ-Native v0.66.8.0-RC7.4

Focused Workbench scalability hotfix after reproducible Chrome OOM at Full Regression step 646 (`api script.runs`).

- `script.runs` is now bounded/paginated and summary-only by default.
- `/api/v1/script/runs` defaults to 50 records and accepts `limit`, `offset`, `status`, `version`, and `session`.
- Script action `api script.runs` defaults to 25 records and supports the same filters.
- Historical run discovery no longer embeds full console logs, source text, or complete artifact path arrays in every list response.
- Browser command display truncates individual outputs above 200,000 characters while the authoritative complete console remains on disk/in the evidence bundle.
- Full Regression discovery calls explicitly request bounded history pages.
- Added focused RC7.4 bounded-history regression.
```

</details>

<details>
<summary>RELEASE_NOTES_0.66.8.0-RC7.3.md</summary>

```text
# ChaseHQ-Native v0.66.8.0-RC7.3

Focused Workbench browser-memory and run-history usability hotfix on the RC7.2 runtime baseline.

## Changes

- Script Console **Show thumbnails** now defaults OFF.
- Artifact output is summarized by file type instead of rendering hundreds of context-free cards by default.
- Opt-in thumbnail rendering is bounded and lazy-loaded (24 PNG previews plus a small non-image set).
- Live Script Console DOM is bounded to the newest 120 output blocks; older blocks are explicitly marked as elided while complete authoritative output remains on disk/in the run bundle.
- Recent Script Runs now derives the package/RC label from the run path where available (for example `0.66.8.0-RC7.2`) instead of collapsing all RCs into `0.66.8.0`.
- Added run-history Status filter: All / PASS / FAIL / CANCELLED.
- Current-session filtering now selects the derived RC-aware version label and current session.
- Added static validation for the RC7.3 browser-memory and history-filter contract.

No emulator/runtime semantics changed from RC7.2.
```

</details>

<details>
<summary>RELEASE_NOTES_0.66.8.0-RC7.2.md</summary>

```text
# ChaseHQ-Native v0.66.8.0-RC7.2

Build-validation-only hotfix for RC7.1. No emulator/runtime behavior change.

- Fixes the RC7 portable timeline analysis/playback static check in `Validate-Workbench.ps1`.
- RC7.1 incorrectly tested `$web`, which is only the filesystem path to `Start-ChaseHQWeb.ps1`, instead of `$text`, which contains the Workbench source text.
- The actual RC7 Workbench already contains `Get-TimelineAnalysisDirectory`, `Invoke-TimelineAutomatedAnalysis`, `timeline.analyze.auto`, and `timeline.play`; the validator was checking the wrong variable.
- Adds direct package-side validation that all four RC7 wiring markers exist in the Workbench source and that the timeline scan targets the current-run analysis artifact directory.
- Runtime behavior remains identical to RC7.1.
```

</details>

<details>
<summary>RELEASE_NOTES_0.66.8.0-RC7.1.md</summary>

```text
# ChaseHQ-Native v0.66.8.0-RC7.1

Packaging/build-validation hotfix for RC7. No emulator/runtime behavior change.

- Fixes `Validate-Workbench.ps1` so function extraction ends at the next PowerShell function declaration instead of assuming two implementation functions are adjacent.
- This removes the false failure `timeline event-window implementation is not streaming/non-destructive` introduced when RC7 inserted helper functions between the already-proven timeline functions.
- ROM staging validation output now labels source and staged-destination checks separately instead of printing the same generic PASS line twice.
- RC7 timeline analysis/playback implementation is otherwise unchanged.
```

</details>

<details>
<summary>RELEASE_NOTES_0.66.8.0-RC6.md</summary>

```text
# ChaseHQ-Native v0.66.8.0-RC6

Focused Forensic Timeline compatibility fix.

- Fixes `timeline.scan.memory` against legacy/damaged RC2 timelines whose `writes.csv` was rewritten by PowerShell `Export-Csv` and therefore contains quoted numeric fields.
- Streaming parser now accepts both original unquoted native CSV and quoted legacy CSV without `Import-Csv`.
- `timeline.scan.memory` now accepts `trigger`/`stop` aliases as well as canonical `triggerFrame`/`stopFrame`, while the packaged recovery script uses the canonical names.
- Adds permanent RC6 legacy timeline CSV compatibility regression.
- No gameplay replay is required; the existing turbo recording remains the investigation source.
```

</details>

<details>
<summary>RELEASE_NOTES_0.66.8.0-RC5.md</summary>

```text
# ChaseHQ-Native v0.66.8.0-RC5

Focused startup/recovery correction after RC4 Windows prove-off.

- Restores the RC3-proven Workbench source as the base and keeps the turbo recovery non-destructive.
- The recovery script no longer calls `timeline.trim.event` on the legacy RC1 capture; it uses the already-proven turbo event frames 2065..2275 and one-frame context directly.
- `timeline.scan.memory` bounds its streaming pass from explicit trigger/stop/window arguments even when no event index exists.
- Corrects stale `0.66.7.4` build identity in `Start-ChaseHQResearch.ps1` startup/session records.
- Workbench child stdout/stderr are now redirected to `.chq/web-stdout.log` and `.chq/web-stderr.log` so an early process failure is no longer silent.
- Adds `.chq/web-startup.log` before the private backend listener is opened and records backend/type/front-proxy startup phases and exceptions.
- Launcher Web readiness deadline increased from 15 to 30 seconds to allow first-run Add-Type/JIT startup.
- Support/diagnostic bundles now carry `web-startup.log`.

The original turbo gameplay must not be replayed for this correction.
```

</details>

<details>
<summary>RELEASE_NOTES_0.66.8.0-RC3.md</summary>

```text
# ChaseHQ-Native v0.66.8.0 RC3

RC3 is a focused hotfix for offline Forensic Timeline analysis after the real 711,374-write turbo recording exposed a >360 s Windows PowerShell timeout in RC2.

Changes:
- `timeline.trim.event` now performs streaming edge detection and stops after the matching falling edge.
- The command is now non-destructive: it writes an event-window view under `analysis/` and does not delete checkpoints or rewrite the authoritative `frames.csv` / `writes.csv`.
- `timeline.scan.memory` now streams the CSV and aggregates only changed writes, automatically using the indexed event window when present.
- Added a focused RC3 streaming-analysis regression and permanent validator guards against reintroducing full `Import-Csv` / destructive trim behaviour.
- The existing RC1 `turbo-complete-cycle` recording remains the dataset to use; do not replay gameplay.
```

</details>

<details>
<summary>RELEASE_NOTES_0.66.8.0-RC2.md</summary>

```text
# ChaseHQ-Native v0.66.8.0 RC2

RC2 is a focused integration hotfix over RC1. Native Forensic Timeline v1 semantics are unchanged.

## Fixed

- Script Library now auto-loads the selected script at initial discovery and after category/version/scope/search filtering, preventing stale editor text from being run under a different visible selection. Frontend bootstrap requires a real selected script load before READY.
- Long `control.run-frames` operations are progress-aware instead of failing at a fixed 15-second threshold. Browser/front-proxy operation budgets were widened to permit heavy per-frame timeline capture.
- Turbo capture advances its 240-frame tail as four 60-frame chunks for clearer progress and additional resilience.
- Restart browser bootstrap uses a unique query URL, clears stale frontend-error diagnostics for the new Workbench instance and waits up to 20 seconds.
- Build-Debug now owns one coherent `[1/6]..[6/6]` sequence; Prepare-Project prints subordinate unnumbered steps.
- Added a focused RC2 long-run regression and a recovery script for the already-captured RC1 turbo timeline.

## Important evidence recovered from RC1

The failed browser run did not lose the turbo recording. The uploaded RC1 dataset is complete: frames 2064..2319 (256 frame checkpoints), 711,374 bus-write records, turbo-active rises at frame 2065 and falls at frame 2275. Do not replay gameplay simply to recreate this event.
```

</details>

<details>
<summary>RELEASE_NOTES_0.66.7.9.md</summary>

```text
# ChaseHQ-Native v0.66.7.9 — Proven Release

Status: **PROVEN** on Windows/SDL, 2026-09-28.

## Release proof

- `Build-Debug.bat` completed through the canonical dependency/validation/build path.
- `Start-ChaseHQ.ps1 -Restart` reached native API, Workbench and browser bootstrap READY.
- `Recovery / Fast Release Gate` passed.
- `Regression - v0.66.7.9 Research Recovery` passed.
- `Full Regression Suite` passed with build `0.66.7.9`, `ok=true`, `status=PASS`; final marker `=== ChaseHQ Full Regression: COMPLETE ===`.

## What this release establishes

- Release/prove-off failures from the v0.66.7.6-v0.66.7.8 cycle are documented and converted into permanent validation/process rules.
- Script execution uses one parent run and one final bundle; transient predicate/preflight calls do not create incidental runs.
- Completed run history and structured snapshots are rediscoverable from disk without unbounded recursive scans.
- Developer handoff packages use `build-local.json` and exclude bundled SDL source, ROMs, `out`, transient evidence and patch/back-up files.
- Turbo gameplay state is already confirmed; active research now returns to the bottom-HUD turbo indicator source/compositor path.

## Next research

Run `research/scripts/graphics/turbo-hud-before-during-after.chqscript`, then use the causal-state and writer-trace scripts only as needed.
```

</details>

<details>
<summary>HOTFIX_NOTES_0.66.5.1.txt</summary>

```text
ChaseHQ-Native v0.66.5.1 - Track Recorder Scripting Hotfix

Purpose
-------
Patch v0.66.5 validation/usability gaps without expanding into the planned layered-frame/HUD feature work.

Changes
-------
* Track View recording is shared server-side state.
* Research Script actions: track.record.start/stop/clear/status/sample.
* track.svg.export writes deterministic SVG evidence and refuses empty recordings.
* Track View UI uses the same shared recorder state.
* Added regression-v06651-track-recorder.chqscript and scripted 3600-frame SVG survey.
* Corrected memory-trace regression/docs: width filters originating write transaction width; use width=any when unknown.
* Build-Debug.bat / Build-Release.bat retain Prepare-Project.bat bootstrap and the narrow known cpu_bus_rom_tests SEGFAULT exception.
```

</details>

<details>
<summary>HOTFIX_NOTES_0.66.1.2.txt</summary>

```text
ChaseHQ-Native v0.66.1.2 - Regression & Resilience Polish

- Restores appendScriptOutput and validates it statically.
- Resolves @runtime/web-listener.log correctly through logs.tail.
- Updates stale logs.tail schema/regression references.
- Stops the script progress ticker immediately on FAILED/CANCELLED paths.
- Moves the full-regression COMPLETE marker to the true end.
- Migrates legacy v1 run-history records into v2 without deleting the v1 backup.
- Adds regression Version and Regression view (Current build/Permanent/All) filters.
- Adds regression-version/regression-scope metadata support.
- Tones down routine web polling in the visible listener console while retaining the full request log on disk.
- Adds regression-v06612-resilience-hotfix.chqscript and stronger Validate-Workbench checks.
```

</details>

<details>
<summary>HOTFIX_NOTES_0.66.1.1.txt</summary>

```text
ChaseHQ-Native v0.66.1.1 — Resilience & Diagnostics Hotfix

This hotfix keeps the v0.66.1 emulator/research changes and hardens the paths that failed during Windows acceptance.

Fixed
- Restored the complete Script Console resilience helper/runtime block accidentally omitted in v0.66.1, including runScriptText, runner phase/state helpers, watchdog, frontend fault logging and diagnostic bundle creation.
- Browser frontend now performs a startup self-test and reports readiness to /api/v1/frontend-ready. The launcher does not print READY until the browser bootstrap reports ready (unless -NoBrowser is used).
- Web /api/health now reports scriptCount and frontendBootstrap state.
- Web listener writes timestamped startup/request messages both to its visible PowerShell console and .chq/web-listener.log.
- Local ROM staging now understands either a ROM root containing chasehq\ or the chasehq folder itself; it validates the 19 ROM files currently required by the native loader before build/launch.
- Native launch is supervised with a retained process handle, PID, stdout/stderr capture, early-exit detection and exit-code reporting.
- Startup state is persisted before API readiness, so -Diagnostics can inspect failed launches.
- Startup failures produce a diagnostic ZIP where possible instead of only a generic timeout.
- Web startup validates /api/health, /api/v1/status, /api/v1/scripts and a non-empty script library before declaring the server ready.
- Added .\Start-ChaseHQ.ps1 -SupportBundle for one-command support snapshots even when diagnosis is needed outside a successful run.
- Added Validate-Workbench.ps1 static resilience/version/script-library validation to the prepare/build path.
- Included the user's build-local.json with C:\Projects\AdditionalFiles\roms and C:\Projects\AdditionalFiles\SDL.

Important
- ROM content is never packaged in this ZIP.
- build-local.json only points to local folders; Prepare-Project fails clearly if they are unavailable/incomplete.
```

</details>

<details>
<summary>HOTFIX_NOTES_0.66.0.7.txt</summary>

```text
ChaseHQ-Native v0.66.0.7 - Runner Resilience / Restart / Diagnostics

- Carries forward the locally proven setScriptRunUi fix using the correct DOM IDs.
- Script runner lifecycle hardened with shared PREPARING/RUNNING/DONE/FAILED/CANCELLED states.
- Global try/catch/finally recovery prevents frontend exceptions leaving the Workbench BUSY.
- Stop now applies during preflight as well as command execution.
- Force reset runner control added for browser-side recovery.
- 30-second no-progress watchdog surfaces STALLED state rather than silently wedging.
- Startup DOM/helper self-test catches missing IDs/helpers before a script is run.
- Browser exceptions/unhandled promises are POSTed to @runtime/web-frontend-errors.log with phase/context/stack/last API call.
- Query/include preflight remains timed, cached, and now cancellation-aware.
- Added zero/one/many-query runner regressions.
- Start-ChaseHQ.ps1 adds -Restart, -HealthCheck and -Diagnostics.
- -Restart stops recorded processes, waits for ports to release, then launches fresh.
- -Diagnostics tails web stdout/stderr, command diagnostics and frontend-error logs.
- No Chase H.Q. emulation behavior changes.
```

</details>

<details>
<summary>HOTFIX_NOTES_0.66.0.6.txt</summary>

```text
ChaseHQ-Native v0.66.0.6 - Zero-query Preflight + Historical Artifact Resolver Hotfix

Fixes:
- Scripts with zero dynamic query functions no longer remain stuck at PREFLIGHT 0 / 0.
- Historical run entries now rehydrate missing version/session metadata from stored output/artifact paths when possible.
- Artifact serving for previous ChaseHQ version/session folders uses safe string/path checks instead of malformed regex matching.
- Previous-session image thumbnails can resolve from exact old paths, or from the same evidence-session suffix in sibling ChaseHQ-Native-v* folders when a version folder was renamed/moved.
- Keeps v0.66.0.5 preflight timeout/cache/progress behavior for scripts that do use query functions.

No emulator behavior changes.
```

</details>

<details>
<summary>HOTFIX_NOTES_0.66.0.5.txt</summary>

```text
ChaseHQ-Native v0.66.0.5 - Preflight Progress / Timeout / Cache Hotfix

- Script Console preflight now shows current query and N/total progress.
- Each query-function and include fetch has an 8 second timeout; hangs become named failures.
- Duplicate query expressions within one preflight are resolved once and cached.
- Full regression no longer includes a second query-function stress block; standalone regression-v066005-preflight.chqscript covers it.
- v0.66.0.4 Windows test-harness heap-fixture fix and history/artifact work are carried forward.
- No emulator behaviour changes.
```

</details>

<details>
<summary>HOTFIX_NOTES_0.66.0.4.txt</summary>

```text
ChaseHQ-Native v0.66.0.4 - Preflight, History & Windows Test Hotfix

- Script Console marks the run busy before query-function preflight, preventing background frame/status polling from racing preflight requests.
- Defensive JSON parsing identifies empty/truncated preflight responses with the action name and raw response sample.
- Run history records build version and session ID and can filter by either.
- Previous-session artifacts are resolved across retained ChaseHQ version folders when the original evidence file still exists.
- Missing historical artifacts render explicitly instead of as silent broken thumbnails.
- Windows runtime tests allocate large Bus test fixtures on the heap to avoid the MSVC Debug stack overflow observed as 0xC00000FD.
- Build scripts distinguish compile success from test failure.
- No Chase H.Q. emulator behaviour changes are introduced by this hotfix.
```

</details>

<details>
<summary>HOTFIX_NOTES_0.66.0.3.txt</summary>

```text
ChaseHQ-Native v0.66.0.4 - Workbench UX, Transport & Build Workflow Patch

User-validated v0.66.0.2 baseline plus:
- Serialized Workbench -> native debugger command dispatcher using a named mutex.
- Background SDL frame preview is suppressed while Script Console execution is active.
- Empty/native command diagnostics remain enabled; no automatic retry masks failures.
- Script output defaults to wrapped lines.
- Generated PNGs render inline beneath the command that produced them and remain in the run artifact gallery.
- Same-tab image modal/lightbox with previous/next/download/escape-close.
- Thumbnail toggle hides image previews only; non-image artifact cards remain visible.
- Artifact downloads preserve original filenames through the concurrent front proxy.
- Artifact serving can resolve prior evidence sessions after a Workbench restart, including safe sibling-version evidence/checkpoint paths inside the ChaseHQ workspace.
- Persistent run history can therefore reopen older thumbnails after restart where the source artifact still exists.
- Carries forward the locally proven PowerShell ${width}: parser fix.
- Adds CMakePresets.json plus Prepare-Project.bat, Build-Debug.bat, Build-Release.bat, Build-And-Run.bat, Rebuild-All.bat, Clean-Build.bat and Find-CMake.bat.
- Prepare-Project.bat recursively Unblock-File's packaged PowerShell scripts without changing machine-wide execution policy.
- Build launchers support both legacy flat x64-Debug executables and Visual Studio generator Debug/ subfolders.

No Chase H.Q. emulation behavior, gameplay addresses, track semantics or audio runtime behavior is intentionally changed by this patch.
```

</details>

<details>
<summary>HOTFIX_NOTES_0.66.0.2.txt</summary>

```text
ChaseHQ-Native v0.66.0.2 - Workbench UX & Diagnostics Patch

Built on the user-validated v0.66.0.1 Windows/SDL baseline. No Chase H.Q. emulation behaviour is intentionally changed.

Script Console UX
- Script library category/search structure retained; selecting a script auto-loads it again.
- PowerShell source is UTF-8 BOM for Windows PowerShell 5.1; script metadata is read explicitly as UTF-8.
- Full-width script output beneath editor/preview.
- Collapsible syntax preview.
- Wrap/no-wrap and maximize/restore output controls.
- Inline generated-image thumbnails are rendered separately from the text log, so Copy output remains plain text.
- Recent run history is persisted in browser localStorage with status, duration, command count, script name, output and detected artifact paths.
- Artifact files under the active session/evidence roots can be viewed/downloaded through a guarded local endpoint.

Evidence / registry
- Evidence list now ignores container directories such as frame-snapshots, experiments and script-runs unless they are real chq-evidence-v2 captures.
- Unknown gameplay-registry values remain null in the API and render as an ASCII-safe placeholder in the UI.

Logs / diagnostics
- logs.tail now accepts the exact @runtime identifiers returned by logs.list in both HTTP and Script Console APIs.
- Added @runtime/web-command-diagnostics.log.
- Invoke-Chq rejects truly empty command arrays before transport and records request/caller/command/status context when native transport returns ERR empty command.
- track.state internal reads now carry field-specific context in failure messages.
- Added v0.66.0.2 diagnostics regression and folded it into full-regression.chqscript.

Validation in build environment
- Embedded Workbench JavaScript syntax: PASS.
- CPU-only CMake build: PASS.
- cpu_bus_rom_tests: 1/1 PASS.
- JSON feature manifest: PASS.
- ZIP integrity: PASS.

Still requires Windows/SDL validation of the new browser UX and runtime log resolver.
```

</details>

<details>
<summary>HOTFIX_NOTES_0.66.0.1.txt</summary>

```text
ChaseHQ-Native v0.66.0.1 - Script Console / Regression Hotfix

Fixes
-----
1. Fixed the first regression/preflight failure:
   System.Management.Automation.PSCustomObject does not contain a method named ContainsKey.
   Get-LiveSprites parsed debugger key/value rows into PSCustomObject instances, then incorrectly used ContainsKey(). The code now uses the common Prop accessor for PSCustomObject-safe field access.

2. Script Console library organisation:
   - category filter
   - text search across name/purpose/path
   - grouped OPTGROUP sections by category
   - script metadata preview (category/path/status/checkpoint/requires)
   - selection no longer auto-loads unexpectedly; Load is explicit

3. Diagnostics:
   Runtime query/preflight errors are now labelled Query/preflight failed rather than Compile failed. Parser/compiler errors retain Compile failed.

Scope
-----
This is a hotfix to v0.66.0 only. It intentionally does NOT include the planned legacy-findings roll-up (collision-response toggle, sprite tie-break policy, layer offsets, etc.). That remains for v0.66.1 after this hotfix passes on Windows/SDL.
```

</details>

<details>
<summary>BUILD_NOTES_0.66.9.2-RC1.txt</summary>

```text
ChaseHQ-Native v0.66.9.2-RC1
Baseline: v0.66.9.1 SHA-256 36c40833f4a077fd0545e64b52585ef2c6ef6ed414ae31832092f5150caf6a4

Scope: Stage-1 track-record semantic naming, road/course signature separation, legacytrace evidence isolation, diagnostic/session metadata correction, checkpoint-context clarity, docs/regression consolidation.

No ROMs, SDL dependency tree, evidence sessions or local build output are packaged. Build/qualify on Windows with chqdev.bat qualify --fresh. Zero tolerated CTest failures.
```

</details>

<details>
<summary>BUILD_NOTES_0.66.9.1.txt</summary>

```text
ChaseHQ-Native v0.66.9.1 BUILD NOTES

Baseline: 0.66.9.0-RC3.4.9
Baseline SHA-256: 30ec017edebb9d221dfed16d090ffd6678b645cff6df8c5445a9ef298efe0ebf
Controller: chqdev v0.5.0
Canonical fresh Windows qualification: .\chqdev.bat qualify --fresh
CTest policy: zero tolerated failures; any failure is fatal
Package policy: deterministic exact immutable ZIP identity
Regression policy: focused + Full Regression against exact extraction
Shell policy: PowerShell/BAT release tooling compatibility/fallback only
Research behavior changes: none intended
```

</details>

<details>
<summary>BUILD_NOTES_0.66.9.0.txt</summary>

```text
ChaseHQ-Native 0.66.9.0-RC1
Game-facing research candidate: Game Lab + TC0100SCN RAM character inspector.
Built from proven v0.66.8.0 baseline.
Runtime version: 0.66.9.0
Package label: RC1 until focused Windows/SDL and Full Regression prove-off.

Packaging-environment proof:
- non-SDL native configure/build PASS
- cpu_bus_rom_tests PASS (1/1)
- Workbench JavaScript syntax PASS
- JSON/schema/catalog/static parity PASS
- package-hygiene precheck PASS

First Windows proof: Regression - v0.66.9.0 Game Lab and Tile Inspector.
```

</details>

<details>
<summary>BUILD_NOTES_0.66.9.0-RC3.4.txt</summary>

```text
ChaseHQ-Native v0.66.9.0-RC3.4 - Mobile Pairing / History Parity

- definitive Windows-proven RC3.3 baseline
- offline/local-only QR pairing descriptor and Workbench QR panel
- HttpOnly chq_token capture + Referrer-Policy no-referrer + history-clean /console transition
- authoritative mobile Recent Script Runs/history paging and selected-run detail
- RC3.4 qualification/static/focused regression coverage
- portable proof: six web/static suites PASS; 131/131 script catalog; Linux no-SDL compile PASS; CTest 4/4 PASS
- no intended gameplay/emulation changes
- exact immutable ZIP still requires Windows/SDL + real second-device QR/history + focused RC3.4 + Full Regression promotion proof
```

</details>

<details>
<summary>BUILD_NOTES_0.66.9.0-RC3.4.9.txt</summary>

```text
ChaseHQ-Native 0.66.9.0-RC3.4.9
Baseline: 0.66.9.0-RC3.4.8
Baseline archive SHA-256: ea24ce76ba1555e23b997a0e925ba85a9c489da6a569c04a8e374df661395990
chqdev: v0.4.2

Purpose: permanently remove the historical Windows cpu_bus_rom_tests exception.

Proven diagnostic root causes carried into this clean source candidate:
- sixteen large stack-local chq::Runtime test objects -> Windows stack exhaustion/SEGFAULT
- eight open input streams -> Windows file-in-use remove/remove_all failures

Policy after RC3.4.9: zero tolerated CTest failures. Any CTest failure blocks qualification.

Pre-freeze portable proof:
- Go tests PASS
- 15/15 static suites PASS
- preflight PASS
- 140/140 script catalog
- Linux CTest 4/4 PASS including cpu_bus_rom_tests
- Linux qualify --fresh PASS
```

</details>

<details>
<summary>BUILD_NOTES_0.66.9.0-RC3.4.8.txt</summary>

```text
ChaseHQ-Native 0.66.9.0-RC3.4.8
chqdev v0.4.1 Parameter Defaults + Failure Diagnostics

Baseline:
  0.66.9.0-RC3.4.7
  SHA-256 991cd12e8ac7f05d394017cc6b6d1dd968d4c678f79b36d925ae6ec5aebf085c

Primary fix:
  Go release-script runner now seeds query resolution and compilation from
  # param default metadata. RC3.4.7 failed before RC347_PARAMETER_FRAMES because
  ${frames} remained unresolved despite default=4.

Diagnostics:
  regression failures now retain nested command error, session ID, run ID,
  run path and bundle path through chqdev qualify --fresh.

Canonical Windows prove-off:
  chqdev.bat qualify --fresh

Portable pre-freeze proof:
  chqdev preflight PASS
  Go tests PASS
  Workbench/static suites 14/14 PASS
  script catalog 139/139 exact
  Windows-owned scripts 57 ASCII-safe
  owned JSON 56 PASS before final manifest regeneration
  Linux chqdev qualify --fresh PASS
  Linux CTest 4/4 PASS
```

</details>

<details>
<summary>BUILD_NOTES_0.66.9.0-RC3.4.7.txt</summary>

```text
ChaseHQ-Native v0.66.9.0-RC3.4.7

Baseline: v0.66.9.0-RC3.4.6 exact candidate SHA-256
1971cc38bd700f0a2afc80345ebe88bf4d59c4640bc88a0726b982543a29201f

Primary build/release controller: chqdev v0.4.0
Canonical fresh Windows prove-off: .\chqdev.bat qualify --fresh

RC3.4.7 adds Go-side Script Console regression orchestration and a single fail-closed fresh qualification switch. No intentional emulator/gameplay behavior change.

Portable pre-freeze proof:
- 13/13 Workbench/static suites PASS
- Go tests PASS
- chqdev preflight PASS
- 138/138 exact script catalog parity
- Linux clean Debug CPU-only build PASS
- Linux CTest 4/4 PASS
- deterministic package/verify PASS

Windows acceptance is intentionally one command from a fresh extraction:
  .\chqdev.bat qualify --fresh
```

</details>

<details>
<summary>BUILD_NOTES_0.66.9.0-RC3.4.6.txt</summary>

```text
ChaseHQ-Native v0.66.9.0-RC3.4.6 BUILD NOTES

Scope: chqdev v0.3 process solidification only. No intentional emulator/gameplay/rendering behavior change.

Changes:
- Native Windows Mark-of-the-Web/Zone.Identifier detection/removal in chqdev.
- chqdev preflight auto-repairs project-owned trust state and verifies zero marked files remain.
- chqdev trust / trust --check added.
- chqdev runtime start|stop|restart added as normal lifecycle entry points.
- Runtime lifecycle normalizes trust before launch, invokes the temporary PowerShell Workbench host with -NoProfile -ExecutionPolicy Bypass, then performs readiness proof after start/restart.
- RC3.4.5 deterministic packaging, exact verify, runtime check, regression orchestration and qualification-state behavior retained.
- RC3.4.6 focused regression added to Full Regression.
- All authoritative current documentation updated to state chqdev authority and prohibit manual Unblock-File as a normal process step.

Expected Windows workflow:
  .\chqdev.bat preflight
  .\chqdev.bat build debug
  .\chqdev.bat test debug
  .\chqdev.bat package
  .\chqdev.bat runtime restart
  .\chqdev.bat runtime check
  .\chqdev.bat regression focused
  .\chqdev.bat regression full

Portable pre-freeze proof:
- Go tests PASS
- chqdev preflight PASS
- 12/12 static suites PASS
- 137/137 exact script catalog PASS
- Linux chqdev build debug PASS
- Linux CTest 4/4 PASS
- chqdev qualify --package PASS
```

</details>

<details>
<summary>BUILD_NOTES_0.66.9.0-RC3.4.5.txt</summary>

```text
ChaseHQ-Native v0.66.9.0-RC3.4.5 - chqdev v0.2 authoritative build/release controller

Canonical Windows:
  .\chqdev.bat preflight
  .\chqdev.bat build debug
  .\chqdev.bat test debug
  .\chqdev.bat qualify --package

Canonical Linux:
  ./chqdev.sh preflight
  ./chqdev.sh build debug
  ./chqdev.sh test debug
  ./chqdev.sh qualify --package

Promotion from exact extraction:
  chqdev qualify --build
  start Workbench/runtime from this extraction
  chqdev runtime check
  chqdev regression focused
  chqdev regression full

Packaging is deterministic and performs a second byte-identical package self-test. Identical source bytes on Windows/Linux must yield the same ZIP SHA-256.

PowerShell/BAT qualification is compatibility/fallback only; new release logic belongs in chqdev.
```

</details>

<details>
<summary>BUILD_NOTES_0.66.9.0-RC3.4.4.txt</summary>

```text
ChaseHQ-Native v0.66.9.0-RC3.4.4 - chqdev v0.1 cross-platform controller foundation

Scope:
- No intended gameplay/emulator changes.
- Adds standard-library-only Go chqdev controller source and bundled Windows/Linux amd64/arm64 binaries.
- Commands: doctor, preflight, build, test, package, verify, qualify, status, version; --json supported.
- Windows chqdev stages ROM/SDL, drives CMake and performs direct launcher smoke without PowerShell.
- Linux chqdev v0.1 drives CPU-only build/test.
- Package/verify owns manifest, ZIP, SHA-256 and independent exact extraction verification.
- Legacy Windows qualification remains mandatory as a parity gate in RC3.4.4.
- All RC3.4.1-RC3.4.3 ASCII/LF/CRLF/CR/handover/document safeguards remain mandatory.
```

</details>

<details>
<summary>BUILD_NOTES_0.66.9.0-RC3.4.3.txt</summary>

```text
ChaseHQ-Native v0.66.9.0-RC3.4.3 - Mobile Pairing / History Parity + Release Guard + Handover Contract + Newline Matrix

- based on the locally Windows-proven RC3.4.2 working tree after the exact newline-normalisation repair
- RC3.4 QR/mobile-history behavior unchanged
- RC3.4.1 Windows-script ASCII/U+00B7 fail-closed policy carried forward
- RC3.4.2 generic version-derived ChatGPT handover contract carried forward
- line-oriented release/document contract validation now normalizes CRLF and CR to LF before regex matching
- Windows PowerShell self-test proves valid LF, CRLF and CR fixtures plus missing-section/stale-version negative fixtures
- independent tests/workbench_rc343_tests.js proves the same newline matrix against the frozen tree
- current documentation matrix is regression-checked for v0.66.9.0-RC3.4.3; release-guard docs must retain the LF/CRLF rule
- exact 1:1 script-catalog table parity retained
- RC3.4.3 canonical qualifier/static/focused/Full Regression wiring added
- no intended gameplay/emulation or QR/mobile-history behavior changes
- portable proof: 9/9 static suites PASS; 50 JSON PASS; 134/134 exact script catalog rows; 48 Windows-script ASCII PASS; 13/13 current docs PASS; LF/CRLF/CR contract guard PASS; 10 JS syntax PASS; Linux no-SDL compile PASS; CTest 4/4 PASS
```

</details>

<details>
<summary>BUILD_NOTES_0.66.9.0-RC3.4.2.txt</summary>

```text
ChaseHQ-Native v0.66.9.0-RC3.4.2 - Mobile Pairing / History Parity + Release Guard + Handover Contract

- based on the locally Windows-proven RC3.4.1 source tree
- RC3.4 QR/mobile-history behavior carried forward unchanged
- RC3.4.1 Windows-script ASCII/U+00B7 fail-closed policy carried forward unchanged
- locally proven handover repair incorporated
- generic Test-ChatGptHandoverContract derives current version from src/version.h
- handover contract requires current v<version>, ## Authoritative continuation and ## Immediate next task
- validator positive/negative handover self-test runs on every source audit
- independent portable RC3.4.2 handover-contract regression
- RC3.4.2 qualifier/static/focused/Full Regression wiring
- no intended gameplay/emulation changes

- portable proof: 8 static suites PASS; 48 JSON PASS; 133/133 exact script catalog rows; Windows-script ASCII PASS; frozen handover contract PASS; JavaScript syntax PASS; Linux no-SDL compile PASS; CTest 4/4 PASS
```

</details>

<details>
<summary>BUILD_NOTES_0.66.9.0-RC3.4.1.txt</summary>

```text
ChaseHQ-Native v0.66.9.0-RC3.4.1 - Mobile Pairing / History Parity + Release Guard

- RC3.4 feature set carried forward unchanged
- exact seven-U+00B7 Workbench ASCII repair incorporated
- all project-owned Windows script surfaces now fail closed on non-ASCII after optional UTF-8 BOM
- U+00B7 policy negative self-test runs in every Windows preflight
- independent portable final-tree ASCII regression
- intentional qualifier fixture shown as EXPECTED FAIL, not ordinary FAIL
- full child stdout/stderr retained for actionable qualification failures
- RC3.4.1 qualifier/static/focused regression coverage
- no intended gameplay/emulation changes
- portable proof: seven static suites PASS; 48 JSON PASS; 132/132 script catalog coverage; final-tree Windows-script ASCII scan PASS; Linux no-SDL compile PASS; CTest 4/4 PASS
```

</details>

<details>
<summary>BUILD_NOTES_0.66.9.0-RC3.3.txt</summary>

```text
ChaseHQ-Native v0.66.9.0-RC3.3 - Definitive Release Qualification / Immutable Provenance

Purpose: permanently harden the release process after the RC3.2 prove-off. No gameplay/emulation semantic change is intended.

Key changes:
- one user-facing qualification path: Qualify-RC3.3.bat -> tools/Invoke-RC33Qualification.ps1
- fail-closed child-gate propagation plus negative self-test
- authoritative manifest-only packaging and independent exact-archive hash verification
- candidate SHA emitted only after archive verification
- machine-readable qualification.json tied to candidate ID and exact ZIP SHA-256
- permanent RC3.3 whole-release regression wired into Full Regression
- strongly worded NON-NEGOTIABLE release process; mutable/repacked candidates cannot inherit qualification
- Windows promotion remains blocked until clean exact-archive build, READY launcher/runtime smoke, RC3.3 regression, authenticated second-client /console proof, and Full Regression pass.

Definitive controller hardening: root BAT launcher with ExecutionPolicy Bypass; exact RC3.3 script-catalog row; relative ProjectRoot/default OutputRoot regression guard; stale LASTEXITCODE reset; post-build PENDING/HANDOFF semantics.
```

</details>

<details>
<summary>BUILD_NOTES_0.66.9.0-RC3.2.txt</summary>

```text
ChaseHQ-Native 0.66.9.0-RC3.2 - Script Parameters / Evidence Closure / LAN Console

Scope:
- carry RC3.1 mapping/object/signed16 fixes forward unchanged;
- parameterized .chqscript metadata + Workbench/API parity;
- trace/checkpoint run-bundle evidence closure;
- opt-in token-authenticated LAN /console companion;
- Stage-1 type-6 material-path knowledge capture.

Portable gates:
- Linux no-SDL configure/build;
- CTest;
- Node Workbench static tests including RC3.2-specific checks;
- JSON parse/schema sanity;
- package manifest/hash verification.

Windows promotion still requires Build-Debug.bat, Workbench parameter + LAN smoke tests, RC3.2 regression, focused Stage-1 survey and Full Regression on the exact archive.

## Checkpoint packaging note

The archive carries the eight canonical checkpoint binaries inherited from RC3.1. The newer October 2 `stage1-page4-material-pre-4704` and `stage1-selector5-page0-8664` states are not included because their real raw bytes were not available to this packaging environment; no placeholders are fabricated. See `docs/RC3.2_CHECKPOINT_IMPORT_STATUS.md`.

Corrected replacement package (2026-10-02):
- fixes Windows Prepare-Project/Validate-Workbench strict-mode crash caused by a validator search literal expanding undefined `$prev`;
- the search now uses a literal single-quoted PowerShell string;
- Validate-Release carries a permanent guard against reintroducing the unsafe interpolated form.
- Any earlier RC3.2 archive/hash is withdrawn.

WINDOWS RELEASE-PREFLIGHT HARDENING (corrected RC3.2 archive)
- Start-ChaseHQWeb.ps1 is byte-ASCII again for Windows PowerShell 5.1; RC3.2 mobile/parameter UI punctuation uses ASCII equivalents.
- Added Validate-WindowsPreflight.ps1 and wired it before Validate-Workbench.ps1 in Prepare-Project.ps1.
- The preflight aggregates PowerShell parser errors, Workbench/BAT encoding hazards, automatic-variable collisions, build-gate contracts, RC3.2 version identity, LAN plumbing and script parameter/evidence plumbing before the deeper validator runs.
- Validate-Workbench.ps1 now validates RC3.2 parameter/evidence/LAN surfaces in addition to the carried-forward RC3.1 mapping-quality checks.
- Validate-Release.ps1 permanently checks the aggregate Windows preflight wiring and Workbench ASCII safety.
- The earlier RC3.2 archives with SHA-256 aee39dd9df4ec57b5812a79b080d9e69e5c2d0833d11528ce73bc0715a93b9ba and c1636f0cd879a87d4fb7b72a242c5704f1a6dd1d58276e24af10a751833f0619 are withdrawn.

WINDOWS PREFLIGHT FALSE-POSITIVE CORRECTION
- Replaced the broad automatic-variable blacklist with runtime Constant/ReadOnly detection using ScopedItemOptions.
- Explicitly permits the canonical PowerShell `$null = ...` output-discard pattern.
- Added a preflight rule self-test: `$null` must be writable; Host/PID/true/false must be protected; an ordinary probe variable must be writable.
- Removed the duplicate reserved-variable implementation from Validate-Workbench.ps1 so the rule has one owner.
- Validate-Release.ps1 now executes the same Windows preflight before Workbench validation.
- Prepare-Project.ps1 now runs source preflight before ROM/SDL staging and CMake, so package faults fail immediately.
Previous RC3.2 aggregate-preflight candidate SHA-256 `58aafb2f6c70e772853e543b81683e8d0b8f2b7f8c2a3dbc29f4d588d773ebf6`: WITHDRAWN after the `$null` false-positive was proven on Windows.

RC3.2 LAN ACL correction: the elevated prerequisite phase must never use an elevated HttpListener bind as evidence that the normal user owns the URL namespace. It now ensures and verifies the exact `http://<resolved-lan-ip>:37680/` URL ACL for the original user, then the parent non-elevated process performs the final bind proof.
```

</details>

<details>
<summary>BUILD_NOTES_0.66.9.0-RC3.1.txt</summary>

```text
ChaseHQ-Native 0.66.9.0-RC3.1 - Course Mapping Survey Quality / Object Tracking

Baseline:
- Fully Windows/SDL-proven baseline remains RC2.9.
- Final RC3.0 focused mapping backend + Track View proof passed on Windows.
- RC3.0 was not promoted before survey-data quality refinements; all final RC3.0 fixes are carried here.

Build intent:
- Prevent volatile TC0150 renderer/perspective words from being counted as separate road materials.
- Preserve those raw renderer words for later material reverse engineering.
- Preserve raw sprite census while adding conservative temporal candidate tracking without unsafe proximity-merging of repeated scenery.
- Avoid long 600/3600-frame survey reruns caused by missing evidence.

Required carried-forward fixes:
- Set-CourseFollowConfig($config), never $args.
- Track tab route = Course Mapping / Live Survey.
- Pause/Break + Ctrl+P SDL pause.
- RC3.0 live course follow/survey API/UI/script parity.

New data outputs:
- Track Recorder v3 quality counters.
- chq-course-map-v2.
- track-map-summary.json.
- track-object-candidates.csv.
- track-surface-segments.csv.
- Stable course-surface candidates separated from raw renderer variants.

Windows prove-off order:
1. .\Build-Debug.bat
2. regression/regression-v06690-rc31-course-mapping-quality.chqscript
3. Inspect Track View + 3-sample mapping summary.
4. track/surveys/stage1-course-mapping-shakedown-600.chqscript
5. Inspect bundle quality before any 3600-frame survey.
6. Full Regression.

Corrected candidate note:
- Withdraw first RC3.1 package after Windows focused regression exposed `Cannot overwrite variable PID because it is read-only or constant.` on the second track.record.sample.
- Root cause: Assign-TrackObjectIds used local `$pid`; PowerShell resolves variable names case-insensitively and `$PID` is read-only.
- Fix: rename local to `$previousTrackId`.
- Add Validate-Workbench + Node static guards rejecting `$pid =` assignments in Start-ChaseHQWeb.ps1; AST parsing alone does not catch this runtime automatic-variable collision.
- Carry forward all prior RC3.0/RC3.1 fixes unchanged.

Object-candidate correction / PowerShell hardening:
- The first complete RC3.1 focused regression exposed 131 raw sprite observations but only 3 candidate rows, with candidate evidence fields null.
- Add Get-TrackObjectCandidates and explicit array capture/binding between sprite candidate generation and temporal tracking.
- Build tracked candidates explicitly so map/palette/source slot/bounds/centre/visible-pixel evidence survives PowerShell object adaptation.
- Add runtime map-export coverage + evidence invariants and expose candidate coverage metrics.
- Add tests/workbench_object_candidate_tests.ps1 to execute the actual packaged helper functions with mock multi-sprite samples.
- Add AST-based PowerShell automatic/read-only/reserved-name collision validation across every packaged .ps1.
- Rename existing unsafe locals using $args, $matches/$Matches and $profile/$PROFILE where they were being used as ordinary variables.
- Preserve all RC3.0 fixes, the surface-stability fix, Ctrl+P, Track View routing, $config bridge and $previousTrackId correction.
Cumulative local-proveoff consolidation:
- Backfill surfaceSegmentsCsv export path key; keep numeric surfaceSegments count.
- Prop() supports production OrderedDictionary/IDictionary rows case-insensitively.
- Prevent vacuous object-candidate coverage and test production-shape inputs.
- Suppress duplicate same-frame Track Recorder samples; Track View does not auto-sample while Script Console is running.
- Rename Start-ChaseHQResearch local emulator argument accumulator to $nativeLaunchArgs so it does not alias the EmulatorArgs parameter case-insensitively and append the base block to itself.
- Local Windows proof before repackaging: Build-Debug PASS with sole historical cpu_bus_rom_tests SEGFAULT; focused RC3.1 PASS; 600-frame shakedown PASS with exactly 41 unique 15-frame samples and full candidate coverage.


Long-survey signed arithmetic correction:
- First 3600-frame run reached frame 3399 with 89 unique ON_ROAD samples, then failed when `[int16]` was asked to convert masked 0xFEF7 / 65271.
- Get-LiveTrackState now uses Signed16() for road delta and lateral-error two's-complement reinterpretation.
- Added tests/workbench_track_signed16_tests.ps1 plus Validate-Workbench/Node guards against direct masked [int16] casts.
```

</details>

<details>
<summary>BUILD_NOTES_0.66.9.0-RC3.0.txt</summary>

```text
ChaseHQ-Native v0.66.9.0-RC3.0
Course Mapping / Live Survey tooling candidate

Baseline: Windows/SDL-proven RC2.9.
Scope: live course-follower API/UI/script parity; lateral-bias branch mapping; Track Recorder v2; player trajectory; target/dynamic-object evidence; raw surface signatures; reconstructable mapping exports; survey scripts and permanent regression.

ROMs and SDL are not bundled. Existing Windows build-local staging remains authoritative.

Candidate correction: fixed Workbench Track View tab routing after the panel rename from "Live SVG Track View" to "Course Mapping / Live Survey". Backend course mapping was proven by the focused Windows regression; the blank tab was a frontend title-routing mismatch. Validate-Workbench now guards the exact mapping.

Replacement-candidate correction: the Track View routing fix and the PowerShell live course configuration fix are now present together. A packaging merge had restored `Set-CourseFollowConfig($args)` in the prior candidate, causing `course.follow.configure` to fail because PowerShell treated `$args` as System.Object[]. The bridge now uses `Set-CourseFollowConfig($config)`, and Validate-Workbench guards both this contract and the Course Mapping panel route. Archive hash 6b209d84945a64f6d33435926a0fdb4a8fbcab7f86e60b4b04b878f8a00f56b1 is withdrawn.
```

</details>

<details>
<summary>BUILD_NOTES_0.66.9.0-RC2.txt</summary>

```text
ChaseHQ-Native v0.66.9.0-RC2
Focused sprite-order / Game Lab semantics candidate.

Highlights:
- sprite.map.inspect map=N fixed
- sprite.order.inspect / sprite.order.set
- Game Lab sprite-order card
- HUD display speed vs raw internal/physics speed clarified
- Game Lab mutations preserve prior run/pause state
- tile.inspect outputs include frame number
- unhandled native ERR fails scripts
- reusable turbo animation and car-shadow/order scripts bundled
- documentation/schema/knowledge synchronized
```

</details>

<details>
<summary>BUILD_NOTES_0.66.9.0-RC2.9.txt</summary>

```text
ChaseHQ-Native v0.66.9.0-RC2.9

Baseline: v0.66.9.0-RC2.8 (Windows/SDL-proven before RC2.9 changes).

Build locally on Windows with Build-Debug.bat. Prepare-Project.bat stages SDL/ROM paths from build-local.json. ROMs are not bundled.

RC2.9 changes native/runtime code (one-hit target and SDL Pause/Break), Workbench/API, canonical checkpoints, regressions and documentation. Linux/static validation cannot prove SDL/PowerShell behavior; run the focused RC2.9 regression and Full Regression after Windows build.

Replacement archive correction:
- Updated Validate-Workbench.ps1 from stale 300000 ms signature to the RC2.9 900000 ms progress-aware contract.
- Validator now checks the 3700 s / 3700000 ms long-operation transport budgets and explicit RC2.9 feature/documentation parity.
- Earlier pre-proof RC2.9 archive/hash withdrawn.

Windows build-gate correction after first local CTest run:
- tests/cli_evidence_tests.ps1 now asserts evidence build identity 0.66.9.0-RC2.9 instead of stale RC2.7.
- Build-Debug.bat and Build-Release.bat now capture findstr ERRORLEVEL with delayed !ERRORLEVEL! inside the parenthesized failure block.
- The historical cpu_bus_rom_tests SEGFAULT is tolerated only when CTest reports exactly one failed test; any additional failing test now returns a non-zero build status.
- Validate-Release.ps1 now guards these release-gate contracts so the same regression is blocked before packaging.
```

</details>

<details>
<summary>BUILD_NOTES_0.66.9.0-RC2.8.txt</summary>

```text
ChaseHQ-Native v0.66.9.0-RC2.8 candidate

Based on Windows-proven, saturated v0.66.9.0-RC2.7 Run 009.

Changes:
- TC0100SCN normal text source X origin +23 -> +16.
- Separate flipped-text coordinate path with tests.
- Independent sprite 4bpp SHADOW decoder and COMPARE utility; diagnostic parity only.
- RC2.8 regression captures Stage 1 driving and end-results/radar/HUD checkpoints.
- 0x1002E8 classified as end-results/intermission gate; 0x1002AE removed as a health claim.
- Responsive Workbench layout, selected-run Open in Evidence action and data-driven Next Steps.
- Native version reporting updated to RC2.8.

Local Windows Build-Debug.bat stages SDL and ROMs through Prepare-Project using build-local.json. Run that exact script for Windows/SDL proof.
```

</details>

<details>
<summary>BUILD_NOTES_0.66.9.0-RC2.7.txt</summary>

```text
RC2.7 source candidate; Windows/SDL proof pending.
Preserve build-local.json. Build-Debug.bat invokes Prepare-Project.ps1 to copy local SDL and Chase H.Q. ROMs, configures Windows/SDL, compiles and runs CTest (including new history and CLI evidence tests).
Run: .\Build-Debug.bat
Then: .\Start-ChaseHQ.ps1 -Restart
Run the focused RC2.7 regression, RC2.6.1 sprite regression and Full Regression in Workbench. Review old/corrected canonical captures before promotion.
See VALIDATION_0.66.9.0-RC2.7.md for actual Linux results and remaining local gates.
```

</details>

<details>
<summary>BUILD_NOTES_0.66.9.0-RC2.4.txt</summary>

```text
ChaseHQ-Native v0.66.9.0-RC2.4

Evidence-driven TC0100SCN vertical-scroll correction.

Corrected:
- BG0/BG1/TEXT source-Y now applies TC0100SCN control Y with the same negative-scroll convention used by the reference device model.
- Historical BG0=-4 / BG1=-20 default presentation offsets are removed; both backgrounds now default to neutral 0:0.
- TC0100SCN geometry trace uses the same corrected source-Y helper as the live renderer.
- Added native regression checks for wrapped source-Y arithmetic, including the proven CTRL3/4=0x01E0 case.

Evidence:
- RC2.3 frame 2065 A/B proof showed that forcing BG0 and BG1 to Y=-64 relocates both building planes from the road/foreground to the horizon.
- At CTRL3=CTRL4=0x01E0, RC2.3's old formula plus a -64 presentation override is mathematically equivalent to the corrected formula with neutral presentation offset: visible-top hardware Y 16 resolves to source Y 40.
- The corrected source coordinate matches MAME's TC0100SCN convention where BG/FG Y scroll is stored as -control and applied directly to the tilemap.

No API or .chqscript grammar changes. Existing --layer-offset remains available as a presentation-only forensic override.

Local static/unit validation:
- CMake CPU-only configure/build: PASS
- ctest cpu_bus_rom_tests: PASS (1/1)

Windows/SDL proof required:
1. .\Build-Debug.bat
2. Run the canonical stage1-gameplay checkpoint with no BG layer offsets and capture frame 2065.
3. Confirm the resulting frame matches the prior RC2.3 both-sign-proxy geometry.
4. Run focused regression, then Full Regression only after the visual proof passes.

Focused Windows visual gate: `./Validate-TC0100SCN-Y.ps1` captures canonical frame 2065 with neutral defaults and requires byte-exact equality with the packaged RC2.3 both-sign-proxy reference.
```

</details>

<details>
<summary>BUILD_NOTES_0.66.9.0-RC2.2.txt</summary>

```text
ChaseHQ-Native v0.66.9.0-RC2.2

Focused renderer correction candidate.
- Default same-priority sprite ordering changed to lower-slot-wins.
- Basis: MAME Chase H.Q. chasehq_draw_sprites_16x16 traverses sprite RAM high-to-low.
- Higher-slot remains available as diagnostic override.
- Adds loop-based multi-scene validation and focused regression.
- Documentation/knowledge/schema updated together.
```

</details>

<details>
<summary>BUILD_NOTES_0.66.8.0.txt</summary>

```text
ChaseHQ-Native v0.66.8.0 - Forensic Timeline v1
STATUS: PROVEN RELEASE (2026-09-29)

Promoted from the proven RC7.4 candidate with no runtime/emulator semantic changes during promotion.

Highlights:
- deterministic per-frame forensic timeline recording plus complete bus-write provenance;
- import/seek/read inspection parity and recorded-state SDL playback;
- fork-live counterfactual continuation;
- immutable source timelines with portable current-run derived analysis;
- streaming candidate/writer analysis, automatic disassembly and HUD/graphics correlation;
- legacy first-turbo-capture compatibility;
- progress-aware long recording waits;
- script autoload/startup diagnostics corrections;
- bounded run-history discovery/browser output and summary-first artifact presentation;
- RC-aware history version filtering and PASS/FAIL/CANCELLED filtering.

Final proof:
- Regression - v0.66.8.0 RC7.4 Bounded History Discovery: PASS (1.071 s).
- Full Regression Suite: PASS (78.422 s), Windows/SDL, 2026-09-29.

Known limitation: authentic audio remains unavailable while the runtime uses sound_stub.

See RELEASE_NOTES_0.66.8.0.md and docs/FORENSIC_TIMELINE.md.
```

</details>

<details>
<summary>BUILD_NOTES_0.66.8.0-RC7.txt</summary>

```text
ChaseHQ-Native v0.66.8.0-RC7 — Portable Automated Timeline Analysis

Purpose
-------
Turn the now-proven offline timeline recovery into a portable, unattended diagnosis workflow.

Key changes
-----------
- timeline.scan.memory writes derived evidence into the active run artifact tree, leaving source timelines immutable.
- source.json hashes the source manifest and writes.csv for provenance.
- improved temporal scoring and known turbo reference tagging.
- timeline.analyze.auto ranks candidates, groups/disassembles writer PCs, captures historical before/active/mid/expiry/after state + layered snapshots, and correlates HUD_TURBO in normal/HUD-only/HUD-hidden modes.
- timeline.play replays historical frame checkpoints through SDL at a requested frame rate; this is recorded-state playback, not CPU re-execution.
- new automated-analysis, SDL replay and permanent RC7 regression scripts.

Proof order
-----------
1. Build-Debug.bat
2. Start-ChaseHQ.ps1 -Restart
3. Regression - v0.66.8.0 RC7 Portable Automated Timeline Analysis
4. Analyze Existing Turbo Forensic Timeline
5. Full Regression Suite once if focused proof is clean.
```

</details>

<details>
<summary>BUILD_NOTES_0.66.7.txt</summary>

```text
ChaseHQ-Native v0.66.7 — Workbench Knowledge / Documentation Foundation

Base: v0.66.6.1 proven.

Adds:
- Workbench Docs / Knowledge tab backed by research/knowledge/documentation.json.
- docs.* / knowledge.* API and script actions.
- HTML/PDF documentation export with evidence modes.
- Curated documentation evidence and provenance model.
- docs/CHATGPT_HANDOVER_PROMPT.md authoritative continuation prompt.
- Graphics Lab individual sprite forensic mode using cropped transparent sprite assets.
- SDL window control parity in Workbench/API/scripts: status, fullscreen, scale, show/hide, minimize/restore, always-on-top; pause/resume/step/snapshot shortcuts.
- Updated schemas, references, changelogs, project state, handover, roadmap and regression.

Validation state: static/non-SDL validation performed in packaging environment; Windows SDL runtime proof required.
```

</details>

<details>
<summary>BUILD_NOTES_0.66.7.9.txt</summary>

```text
ChaseHQ-Native v0.66.7.9 - Research Recovery / Release Discipline

Purpose
-------
This revision deliberately stops expanding platform tooling and turns the v0.66.7.6-v0.66.7.8 prove-off failures into explicit release rules, validation guards and reusable research scripts. Runtime/emulator semantics are carried forward from v0.66.7.8.

Key changes
-----------
- Canonical docs/RELEASE_PROCESS.md with staged build/launch/fast-gate/focused-regression/full-regression order.
- Post-package tools/Test-ReleaseArchive.ps1 validator.
- Corrected turbo/HUD knowledge: gameplay state is confirmed; HUD rendering path remains open.
- New recovery, turbo/HUD, checkpoint survey and target/end-level .chqscript workflows.
- Regenerated Script Catalog from packaged headers.
- New permanent v0.66.7.9 research-recovery regression wired into Full Regression.
- Developer handoff package keeps build-local.json but excludes SDL, out, ROMs and transient evidence.

Immediate research after proof
------------------------------
Run turbo-state-causal-validation, then turbo-hud-before-during-after. Use turbo-state-writer-trace only after the visual source path is localized. Then move to target/end-level state survey and track topology.
```

</details>

<details>
<summary>BUILD_NOTES_0.66.7.8.txt</summary>

```text
ChaseHQ-Native v0.66.7.8 RC2

- Focused discovery/performance regression proven on Windows.
- Permanent fix for missing closing brace in Start-ChaseHQWeb.ps1 / Get-StructuredFrameSnapshotRoots().
- Launcher health metadata synchronized to build 0.66.7.8.
- Cross-version run history and completed-run snapshot discovery retained.
- Full SDL source tree is not bundled; build-local.json points at local SDL/ROM dependencies.
- Remaining release gate: Full Regression Suite against this RC2 package.

RC3: defer script-run ZIP creation until one PASS/FAIL/CANCELLED finalization; transient preflight/predicate requests no longer create incidental runs. Added regression-v06678-run-finalization.chqscript and Full Regression coverage.
```

</details>

<details>
<summary>BUILD_NOTES_0.66.7.6.txt</summary>

```text
ChaseHQ-Native v0.66.7.6 - Docs / Knowledge Consolidation Release Candidate

Purpose
-------
Consolidate the locally proven v0.66.7.5 handbook/search/export fixes into one clean release candidate and add release-hardening validation before returning to gameplay research.

Highlights
----------
* Project Knowledge articles contain discovery/significance/confirmed-values/method/reproduction/open-question sections.
* Unified UI search and docs.search/knowledge.search index article-body text.
* HTML/PDF export path renders article bodies and omits empty reference sections.
* Project Knowledge landing groups clickable entries by proof/status.
* Full ChatGPT handover remains visible/copyable/downloadable in Workbench.
* Workbench source is kept ASCII-safe for Windows PowerShell 5.1.
* Release validation checks article/schema integrity, search/export wiring, single handbook handler, mojibake sentinels and backup-file exclusion.
* No emulator/gameplay semantics and no Research API action names or .chqscript grammar changed.

Proof status
------------
Release candidate assembled from the locally tested v0.66.7.5 tree. Windows native build, SDL controls, Graphics Lab and full regression proof are still required.
Run: Regression - v0.66.7.6 Docs / Knowledge Consolidation
```

</details>

<details>
<summary>BUILD_NOTES_0.66.7.5.txt</summary>

```text
ChaseHQ-Native v0.66.7.5 — User Handbook / Live API & Scripting Reference

Purpose
-------
Turn the v0.66.7.x Docs / Knowledge foundation into actual user-facing documentation rather than a raw knowledge-object browser.

Highlights
----------
* Docs / Knowledge now has five first-class views: Handbook, API Reference, Scripting Reference, Project Knowledge, Glossary.
* New research/knowledge/handbook.json is the authored manual/navigation source.
* Getting Started, Workbench guides and task-oriented research recipes are included.
* Complete API reference is generated from the authoritative action schema at runtime (132 actions in this package).
* Complete .chqscript reference is generated from the authoritative script-language schema.
* Search spans handbook pages, API actions, script constructs and project knowledge.
* Code/API/script examples have Copy controls.
* Project findings remain evidence-backed and can link to API actions, scripts, checkpoints and screenshots.
* docs.handbook and docs.page are available through the API and existing .chqscript `api ...` statement.
* Full HTML/PDF export now combines handbook + API reference + scripting reference + project knowledge/glossary + selected evidence.
* The standalone packaged docs/handbook/ChaseHQ-Native-Documentation-Handbook.html is regenerated from the same data sources.
* v0.66.7.4 SDL controls, per-sprite Graphics Lab controls, evidence integration and ChatGPT handover remain carried forward unchanged.

Proof status
------------
Implemented and statically validated in the build environment. Windows/SDL/Workbench proof is still required.
Run: Regression - v0.66.7.5 User Handbook / Live Reference
```

</details>

<details>
<summary>BUILD_NOTES_0.66.6.1.txt</summary>

```text
Chase H.Q. Native v0.66.6.1 - Layered Frame Forensics reconstruction hotfix

Hotfix validation change:
- Normalises exported compositor finals to opaque RGBA, matching the effective SDL framebuffer.
- Encodes every changed reconstruction pixel as an opaque replacement colour, so alpha-over can represent transitions that previously ended as RGBA 0,0,0,0.
- Self-verifies reconstruction in the native capture path. frame.snapshot now fails if any reconstructed pixel differs from final.png.
- reconstruction.json records verifiedExact and mismatchedPixels.
- Regression/library/docs/contracts remain aligned with the 0.66.6.1 build.

ChaseHQ-Native v0.66.6.1 - Layered Frame Forensics + HUD Isolation
=================================================================

Baseline
--------
Built from proven v0.66.5.1 Track Recorder/Scripting Hotfix.

Delivered
---------
- Native `graphics snapshot PATH` command and `graphics.layered_snapshot` capability.
- Structured frame snapshot schema v2.
- `final.png`, `final-no-hud.png`, and `hud-only.png` per snapshot.
- Exact reconstruction contribution PNGs in compositor order under `layers/`.
- Opaque `layers/00_base.png` is included as the first reconstruction stage so reconstruction does not rely on an assumed viewer background.
- Transparent raw forensic source PNGs under `sources/` for bottom background, road, upper background, sprites and TC0100SCN text/HUD.
- `reconstruction.json` records compositor selection, layer order, layer offsets, PROM input mask, tile-control state, source-region descriptions, HUD model and palette-state path.
- Complete live TC0110PCR `palette.csv` with raw value and write provenance fields.
- Source-aware HUD isolation: normal / hidden / only. HUD hiding suppresses the TC0100SCN text source; it does not crop or apply a fixed screen mask.
- `frame.snapshot` now creates the v2 reconstructable snapshot format.
- `image.compare` and `image.diff` accept `hud=normal|hidden|only` (`exclude` is accepted as a hidden alias internally).
- HTTP snapshot manifest and asset endpoints for the Graphics Lab viewer.
- Graphics Lab layered snapshot viewer with normal / HUD hidden / HUD only / reconstruction modes, per-layer show/hide, solo and opacity controls, plus raw text/HUD source access.
- v0.66.6.1 regression script covering snapshot creation/listing and all HUD compare modes.
- Full regression suite includes the v0.66.6.1 regression.

Documentation/contracts updated in the same build
-------------------------------------------------
- docs/API_REFERENCE.md
- docs/API_CHANGELOG.md
- docs/SCRIPT_LANGUAGE_REFERENCE.md
- docs/SCRIPT_CHANGELOG.md
- docs/ROADMAP.md
- docs/PROJECT_STATE.md
- docs/HANDOVER.md
- docs/NEXT_BUILD_CHANGELOG.md
- research/scripts/graphics/README.md
- research/schema/api-actions.json
- research/schema/script-language.json
- research/feature-manifest.json

Important HUD semantics
-----------------------
`hud-only.png` is the pixel contribution/influence caused by enabling the TC0100SCN text/HUD source relative to the otherwise-identical compositor pass with that source disabled. `sources/text-hud.png` is the independent raw text/HUD source render.

Required dev-machine proof
--------------------------
The package environment used to prepare this build does not contain SDL3 source or PowerShell, so the Windows SDL target must still be compiled on the normal dev machine and the bundled v0.66.6.1 regression must be run there before this version is considered fully proven.
```

</details>

<details>
<summary>BUILD_NOTES_0.66.5.txt</summary>

```text
ChaseHQ-Native v0.66.5 - Memory Trace / Track / Workbench Polish

- Generic memory-write trace API + Research Script integration.
- Track View auto-fit, Fit Track, and Save SVG.
- Removed Script Console Syntax Preview.
- Build scripts retain Prepare-Project.bat unblocking/bootstrap and whitelist only the historical cpu_bus_rom_tests SEGFAULT signature.
- Restart cleanup avoids PID 4/System and cleans stale Workbench hosts.
- Carries forward v0.66.4 palette-write tracing and run reliability fixes.
```

</details>

<details>
<summary>BUILD_NOTES_0.66.4.txt</summary>

```text
ChaseHQ-Native v0.66.4 - Palette Trace + Release Reliability

Highlights
- Adds first-class TC0110PCR palette-write tracing with frame, CPU, PC, palette index/bank/pen, old/new raw values, changed flag and write count.
- Adds Script API actions: palette.trace.start/status/tail/stop/clear.
- Adds v0.66.4 palette-trace regression and includes it in Full Regression.
- Fixes per-run duration metadata so multi-command Script Console runs retain the first command's start time instead of overwriting duration on every command request.
- Adds descriptive run-bundle download names while keeping the short physical bundle.zip inside each run folder.
- Carries forward the full console transcript dictionary-key hotfix from the validated v0.66.3.1 working tree.
- Adds Validate-Release.ps1 as the packaging gate and updates API/Script documentation.

Immediate Chase H.Q. use
Trace palette indices 1037 (bank 64 pen 13) and 1053 (bank 65 pen 13) around the Stage 1 brake-lamp transition, then disassemble the exact writer PC(s).
```

</details>

<details>
<summary>BUILD_NOTES_0.66.3.txt</summary>

```text
ChaseHQ-Native v0.66.3 - Research Contracts, Run Artifacts & Sprite Provenance

- Authoritative API/script schema docs and introspection (`schema.all`, `script.schema`).
- Per-script-run folder: exact source, console output, artifacts, metadata, bundle manifest and automatic ZIP.
- `sprite.map.inspect` exposes spritemap offsets/chunk tile codes and raw pen histogram.
- `ui.prompt` renders a research prompt over SDL with Enter/Escape completion, optional pause and Always On Top.
- Carries forward v0.66.2 steering/driving controls and canonical checkpoints.
```

</details>

<details>
<summary>BUILD_NOTES_0.66.3.1.txt</summary>

```text
ChaseHQ-Native v0.66.3.1 - Workbench Reliability & Run History Hotfix

Scope is deliberately bounded: restore a dependable research workflow, then return to Chase H.Q. game/render investigation.

Fixes:
- corrected full-regression v0.66.3 include ordering/path
- restart port-owner fallback
- compact session/evidence/runs layout (no web-evidence)
- sequential per-session run IDs and automatic bundle.zip
- explicit run build/session/name/purpose/source provenance
- pasted script metadata support (# name/# purpose/# category)
- selected-row shared history actions
- no folder-regex version inference for new history; malformed legacy versions display unknown
- script include and metadata validation
- carried forward API/script schema documentation discipline

Gameplay/research handover:
- do not expand tooling unless it blocks the active investigation
- investigate/remediate before rebuilding or packaging
- immediate target remains map 516 raw spritemap/OBJ provenance for the confirmed 140 pen-13 rear-window pixels
```

</details>

<details>
<summary>BUILD_NOTES_0.66.2.txt</summary>

```text
ChaseHQ-Native v0.66.2 — Player Steering & Driving Controls

- Confirmed authoritative steering chain: IOC ports 0x0C/0x0D -> CPU-A 0x10014C raw -> CPU-A 0x100300 processed -> handling/cornering -> 0x10A044 lateral position.
- SDL gameplay keys: Left/Right steer at proven ±96 raw values; Up accelerator; Down brake; Space turbo. All use IOC input rather than RAM patches.
- Workbench browser gamepad live steering toggle uses axis 0 with deadzone and authentic signed-12-bit IOC encoding.
- Gameplay Registry now exposes raw and processed steering as CONFIRMED.
- Permanent v0.66.2 steering regression included and added to full regression.
- Web command dispatcher records and retries one intermittent empty native response once, preserving RECOVERED/RETRY_FAILED telemetry.
- Carries forward v0.66.1.2 regression/resilience fixes and build-local staging.
```

</details>

<details>
<summary>BUILD_NOTES_0.66.1.txt</summary>

```text
# ChaseHQ-Native v0.66.1 — Legacy Findings / API / Workbench Roll-up

This build promotes existing Chase H.Q. discoveries into discoverable, regression-tested tooling without changing the primary emulator target.

## Added
- Collision response state/control API. Disabling response suppresses only proven lateral/speed response writes; detection/event logic remains live.
- Steering injection API is explicitly distinguished from unresolved authoritative game-side steering state.
- Sprite structured records now report the hardware priority source (`sprite word1 bit15`) and draw-order/slot provenance.
- Expanded semantic-object registry for player/target/turbo/brake-lamp/road-edge/collision research.
- Script-run provenance: library / modified / ad-hoc / selection / line, SHA-256 source hash, retained source text.
- History semantic-version parsing fix and current v0.66.1 metadata.
- Feature manifest build consistency guard.
- Local build staging via `build-local.json` or `CHASEHQ_ROM_SOURCE` / `CHASEHQ_SDL_SOURCE`; `Prepare-Project.bat --check` validates without copying.

## Safety
ROM files remain user-local and are never added to release packages by the build scripts. Collision suppression is evidence-scoped and reversible.
```

</details>

<details>
<summary>BUILD_NOTES_0.66.1.2.txt</summary>

```text
ChaseHQ-Native v0.66.1.2 — Regression & Resilience Polish

This build is the Windows-acceptance follow-up to v0.66.1.1.

Key changes:
- Fixes Script Console appendScriptOutput helper omission and validates the helper statically.
- Fixes @runtime/web-listener.log resolution through both Script API logs.tail and HTTP logs/tail.
- Updates the logs.tail schema/example and regression references away from removed web-stdout.log.
- Stops the run progress ticker immediately on FAILED/CANCELLED paths.
- Moves full-regression COMPLETE to the actual end of the script.
- Migrates legacy chq-script-run-history-v1 records into v2 while retaining the legacy key until Clear history is explicitly used.
- Adds regression-version and regression-scope metadata with Version and Current build/Permanent/All UI filters.
- Keeps full request logging in .chq/web-listener.log while suppressing routine successful poll requests from the visible listener console unless slow.
- Retains v0.66.1.1 supervised startup, ROM/SDL staging validation, frontend bootstrap handshake, HealthCheck/Diagnostics/SupportBundle and build-local.json.

Local validation performed before packaging:
- Workbench JavaScript syntax: PASS
- JSON/data parse: PASS
- Static resilience/helper/alias/version checks: PASS
- CPU-only CMake configure/build: PASS
- CPU/bus tests: 1/1 PASS
- Regression metadata coverage: 28/28 scripts
- Package ROM exclusion: PASS
```

</details>

<details>
<summary>BUILD_NOTES_0.66.1.1.txt</summary>

```text
ChaseHQ-Native v0.66.1.1 — Resilience & Diagnostics Hotfix

This hotfix keeps the v0.66.1 emulator/research changes and hardens the paths that failed during Windows acceptance.

Fixed
- Restored the complete Script Console resilience helper/runtime block accidentally omitted in v0.66.1, including runScriptText, runner phase/state helpers, watchdog, frontend fault logging and diagnostic bundle creation.
- Browser frontend now performs a startup self-test and reports readiness to /api/v1/frontend-ready. The launcher does not print READY until the browser bootstrap reports ready (unless -NoBrowser is used).
- Web /api/health now reports scriptCount and frontendBootstrap state.
- Web listener writes timestamped startup/request messages both to its visible PowerShell console and .chq/web-listener.log.
- Local ROM staging now understands either a ROM root containing chasehq\ or the chasehq folder itself; it validates the 19 ROM files currently required by the native loader before build/launch.
- Native launch is supervised with a retained process handle, PID, stdout/stderr capture, early-exit detection and exit-code reporting.
- Startup state is persisted before API readiness, so -Diagnostics can inspect failed launches.
- Startup failures produce a diagnostic ZIP where possible instead of only a generic timeout.
- Web startup validates /api/health, /api/v1/status, /api/v1/scripts and a non-empty script library before declaring the server ready.
- Added .\Start-ChaseHQ.ps1 -SupportBundle for one-command support snapshots even when diagnosis is needed outside a successful run.
- Added Validate-Workbench.ps1 static resilience/version/script-library validation to the prepare/build path.
- Included the user's build-local.json with C:\Projects\AdditionalFiles\roms and C:\Projects\AdditionalFiles\SDL.

Important
- ROM content is never packaged in this ZIP.
- build-local.json only points to local folders; Prepare-Project fails clearly if they are unavailable/incomplete.
```

</details>

<details>
<summary>BUILD_NOTES_0.66.0.txt</summary>

```text
ChaseHQ-Native v0.66.0 - Research Automation, Introspection & Audio Foundations

Focus
-----
An infrastructure release that strengthens self-description and reusable research while the user is away from the Windows dev machine. Chase H.Q. remains the authoritative runtime target.

Added
-----
* platform.info and machine-readable feature manifest
* script.functions self-description
* API-backed query functions: getregistry(), gettrackrecords(), getevidence(), getobjects(), getaudiochannels()
* track.records structured window around the current live track record
* semantic-object registry foundation with evidence/confidence fields
* evidence.names structured evidence query
* v0.66 introspection regression
* audio research namespace and capability descriptors
* waveform viewer/capture design contract
* sample/music export design contract
* guarded runtime audio-injection design contract
* audio foundation regression

Audio guardrail
---------------
No runtime audio injection, ROM modification, or claimed waveform capture is enabled in v0.66.0. The namespace/data model is deliberately established now so later audio tooling can use the same API/script/evidence architecture. Native Windows/SDL timing, mixer and save-state behavior must be validated first.

ChaseHQ guardrail
-----------------
No ChaseHQ gameplay addresses were moved and SCI remains metadata-only/non-activatable.

Additional automation
---------------------
* experiment.begin / experiment.capture / experiment.status / experiment.end
* include "path/to/script.chqscript" preprocessor for reusable script fragments
* collection helpers over API-backed query functions: count(), first(), last(), unique(), contains()
* script-library metadata now exposes category/status/requires when present
```

</details>

<details>
<summary>BUILD_NOTES_0.65.1.txt</summary>

```text
ChaseHQ-Native v0.65.1

Experimental Profile Plumbing

This is an additive, low-risk layer on top of v0.65.0. Chase H.Q. remains the sole active runtime and no existing game-specific address or emulation path has been moved behind a profile.

Adds:
- games/chasehq/profile.json authoritative primary descriptor
- games/sci/profile.json metadata-only experimental probe
- hardware/taitoz/profile.json adapter-boundary descriptor
- GET /api/v1/profiles
- GET /api/v1/profile?game=<id>
- script actions profiles.list / profile.get
- Script Console getprofiles() query function

SCI activation/runtime support is deliberately absent.
```

</details>

<details>
<summary>BUILD_NOTES_0.65.0.txt</summary>

```text
ChaseHQ-Native v0.65.0 — Live Graphics & Script Query Workbench

Built from the documentation-updated v0.64.9 source baseline.

Major implementation areas:
- native compositor sprite list/inspect/presentation overrides and solo/hide/flash controls
- first-class Research API + REST surfaces for sprites/regions/state snapshot
- API-backed Script Console query functions with direct diagnostic printing
- image region v2 and frame.snapshot name support in compare/diff
- script library category cleanup + reusable graphics/evidence scripts
- living regression suite additions and explicit regression-maintenance rule

Validation status in packaging environment:
- embedded Web Workbench JavaScript syntax checked with Node.js
- CPU-only CMake configure/build/test is run where supported
- full SDL3/Windows native compile/runtime validation remains required on the development machine because the packaged source does not carry the local SDL source tree used by the Windows build workflow
```

</details>

<details>
<summary>BUILD_NOTES_0.64.9.txt</summary>

```text
ChaseHQ-Native v0.64.9
Graphics Research & Autonomous Experimentation

Key additions:
- Live Sprite Monitor
- Structured frame snapshots
- Experimental track mutation scripts
- 1800/3600-frame autonomous track surveys
- Graphics baseline capture script

Validation: CPU/runtime build + tests and Workbench embedded JavaScript syntax checks should pass before release. Windows SDL build remains the user's canonical full validation environment.
```

</details>

<details>
<summary>BUILD_NOTES_0.64.8.4.txt</summary>

```text
ChaseHQ-Native v0.64.8.4
Reliability & Autonomous-Research Foundation

Packaging-environment validation completed:
- CMake configure/build with CHASEHQ_BUILD_SDL=OFF: PASS
- runtime_tests / ctest: PASS (includes patch enforcement while research tracing disabled)
- embedded Workbench JavaScript syntax (node --check): PASS

Windows acceptance tests are bundled in:
  research\scripts\regression\quick-smoke.chqscript
  research\scripts\regression\full-regression.chqscript

Key fixes:
- live patch freeze/replace/suppress enforcement independent of trace enable state
- patch hit count / last-PC now update on intercepted writes
- out-of-band Script Stop via emulator pause
- cancellation-aware run-frames waiting and better timeout diagnostics
- collision-proof frame.capture naming (milliseconds + suffix)
- timestamped Web request logs
- Debug /Od + /O2 warning cleanup
- explicit SDL3-shared dependency/link staging
```

</details>

