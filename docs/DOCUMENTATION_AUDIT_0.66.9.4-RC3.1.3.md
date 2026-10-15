# Documentation audit — v0.66.9.4-RC3.1.3

RC3.1.3 is a release-tooling correction only. The RC3.1.2 manual Windows proof passed runtime check, focused regression and Full Regression, while `qualify --fresh` failed before regression because the cold authoritative run-history scan consumed the startup readiness budget.

RC3.1.3 separates lightweight startup readiness (Workbench health, exact native identity, mobile pairing) from the potentially expensive authoritative run-history scan. `qualify --fresh` runs the history gate after focused + Full Regression with a 30-second history timeout, prints per-component elapsed timing, and reports the exact failed readiness component during retries. `chqdev runtime check` remains comprehensive and includes authoritative history.

The focused regression is `research/scripts/regression/regression-v06694-rc313.chqscript`; it carries forward the already proven RC3.1.2 semantic yellow-target steering persistence and live SPEED suppression gates. Full Regression includes RC3.1.3 as its current release tail.

## RC3.1.3 release-process contract

- Startup/runtime restart must not enumerate authoritative history.
- `qualify --fresh` must reach focused regression once lightweight readiness passes.
- Authoritative history remains fail-closed before `promotable`.
- Exact-archive Windows `chqdev qualify --fresh` remains mandatory before promotion.

## Authoritative release process

`chqdev` remains the authoritative build/release controller. `chqdev qualify --fresh` is mandatory on the exact Windows archive, any CTest failure is fatal (zero tolerated CTest failures), packaging must produce a deterministic archive, focused and Full Regression must pass, and PowerShell/BAT shells remain compatibility wrappers rather than the source of new build/release logic.
