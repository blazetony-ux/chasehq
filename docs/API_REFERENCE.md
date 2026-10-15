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

> **AUTHORITATIVE CURRENT — v0.67.0.0-RC6.3.14 CPU-B DUAL-CANDIDATE DISPATCH CORRECTIVE (2026-10-14).** Actual RC6.3.13 Windows build PASS and Ghidra pair ABI PASS, but BEQ live acceptance BLOCKED in the Go verifier: 22 valid inactive pair original-ROM events were rejected despite 17,892 single-BEQ native commits. RC6.3.14 corrects only the verified audit/proof contract, adds inverse strict pair-mode audits and mutation regressions; **Windows RC6.3.14 genuine BEQ+pair live three-mode parity PENDING**. No whole-function or autonomous promotion; inherited Go Full Regression structural count 1,176 >850 remains FAIL. See `docs/RC6.3.14_CPU_B_DISPATCH_AUDIT_CORRECTIVE.md`. Older CURRENT banners are historical.

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

**CURRENT RELEASE CANDIDATE: 0.67.0.0-RC6.2.4.** Real opt-in CPU-A `0x820C` guarded generated-library native instruction replacement and original/native/rollback full-state acceptance; Windows and ROM-backed proof pending. Enclosing Ghidra function still unreviewed. See `RC6_STATUS.md` and `docs/RC6.2.4_M1_M7_ACCEPTANCE.md`. All prior “current” paragraphs below are **historical only**.

> **AUTHORITATIVE CURRENT STATUS — 0.67.0.0-RC6.2.2 SOURCE CANDIDATE (2026-10-12).** RC6.2.1 Windows qualification PASS (SHA256 `c8422141d9d079ce522f0b1dc12a95527c5d80e9055b4c874fb356493b46174b`); actual CPU-A Ghidra steering export `0x820C` PASS. RC6.2.2 adds self-contained, fail-closed M1–M7 **acceptance evaluation**, not automatically proven unrestricted native replacement. Full acceptance and Windows qualification must be tested; see `docs/RC6.2.2_M1_M7_ACCEPTANCE.md` and `RC6_STATUS.md`. Prior CURRENT/unqualified statements below are preserved as **historical snapshots, not current release status**.

> **CURRENT RC6.1 DEVELOPMENT — v0.67.0.0-RC6.2.1.1; native steering shadow opt-in, Windows qualification outstanding.** See `RC6_STATUS.md`; older status below is historical.

> **RC6 DEVELOPMENT STATUS (not qualified):** v0.67.0.0-RC6. RC5.3 sections below are historical. See `RC6_STATUS.md` at release root.

> **CURRENT: v0.67.0.0-RC5.3 native sprite-clear SUBSTITUTION DEVELOPMENT CANDIDATE (UNQUALIFIED), 2026-10-11.** The original Windows-qualified RC5 parent SHA-256 `47def65d75056fb765d5b60dfe5a4c4a21bdcd3214ebb84920dd62a0d8b37ddf` remains immutable. RC5.1 headless live-shadow corpus PASS 12/12 cases (96/96 sprite transfers and 24/24 class-6 road generator comparisons); RC5.3 adds an experimental, opt-in native substitution of the trailing sprite-clear DBRA loop ONLY. Bulk copier and final RTS remain original ROM; normal emulation is default. RC5.3 Windows native A/B equivalence and exact-archive qualification pending. See `docs/RC5.3_NATIVE_CLEAR_SUBSTITUTION.md`; all older release-status headings below are historical.

> **RC5 SOURCE CANDIDATE (`0.67.0.0-RC5`), NOT WINDOWS-QUALIFIED.** Original RC4.7 source archive supplied by user; RC5 integrates offline native C++ sprite/road candidates and standalone synthetic Windows preview, updates live Workbench Next Steps. No live SHADOW/COMPARE or native replacement is implemented. Follow `docs/RC5_NATIVE_PREVIEW_AND_SHADOW.md` and exact Windows `chqdev.bat qualify --fresh` gate before promotion. Older version status sections below are historical.

## RC4.7 — Static Analysis / REA tooling candidate

See `docs/REA_GHIDRA_RC47.md` and `docs/DOCUMENTATION_AUDIT_0.67.0.0-RC4.7.md`. **Current candidate RC4.7; promoted parent RC4.6. Windows qualification pending.** Dedicated Workbench REA workspace, bounded `ghidra.wait`, Ghidra run evidence attachment, corrected result path, Java/Jython preflight and flow-edge export. No Capstone library is shipped in this candidate; source vendoring is blocked pending a verified upstream archive. No native emulator CPU replacement.

> **AUTHORITATIVE PROMOTED BASELINE — v0.67.0.0-RC4.6 (2026-10-10).** Windows `chqdev.bat qualify --fresh` reports **11/11 stages PASS** for exact frozen ZIP SHA-256 `9e76a107e19f3e28a54a8a8b3bdaed630511e02f1da3e3e7ae274c07d863580f`. User explicitly authorised promotion following focused, full, and successful headless Ghidra/REA crosswalk evidence. This is a **post-qualification documentation overlay**; it does not change the immutable tested archive. See `docs/RELEASE_VERIFICATION_RC46.md`, `promotion/PROMOTION_RECORD.json`, and `docs/REA_GHIDRA_NEXT_SCOPE.md`. Earlier RC4.6 “candidate/unqualified” statements below are **historical build-time notes, not current release status**.

> **Current development candidate:** `v0.67.0.0-RC4.4`. Windows exact-archive qualification pending. See `docs/NATIVE_PROJECTION_RC44.md` for present scope and limitations.

## v0.67.0.0-RC4.3.2 CURRENT DEVELOPMENT CANDIDATE — NOT QUALIFIED

This source candidate extends v0.67.0.0-RC4.2.3. Its authoritative new feature and limitation inventory is `docs/NATIVE_MIGRATION_RC43.md`. The prior RC4.2.3 Windows debug compile was PASS, but it does **not** qualify RC4.3. Required before promotion: exact-archive Windows `chqdev.bat qualify --fresh` including strict CTest, focused and full regressions, deterministic package verification and readiness. The known pre-existing Full Regression Go structural budget check remains FAIL (1,170+ commands versus <850); do not delete unique assertions or relax that gate to conceal the failure. New code paths MUST keep native replacement disabled by default. User-controlled ROM, SDL and content paths stay outside release ZIPs. Fresh release setup MUST find the existing stable `build-local.json` or provide a precise doctor error. The bundled Windows chqdev executables MUST be present, tested and accompanied by matching release metadata.

> Current development candidate: **v0.67.0.0-RC4.2.3.1** — CHQPACK Browser, overlay inspection and provenance. Windows qualification pending.

> Current development candidate: **v0.67.0.0-RC4** — ROM/CHQPACK source switching and RC3.1 release consistency corrections.

> Current development candidate: **v0.67.0.0-RC3** — CHQPACK runtime content boundary; ROM-free runtime after deterministic import; RC2.1 native rendering retained.

## v0.67.0.0-RC2.1 native display backends

