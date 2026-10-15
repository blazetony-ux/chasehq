# 0.67.0.0-RC6.2.4 — bounded M1–M7 technical acceptance

**Release status:** SOURCE CANDIDATE, Windows fresh qualification and real user-owned ROM-backed acceptance PENDING. **Qualified parent:** RC6.2.1 SHA-256 `c8422141d9d079ce522f0b1dc12a95527c5d80e9055b4c874fb356493b46174b`. **Original RC6.2.2 development candidate:** archival, not qualified.

## Supported acceptance scope and honest boundary

RC6.2.4 implements real opt-in replacement of **one authentic CPU-A instruction**: `A:0x00820C`, `MOVE.W (A0),(-0x7D00,A5)`, normal-mode steering conversion. Ghidra Run032 independently located it within **unreviewed** tentative function `0x81C4–0x8211`. This release **does not** reconstruct that full function, discover arbitrary P-code routines, or reconstruct CPU-B. Those remain explicit RC6.3 goals. Instruction-level proof must never be called proof of complete function replacement. Native replacement is disabled by default.

On Windows after a fresh source extraction with CMake/MSVC and the existing shared AdditionalFiles layout, run these separately in the same installation:

`.\chqdev.bat qualify --fresh`

`.\chqdev.bat native accept --fresh`

The first validates the exact Windows package, full regression, runtime/Workbench/mobile and release metadata. The second performs the ROM-backed **instruction-scoped** M1–M7 proof unattended, writes `acceptance.json` and a hashed sibling evidence ZIP regardless of PASS or FAIL. It returns nonzero when mandatory proof is missing. **No Python installation, manual ROM census or repeat Ghidra execution required.** Dependencies remain external; dynamic discovery of `C:/Projects/AdditionalFiles/roms` and `C:/Projects/AdditionalFiles/SDL` is retained.

## What each milestone independently requires

| Stage | Mandatory acceptance |
|---|---|
| M1 | Real Ghidra run manifest and signed ROM SHA, verified 68000 instruction/opcode boundary; tentative enclosing function explicitly unreviewed |
| M2 | Fresh original ROM CPU-A/B census, actual steering PC hits, workload with varied signed steering, exact checkpoint identity |
| M3 | Contract derived from real static and ROM/runtime observations, guarded registers/flags/ROM/memory/address mode; unreviewed alternate function branches excluded from scope |
| M4 | Native C++ source generated from current authentic ROM table, independently compiled as executable AND shared library, tested against independent 193-value historical ROM oracle |
| M5 | Actually load generated shared candidate into emulator; replace original instruction in real CPU-A execution, compare full recorded A/D/SR/PC/cycles at return boundary, observable original/new ordered bus events and final mapped CPU-A/CPU-B/memory/video state |
| M6 | Inject wrong native output; prove on-live-state rollback and subsequent original ROM execution, original/mutated contract and ROM evidence rejections; guard original execution by default |
| M7 | Single unattended evidence run with full stage gating, evidence ZIP and SHA manifest, exit nonzero unless in-scope proof passes |

**Decision semantics:** `SIGNED_OFF` attests ONLY to the scoped native **instruction** candidate, not the enclosing Ghidra function or full general-purpose autonomous reconstruction. A successful Windows run is necessary but not sufficient to sign off a broader M1–M7 whole-function objective; this scope distinction must be explicitly approved before moving to RC6.3. `NOT_SIGNED_OFF` is mandatory if any in-scope equality/rollback/negative gate fails.

**Evidence provenance and limitations:** The immutable original verified Ghidra ZIP `research/acceptance/ghidra-steering-RC621-verified.zip` must be packaged. No original ROM bytes distributed. Reference `docs/reference/steering_reference_193.csv` is historical independent ROM evidence. Full-state comparison hashes mutable mapped regions and public CPU registers; does not independently timestamp unrelated external physical bus devices. CPU flags/PC captured at instruction return boundary; guarded execution applies only for normal steering range (-96..96), gate zero. Generated C++ is NOT a general decompiler/P-code generator.

**Release blockers:** Final Windows native qualification has not been run in this environment. The inherited Go `TestFullRegressionConsolidatedCommandBudget` structural test currently reports 1,176 scripted commands versus its historic 850 budget, with all unique regression assertions retained. This failure is not waived or silently removed; fix it under a subsequent version if necessary. Cross-platform CPU-only CTests do not substitute for Windows proof.

**Release guard:** Authoritative documentation and Windows launcher scripts must preserve ASCII, LF and CRLF compatibility on Windows PowerShell 5.1.

**RC6.2.4 corrective note:** resolves the Windows SDK/Musashi `FLOAT128` and `min/max` macro compile collision observed in RC6.2.3. This is not new M1–M7 functionality, and no additional acceptance milestone is claimed from the correction itself.
