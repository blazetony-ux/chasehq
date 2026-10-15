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

> **AUTHORITATIVE CURRENT — ChaseHQ-Native 0.67.0.0-RC6.3.22 CPU-B ORDERED HOOK/STACK SOURCE CANDIDATE (2026-10-15).** Exact immutable RC6.3.21 source SHA-256 `bdfbe71f911b2e283132410b3c22fb4159ab044baabb00a4298af3d76d93cd54`; latest Windows fresh-qualified RC6.3.21 release archive SHA-256 `fbfc6c19131a3a10a28de60449a9716111179fe03cc4285f2f970a0e8646ea3a` as recorded by the qualifier (Windows `dist` bytes not independently supplied). Genuine RC6.3.21 ROM/Ghidra native reconstruction PASS; Windows 18/18 CTests, focused 10 commands and Full Regression 1,153 commands PASS. RC6.3.22 instruments *original-ROM* CPU-B instruction hooks to record chronological selected PC visits with ordinal/cycles/SP/SR and marks IRQ vector/RTE sites separately; checks per-checkpoint actual BSR/RTS timeline with sampled stack-pointer pairing. **The selection is filtered and does not establish exhaustive IRQ/scheduling/bus parity, complete CFG, 0x0518 execution or promotable function.** Windows native acceptance and `qualify --fresh` PENDING. Evidence `--out C:\Projects\Evidence\CHQ6322`. See `docs/RC6.3.22_CPU_B_ORDERED_HOOK_STACK_EVIDENCE.md`. Older release banners below are historical.

> **AUTHORITATIVE CURRENT — ChaseHQ-Native 0.67.0.0-RC6.3.21 CPU-B NESTED CALL/RETURN SOURCE CANDIDATE (2026-10-15).** Exact immutable 0.67.0.0-RC6.3.20 source SHA-256 `f52d6abd6b2062050313a8e4bd4370311b3d39ebf956aae3510ee81da35b1a33`; latest fresh Windows-qualified 0.67.0.0-RC6.3.20 release archive SHA-256 `37ff80b5063a687765468c8dc2bde6d3b5f9c32e004408631e233b9eb6bdaab1` (reported by the Windows qualifier, actual `dist` ZIP not provided). RC6.3.20 native ROM/Ghidra reconstruct PASS, 18/18 Windows CTests, focused Run 001 ten commands and Full Regression Run 002 1,153 commands PASS. RC6.3.21 extends original-ROM CPU-B call-return research to three nested BSR instructions (`0x4F70/0x4F74/0x4F78`), nested RTS (`0x5062`) and outer RTS (`0x4F7C`), with fresh Ghidra provenance and five original-ROM scenario graphs, seven counted edges per case. **Aggregated edge counts do not prove temporal stack pairing, complete function closure or native replacement**. CPU-B `0x0518` runtime branch remains unseen; no native promotion. RC6.3.21 Windows native acceptance and `qualify --fresh` PENDING. Evidence `--out C:\Projects\Evidence\CHQ6321`. See `docs/RC6.3.21_CPU_B_NESTED_CALL_RETURN_AUDIT.md`. Older document banners below are retained as history.

> **AUTHORITATIVE CURRENT — ChaseHQ-Native 0.67.0.0-RC6.3.20 CORRECTIVE SOURCE CANDIDATE (2026-10-15).** Latest fully Windows-qualified baseline 0.67.0.0-RC6.3.18 SHA-256 `7978d2e4bdac825e81ef9540694b0ccb54edcd5d04c342b2472198dc25500099`. Exact immutable failed 0.67.0.0-RC6.3.19 source archive SHA-256 `0900d52cb41bea82203695517c708e4a399dab971db878ae85f465c0e099dde0`. RC6.3.19 Windows/MSVC build PASS, but `M2-CPU-B-original-ROM-call-return-audit` **FAIL** due solely to string-format comparison of Ghidra `0x000516` vs runtime-canonical `0x00000516` for post-return CPU-B `BTST` at `0x050E`. RC6.3.20 changes the comparison to strict numeric 32-bit equality and adds independent replay of the actual CHQ6319 Windows Ghidra JSON and all five original-ROM traces (PASS after correction), plus negative guards. Full enclosing function CFG, native substitution composition, unobserved `0x0518` path, timing parity and autonomous promotion remain **UNPROVEN/BLOCKED**. Windows RC6.3.20 native acceptance/`qualify --fresh` PENDING. One ZIP: `--out C:\Projects\Evidence\CHQ6320`. See `docs/RC6.3.20_REAL_GHIDRA_ADDRESS_CORRECTIVE.md`. Older banners are historical.

> **AUTHORITATIVE CURRENT — ChaseHQ-Native 0.67.0.0-RC6.3.19 SOURCE CANDIDATE (2026-10-15).** Immutable RC6.3.18 original authoritative source SHA-256 `7968f432939be3a16de942d46afb8caeeffddba39df3a32c2689c05469530369`; fresh Windows qualified archive SHA-256 `7978d2e4bdac825e81ef9540694b0ccb54edcd5d04c342b2472198dc25500099` (recorded by user Windows qualify), CHQ6318 ROM/Ghidra native reconstruction PASS; 18/18 Windows CTest, focused Run001 10 commands, Full Regression Run002 1,153 commands PASS. RC6.3.19 adds **authentic CPU-B observed JSR→callee→RTS→postreturn provenance** across five original-ROM checkpoints; Ghidra exports new CPU-B 0x050C/0x050E/0x08AC statics and the unobserved 0x0518 branch path (static only). Local replay uses authentic CPU-B TSV and a clearly synthetic static Ghidra double. 0x0518 is NOT runtime proven; no complete function CFG, nested-call closure, timing/composition verification, minimiser or autonomous promotion. RC6.3.19 Windows ROM/Ghidra acceptance is PENDING. One evidence ZIP via `--out C:\Projects\Evidence\CHQ6319`. See `docs/RC6.3.19_CPU_B_ORIGINAL_CALL_RETURN_AND_UNOBSERVED_BNE.md`; older banners are historical.

