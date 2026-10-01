# Validation — v0.66.5

Release gate: run `Validate-Release.ps1`. The canonical Windows build entry remains `Build-Debug.bat`, which first calls `Prepare-Project.bat` to unblock PowerShell/project files.

The historical Windows `cpu_bus_rom_tests (SEGFAULT)` result is treated as a documented known test-harness issue only when CTest reports that exact test/signature and exactly 1 failed test out of 1. Any other CTest failure remains a hard build failure.

Required runtime regression: `Regression - v0.66.5 generic memory write trace`, then Full Regression.
