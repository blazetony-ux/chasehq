# ChaseHQ-Native handover — v0.66.9.0-RC2.9

Use the packaged RC2.9 source as the current candidate. RC2.8 is the last Windows/SDL-proven baseline; RC2.9 needs its own Windows proof.

## Do not reopen these settled questions

- Target remaining-hit counter = CPU-A `0x1002AE` (CONFIRMED). PC `0xA112` decrements genuine damage; `FFFF` is terminal.
- Normal hit path = `0xA124`; terminal underflow path = `0xA118` / `0x1002D8=0x1400`.
- Post-defeat chain: `0x1002D0` minimum delay -> `0x1002C8` readiness -> `0x1002CE` completion handoff -> results worker -> `0x100248` -> `0x1002E8`.
- Preferred damage anchor = `stage1-target-immediate-pre-damage-6333.chqstate`; matching hit = 6334.
- Physical contact-only reference pair = 5587/5588.
- Brake-lamp mechanism and Stage-1 topology selector/fork machinery are behavior-understood.

## RC2.9 features awaiting Windows proof

- Experimental `One-hit Target Kill` toggle and `api game.target.one-hit enabled=true|false`; authentic A112/A118 path is preserved.
- SDL Pause/Break global pause/resume shortcut plus paused title indication.
- Dashboard wide-screen 7/5 live-frame + controls layout, responsive collapse on smaller widths.
- `control.run-frames timeout=...`, default 900000 ms, with existing progress/stall checks.
- Gameplay Registry and Docs/Knowledge promoted to confirmed target-health semantics.

## Known tooling caveat

`get*()` script query functions are preflight-evaluated and cached. Do not use a preflight collection as if it were dynamically re-evaluated after `checkpoint.load` in the same script. Direct `api sprites.maps`, `api sprites.list`, etc. are runtime state. A future DSL update should add explicit runtime query evaluation and structured tuple/record iteration.

## Next actions

Build RC2.9 on Windows, run the focused RC2.9 regression, verify SDL Pause/Break hotkey + Dashboard, then Full Regression. After promotion, prioritize a directed Stage-1 course graph/minimap.

