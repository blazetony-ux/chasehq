# Release notes — v0.59.2

## Centre-line hybrid follower
The default follower is now `hybrid`. It keeps course-data feed-forward and PD correction while the car is stable, but switches to unsmoothed high-authority centre recovery when off-road, at an edge, above 72% normalized half-width error, or at very large absolute lateral error. `legacy` and `predictive` remain available. Default look-ahead is eight records; predictive speed reduction is disabled by default pending evidence that it improves road holding.

## Road geometry
Course/gameplay logging now includes road width and normalized centre error. The F1 debug overlay exposes road width and centre error directly.

## Handling/cornering telemetry
The runtime captures the proven `$008Axx` handling path at instruction time: turn state, table index, internal speed, forward coefficient, lateral coefficient, and their resulting components. Course surveys export `handling_state.csv`. This is diagnostic only: v0.59.2 does not yet alter the game coefficients.

## First-pass driving profile groundwork
Course surveys export `driver_profile.csv`, keyed by course position/bank/record and containing curve, road geometry, centre error, steering target, accelerator/brake command, target speed and controller mode. This is intended to become the feed-forward source for iterative learned course driving.

## Retained v0.59.1 features
Generalized evidence-scoped collision-response suppression, target/collision telemetry and bounded-run ETA remain unchanged.