- `api backend.sprite.get/set/reset-verification` — sprite subsystem is NATIVE by default; Original/Shadow/Native-only remain verification modes.
- `api backend.text.get/set/reset-verification` — TC0100SCN text/HUD is NATIVE by default after the RC1.1 zero-diff + Native-only promotion corpus.
- `api backend.bg.get` — read independent TC0100SCN BG0/BG1 Shadow/native per-layer counters.
- `api backend.bg.set mode=original|shadow|native|native-only` — select the BG renderer backend; RC2 defaults to Shadow.
- `api backend.bg.reset-verification` — clear BG0/BG1 comparison and first-divergence counters.
- `api native.progress` — reports sprites/text as Native and live BG promotion state.

> Exact source baseline: qualified **v0.67.0.0-RC1.1** SHA-256 `ea4ebaafe07cd1c15b64c07ed8e92e32fd0e39cdcc360e31dd7b09c676b77f1d`. RC2 keeps the tooling freeze and moves visible rendering forward. See `docs/DOCUMENTATION_AUDIT_0.67.0.0-RC2.1.md`.

> Historical qualified candidate: **v0.67.0.0-RC1.1** — native sprites default; text/HUD Shadow zero-diff bring-up; native sprite forensic overlap-coverage corrective.

> Current API build: **v0.66.9.5-RC3**.

## Native Promotion Framework APIs (v0.66.9.5-RC3)

- `api backend.sprite.set mode=original|shadow|native|native-only` — `native-only` forbids silent Original fallback.
- `api native.promotion.reset` — reset sprite promotion/verification counters.
- `api native.promotion.status` — machine-readable live promotion counters/readiness.
- `api native.promotion.assert minShadowFrames=N minNativeOnlyFrames=N` — fail-closed promotion gate requiring clean Shadow and Native-only execution.
- `api native.layer.capture name=NAME` — same-state Original/Native/diff layer evidence.
- `api native.progress` — Workbench/project maturity matrix plus live sprite promotion state.

A first Shadow mismatch automatically writes Original/Native/diff evidence under the session `native_promotion` area.

> Historical candidate: **v0.66.9.5-RC2**. See `docs/DOCUMENTATION_AUDIT_0.66.9.5-RC2.md` for Runs003-012 evidence and RC2 changes.

## RC2 graphics additions

`api sprite.rom-atlas.export start=450 end=620 format=all palette=65 name=vehicle-range` decodes map definitions directly from the loaded Chase H.Q. spritemap/OBJ ROM regions and writes raw-pen/palette PNGs, CSV provenance, contact sheets, manifest and ZIP. `sprite.inspect` / `sprites.list` now expose source and effective presentation identities plus override provenance.

> Current development candidate: **v0.66.9.5-RC1.1**, a narrow Workbench web-bridge hotfix built from the exact Windows-qualified **v0.66.9.5-RC1** archive SHA-256 `26bda3287a3256fe8bf2c46aaaafba68546c02d2e1922465abcbacb0a62d262e`. RC1.1 changes no intended emulation behaviour and requires exact-archive Windows `chqdev qualify --fresh` before promotion.

> Current release: **v0.66.9.4**. Promoted from exact Windows-qualified **v0.66.9.4-RC3.1.3** SHA-256 `48534c0bd3d8e4a40987e65089b07046535976675ca2e36d0ebf3a6d9584383e`. The final archive is regenerated for release identity/docs/focused-regression wiring only and requires one exact-final-archive `chqdev qualify --fresh` closure before immutable publication.

> Current development candidate: **v0.66.9.4-RC3.1.2**. RC3.1.2 keeps the proven cross-format yellow-target renderer but binds it to semantic `player_car.body` across steering/orientation maps, expands SPEED suppression to the complete live destination block, and retains fail-closed proof plus chqdev readiness/console UX. Exact-archive Windows `chqdev qualify --fresh` remains required.

> Current development candidate: **v0.66.9.4-RC3**. Source lineage remains the immutable promoted **v0.66.9.3** baseline (SHA-256 `6da88889dc8214c6058644e44e4d29f6a2117c9f0e92d99c8d906c280fc3462e`); **v0.66.9.4-RC2** (`50a8fb6d2c4be5fdafdea5b3804d6f2e1f1e5e62685160d3dc6f6d65237a6f41`) passed fresh Windows qualification. RC3 adds format-aware cross-OBJ native asset replacement, the first player-car → Stage-2 yellow-target preset, corrected dynamic SPEED HUD suppression, and expanded native/font tooling. RC3 requires exact-archive Windows `chqdev qualify --fresh` before promotion.

## v0.66.9.4-RC3 native / asset actions

- `backend.sprite.get` - read current Original/Shadow/Native mode and live comparison counters.
- `backend.sprite.set mode=original|shadow|native` - switch the live sprite source; Original remains the default/oracle.
- `backend.sprite.reset-verification` - clear accumulated live Shadow comparison/first-divergence status.
- `sprite.map-override source=N target=N [palette=-1..255]` - presentation-only global existing-OBJ substitution across transient slots.
- `sprite.map-override.clear` - remove all global map substitutions.
- `sprite.asset-replace source=N|object=player_car.body target=N format=preserve|obj-a|obj-b64|obj-b32 [palette=-1..255] [flipx=-1|0|1] [flipy=-1|0|1] [dx=N] [dy=N]` - format-aware Native replacement across OBJ classes; semantic player-body mode persists across the known steering/orientation body family; automatically selects Native.
- `sprite.asset-replace.clear` - remove all format-aware asset replacements.
- `text.charset.export [palette=N] [scale=1..8]` - export all 256 live TC0100SCN RAM characters, atlas, glyph PNGs, raw RAM, usage JSON and ZIP.

## v0.66.9.4-RC1 experimental controls

- `game.beacon.get` — read current beacon override plus phase/timer/palette/request state. Example: `api game.beacon.get`.
- `game.beacon.override mode=authentic|on|off` — remove the intervention, hold deployed phase 6, or hold phase 0. Example: `api game.beacon.override mode=on`.
- `hud.layout.get` — read semantic HUD presentation state.
- `hud.layout.set element=high-score|time|distance|score|speed|turbo|stage visible=true|false dx=N dy=N` — hide/show and reposition one HUD group; offsets are pixels and bounded to the screen dimensions.
- `hud.layout.reset` — restore authentic HUD presentation.
- `hud.layout.preset name=authentic|minimal|none` — apply a named HUD preset. Minimal keeps Time, Distance and Turbo markers.

Native command equivalents are `beacon status`, `beacon override authentic|on|off`, `hud status`, `hud set ELEMENT visible|hidden DX DY`, `hud reset`, and `hud preset authentic|minimal|none`. HUD operations are presentation-only and do not write TC0100SCN RAM.

> Current release: **v0.66.9.3**. Promotion provenance is the exact Windows-qualified `v0.66.9.3-RC5` archive SHA-256 `19b7ed48edae98fd99f660ef56a949573050cabc2958fcdba2833a7888103961`. That candidate passed 8/8 CTest, deterministic package/verify, runtime readiness, focused regression, Full Regression and `promotable`; the final promotion changes release identity/documentation/focused-regression wiring and adds a release-package metadata guard only; it does not intentionally change emulation/gameplay behaviour.


# RC2.9 API additions
## v0.66.9.3 current research API additions

