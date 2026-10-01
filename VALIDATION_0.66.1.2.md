# v0.66.1.2 validation

Recommended Windows acceptance:

1. `Prepare-Project.bat --check`
2. `Build-Debug.bat`
3. `Start-ChaseHQ.ps1 -Restart`
4. Run `regression/regression-v06612-resilience-hotfix.chqscript`.
5. Run `regression/full-regression.chqscript`.
6. Confirm old v1 script history appears alongside v2 history without clearing localStorage.
7. Confirm routine 1-second polling is retained in `.chq/web-listener.log` but suppressed from the visible console unless slow/error/non-routine.