> **AUTHORITATIVE CURRENT — ChaseHQ-Native 0.67.0.0-RC6.3.18 SOURCE CANDIDATE (2026-10-15).** Exact RC6.3.17 fresh Windows qualification PASS: 18/18 CTest, focused Run001 10 commands, Full Regression Run002 1,153 commands, deterministic Windows release ZIP SHA-256 `6fc40389db4bccd47aef40b7a2c874c43517e2db45b2ac29ced4e72c3f8c1d79`. Genuine CHQ6317 bounded CPU-A/B native/rollback evidence PASS; CPU-B observed research frontier 5,234 edges PASS. RC6.3.18 adds **five extra real-ROM Ghidra continuation instruction sites** (0x04E0/0x04E6/0x04EC/0x0516/0x052C), statically checks lengths/flows and crosswalks with five genuine original-ROM checkpoint cases; entire function CFG, BNE fallthrough, generated whole-function C++, minimiser, composition and native promotion remain BLOCKED. Local static Ghidra cannot run here; RC6.3.18 genuine Windows execution is PENDING. One evidence ZIP: `--out C:\Projects\Evidence\CHQ6318`. See `docs/RC6.3.18_CPU_B_GHIDRA_CONTINUATION_CROSSWALK.md`. Prior banners are historical.

> **AUTHORITATIVE CURRENT — ChaseHQ-Native 0.67.0.0-RC6.3.17 SOURCE CANDIDATE (2026-10-15).** Exact RC6.3.16 Windows-qualified baseline SHA-256 `79c94b0090009b5f5dd29dc2feb543d5d758c072919bb428a761342f732a646d` passed 18/18 Windows CTest, focused 10-command Run001 and Full Regression 1153-command Run002; RC6.3.16 genuine CPU-A/B bounded native/rollback evidence PASS. RC6.3.17 adds a read-only two-hop CPU-B *observed* original-ROM function frontier and heuristic, risk-labelled candidate research ranking (NOT measured cycle savings). Complete enclosing function, automatic minimiser, independent composition gate, native whole-function promotion and RC6.3.17 Windows acceptance remain PENDING/BLOCKED. Next proof: `--out C:\Projects\Evidence\CHQ6317`. See `docs/RC6.3.17_OBSERVED_FUNCTION_FRONTIER.md`. Prior release status banners below are historical.

> Current ChaseHQ-Native 0.67.0.0-RC6.3.16 source candidate. See `docs/RC6.3.16_BOUNDED_EDGE_AND_PORTFOLIO_RECONCILIATION.md`; historical text follows.

> Current source release v0.67.0.0-RC6.3.15: historical release-specific findings below remain unchanged; see docs/RC6.3.15_FUNCTION_DOSSIER_AND_EVIDENCE.md.

> Current RC6.3.15 source candidate: see `docs/RC6.3.15_FUNCTION_DOSSIER_AND_EVIDENCE.md`; historical content retained unchanged.

> **AUTHORITATIVE CURRENT — v0.67.0.0-RC6.3.14 CPU-B dual-candidate audit corrective (2026-10-14).** RC6.3.13 Windows Debug PASS, generated Ghidra pair ABI 4/4 PASS; M5 single BEQ verifier BLOCKED because inactive TST+BEQ original-ROM audit rows were rejected. This candidate repairs strict audit identities and tests, retains complete CPU/IRQ/bus/machine comparisons; **RC6.3.14 Windows pair live three-mode verification PENDING**, function promotion BLOCKED, inherited Go Full Regression 1176 >850 FAIL. See `docs/RC6.3.14_CPU_B_DISPATCH_AUDIT_CORRECTIVE.md`. Older CURRENT text is historical.

> **CURRENT RELEASE — v0.67.0.0-RC6.3.13 CPU-B TWO-INSTRUCTION LIVE SOURCE CANDIDATE (2026-10-14).** RC6.3.12 original-ROM Ghidra generated pair ABI on Windows PASS; RC6.3.13 opt-in live pair ROM/native/forced-rollback gate pending authentic Windows qualification. Full Go structural budget 1176 > 850 remains FAIL, no function/autonomous promotion. See `docs/RC6.3.13_LIVE_CPU_B_PAIR.md`. Older sections historical.

> **AUTHORITATIVE CURRENT — v0.67.0.0-RC6.3.12 CPU-B TWO-INSTRUCTION GENERATED ABI SOURCE CANDIDATE (2026-10-14).** RC6.3.11 authentic Windows single CPU-B BEQ original/native/fault PASS (17,892 commits, exact CPU/IRQ/machine parity; 869/869 evidence hashes). RC6.3.12 adds separate original-Ghidra TST.W 0x4D8 + BEQ 0x4DE pair C++ ABI from authenticated ROM extension words and 17,903 consecutive observed edges; **pair is offline only, NOT yet a live native replacement**. All CPU-A/B enclosing function boundaries, autonomous M1–M7 and full Go budget 1176>850 remain open. See `docs/RC6.3.12_CPU_B_TWO_INSTRUCTION_ABI.md`. Earlier status paragraphs remain historical.

> **AUTHORITATIVE CURRENT — v0.67.0.0-RC6.3.11 CPU-B IRQ-OWNERSHIP CORRECTIVE SOURCE CANDIDATE (2026-10-14).** RC6.3.10 actual Windows `native reconstruct --cpu-b-live` failed only at the CPU-B original-ROM baseline because 12 interrupt-vector/stack bus effects (four genuine IRQ entry groups) were wrongly tagged as BEQ.B data effects by stale `trace_pc`; its CPU-A health/copy, shared dispatcher and real CPU-B Ghidra/compiled ABI gates all PASS. RC6.3.11 uses Musashi interrupt ACK to assign each IRQ vector/stack transaction to a dedicated ordered IRQ stream, preserves strict zero-branch-data and original/native/fallback CPU/cycle/bus/machine parity, and adds real-ACK synthetic and missing/mutated IRQ evidence tests. **RC6.3.11 authentic Windows native CPU-B equivalence is PENDING**, as are complete-function/autonomous promotion and inherited Go Full Regression budget 1176>850. See `docs/RC6.3.11_IRQ_OWNERSHIP_CORRECTIVE.md`. Earlier status blocks remain historical.

> **RC6.3.10 current source and next steps.** This historical section is retained for provenance; current build is 0.67.0.0-RC6.3.10. RC6.3.10 live CPU-B BEQ ROM/native/fallback Windows acceptance is PENDING; full-function and autonomous promotion BLOCKED. See `docs/RC6.3.10_LIVE_CPU_B_BEQ.md`.

