# Continue from v0.67.0.0-RC6.3.33

# Continue ChaseHQ-Native 0.67.0.0-RC6.3.33: Genuine CPU-B pair checkpoint replay

Predecessor RC6.3.32 Windows QUALIFIED; exact source SHA256 `3f72489f25b144cda5ca7d7416670b02e7f7fa02b70fc32811320943b75bccb6`; Windows qualifier-reported package SHA256 `cb4582d8abcac2f615de51a99065fb54f45d1f06cbed271135e146fe4f7e4c9e` (dist not uploaded). RC6.3.32 real native reconstruct and fresh qualify PASS, Windows CTest 18/18, Workbench 001 ten commands, Full Regression 002 1153. Independent CHQ6332.zip manifest 1064/1064 authentic native files verified.

RC6.3.33 chqdev 0.5.52 **source candidate**, fresh Windows acceptance pending. A new `M5-RC6.3.33-genuine-CPU-B-checkpoint-replay-and-rollback` gate runs if native reconstruct uses `--live-health --cpu-b-live`. It launches original ROM, generated native pair (TST.W+BEQ) and forced original ROM rollback in independent real ChaseHQRuntime processes from **identical stage1-target-immediate-pre-damage-6333.chqstate** at absolute frames 6337 and 6340. Five semantic streams are exact-compared: native-cpu-b-pair-state.tsv, -bus.tsv, -irq.tsv, native-machine-endstate.json, bus.log. Requires native commits >0, 1 expected explicit rollback, authentic pair return states >0, fresh deterministic replay of original and native at 6340, and SHA-pinned checkpoint, binary and generated DLL. On a mismatch write first-divergence witness and invoke real RC6.3.32 subprocess frame reducer for affected original/native stream; retain all evidence even if FAIL. No ROM edits, no generated function-wide substitution. A copied genuine trace is separately mutated only as a fail-closed diagnostic NEGATIVE CONTROL.

Prior Windows CHQ6332.zip supplies genuine sample proof that the 5 signals match original/native/forced at 6340 and a mutation is detectable; this is NOT a fresh RC6.3.33 Windows execution. The natural checkpoint is authentic original-ROM; controlled health-case outputs elsewhere in the archive are not natural gameplay. Complete CPU-B function CFG/entry/callers/exits, 0x0518 and pre-checkpoint writer, global scheduler IRQ timing, and simultaneous native replacements remain BLOCKED. NativePromotionAllowed=false.

Acceptance from freshly extracted source root: `New-Item -ItemType Directory -Force 'C:\Projects\Evidence' | Out-Null; .\chqdev.bat build debug; if ($LASTEXITCODE -eq 0) { .\chqdev.bat native reconstruct --fresh --live-health --cpu-b-live --out C:\Projects\Evidence\CHQ6333 }`. One native evidence ZIP `C:\Projects\Evidence\CHQ6333.zip`. Require all earlier gates plus **M5-RC6.3.33-genuine-CPU-B-checkpoint-replay-and-rollback PASS** and CHQDEV NATIVE RECONSTRUCT: PASS. Only then `.\chqdev.bat qualify --fresh` to verify Windows CTest 18/18, focused regression Run 001 10 commands and Full Regression Run 002 1153 commands, deterministic archive, workbench/mobile health and promotion. Validate real runId from run-metadata.json and independently hash evidence. Preserve qualified RC6.3.32 immutable. All docs/Workbench Next Steps synced.

See `docs/RC6.3.33_GENUINE_SUBPROCESS_REPLAY_AND_ROLLBACK.md` and `docs/ROADMAP.md` for remaining autonomous native function reconstruction work.

---
# Continue ChaseHQ-Native v0.67.0.0-RC6.3.32

## RC6.3.32 SOURCE CANDIDATE — replay-based original/native divergence reduction (Windows pending)

Qualified predecessor RC6.3.31 immutable source SHA256 `8ca40c81ba661a0a3274ce4203dd541fe34d39743cc914c2a63b07b037061cd4`; its Windows qualifier reported dist SHA256 `004f00ae527e5bacc8cb0791840808ff4e5e4da352c72fb2e37cd9ac8a6bfc76`. CHQ6331.zip five original CPU-B ROM windows show 35 writes at PC 0x04A8, address 0x100802, before/after 0x80, no bit7 change or 0x0518. 18/18 Windows CTest and 1153-command Full Regression passed. Controlled CPU-A health states must not be relabelled natural gameplay.

RC6.3.32 `chqdev native minimize --plan PLAN.json --out NEW_DIRECTORY` executes original/native subcommands independently with identical explicit checkpoint input and frame bound, bounded timeouts and subprocess logs, hashes checkpoint, plan and both executables, captures actual trace files, records per-bound first differing line, binary-searches the earliest reproduced divergent end-frame and runs a fresh confirmation. It DOES NOT establish monotonic divergence at every unsampled frame, input-event minimisation, full function parity or native promotability. CLI emits `replay-report.json` with explicit evidence classification and remaining blockers. The companion verifier in native reconstruct uses **synthetic actual subprocess fixture**, not real ROM-native replay; its code cannot claim a function is converted.

`cpu-b-rc6332-boundary-confidence.json` uses real five-case CPU-B writer evidence from fresh CHQ6332 to identify 0x04A8 with source bit still 0x80, but unknown pre-checkpoint producer, missing 0x0518, indirect CFG/callers/exits and full native function remain blocked. No native dispatcher change.

Windows one-liner from RC6.3.32 root: `New-Item -ItemType Directory -Force 'C:\Projects\Evidence' | Out-Null; .\chqdev.bat build debug; if ($LASTEXITCODE -eq 0) { .\chqdev.bat native reconstruct --fresh --live-health --cpu-b-live --out C:\Projects\Evidence\CHQ6332 }`. Require `M2-RC6.3.32-boundary-confidence PASS`, `M2-RC6.3.32-executable-replay-reducer-smoke PASS`, `CHQDEV NATIVE RECONSTRUCT: PASS`, and automatic `CHQ6332.zip`. Only after that run `.\chqdev.bat qualify --fresh`: Windows 18/18 CTest, focused Run001 ten commands, Full Regression Run002 1153, deterministic archive and final promotable required. Preserve earlier immutable baselines. For actual original/native replay testing, separately author a truthful, SHA-pinned controlled checkpoint plan with runtime EXEs, validated content and comparable trace contracts; sample synthetic demonstration is not real-ROM evidence.

Read `docs/RC6.3.32_CHECKPOINT_REPLAY_AND_FUNCTION_BOUNDARY_CONFIDENCE.md` plus `docs/ROADMAP.md` and `games/chasehq/next-steps.json` for milestones. NEXT: add programmatic native checkpoint scenario generation, real triggered divergence minimization of emulator state, verified full function CFG, complete C++ generator and safe multi-replacement scheduling verification.

---
# Continue ChaseHQ-Native v0.67.0.0-RC6.3.31

## RC6.3.31 SOURCE CANDIDATE — automatic multi-source function blocker/first-divergence tooling (Windows pending)

Fully Windows-qualified RC6.3.30 immutable exact source SHA256 `24e7cf0fa32741f584d74908b7b707afc02797a699f9d40e1ab5c8dd32313a82`, Windows qualifier-reported dist SHA256 `51067d3a2dd9a8d370886619e18c6a0427c207d9f9517ece241e5ed374065ea1` only, not byte-verified independently. Real `CHQ6330.zip` proves 35 sampled original CPU-B BTST sources byte=0x80, SR.Z=0, branch 0x052C across five checkpoint scenarios. `0x0518` not observed. RC6.3.30 Windows fresh PASS 18/18 CTest, Workbench focused Run001 10 commands and Full Regression Run002 1,153.

