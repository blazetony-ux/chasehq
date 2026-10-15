> **RC6.3.33 authoritative current release metadata.** This document retains its historical content; current version is 0.67.0.0-RC6.3.33. See docs/RC6.3.33_GENUINE_SUBPROCESS_REPLAY_AND_ROLLBACK.md.

> Current development candidate: ChaseHQ-Native 0.67.0.0-RC6.3.32; original-ROM CPU-B writer results from RC6.3.31 are genuine but full function CFG/native promotion remain blocked. Check docs/RC6.3.32_CHECKPOINT_REPLAY_AND_FUNCTION_BOUNDARY_CONFIDENCE.md.

> CURRENT RELEASE 0.67.0.0-RC6.3.31: see `docs/RC6.3.31_AUTONOMOUS_FUNCTION_BLOCKERS_AND_DIAGNOSIS.md`; real Windows new writer capture PENDING, complete function native promotion BLOCKED; older sections historical.

# RC6.3.30 — authentic CPU-B BTST bit-7 source provenance

**Candidate:** `0.67.0.0-RC6.3.30` · `chqdev v0.5.49` · Windows ROM qualification PENDING. Immutable fully Windows-qualified RC6.3.29 source SHA256 `087e973345581eb8603bba99ae69907a3403885449a21d4b9063e745eb094615`, qualifier-only Windows dist hash `4e2150384994fbb05876ff03b666fce6542cafc38ebdf1a2868ce1287bc3c6e8`. Genuine CHQ6329.zip original-ROM evidence shows exactly seven 0x050E->0x0516 sampled instruction pairs per checkpoint in natural/bypass/nonterminal/terminal/overflow, 35 visits; all Z=0 and successor 0x052C. No 0x0518.

## Upstream condition and new capture

Headless Ghidra authentic CPU-B opcode at `0x050E` is `0x0839`, disassembly `btst.b #0x7,(0x00100802).l`, length eight, next instruction `0x0516` is `bne.b 0x0000052c`, opcode `0x6614`. 68000 BTST sets **Z=0 if tested bit7 is 1** and **Z=1 if tested bit7 is 0**. In a genuine, unmodified ROM run, read **one byte** nonmutating from mapped CPU-B `0x100802` at the entry of BTST. Capture the very next original CPU-B hook PC/SR/ordinal. `cpu-b-original-050e-btst-source.tsv` and `cpu-b-original-050e-btst-cap.json` are emitted alongside prior evidence. Native reconstruction audit `M2-CPU-B-original-ROM-050E-BTST-source-bit7` rejects wrong Ghidra site, wrong byte address/opcode, missing/dropped/pending records, nonconsecutive ordinal, wrong BNE PC, mismatched Z, disconnected previous 0x0516 branch report, or unexpectedly observed 0x0518. It writes `cpu-b-original-rom-050e-btst-bit7-source.json`. No game code or flags are modified; this is NOT a real hardware bus access/cycle trace and cannot alone certify full CFG.

## Local and Windows provenance boundary

Optional test `CHQ_RC6329_EVIDENCE_ZIP=/path/CHQ6329.zip go test ./...` imports genuine earlier original-ROM Ghidra, hooks, observed BNE successors and branch edges. That prior ZIP contains **no new RC6.3.30 byte**. Test must first refuse to certify missing new data, then inject explicitly **synthetic sourceByte=0x80** consistent with old Z=0 to exercise parser and negative mutations. Synthetic fixture PASS does NOT establish the actual sampled bytes from ROM. Fresh Windows native replay is mandatory to confirm the new byte observations.

## Safety and release gates

No synthetic forced 0x0518 is presented as original ROM; any Z=1 or new destination fails closed until separately reviewed. Whole CPU-B CFG, all indirect callers/exit paths, irq/scheduler precise parity and native autonomous whole-function promotion blocked.

PowerShell from extracted RC6.3.30 root (one line):

`New-Item -ItemType Directory -Force 'C:\Projects\Evidence' | Out-Null; .\chqdev.bat build debug; if ($LASTEXITCODE -eq 0) { .\chqdev.bat native reconstruct --fresh --live-health --cpu-b-live --out C:\Projects\Evidence\CHQ6330 }`

Expect `C:\Projects\Evidence\CHQ6330.zip`, `[PASS] M2-CPU-B-original-ROM-050E-BTST-source-bit7` and `CHQDEV NATIVE RECONSTRUCT: PASS`. **Then** `.\chqdev.bat qualify --fresh` requiring 18/18 Windows CTest and focused/full regression Run001/Run002 10/1153 commands and verified run metadata. Keep older source/archive immutable, use automatically produced evidence ZIP. Workbench Next Steps synced from `games/chasehq/next-steps.json`.

Release guard: preserve CRLF/LF newline handling and strict Windows `qualify --fresh` gates.
