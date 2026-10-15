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

> **Current source candidate: v0.67.0.0-RC4.6 (NOT QUALIFIED).** Exact immediate source parent is **promoted Windows-qualified RC4.5** SHA-256 `443ffe623a38de4813691c436e05d6f92d9e113a8abae0219824fcc77e9aae11`. RC4.6 adds asynchronous local Ghidra headless analysis through Workbench Script/API/UI and carries forward RC4.5 REA import/census. See `docs/GHIDRA_WORKBENCH_RC46.md`. Original runtime and emulator remain unchanged. Exact fresh-archive Windows `chqdev.bat qualify --fresh` is mandatory before promotion.

> **Current development candidate:** `v0.67.0.0-RC4.4`. Windows exact-archive qualification pending. See `docs/NATIVE_PROJECTION_RC44.md` for present scope and limitations.

## v0.67.0.0-RC4.3.2 CURRENT DEVELOPMENT CANDIDATE — NOT QUALIFIED

This source candidate extends v0.67.0.0-RC4.2.3. Its authoritative new feature and limitation inventory is `docs/NATIVE_MIGRATION_RC43.md`. The prior RC4.2.3 Windows debug compile was PASS, but it does **not** qualify RC4.3. Required before promotion: exact-archive Windows `chqdev.bat qualify --fresh` including strict CTest, focused and full regressions, deterministic package verification and readiness. The known pre-existing Full Regression Go structural budget check remains FAIL (1,170+ commands versus <850); do not delete unique assertions or relax that gate to conceal the failure. New code paths MUST keep native replacement disabled by default. User-controlled ROM, SDL and content paths stay outside release ZIPs. Fresh release setup MUST find the existing stable `build-local.json` or provide a precise doctor error. The bundled Windows chqdev executables MUST be present, tested and accompanied by matching release metadata.

> Current development candidate: **v0.67.0.0-RC4.2.3.1** — CHQPACK Browser, overlay inspection and provenance. Windows qualification pending.

> Current development candidate: **v0.67.0.0-RC4** — ROM/CHQPACK source switching and RC3.1 release consistency corrections.

> Current development candidate: **v0.67.0.0-RC3** — CHQPACK runtime content boundary; ROM-free runtime after deterministic import; RC2.1 native rendering retained.

> Exact source baseline: **v0.67.0.0-RC2.1** SHA-256 `0199faf2333ebbe25e7c23df2c570d0f5f35c9179b8ea53b43983136f3885fb6`. See `docs/DOCUMENTATION_AUDIT_0.67.0.0-RC3.md`.

> Historical qualified candidate: **v0.67.0.0-RC1.1** — native sprites default; text/HUD Shadow zero-diff bring-up; native sprite forensic overlap-coverage corrective.

> Current script/API build: **v0.66.9.5-RC3**. No general grammar expansion is introduced by RC3; the native-promotion workflow is exposed through ordinary `api` actions so promotion scripts remain bounded and deterministic. New actions are documented in `API_REFERENCE.md` and the authoritative schema.

> Historical candidate: **v0.66.9.5-RC2**. See `docs/DOCUMENTATION_AUDIT_0.66.9.5-RC2.md` for Runs003-012 evidence and RC2 changes.

> Current development candidate: **v0.66.9.5-RC1.1**, a narrow Workbench web-bridge hotfix built from the exact Windows-qualified **v0.66.9.5-RC1** archive SHA-256 `26bda3287a3256fe8bf2c46aaaafba68546c02d2e1922465abcbacb0a62d262e`. RC1.1 changes no intended emulation behaviour and requires exact-archive Windows `chqdev qualify --fresh` before promotion.

> Current release: **v0.66.9.4**. Promoted from exact Windows-qualified **v0.66.9.4-RC3.1.3** SHA-256 `48534c0bd3d8e4a40987e65089b07046535976675ca2e36d0ebf3a6d9584383e`. The final archive is regenerated for release identity/docs/focused-regression wiring only and requires one exact-final-archive `chqdev qualify --fresh` closure before immutable publication.