RC6.3.31 **NEW read-only writer provenance**: original-ROM CPU-A and CPU-B Bus::write postcommit windows intersecting mapped address `0x100802` are recorded in `cpu-b-original-100802-writer-events.tsv` and fail-closed cap. Empty event streams DO NOT imply no writer exists: checkpoint RAM may predate capture, and shared-RAM aliasing not fully proven. `M2-CPU-B-autonomous-function-frontier-and-writer-dossier` joins authenticated Ghidra/ROM sites, existing CPU-B observed-edge CFG frontier, genuine BTST/BNE source-byte values, and writer candidates. It creates `cpu-b-automated-function-and-writer-dossier.json`, always explicitly blocking complete-function promotion until complete CFG/entry/exits/callers, 0x0518, all-instruction generation and full-function original/native/fallback equivalence with IRQ and scheduling are demonstrated. Every CPU-A-controlled health case remains labelled separate from natural, although CPU-B executes genuine ROM code in each.

Reusable `chqdev native diagnose --candidate ID --original FILE --native FILE --out JSON` computes SHA256-pinned first-different-row and compact context; it is a **trace witness only**, NOT deterministic workload/input minimization or actual replay. New writer-cap local tests are synthetic fixtures. `CHQ_RC6330_EVIDENCE_ZIP=/mnt/data/CHQ6330.zip go test ./...` replays authentic prior original-ROM Ghidra/BTST edges but explicitly FAILS certification when missing newly instrumented writer file until synthetic test fixture is inserted.

**Windows acceptance pending.** PowerShell one line from extracted RC6.3.31 root:

`New-Item -ItemType Directory -Force 'C:\Projects\Evidence' | Out-Null; .\chqdev.bat build debug; if ($LASTEXITCODE -eq 0) { .\chqdev.bat native reconstruct --fresh --live-health --cpu-b-live --out C:\Projects\Evidence\CHQ6331 }`

Require `[PASS] M2-CPU-B-autonomous-function-frontier-and-writer-dossier`, `CHQDEV NATIVE RECONSTRUCT: PASS`, `C:\Projects\Evidence\CHQ6331.zip`, and exact per-case original-ROM writer caps/dossier with missing origins honestly marked. **Then** `.\chqdev.bat qualify --fresh` must PASS Windows CTest 18/18, Workbench focused Run 001 (10 commands) and Full Regression Run 002 (1153 commands), deterministic package and authoritative run metadata. Documentation and games/chasehq/next-steps.json must match the Workbench UI. Do not relabel or change existing releases. No new native whole-function promotion. See `docs/RC6.3.31_AUTONOMOUS_FUNCTION_BLOCKERS_AND_DIAGNOSIS.md` and latest `RC6_STATUS.md`.

---

# Continue ChaseHQ-Native v0.67.0.0-RC6.3.30

## RC6.3.30 source candidate — original-ROM CPU-B BTST source-byte provenance (Windows acceptance pending)

Exact immutable fully Windows-qualified RC6.3.29 baseline source ZIP SHA256 `087e973345581eb8603bba99ae69907a3403885449a21d4b9063e745eb094615`; qualification-reported Windows `dist` ZIP hash `4e2150384994fbb05876ff03b666fce6542cafc38ebdf1a2868ce1287bc3c6e8` only, not independently byte-verified. Genuine CHQ6329.zip and 18/18 Windows CTests, focused 10-command Run001 and 1,153-command Run002 Full Regression PASS. Runtime original-ROM selected hooks: CPU-B 0x050E `BTST.B #7,(0x00100802).L` directly precedes 0x0516 `BNE.B 0x052C` on 35 authentic visits. 35 observed status Z flags were CLEAR and 0x0518 never observed. RC6.3.30 new **read-only** byte peek at 0x100802 at original-ROM 0x050E hook, plus next hook SR/ordinal/frame, gates the actual byte bit7 vs 0x0516 SR.Z, crosschecks Ghidra and existing BNE next-PC. It is **not** a bus transaction trace, modified-state execution, or whole-function native parity.

New byte values are synthetic-only in local fixtures; the genuine old ZIP is only a prerequisite, **not** proof of the new byte values. Fresh Windows original-ROM execution and 18/18 CTests + Full Regression are **PENDING**. Fail closed on any Z=1/0x0518 branch until explicit separate original/controlled contract. No autonomous native function promotion.

Windows one-line PowerShell from extracted RC6.3.30 root:

`New-Item -ItemType Directory -Force 'C:\Projects\Evidence' | Out-Null; .\chqdev.bat build debug; if ($LASTEXITCODE -eq 0) { .\chqdev.bat native reconstruct --fresh --live-health --cpu-b-live --out C:\Projects\Evidence\CHQ6330 }`

Require `[PASS] M2-CPU-B-original-ROM-050E-BTST-source-bit7`, `CHQDEV NATIVE RECONSTRUCT: PASS`, one C:\Projects\Evidence\CHQ6330.zip. **Then only** `.\chqdev.bat qualify --fresh`, all CTests and focused/full regressions, upload actual evidence with run-metadata run IDs. Review `games/chasehq/next-steps.json` synced with Workbench and `docs/RC6.3.30_CPU_B_BTST_UPSTREAM_BIT7_SOURCE.md`. Preserve all prior release artefacts immutably.

---

# Continue ChaseHQ-Native v0.67.0.0-RC6.3.29

## Authoritative RC6.3.29 corrective candidate — pending Windows acceptance

Preserve immutable RC6.3.28 source candidate ZIP SHA256 `18b2a4a5c32aa750e57eb525fc6b29697b7db77acca420268e084f7ce8a3ab4f` (genuine Windows ROM audit **FAIL** at first BNE decision because Go required `nextCyclesB > cyclesB` but the real instrumented next instruction hook has the **same batch-level cyclesB**). Its `CHQ6328.zip` genuine evidence manifest had 1007/1007 SHA256/size PASS. The complete corrected Ghidra -> real original-ROM ordered-hook SR/ordinal -> actual immediate next-PC -> aggregate edge audit PASS with 35 authentic visits across 5 checkpoint cases; all 35 SR.Z=0 and next PC 0x052C with equal clocks. RC6.3.29 fixes `nc <= cycle` to `nc < cycle`, retains cross-visit monotonicity, adds negative reversed clock fixture and optional genuine `CHQ_RC6328_EVIDENCE_ZIP` integration test; native code substitution is unchanged.

**Latest qualified baseline remains RC6.3.27**, source SHA256 `deaf140c0ee129804785ee2c9026af240d7ee9d5806c77095eead93795932b39`, Windows tool-reported `dist` SHA256 `828876f06979ce6f08b3970374e796abbc03b1980d561f7e09c32602ea5e5604`; RC6.3.28 was NOT Windows-qualified. RC6.3.29 requires new fresh Windows ROM acceptance and Full Regression; it is not yet promoted. Whole-function native promotion, full CPU-B CFG and observed 0x0518 remain BLOCKED, no false new path claims.

Exact one-line PowerShell from extracted RC6.3.29 root:

`New-Item -ItemType Directory -Force 'C:\Projects\Evidence' | Out-Null; .\chqdev.bat build debug; if ($LASTEXITCODE -eq 0) { .\chqdev.bat native reconstruct --fresh --live-health --cpu-b-live --out C:\Projects\Evidence\CHQ6329 }`

