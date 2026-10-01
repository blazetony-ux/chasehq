# v0.52.0 experiment platform

Current evidence-backed geometry candidate: BG0 Y=-16, BG1 Y=-20. Sprite global offset remains 0,0: the v0.51.2 XY sweep showed that a blanket sprite translation does not solve the roadside-scenery problem without displacing the player car.

Immediate validation target is same-priority sprite overlap. The player shadow is slot N and body N+1 with equal priority; v0.51.2 evidence showed lower-slot ownership at every sampled overlap. Use `--sprite-tie-break higher-slot` as a controlled experiment and compare the player car plus unrelated overlapping sprites before changing the permanent default.

For source/layer questions, use existing pixel provenance (`--pixel-provenance X:Y`) and region tracing (`--gfx-trace-region X1:Y1:X2:Y2`) before adding one-off instrumentation. Targeted sprite CSV now exposes native and presentation coordinates.
