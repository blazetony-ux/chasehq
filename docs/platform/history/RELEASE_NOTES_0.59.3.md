# v0.59.3 — Handling Override & Learned Course-Profile Driver

## Added

- Correct handling coefficient/component capture at the proven instruction lifetimes:
  - `$008AAA`: original/applied forward coefficient in D4 immediately before multiply.
  - `$008AAC`: forward product/component before the game's `ASR #8`.
  - `$008AB2`: original/applied lateral coefficient in D3 immediately before multiply.
  - `$008AB4`: lateral product/component before the game's `ASR #8`.
- `--cornering-scale X` (`0.0..4.0`) scales the lateral coefficient without modifying ROM data.
- `--cornering-speed-retain X` (`0.0..2.0`) scales the forward coefficient without modifying ROM data.
- F1 displays original coefficients, applied coefficients when an override is active, and resulting components.
- `handling_state.csv` exports original + applied coefficients, ratios, components, override-active state and sample count.
- `--course-profile FILE` loads a `driver_profile.csv` and selects `profile` controller mode.
- Profile controller replays nearest course-position steering/throttle/brake as feed-forward, continuously corrects toward live road centre, and uses hard recovery when strongly displaced/off-road.
- `driver_profile.csv` now also records profile feed-forward steering and course-position match delta.

## Fixed

v0.59.2 captured D4 after the forward multiply and incorrectly labelled it as the lateral coefficient. The register-lifetime trace proved that the lateral coefficient exists in D3 at `$008AB2`; v0.59.3 captures the four handling values at their actual lifetimes.

## Retained

- Evidence-scoped `--no-collisions` suppression of the four proven lateral collision responses and the proven 25% speed penalty.
- Road left/centre/right, road width and normalized centre error telemetry.
- Legacy/predictive/hybrid controllers.
- Quiet fast-forward, progress/ETA, target/pursuit telemetry and course-survey exports.
- Existing checkpoint v1 compatibility; CHQSTATE v2 preview/metadata remains future work.

## First validation goals

1. Confirm corrected F1/CSV coefficients against instruction trace.
2. Run same-state native handling versus controlled lateral-coefficient scales (for example 1.10, 1.20, 1.30) while retaining forward coefficient at native 1.00 scale.
3. Compare centre error, INTERIOR/OFF_ROAD occupancy, speed and course distance.
4. Record a stable first-pass `driver_profile.csv`, then replay it with `--course-profile` and compare against legacy/hybrid from the same checkpoint.