Expected `C:\Projects\Evidence\CHQ6329.zip`, `[PASS] M2-CPU-B-original-ROM-0516-BNE-condition-and-next-PC`, `CHQDEV NATIVE RECONSTRUCT: PASS`. **Only then** run `.\chqdev.bat qualify --fresh`; inspect 18/18 CTests, focused Run001 10 commands, Full Regression Run002 1153 commands, actual run IDs and manifests. Documentation and Workbench Next Steps in the current candidate are authoritative. Preserve older handovers below as history.

---

# Continue ChaseHQ-Native v0.67.0.0-RC6.3.28

## Authoritative current release and outstanding acceptance
Latest FULL Windows-qualified immutable baseline is RC6.3.27, source ZIP SHA256 `deaf140c0ee129804785ee2c9026af240d7ee9d5806c77095eead93795932b39`, Windows tool-reported dist `828876f06979ce6f08b3970374e796abbc03b1980d561f7e09c32602ea5e5604` (dist archive not supplied). Genuine CPU-B BNE site 0x0516 -> 0x052C was observed 7 times in each of five Windows checkpoint cases; all sampled SR.Z=0. CPU-B 0x0518 remains runtime UNOBSERVED.

RC6.3.28 adds read-only capture of CPU-B original-ROM 0x0516 pre-branch SR and the immediate actual next instruction, and crosschecks this against independent original-ROM chronological hooks, Ghidra and edges. Local tests replay actual RC6.3.27 prerequisites but synthesize the NEW next-PC fields, so fresh RC6.3.28 Windows execution is REQUIRED. Never describe 0x0518, full function CFG, IRQ timing or autonomous native promotion as verified. Never modify immutable predecessors. Current source candidate ZIP must be verified by SHA256 before testing.

Windows run: `New-Item -ItemType Directory -Force 'C:\Projects\Evidence' | Out-Null; .\chqdev.bat build debug; if ($LASTEXITCODE -eq 0) { .\chqdev.bat native reconstruct --fresh --live-health --cpu-b-live --out C:\Projects\Evidence\CHQ6328 }`. Upload `CHQ6328.zip` and console output. Required gate `M2-CPU-B-original-ROM-0516-BNE-condition-and-next-PC` PASS, then `CHQDEV NATIVE RECONSTRUCT: PASS`; only afterward `./chqdev.bat qualify --fresh` with 18/18 CTests and focused/full Workbench regressions. Use actual `run-metadata.json` run IDs and independent evidence hashes. Full history and roadmap retained in this source.

# Continue ChaseHQ-Native v0.67.0.0-RC6.3.27

## Authoritative current baseline and task
RC6.3.26 is fully Windows-qualified and preserved; source SHA256 `aee45c18268daa006f13a0bebaed43f61e9dbd804a63cf1d34b426ae12650386`, Windows qualifier dist hash `7cb0ebcfe2ef5076e058a2b1f0d82f29f244fa45cc476f16f6f3561072eff6cf`. RC6.3.25 failed, RC6.3.26 corrected and its real Windows ROM+Full Regression PASS. RC6.3.27 new read-only ACK→vector read→PC/SR writes→handler bus capture crosschecks existing authentic frame/USP/ISP evidence, but this NEW trace has no original-ROM Windows verification until user runs acceptance. Bounded proof does NOT establish board-level IRQ ownership, precise bus cycles/timing, complete scheduler or functions. Native promotion forbidden.

Immediate acceptance from the RC6.3.27 extracted root: `New-Item -ItemType Directory -Force 'C:\Projects\Evidence' | Out-Null; .\chqdev.bat build debug; if ($LASTEXITCODE -eq 0) { .\chqdev.bat native reconstruct --fresh --live-health --cpu-b-live --out C:\Projects\Evidence\CHQ6327 }`; expect `[PASS] M2-CPU-B-original-ROM-IRQ-vector-and-stack-bus-order`, `CHQDEV NATIVE RECONSTRUCT: PASS`, upload `CHQ6327.zip` plus console. Only then run `.\chqdev.bat qualify --fresh` and examine correct runId-based metadata.

---

## Historical handover below

# Continue ChaseHQ-Native v0.67.0.0-RC6.3.26

## Authoritative continuation

RC6.3.25 failed its Windows ROM-backed active-SP check; RC6.3.26 strictly samples both USP and ISP.

## Immediate next task

Perform one native reconstruct then fresh qualify only after genuine gate PASS.


RC6.3.24 is last full Windows-qualified immutable baseline (qualify dist hash 78bc9b7fdaa00a55294f55099c18df82ed69e07b0d53e57b0bffded3308de014). RC6.3.25 source SHA256 64e62b77b118577cecfa2d0d4c3ed6eb57ccc456344f11e6a3229a09e0d525b8 Windows build PASS but new exception frame restoration audit FAIL. Real saved SR has S=0; active SP is switched USP after RTE, not ISP+6. RC6.3.26 implements strict observed USP/ISP stack-bank capture and mode-aware restored SP verification, synthetic user/supervisor tests, no weaker fallback. New v2 snapshots require fresh Windows ROM validation. From extracted root run `New-Item -ItemType Directory -Force 'C:\Projects\Evidence' | Out-Null; .\chqdev.bat build debug; if ($LASTEXITCODE -eq 0) { .\chqdev.bat native reconstruct --fresh --live-health --cpu-b-live --out C:\Projects\Evidence\CHQ6326 }`; after native PASS ` .\chqdev.bat qualify --fresh `. Full function, 0x0518 and bus/IRQ scheduler proof remain BLOCKED.

---

> **AUTHORITATIVE CURRENT — ChaseHQ-Native 0.67.0.0-RC6.3.26 CPU-B DUAL-STACK EXCEPTION RETURN CORRECTIVE SOURCE CANDIDATE (2026-10-15).** Latest fully Windows-qualified baseline RC6.3.24, qualifier-reported dist SHA256 `78bc9b7fdaa00a55294f55099c18df82ed69e07b0d53e57b0bffded3308de014`. RC6.3.25 exact source SHA256 `64e62b77b118577cecfa2d0d4c3ed6eb57ccc456344f11e6a3229a09e0d525b8` is **FAILED Windows ROM acceptance**: handler `0x0474` and RTE `0x04D6` returned to CPU-B user SR `0x0004`, restored PC `0x04D8/0x04DE` correctly, but active A7 switched to USP `0x100800` rather than popped ISP `0x100400`; the original RC6.3.25 verifier incorrectly insisted on `SP+6` for active A7. RC6.3.26 instruments and verifies original-ROM USP and ISP independently, plus saved frame, post-RTE PC/SR and bank-selected active SP; fail-closed synthetic user/supervisor mutations PASS. Prior genuine CHQ6325 evidence supports diagnosis, NOT NEW dual-bank capture. **Fresh RC6.3.26 Windows ROM native and `qualify --fresh` remain PENDING.** `0x0518`, vector/bus timing, full function/native promotion remain BLOCKED. Evidence: `C:\Projects\Evidence\CHQ6326.zip`. See `docs/RC6.3.26_CPU_B_DUAL_STACK_BANK_CORRECTIVE.md`. Previous banners are historical.

# Resume ChaseHQ Native v0.67.0.0-RC6.3.25

## Authoritative continuation

RC6.3.24 is Windows-qualified and immutable. RC6.3.25 is a bounded original-ROM CPU-B exception-frame snapshot and RTE restoration candidate. The new bytes have not yet been seen in Windows; do not infer vector ACK or native function promotion.

