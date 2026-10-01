# Validation — v0.66.9.0-RC2.4

## Completed in packaging environment

- [x] RC2.3 four-way Y-sign evidence inspected.
- [x] Both-sign proxy places both TC0100SCN building planes at the Stage 1 horizon instead of on the lower road/foreground.
- [x] Source arithmetic checked: with visible-top hardware Y 16 and CTRL Y `0x01E0`, corrected source Y is 40.
- [x] Shared source-Y helper added for BG0/BG1/TEXT and TC0100SCN trace.
- [x] Historical BG default presentation offsets removed.
- [x] CPU-only CMake configure/build passes.
- [x] `cpu_bus_rom_tests` passes 1/1.
- [x] JSON knowledge files parse.

## Windows/SDL release gate

- [ ] `Build-Debug.bat` passes.
- [ ] Canonical `stage1-gameplay-2064` frame 2065 with default offsets matches RC2.3 `both-sign-proxy`.
- [ ] Attract/title and at least one later Stage 1 checkpoint show no TC0100SCN vertical regression.
- [ ] Focused regression passes.
- [ ] Full Regression passes with terminal COMPLETE marker last.

RC2.4 must remain a candidate until those Windows/SDL checks pass.

Focused Windows visual gate: `./Validate-TC0100SCN-Y.ps1` captures canonical frame 2065 with neutral defaults and requires byte-exact equality with the packaged RC2.3 both-sign-proxy reference.
