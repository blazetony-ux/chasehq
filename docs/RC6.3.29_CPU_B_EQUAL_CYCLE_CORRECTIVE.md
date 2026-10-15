> **RC6.3.33 authoritative current release metadata.** This document retains its historical content; current version is 0.67.0.0-RC6.3.33. See docs/RC6.3.33_GENUINE_SUBPROCESS_REPLAY_AND_ROLLBACK.md.

> Current development candidate: ChaseHQ-Native 0.67.0.0-RC6.3.32; original-ROM CPU-B writer results from RC6.3.31 are genuine but full function CFG/native promotion remain blocked. Check docs/RC6.3.32_CHECKPOINT_REPLAY_AND_FUNCTION_BOUNDARY_CONFIDENCE.md.

> CURRENT RELEASE 0.67.0.0-RC6.3.31: see `docs/RC6.3.31_AUTONOMOUS_FUNCTION_BLOCKERS_AND_DIAGNOSIS.md`; real Windows new writer capture PENDING, complete function native promotion BLOCKED; older sections historical.

> CURRENT BUILD: 0.67.0.0-RC6.3.30 — read-only BTST bit7 source-verification candidate; Windows ROM gate pending; historical body follows.

> **RC6.3.30 current source-candidate context (2026-10-15):** immutable RC6.3.29 Windows-qualified baseline; new 0x050E BTST byte evidence Windows execution pending, no whole-function native promotion. Historical discussion below.

# RC6.3.29 — strict original-ROM CPU-B BNE equal-cycle verifier corrective

**Source candidate:** `0.67.0.0-RC6.3.29` · `chqdev v0.5.48` · Windows qualification **PENDING**. Preserve immutable RC6.3.28 source ZIP SHA256 `18b2a4a5c32aa750e57eb525fc6b29697b7db77acca420268e084f7ce8a3ab4f` and latest qualified RC6.3.27 source SHA256 `deaf140c0ee129804785ee2c9026af240d7ee9d5806c77095eead93795932b39`. Actual RC6.3.27 Windows dist hash `828876f06979ce6f08b3970374e796abbc03b1980d561f7e09c32602ea5e5604` is tool-reported only; bytes weren't uploaded.

## Authentic Windows failure and exact source defect

On Windows RC6.3.28, MSVC/SDL x64 debug build PASS, previous M1–M7 gates PASS, but `[FAIL] M2-CPU-B-original-ROM-0516-BNE-condition-and-next-PC - natural decisions: untrusted BNE decision row 1` and `CHQDEV NATIVE RECONSTRUCT: FAIL`. `CHQ6328.zip` (1007 SHA256+size entries verified, 0 malformed/extra) contains **35 real CPU-B per-visit next-PC samples** across the five original-ROM natural/bypass/nonterminal/terminal/overflow cases, seven each. At original instruction `0x0516`, opcode `0x6614`, all sampled `SR.Z=0`, all actual next-PC values `0x052C` and 0x0518 **unobserved**. Both branch and next instruction hooks sampled identical `cyclesB` in every visit (first natural frame 6333, `cyclesB=1241210430` at both hooks). RC6.3.28 Go analyser incorrectly rejected equality via `nc <= cycle`.

## Narrow corrected evidence contract

In `tools/chqdev/native_rc6328.go` the check now rejects only `nextCyclesB < cyclesB`, while allowing equality. Strict increasing **branch ordinals** and strict increasing **cyclesB between separate branch visits**, frame continuity, original-ROM opcode, SR, Ghidra, observed PC edge consistency, exact cap counts and zero drop/pending remain enforced. No original CPU-B emulator/runtime instruction or native substitution code changed. `src/runtime.cpp` records genuinely equal clocks; no synthetic data is used to claim a new path. If 0x0518 is observed, the existing audit fails closed until Ghidra/path/ABI review; complete function/whole-function promotion still BLOCKED.

## Permanent tests and genuine provenance

The unit negative fixture with branch cycle=100 and next cycle=99 is rejected; equality 100→100 and 180→180 is valid, yet cross-visit cycles and ordinals must increase. New `TestRC6329AuthenticWindowsCHQ6328Integration` uses optional environment variable `CHQ_RC6328_EVIDENCE_ZIP` to replay **the complete original-ROM Ghidra/call-return and next-PC branch frontier audit** with raw genuine Windows ZIP bytes, no synthetic next PC. It asserts five cases, 35/35 Z-clear taken decisions, equality of every next-hook cycle, 0x0518 still unseen, no complete CFG/whole-function native promotion, plus mutation rejection with next cycle less than original. Without the user ZIP the optional integration is skipped; all synthetic guards still run. Local validation uses the genuinely uploaded evidence and must report test PASS.

## Release and Windows acceptance

New release identity, `chqdev v0.5.48`, version-specific research regression and SCRIPT_CATALOG row, Workbench Next Steps and documentation. Portable preflight, Go full suite, Linux native CPU-only 15/15 CTests, cross-built Windows amd64+arm64 chqdev, byte-identical deterministic ZIP, independent file-by-file authoritative manifest audit are local requirements (record exact actual results in validation status, not claims made by this plan).

Single-line Windows PowerShell from extracted candidate root:

`New-Item -ItemType Directory -Force 'C:\Projects\Evidence' | Out-Null; .\chqdev.bat build debug; if ($LASTEXITCODE -eq 0) { .\chqdev.bat native reconstruct --fresh --live-health --cpu-b-live --out C:\Projects\Evidence\CHQ6329 }`

Require `[PASS] M2-CPU-B-original-ROM-0516-BNE-condition-and-next-PC` and `CHQDEV NATIVE RECONSTRUCT: PASS`, evidence ZIP `C:\Projects\Evidence\CHQ6329.zip`. **After native PASS only:** `.\chqdev.bat qualify --fresh`; demand fresh 18/18 CTests, focused regression Run001 and Full Regression Run002 with actual run-metadata runIDs, all manifests verified. Retain RC6.3.27 qualified until evidence proves RC6.3.29.

Release documentation syntax: preservation of CRLF/LF line endings and authoritative qualification gates is mandatory.