> **AUTHORITATIVE CURRENT — v0.67.0.0-RC6.3.9 CPU-B ROM/GHIDRA GENERATED ABI (2026-10-14).** RC6.3.8 Windows original/native/fallback shared CPU-A dispatch PASS with authentic 5,234 CPU-B transition rows. RC6.3.9 adds guarded original CPU-B World ROM CRC/SHA verification, five-PC Ghidra static provenance, and compiled BEQ branch C++ shared ABI candidate; **not yet run against real CPU-B Ghidra on Windows**, no CPU-B live native substitution, M6–M7 autonomous promotion BLOCKED. See `docs/RC6.3.9_GENUINE_CPU_B_GHIDRA_GENERATOR.md`. Historical sections retained.

> **AUTHORITATIVE CURRENT - v0.67.0.0-RC6.3.6 SECOND GHIDRA-GENERATED COPY BLOCK (2026-10-14).** RC6.3.5 Windows authentic caller-to-health-tail and CPU-B original census PASS. RC6.3.6 compiles a second Ghidra-derived CPU-A 16-instruction copy library and adds opt-in ROM-backed original/native/forced-rollback parity; 15/15 local CPU tests, Windows native proof PENDING. No full-function or CPU-B native promotion. See `docs/RC6.3.6_SECOND_LIVE_BLOCK.md`. Historical sections below remain as recorded.

> **AUTHORITATIVE CURRENT — v0.67.0.0-RC6.3.4 OBSERVED HEALTH FUNCTION BOUNDARY / CPU-B CANDIDATE PORTFOLIO SOURCE CANDIDATE (2026-10-13).** RC6.3.3 real Windows 5-case controlled original/native/forced-rollback health matrix PASS with 3 distinct authentic original-ROM paths, 9/9 candidate sites, five native commits, five verified ROM fallbacks; 508/508 evidence hashes independently verified. RC6.3.4 adds real 68000 stacked return/entry captures, independent ROM RTS and observed caller audit, and CPU-B original-ROM PC census into a persistent candidate portfolio; **actual Windows RC6.3.4 execution and release qualification PENDING**. Observed boundary does not independently certify all callers or a complete function CFG. CPU-B observed PCs are not functions. No generalized autonomous promotion. RC6.2.6 steering instruction baseline remains signed off. See `docs/RC6.3.4_BOUNDARY_PORTFOLIO.md`. All older CURRENT headings below are historical.

> **AUTHORITATIVE CURRENT — v0.67.0.0-RC6.3.3 CONTROLLED HEALTH BRANCH-MATRIX DEVELOPMENT CANDIDATE (2026-10-13).** RC6.3.2 real Windows bounded CPU-A health-function live differential PASS: one generated multi-instruction commit and authentic forced rollback with exact CPU/bus/mapped state; manifest 198/198 verified. RC6.3.3 adds original-ROM instruction-hook path capture and controlled entry-state 5-case x original/native/rollback differential; real Windows matrix and release qualification **PENDING**. Controlled branch coverage cannot be presented as natural gameplay or independently reviewed full-function coverage. No automatic CPU-B promotion or autonomous multi-routine orchestration. RC6.2.6 steering instruction baseline remains signed off. See `docs/RC6.3.3_BRANCH_MATRIX.md`. All older CURRENT headings below are historical.

> **AUTHORITATIVE CURRENT — v0.67.0.0-RC6.3.2 TRACE-BOUNDARY CORRECTIVE SOURCE CANDIDATE (2026-10-13).** Actual Windows RC6.3.1 evidence showed one generated CPU-A target-health function commit with exact CPU-state and mapped end-state parity but an extra caller W32 0x102DE0 attributed to the function. RC6.3.2 fixes post-native-commit bus PC attribution and adds a synthetic Musashi original/native/forced-rollback regression which fails against RC6.3.1 and passes with the correction; full game-ROM Windows differential and release qualification are **PENDING**. No RC6.3 general-function promotion or CPU-B autonomy. RC6.2.6 bounded steering M1-M7 signoff retained. See `docs/RC6.3.2_TRACE_BOUNDARY.md`. All older CURRENT statements below are historical.

> **Release provenance:** Current development source 0.67.0.0-RC6.3.1. Historical research content below is retained unchanged. See `RC6_STATUS.md` for authoritative maturity and qualification.

> **AUTHORITATIVE CURRENT — 0.67.0.0-RC6.3 DEVELOPMENT SOURCE CANDIDATE (2026-10-13).** RC6.2.6 bounded CPU-A steering instruction M1–M7 technical acceptance SIGNED_OFF from real Windows evidence: 240 native replacements, 240 identical CPU return states, ordered bus/mapped-state parity, genuine forced rollback. RC6.3 adds three real Ghidra-derived multi-instruction generated C++ candidates including target-health branching; offline fixtures PASS but **RC6.3 whole-function autonomous M1–M7 and Windows qualification are NOT SIGNED OFF**. See `docs/RC6.3_AUTONOMOUS_RECONSTRUCTION.md` and `RC6_STATUS.md`. Older CURRENT headings below are historical snapshots only.

> **CURRENT RELEASE CANDIDATE: 0.67.0.0-RC6.2.6** — RC6.2.4 proved 240 actual CPU-A steering hits and 48 false shadow mismatches for raw 0x0FE0. RC6.2.6 repairs M2 census case normalisation and 68000 low-byte sign extension in shadow and native substitution; Windows qualification and ROM-backed M1–M7 acceptance PENDING. See RC6_STATUS.md and docs/RC6.2.6_M1_M7_ACCEPTANCE.md. All older current-status statements are historical.

> CURRENT 0.67.0.0-RC6.2.4: bounded generated-library native steering instruction A:0x820C proof; Windows acceptance pending; see RC6_STATUS.md. Prior states historical.

