# Historical launcher note redirect - v0.66.9.1

The original v0.62.2 launcher repair note has moved to `docs/platform/history/LAUNCHER_0.62.2.md`.

Current operational authority is `docs/RELEASE_GOVERNANCE.md`, `docs/RELEASE_PROCESS.md`, `docs/CHQDEV.md` and `QUICK_START.txt`. `chqdev` is the de facto and authoritative build/release process. Fresh Windows promotion uses `chqdev qualify --fresh`; zero tolerated CTest failures, deterministic exact archive proof, focused + Full Regression and PowerShell/BAT compatibility/fallback-only status are mandatory.