## Immediate next task

Run one RC6.3.25 Windows debug + `native reconstruct --fresh --live-health --cpu-b-live --out C:\Projects\Evidence\CHQ6325`, inspect ZIP with genuine stack snapshots and gate PASS, then `qualify --fresh` if successful.

> **AUTHORITATIVE CURRENT - ChaseHQ-Native 0.67.0.0-RC6.3.25 CPU-B EXCEPTION-FRAME RAM/POST-RTE RESTORATION SOURCE CANDIDATE (2026-10-15).** Latest fully Windows-qualified immutable baseline **RC6.3.24**, exact source ZIP SHA-256 `cf1e9c75755c8ee13f8786c552bdf58e67ca930d86a2d5960c179745dd89b1d3`; reported Windows dist SHA-256 `78bc9b7fdaa00a55294f55099c18df82ed69e07b0d53e57b0bffded3308de014` (Windows package bytes not independently uploaded). RC6.3.24 genuine native and 18/18 Windows CTests, focused 10 commands and Full Regression 1,153 commands PASS. RC6.3.25 adds **new read-only original-ROM CPU-B six-byte exception frame RAM peeks** at 0x0474/0x04D6 and post-RTE next-hook PC/SR/SP crosscheck. **Authentic CHQ6324 prerequisite 35/35 PASS; NEW frame data is synthetic in local tests and genuine Windows qualification PENDING.** Not a bus transaction trace, vector ACK, all IRQs, scheduler or native function promotion. `0x0518` remains unobserved; `--out C:\Projects\Evidence\CHQ6325`. See `docs/RC6.3.25_CPU_B_EXCEPTION_FRAME_RESTORATION.md`. Older banners below historical.

# Resume ChaseHQ Native v0.67.0.0-RC6.3.24

## Authoritative continuation

RC6.3.23 is fully Windows-qualified. RC6.3.24 is a new bounded CPU-B original-ROM IRQ handler/RTE selected-hook evidence candidate; ROM-backed Windows acceptance remains pending.

## Immediate next task

Build RC6.3.24 debug on Windows, run one native reconstruction with evidence CHQ6324.zip, inspect the IRQ-RTE hook gate, then qualify --fresh only after native PASS.

RC6.3.23 is the fully Windows-qualified immutable baseline (source SHA256 9d7b33423e6c7f022059e77eca233f7cdae9b497ab5fedbd0eb60765ab75c55a; qualifier Windows dist SHA256 3586f6fada3fc56384fb7a213f9c289a68b998427f22a3bf962ae81c28636e5b reported, not independently verified). RC6.3.24 source candidate adds authenticated CPU-B original-ROM sampled handler-entry 0x0474/RTE 0x04D6 chronology and SP/SR evidence (35 real RC6.3.23 sample pairs), no exception-frame bus or vector proof, no whole function promotion. Run fresh Windows build/native `--out C:\Projects\Evidence\CHQ6324`, then `qualify --fresh` only after native PASS. Preserve RC6.3.23; investigate `0x0518` separately.

---

> **AUTHORITATIVE CURRENT — ChaseHQ-Native 0.67.0.0-RC6.3.24 ORIGINAL-ROM CPU-B IRQ/RTE SELECTED-HOOK CONTEXT SOURCE CANDIDATE (2026-10-15).** Latest fully Windows-qualified immutable baseline **RC6.3.23**, exact source ZIP SHA-256 `9d7b33423e6c7f022059e77eca233f7cdae9b497ab5fedbd0eb60765ab75c55a`, qualifier-reported Windows distribution SHA-256 `3586f6fada3fc56384fb7a213f9c289a68b998427f22a3bf962ae81c28636e5b` (distribution bytes not independently uploaded). RC6.3.23 Windows native, 18/18 CTest and Full Regression 1153/1153 PASS. RC6.3.24 adds fail-closed `M2-CPU-B-original-ROM-IRQ-RTE-hook-context`: authentic CHQ6323.zip selected CPU-B 0x0474 handler-entry and 0x04D6 RTE observations, 7 sampled pairs/case across five cases (35), supervisor-mode, opcode, frame, order and sampled SP consistency. Actual exception-frame bytes/bus transactions, vector ownership, interrupted SR/PC, all IRQs, scheduler and native function promotion NOT VERIFIED. **RC6.3.24 ROM-backed Windows acceptance and `qualify --fresh` PENDING.** Evidence output `C:\Projects\Evidence\CHQ6324.zip`; `0x0518` remains unobserved. See `docs/RC6.3.24_CPU_B_IRQ_RTE_HOOK_CONTEXT.md`. Older status banners below are historical.

# Continue from v0.67.0.0-RC6.3.23

## Authoritative continuation

RC6.3.21 is the latest fully Windows-qualified immutable baseline. RC6.3.22 Windows MSVC build passed, but its new original-ROM CPU-B ordered-hook audit **FAILED** because it rejected the genuine ordinal-zero first event. RC6.3.23 corrects this one off-by-one predicate with authentic `CHQ6322.zip` five-checkpoint ROM/Ghidra replay and negative tests. Windows acceptance and fresh qualification remain PENDING. No whole-function or autonomous native promotion.

## Immediate next task

From the extracted RC6.3.23 root, run one PowerShell line: `New-Item -ItemType Directory -Force 'C:\Projects\Evidence' | Out-Null; .\chqdev.bat build debug; if ($LASTEXITCODE -eq 0) { .\chqdev.bat native reconstruct --fresh --live-health --cpu-b-live --out C:\Projects\Evidence\CHQ6323 }`. Upload the ZIP/log, verify full genuine CPU-B ordered-stack audit, and only after native PASS run `.\chqdev.bat qualify --fresh` to qualify the exact RC6.3.23 candidate. Full CPU-B function and alternate 0x0518 remain blocked.

---

> **AUTHORITATIVE CURRENT — ChaseHQ-Native 0.67.0.0-RC6.3.23 ORIGINAL-ROM CPU-B ORDINAL-ZERO CORRECTIVE SOURCE CANDIDATE (2026-10-15).** Exact immutable RC6.3.22 source archive SHA-256 `c17673a958446875f047bfd1dceab5675ff5e81084949578615edc9929c3684e` passed local tests and Windows build but **FAILED genuine Windows native acceptance** in `M2-CPU-B-ordered-original-ROM-stack-audit`: first authentic selected hook has zero-based ordinal `0`, incorrectly rejected by `ordinal == 0`. RC6.3.23 removes only this incorrect prohibition while retaining strict increasing ordinals, transition cap, original opcode, sampled SP/return, IRQ separation and fail-closed blockers. Authentic failed `CHQ6322.zip` five-case ROM+Ghidra integration replay now PASS; duplicate-zero and malformed-order tests reject. **Latest fully Windows-qualified baseline RC6.3.21**, Windows qualifier SHA-256 `fbfc6c19131a3a10a28de60449a9716111179fe03cc4285f2f970a0e8646ea3a` (not independently byte-verified). RC6.3.23 Windows native acceptance and `qualify --fresh` PENDING; `--out C:\Projects\Evidence\CHQ6323`. Unobserved `0x0518`, full CPU-B CFG/IRQ stack parity and native promotion BLOCKED. See `docs/RC6.3.23_CPU_B_ORDINAL_ZERO_CORRECTIVE.md`. Earlier banners below are historical.

# Continue from v0.67.0.0-RC6.3.22

