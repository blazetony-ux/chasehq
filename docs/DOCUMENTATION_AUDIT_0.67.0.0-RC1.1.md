# Documentation audit — v0.67.0.0-RC1.1

## Scope

v0.67.0.0-RC1.1 is a narrow qualification corrective over exact v0.67.0.0-RC1. It does not reopen the tooling freeze or add a new emulator subsystem.

## Windows evidence carried forward

RC1 focused Run001 passed: Native sprites were active and the independent TC0100SCN text/HUD Shadow renderer accumulated 171 compared frames with zero mismatch frames. RC1 Full Regression Run002 failed at the historical sprite-overlap assertion. The full, body-solo and shadow-solo PNGs were byte-identical to the prior qualified RC3 outputs and all final owner IDs were identical; only `sprite-ownership.csv` overlap density was wrong. Native coverage had collapsed every occupied pixel to 1 instead of preserving Original counts 2/3/4 for occluded non-zero candidates.

## Corrective implementation

The independent native sprite layer now records a saturated 16-bit per-pixel coverage count before first-nontransparent occupancy rejects later candidates. The live Native renderer exports that count to the standard forensic coverage buffer. This restores the Original diagnostic contract without changing rendered pixels, winning owner, draw order or sprite semantics.

## Regression

`regression-v0670-rc11.chqscript` explicitly snapshots Stage-1 frame 2352 under the Native default, captures body/shadow solo references and executes `frame.snapshot.assert-overlap` for slots 82/81. The independent sprite-layer unit test also requires coverage=2 on an overlapping two-slot fixture.

## Native implementation direction

The broad tooling freeze remains in force. After fresh Windows qualification, development continues with TC0100SCN text/HUD native promotion rather than another general tooling pass.