> Current development candidate: **v0.66.9.4-RC3.1.2**. RC3.1.2 keeps the proven cross-format yellow-target renderer but binds it to semantic `player_car.body` across steering/orientation maps, expands SPEED suppression to the complete live destination block, and retains fail-closed proof plus chqdev readiness/console UX. Exact-archive Windows `chqdev qualify --fresh` remains required.

> Current development candidate: **v0.66.9.4-RC2**. Source lineage remains the immutable promoted **v0.66.9.3** baseline (SHA-256 `6da88889dc8214c6058644e44e4d29f6a2117c9f0e92d99c8d906c280fc3462e`); **v0.66.9.4-RC1** (`82d0ee720b40f248624e6c3b2b532ecbef83a68c80a45c68e70556a49b1b1a3a`) passed fresh Windows qualification. RC2 adds the Native Reconstruction Lab foundation, Original/Shadow/Native sprite backend switching, global existing-sprite map substitution, complete live TC0100SCN charset export, Stage-2 checkpoint corpus expansion and the HUD Layout responsive-width fix. RC2 still requires exact-archive Windows `chqdev qualify --fresh` before promotion.

## v0.66.9.4-RC2 note

Research Script v2 grammar is unchanged. RC2 native/asset functionality is exposed through ordinary schema-backed `api` actions: `backend.sprite.*`, `sprite.map-override*`, and `text.charset.export`. No new language statement or query syntax is introduced.

## v0.66.9.4-RC1 note

The Research Script v2 grammar is unchanged. Beacon and semantic HUD functionality uses ordinary `api ACTION key=value` statements through the authoritative action schema; no new language construct is introduced.

> Current release: **v0.66.9.3**. Promotion provenance is the exact Windows-qualified `v0.66.9.3-RC5` archive SHA-256 `19b7ed48edae98fd99f660ef56a949573050cabc2958fcdba2833a7888103961`. That candidate passed 8/8 CTest, deterministic package/verify, runtime readiness, focused regression, Full Regression and `promotable`; the final promotion changes release identity/documentation/focused-regression wiring and adds a release-package metadata guard only; it does not intentionally change emulation/gameplay behaviour.


# Research Script v2 - current v0.66.9.4-RC1 notes

The grammar remains Research Script v2.
The v0.66.9.3 release adds no incompatible grammar change. New capability is exposed through ordinary `api` actions, notably `sprite.layer-shadow.compare`, read-capable `memory.trace.start access=...`, `collision.events` and `audio.events`. Runtime-query/tuple/corpus semantics from RC3 remain current.
 RC2.9 adds `api game.target.one-hit enabled=true|false` and an optional `timeout=` parameter to `api control.run-frames`.

> **State-dependent query rule:** existing `get*()` collection helpers remain preflight-resolved for backward compatibility. When machine state may have changed (especially after `checkpoint.load`), use `api query.runtime function=getmaps()` or the corresponding supported query expression to evaluate against the state at that execution point.

Structured tuple iteration is supported in bounded form: `for address,width in [(1002AE,16),(1002D8,16)] ... end`. Tuple rows must have exactly the same field count as the binding list; loop iteration remains capped at 256. This is intentionally a small data-iteration feature, not a general programming language. Nested tuple/repeat/if/try compilation preserves the authored top-level source line number for each expanded command so errors and console provenance point back to the original script.

`corpus.run` returns parsed `result` objects for JSON-producing actions and falls back to `raw` only when output is not JSON. Browser-run finalization supplies the full executed command count, including client-side `wait` statements.

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

## v0.66.9.0-RC1 action additions

The scripting language grammar is unchanged. New functionality is exposed through ordinary `api ACTION key=value` calls, notably the `game.*` and `tile.inspect` actions documented in the live API schema. Reusable turbo/HUD investigation scripts are now bundled under `research/scripts/graphics/` and `research/scripts/gameplay/`.

# Research Script v2 Language Reference — v0.66.9.4-RC2
> **Current candidate:** v0.66.9.4-RC2. Promoted baseline is exact v0.66.9.3 SHA-256 `6da88889dc8214c6058644e44e4d29f6a2117c9f0e92d99c8d906c280fc3462e`; fresh exact-archive qualification remains `chqdev qualify --fresh`.


