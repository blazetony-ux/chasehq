# Documentation Audit — v0.67.0.0-RC3

> Current development candidate: **v0.67.0.0-RC3**. Exact source baseline: **v0.67.0.0-RC2.1** archive SHA-256 `0199faf2333ebbe25e7c23df2c570d0f5f35c9179b8ea53b43983136f3885fb6`.

RC3 introduces `chqimport` and the deterministic `CHQPACK1` runtime-content format. CPU program data, graphics regions and mixer/road PROMs can be imported once and subsequently loaded through `--content-pack` with no loose-ROM-directory dependency. The direct ROM path remains available as the reference oracle.

The native-rendering state inherited from RC2.1 is intentionally unchanged. RC3's architectural change is the content-source boundary.

## Release authority and qualification

`chqdev` is the **de facto and authoritative build/release process** for this candidate. The canonical Windows promotion command is `.\chqdev.bat qualify --fresh`; fresh qualification is fail-closed and requires **zero tolerated CTest failures; any CTest failure is fatal**. Qualification must also complete the focused release regression and **Full Regression** before promotion. Packaging remains deterministic with archive verification. PowerShell/BAT release tooling is **compatibility/fallback only** and is not a second release authority.

The Linux/source preflight and portable CTests can establish source consistency, but they do not replace the required fresh Windows/SDL qualification.