- `api sprite.layer-shadow.compare [failOnMismatch=true|false] [output=NAME.json]` compares the authoritative placed sprite source against an independent raw-RAM/spritemap/gfx/palette/zoom/flips/clipping/ownership renderer. The canonical Windows corpus is SHADOW VERIFIED.
- `api memory.trace.start ... access=read|write|rw` supports bounded read provenance as well as write tracing.
- `api collision.events [count=N]` / `api collision.events.clear` expose structured mapped collision-response events with interaction/object context and before/attempted/applied values.
- `api audio.events [count=N]` / `api audio.events.clear` expose structured writes to the existing `0x820000..03` command interface; this is not sound CPU/chip/mixer emulation.
- `api image.compare` reports RGB plus RGBA/alpha/occupancy differences.
- IOC sweeps retain gameplay/track/input state, and `input.steering.set` rejects values outside the signed-12-bit raw domain (`0000..0FFF`).

- `api sprite.shadow.compare scope=visible|slot [slot=N] [failOnMismatch=true|false]` compares authoritative live sprite 4bpp decode against the independent shadow decoder without changing renderer state. HTTP parity: `GET /api/v1/sprite/shadow-compare`.
- `api query.runtime function=getmaps()` (and the documented supported `get*` family) evaluates the query at execution time rather than browser preflight.
- `api corpus.run checkpoints=a,b action=ACTION ...` runs one action across 1..32 packaged checkpoints and restores the caller machine state. Validation mode resolves/checks inputs without mutating runtime state.
- Authoritative run bundles use `chq-run-bundle-manifest-v3` with per-file byte length/SHA-256 and `chq-run-bundle-index-v2`. RC3 normalizes manifest/index/ZIP paths to `/`, writes core run metadata as UTF-8 without BOM, records client-only waits authoritatively, preserves authored + resolved script sources, and emits `resolved-command-map.jsonl` with execution sequence and resolved-source line. Finalization remains fail-closed with `FINALIZATION_FAILED` cleanup.
- `api corpus.run ...` rows expose a parsed `result` object when the nested action returns JSON; non-JSON output is retained as `raw`. Browser Script Console finalization sends its executed-command total so client-side `wait` commands are included in authoritative metadata.


## v0.66.9.1 release-process note

`chqdev` is the de facto and authoritative build/release process. Fresh Windows promotion uses `chqdev qualify --fresh`; zero tolerated CTest failures and exact deterministic archive/regression proof are mandatory. This does not change the Research Script v2 grammar.

## RC2.7 public contracts

The full release label comes from `src/version.h`. These actions share native control paths and are exposed in the Script Console allow-list and authoritative schema. Layer offsets are presentation overrides, default zero, bounded -4096..4096, and do not change hardware RAM. `tc0100.mode` is an old/corrected diagnostic, not shadow verification.

- `layer.offsets.get` — Read live presentation offsets for all five layers. Parameters: none. Example: `api layer.offsets.get`.
- `layer.offsets.set` — Set reversible live presentation offsets; hardware state is unchanged. Parameters: layer=bg0|bg1|text|sprites|road|all required, x=-4096..4096 required, y=-4096..4096 required. Example: `api layer.offsets.set layer=bg0 x=0 y=4`.
- `layer.offsets.reset` — Reset presentation offsets to zero. Parameters: layer=bg0|bg1|text|sprites|road|all optional default all. Example: `api layer.offsets.reset layer=all`.
- `layer.offsets.assert` — Assert live layer offsets against expected values. Parameters: layer=bg0|bg1|text|sprites|road required, x=INTEGER required, y=INTEGER required. Example: `api layer.offsets.assert layer=bg0 x=0 y=4`.
- `tc0100.mode` — Select diagnostic old/new X transform; this is not independent shadow verification. Parameters: mode=legacy|corrected optional. Example: `api tc0100.mode mode=corrected`.
- `release.assert-version` — Require native status, Workbench and feature manifest to share the full release label. Parameters: none. Example: `api release.assert-version`.
- `frame.snapshot.assert-exact` — Require exact=true, verifiedExact=true and zero reconstruction mismatches. Parameters: name=NAME required, requireColumnZero=true|false optional default false. Example: `api frame.snapshot.assert-exact name=rc27-corrected`.
- `frame.snapshot.assert-scene-no-sprites` — Require exact reconstruction, visible sprite ownership and changed no-sprites pixels. Parameters: name=NAME required. Example: `api frame.snapshot.assert-scene-no-sprites name=rc27-corrected`.

Native CLI equivalents: `chqctl layer-offset get`, `chqctl layer-offset set bg0 0 4`, `chqctl layer-offset reset all`, `chqctl tc0100-mode legacy|corrected`. Existing startup `--layer-offset`/config `layer_offset` remain supported. Script API actions can also be submitted through the documented HTTP script endpoint; no new dedicated REST routes are claimed.

## v0.66.9.0-RC1 Game Lab / tile actions

New scriptable actions: `game.experiment.status`, `game.turbo.stock`, `game.turbo.active`, `game.turbo.fire`, `game.turbo.timer.reset`, `game.turbo.remaining.set`, `game.speed.set`, `game.speed.freeze`, `game.speed.clear`, and `tile.inspect`. Game Lab actions are reversible and target confirmed Chase H.Q. state. `tile.inspect` currently covers TC0100SCN **RAM text characters** (256 x 8x8, 2bpp at tilemap character RAM `0xC06000`); background ROM tile inspection remains future work. See the machine-readable `research/schema/api-actions.json` for the exact live contract.


### Workbench convenience endpoints (v0.66.9.0-RC1)

The Game Lab UI is a convenience front end over the same script actions. `POST /api/v1/game/action` accepts an `action` plus action-specific fields and maps to the documented `game.*` actions. `POST /api/v1/tile/inspect` accepts `code`, `palette`, and `scale` and maps to `tile.inspect`. Automation should prefer the Research Script/API action names so CLI/script/UI behaviour stays aligned.

# ChaseHQ-Native Script API Reference — v0.66.9.4-RC2
> **Current candidate:** v0.66.9.4-RC2. Promoted baseline is exact v0.66.9.3 SHA-256 `6da88889dc8214c6058644e44e4d29f6a2117c9f0e92d99c8d906c280fc3462e`; fresh exact-archive qualification remains `chqdev qualify --fresh`.


**102 documented actions.** This file is generated from the same `$scriptActionSchemas` table used by the running Workbench.

> Maintenance rule: implementation + schema + documentation + regression must change together. Never infer or guess action names.

Runtime discovery: `api actions`, `api schema action=NAME`, `api schema.all`, or `GET /api/v1/schema/actions`.

## Generic memory-write tracing (v0.66.9.0-RC2.9)

`memory.trace.*` captures writer provenance at the `Bus::write` transaction boundary. It is game-agnostic and records matching writes even when the value does not change.

### `memory.trace.start`

**Example:** `api memory.trace.start cpu=A address=0x100303 length=1 width=8 limit=256`

Parameters: `cpu=A|B`, `address=HEX`, optional `length` (default 1), optional `width=8|16|32|any` (default `any`), and optional bounded `limit` (64..1000000, default 4096). A write matches when its transaction overlaps the selected range and, when specified, its write width matches.

Each event records frame, CPU, exact writer PC, write address, width, old value, new value, changed flag and trace-local write count.

### `memory.trace.status` / `memory.trace.tail` / `memory.trace.stop` / `memory.trace.clear`

Examples: `api memory.trace.status`, `api memory.trace.tail count=50`, `api memory.trace.stop`, `api memory.trace.clear`. `stop` retains events; `clear` removes events and resets the trace-local write count.


## Palette write tracing (v0.66.4)

