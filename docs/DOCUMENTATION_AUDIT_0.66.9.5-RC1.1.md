> **AUTHORITATIVE CURRENT — v0.67.0.0-RC6.3.11 CPU-B IRQ-OWNERSHIP CORRECTIVE SOURCE CANDIDATE (2026-10-14).** RC6.3.10 actual Windows `native reconstruct --cpu-b-live` failed only at the CPU-B original-ROM baseline because 12 interrupt-vector/stack bus effects (four genuine IRQ entry groups) were wrongly tagged as BEQ.B data effects by stale `trace_pc`; its CPU-A health/copy, shared dispatcher and real CPU-B Ghidra/compiled ABI gates all PASS. RC6.3.11 uses Musashi interrupt ACK to assign each IRQ vector/stack transaction to a dedicated ordered IRQ stream, preserves strict zero-branch-data and original/native/fallback CPU/cycle/bus/machine parity, and adds real-ACK synthetic and missing/mutated IRQ evidence tests. **RC6.3.11 authentic Windows native CPU-B equivalence is PENDING**, as are complete-function/autonomous promotion and inherited Go Full Regression budget 1176>850. See `docs/RC6.3.11_IRQ_OWNERSHIP_CORRECTIVE.md`. Earlier status blocks remain historical.

> Current source candidate: **v0.66.9.5-RC2**. See `docs/DOCUMENTATION_AUDIT_0.66.9.5-RC2.md` for Runs003-012 evidence and RC2 changes.

# Documentation audit - v0.66.9.5-RC1.1

Status: RELEASE CANDIDATE. Source baseline is the exact supplied final v0.66.9.4 archive SHA-256 `a50b3c571c845b16d3a3f9250144c345d25025bb516458f9f2dedf10885e9a40`.

## RC1 evidence basis

The candidate implements issues isolated by v0.66.9.4 research Runs005-010 rather than speculative rewrites:

- semantic sprite asset replacement worked in the live SDL renderer but was omitted by structured `frame.snapshot` sprite reconstruction/export;
- asset-visible telemetry could remain stale after clear/backend transitions;
- Run009 proved `player_car.body` was missing map509/palette65;
- Run010 proved `game.speed.freeze` installed a 16-bit patch on the actual 32-bit fixed-point field at CPU-A `0x10041C`;
- Runs009-010 proved the phase-only police beacon intervention changed `0x101854` state without changing the presented beacon;
- Run010 proved target-local physical contact PC `0xA13C -> 0x10A084` was absent from structured collision events;
- Run008 showed the confirmed target-health write could rank hundreds of timeline candidates down without semantic labeling.

## Implemented RC1 corrections

- Structured snapshots render sprites through the selected Original/Shadow/Native path and semantic asset resolver; per-sprite exports carry source/effective map/palette and replacement metadata.
- New `frame.snapshot.parity` captures the actual SDL framebuffer and structured snapshot at one paused instant; `require=true` fails closed on any mismatch.
- New `sprite.asset-resolve` reports live sprite-to-semantic-family matching and active replacement state.
- `player_car.body` includes proven map509.
- Asset-visible telemetry resets on rule clear/set and backend transition.
- `game.speed.set` and `game.speed.freeze` use 32-bit `0x10041C`, encoding user speed as `value << 16`.
- Beacon Force ON/OFF couples visible phase `0x101854` with deployment request `0x108082`; Windows presentation proof is still required.
- Collision events add observational class `target_contact` for PC A13C target-local lateral response without broadening the existing suppression scope.
- Forensic Timeline semantic ranking recognizes confirmed target-health/defeat/interaction/target-lateral/internal-speed addresses.

## New API actions

- `api frame.snapshot.parity name=NAME require=true|false`
- `api sprite.asset-resolve`

## Governance

`chqdev` remains the de facto and authoritative build/release process. Promotion requires exact-archive Windows `chqdev qualify --fresh`, zero tolerated CTest failures (any CTest failure is fatal), deterministic exact-archive package verification, focused regression PASS, Full Regression PASS and promotable PASS. PowerShell/BAT/shell launchers remain compatibility surfaces. Release-owned text must remain valid under LF and CRLF line endings.

## RC1.1 Workbench bridge closure

- Native Reconstruction Lab direct UI commands use the native `backend sprites ...` family.
- RC1 exposed those controls while the safe raw-command bridge omitted `backend` from `$allowedFirst`, so all four backend buttons were rejected before reaching the native API.
- RC1.1 adds `backend` to the safe first-token allow-list and static parity coverage that enumerates literal Workbench `cmd(...)` verbs against that allow-list.
