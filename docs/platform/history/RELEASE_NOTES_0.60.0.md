# Release notes — v0.60.0 Research Workbench

v0.60.0 is a tooling milestone. It carries forward v0.59.4 and adds the first coherent live reverse-engineering/remediation environment.

## Added

- dedicated F12 Research Workbench;
- automatic pause-point checkpoint on Workbench entry;
- live arbitrary 8/16/32-bit CPU-A/CPU-B RAM editor;
- WRITE ONCE, FREEZE and SUPPRESS interventions;
- generic write/change watches with safe-frame auto-pause;
- rolling research memory-event ring;
- paused CPU-A/CPU-B instruction stepping;
- +1/+5/+30 controlled frame trials;
- instant F9 restore to the Workbench pause baseline;
- semantic sprite-change history and recent sprite-RAM writer-PC correlation;
- active patch interception counters/last-writer telemetry;
- research CSVs included in F10 interactive bundles;
- runtime unit coverage for write-once, freeze, suppress and watched-change break primitives.

## Retained

- v0.59.4 four-page F1 overlay;
- v0.59.3 handling override/profile-driver groundwork;
- 1.15× best tested mapping cornering scale;
- generalized evidence-scoped collision suppression;
- Stage-1 target defeat/motion telemetry and bundled 9548/9988 checkpoints;
- quiet fast-forward, scenario and trace infrastructure.

## Important semantics

Research patches are temporary interventions. A successful intervention is evidence for a future runtime option/mod/fix only after causal validation; v0.60.0 does not automatically rewrite ROM/source code.

Watch-triggered pauses occur at the next safe frame boundary in this release. CPU-local single-instruction stepping is available while paused, but does not constitute whole-machine reverse/forward time travel.
