# Next milestone — dedicated REA/Ghidra Workbench and reliable static-analysis workflow

**Authoritative baseline:** v0.67.0.0-RC4.6, immutable Windows-qualified ZIP SHA-256 `9e76a107e19f3e28a54a8a8b3bdaed630511e02f1da3e3e7ae274c07d863580f`. **This document proposes future work; the tab and corrective APIs described below are NOT in RC4.6.**

## Current proof and context

- Run 032's **genuine** Ghidra 12.1.4 headless analysis imported CPU-A `cpu_main.bin` (512 KiB, SHA-256 `5cf1c9e786a39795ebd8e99c8e8be0d2398cc4f74221067cf2844945308d6c5a`, Motorola 68000 big-endian, image base `0x0`) and produced eight instruction-site windows.
- Ghidra auto-analysis identifies `FUN_0000a10c` (candidate region `0xA10C–0xA129`), including original health decrement `subq.w #1,(-0x7d52,A5)` at `0xA112`. The user-inspected Workbench record correlated seven distinct executed PCs/7 hits in the authentic damage scenario; confidence remains **hypothesis**.
- Ghidra associates cache-copy PC `0x40682` with candidate `0x40672–0x4082F` and yields disassembly at projection PCs. It does **not** prove a containing projection function or fully resolve calling/CFG relationships.
- Run 035 (19 commands PASS) used `rea.crosswalk` on three verified gameplay census scenarios and generated three reports; the compact Native Lab UI is visibly too constrained to browse them comfortably.

## Top-level tab (P0 UX)

Create an independently navigable **Static Analysis / REA** main tab, with a link from Native Lab. Desktop layout: full-width filters/report picker/job controls and a flexible split view for function list and function detail (disassembly, call/branch/data xrefs, observed PCs/hits, per-checkpoint coverage, confidence, provenance and evidence links). Avoid giant raw-JSON-only displays; provide raw JSON/export when requested. On narrow widths, stack the panes and keep report/function selectors accessible. Report selection must populate functions automatically, retaining selection when valid.

Show job state with a compact status, elapsed time, logs/progress and Download Evidence action; do not confuse `PASS` static analysis with `0` imported census reports. Surface the absence of staged census files and provide a same-session import route. Keep the ROM SHA, Ghidra version, image base, static-confidence and whether REA's external agent was actually invoked prominent.

## One-script research API (P0 reliability)

Prefer an asynchronous job with an explicit await operation, rather than general-purpose unbounded polling. Proposed (not RC4.6 syntax):

```text
api ghidra.start
api ghidra.wait timeout=600000 poll=2000
api ghidra.results
api ghidra.import
api ghidra.evidence.attach include=all
api rea.reports
```

Guarantee terminal-state correctness (PASS/FAIL/TIMEOUT/CANCEL), bounded timeout and cancellation, no backend monopolisation, same desktop/mobile/CLI behaviour, fail-closed input paths and ROM ownership. In a `finally` path, attach relevant logs/metadata even on failure without masking the original error. Store run id, job id, ROM provenance and downstream report ids in the standard evidence manifest.

## Environment and analysis improvements

1. `chqdev doctor` and runtime lifecycle should detect/select a suitable local JDK for spawned worker processes (without modifying global Windows env), verify Ghidra and a suitable script engine; for Jython-based script workflows, inspect version-matched extension availability and require consent before optional local installation. Prefer port to supported Java/PyGhidra over permanent legacy Jython dependency.
2. Fix `ghidra.results` to resolve files from the correct job run folder; make `ghidra.import` report missing source census dependencies explicitly, and locate previously captured reports through provenance rather than guessed sibling directories.
3. Recover true projection routine entry/exit/branches and callers using Ghidra/Capstone with manual review checkpoints, beginning at `0x434EE` and sprite writer `0x43828`; never upgrade a hypothesis to verified merely on matching execution counts.
4. Optional, source-buildable Capstone 68000 bridge for independent disassembly and structured Script Console/CLI/Workbench output, PC attribution, side-by-side Musashi/Ghidra comparisons; no CPU replacement.
5. Live checkpoint-specific ROM execution census, target-PC counts and optional projection shadow in existing Workbench session. Avoid restarting emulator for routine investigations.

## Quality gates

Tests for correct release/ROM hash, wrong ROM and bad static base rejection, Ghidra pass/fail/logs, Java missing, Jython missing, status truthfulness, `ghidra.wait` timeout/cancel, correct bundle retention, missing census explanation, UI report selection, responsive desktop/mobile panes and no hidden backend stalls. Preserve all current regression checks, full Windows `chqdev qualify --fresh`, no arbitrary command execution from API or raw ROM export. Structural Go regression budget issue remains unresolved and cannot be hidden.