## Authoritative continuation

RC6.3.21 is the latest fully Windows-qualified immutable baseline. RC6.3.22 adds selected original-ROM CPU-B chronological instruction-hook and sampled stack-pointer evidence. This source candidate is locally tested but Windows ROM/Ghidra native reconstruction and fresh qualification are still pending. No function/autonomous promotion.

## Immediate next task

From extracted RC6.3.22 root run the one-line `chqdev.bat build debug` plus `native reconstruct --fresh --live-health --cpu-b-live --out C:\Projects\Evidence\CHQ6322`; upload ZIP and console log. Verify five genuine ordered-ROM samples and `cpu-b-ordered-original-rom-stack-audit.json` fail-closed. Only after authentic native PASS run `chqdev.bat qualify --fresh`, archive the exact candidate SHA and authoritative Run IDs. Keep Windows PowerShell commands on one line; preserve CRLF/LF compatibility.

---

> **AUTHORITATIVE CURRENT — ChaseHQ-Native 0.67.0.0-RC6.3.22 CPU-B ORDERED HOOK/STACK SOURCE CANDIDATE (2026-10-15).** Exact immutable RC6.3.21 source SHA-256 `bdfbe71f911b2e283132410b3c22fb4159ab044baabb00a4298af3d76d93cd54`; latest Windows fresh-qualified RC6.3.21 release archive SHA-256 `fbfc6c19131a3a10a28de60449a9716111179fe03cc4285f2f970a0e8646ea3a` as recorded by the qualifier (Windows `dist` bytes not independently supplied). Genuine RC6.3.21 ROM/Ghidra native reconstruction PASS; Windows 18/18 CTests, focused 10 commands and Full Regression 1,153 commands PASS. RC6.3.22 instruments *original-ROM* CPU-B instruction hooks to record chronological selected PC visits with ordinal/cycles/SP/SR and marks IRQ vector/RTE sites separately; checks per-checkpoint actual BSR/RTS timeline with sampled stack-pointer pairing. **The selection is filtered and does not establish exhaustive IRQ/scheduling/bus parity, complete CFG, 0x0518 execution or promotable function.** Windows native acceptance and `qualify --fresh` PENDING. Evidence `--out C:\Projects\Evidence\CHQ6322`. See `docs/RC6.3.22_CPU_B_ORDERED_HOOK_STACK_EVIDENCE.md`. Older release banners below are historical.

# Continue from v0.67.0.0-RC6.3.21

## Authoritative continuation

RC6.3.20 is Windows-qualified and immutable (18/18 CTest, 10-command focused and 1,153-command Full Regression PASS); RC6.3.21 nested original-ROM BSR/RTS source implementation is local-only, and no complete CPU-B function promotion is authorized.

## Immediate next task

From extracted RC6.3.21 root use one line: `New-Item -ItemType Directory -Force 'C:\Projects\Evidence' | Out-Null; .\chqdev.bat build debug; if ($LASTEXITCODE -eq 0) { .\chqdev.bat native reconstruct --fresh --live-health --cpu-b-live --out C:\Projects\Evidence\CHQ6321 }`; upload `CHQ6321.zip`, check fresh Ghidra nested audit PASS, then independently run `.\chqdev.bat qualify --fresh` for strict release qualification.

> **AUTHORITATIVE CURRENT — ChaseHQ-Native 0.67.0.0-RC6.3.21 CPU-B NESTED CALL/RETURN SOURCE CANDIDATE (2026-10-15).** Exact immutable 0.67.0.0-RC6.3.20 source SHA-256 `f52d6abd6b2062050313a8e4bd4370311b3d39ebf956aae3510ee81da35b1a33`; latest fresh Windows-qualified 0.67.0.0-RC6.3.20 release archive SHA-256 `37ff80b5063a687765468c8dc2bde6d3b5f9c32e004408631e233b9eb6bdaab1` (reported by the Windows qualifier, actual `dist` ZIP not provided). RC6.3.20 native ROM/Ghidra reconstruct PASS, 18/18 Windows CTests, focused Run 001 ten commands and Full Regression Run 002 1,153 commands PASS. RC6.3.21 extends original-ROM CPU-B call-return research to three nested BSR instructions (`0x4F70/0x4F74/0x4F78`), nested RTS (`0x5062`) and outer RTS (`0x4F7C`), with fresh Ghidra provenance and five original-ROM scenario graphs, seven counted edges per case. **Aggregated edge counts do not prove temporal stack pairing, complete function closure or native replacement**. CPU-B `0x0518` runtime branch remains unseen; no native promotion. RC6.3.21 Windows native acceptance and `qualify --fresh` PENDING. Evidence `--out C:\Projects\Evidence\CHQ6321`. See `docs/RC6.3.21_CPU_B_NESTED_CALL_RETURN_AUDIT.md`. Older document banners below are retained as history.

> **AUTHORITATIVE CURRENT — ChaseHQ-Native 0.67.0.0-RC6.3.20 CORRECTIVE SOURCE CANDIDATE (2026-10-15).** Latest fully Windows-qualified baseline 0.67.0.0-RC6.3.18 SHA-256 `7978d2e4bdac825e81ef9540694b0ccb54edcd5d04c342b2472198dc25500099`. Exact immutable failed 0.67.0.0-RC6.3.19 source archive SHA-256 `0900d52cb41bea82203695517c708e4a399dab971db878ae85f465c0e099dde0`. RC6.3.19 Windows/MSVC build PASS, but `M2-CPU-B-original-ROM-call-return-audit` **FAIL** due solely to string-format comparison of Ghidra `0x000516` vs runtime-canonical `0x00000516` for post-return CPU-B `BTST` at `0x050E`. RC6.3.20 changes the comparison to strict numeric 32-bit equality and adds independent replay of the actual CHQ6319 Windows Ghidra JSON and all five original-ROM traces (PASS after correction), plus negative guards. Full enclosing function CFG, native substitution composition, unobserved `0x0518` path, timing parity and autonomous promotion remain **UNPROVEN/BLOCKED**. Windows RC6.3.20 native acceptance/`qualify --fresh` PENDING. One ZIP: `--out C:\Projects\Evidence\CHQ6320`. See `docs/RC6.3.20_REAL_GHIDRA_ADDRESS_CORRECTIVE.md`. Older banners are historical.

# Continue from v0.67.0.0-RC6.3.20

## Authoritative continuation

RC6.3.18 remains fully Windows-qualified (18/18 CTest, focused Run 001 ten commands, Full Regression Run 002 1,153 commands; `dist` SHA-256 `7978d2e4bdac825e81ef9540694b0ccb54edcd5d04c342b2472198dc25500099`). Exact immutable RC6.3.19 source SHA-256 `0900d52cb41bea82203695517c708e4a399dab971db878ae85f465c0e099dde0`; RC6.3.19 Windows debug build PASS but genuine original-ROM Ghidra M2 call/return gate FAIL because text `0x000516` was incorrectly compared with `0x00000516`. RC6.3.20 corrects numerical CPU-B post-return BTST PC comparison; actual CHQ6319 Ghidra export + five original-ROM path tables independently replay **PASS** (all 7 edges). Mutated/missing/incorrect PC values still FAIL. The CPU-B `0x0518` branch remains unobserved in runtime; enclosing function, native composition and autonomous promotion remain BLOCKED. Native execution code unchanged. CRLF and LF compatibility maintained. Earlier sections below are historical.

## Immediate next task

