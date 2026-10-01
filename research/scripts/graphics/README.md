# Graphics scripts

Reusable graphics, image-diff, palette and live SDL sprite research workflows.


## v0.66.6.1 layered frame snapshots

Use `api frame.snapshot name=NAME` to capture a v2 reconstructable graphics bundle. The bundle contains exact alpha contribution layers, raw source PNGs, Normal/HUD-hidden/HUD-only finals, reconstruction metadata and semantic sprite evidence. Use `api image.compare ... hud=hidden` when HUD motion would otherwise contaminate a scene comparison.

## v0.66.7 individual-sprite snapshot forensics

Layered `frame.snapshot` capture now retains the aggregate sprite contribution used by exact reconstruction and additionally writes `scene-no-sprites.png`, `sprites/sprites.json`, and cropped transparent `sprites/slot_NNN.png` assets for visible sprites. These per-sprite assets are exploratory forensic renders for Graphics Lab hide/show, solo, opacity, selection and provenance inspection; they are not a replacement for the aggregate compositor contribution layer. The Workbench exposes the controls under the **Individual sprites** snapshot mode. API/script callers continue to use `frame.snapshot` through the documented action surface.

## v0.66.7.9 turbo/HUD recovery workflow

`turbo-hud-before-during-after.chqscript` is the primary next graphics investigation. It captures before/during/after structured snapshots around a genuine turbo activation and compares `HUD_TURBO` in normal, HUD-only and HUD-hidden modes. Use the result to decide whether to investigate TC0100SCN text/tile generation or later composition. `canonical-checkpoint-visual-survey.chqscript` provides a four-checkpoint visual baseline after renderer changes.

## v0.66.9.0-RC2
v0.66.9.0-RC2 adds timeline-backed turbo animation verification plus player-car shadow/sprite-order A/B investigations.

v0.66.9.0-RC2.2 adds `same-priority-sprite-order-multiscene-validation.chqscript`, using list/for constructs to compare the reference lower-slot ordering against the legacy higher-slot mode across existing captured scenes.

## v0.66.9.0-RC2.4.2 / RC2.4.3 TC0100SCN post-fix reference

`tc0100scn-post-y-fix-layer-isolation.chqscript` is retained as a deterministic post-Y-fix layered reference. The RC2.4.2 Windows capture already localized the visible sky texture to the raw TC0100SCN bottom/background source before composition; it is therefore not an active compositor-bug target without contradictory reference evidence.

The snapshot name in the script is only an evidence label. RC2.4.3 makes its immediate list check assertive with `frame.snapshot.list require=tc0100scn-yfix-2065`.
