# 0.67.0.0-RC6.2.2 — M1–M7 end-to-end acceptance contract

**Version:** 0.67.0.0-RC6.2.2 • **Current state:** DEVELOPMENT SOURCE CANDIDATE, Windows qualification pending. **Parent:** Windows-qualified RC6.2.1 SHA-256 `c8422141d9d079ce522f0b1dc12a95527c5d80e9055b4c874fb356493b46174b` remains immutable.

## Single Windows acceptance command (implemented)

After `chqdev.bat qualify --fresh` on the exact extracted candidate:

`./chqdev.bat native accept --fresh`

PowerShell form: `.\chqdev.bat native accept --fresh`. No Python required. Do not manually assemble a runtime census or repeat Ghidra Run032. The command preserves and hashes verified Ghidra Run032 CPU-A findings from `research/acceptance/ghidra-steering-RC621-verified.zip`; verifies against actual staged user-owned World ROMs; collects fresh standalone original-ROM `--rom-census`, read-only steering-shadow evidence and entry observations; generates and compiles separate C++17; evaluates historical independent ROM vectors; exercises defensive negative tests; writes `acceptance.json` and automatic sibling ZIP even when a gate fails. It does not mutate the SDL emulator or enable native substitution.

## Acceptance and non-negotiable thresholds

| Gate | Required, grounded proof |
|---|---|
| M1 | Real Ghidra job provenance, matching raw ROM SHA, requested CPU-A steering site `0x820C`, static instruction candidate and explicitly provisional function boundary; CPU-B/static completeness separate |
| M2 | Fresh ROM-backed CPU-A/B execution census; steering writer PC actually executes; authoritative observed hit counts, no fabricated global coverage denominator |
| M3 | Inputs/outputs, address-mode semantics, register/CPU flag effects, ordered memory/bus side effects, branches/callers/cycle dependencies proven with uncertainty attached; any missing dependency blocks sign-off |
| M4 | Real generated C++17 independently compiles, executes and matches 193 independently recorded steering reference vectors (not self-comparison alone); generalized decompiler/P-code codegen still separately required for broader autonomy |
| M5 | Original ROM versus independent native implementation compared under real execution; all relevant CPU registers, flags, memory writes, bus ordering and cycle/timing effects require equivalence, not only scalar output matches |
| M6 | Demonstrated rejection of wrong ROM, changed native output and unreviewed contract; reversible native substitution/rollback must be independently proven before replacement authorization |
| M7 | One unattended workflow collecting M1–M6 provenance and a deterministic, self-describing integrity-checkable evidence ZIP; follow strict fail-closed sign-off statuses |

**Full sign-off requires all mandatory proof.** Read-only steering matches or a synthetic fixture can never self-authorize substitution. Status must be `NOT_SIGNED_OFF` and process nonzero when unresolved dependencies remain. The acceptance run must differentiate milestone execution from complete technical sign-off. RC6.3 is deferred, not a substitute for this sign-off. A second genuinely different routine is needed to demonstrate general-purpose reconstruction beyond steering.

## Tooling and documentation authority

The AdditionalFiles shared-dependency discovery is included and regression-tested. `chqdev` is the de facto authoritative build/release tool; PowerShell/bat shell launchers are compatibility wrappers. Windows exact-archive `qualify --fresh` requires zero tolerated CTest failures, deterministic archive/manifest, Workbench/mobile parity and Full Regression PASS. Do not report Linux CPU-only validation as Windows qualification. Historical RC4–RC6 statuses in older docs are snapshots; refer to `RC6_STATUS.md` for current facts.

Release guard: LF and CRLF both required; new Windows launchers remain PowerShell/bat compatibility wrappers.