Authoritative runtime discovery: `api script.schema` or `GET /api/v1/schema/script`. Static schema snapshot: `research/schema/script-language.json`.

## Statements

### `# comment`

Comment

### `set NAME = VALUE`

Set a variable

### `${NAME}`

Variable substitution

### `for ITEM in LIST ... end`

Iterate a list/query result

### `repeat N ... end`

Repeat a block; exposes ${repeat_index}

### `if status KEY=VALUE ... [else] ... end`

Conditional status check

### `if memory cpu=A address=HEX width=8|16|32 value=HEX ... end`

Conditional memory check

### `if capability NAME ... end`

Conditional capability check

### `wait MS`

Sleep 0..5000 ms

### `wait-until step-complete [TIMEOUT_MS]`

Wait for native frame-step completion

### `require capability NAME`

Require native capability

### `assert status KEY=VALUE`

Assert a status field

### `echo TEXT`

Write run output

### `api ACTION key=value ...`

Invoke a documented Script API action

### `include "path.chqscript"`

Include a saved library script

### `zip evidence latest|NAME`

ZIP one evidence capture

### `zip evidence-match PREFIX [BUNDLE_NAME]`

ZIP captures sharing a prefix

## Query/collection functions

- `getregions()`
- `getprofiles()`
- `getsprites()`
- `getpaletteentries(bank=N)`
- `getcheckpoints()`
- `getcapabilities()`
- `getmaps()`
- `getpalettes()`
- `getregistry()`
- `gettrackrecords()`
- `getevidence()`
- `getobjects()`
- `getaudiochannels()`
- `count()`
- `first()`
- `last()`
- `contains()`
- `unique()`

## Limits

- `sourceBytes`: 65536
- `sourceLines`: 1000
- `expandedCommands`: 1000

v0.66.9.0-RC2.4.4 raises the expanded-command ceiling from 500 to 1000 because the permanent Full Regression suite now expands to 503 commands. Loop-iteration and nesting limits are unchanged.
- `loopIterations`: 256
- `nestingDepth`: 4
- `waitUntilMs`: 30000

## SDL interactive prompt

The prompt is an API action used from scripts:

```text
api ui.prompt message=Hold_DOWN_until_the_brake_state_is_visible_then_press_ENTER pause=false top=true timeout=300000
api control.pause
```

`message` underscores render as spaces. Enter accepts; Escape cancels. Physical gameplay controls continue through the authentic IOC input path while the overlay is shown. Use `pause=false` when the condition must develop while the game is running.


## Palette-write tracing from Research Script (v0.66.4)

The Script language itself is unchanged; palette tracing is exposed through documented `api ACTION` calls. Use runtime discovery (`api schema action=palette.trace.start` or `api schema.all`) rather than guessing action names.

```text
api palette.trace.clear
api palette.trace.start indices=1037,1053 limit=256
api checkpoint.load file=stage1-gameplay-2064.chqstate run=false
api control.run-frames frames=5
api input.ioc.xor port=2 mask=20
api control.run-frames frames=8
api palette.trace.tail count=50
api palette.trace.stop
```

Palette trace events contain the exact writer PC and complete palette old/new values, which makes them suitable for follow-up disassembly/provenance work.

## Generic memory-write tracing from Research Script (v0.66.9.0-RC2.9)

The Script grammar is unchanged. The following first-class API actions are available through `api ACTION`: `memory.trace.start`, `memory.trace.status`, `memory.trace.tail`, `memory.trace.stop`, and `memory.trace.clear`. Example:

```text
api checkpoint.load file=stage1-gameplay-2064.chqstate run=false
api memory.trace.clear
api memory.trace.start cpu=A address=0x100303 length=1 width=any limit=256
api control.run-frames frames=5
api input.ioc.xor port=2 mask=0x20
api control.run-frames frames=6
api memory.trace.tail count=50
api memory.trace.stop
```

Use `api schema action=memory.trace.start` or `api schema.all` for the authoritative runtime contract.

## Scriptable shared Track View recorder (v0.66.5.1)

Track recording and SVG export are first-class Script API actions. They use the same server-side recorder as the Track View UI.