> **AUTHORITATIVE CURRENT STATUS — 0.67.0.0-RC6.2.2 SOURCE CANDIDATE (2026-10-12).** RC6.2.1 Windows qualification PASS (SHA256 `c8422141d9d079ce522f0b1dc12a95527c5d80e9055b4c874fb356493b46174b`); actual CPU-A Ghidra steering export `0x820C` PASS. RC6.2.2 adds self-contained, fail-closed M1–M7 **acceptance evaluation**, not automatically proven unrestricted native replacement. Full acceptance and Windows qualification must be tested; see `docs/RC6.2.2_M1_M7_ACCEPTANCE.md` and `RC6_STATUS.md`. Prior CURRENT/unqualified statements below are preserved as **historical snapshots, not current release status**.

> **CURRENT RC6.1 DEVELOPMENT — v0.67.0.0-RC6.2.1; native steering shadow opt-in, Windows qualification outstanding.** See `RC6_STATUS.md`; older status below is historical.

> **RC6 DEVELOPMENT STATUS (not qualified):** v0.67.0.0-RC6. RC5.3 sections below are historical. See `RC6_STATUS.md` at release root.

> **CURRENT: v0.67.0.0-RC5.3 native sprite-clear SUBSTITUTION DEVELOPMENT CANDIDATE (UNQUALIFIED), 2026-10-11.** The original Windows-qualified RC5 parent SHA-256 `47def65d75056fb765d5b60dfe5a4c4a21bdcd3214ebb84920dd62a0d8b37ddf` remains immutable. RC5.1 headless live-shadow corpus PASS 12/12 cases (96/96 sprite transfers and 24/24 class-6 road generator comparisons); RC5.3 adds an experimental, opt-in native substitution of the trailing sprite-clear DBRA loop ONLY. Bulk copier and final RTS remain original ROM; normal emulation is default. RC5.3 Windows native A/B equivalence and exact-archive qualification pending. See `docs/RC5.3_NATIVE_CLEAR_SUBSTITUTION.md`; all older release-status headings below are historical.

> **RC5 SOURCE CANDIDATE (`0.67.0.0-RC5`), NOT WINDOWS-QUALIFIED.** Original RC4.7 source archive supplied by user; RC5 integrates offline native C++ sprite/road candidates and standalone synthetic Windows preview, updates live Workbench Next Steps. No live SHADOW/COMPARE or native replacement is implemented. Follow `docs/RC5_NATIVE_PREVIEW_AND_SHADOW.md` and exact Windows `chqdev.bat qualify --fresh` gate before promotion. Older version status sections below are historical.

> **RC4.7 current source candidate `0.67.0.0-RC4.7`; qualified promoted baseline remains RC4.6.** See `docs/REA_GHIDRA_RC47.md`. Windows `chqdev.bat qualify --fresh` pending.

> **AUTHORITATIVE PROMOTED BASELINE — v0.67.0.0-RC4.6 (2026-10-10).** Windows `chqdev.bat qualify --fresh` reports **11/11 stages PASS** for exact frozen ZIP SHA-256 `9e76a107e19f3e28a54a8a8b3bdaed630511e02f1da3e3e7ae274c07d863580f`. User explicitly authorised promotion following focused, full, and successful headless Ghidra/REA crosswalk evidence. This is a **post-qualification documentation overlay**; it does not change the immutable tested archive. See `docs/RELEASE_VERIFICATION_RC46.md`, `promotion/PROMOTION_RECORD.json`, and `docs/REA_GHIDRA_NEXT_SCOPE.md`. Earlier RC4.6 “candidate/unqualified” statements below are **historical build-time notes, not current release status**.

> RC4.6 source candidate documentation identity: `0.67.0.0-RC4.6`; prior sections retain historical findings. Promoted baseline is RC4.5.

> RC4.5 CURRENT SOURCE CANDIDATE: `0.67.0.0-RC4.5` (not Windows-qualified). Promoted RC4.4 remains authoritative. REA Research API, Workbench view and checkpoint-aware census scope: `docs/REA_RC45.md`. This document retains historical observations.

> **AUTHORITATIVE PROMOTED BASELINE — v0.67.0.0-RC4.4 (2026-10-10).** Exact Windows-qualified archive SHA-256: `8a8b715646ee07619891ea137425bd24272d5df2b84c60170105a164ec3d8912`. User-authorised promotion; all eleven stages in Windows `out/chqdev/qualification.json` are PASS, including `promotable`, focused and full regressions, and run-history. The immutable qualified archive is unchanged. Full provenance: `promotion/PROMOTION_RECORD.json` in the separate promotion supplement; historical candidate notes below describe earlier states and are retained.

## RC4.6 post-promotion REA/Ghidra limitation intake (2026-10-10)

All entries below are **backlog**, not implemented in the immutable RC4.6 archive. Keep the earlier LIM-001–014 provenance entries intact. The authoritative release is RC4.6 and these are limitations discovered through its actual Windows Ghidra/REA proof, not a release-qualification failure.

### LIM-015 — Ghidra results path and run-to-run import discovery (P0, confirmed defect)
- **Evidence:** Successful Ghidra job `ghidra-20261010-161350-586bf542`, findings ZIP `ghidra-rea-run032-20261010-161351-650.zip`; `api ghidra.results` reported missing findings despite the file existing under the run's `analysis/` folder. First `ghidra.import` returned `reports:[]` when the previous release's actual census TSV files were not staged. Manual Run 035 crosswalk later passed.
- **Impact:** Passing headless analysis looks broken; users must stage inputs manually and cannot rely on a one-command investigation.
- **Workaround:** Inspect verified Ghidra evidence ZIP/run `analysis/chq-ghidra-findings.json`, stage matching census TSVs and call `rea.crosswalk` per scenario.
- **Acceptance:** Resolve findings via job's authoritative result/run path; make staged census dependencies visible; an API PASS with missing inputs must report the reason, not imply failed static analysis. Add Windows/file layout tests and release-migration fixtures.

### LIM-016 — Java/Jython preflight and worker process inheritance (P0, confirmed defect)
- **Evidence:** Two early headless jobs failed because the Workbench process could not find Java; later Ghidra launched with Java 25 but failed when Jython 12.1.4 was not enabled; qualified CLI health/status falsely reported readiness. `chqdev runtime restart` with explicit child-process JAVA_HOME/PATH passed, then Jython was enabled in Ghidra settings.
- **Impact:** Valid installed Ghidra is unusable from Workbench, with opaque repeated runs.
- **Workaround:** Verify Java 25/Jython locally, explicitly set JAVA_HOME/PATH for `chqdev runtime restart`, enable version-compatible bundled Ghidra Jython extension.
- **Acceptance:** `chqdev doctor`, `runtime start/restart`, `ghidra.status/start` check actual *worker environment*; report Java/Ghidra/scripting engine versions and readiness separately; fail before launching if dependencies missing. Consider Java/PyGhidra migration. No silent global Windows env changes or unapproved software installs.

