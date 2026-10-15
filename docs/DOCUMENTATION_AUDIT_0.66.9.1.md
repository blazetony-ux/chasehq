# Documentation audit - ChaseHQ-Native v0.66.9.1

## Scope

This is the release-governance consolidation audit performed before packaging v0.66.9.1. The package contains 489 textual `.md` / `.txt` / `.html` / `.json` surfaces outside generated/vendor/runtime output directories. Historical release notes, validation records and evidence are preserved as historical records rather than rewritten to pretend they were created under the current process.

The audit explicitly covered:

- root operational docs (`README`, `VALIDATION`, `AGENTS`, `QUICK_START`, local staging);
- all authoritative docs listed by `chqdev.json`;
- `docs/platform/current` operational/launcher guidance;
- release process, governance, roadmap, state and handover;
- API/script reference current identity headers;
- `research/knowledge/handbook.json` and generated handbook HTML;
- `research/knowledge/documentation.json` and current machine-readable build identities;
- feature/schema build identities;
- focused/Full Regression/catalog release wiring.

## Drift found and corrected

The deep pass found real stale operational guidance despite RC3.4.9 itself being fully qualified:

1. `QUICK_START.txt` still presented direct `Start-ChaseHQ.ps1` as normal.
2. `BUILD_LOCAL_SETUP.md` still directed the developer handoff toward `Build-Debug.bat`.
3. the generated handbook still described `Build-Debug.bat` / direct `Start-ChaseHQ.ps1` as canonical and still described the old tolerated Windows CTest exception;
4. `docs/CHQDEV.md` still contained an obsolete sentence allowing the historical `cpu_bus_rom_tests` failure and an old focused-regression path;
5. older launcher notes under `docs/platform/current` still contained operational commands that could be mistaken for current guidance;
6. API/script reference headers still named RC3.4.8 as promoted baseline;
7. machine-readable documentation still contained a historical release-process article whose “canonical” values had not been superseded by chqdev.

All of those current-facing conflicts were corrected. Original old launcher notes were preserved under `docs/platform/history`; current paths now contain explicit historical redirects.

## Permanent enforcement added

`chqdev` v0.5.0 adds the `release-process-contract` preflight gate. Every configured release-process document must contain the current permanent contract:

- chqdev authority;
- `chqdev qualify --fresh` as canonical fresh Windows qualification;
- zero tolerated CTest failures / any CTest failure fatal;
- deterministic archive identity;
- Full Regression requirement;
- PowerShell/BAT or shell tooling explicitly compatibility-only.

The gate also rejects known obsolete canonical claims such as “canonical Windows build path is Build-Debug.bat”. A dedicated Go unit test proves both valid-contract acceptance and legacy-claim rejection. `tests/workbench_v06691_tests.js` independently checks the human and machine-readable documentation surfaces.

## Current authoritative rule

`chqdev` is the de facto and authoritative build/release process. A fresh Windows candidate is promoted with:

```powershell
.\chqdev.bat qualify --fresh
```

There are zero tolerated CTest failures. Package bytes are deterministic. Promotion belongs to one exact immutable archive SHA and requires runtime readiness, focused regression, Full Regression and final `promotable` PASS. PowerShell/BAT release tooling is compatibility/fallback only.

## Historical documentation policy

Historical/versioned files remain valid evidence of what was true at that time. They are not automatically rewritten to current commands, because doing so would falsify historical evidence. Current operational docs and redirects explicitly state that historical commands do not override the current release-governance contract.

## Release identity

v0.66.9.1 is derived from promoted v0.66.9.0-RC3.4.9 SHA-256 `30ec017edebb9d221dfed16d090ffd6678b645cff6df8c5445a9ef298efe0ebf`.

## Audit prove-off

The final current-document set passes chqdev's version, line-ending, authority and release-process contract gates. The full carried-forward Workbench/static suite is 16/16 PASS, including the new v0.66.9.1 governance test. Go tests PASS and the portable `qualify --fresh` path passes with Linux CTest 4/4 including `cpu_bus_rom_tests`.