Palette tracing is semantic TC0110PCR provenance, not a generic memory watch. It records the complete palette transaction after the low-byte commit and retains matching writes even when the raw value does not change. This is intended for exact game-code provenance such as the Chase H.Q. brake-lamp palette path.

### `palette.trace.start`

Start bounded tracing for one or more palette indices. Starting a new trace clears prior palette-trace events.

**Example:** `api palette.trace.start indices=1037,1053 limit=4096`

**Parameters:**

- `indices=comma-separated palette indices 0..4095 required`
- `limit=64..1000000 optional default 4096`

Each event records: frame, CPU, exact writer PC, palette index, bank, pen, old raw value, new raw value, changed flag, and cumulative write count.

### `palette.trace.status`

**Example:** `api palette.trace.status`

Returns whether capture is enabled, selected indices, buffered event count, and history limit.

### `palette.trace.tail`

**Example:** `api palette.trace.tail count=50`

**Parameters:** `count=N optional default 20`

Returns the most recent matching writes.

### `palette.trace.stop`

**Example:** `api palette.trace.stop`

Stops capture while retaining buffered events.

### `palette.trace.clear`

**Example:** `api palette.trace.clear`

Clears buffered palette-write events without changing the selected filter.

## `status`

Read machine/frame status

**Example:** `api status`

**Parameters:** none

## `capabilities`

List native debugger capabilities

**Example:** `api capabilities`

**Parameters:** none

## `memory.read`

Read one value

**Example:** `api memory.read cpu=A address=10A048 width=8`

**Parameters:**

- `cpu=A|B required`
- `address=HEX required`
- `width=8|16|32 optional default 16`

## `memory.read-range`

Read a contiguous byte range and return a compact hex dump

**Example:** `api memory.read-range cpu=A address=109000 length=512`

**Parameters:**

- `cpu=A|B required`
- `address=HEX required`
- `length=COUNT required, decimal or 0xHEX, max 65536`

## `memory.write`

Debugger-safe write; pauses machine and reports old/new values

**Example:** `api memory.write cpu=A address=10A04A width=16 value=0051 expect=0011`

**Parameters:**

- `cpu=A|B required`
- `address=HEX required`
- `width=8|16|32 optional default 16`
- `value=HEX required`
- `expect=HEX optional compare-before-write`

## `register.read`

Read one register or complete CPU register set

**Example:** `api register.read cpu=A name=D0`

**Parameters:**

- `cpu=A|B required`
- `name=D0..D7|A0..A7|PC|SR|SP optional`

## `register.write`

Write a CPU register while paused

**Example:** `api register.write cpu=A name=D0 value=00000001`

**Parameters:**

- `cpu=A|B required`
- `name=REGISTER required`
- `value=HEX required`

## `patch.freeze`

Immediately write and continuously freeze a value; returns patch ID

**Example:** `api patch.freeze cpu=A address=10A04A width=16 value=0051`

**Parameters:**

- `cpu=A|B required`
- `address=HEX required`
- `width=8|16|32 required`
- `value=HEX required`
- `expect=HEX optional`

## `patch.replace`

Replace matching game writes with a fixed value

**Example:** `api patch.replace cpu=A address=10A04A width=16 value=0051`

**Parameters:**

- `cpu=A|B required`
- `address=HEX required`
- `width=8|16|32 required`
- `value=HEX required`

## `patch.suppress`

Suppress writes and preserve current value

**Example:** `api patch.suppress cpu=A address=10A04A width=16`

**Parameters:**

- `cpu=A|B required`
- `address=HEX required`
- `width=8|16|32 required`

## `patch.list`

List active live patches

**Example:** `api patch.list`

**Parameters:** none

## `patch.remove`

Remove one live patch

**Example:** `api patch.remove id=1`

**Parameters:**

- `id=N required`

## `patch.clear`

Remove all live patches

**Example:** `api patch.clear`

**Parameters:** none

## `memory.export`

Export a contiguous debugger-safe byte range under the evidence root

**Example:** `api memory.export cpu=A address=109000 length=512 output=evidence\\stage1-track-109000.bin`

**Parameters:**

- `cpu=A|B required`
- `address=HEX + length=COUNT OR start=HEX + end=HEX`
- `output=relative evidence path optional`
- `name=basename optional legacy alias`
- `max 1 MiB`

## `cpu.disassemble`

Read-only disassembly

**Example:** `api cpu.disassemble cpu=A address=008120 count=64`

**Parameters:**

- `cpu=A|B optional default A`
- `address=HEX required`
- `count=N optional default 16`

## `control.run-frames`

Run exactly N emulated frames and pause; cancellation-aware

**Example:** `api control.run-frames frames=100`

**Parameters:**

- `frames=1..100000 required`

## `control.abort`

Out-of-band pause/cancel of active frame execution

**Example:** `api control.abort`

**Parameters:** none

## `checkpoint.load`

Load a packaged deterministic checkpoint

**Example:** `api checkpoint.load file=stage1-gameplay-2064.chqstate run=false`

**Parameters:**

- `file=packaged .chqstate required`
- `run=true|false optional default false`

## `checkpoint.save`

Save current deterministic state into packaged checkpoints and write metadata

**Example:** `api checkpoint.save file=stage1-gradient-test.chqstate role=research`

**Parameters:**

- `file=packaged filename required`
- `name=display name optional`
- `role=role optional`
- `description=text optional`

## `frame.snapshot`

Capture a reconstructable layered frame snapshot. v0.66.6.1 self-verifies the alpha-over reconstruction and fails capture on any pixel mismatch; `reconstruction.json` exposes `verifiedExact` and `mismatchedPixels`. In v0.66.7, v2 additionally writes `scene-no-sprites.png`, `sprites/sprites.json`, and cropped transparent `sprites/slot_NNN.png` forensic assets for visible sprites, while retaining the aggregate sprite contribution as the authoritative exact-reconstruction layer. The per-sprite assets are intended for Graphics Lab hide/show, solo, opacity and provenance inspection. The snapshot also writes `final.png`, `final-no-hud.png`, `hud-only.png`, exact alpha contribution layers under `layers/`, raw hardware-source renders under `sources/`, a complete `palette.csv`, and `manifest.json`.

**Example:** `api frame.snapshot name=stage1-bad-foreground`

**Parameters:**

- `name=optional snapshot name`

The HUD model is source-aware: Chase H.Q. currently maps HUD isolation to the TC0100SCN text layer. No fixed screen rectangle is masked. `reconstruction.json` records compositor selection, layer order and the files required to rebuild the captured final frame.

**Workbench HTTP v1.9 snapshot routes:** `GET /api/v1/frame/snapshots`, `GET /api/v1/frame/snapshot/{name}/manifest`, and `GET /api/v1/frame/snapshot/{name}/asset/{path}`. The manifest route returns both `manifest.json` and parsed reconstruction metadata; the asset route serves the snapshot PNG/JSON/CSV files used by Graphics Lab.

## `frame.snapshot.list`

List structured frame snapshots. RC2.4.3 adds an optional assertion for same-turn discovery.

**Example:** `api frame.snapshot.list require=stage1-study`

**Parameters:**

- `require=NAME` optional; throw unless exactly one snapshot named `NAME` is present in the returned list.

Omit `require` to retain the normal non-assertive listing behavior.

## `regions.list`

Return authoritative image regions as structured JSON

**Example:** `api regions.list`

**Parameters:** none

## `profiles.list`

List packaged game-profile descriptors; metadata only and does not switch runtime

