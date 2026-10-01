# v0.66.4 validation

## Completed in packaging environment

- Source tree version/schema/document consistency checks.
- Research Script metadata/include checks.
- `palette.trace.*` API appears in the Workbench action registry, static schema, API documentation and same-build regression.
- CPU/runtime library builds and `runtime_tests` pass with `CHASEHQ_BUILD_SDL=OFF` in the packaging environment.
- Full regression script keeps the final COMPLETE marker last and includes the new v0.66.4 palette-trace regression.
- Workbench source carries the validated full-console-transcript dictionary-key fix.
- Run metadata now preserves the first request start time for a multi-command run; descriptive bundle download names are stored in metadata and returned to the UI.

## Required on Tony's Windows development machine

Run `Validate-Release.ps1`. It performs PowerShell parsing/StrictMode-sensitive static checks, Workbench validation, version/schema/docs consistency, regression wiring, path-length validation and the normal Debug build/tests unless `-SkipBuild` is explicitly supplied.

Then launch with `Start-ChaseHQ.ps1 -Restart`, run `Regression - v0.66.4 palette write trace`, then Full Regression. Confirm the new trace reports exact writer PCs for palette indices 1037 and 1053 around the Stage 1 brake-lamp transition.

Only package/promote a Windows-built candidate after `READY TO PACKAGE: YES` and live regression completion.