### LIM-017 — Single-script Ghidra wait and complete evidence (P0, missing capability)
- **Evidence:** Run 033 starts asynchronously; Run 033B fails when job state is FAIL; user had to collect `headless-console.txt` using PowerShell. RC4.6 lacks a script-aware wait that returns on success/failure/timeout and bundles remote-process outputs.
- **Impact:** Fragmented multi-script, multi-zip workflow; missing failure cause in standard Script Run evidence.
- **Workaround:** Poll `api ghidra.status`, download Ghidra ZIP separately, upload logs manually on failure.
- **Acceptance:** Bounded `ghidra.wait timeout=... poll=...` and evidence attachment on both success and failure, preserving original failure signal; proven no busy loops, no blocked emulator, cancellation, mobile parity, deterministic ZIP manifest and path restrictions.

### LIM-018 — Dedicated top-level Static Analysis / REA UI (P0 UX, requested)
- **Evidence:** User screenshot after Run 035 shows REA/import/report/function controls compressed into a narrow Native Lab column despite three imported reports and seven-PC function detail.
- **Impact:** Function browsing and disassembly/xref comparison are impractical, misleading empty Function selector until View functions.
- **Workaround:** Current compact Native Lab panel and API text/JSON inspection.
- **Acceptance:** Top-level responsive REA/Ghidra tab; wide function list and detail panels, ROM fingerprint/source provenance, CPU filter/search, live-vs-static evidence and job status, automatic function population on report selection, Ghidra artifacts and confidence labelling; preserve script/API/mobile parity and user session.

### LIM-019 — Source-built Capstone 68000 disassembly bridge (P1, missing capability)
- **Evidence:** RC4.6 release scope contains Ghidra/REA/Musashi but no Capstone dependency or Research API commands, despite earlier Reverse Engineering Tool Bridge agreement.
- **Impact:** No independent instruction-decoder comparison or convenient static disassembly export in a live script.
- **Workaround:** Ghidra 68000 static disassembly and existing Musashi debug instructions.
- **Acceptance:** Optional source-built Capstone 68000 for CPU-A/B, explicit original ROM mapping, decode accuracy tests against original PCs, Workbench and Script Console parity, structured evidence, avoid replacement/instruction mutation.

## LIM-014 — REA static-analysis visibility and Workbench/API parity (P1 deferred)

- **Source:** User request following Windows `0.67.0.0-RC4.4` qualification and promotion (2026-10-10). RC4.4 implements `chqdev rea crosswalk` output but has no Workbench Static Analysis view or import API.
- **Impact:** Static-to-runtime correlations remain difficult to inspect during live investigation; no linked function evidence is surfaced in Native Reconstruction Lab.
- **Workaround:** Run the offline adapter, inspect deterministic crosswalk JSON in a text editor, and correlate PC counts/evidence manually.
- **Acceptance:** Add a read-only REA/Static Analysis view, reviewed-export import, provenance/confidence/ROM-sha validation, CPU function detail, original disassembly and evidence cross-links, CLI/script parity, and performance/Windows regression tests. Do not confuse this with direct REA/Ghidra raw-ROM support.
- **Schedule:** Coherent next native-reconstruction tool build, not a standalone release.

## LIM-013 — Projection shadow CLI / Workbench parity (P1 deferred)

- **Source:** RC4.4 design review after Run 029; `ChaseHQRuntime --native-projection-shadow` implements bounded CPU-A oracle with native CTest, but the live Windows SDL/Workbench Script API does not expose its counters.
- **Impact:** A researcher cannot yet invoke the projection shadow from an existing live Workbench checkpoint; CPU-only command runs from boot and require separate scenario setup. Multi-checkpoint parity remains unproven.
- **Workaround:** Run explicit CPU-only CLI sessions with locally owned content and retain projection JSON; use Workbench scripts separately for controlled target interventions, correlate manually. No live backend substitution.
- **Acceptance criteria:** Wire the same independent oracle into live SDL runtime with Script Console/UI/CLI parity, reset/status/trace export, canonical checkpoint coverage, bounded event accounting, zero accidental mutation and gateable comparison evidence. Keep `Replace` disabled until native geometry/side-effect contract is proven.

> **Current development candidate:** `v0.67.0.0-RC4.4`. Windows exact-archive qualification pending. See `docs/NATIVE_PROJECTION_RC44.md` for present scope and limitations.

## RC4.4 status update (source candidate; not yet Windows qualified)

Runs 028-029 confirm staged map-19 sprite correlation. No additional emulator limitation is inferred from Run 028's initial delayed snapshot. The optional offline correlation helper handles changing hardware slots when map/X/frame evidence are available, but automatic structured Workbench trace/paired-frame export remains on the backlog. LIM-010 trace window, LIM-011 saturation visibility, and LIM-012 checkpoint preflight remain outstanding; no defaults are weakened to hide them. The P1 REA crosswalk adapter is implemented in RC4.4 but live raw 68000 REA provider support remains deferred.

# Investigation limitation register — ChaseHQ-Native

Status: **deferred development backlog**, captured as of 2026-10-10 through Workbench Run 029 on **Windows-qualified RC4.3.2**. No source code changed, no next release number assigned, and no candidate promoted. This register is the intake queue for limitations discovered in investigations. Before an implementation is chosen, validate whether its capability already exists in the schema/API and whether a documented workaround suffices.

> Run 027 (bundle 20261010-105455) PASS 262 commands / 115.8s: core cache producer and coordinate reader present at all six checkpoints; companion reader and sprite producer absent at gameplay2064; post-final-hit target-like sprite map19 remains subject to controlled identity proof. No new tooling limitation or release cut. The previously agreed REA integration is explicitly queued in ROADMAP/NEXT_BUILD_CHANGELOG, separate from the limitation register.

## Standing capture and release-triage rule

