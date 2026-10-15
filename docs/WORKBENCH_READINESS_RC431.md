# RC4.3.1 Workbench readiness/history hotfix

Status: **unqualified development candidate** until the exact final Windows archive passes `chqdev.bat qualify --fresh`. Baseline: RC4.3 candidate, which passed Windows build, 13/13 CTests, deterministic packaging and initial frontend bootstrap, but FAILED runtime-restart because readiness requests to port 37680 timed out.

## Evidence and cause

The 2026-10-10 RC4.3 qualification log showed `/api/v1/status` stalling for 10 seconds and four `/api/health` retries stalling for 10 seconds each after startup. A later support bundle showed the listener alive and individual status/frame requests working; expensive `/api/v1/script/runs` calls took 23-29 seconds when examining old evidence. `Start-ChaseHQWeb.ps1` uses a serial PowerShell HttpListener backend. The desktop browser originally awaited `refreshAuthoritativeRunHistory()` before starting dashboard refresh. Mobile console similarly requested history during startup. Listing runs scanned sibling releases and parsed *every run's bundle-manifest.json*, including enormous bundles, for artifact totals. This allows history to monopolize the request handler.

The direct debugger at port 37600 speaks the native text protocol, **not HTTP**. `curl http://127.0.0.1:37600/api/v1/status` returning HTTP/0.9 is a client protocol mismatch, not evidence of debugger failure. Use `chqctl` for the native port and HTTP only for Workbench port 37680.

## Implementation

- Desktop boot and mobile boot no longer request run history. Desktop loads it when the **Script Console tab** is opened; the mobile console loads it when **Refresh** is selected or after a run. Startup readiness, status and frames never wait for an automatic cross-release history request.
- Script-history listing uses run-metadata.json only, not every large bundle-manifest.json. New finalized runs persist compact `artifactCount` and `artifactTypes` in run metadata before creating the authoritative manifest and its hashes.
- Current-session history queries use the current release's evidence root and filter sessions *before* enumerating run directories. All-sessions history remains available on demand; older runs lacking compact artifact counts report `not indexed`, never an invented zero. Evidence bundles are not altered.
- A permanent `tools/chqdev/web_history_startup_test.go` tests startup isolation, on-demand history access, absence of manifest parsing in listing, session filtering and summary-before-hash ordering.

## Gates and limitations

- Local Go tests, static browser JS parsing, preflight, deterministic packaging, archive verification and Windows `chqdev` binary presence can run on Linux. **This is not a substitute for Windows Workbench qualification**.
- Mandatory Windows reproduction: extract this candidate beside previously configured releases; run `./chqdev.bat qualify --fresh` (PowerShell: `.\chqdev.bat qualify --fresh`). Verify runtime readiness stays responsive while a substantial legacy history corpus is present. Then verify historical browsing from Script Console and Mobile Console works on demand and evidence bundles remain accessible.
- All-sessions manual browsing still uses the serial backend; a larger asynchronous on-disk history index remains appropriate future work. Do not claim fully non-blocking history under arbitrary manual requests until that architecture is implemented and tested.
- The pre-existing Full Regression structural command-budget gate remains failing (greater than 850); **do not** raise it or delete unique regression assertions to conceal it.
- Never promote until exact-archive Windows qualification, including focused and Full Regression, passes.