**Example:** `api profiles.list`

**Parameters:** none

## `profile.get`

Read one packaged game profile and whether it is active/activatable

**Example:** `api profile.get id=sci`

**Parameters:**

- `id=profile id optional default chasehq`

## `sprites.list`

Return current live composed sprite records as structured JSON

**Example:** `api sprites.list`

**Parameters:** none

## `sprites.maps`

Return unique live sprite map IDs

**Example:** `api sprites.maps`

**Parameters:** none

## `sprites.palettes`

Return unique live sprite palette IDs

**Example:** `api sprites.palettes`

**Parameters:** none

## `sprite.inspect`

Inspect one currently active sprite slot

**Example:** `api sprite.inspect slot=44`

**Parameters:**

- `slot=N required`

## `sprite.map.inspect`

Inspect raw spritemap/chunk provenance and raw pen histogram for one active sprite slot

**Example:** `api sprite.map.inspect slot=45`

**Parameters:**

- `slot=N required`

## `schema.all`

Return every Script API action contract from the authoritative in-process schema

**Example:** `api schema.all`

**Parameters:** none

## `script.schema`

Return the authoritative Research Script v2 grammar and limits

**Example:** `api script.schema`

**Parameters:** none

## `ui.prompt`

Show an SDL-native prompt overlay and wait for Enter/Escape while physical gameplay keys remain live

**Example:** `api ui.prompt message=Hold_DOWN_then_press_ENTER pause=true top=true`

**Parameters:**

- `message=TEXT required; underscores render as spaces`
- `pause=true|false optional default true`
- `top=true|false optional default true`
- `timeout=1000..3600000 optional default 300000`

## `sprite.override`

Presentation-only live SDL sprite override; omitted fields retain original values

**Example:** `api sprite.override slot=44 map=182`

**Parameters:**

- `slot=N required`
- `map=N optional`
- `palette=N optional`
- `visible=true|false optional`

## `sprite.override.clear`

Clear all live sprite presentation overrides

**Example:** `api sprite.override.clear`

**Parameters:** none

## `sprite.visual`

Live SDL sprite isolation/highlight mode

**Example:** `api sprite.visual slot=44 mode=flash`

**Parameters:**

- `slot=N required`
- `mode=solo|hide|flash required`

## `sprite.visual.clear`

Clear sprite solo/hide/flash presentation mode

**Example:** `api sprite.visual.clear`

**Parameters:** none

## `palette.entries`

Return the 16 palette entry indices for a bank

**Example:** `api palette.entries bank=75`

**Parameters:**

- `bank=0..255 required`

## `state.snapshot`

Capture coherent structured research state from the current paused instant

**Example:** `api state.snapshot include=sprites,regions,gameplay,track`

**Parameters:**

- `include=sprites,regions,gameplay,track optional`

## `input.ports`

Read current IOC/input port state

**Example:** `api input.ports`

**Parameters:** none

## `events.tail`

Return recent research events

**Example:** `api events.tail count=50`

**Parameters:**

- `count=N optional default 100`

## `sprites.tail`

Return recent semantic sprite-change events

**Example:** `api sprites.tail count=100`

**Parameters:**

- `count=N optional default 100`

## `control.pause`

Pause emulation

**Example:** `api control.pause`

**Parameters:** none

## `control.resume`

Resume emulation

**Example:** `api control.resume`

**Parameters:** none

## `control.step-frame`

Advance paused emulation by N frames

**Example:** `api control.step-frame frames=1`

**Parameters:**

- `frames=N optional default 1`

## `control.step-instruction`

Step one CPU instruction

**Example:** `api control.step-instruction cpu=A`

**Parameters:**

- `cpu=A|B optional default A`

## `timer.hold`

Freeze or resume the game timer helper

**Example:** `api timer.hold enabled=true`

**Parameters:**

- `enabled=true|false optional default true`

## `window.always-on-top`

Toggle SDL always-on-top

**Example:** `api window.always-on-top enabled=true`

**Parameters:**

- `enabled=true|false optional default true`

## `tuning.handling.get`

Read handling override plus instruction-time turn/table/speed/lateral coefficient telemetry when valid

**Example:** `api tuning.handling.get`

**Parameters:** none

## `tuning.handling.set`

Set live handling multipliers

**Example:** `api tuning.handling.set cornering=1.15 speedRetain=1.0`

**Parameters:**

- `cornering=NUMBER optional default 1`
- `speedRetain=NUMBER optional default 1`

## `tuning.handling.reset`

Restore authentic handling values

**Example:** `api tuning.handling.reset`

**Parameters:** none

## `collision.state`

Read evidence-scoped collision-response suppression state and counters; detection remains live

**Example:** `api collision.state`

**Parameters:** none

## `collision.response.get`

Read whether authentic collision response writes are enabled

**Example:** `api collision.response.get`

**Parameters:** none

## `collision.response.set`

Enable/disable only proven collision response writes while preserving detection/event logic

**Example:** `api collision.response.set enabled=false`

**Parameters:**

- `enabled=true|false required`

## `collision.response.reset`

Restore authentic collision response

**Example:** `api collision.response.reset`

**Parameters:** none

## `input.steering.get`

Read raw IOC steering injection (signed 12-bit encoding, centre=0000)

**Example:** `api input.steering.get`

**Parameters:** none

## `input.steering.set`

Set raw IOC steering injection; use 0000 centre, 0060 right, 0FA0 left for proven full steering

**Example:** `api input.steering.set value=0060`

**Parameters:**

- `value=HEX required`

## `palette.bank`

Inspect one 16-pen palette bank

**Example:** `api palette.bank bank=75`

**Parameters:**

- `bank=N required`

## `palette.entry.set`

Freeze one live palette entry

**Example:** `api palette.entry.set index=1209 raw=7FFF`

**Parameters:**

- `index=N required`
- `raw=HEX required`

## `palette.entry.restore`

Return one palette entry to game control

**Example:** `api palette.entry.restore index=1209`

**Parameters:**

- `index=N required`

## `palette.overrides.clear`

Clear all palette freezes

**Example:** `api palette.overrides.clear`

**Parameters:** none

## `input.ioc.xor`

Set held IOC XOR mask

**Example:** `api input.ioc.xor port=3 mask=20`

**Parameters:**

- `port=N required`
- `mask=HEX required`

## `input.ioc.clear`

Clear held IOC XOR masks

**Example:** `api input.ioc.clear`

**Parameters:** none

## `input.ioc.pulse`

Pulse IOC mask for emulated frames

**Example:** `api input.ioc.pulse port=3 mask=20 frames=3`

**Parameters:**

- `port=N required`
- `mask=HEX required`
- `frames=N optional default 3`

## `input.ioc.sweep`

Run deterministic IOC mask sweep and collect evidence

**Example:** `api input.ioc.sweep checkpoint=stage1-gameplay-2064.chqstate port=3 masks=01,02,04`

**Parameters:**

- `checkpoint=FILE optional`
- `port=N optional default 3`
- `masks=comma list optional`
- `pulseFrames=N optional`
- `observeFrames=N optional`

## `checkpoint.list`

List packaged checkpoints

**Example:** `api checkpoint.list`

**Parameters:** none

## `logs.list`

List active-session logs

**Example:** `api logs.list`

**Parameters:** none

## `logs.tail`

Tail a log using the exact identifier returned by logs.list