From RC6.3.20 extracted root run one PowerShell line: `New-Item -ItemType Directory -Force 'C:\Projects\Evidence' | Out-Null; .\chqdev.bat build debug; if ($LASTEXITCODE -eq 0) { .\chqdev.bat native reconstruct --fresh --live-health --cpu-b-live --out C:\Projects\Evidence\CHQ6320 }`. Upload the one resulting CHQ6320.zip plus log whether PASS or FAIL, verify M2 CPU-B original-ROM call-return audit and manifest. ONLY if successful run ` .\chqdev.bat qualify --fresh `, focused and Full Regression; archive exact SHA. Do not promote any unproven function. See docs/RC6.3.20_REAL_GHIDRA_ADDRESS_CORRECTIVE.md.

> **AUTHORITATIVE CURRENT — ChaseHQ-Native 0.67.0.0-RC6.3.19 SOURCE CANDIDATE (2026-10-15).** Immutable RC6.3.18 original authoritative source SHA-256 `7968f432939be3a16de942d46afb8caeeffddba39df3a32c2689c05469530369`; fresh Windows qualified archive SHA-256 `7978d2e4bdac825e81ef9540694b0ccb54edcd5d04c342b2472198dc25500099` (recorded by user Windows qualify), CHQ6318 ROM/Ghidra native reconstruction PASS; 18/18 Windows CTest, focused Run001 10 commands, Full Regression Run002 1,153 commands PASS. RC6.3.19 adds **authentic CPU-B observed JSR→callee→RTS→postreturn provenance** across five original-ROM checkpoints; Ghidra exports new CPU-B 0x050C/0x050E/0x08AC statics and the unobserved 0x0518 branch path (static only). Local replay uses authentic CPU-B TSV and a clearly synthetic static Ghidra double. 0x0518 is NOT runtime proven; no complete function CFG, nested-call closure, timing/composition verification, minimiser or autonomous promotion. RC6.3.19 Windows ROM/Ghidra acceptance is PENDING. One evidence ZIP via `--out C:\Projects\Evidence\CHQ6319`. See `docs/RC6.3.19_CPU_B_ORIGINAL_CALL_RETURN_AND_UNOBSERVED_BNE.md`; older banners are historical.

> **AUTHORITATIVE CURRENT — ChaseHQ-Native 0.67.0.0-RC6.3.18 SOURCE CANDIDATE (2026-10-15).** Exact RC6.3.17 fresh Windows qualification PASS: 18/18 CTest, focused Run001 10 commands, Full Regression Run002 1,153 commands, deterministic Windows release ZIP SHA-256 `6fc40389db4bccd47aef40b7a2c874c43517e2db45b2ac29ced4e72c3f8c1d79`. Genuine CHQ6317 bounded CPU-A/B native/rollback evidence PASS; CPU-B observed research frontier 5,234 edges PASS. RC6.3.18 adds **five extra real-ROM Ghidra continuation instruction sites** (0x04E0/0x04E6/0x04EC/0x0516/0x052C), statically checks lengths/flows and crosswalks with five genuine original-ROM checkpoint cases; entire function CFG, BNE fallthrough, generated whole-function C++, minimiser, composition and native promotion remain BLOCKED. Local static Ghidra cannot run here; RC6.3.18 genuine Windows execution is PENDING. One evidence ZIP: `--out C:\Projects\Evidence\CHQ6318`. See `docs/RC6.3.18_CPU_B_GHIDRA_CONTINUATION_CROSSWALK.md`. Prior banners are historical.

# Continue from v0.67.0.0-RC6.3.18

## Authoritative continuation

RC6.3.17 is the exact Windows-qualified immutable baseline, Windows archive SHA256 `6fc40389db4bccd47aef40b7a2c874c43517e2db45b2ac29ced4e72c3f8c1d79`. Fresh qualification PASSED Windows CTest 18/18, focused Run001 ten commands, Full Regression Run002 1,153 commands. Native original/generated/forced-ROM-rollback bounded CPU-A/B parity passed; CPU-B RC6.3.17 observed edges and research frontier passed. RC6.3.18 introduces genuine headless Ghidra disassembly of five further CPU-B continuation instructions plus strict five-scenario original-ROM edge crosswalk. No complete function proven; native dispatcher and generated substitution are unchanged. CRLF and LF script compatibility preserved.

## Immediate next task

Extract RC6.3.18 and run one PowerShell line: `New-Item -ItemType Directory -Force 'C:\Projects\Evidence' | Out-Null; .\chqdev.bat build debug; if ($LASTEXITCODE -eq 0) { .\chqdev.bat native reconstruct --fresh --live-health --cpu-b-live --out C:\Projects\Evidence\CHQ6318 }`. Upload CHQ6318.zip. Independently verify original-ROM Ghidra continuation crosswalk including 0x0516 branch. After PASS run `.\chqdev.bat qualify --fresh` for CTest, deterministic package, Workbench and full/focused regression. First complete CPU-B function and P0 composition/minimiser remain future work. Older handover sections below are historical only.

# Continue from v0.67.0.0-RC6.3.17

## Authoritative continuation

RC6.3.16 is the exact Windows-qualified immutable baseline, archived SHA-256 `79c94b0090009b5f5dd29dc2feb543d5d758c072919bb428a761342f732a646d`. Windows CTest 18/18, focused regression Run001 10 commands, Full Regression Run002 1,153 commands PASS. Native RC6.3.16 CPU-A/B original/native/rollback bounded gates PASS. RC6.3.17 adds authentic five-checkpoint original-ROM CPU-B two-hop *research* frontier and risk-priority report, not new native function code. Whole-function entry/caller/CFG/exit and M6 composition remain blocked. All older handover sections below are historical only.

## Immediate next task

On Windows extract the immutable RC6.3.17 source candidate, run `New-Item -ItemType Directory -Force 'C:\Projects\Evidence' | Out-Null; .\chqdev.bat build debug; if ($LASTEXITCODE -eq 0) { .\chqdev.bat native reconstruct --fresh --live-health --cpu-b-live --out C:\Projects\Evidence\CHQ6317 }`. Inspect `cpu-b-observed-function-frontier.json` and non-promoting research queue, upload CHQ6317.zip. Only after ROM proof run `chqdev.bat qualify --fresh` to verify Windows CTest, focused/Full regression and release packaging. Next implementation goal: Ghidra/callgraph crosswalk for actual enclosing CPU-B function, not false full-function promotion. Preserve CRLF/LF compatibility.

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

> **AUTHORITATIVE CURRENT — 0.67.0.0-RC6.3.1 DEVELOPMENT SOURCE CANDIDATE (2026-10-13).** RC6.2.6 bounded CPU-A steering instruction M1–M7 technical acceptance SIGNED_OFF from real Windows evidence: 240 native replacements, 240 identical CPU return states, ordered bus/mapped-state parity, genuine forced rollback. RC6.3 adds three real Ghidra-derived multi-instruction generated C++ candidates including target-health branching; offline fixtures PASS but **RC6.3 whole-function autonomous M1–M7 and Windows qualification are NOT SIGNED OFF**. See `docs/RC6.3_AUTONOMOUS_RECONSTRUCTION.md` and `RC6_STATUS.md`. Older CURRENT headings below are historical snapshots only.

