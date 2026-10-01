# ChaseHQ-Native v0.66.9.0-RC2.4

RC2.4 fixes a real TC0100SCN renderer error exposed by the RC2.3 RAM-layout and controlled layer-offset experiments.

## TC0100SCN Y-scroll fix

The renderer previously converted control Y to a negative `scrolly`, then subtracted that value again while calculating source Y. That inverted the intended sign. With Chase H.Q. `CTRL3/CTRL4 = 0x01E0`, this displaced the background planes by 64 source lines modulo the 512-line tilemap and led to long-lived presentation-offset compensation.

RC2.4 uses one shared `tc0100scn_source_y()` helper for BG0, BG1 and TEXT and for geometry tracing. The historical BG0 `0:-4` and BG1 `0:-20` defaults are removed; both now default to `0:0`. `--layer-offset` remains available for explicit forensic experiments.

The RC2.3 A/B evidence is especially strong: the `-64` sign-proxy moved the dark and pale building planes from the lower road/foreground region to the skyline/horizon. At the captured `0x01E0` control value, that proxy is exactly equivalent to the corrected renderer with neutral offsets.

## Regression

A native geometry regression now asserts the wrapped source-Y result for the proven `0x01E0` case. CPU-only CMake build and `cpu_bus_rom_tests` pass locally. Windows/SDL visual and full-regression proof remain the release gate.

## Compatibility

No API, schema or `.chqscript` grammar changes. Evidence metadata continues to report presentation offsets; their defaults are now neutral.

Focused Windows visual gate: `./Validate-TC0100SCN-Y.ps1` captures canonical frame 2065 with neutral defaults and requires byte-exact equality with the packaged RC2.3 both-sign-proxy reference.