**Example:** `api logs.tail file=@runtime/web-listener.log lines=80`

**Parameters:**

- `file=RELATIVE required`
- `lines=N optional default 80`

## `evidence.capture`

Capture checkpoint/events/sprites/screenshot evidence bundle

**Example:** `api evidence.capture name=experiment-a`

**Parameters:**

- `name=NAME optional`

## `evidence.list`

List evidence captures

**Example:** `api evidence.list`

**Parameters:** none

## `evidence.view`

Read evidence manifest

**Example:** `api evidence.view name=experiment-a`

**Parameters:**

- `name=NAME optional latest`

## `evidence.load`

Load captured evidence checkpoint paused

**Example:** `api evidence.load name=experiment-a`

**Parameters:**

- `name=NAME optional latest`

## `evidence.zip`

Create ZIP for one evidence capture

**Example:** `api evidence.zip name=latest`

**Parameters:**

- `name=NAME|latest optional`

## `frame.capture`

Capture current SDL frame PNG

**Example:** `api frame.capture name=control`

**Parameters:**

- `name=NAME optional`

## `image.compare`

Compare two captures/snapshots and return pixel-difference metrics. Layered snapshots support source-aware HUD selection.

**Example:** `api image.compare a=control b=test region=PLAYER_CAR hud=hidden`

**Parameters:**

- `a=NAME required`
- `b=NAME required`
- `region=NAME optional default FULL`
- `hud=normal|hidden|only optional default normal`

`hud=hidden` (alias `exclude`) resolves `final-no-hud.png`; `hud=only` resolves `hud-only.png`. These modes require v2 layered snapshots.

## `image.diff`

Compare two images and save a grayscale diff PNG, with the same HUD selection rules as `image.compare`.

**Example:** `api image.diff a=control b=test region=FULL hud=hidden output=diff`

**Parameters:**

- `a=NAME required`
- `b=NAME required`
- `region=NAME optional default FULL`
- `output=NAME optional default image-diff`
- `hud=normal|hidden|only optional default normal`

## `image.regions`

Return raw named image-region document

**Example:** `api image.regions`

**Parameters:** none

## `gameplay.registry`

Return live evidence-backed gameplay registry

**Example:** `api gameplay.registry`

**Parameters:** none

## `track.state`

Return current authoritative track/road state

**Example:** `api track.state`

**Parameters:** none

## `next-steps`

Return packaged high-level research next steps

**Example:** `api next-steps`

**Parameters:** none

## `experiment.begin`

Start a grouped research experiment/evidence manifest

**Example:** `api experiment.begin name=player-car-ab`

**Parameters:**

- `name=NAME optional`

## `experiment.capture`

Capture screenshot plus structured state into the active experiment

**Example:** `api experiment.capture label=control`

**Parameters:**

- `label=NAME optional`
- `include=sprites,regions,gameplay,track optional`

## `experiment.status`

Return the active experiment manifest or inactive state

**Example:** `api experiment.status`

**Parameters:** none

## `experiment.end`

Finalize and close the active experiment manifest

**Example:** `api experiment.end`

**Parameters:** none

## `platform.info`

Return build, script engine, active profile and research-platform identity

**Example:** `api platform.info`

**Parameters:** none

## `features.list`

Return machine-readable feature/regression manifest

**Example:** `api features.list`

**Parameters:** none

## `script.functions`

List supported Script Console query/collection functions

**Example:** `api script.functions`

**Parameters:** none

## `registry.list`

Return gameplay registry items as a flat structured list

**Example:** `api registry.list`

**Parameters:** none

## `track.records`

Read track records around the current authoritative record

**Example:** `api track.records before=2 after=8`

**Parameters:**

- `before=0..64 optional default 4`
- `after=0..64 optional default 12`

## `objects.list`

Return semantic-object registry candidates and evidence confidence

**Example:** `api objects.list`

**Parameters:** none

## `audio.capabilities`

Describe planned/current audio research capabilities without mutating audio runtime

**Example:** `api audio.capabilities`

**Parameters:** none

## `audio.channels`

Return audio channel/source descriptors from the audio research profile

**Example:** `api audio.channels`

**Parameters:** none

## `audio.events`

Return current audio-event foundation status; runtime capture is intentionally not enabled yet

**Example:** `api audio.events`

**Parameters:** none

## `audio.waveform.plan`

Return waveform-viewer/capture implementation contract for future validated audio tooling

**Example:** `api audio.waveform.plan`

**Parameters:** none

## `audio.export.plan`

Return planned sample/music export contract and evidence formats

**Example:** `api audio.export.plan`

**Parameters:** none

## `audio.inject.plan`

Return guarded runtime audio-injection design; does not inject audio

**Example:** `api audio.inject.plan`

**Parameters:** none

## `evidence.names`

Return structured evidence-session directory list

**Example:** `api evidence.names`

**Parameters:** none

## `schema`

Describe scriptable API actions and parameter contracts

**Example:** `api schema action=memory.export`

**Parameters:**

- `action=ACTION optional`


## Shared Track View recorder and SVG export (v0.66.5.1)

The Track View recorder is now server-side shared state. The browser Track View buttons and Research Script API operate on the same sample set, so a survey can be recorded and exported without the Track View tab being open.

```text
api track.record.clear
api track.record.start
api track.record.sample
api track.record.status
api track.record.stop
api track.svg.export name=stage1-track-survey-3600
```

`track.record.sample` appends one deterministic sample of the authoritative `track.state` data and derives the same 2D reconstruction coordinates used by Track View. `track.record.status` returns enabled state, sample/segment counts, first/last frame, spatial bounds and recorder capacity. `track.svg.export` writes a genuine vector SVG under `evidence/track-exports` and refuses to export fewer than two samples.

The recorder retains up to 5000 samples per Workbench process. `track.record.stop` does not clear samples. `track.record.clear` clears them explicitly.

### Memory trace transaction-width clarification (v0.66.5.1)

`memory.trace.start width=...` filters the width of the originating CPU/bus write transaction, not the width of the individual byte being observed. A byte address can therefore change as part of a 16- or 32-bit write. When the producer width is not already known, use `width=any`; address/range overlap is still enforced independently.

```text
api memory.trace.start cpu=A address=0x100303 length=1 width=any limit=256
```

## v0.66.7 — Documentation/Knowledge and SDL window control

New read-only documentation actions: `docs.index`, `docs.search`, `docs.get`, `docs.glossary`, `docs.handover`, `docs.export`, plus `knowledge.list`, `knowledge.get`, `knowledge.search`, `knowledge.related`. These are callable from the HTTP/Workbench bridge and through `.chqscript` using `api ...`.

SDL/runtime control parity is expanded with `window.status`, `window.fullscreen`, `window.scale`, `window.show`, `window.hide`, `window.minimize`, `window.restore`, alongside existing `window.always-on-top`. Scale accepts 1..8 and leaves fullscreen. Window/status output is native authoritative state.

Layered frame snapshots now also emit `scene-no-sprites.png` and `sprites/sprites.json` plus cropped transparent per-visible-sprite assets. These are exploratory forensic controls; exact reconstruction continues to use the aggregate sprite contribution layer recorded in `reconstruction.json`.


