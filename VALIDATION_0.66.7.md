# v0.66.7 validation record

Status at packaging: **IMPLEMENTED — AWAITING WINDOWS/SDL PROOF-OFF**.

Previous proven baseline: **v0.66.6.1**.

## Completed in packaging environment

- Native version, CMake package version, Research API schema, script schema, feature manifest and knowledge dataset identify build `0.66.7`.
- Machine-readable JSON contracts parse successfully.
- Knowledge integrity checks pass: unique IDs, required metadata, valid related-entry references and referenced curated evidence files.
- Workbench embedded JavaScript passes `node --check`.
- CMake non-SDL configuration/build completes successfully.
- CTest non-SDL suite: **1/1 PASS** (`cpu_bus_rom_tests`).
- v0.66.7 regression is present in the full regression suite.
- `docs/CHATGPT_HANDOVER_PROMPT.md` is current for v0.66.7 and declares the ZIP + prompt as the authoritative continuation source.

## Environment-limited validation

This packaging environment does not provide the project's Windows SDL3 source tree/toolchain or PowerShell runtime. Therefore the following remain release prove-off requirements on the normal Windows development machine:

1. `Build-Debug.bat` must compile the SDL frontend including the new window-control and individual-sprite snapshot paths.
2. Workbench/PowerShell launch and route parsing must complete normally.
3. Run **Regression - v0.66.7 Workbench Knowledge / SDL / Sprite Controls** and inspect its evidence bundle.
4. Confirm `frame.snapshot` still reports exact aggregate reconstruction and produces `scene-no-sprites.png`, `sprites/sprites.json` and the expected per-sprite PNG assets.
5. Confirm Workbench Docs/Knowledge search/export/handover, Graphics Lab Individual sprites, and SDL Window Controls operate against the live runtime.

v0.66.7 must not be called proven until that Windows/SDL regression passes.
