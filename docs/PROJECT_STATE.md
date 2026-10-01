# Active candidate: v0.66.9.0-RC2.9

RC2.8 is the Windows/SDL-proven baseline. RC2.9 consolidates the subsequent evidence harvest and usability changes; it is **not yet Windows/SDL-proven**.

## Newly authoritative in RC2.9

- **Target health/damage:** CPU-A `0x1002AE` is causally confirmed as the Stage-1 special-target remaining-hit counter. Genuine damaging hit PC `0xA112` decrements it. Non-negative results take normal path `0xA124`; underflow `0000 -> FFFF` takes terminal PC `0xA118`, writes `0x1002D8=0x1400`, then enters the mapped defeat sequence.
- **Canonical target anchors:** `stage1-target-immediate-pre-damage-6333` is the preferred target-damage checkpoint; `stage1-target-first-damage-6334` is the matching first-damage state. Frames 5587/5588 are retained as physical-contact references.
- **One-hit Target Kill:** Experimental, default OFF, special Stage-1 target only. Native instruction-hook arming sets the counter to zero immediately before authentic PC `0xA112`; the real CPU decrement/flags/terminal branch execute normally. Script/API surface: `api game.target.one-hit enabled=true|false`; native debugger: `target one-hit on|off|status`; startup CLI: `--one-hit-target`.
- **Brake lamps:** processed brake `0x100303` and palette banks 64/65 pen 13 path are causally understood.
- **Track topology:** page `0x10A058`, selector `0x10A059`, lateral branch rule and selector 0-9 lifecycle are behavior-understood. Next value is a directed Stage-1 graph/minimap, not more low-level reproving.
- **Research UX:** Pause/Break toggles SDL pause/resume globally; Dashboard puts SDL controls beside the live frame at wide widths; `control.run-frames` accepts configurable timeout (default 900000 ms).

## Important current limitations

- Sprite 4bpp decoder remains **NATIVE-CANDIDATE / diagnostic parity**, not whole-scene SHADOW VERIFIED.
- Audio remains `FOUNDATION_ONLY`; authentic runtime audio needs new instrumentation.
- Script `get*()` query functions are preflight-evaluated/cached. A `getmaps()`/`getsprites()` iterable can therefore describe pre-run state after an in-script checkpoint load; use direct runtime API calls for state-sensitive inspection until explicit runtime-query syntax is added.

## RC2.9 release gate

1. Build the exact tree on Windows through `Build-Debug.bat` (local SDL/ROM staging comes from `build-local.json`).
2. Run `regression-v06690-rc29-target-health-tooling.chqscript`.
3. Prove one-hit mode from canonical frame 6333: authentic frame-6334 damage must produce `0x1002AE=FFFF` and the known defeat state.
4. Verify Pause/Break in the live SDL window and responsive Dashboard layout.
5. Run Full Regression to its COMPLETE marker and package the resulting evidence.

## Immediate research after release proof

Build the authoritative Stage-1 directed course graph/minimap from the already-proven selector tables and branch rule.