## Documentation handbook actions (v0.66.7.5)
- `docs.handbook` — returns the complete authored user-facing handbook tree and page blocks. Example: `api docs.handbook`.
- `docs.page id=PAGE` — returns one handbook page and its parent section. Example: `api docs.page id=graphics-lab-guide`.
- `docs.export` with no `id`/`query` now exports the **full User & Research Handbook**, combining authored handbook content, the generated API action reference, generated scripting-language reference, project knowledge/glossary, and selected evidence.

The Workbench API Reference view is generated directly from the authoritative action schema, so parameters/examples shown to users should match `api schema.all` / `api schema action=NAME`.


## Documentation search/export consolidation (v0.66.7.6)
No action names or parameters change. `docs.search` / `knowledge.search` now include the structured Project Knowledge article body plus API/script/docs references in their searchable corpus. `docs.export` emits the same article sections used by the Workbench and omits empty reference headings.


## Run-history / completed-run snapshot discovery (v0.66.7.7)

`GET /api/v1/script/runs` returns up to 100 completed script runs discovered from authoritative on-disk `run-metadata.json` files. The Workbench merges this server list with browser-local history so a valid completed run is not lost merely because localStorage missed the completion event.

The Script API mirrors this as `api script.runs`.

`GET /api/v1/frame/snapshots`, `frame.snapshot.list`, and the existing frame-snapshot manifest/asset routes now also discover snapshots inside completed run artifact roots. When the same snapshot name exists in more than one location, the newest matching snapshot is resolved.


### v0.66.7.8
`script.runs` keeps the same contract but uses shallow known-layout discovery plus bundle-manifest artifact enumeration and short-lived caching to avoid blocking the Workbench listener. No action names or parameters changed.

## Forensic Timeline v1 (v0.66.8.0)

Forensic Timeline records a bounded gameplay interval once and makes it reusable through the same inspection APIs used for a paused live machine. See `docs/FORENSIC_TIMELINE.md`.

- `timeline.record.start name=NAME` — start per-frame deterministic capture in the current run artifacts; current frame is captured immediately.
- `timeline.record.stop` — finalize the active timeline.
- `timeline.status` — native recording/import/source status.
- `timeline.list` — discover timelines from active/completed runs and packaged research data.
- `timeline.load name=NAME|path=PATH` — import and load the first frame into paused inspection mode.
- `timeline.seek frame=N` — load an exact recorded frame.
- `timeline.next` / `timeline.prev` / `timeline.frames` — navigate/index the recording.
- `timeline.play name=NAME from=N to=N fps=1..60` — paced recorded-state playback through the SDL renderer; this loads historical frame checkpoints in sequence and does not re-execute CPUs.
- `timeline.trim.event ...` — find a masked rising/falling event window with configurable pre/post frames and persist it as a non-destructive analysis view; the master recording is left intact.
- `timeline.scan.memory ...` — offline whole-write-stream candidate/writer ranking; exports portable CSV/JSON/HTML into the current script run artifacts while leaving the source timeline immutable.
- `timeline.analyze.auto ...` — full unattended pipeline: portable memory ranking, known-reference tagging, writer-PC disassembly, historical milestone state/layered snapshots and HUD_TURBO graphics correlation.
- `timeline.fork-live` — keep the selected historical checkpoint loaded and return to LIVE paused execution.
- `timeline.unload` — restore the live state that existed before import.

While `source=timeline`, ordinary read-side actions such as `memory.read`, `register.read`, `gameplay.registry`, `sprites.list`, palette inspection and `state.snapshot` operate on the selected recorded checkpoint. Timeline v1 captures complete sprite/object RAM. Authentic audio capture remains unavailable because the native runtime currently has only `sound_stub`; this limitation is explicit in the timeline manifest and audio capability descriptor.

## v0.66.9.0-RC2 additions
- `sprite.map.inspect slot=N|map=N`
- `sprite.order.inspect`
- `sprite.order.set mode=higher-slot|lower-slot`
- `game.sprite.order.inspect`
- `game.sprite.order.set mode=higher-slot|lower-slot`

### Speed semantics
`game.speed.set` and `game.speed.freeze` target CPU-A `0x10041C`, the raw internal/physics speed. The visible HUD speed is the separate packed-BCD field at `0x100400`; the numeric values are not expected to match. Game Lab mutations preserve the prior RUNNING/PAUSED state, while generic `memory.write` intentionally pauses for debugger safety.

> RC2.2 note: Chase H.Q. native same-priority sprite ordering defaults to `lower-slot`, matching MAME reverse sprite-RAM traversal; `higher-slot` remains an explicit diagnostic override.
## v0.66.9.0-RC2.5 sprite ownership assertions

`api frame.snapshot.assert-overlap name=normal front=82 back=81 frontSolo=body backSolo=shadow`

All five parameters are required. Names resolve existing snapshots. The action throws unless frames agree, both exported sprites exactly match their separate solo runtime source renders at the exported coordinates, overlapping source pixels have coverage >=2 and are never owned by the back sprite, front-owned overlap is nonzero, and the back sprite remains visible outside the front. It returns frame/map IDs and overlap/visibility counts. It performs no emulation steps or presentation changes. Offline CLI equivalent: dot-source `scripts/Assert-SpriteSnapshotOverlap.ps1`, then call `Assert-SpriteSnapshotOverlap -Snapshot DIR -Front 82 -Back 81 -FrontSolo DIR -BackSolo DIR`.

Snapshots add `sprite-ownership.csv` (x,y,owner,coverage; 65535=no sprite-stage owner). Coverage counts blocked/occluded nonzero pens; owner records accepted color. Individual sprite asset coordinates now use the same visible Y=16 origin as live rendering.

Legacy `sprite.order.set` mode names remain accepted: `lower-slot` selects descending traversal, `higher-slot` ascending. First nontransparent candidates retain occupancy in both modes, including layer-blocked candidates. `sprite.order.inspect` now reports `ownership=first-nontransparent occupancy=includes-layer-blocked`. RC2.2's inference that descending traversal means lower-slot wins was incorrect. See [SPRITE_OWNERSHIP.md](SPRITE_OWNERSHIP.md).

## v0.66.9.0-RC2.5 lifecycle additions

### `timeline.reset`

`api timeline.reset`

Idempotent cleanup for script-initiated timeline work. The first `timeline.load` in a transaction captures the pre-load live state. `timeline.reset` unloads a read-only timeline or restores that captured live checkpoint after `timeline.fork-live`, then clears the transaction. If no transaction/timeline is active it succeeds without changing state. Use it before and in `finally` for mutation experiments.

### `script.runs` current-session metadata

`api script.runs` continues to support bounded `limit`, `offset`, `status`, `version`, and `session` filters. Status accepts `PASS`, `FAIL`, `CANCELLED`, or `FINALIZATION_FAILED`; the latter distinguishes a script body result from a failed authoritative evidence finalization. The response now includes `currentSession`, which the Workbench uses to default Script Console history to the active session; users can still select all sessions or filter by build/version.

### API exposure parity

Workbench startup now validates both directions between the Script Console allow-list and the action schema. A schema action missing from the executable allow-list (or vice versa) is a startup error rather than a silently unreachable documented command.


## v0.66.9.0-RC3.3 script-run parameters and evidence

`api script.run file=<library-path> key=value ...` executes a compatible library script with the supplied declared `# param` values. Unknown parameter names, invalid typed values and values outside a declared `select` set are errors. Parameters seed the existing `${name}` substitution environment; they are not a parallel scripting runtime.

