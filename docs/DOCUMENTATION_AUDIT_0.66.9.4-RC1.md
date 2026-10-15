# Documentation audit - v0.66.9.4-RC1

## Candidate identity

`v0.66.9.4-RC1` is a development candidate built from immutable promoted `v0.66.9.3` SHA-256 `6da88889dc8214c6058644e44e4d29f6a2117c9f0e92d99c8d906c280fc3462e`. It is **not promoted** until the exact packaged archive passes the complete Windows qualification gate.

## Implemented scope

- Experimental police-beacon override with three modes: **Authentic**, **Force ON**, **Force OFF**. Force ON holds authoritative CPU-A `0x101854` at deployed phase `6`; Force OFF holds phase `0`; Authentic removes the intervention. Checkpoint load resets the beacon override to Authentic. The authentic palette-flash path remains live while Force ON is active.
- Presentation-only semantic HUD layout controls for **High Score, Time, Distance, Score, Speed, Turbo markers and Stage/Credit**. Each group can be shown/hidden and moved by signed pixel X/Y offsets without mutating emulated TC0100SCN tile RAM. Presets: Authentic, Minimal (Time + Distance + Turbo), and No HUD.
- HUD semantic cell map is grounded in canonical gameplay checkpoints: High Score `ty2 tx3..12` + `ty3 tx11..17`; Time `ty2 tx20..23` + `ty3..5 tx20..23`; Distance `ty2 tx26..38`; Score `ty4 tx3..7` + `ty5 tx11..17`; Speed `ty4 tx26..30` + `ty4..5 tx36..41`; Turbo `ty29..30 tx3..8`; Stage/Credit `ty29 tx34..40`.
- Frame-snapshot reconstruction metadata records the active semantic HUD presentation transform so captured evidence remains self-describing.
- The independent 4bpp decoder and independent placed sprite-source layer were already SHADOW VERIFIED in v0.66.9.3 across the eight canonical checkpoints. RC1 carries that implementation forward; it does not duplicate or weaken it.
- Native command, Script API, Workbench Experimental/Game Lab, machine-readable schema, tests and focused regression are updated together.

## Validation state

Portable Linux validation may prove compile/unit/static contracts, but it cannot substitute for Windows/SDL/Workbench qualification. The final RC1 archive must still be qualified on Windows.

`chqdev` is the **de facto and authoritative build/release process**. Fresh Windows exact-archive qualification is `chqdev qualify --fresh` (or `./chqdev.bat qualify --fresh` from PowerShell/cmd). There are **zero tolerated CTest failures; any CTest failure is fatal**. Promotion requires a **deterministic archive**, exact archive verification, runtime readiness, focused regression PASS, **Full Regression** PASS and `promotable` PASS.

PowerShell/BAT release orchestration is **compatibility/fallback only**; it is not the canonical release authority. Release-text parsing and guards must remain valid for LF, CRLF and CR newline forms.

## Evidence boundaries

The beacon and HUD controls are explicit research/presentation interventions. Authentic mode remains the default. No forced state is to be described as natural gameplay evidence. Audio remains foundation/command-interface instrumentation only; this RC does not implement sound CPU/chips/mixer/PCM runtime.
