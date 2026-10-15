# RC4.5 - REA static analysis, Workbench Research API, and checkpoint census repair

**Current source candidate:** `v0.67.0.0-RC4.5`, derived from the **promoted, Windows-qualified, immutable RC4.4** archive SHA-256 `8a8b715646ee07619891ea137425bd24272d5df2b84c60170105a164ec3d8912`. RC4.5 is NOT Windows-qualified and NOT promoted. The original RC4.4 ZIP remains untouched. The next gate is `chqdev.bat qualify --fresh` on the *exact fresh-extracted RC4.5 ZIP*, with zero tolerated CTest failures, focused + Full Regression, runtime and mobile readiness, deterministic package verification, and promotable PASS.

## Implemented scope

1. **Workbench Native Lab -> Static Analysis / REA**: import an operator-reviewed `chq-rea-static-export-v1` JSON function map and matching real `rom-execution-pc-counts.tsv` using browser file inputs; crosswalk locally through the bundled chqdev adapter; browse reports and function-specific ROM execution hits. UI is opt-in/on-demand, with no startup enumeration, no ROM upload to external services, and no direct dependency on REA/Ghidra.
2. **Script/API parity**: `api rea.reports`, `api rea.crosswalk name=...`, `api rea.functions report=...` and `api rea.function.inspect report=... function=...`. Staged inputs are `research/rea/imports/NAME.static.json` and `NAME.census.tsv`, where `NAME` is an ASCII basename. Output is `evidence/rea-crosswalks/NAME.json`, additionally copied to the active script-run's `artifacts/` folder and therefore its automatic evidence ZIP. Local HTTP endpoints `/api/v1/rea/import`, `/api/v1/rea/crosswalk`, `/api/v1/rea/reports`, `/api/v1/rea/functions` share the existing authenticated listener. The Research API action schema is updated.
3. **Crosswalk inspection**: each function carries sorted observed PC addresses and their counts, alongside total hits, distinct PCs, CPU, ROM offsets, supplied ROM SHA-256 and static confidence. A PC falling inside a declared function is *not* proof that the function's actual boundary or semantic name is correct. Input limits: 4096 functions, 1 million TSV rows, 2 MiB static JSON and 8 MiB census upload.
4. **Standalone checkpoint-aware census**: `ChaseHQRuntime.exe --load-checkpoint PATH --rom-census --frames END` now actually loads the checkpoint after CPU initialisation, starts at its restored frame, and interprets END as the absolute exclusive ending frame. Failure to load a checkpoint fails the process rather than silently running from reset. A `run_manifest.json` records checkpoint requested/verified, start/end frames, scheduled interval, ROM census and projection shadow flags; `run_phase.txt` and console `[CHECKPOINT LOAD]` record the restore. Census counting begins from a cleared collector and uses the restored CPU state.
5. **ROM-free tests and qualification**: targeted Go tests exercise exact observed-PC intersection and invalid raw ROM mapping; CPU-only CMake/CTest tests remain. The existing Go Full Regression *structural command-budget* gate is still FAIL (1176 expanded vs required fewer than 850) and is neither relaxed nor concealed. Successful Windows runtime Full Regression does not resolve this separate Go test.

## Explicit non-goals and limits

- No direct Windows `morluto/rea` call, automated REA provider install, Ghidra headless importer for raw 68000, inferred function ownership, external network access or forced static-confidence promotion. The optional adapter accepts reviewed source; ROM digest *format* is checked, but actual ROM-file byte hashing and mapping verification remain an operator duty.
- No embedded per-instruction census collector added to the live SDL Workbench yet. The new `rea.*` API compares **existing** staged census data to reviewed static exports while preserving the current live SDL/emulator session. A checkpoint-aware offline CPU-only runtime generates per-window census evidence with explicit frame provenance.
- No native 68000 replacement and no completed fixed-point geometry parity. Runs 018-029 remain the authoritative projection research, with staging latency verified for the specific post-final-hit map19 replay only.
- Linux CMake/CTest or static JavaScript parse cannot substitute for Windows PowerShell/SDL/API regression and clean-extraction qualification. Never promote RC4.5 without exact-archive `promotable=PASS`.

## Workbench usage

Open **Native Lab -> Static Analysis / REA**. Choose an ASCII case name (for example `contact-5587`), select your **reviewed** static function JSON file and a ROM census TSV captured from the same ROM revision; click **Import and crosswalk**, then select a function to inspect. No emulator restart is needed for this offline crosswalk.

## Research Script usage

```
# name: REA imported routine inspection
# category: native-reconstruction
# purpose: Read reviewed static-vs-runtime evidence from an operator-staged ROM census.
try
  api rea.reports
  api rea.crosswalk name=contact-5587
  api rea.functions report=contact-5587
  api rea.function.inspect report=contact-5587 function=projection_clip_candidate
finally
  api memory.trace.stop
  api memory.trace.clear
  api checkpoint.load file=stage1-gameplay-2064.chqstate run=false
end
```

The `rea.crosswalk` action requires `research/rea/imports/contact-5587.static.json` and `contact-5587.census.tsv`, prepared by the UI import or operator. The function name must exactly match the static export; this snippet is illustrative and will fail if not present.

## Headless genuine gameplay census, one-line PowerShell example

From the extracted RC4.5 directory with user-owned CHQPACK configured:

```
.\out\build\x64-Debug\Debug\ChaseHQRuntime.exe --content-pack "C:\Projects\AdditionalFiles\content\chasehq-world.chqpack" --load-checkpoint .\checkpoints\stage1-target-immediate-pre-contact-5587.chqstate --rom-census --native-projection-shadow --irq4 --frames 5603 --logs .\evidence\rea-contact-5587
```

Verify that the console states `[CHECKPOINT LOAD] frame=5587` and `run_manifest.json` states `startFrame=5587`, `endFrameExclusive=5603`, `framesScheduled=16` before attributing the ROM census to this checkpoint. This command runs a *separate* CPU-only executable and does not restart the SDL Workbench.

## Future work / acceptance criteria

- **P1: live Workbench ROM census and projection shadow** with CPU-A/B capture, start/stop/reset/snapshot/export from the same emulator and automatic evidence export; no new emulator startup.
- **P1: raw 68000 static Ghidra/REA provider** with explicit CPU ROM byte layout, independently checked hashes, relocation/base mapping, function boundaries and cross references. No inference is authoritative until validated.
- **P1: linked function dossiers**: static disassembly, callers, live PC traces, previous run evidence and native-maturity ledger. Initial dossiers: CPU-A `0xA112`, `0x434EE`, `0x43828`.
- Preserve reproducibility on Windows, mobile-console parity and 1:1 API schema/allow-list matching. All errors fail closed.
