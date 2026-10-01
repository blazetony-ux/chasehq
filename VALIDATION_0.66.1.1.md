# v0.66.1.1 validation

## First Windows acceptance

```powershell
.\Prepare-Project.bat --check
.\Build-Debug.bat
.\Start-ChaseHQ.ps1 -Restart
```

Expected launcher phases should include native PID/API readiness, Web server readiness, and `Browser frontend bootstrap: READY` before the final `READY` banner.

## v0.66.1 regressions
Run these from Script Console after startup:

- `regression/regression-v0661-collision-api.chqscript`
- `regression/regression-v0661-sprite-order.chqscript`
- `regression/regression-v0661-manifest-version.chqscript`
- `regression/regression-v0661-script-provenance.chqscript`
- `regression/full-regression.chqscript`

## Failure diagnostics

```powershell
.\Start-ChaseHQ.ps1 -Diagnostics
.\Start-ChaseHQ.ps1 -SupportBundle
```

The listener console also logs to `.chq\web-listener.log`. Native stdout/stderr are stored in the current evidence session.