> **CURRENT RELEASE CANDIDATE: 0.67.0.0-RC6.2.6** — RC6.2.4 proved 240 actual CPU-A steering hits and 48 false shadow mismatches for raw 0x0FE0. RC6.2.6 repairs M2 census case normalisation and 68000 low-byte sign extension in shadow and native substitution; Windows qualification and ROM-backed M1–M7 acceptance PENDING. See RC6_STATUS.md and docs/RC6.2.6_M1_M7_ACCEPTANCE.md. All older current-status statements are historical.

# Continue from v0.67.0.0-RC6.3.3

## Authoritative continuation
RC6.3.3 is the current development candidate. Run one ROM-backed 5-case natural/controlled original/native/rollback branch matrix; RC6.3.2 one-path live function parity was already proved on Windows. Prior RC6.2.6 bounded instruction sign-off stands; broad RC6.3 autonomy still blocked.

RC6.2.6 repairs two evidenced RC6.2.4 M2/steering signed-index defects; RC6.2.4 guarded native steering opcode replacement, independently generated DLL/SO, original/native/rollback evidence; Windows and ROM-backed technical acceptance still pending; see RC6_STATUS.md and docs/RC6.2.4_M1_M7_ACCEPTANCE.md. Prior version/status narratives below are historical.

## Immediate next task
Qualify exact package on Windows, then run `chqdev.bat native accept --fresh` and independently verify signed-off scope, unreviewed Ghidra enclosing function and evidence ZIP. Never claim full function/pipeline acceptance from single opcode.

**CURRENT RELEASE CANDIDATE: 0.67.0.0-RC6.2.4.** Real opt-in CPU-A `0x820C` guarded generated-library native instruction replacement and original/native/rollback full-state acceptance; Windows and ROM-backed proof pending. Enclosing Ghidra function still unreviewed. See `RC6_STATUS.md` and `docs/RC6.2.4_M1_M7_ACCEPTANCE.md`. All prior “current” paragraphs below are **historical only**.

> **AUTHORITATIVE CURRENT STATUS — v0.67.0.0-RC6.2.2 SOURCE CANDIDATE (2026-10-12).** RC6.2.1 Windows qualification PASS (SHA256 `c8422141d9d079ce522f0b1dc12a95527c5d80e9055b4c874fb356493b46174b`); actual CPU-A Ghidra steering export `0x820C` PASS. RC6.2.2 adds self-contained, fail-closed M1–M7 **acceptance evaluation**, not automatically proven unrestricted native replacement. Full acceptance and Windows qualification must be tested; see `docs/RC6.2.2_M1_M7_ACCEPTANCE.md` and `RC6_STATUS.md`. Prior CURRENT/unqualified statements below are preserved as **historical snapshots, not current release status**.

> **CURRENT RC6.1 DEVELOPMENT — v0.67.0.0-RC6.2.1.1; native steering shadow opt-in, Windows qualification outstanding.** See `RC6_STATUS.md`; older status below is historical.

> **RC6 DEVELOPMENT STATUS (not qualified):** v0.67.0.0-RC6. RC5.3 sections below are historical. See `RC6_STATUS.md` at release root.

> **CURRENT: v0.67.0.0-RC5.3 native sprite-clear guard corrective DEVELOPMENT CANDIDATE (UNQUALIFIED), 2026-10-11.** RC5 remains Windows-qualified, immutable SHA-256 `47def65d75056fb765d5b60dfe5a4c4a21bdcd3214ebb84920dd62a0d8b37ddf`. RC5.1 original shadow corpus passed (96/96 sprites, 24/24 class-6 road). RC5.2 Windows substitution failed: all 85 native attempts were guard-rejected (0 actual skips); final CSV export crashed. RC5.3 fixes the instruction-proven ROM branch fingerprint, uses entry-latched records for the destination guard, adds observed values, and repairs PS5.1 CSV. RC5.3 native equivalence and Windows qualification remain PENDING. See `docs/RC5.3_NATIVE_CLEAR_SUBSTITUTION.md`.

> **CURRENT: v0.67.0.0-RC5.3 native sprite-clear SUBSTITUTION DEVELOPMENT CANDIDATE (UNQUALIFIED), 2026-10-11.** The original Windows-qualified RC5 parent SHA-256 `47def65d75056fb765d5b60dfe5a4c4a21bdcd3214ebb84920dd62a0d8b37ddf` remains immutable. RC5.1 headless live-shadow corpus PASS 12/12 cases (96/96 sprite transfers and 24/24 class-6 road generator comparisons); RC5.3 adds an experimental, opt-in native substitution of the trailing sprite-clear DBRA loop ONLY. Bulk copier and final RTS remain original ROM; normal emulation is default. RC5.3 Windows native A/B equivalence and exact-archive qualification pending. See `docs/RC5.3_NATIVE_CLEAR_SUBSTITUTION.md`; all older release-status headings below are historical.

> **RC5 Windows qualification update, 2026-10-11 (UNQUALIFIED):** Windows `chqdev.bat qualify --fresh` passed clean/doctor/preflight/build/CTest/package/runtime/focused, but Full Regression failed at command 875 on `rc1-player-asset-parity`: 76,050/76,800 pixels differed because SDL F1/B forensic presentation contaminated the live screenshot while structured snapshot showed true gameplay. Source corrective offers `screenshot PATH composited` used only by parity; retains exact-zero pixel assertion and normal screenshot presentation, restores SDL user display immediately. **Corrective requires fresh Windows qualification and is not promoted.** See `docs/RC5_WINDOWS_PARITY_CORRECTIVE.md`.

> **RC5 SOURCE CANDIDATE (`0.67.0.0-RC5`), NOT WINDOWS-QUALIFIED.** Original RC4.7 source archive supplied by user; RC5 integrates offline native C++ sprite/road candidates and standalone synthetic Windows preview, updates live Workbench Next Steps. No live SHADOW/COMPARE or native replacement is implemented. Follow `docs/RC5_NATIVE_PREVIEW_AND_SHADOW.md` and exact Windows `chqdev.bat qualify --fresh` gate before promotion. Older version status sections below are historical.

## RC4.7 — Static Analysis / REA tooling candidate

See `docs/REA_GHIDRA_RC47.md` and `docs/DOCUMENTATION_AUDIT_0.67.0.0-RC4.7.md`. **Current candidate RC4.7; promoted parent RC4.6. Windows qualification pending.** Dedicated Workbench REA workspace, bounded `ghidra.wait`, Ghidra run evidence attachment, corrected result path, Java/Jython preflight and flow-edge export. No Capstone library is shipped in this candidate; source vendoring is blocked pending a verified upstream archive. No native emulator CPU replacement.

> **AUTHORITATIVE PROMOTED BASELINE — v0.67.0.0-RC4.6 (2026-10-10).** Windows `chqdev.bat qualify --fresh` reports **11/11 stages PASS** for exact frozen ZIP SHA-256 `9e76a107e19f3e28a54a8a8b3bdaed630511e02f1da3e3e7ae274c07d863580f`. User explicitly authorised promotion following focused, full, and successful headless Ghidra/REA crosswalk evidence. This is a **post-qualification documentation overlay**; it does not change the immutable tested archive. See `docs/RELEASE_VERIFICATION_RC46.md`, `promotion/PROMOTION_RECORD.json`, and `docs/REA_GHIDRA_NEXT_SCOPE.md`. Earlier RC4.6 “candidate/unqualified” statements below are **historical build-time notes, not current release status**.

# v0.67.0.0-RC4.4 candidate handover