For every future research run, record **all encountered limitations** even when the investigation succeeds. Each entry must state: stable ID; originating run and exact supporting evidence; observed failure or missing capability (distinguish confirmed defect from improvement); research consequence; existing workaround; precise expected behaviour/acceptance gates; priority and dependencies; status; and target release **TBD** until reviewed. Never fabricate an API or call a feature absent without inspecting the current source/schema. Do not schedule a new build for a backlog-only change.

Triage alongside *actual game progress* at the next coherent build decision. Prefer grouping related CPU instrumentation / Workbench API / evidence export changes; do not allow the backlog to displace current gameplay research. Promote individual items to a named release only after selecting scope and regression gates. Keep the release immutable until the user explicitly approves promotion, even after Windows qualification. At packaging, update `docs/ROADMAP.md`, `docs/NEXT_BUILD_CHANGELOG.md`, `docs/PROJECT_STATE.md`, `docs/HANDOVER.md`, API/schema docs and Workbench Next Steps from one reconciled source of truth. Retain historical records as such rather than presenting them as active.

## Open limitations and opportunities

### LIM-001 — Live, checkpoint-aware ROM Execution Census / Observe bridge

- **Status:** DEFERRED; proposed next coherent native-tooling update; **priority:** P1 research blocker; **target:** TBD.
- **Evidence:** Offline 120-frame boot and IRQ4 runs on RC4.3.2 both measured 1,312,666 CPU-A instructions, 324 PCs, zero CPU-B instructions and did not leave startup; IRQ4 produced 120 requests/CPU but zero acknowledgements. Existing Workbench gameplay checkpoints cannot currently enable the offline census recorder in-process.
- **Impact:** Cannot rank representative game-code hotspots from canonical gameplay states despite a working recorder.
- **Workaround:** Continue Workbench memory/PC read-write provenance on live gameplay; preserve offline results as instrumentation sanity checks, not gameplay coverage.
- **Acceptance:** Start/stop/reset/snapshot/compare/export in current live session; CPU-A/B, checkpoint/frame/release/ROM-pack provenance; native Observe counters; JSON/TSV in evidence ZIP and UI/CLI parity; unchanged execution and deterministic bounded tests, including both CPUs active in a canonical gameplay scenario.
- **Dependencies:** May group with LIM-002/LIM-003/LIM-004; no independent RC.

### LIM-002 — Instruction-boundary registers and correlated routine entry/return traces

- **Status:** DEFERRED, documented in backlog v4; **priority:** P1 native replacement blocker; **target:** TBD.
- **Evidence:** Run 015 could disassemble and trace memory by PC but could not capture register state before/after `0x434E8–0x438C6` or link calls/returns with register and stack snapshots. Candidate renderer contract remains incomplete.
- **Impact:** Cannot independently implement or SHADOW-verify a safely bounded replacement solely from existing logs.
- **Workaround:** Disassembly, memory provenance, repeatable checkpoint A/B and controlled interventions. Do not mislabel writer PCs as a complete routine contract.
- **Acceptance:** Filtered entry/return and pre/post-instruction `D0-D7`, `A0-A7`, `PC`, `SR`, stack, frames/cycles, memory delta; invocation correlation and unmatched/overflow indicators; bounded on-demand traces without restart; structured evidence, CLI/Workbench parity and determinism/no-perturbation tests.
- **Dependencies:** LIM-001/003/004; SHADOW/REPLACE remains fail-closed.

### LIM-003 — Correlated multi-range / producer-consumer trace sessions

- **Status:** NEEDS CURRENT-API CAPABILITY AUDIT, then deferred if missing; **priority:** P2 research efficiency; **target:** TBD.
- **Evidence:** Run 015 required separate checkpoint replays to trace target presentation RAM, sprite output RAM and two ROM-table regions. The supplied scripts use only one `memory.trace.start` watch at a time, so cross-range causality is reconstructed later from independent passes.
- **Impact:** Extra script runs/commands and potential loss of within-invocation ordering, making routine contract extraction slower.
- **Workaround:** Repeat deterministic 13-frame branches with bounded traces, correlate PCs and frame sequence offline.
- **Acceptance:** First audit existing watch/trace interfaces. If absent, add configurable simultaneous named read/write regions and optional PC/call-stack filters sharing a monotonic sequence/cycle timebase; reject excessive caps cleanly; deterministic JSONL export with overflow indicators and UI/CLI parity. No additional restart.
- **Dependencies:** LIM-002 and LIM-004; bundle with same instrumentation milestone.

### LIM-004 — Structured, complete memory-trace evidence in automatic bundles

- **Status:** EVIDENCE-CAPTURE GAP OBSERVED / implementation needs API review; **priority:** P1–P2; **target:** TBD.
- **Evidence:** Run 015 bundle verifies, records 150 successful commands, but contains only one standalone investigation artifact (`artifacts/checkpoints/loaded.jsonl`); 550,650 bytes of `console-output.txt` hold the disassembly and trace lines. Individual trace streams are not included as dedicated machine-readable artifacts in this bundle. Traces are below their caps; this is a packaging/export limitation, not trace overflow.
- **Impact:** Hard to compare high-volume branches, prove absence of missed events, or feed automated native-vs-original checks.
- **Workaround:** Parse console logs / use explicit structured APIs where already supported; retain raw bundles.
- **Acceptance:** Export every named trace pass as lossless bounded JSONL/TSV with CPU, frame/cycle, PC, access, address, width, value, branch/phase, event sequence, dropped-event counts, session/checkpoint identity; automatic ZIP and checksum manifest; no hidden truncation by `tail`; regression confirms reconstructable counts and control/intervention comparisons.
- **Dependencies:** LIM-003, LIM-002; candidate for next meaningful instrumentation update.

### LIM-005 — Stable semantic sprite identity and automatic branch-difference reports

