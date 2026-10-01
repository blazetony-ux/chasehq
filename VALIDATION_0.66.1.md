# v0.66.1 validation

Validated in the build environment:
- Embedded Workbench JavaScript syntax: PASS (`node --check`).
- Feature manifest / semantic object JSON: PASS.
- CPU-only CMake configure/build: PASS.
- `cpu_bus_rom_tests`: 1/1 PASS.
- Native collision command additions compile as part of `ChaseHQNative` source path in CPU-shared compilation units; full Windows SDL linking/runtime remains a Windows-machine validation step.

Recommended Windows smoke:
1. `Prepare-Project.bat --check`
2. `Build-Debug.bat`
3. `Start-ChaseHQ.ps1 -Restart`
4. Run `regression/regression-v0661-collision-api.chqscript`
5. Run `regression/regression-v0661-sprite-order.chqscript`
6. Run `regression/regression-v0661-manifest-version.chqscript`
7. Run `regression/full-regression.chqscript`
