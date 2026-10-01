# v0.66.2 validation

Container validation performed before packaging:
- JSON manifests/registry parse.
- C++ CPU-only configure/build/tests.
- JavaScript syntax extraction/check where available.
- Version consistency scan.
- Steering regression metadata/static checks.
- Package ROM exclusion and ZIP integrity.

Windows acceptance:
1. `.\Prepare-Project.bat --check`
2. `.\Build-Debug.bat`
3. `.\Start-ChaseHQ.ps1 -Restart`
4. Run `regression/regression-v0662-steering-controls.chqscript`
5. Run `regression/full-regression.chqscript`
6. Load `stage1-gameplay-2064.chqstate`; hold Up and use Left/Right in SDL.

Observed in container validation: CPU-only CMake build succeeded and `cpu_bus_rom_tests` passed 1/1. Inline Workbench JavaScript passed `node --check`. JSON/static steering checks passed.