```text
api checkpoint.load file=stage1-driving-2352.chqstate run=false
api track.record.clear
api track.record.start
api track.record.sample
repeat 120
    api control.run-frames frames=30
    api track.record.sample
end
api track.record.stop
api track.record.status
api track.svg.export name=stage1-track-survey-3600
```

This makes long track surveys reproducible without browser polling. The packaged example is `research/scripts/track/surveys/stage1-track-survey-3600-scripted-svg.chqscript`.

For memory tracing, remember that `width` describes the originating write transaction. Use `width=any` if a target byte may be modified by a wider write.


## Reconstructable layered frame capture and HUD-aware comparison (v0.66.6.1)

The grammar remains unchanged; these are API actions available from Research Script.

```text
api checkpoint.load file=stage1-driving-2352.chqstate run=false
api frame.snapshot name=layered-baseline
api control.run-frames frames=30
api frame.snapshot name=layered-test
api image.compare a=layered-baseline b=layered-test region=FULL hud=hidden
api image.diff a=layered-baseline b=layered-test region=FULL hud=only output=hud-only-diff
```

`frame.snapshot` v2 self-verifies reconstruction in v0.66.6.1 and fails if any reconstructed pixel differs from `final.png`. It writes exact transparent contribution layers under `layers/`, raw source renders under `sources/`, and `reconstruction.json`. `hud=hidden` suppresses the Chase H.Q. TC0100SCN text/HUD source; it is not a fixed rectangular crop. `hud=only` compares only the pixels whose final output is influenced by that HUD source.

## v0.66.7 API-facing additions
No new language syntax is introduced. Use the existing `api ...` command for documentation/knowledge and SDL controls, for example:

```text
api docs.search query=turbo
api docs.handover
api docs.export format=html evidence=key output=project-handbook
api window.status
api window.fullscreen enabled=false
api window.scale scale=3
api window.always-on-top enabled=true
```

This preserves the small deterministic language while maintaining Workbench/API/script parity.


## Built-in scripting reference (v0.66.7.5)
The Workbench Docs / Knowledge tab now contains a complete Scripting Reference generated from the authoritative script-language schema. It lists every supported statement, query function and parser limit, with copyable examples. No new core language syntax is introduced in v0.66.7.5; the new handbook is exposed through the existing generic API statement:

```text
api docs.handbook
api docs.page id=api-overview
api schema.all
api script.schema
```


## v0.66.7.6 documentation consolidation
No `.chqscript` grammar changes. Existing `api docs.search`, `api knowledge.search`, `api docs.get` and `api docs.export` actions expose the richer article search/export behaviour through the unchanged generic `api ACTION key=value` statement.


## v0.66.7.7 persistence/discovery addition

No grammar change. The generic `api` statement gains the documented action `script.runs`, which lists authoritative completed script runs from on-disk run metadata.

## Native API error propagation
As of v0.66.9.0-RC2, an `api ...` command whose native result begins with `ERR` is treated as a script failure. With **Stop on error** enabled, execution stops and the run is finalized FAIL. This closes the older gap where a command could print `ERR` inside an otherwise green PASS.

> RC2.2 note: Chase H.Q. native same-priority sprite ordering defaults to `lower-slot`, matching MAME reverse sprite-RAM traversal; `higher-slot` remains an explicit diagnostic override.
## v0.66.9.0-RC2.5 sprite correctness

Grammar is unchanged. `frame.snapshot.assert-overlap name=NAME front=SLOT back=SLOT frontSolo=NAME backSolo=NAME` is an assertive API action; failure stops a normal stop-on-error script. Full Regression includes `regression-sprite-ownership.chqscript`. Legacy order mode names select traversal, not winning slot: descending `lower-slot` now reserves each nontransparent pixel for its first candidate. See the API reference and `docs/SPRITE_OWNERSHIP.md`.

## v0.66.9.0-RC2.5 safe experiment lifecycle

Research Script v2 adds `try ... finally ... end` in the Workbench compiler. The `finally` block runs after the body on success and on API/assert/runtime failure; on user cancellation it is attempted where practical. If both body and cleanup fail, the body error remains the primary failure and cleanup errors are reported separately.

