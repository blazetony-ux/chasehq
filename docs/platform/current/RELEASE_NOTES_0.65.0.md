# ChaseHQ-Native v0.65.0 — Live Graphics & Script Query Workbench

v0.65.0 builds on the validated v0.64.9 graphics/autonomous experimentation baseline.

## Headline changes

- **Live SDL sprite research controls**: inspect a live sprite slot, apply presentation-only map/palette/visibility overrides, and use solo/hide/flash visualization modes. These alter the compositor presentation only; emulated game RAM is not rewritten.
- **First-class sprite Research API**: `sprites.list`, `sprite.inspect`, `sprite.override`, `sprite.override.clear`, `sprite.visual`, and `sprite.visual.clear`, plus matching localhost REST endpoints.
- **API-backed script query functions**: `getregions()`, `getsprites()`, `getmaps()`, `getpalettes()`, `getpaletteentries(bank=N)`, `getcheckpoints()`, and `getcapabilities()`. Assignment returns iterable comma-separated values; invoking a function directly prints the authoritative backing API result for diagnosis.
- **Structured state capture**: `api state.snapshot include=sprites,regions,gameplay,track` returns one coherent current research-state object.
- **Image analysis improvements**: frame-snapshot names now resolve directly in `image.compare` / `image.diff`; image regions are schema v2 with narrower/non-overlapping player/HUD boundaries and road-near/mid/far regions.
- **Script engine practical limit update**: loop/repeat limit raised from 64 to 256 while retaining the 500-expanded-command ceiling and nesting safety limit.
- **Graphics Lab live-preview controls**: row selection feeds the SDL sprite slot control; inspect/apply/solo/hide/flash/clear actions are available without typing commands.
- **Bundled script library cleanup**: stable categories are now regression, examples, graphics, track, IOC, gameplay, evidence, diagnostics and scenarios, while hypothesis-driven work remains under experimental.
- **Living regression policy**: new Script Console syntax/API features must add or update regression coverage in the same build.

## Important behaviour

Live sprite overrides are deliberately **presentation-only**. They are designed to let a researcher eyeball a map/palette/visibility change in the real SDL compositor while deterministic A/B captures quantify the same experiment. Clearing overrides restores normal presentation immediately.

`flash` is frame-driven in this first implementation: it alternates while emulation advances. Solo/hide and map/palette overrides refresh immediately on a paused machine.

## Windows validation requested

Run the normal launcher, then run `research/scripts/regression/quick-smoke.chqscript`, `regression-script-query-functions.chqscript`, `regression-checkpoint-render-determinism.chqscript`, and `regression-live-sprite-api.chqscript`. See `VALIDATION.md`.
