# Validation — v0.66.9.0-RC2.1

RC2.1 is a build-gate metadata hotfix over RC2.

## Packaging checks performed before handoff

- Required script metadata (`name`, `category`, `purpose`) checked across all packaged `.chqscript` files.
- Required regression metadata (`regression-version`, `regression-scope`) checked across all packaged scripts in `research/scripts/regression`.
- The RC2 regression script now declares `regression-version: 0.66.9.0` and `regression-scope: version`.
- Script include targets checked for existence.
- JSON files parsed successfully.
- `build-local.json` retained.
- Package checked to exclude `out`, ROM payloads, transient evidence and bundled SDL source.
- ZIP integrity verified after packaging.

Windows/SDL runtime proof remains the user-side gate; begin with `Build-Debug.bat`.
