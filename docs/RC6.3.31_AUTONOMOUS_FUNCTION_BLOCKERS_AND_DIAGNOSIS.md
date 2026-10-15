> **RC6.3.33 authoritative current release metadata.** This document retains its historical content; current version is 0.67.0.0-RC6.3.33. See docs/RC6.3.33_GENUINE_SUBPROCESS_REPLAY_AND_ROLLBACK.md.

> Current development candidate: ChaseHQ-Native 0.67.0.0-RC6.3.32; original-ROM CPU-B writer results from RC6.3.31 are genuine but full function CFG/native promotion remain blocked. Check docs/RC6.3.32_CHECKPOINT_REPLAY_AND_FUNCTION_BOUNDARY_CONFIDENCE.md.

# RC6.3.31 — Multi-source function investigation, writer provenance and first-divergence tooling

**Release:** `0.67.0.0-RC6.3.31` · `chqdev v0.5.50` · **Windows real-ROM acceptance PENDING**, full-function native promotion BLOCKED. Baseline: exactly qualified immutable RC6.3.30 source ZIP SHA-256 `24e7cf0fa32741f584d74908b7b707afc02797a699f9d40e1ab5c8dd32313a82`; Windows qualifier-reported dist hash `51067d3a2dd9a8d370886619e18c6a0427c207d9f9517ece241e5ed374065ea1` (not independently hashed).

## 1. Evidence-led function boundary and producer research

Genuine CHQ6330.zip RC6.3.30 evidence: 35 original CPU-B instruction-hook samples at `0x050E`, RAM `0x100802` byte `0x80`, instruction `BTST.B #7,(0x00100802).L`, immediate BNE at `0x0516` with zero flag clear and successor `0x052C`. Five health scenarios: natural unmodified gameplay health, bypass/nonterminal/terminal/overflow controlled CPU-A entry RAM, **all CPU-B instructions still executed by original-ROM emulator**. `0x0518` still unobserved. This identifies the condition, not the writer, and cannot certify complete function CFG.

RC6.3.31 instruments **actual posted CPU-A and CPU-B Bus::write calls** overlapping address `0x100802`, including width, old/new byte, CPU space, executing-PC attribution, frame, CPU-B transition ordinal and cycle sample. The trace uses `cpu-b-original-100802-writer-events.tsv` and fail-closed `cpu-b-original-100802-writer-cap.json` (zero records valid only as *NO_WRITER_OBSERVED_IN_BOUNDED_WINDOW*). It does **not** patch the ROM, force the branch, invent a producer, or prove cross-CPU shared RAM aliasing; an earlier unknown writer can have established the checkpoint byte. No event cap must always fail. No dropped events permitted. Event ordering/byte width/overlap/CPU space and counts strictly checked.

`M2-CPU-B-autonomous-function-frontier-and-writer-dossier` cross-checks the new writer observation with existing authenticated original-ROM five-case BTST/BNE source and Ghidra-observed CPU-B function frontier. `cpu-b-automated-function-and-writer-dossier.json` explicitly enumerates observed A/B writers, possible writer PCs and bit7 changes per case, plus missing/static CFG/entry/caller/exit and IRQ/scheduling limitations. It always sets `verifiedCompleteCFG=false`, `writerProvenanceComplete=false`, `wholeFunctionNativeEquivalence=false` and `nativePromotionAllowed=false`. This is an **automated blocker dossier**, not a full function replacement. Complete function-entry/exit discovery, synthetic and authentic 0x0518 conditions, native ABI and scheduling remain incomplete.

## 2. Reusable deterministic first-divergence witness

`chqdev native diagnose --candidate ID --original FILE --native FILE --out JSON` compares bounded ordered text streams, anchors full source SHA256, finds the first mismatched line, emits up to four preceding common rows and reports whether streams are equal. Result is a *minimal first-difference trace witness*, **not** an autonomously reduced input, scenario or independently replayed failure; `inputScenarioMinimized=false` and `nativePromotionAllowed=false`. It can consume original/native evidence captures produced by future full-function candidates without writing a dedicated comparator per opcode. Tests cover identical, changed, truncated and malformed samples.

## 3. Tests, provenance and blockers

Optional genuine-prerequisite test: `CHQ_RC6330_EVIDENCE_ZIP=/path/CHQ6330.zip go test ./...` verifies the exact older original-ROM Ghidra, 35 BTST sources and branches. The RC6.3.30 archive **does not** include RC6.3.31 writer captures. The new verifier must refuse to certify when the cap is absent, then a test can inject distinctly **synthetic empty writer caps** to check dossier structure without claiming any writer observation. Separate negative mutation tests reject corrupt CPU, width, addresses, byte ranges, frame order and cap count/drops. Local Linux 15/15 CTests are CPU-only and not a Windows substitute.

The pipeline must not enable unrestricted multi-function native substitution or conflate natural vs controlled cases. A candidate with missing 0x0518/callers/CFG or unknown precheckpoint writer must be reported as BLOCKED rather than silently promoted.

## 4. Windows acceptance procedure

PowerShell single-line in extracted RC6.3.31 root:

`New-Item -ItemType Directory -Force 'C:\Projects\Evidence' | Out-Null; .\chqdev.bat build debug; if ($LASTEXITCODE -eq 0) { .\chqdev.bat native reconstruct --fresh --live-health --cpu-b-live --out C:\Projects\Evidence\CHQ6331 }`

Expected `C:\Projects\Evidence\CHQ6331.zip`; `M2-CPU-B-autonomous-function-frontier-and-writer-dossier PASS` and `CHQDEV NATIVE RECONSTRUCT: PASS`. Examine genuine 5-case writer caps; zero writers is acceptable as honest missing evidence. **Only after native acceptance passes**, run `.\chqdev.bat qualify --fresh`, check Windows 18/18 CTest, focused regression Run001 10 cmds, Full Regression Run002 1153 cmds, deterministic archive and run-history. One automatic evidence ZIP per run. Actual run IDs from run-metadata.json. Preserve RC6.3.30 immutable.

## 5. Following milestones

RC6.3.32+: extended original-ROM survey to capture upstream writers before checkpoints, separately labelled controlled 0x0518 path, complete Ghidra CFG/function-entry/callers/exits including indirect targets and unsupported instructions, broader C++ generator with full-function original/native/forced equivalence and actual replay-based input minimisation. Caching and multiple-native-replacement IRQ/scheduling composition are still required for autonomous reconstruction.

Release guard: LF and CRLF compatibility; immutable artifacts must be preserved.