## Authoritative continuation
**CURRENT: v0.67.0.0-RC5 source candidate** from uploaded RC4.7 ZIP; original ROM execution remains default; two offline candidate C++ functions compiled, synthetic preview bundled. No RC5 Windows qualification. Strict `chqdev.bat qualify --fresh` still required. See `docs/RC5_NATIVE_PREVIEW_AND_SHADOW.md`.
**RC4.7 unqualified candidate v0.67.0.0-RC4.7; promoted baseline RC4.6**. Release scope: `docs/REA_GHIDRA_RC47.md`. No Windows qualification yet.
This is an unpromoted RC4.4 source candidate built from Windows-qualified (unpromoted) RC4.3.2. The current evidence base is Runs 018-029. The independent CPU-A clip/index shadow is read-only and opt-in, and direct REA raw 68000 integration is *not* implemented. See `docs/NATIVE_PROJECTION_RC44.md`.

## Immediate next task
Qualify v0.67.0.0-RC5 on a real Windows host only after completing live shadow and remaining P0 work. Update Workbench Next Steps from `games/chasehq/next-steps.json`, run strict build/CTest/focused/full/package/verification gates, and retain release candidate classification pending all gates.
Qualify exact RC4.7 archive on Windows via `chqdev.bat qualify --fresh`; run genuine Ghidra wait/evidence proof, and leave RC4.6 promoted until explicit approval.
From fresh exact RC4.4 ZIP extraction on Windows, run `.\chqdev.bat qualify --fresh`; require zero failing CTests, deterministic packaging, runtime/Workbench readiness, focused and Full Regression, and promotable. Do not infer qualification from Linux CPU tests. After green gates, execute controlled canonical-checkpoint CPU-A shadow trials, address the Go regression structural command-budget failure without reducing assertion coverage, then assess independent native geometry.

## RC4.3.2 — authoritative history CTest signature corrective (unqualified)

The exact RC4.3.1 Windows `chqdev.bat qualify --fresh` log passed configuration and debug compilation, then failed CTest 10/13, `authoritative_history_tests`, with `Authoritative history function absent`. The production `Get-AuthoritativeScriptRuns` function is present, but its declaration changed in RC4.3.1 from `()` to `([string]$onlySession='')`. The PowerShell test's regex still matched only `()` and falsely reported absence. Correct the extractor to allow parameters and whitespace, **without dropping** its 320-run ordering/pagination/version tests. Extend Windows CTest coverage to exercise the new current-session filtering path. Add an independent Go regression guard against repeating this signature drift. Do not change production execution, the existing run-history performance fix, or Full Regression assertions. 0 tolerated CTest failures; this candidate is NOT promoted until the final exact ZIP passes Windows qualification. The historical Full Regression expanded-command budget failure remains unresolved.

# v0.67.0.0-RC4.3.2 authoritative continuation (current candidate)

## Authoritative continuation
RC4.3 is **NOT PROMOTED OR WINDOWS QUALIFIED**. Maintain v0.67.0.0-RC4.2.3 as the immediate source parent; retain previously Windows-qualified RC4 evidence separately. RC4.3 builds lossless indexed PNG asset atlases, a measured ROM execution census, and an observe-only native execution bridge. See `docs/NATIVE_MIGRATION_RC43.md` for exact commands and limits.

## Immediate next task
Verify fresh RC4.3 packaged archive and bundled Windows executables, resolve the legacy <850 Full Regression structural budget failure **without reducing unique coverage**, then perform `chqdev.bat qualify --fresh` against the exact archive on Windows. Only after green gates investigate independently implemented Shadow candidates. Never silently promote or claim CPU routines replaced.

> Historical unqualified RC4.2.3-line candidate: **v0.67.0.0-RC4.2.3.1** — CHQPACK Browser, overlay inspection and provenance. Windows qualification pending.

> Current development candidate: **v0.67.0.0-RC4** — ROM/CHQPACK source switching and RC3.1 release consistency corrections.

> Current development candidate: **v0.67.0.0-RC3** — CHQPACK runtime content boundary; ROM-free runtime after deterministic import; RC2.1 native rendering retained.

> Exact source baseline: qualified **v0.67.0.0-RC1.1** SHA-256 `ea4ebaafe07cd1c15b64c07ed8e92e32fd0e39cdcc360e31dd7b09c676b77f1d`. RC2 keeps the tooling freeze and moves visible rendering forward. See `docs/DOCUMENTATION_AUDIT_0.67.0.0-RC2.1.md`.

> Historical qualified candidate: **v0.67.0.0-RC1.1** — native sprites default; text/HUD Shadow zero-diff bring-up; native sprite forensic overlap-coverage corrective.

> **RC1.1 qualification corrective:** RC1 Windows focused regression proved 171 zero-diff TC0100SCN text Shadow frames. Full Regression then isolated a forensic-only Native sprite defect: rendered pixels and final owners were exact, but overlap `coverage` collapsed to 1. RC1.1 restores full Original-equivalent overlap density without changing sprite output.


# Continue from v0.66.9.5-RC3

## Authoritative continuation

Continue ChaseHQ-Native from release candidate **v0.66.9.5-RC3**, built directly on the exact **v0.66.9.5-RC2** candidate archive SHA-256 `3aaaebf55cac7cfa9150754556db42438083b52c2965fd51ce36cc5f2bdca0f7`. RC3 is intentionally the final broad tooling pass before native implementation work.

RC3 adds the Native Promotion Framework: Sprite backend modes `original`, `shadow`, `native` and fail-closed `native-only`; native promotion status/reset/assert/capture APIs; automatic first-divergence Original/Native/diff evidence; performance, fallback and blocked-frame counters; a Workbench Native Progress matrix; and a canonical sprite promotion corpus. Sprite rendering remains **REPLACEABLE**, not NATIVE, until that corpus passes on the exact Windows-qualified candidate.

Carry forward the RC2 ROM Sprite/Vehicle Atlas and all evidence-backed semantic vehicle knowledge. Do not silently promote ROM-visual candidates to gameplay semantics. Keep Original authoritative until an independent subsystem has passed its promotion gate.

Tooling is now frozen by policy except where native implementation reveals a concrete blocker. Avoid release churn and avoid new broad diagnostics for their own sake.

## Immediate next task

1. Qualify the exact v0.66.9.5-RC3 Windows archive with `chqdev.bat qualify --fresh`.
2. If green, run `research/scripts/graphics/native-promotion-sprites.chqscript` in the Workbench.
3. If the promotion assert passes with zero Shadow mismatches, zero Native-only blocked frames and zero Original fallbacks, update sprite maturity from **REPLACEABLE** to **NATIVE** and retain Original only as oracle/fallback diagnostic code.
4. Immediately begin the independent **TC0100SCN text/HUD Shadow renderer**, using the same Original -> Shadow -> Native -> Native-only promotion framework.

Do not return to broad tooling work unless step 4 exposes a real implementation blocker.

## Release guard

`chqdev` is the de facto and authoritative build/release process. Final Windows qualification is `chqdev.bat qualify --fresh`; any CTest failure is fatal with zero tolerated CTest failures. Qualification must include deterministic archive packaging/verification, the focused regression, Full Regression and authoritative run-history closure. PowerShell/BAT entry points are shell compatibility wrappers around the same authority rather than independent release processes. Release scripts and guard documents must remain safe under both CRLF and LF line endings.

## Authoritative continuation
Qualified predecessor RC6.3.32 remains the immutable baseline; RC6.3.33 has genuine two-bound replay implementation pending new Windows run.

## Immediate next task
Run RC6.3.33 native reconstruct with --cpu-b-live then qualify --fresh and independently verify manifests and matrix; no function promotion.
