# Documentation audit - v0.66.9.2

Current release: **v0.66.9.2**, promotion provenance Windows-qualified v0.66.9.2-RC4 SHA-256 `45f4538841a438e951d0a88a09a7a428eca17452675ada5be9619884c67b7d8d`.

## Scope

This audit closes the v0.66.9.2 release-documentation cycle. RC4 passed Windows `chqdev qualify --fresh` with 7/7 CTest, runtime readiness, focused regression, Full Regression and promotable PASS. Post-run inspection confirmed the RC4 provenance repair: focused and Full Regression bundles retained complete authored source and authoritative command counts. The final promotion changes release identity/current documentation and adds the final focused regression; it does not intentionally change emulation/gameplay behaviour.

## Synchronized current surfaces

- `src/version.h` and `chqdev.json` identify v0.66.9.2; direct promotion provenance points to RC4 SHA `45f4538841a438e951d0a88a09a7a428eca17452675ada5be9619884c67b7d8d`.
- `README.md`, `QUICK_START.txt`, `VALIDATION.md`, `AGENTS.md`, `BUILD_LOCAL_SETUP.md`, `docs/README.md`, `docs/PROJECT_STATE.md`, `docs/ROADMAP.md`, `docs/HANDOVER.md`, `docs/CHATGPT_HANDOVER_PROMPT.md`, `docs/RELEASE_PROCESS.md`, `docs/RELEASE_GOVERNANCE.md`, `docs/CHQDEV.md`, API/script references and current-platform documentation identify the final release.
- Machine-readable handbook/knowledge/schema/feature/semantic-object build identities identify v0.66.9.2; generated handbook HTML is regenerated from those sources.
- `research/scripts/regression/regression-v06692.chqscript` is the final focused release regression and is permanently included in Full Regression/catalog.
- Historical RC4 notes/regression/audit remain provenance and are not rewritten as if the final promotion produced the original fixes.

## Permanent release rules

Fresh Windows qualification remains exactly `.\chqdev.bat qualify --fresh`; zero tolerated CTest failures, deterministic exact-archive verification, runtime readiness, focused regression, Full Regression and promotable PASS are mandatory. PowerShell/BAT release tooling remains compatibility/fallback only. Current release text contracts remain LF/CRLF/CR invariant.

## Next development phase

After exact final archive qualification, move immediately to independent whole-frame sprite 4bpp shadow verification. Do not reopen Stage-1 track-byte probing without contradictory evidence.
