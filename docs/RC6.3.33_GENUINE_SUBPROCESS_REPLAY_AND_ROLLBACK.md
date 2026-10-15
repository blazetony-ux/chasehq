# RC6.3.33 — Real original-ROM / Ghidra-generated native / rollback replay matrix

**Release `0.67.0.0-RC6.3.33` / chqdev 0.5.52 source candidate; Windows qualification PENDING.** Qualified RC6.3.32 source `3f72489f25b144cda5ca7d7416670b02e7f7fa02b70fc32811320943b75bccb6` (Windows qualifier-reported separate dist `cb4582d8abcac2f615de51a99065fb54f45d1f06cbed271135e146fe4f7e4c9e`).

## Scope

RC6.3.32 introduced `chqdev native minimize` with two real subprocesses and synthetic traces in its native reconstruction smoke. RC6.3.33 additionally runs **real original Taito Chase H.Q. ROM checkpoint executions and actual generated native C++ CPU-B two-instruction substitution** using `ChaseHQRuntime` with `--native-cpu-b-pair-library` and `--native-cpu-b-pair-replace`, and an independently forced original-ROM fallback using `--native-cpu-b-pair-fault-probe`. It intentionally retains all existing whole-function blockers: only the bounded two-opcode TST.W+BEQ pair at CPU-B 0x04D8 is eligible for this test, not its enclosing function.

### Genuine two-bound matrix

The source ROM, original `stage1-target-immediate-pre-damage-6333.chqstate`, generated Ghidra native DLL, executable and ROM directory are unchanged across the 3 modes x 2 absolute end-frames **6337** and **6340**. Each process starts from the same checkpoint, captures runtime console and 5 exact semantic streams: `native-cpu-b-pair-state.tsv` (registers/SR/cycles), `native-cpu-b-pair-bus.tsv` (mapped R16 data), `native-cpu-b-pair-irq.tsv` (ordered selected IRQ activity), `native-machine-endstate.json` (machine snapshot) and `bus.log` (raw bus). Requires exact original/native AND original/fallback file equality at both bounds, at least one real pair return and native commit per bound, and exactly one forced rejection with genuine original-ROM fallback. Frame 6340 original/native modes run a second time to confirm all signal hashes match; checkpoint, runtime EXE and generated DLL SHA256 must remain identical for the entire matrix. All paths under `rc6333-authentic-checkpoint-replay/`, plus `cpu-b-rc6333-genuine-checkpoint-replay.json`.

On any unequal signal the gate emits a SHA-pinned `first-divergence.json` witness and executes the original/native `rc6332Reduce` original-ROM checkpoint process replay from frame 6334 to the mismatch frame. It refuses to overwrite any prior evidence and keeps partial failure material, including subprocess logs. The automatic reduction applies to an actual mismatch; when genuine original/native streams agree, **no genuine divergence is fabricated**, and there is nothing to minimise. A byte mutated in a **copy** of the genuine CPU-B original state stream validates first-difference rejection and is saved as `CONTROLLED-TRACE-MUTATION-NOT-ROM.tsv`; it is not evidence of an emulator discrepancy, and never modifies the saved ROM run or checkpoint. No randomised search, input-event delta-debugger, full-function verification or exhaustive monotonicity claim.

### Predecessor and coverage limits

The real uploaded RC6.3.32 native evidence `CHQ6332.zip` contains original/native/rollback files for frame 6340. An optional Go integration test reads them, compares all five streams, validates native commits and one rollback, mutates a *copy* and proves the comparator detects that change. **This predecessor replay is genuine evidence content but is not new Windows RC6.3.33 executable execution.** Local synthetic comparator/unit tests use separate tags. 0x0518 is still unobserved; memory writer PC 0x04A8 preserved 0x80 over 35 sampled events, earlier producer unknown; static CFG, dynamic indirect edges and complete scheduler/IRQ correctness remain unknown. Reports explicitly declare WholeFunctionVerified=false, NativePromotionAllowed=false.

### Windows acceptance

From freshly extracted source root, after staging local ROM and SDL dependencies:

`New-Item -ItemType Directory -Force 'C:\Projects\Evidence' | Out-Null; .\chqdev.bat build debug; if ($LASTEXITCODE -eq 0) { .\chqdev.bat native reconstruct --fresh --live-health --cpu-b-live --out C:\Projects\Evidence\CHQ6333 }`

Require `M5-RC6.3.33-genuine-CPU-B-checkpoint-replay-and-rollback PASS` and `CHQDEV NATIVE RECONSTRUCT: PASS`; evidence auto ZIP at `C:\Projects\Evidence\CHQ6333.zip`. Only then `.\chqdev.bat qualify --fresh`: 18/18 Windows CTest, focused Run001 ten commands, Full Regression Run002 1,153 commands, deterministic package and Workbench/mobile readiness. Inspect actual `run-metadata.json`, ZIP SHA256/size CRC and new genuine matrix reports. Promotion of the **release tooling** does NOT promote a whole function to NATIVE; native dispatcher remains unchanged.

Release guard: ASCII Windows scripts and LF/CRLF line ending normalization are preserved and verified by preflight.