Workbench `/api/v1/script` accepts a `parameters` object for the same parameter resolution model. Authoritative run metadata records the selected parameter map and writes expanded commands to `resolved-commands.txt`. Trace/checkpoint evidence is collected under the run-local artifact tree.

LAN `/console` uses the same front-end/API surface. It is reachable remotely only when the launcher is explicitly started with `-LanAccess`; the private backend/native API remain loopback-bound and remote front requests are token authenticated.


## v0.66.9.0-RC3.1 course-mapping data quality

No existing action name is removed. The RC3.0 live course follower/survey actions remain the control surface. RC3.1 changes the **mapping data model** so long surveys do not confuse dynamic renderer state with road material and so raw sprite evidence can be followed as candidate objects.

`api track.record.status` now reports Track Recorder v3 fields including `surfaceCandidateCount`, `surfaceSegmentCount`, `rawSurfaceRenderVariantCount`, `rawSpriteRowCount`, `eligibleObjectCandidateSourceRowCount`, `objectCandidateRowCount`, `objectCandidateCoverage`, `objectCandidateTrackCount`, and `persistentObjectCandidateTrackCount`. Under the current conservative one-eligible-sprite -> one-candidate policy, `objectCandidateCoverage` is expected to be `1.0`.

`api track.record.sample` retains the existing road/player/follower fields and adds:

- `featureType`, `roadProcessingSelector`, `paletteTransitionSelector`, `environmentTransitionSelector`, `featureParameter`: evidence-backed names for record +2/+4/+5/+6/+7. Legacy `flagsType/eventCode/visualSelector/featureTrigger/controlFlags` remain deprecated compatibility aliases.
- `featureParameterSemantic`: conservative type-dependent interpretation; concrete route-indicator meanings are emitted only for the causally proven `featureType=0xEB` family.
- `courseRecordSignature`: exact eight-byte course-record identity.
- `roadProcessingSignature` / `roadMaterialCandidateSignature`: record +4 only; used for road/material candidate grouping.
- `surfaceStableSignature`: compatibility field now equal to the +4 road-processing candidate, not the complete course record.
- `courseSurfaceSignature`: deprecated compatibility field retaining complete course-record identity; it is not a material signature.
- `surfaceRenderCandidateSignature`: masked TC0150-derived candidate evidence, not a material label.
- `roadRenderSignature`: full raw representative TC0150 renderer signature; retained but excluded from stable surface counts.
- `visibleSprites`: raw `UNCLASSIFIED_DYNAMIC_OBJECT` rows with visible bounds.
- `objectCandidates`: per-sprite `UNCLASSIFIED_OBJECT_CANDIDATE` observations with heuristic screen-space temporal tracks. Each row preserves source slot/map/palette, visible bounds, centre, dimensions and visible-pixel evidence. Candidate IDs are useful for correlation only and do not prove AI/obstacle identity.

`api track.map.export name=NAME` now emits `chq-course-map-export-v2` / `chq-course-map-v2` and adds `track-map-summary.json`, `track-object-candidates.csv`, and `track-surface-segments.csv`. Existing `track-map.json`, `track-samples.csv`, `car-trajectory.csv`, `track-entities.csv`, `track-surfaces.csv`, and `track-map.svg` remain. Export is fail-closed for the current release policy: candidate-row count must equal eligible source-sprite count and every candidate must retain map/palette/bounds/position/pixel evidence; otherwise the export throws instead of writing a misleading dataset.

The road/material candidate key intentionally excludes volatile TC0150 scanline words **and** the now-proven non-surface bytes +5 palette transition, +6 environment transition, and +7 feature parameter. Full record identity remains in `courseRecordSignature`; raw TC0150 words remain in the exported sample so later material classification can be improved without rerunning the course.


## RC3.4 mobile pairing descriptor

`GET /api/v1/mobile/pairing` is available only to a local Workbench request. When LAN/mobile access is active it returns the clean `/console` URL and the token-bearing pairing URL used to render the desktop QR. Remote requests receive HTTP 403 so an authenticated phone cannot ask the frontend to reveal the session token. The first remote `/console?token=...` request captures the token into an HttpOnly `chq_token` cookie and redirects to clean `/console`.

Mobile run history uses the existing authoritative `GET /api/v1/script/runs` endpoint and its `limit`, `offset`, `status`, `version` and `session` filters.


## RC3.4.8 chqdev regression-runner note

The authoritative release runner in `chqdev` v0.4 executes the current packaged Research Script v2 control-flow subset client-side in Go and sends ordinary commands through `/api/v1/script` under one authoritative run ID. This mirrors the Workbench Script Console model and avoids sending an entire control-flow script to the reduced server parser. The public Research API itself is unchanged by RC3.4.8.


## v0.66.9.2 Stage-1 track-record semantic refinement

Runs 019-029 promoted the current record field names to `featureType` (+2), `roadProcessingSelector` (+4), `paletteTransitionSelector` (+5), `environmentTransitionSelector` (+6), and polymorphic `featureParameter` (+7). `track.state` and `track.records` expose those fields and keep older names as deprecated aliases.

For `featureType=0xEB` only, `featureParameterSemantic` can report the proven route-indicator modes: `FF` suppress/clear, `00` standard route indicator, and `02` route-indicator variant. Other feature classes remain type-dependent and are not overgeneralized.

## v0.66.9.2 authoritative-run provenance

For chqdev Go-runner executions, the first `/api/v1/script` request carries `runSource` containing the complete authored top-level `.chqscript`; subsequent resolved-command requests reuse the same `runId`. `run-metadata.json` persists `commands`, and finalization may provide `commandCount` so the authoritative Go-runner total is retained. `/api/v1/script/runs` exposes that persisted count to desktop/mobile Recent Script Runs.


## v0.66.9.5-RC1.1 corrective diagnostics

### `frame.snapshot.parity`

`api frame.snapshot.parity name=asset-check require=true` captures the live gameplay compositor without F1/Workbench overlays, temporary road/BITS/PROM forensic visualisations or diagnostic presentation backgrounds, then independently produces the structured layered snapshot, compares them pixel-for-pixel, writes a parity JSON manifest and fails when `require=true` and any pixel differs. Ordinary `screenshot PATH` still captures the user-visible presentation; `screenshot PATH composited` is the parity-only, temporary compositor capture that restores the SDL view immediately afterward. This is the release regression primitive for the v0.66.9.4 semantic-asset snapshot omission.

### `sprite.asset-resolve`

`api sprite.asset-resolve` reports live source map/palette records, semantic object matches and active map/asset replacement status. `player_car.body` includes the Run009-proven map509/palette65 family member.

### Corrected `game.speed.set` / `game.speed.freeze`

The user-facing value is an integer internal speed. The confirmed runtime field CPU-A `0x10041C` is 32-bit fixed point, so RC1 writes/freezes `value << 16`.

### Structured `target_contact` collision event

`collision.events` can report observational class `target_contact` for the proven PC A13C write to target-local lateral `0x10A084`. It is deliberately not included in `collision.response.set enabled=false`; that control remains scoped to the previously proven player lateral/speed response writes.

### `events.clear`

`api events.clear` clears generic research-event history before an experiment phase, allowing subsequent `events.tail` output to be interpreted as a local causal window instead of inherited session history.

> **0.67.0.0-RC5 Windows parity corrective — UNQUALIFIED:** see `docs/RC5_WINDOWS_PARITY_CORRECTIVE.md` for evidence and fresh Windows gates.
