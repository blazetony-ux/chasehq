# Documentation audit - v0.66.9.2-RC4

Current candidate: **v0.66.9.2-RC4**, derived from promoted v0.66.9.1 SHA-256 `36c40833f4a077fd0545e64b52585ef2c6ef6ed414ae31832092f5150caf6a4`.

## Scope

RC4 is a narrow authoritative-run evidence-integrity repair over the functionally Windows-qualified RC3 candidate. No emulation/gameplay behaviour is intentionally changed.

## Contract checked

- `chqdev` v0.5.2 remains the de facto and authoritative build/release controller.
- Fresh Windows promotion remains exactly `.\chqdev.bat qualify --fresh`; zero tolerated CTest failures, deterministic exact-archive verification, runtime readiness, focused regression, Full Regression and promotable PASS are mandatory.
- Go-runner evidence keeps the complete authored top-level source in `script.chqscript`; `resolved-commands.txt` remains the expanded executed stream.
- `run-metadata.json.commands` is cumulative during execution and is finalized from the Go runner authoritative count. Desktop/mobile Recent Script Runs consume that persisted field.
- RC3 frontend-bootstrap readiness, track semantics, checkpoint context, trace freshness and release-root hygiene remain unchanged.
- Current release docs, machine-readable knowledge/schema build identities, script catalog and Full Regression include RC4.
- Historical RC3 material remains provenance and is not rewritten as if RC4 produced the original track findings.

## Windows proof still required

Qualify the exact frozen RC4 archive and inspect the focused and Full Regression bundles to confirm multi-line `script.chqscript` plus non-zero command counts before promotion.