- **Status:** DEFERRED / audit semantic resolver first; **priority:** P2; **target:** TBD.
- **Evidence:** Runs 004–012B showed the Stage-1 target map/palette sprite moving from hardware slot 68 to slot 70/75/74 depending on frame and coordinate intervention. Run 012 initially used screenshot `hud=hidden` comparison without a layered snapshot, causing failure. Run 012B succeeded with normal screenshot comparisons.
- **Impact:** Fixed slot comparisons can falsely report unchanged target graphics; screenshot-vs-layered comparison modes can be confused.
- **Workaround:** Full `state.snapshot` sprite arrays and keyed map/palette/object-context comparison, normal PNG image comparison, explicit `frame.snapshot` for HUD-hidden outputs.
- **Acceptance:** Verified semantic object association across frame/slot reorder (with confidence/unresolved markers); automated multi-branch sprite/pixel difference report; script preflight validates source modality for `image.compare`/HUD-hidden and offers corrective guidance; regression on slot reordering and negative intervention. No invented identity.
- **Dependencies:** Rendering research; can be grouped separately from CPU tracing if justified.

### LIM-006 — Workbench Next Steps / research documentation drift

- **Status:** OPEN documentation/data consistency; **priority:** P2; **target:** TBD.
- **Evidence:** Workbench still labelled the police pursuit beacon producer as NEXT/HIGH even though authoritative roadmap records the investigated beacon state producer complete at its defined scope.
- **Impact:** Time wasted retesting completed tasks and misrepresenting native progress.
- **Workaround:** Manually reconcile Workbench's `games/chasehq/next-steps.json`, Registry and project roadmap before choosing an investigation.
- **Acceptance:** Status/ID consistency checks across Next Steps/Registry/roadmap; distinguish observed, native-shadow, replaceable and native states; test stale/contradictory items; update docs on meaningful next release or documentation-only source patch without forcing RC.

### LIM-007 — Full Regression expanded-command structural budget

- **Status:** OPEN, existing known Go test; **priority:** P2 release hygiene; **target:** TBD.
- **Evidence:** Full Regression successfully ran 1,153 commands on qualified RC4.3.2; Go structural check still rejects ~1,176 expanded commands against a <850 threshold.
- **Impact:** A test suite can qualify in Windows while the independent command-budget check remains red; manual runs remain longer than necessary.
- **Workaround:** Preserve all unique regression assertions and canonical checkpoint isolation, measure command timings, batch only equivalent diagnostics.
- **Acceptance:** Coherently reduce expanded commands and runtime without deleting unique scenarios/assertions; no arbitrary limit increase or gate suppression; check Go budget and exact-archive Windows qualification.

### LIM-008 — Historical global script-run listing can still block serial backend

- **Status:** FOLLOW-UP HARDENING / severity needs reproduction on RC4.3.2; **priority:** P2; **target:** TBD.
- **Evidence:** RC4.3 support bundle showed 23–29 s global history scans blocking the serial PowerShell backend; RC4.3.1/RC4.3.2 eliminated startup history blocking and passed Windows readiness, but manual all-release history performance has not yet been proven under a large workspace.
- **Impact:** Possible disruption during explicit historical discovery, even though startup qualification now passes.
- **Workaround:** Prefer current-session run history and indexed summaries; avoid unnecessary global history refresh during gameplay.
- **Acceptance:** Reproduce or clear this limitation first; responsiveness under a deliberately slow historical scan, bounded indexing/pagination and complete history with no lost runs.

### LIM-009 — Static disassembly can begin mid-instruction

- **Status:** USABILITY REVIEW; **priority:** P3; **target:** TBD.
- **Evidence:** Run 015 disassembled from `0x40F40` (inside a branch instruction) and the first line rendered `dc.w $39e4; ILLEGAL`, while starting at `0x40F42` shows the expected branch. This is a caller-chosen unaligned-to-instruction-boundary start, not proof of illegal ROM execution.
- **Impact:** Can mislead instruction-boundary analysis and cause false reverse-engineering conclusions.
- **Workaround:** Start disassembly from a known branch target or prior decoded instruction boundary; use surrounding code to verify alignment.
- **Acceptance:** Optional boundary/context warnings or start-at-prior-instruction navigation; never silently rewrite requested address; validate with known 68000 multiword instructions.

### LIM-010 — Dynamic ROM-lookup trace range coverage / false-negative warning

- **Status:** OBSERVED INVESTIGATION DESIGN LIMITATION; implementation NEEDS AUDIT; **priority:** P2; **target:** TBD.
- **Evidence:** Run 021 (`bundle(20261010-101052).zip`) traced only `0x54000–0x54FFF`; for target/reference separation `+0x7FFB` and `+0x7FFF`, the disassembled `asr.l #1; andi.w #$fff8; adda.w D1,A2` computes a lookup base around `0x580F8`, **outside** that window. The five renderer PCs were absent from the trace, but the result cannot be classified as an early exit. By contrast, the lower boundary at `−0x180` (observed reader addresses `0x54041` etc.) and rejected `−0x181` were in-range and distinguishable.
- **Impact:** A zero-event trace can be mistaken for a skipped branch or clipping when code actually reads outside the requested range; invalidates conclusions and causes unnecessary diagnostic runs.
- **Workaround:** Compute candidate effective addresses from disassembly before choosing trace range, validate sample addresses against the range, and use a second trace window for distinct lookup-index regions. Preserve all reported range/cap metadata and use pre/post reference/cache values to distinguish actual control-flow changes.
- **Acceptance:** Audit existing Script Console/tracer support for dynamic address-range coverage; ideally warn when computed table accesses exceed a declared watch range or offer an opt-in PC-filtered lookup trace covering multiple address windows, with explicit range-exclusions and no misleading "no hits means branch skipped" summaries. Reproduce the Run 021 upper case with tracing near `0x580F8` and test accepted/rejected cases without weakening trace caps. Group with LIM-003/004 only if new instrumentation is genuinely necessary.
- **Release triage:** Backlog intake only, not a request for a new release. The immediately usable workaround is Run 022 with an upper-index trace window.

### LIM-011 — Trace event cap saturation without definitive dropped-event accounting

