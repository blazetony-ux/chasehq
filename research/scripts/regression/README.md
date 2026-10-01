# Regression scripts

Living regression suite. Every new Script Console syntax feature or Research API action should add/update coverage here in the same release.

- `regression-v06661-layered-frame-snapshot.chqscript` validates v2 layered frame capture and HUD-aware image comparison.
- `regression-v06674-knowledge-sdl-sprite-controls.chqscript` validates v0.66.7.4 Docs/Knowledge APIs, authoritative handover access, SDL window control actions, HTML docs export, and per-sprite layered snapshot artifacts.

## v0.66.9.0-RC2
v0.66.9.0-RC2 adds `regression-v06690-rc2-sprite-order-speed-semantics.chqscript`; run it before Full Regression.

v0.66.9.0-RC2.2 added `regression-v06690-rc22-mame-sprite-order.chqscript` to prove the MAME-reference descending traversal. RC2.2 incorrectly inferred that later-drawn lower-numbered entries therefore owned overlaps; RC2.5 keeps the descending traversal but adds the missing first-nontransparent occupancy semantics.

## v0.66.9.0-RC2.4.2 / RC2.4.3

The RC2.4.2 `regression-v06690-rc242-tc0100scn-snapshot-refresh.chqscript` is retained as historical evidence but is **not sufficient** to prove immediate discovery: Windows bundles showed its `frame.snapshot.list` output omitted the just-created snapshot while the run still reported PASS.

RC2.4.3 adds `regression-v06690-rc243-snapshot-discovery.chqscript`. It uses `frame.snapshot.list require=regression-rc243-snapshot-discovery`, so the run must fail if the current-run snapshot is not immediately returned. `Validate-FrameSnapshotRefresh.ps1` remains the independent HTTP-level assertion.


v0.66.9.0-RC2.5 adds `regression-v06690-rc25-script-safety.chqscript` for `try/finally` + idempotent timeline cleanup, and keeps `regression-sprite-ownership.chqscript` as the canonical frame-2352 sprite-ownership/export-coordinate assertion.
## v0.66.9.0-RC2.6.1

RC2.6.1 adds the explicitly version-labelled `regression-v06690-rc261-sprite-bitplane-significance.chqscript`. It is the focused Script Console integration regression for the RC2.6 shared sprite-decoder correction and is included before the Full Regression terminal COMPLETE marker. `Test-SpriteGfx.bat` remains the exhaustive all-16-pen native assertion; `graphics/sprite-bitplane-significance-proveoff.chqscript` remains the visual/structured no-override prove-off.

