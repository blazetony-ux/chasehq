> **RC6.3.33 authoritative current release metadata.** This document retains its historical content; current version is 0.67.0.0-RC6.3.33. See docs/RC6.3.33_GENUINE_SUBPROCESS_REPLAY_AND_ROLLBACK.md.

# RC6.3.32 — Checkpoint-replay divergence reduction and CPU-B function confidence

**Version:** `0.67.0.0-RC6.3.32` / `chqdev v0.5.51`. Immutable source baseline RC6.3.31 SHA256 `8ca40c81ba661a0a3274ce4203dd541fe34d39743cc914c2a63b07b037061cd4`; qualifier-reported Windows `dist` SHA256 `004f00ae527e5bacc8cb0791840808ff4e5e4da352c72fb2e37cd9ac8a6bfc76` separately. **Fresh RC6.3.32 Windows original-ROM acceptance PENDING. Full function native promotion BLOCKED.**

## What is actually new

`chqdev native minimize --plan PLAN.json --out NEW_DIRECTORY` is a reusable **executable-backed replay reducer**, not just RC6.3.31's saved TSV comparator. A plan specifies one absolute, immutable checkpoint, inclusive starting/ending end-frame bounds (maximum span 8192), separate absolute executables and command arrays with `{checkpoint}`, `{frame}`, `{run_dir}` substitutions, a trace pathname rooted inside the isolated run directory, and a subprocess timeout (1–120s). No shell is involved. Each trial launches separate original and native processes and compares emitted trace content. It rejects missing/failed traces, equal streams when divergence is requested, reused output directories, altered executable/checkpoint hashes, malformed plans or timeouts. SHA-pinned per-frame traces, executable process logs, comparison witnesses and `replay-report.json` are retained. Replay bisection reduces the earliest **observed end-frame bound** and reruns a fresh pair at the minimum to confirm it. Unsampled intermediate bounds are not a proof of monotonicity or globally earliest divergence; event/input delta-debugging is not implemented. Whole function parity and native promotion always remain false.

A built-in `native replay-fixture` is reserved for reproducible synthetic E2E testing (two real processes). Native reconstruction runs this fixture and proves first divergence at synthetic frame 5. **Synthetic process results are not original Chase H.Q. ROM evidence and must never be included as complete-function proof.** The real-ROM replay command is opt-in with a separate user-authored plan; no plan is silently inferred from unrelated trace formats.

### Plan schema example — adjust executable/trace arguments to the exact verified runtime build

```json
{
 "schema":"chq-rc6332-checkpoint-replay-plan-v1", "candidate":"CPU-B-BEQ-BOUNDARY", "evidenceClass":"CONTROLLED_CHECKPOINT_REPLAY",
 "checkpoint":"C:/Projects/ChaseHQ/checkpoints/known.chqstate", "startFrame":6333,"endFrame":6340,"timeoutSeconds":60,
 "original":{"executable":"C:/Projects/ChaseHQ/out/build/x64-Debug/Debug/ChaseHQNative.exe", "args":["--roms","C:/Projects/ChaseHQ/roms/chasehq","--load-checkpoint","{checkpoint}","--frames","{frame}","--logs","{run_dir}/logs","--trace-cpu","B","--trace-pc","04D8:0540"],"trace":"{run_dir}/logs/trace.log"},
 "native":{"executable":"C:/Projects/ChaseHQ/out/build/x64-Debug/Debug/ChaseHQNative.exe", "args":["--roms","C:/Projects/ChaseHQ/roms/chasehq","--load-checkpoint","{checkpoint}","--frames","{frame}","--logs","{run_dir}/logs","--trace-cpu","B","--trace-pc","04D8:0540","--native-cpu-b-replace","--native-cpu-b-library","C:/Projects/ChaseHQ/generated/cpu-b-candidate.dll"],"trace":"{run_dir}/logs/trace.log"}
}
```

Example is **illustrative only**. Verify that both traces represent compatible semantic state before treating differences as defects; bypassed original instruction hooks might deliberately change trace content. A real original/native/rollback equivalence and the generated library path must be established independently. One-off command: `.\chqdev.bat native minimize --plan C:\Projects\Evidence\controlled-plan.json --out C:\Projects\Evidence\RC6332-Divergence-Run001` (the output directory must not already exist). Synthetic fixture tests do not authorize any generated native function.

## Genuine predecessor evidence and outstanding blockers

Genuine `CHQ6331.zip` Windows sample: CPU-B original ROM `0x04A8` performs word writes overlapping `0x100802` seven times in each natural/bypass/nonterminal/terminal/overflow case; 35 writes `0x80→0x80`, zero bit 7 transitions and no sampled CPU-A writers. BTST at 0x050E keeps Z clear, BNE at 0x0516 takes 0x052C on all 35 samples. Only natural health case is unmodified; four are controlled CPU-A entry-state cases. RC6.3.32 outputs `cpu-b-rc6332-boundary-confidence.json`, declaring original-ROM authoring provenance, writer candidate PC and known unknowns. It **does not** invent the pre-checkpoint origin, complete Ghidra boundaries, alternative 0x0518, full-function C++ generation/verification, real hardware timing or multi-native scheduling. `nativePromotionAllowed=false`.

## Acceptance and handoff

Local checks: full Go tests (including actual child-process fixture + mutation tests), Linux native CTest 15/15, portable preflight, reproducible source package, independent per-file hash validation; bundled Windows amd64/arm64 tools. Real Windows tests still required. From extracted source root:

`New-Item -ItemType Directory -Force 'C:\Projects\Evidence' | Out-Null; .\chqdev.bat build debug; if ($LASTEXITCODE -eq 0) { .\chqdev.bat native reconstruct --fresh --live-health --cpu-b-live --out C:\Projects\Evidence\CHQ6332 }`

Require native reconstruction pass, new `M2-RC6.3.32-boundary-confidence` and **SYNTHETIC** `M2-RC6.3.32-executable-replay-reducer-smoke` gates, one generated `CHQ6332.zip`. Then `.\chqdev.bat qualify --fresh` must pass 18/18 Windows CTest, focused Run001 and Full Regression Run002, 1,153 commands, deterministic package hash, Workbench runtime and promotion. Preserve RC6.3.31 immutable. Workbench Next Steps/roadmap/handover and versioned regression have been updated; no whole-function promotion.

Release guard: source files maintain Windows ASCII scripts, LF and CRLF compatibility and immutable SHA-256 archive provenance.