- **Status:** OBSERVED CAP-REACHED / overflow status NEEDS AUDIT; **priority:** P2; **target:** TBD.
- **Evidence:** Run 024 (`bundle(20261010-103200).zip`), Phase A, traced CPU-A `0x10A084` read/write over 13 frames with `limit=128`. Both control and lateral+0x40 traces returned **exactly `events=128 limit=128`**, final stored event on frame 5599. The result does not expose an explicit lost/dropped count, and reaching the cap does **not** by itself prove that an event was lost. The trace adequately captured the early intervention overwrite but cannot certify complete coverage beyond the cap.
- **Impact:** Longer provenance sweeps may silently appear complete despite cap saturation, weakening claims that no later readers/writers existed.
- **Workaround:** Prefer shorter scoped windows, raise per-pass caps proportionately where supported, examine the last event frame and the `events/limit` status, and label results at cap **coverage uncertain** rather than *no overflow*.
- **Acceptance:** Trace status and structured bundle metadata differentiate `events_recorded`, `events_seen`, `events_dropped`, `limit_reached` (and first dropped frame/PC if available), plus visible Workbench warnings. Regression exercises a deliberately overfull 128-event trace and proves warnings and output completeness expectations. Do not increase limits blindly or suppress saturation signals.
- **Dependencies:** Audit current `memory.trace` implementation; group with LIM-003/LIM-004 if warranted. No standalone RC.


### LIM-012 — Checkpoint reference validation before running Workbench scripts

- **Status:** CONFIRMED SCRIPT VALIDATION GAP / capability audit required; **priority:** P2; **target:** TBD.
- **Evidence:** Run 026 (`bundle(20261010-104649).zip`; diagnostic `diagnostic-20261010-114635-458.zip`) completed the frame-5587 cache scenario but failed on `checkpoint.load file=stage1-target-near-destruction-9548.chqstate`. RC4.3.2 already ships `stage1-target-post-final-hit-9548.chqstate` and explicitly documents why that historic name was removed: frame 9548 is **post-final-hit**, not a near-destruction/pre-hit state. Workbench accepted/started a long multi-checkpoint script before discovering the nonexistent literal checkpoint on the second scenario. `try/finally` correctly cleared tracing and restored frame 2064.
- **Impact:** Wasted investigator time and incomplete evidence bundles; silently substituting a state with different gameplay semantics would corrupt cross-checkpoint claims.
- **Workaround:** Consult `api checkpoint.list` and `checkpoints/README.md`; validate all literal `checkpoint.load` references against the current release's checkpoint catalogue before submitting a multi-scenario script; document post-final-hit vs active-target roles. Run 027 references six names verified against the RC4.3.2 packaged corpus.
- **Acceptance:** In Script Console validation and Go CLI/script preflight, parse literal `checkpoint.load file=...` references (including tuple-list loop substitutions) and fail **before any commands execute** if a file does not exist, reporting its name, line and available near matches with semantic descriptions. Support dynamic values with an explicit unresolved-at-preflight notice and safe runtime failure; do not silently resolve a historic alias to a different-state checkpoint. Regression covers a nonexistent second checkpoint following a valid first one and asserts zero prior frame execution, no dirty trace, and clear explanation. Audit whether existing `checkpoint.list`/schema facilities can implement this without new native instrumentation.
- **Dependencies:** Script compiler/Go preflight, canonical checkpoint catalogue, LIM-006 documentation consistency. **Backlog only; no standalone release.**

## Run 024 evidence intake (2026-10-10)

- **Bundle:** `bundle(20261010-103200).zip`, run ID `024`, RC4.3.2; `436` commands PASS in `139824` ms; integrity `ok=true`; cleanup restored frame 2064 paused, no trace active or fault.
- **Causal matrix:** 20 replay branches: depth offsets `-0x200, 0, +0x200, +0x400` crossed with lateral offsets `-0x40,-0x20,0,+0x20,+0x40`. All 20 map13/palette81 sprites at frame5590 had slot70.
- **X by depth, across five lateral offsets:** near `[163,166,169,172,175]`; base `[160,163,165,167,169]`; far+0x200 `[159,160,162,164,166]`; far+0x400 `[157,158,159,161,162]`.
- **Constant size/Y within each depth row:** near 99x99/Y125; base 77x77/Y128; far+0x200 59x59/Y131; far+0x400 48x48/Y132.
- **Source producer:** `0x43828` writes prepared sprite word `0x1027C4`, varying with lateral input; `0x4382A` writes the coordinate/scale words at `0x1027C0/0x1027C2`, constant across lateral perturbations at matched depth in frame5587. Later writes may reflect gameplay overwrites.
- **Lateral state writers:** original CPU-A `0xA13C` and `0x4FE4` write `0x10A084`; both early changes converge to the same normal state by frame5588. Phase A caps reached exactly 128/128, so full 13-frame trace coverage is uncertain (LIM-011).
- **Interpretation:** This supplies 20 checked observable geometry examples, **not** a complete fixed-point mathematics or instruction-boundary register contract. Distinguish immediate sprite response from downstream gameplay dynamics, and do not claim native replacement or Shadow verification.

## Run 015 evidence intake (2026-10-10)

- **Bundle:** `bundle(20261010-093625).zip`; source run ID `015`; build `0.67.0.0-RC4.3.2`; 150/150 commands PASS; 54,506 ms; verified archive; final paused checkpoint frame 2064; cleanup success.
- **Dispatch disassembly:** `0x40ED6 jmp (A6,D1.w)`, table starting `0x40EDA`, including targets `0x434E8` and `0x443AA`. This supports multiple renderer paths, not one universal routine contract.
- **Control traces:** presentation field `0x10A0B0–BF`: 122 events; prepared output `0x102760–85F`: 560 writes; ROM reads `0x54100–542FF`: 77.
- **Each modified branch (`+0x400`, `-0x400`, `+0x800`):** presentation 117 events and output 560 writes; no cap reached. Positive-400 ROM-table pass `0x54100–542FF`: 57 reads (vs 77 control); second lookup `0x66874–66A73`: 190 reads in control and +400, but address distribution differs.
- **Interpretation:** Inputs perturb dispatch/table access patterns, while total writes can remain identical. The run **does not establish entry/return register state, a bounded independent native routine contract, or that each writer exclusively belongs to the target car**. Don't promote Native Execution Bridge SHADOW/REPLACE on this basis.

## Intake template for subsequent runs

```
ID: LIM-###
Run/bundle + exact evidence:
Status: OBSERVED / NEEDS-AUDIT / CONFIRMED / DEFERRED / IMPLEMENTED / VERIFIED
Research impact + workaround:
Proposed acceptance test:
Priority / grouping dependencies / target release: TBD
```