Use `api timeline.reset` as the idempotent timeline cleanup primitive. A script-initiated `timeline.load` captures the pre-load live state; after `timeline.fork-live`, `timeline.reset` restores that state. Calling it with no active transaction succeeds without changing the machine.

Mutation scripts should include `name`, `purpose`, `category`, `target`, `method`, `expected`, and `cleanup` header fields, establish known preconditions, and place temporary timeline/sprite/patch/presentation cleanup in `finally`.

Example:

```text
# name: Safe timeline experiment
# purpose: Demonstrate cleanup after a counterfactual fork.
# category: example
# target: recorded frame 2352
# method: load, seek and fork inside try
# expected: experiment runs at frame 2352
# cleanup: timeline.reset restores the pre-load live state

api timeline.reset
try
    api timeline.load name=car-shadow-layer-investigation
    api timeline.seek frame=2352
    api timeline.fork-live
    # experiment commands
finally
    api timeline.reset
end
```


## RC3.0 course-mapping recipes

Long course surveys should use the bundled scripts under `research/scripts/track/surveys/` rather than manually unrolled commands. `track.record.sample` now returns a compact result to Script Console runs while the shared recorder retains the complete dataset, preventing O(n²) console/evidence growth during long surveys. Use `track.map.export` for the reconstructable dataset.


## RC3.2 declared script parameters

Scripts may declare bounded run parameters in metadata comments. Example:

```text
# param frames type=select values=300,600,900,1200,2400,3600 default=1200
api control.run-frames frames=${frames}
```

Supported declaration types are `string`, `int`, `bool`, and `select`. A declared default is used when the caller does not supply a value. `select` values are restricted to the declared comma-separated set. Unknown supplied parameter names and invalid values fail validation rather than silently creating variables. Parameter values seed the existing variable substitution model, so scripts without `# param` declarations retain their previous behaviour.

Workbench displays controls for declared parameters and previews the resolved commands. API/CLI-compatible library scripts may be invoked with `api script.run file=<path> name=value`. Authoritative runs store selected parameter values in `run-metadata.json` and the expanded command stream in `resolved-commands.txt`.


## RC3.1 course-mapping survey scripts

The language grammar is unchanged. RC3.1 keeps the live `course.follow.*` / `course.survey.*` APIs and upgrades the data produced by the existing `track.record.*` / `track.map.export` calls. Prefer the packaged scripts rather than manually unrolling long runs:

```text
regression/regression-v06690-rc31-course-mapping-quality.chqscript
track/surveys/stage1-course-mapping-shakedown-600.chqscript
track/surveys/stage1-course-mapping-3600.chqscript
track/surveys/current-state-material-object-survey-1200.chqscript
```

The current-state material/object survey intentionally does not load a Stage-1 checkpoint, making it suitable for later-stage checkpoints containing candidate obstacles or alternate road surfaces. Candidate object IDs and surface keys remain evidence-scoped; scripts must not promote them to semantic labels without separate proof.


## RC3.4.8 chqdev regression-runner note

The authoritative release runner in `chqdev` v0.4 executes the current packaged Research Script v2 control-flow subset client-side in Go and sends ordinary commands through `/api/v1/script` under one authoritative run ID. This mirrors the Workbench Script Console model and avoids sending an entire control-flow script to the reduced server parser. The public Research API itself is unchanged by RC3.4.8.

## v0.66.9.2 run evidence contract

An authoritative chqdev run now preserves two distinct views deliberately: `script.chqscript` is the complete authored top-level source, while `resolved-commands.txt` is the actual expanded command stream executed by the Go runner. `run-metadata.json.commands` is the authoritative final command count and is the value shown by Recent Script Runs.


## Authoritative source provenance (v0.66.9.3-RC3)

Browser and chqdev runners send the compiled item's resolved-source line with every authoritative command. Runs preserve the original `script.chqscript`, the include/query-resolved `resolved-script.chqscript`, and `resolved-command-map.jsonl` containing sequence, sourceLine, command and clientRecord. Client-side `wait` remains cancellable in the browser/Go runner but is also recorded server-side so metadata command totals cannot exceed the evidence trail.


Current development candidate: v0.66.9.4-RC3.
