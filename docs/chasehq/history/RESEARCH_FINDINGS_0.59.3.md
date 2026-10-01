# Research Findings — v0.59.3 baseline

## Handling/cornering transform

Instruction-level tracing proves that the normal player movement path uses a paired ROM coefficient table around `$00AD46`. Internal speed is in D2. Turn/handling state selects the table index.

The authoritative lifetimes are:

- `$008AAA` before `MULS.W D2,D4`: D4 = forward coefficient.
- `$008AAC` before `ASR.L #8,D4`: D4 = forward product; arithmetic shift by 8 produces the forward component.
- `$008AB2` before `MULS.W D2,D3`: D3 = lateral coefficient.
- `$008AB4` before `ASR.L #8,D3`: D3 = lateral product; arithmetic shift by 8 produces the lateral component.

Observed native forward coefficients include `$0100`, `$00FE`, `$00F4`; the paired lateral coefficient rises with turn magnitude. These are best described as proven movement/handling coefficients. Do not label them tyre grip until further state-path evidence supports that semantic interpretation.

v0.59.3 performs optional overrides after the game has loaded each table coefficient but before the original multiply instruction executes. This preserves table selection, speed input, downstream game logic and ROM bytes.

## Course-follow experiments before v0.59.3

A same-state v0.59.1 legacy-versus-predictive run showed predictive steering was smoother but road holding was worse: predictive spent much longer in recovery and covered less course. A short v0.59.2 hybrid acceptance likewise remained off-road for most of the sampled window. Therefore the current preferred mapping-driver architecture is not purely reactive prediction.

The new direction is:

1. course-position-indexed feed-forward steering/throttle/brake learned from a prior pass;
2. authoritative live centre-line correction at all times;
3. aggressive recovery that temporarily discards feed-forward when far from centre/off-road;
4. branch-specific profiles after route-selection state is proven.

## Road geometry

`$10A05C` supplies the live boundary pair and `$10A044` the player lateral coordinate. Derived road centre, width, signed lateral error and normalized error are now first-class mapping/controller telemetry. Centre of the live road remains the mapping driver's default target line.

## Collision-free mapping

The current evidence-scoped suppression covers lateral response PCs `$00A142/$00A156/$00A1BE/$00A1C4` and speed penalty `$00A200`. Detection, interaction state, target events and scoring remain live. Additional collision families must be proven before inclusion.

## Open questions

- Does increasing the lateral coefficient alone materially increase high-speed cornering authority while keeping the car centred?
- Is there another road-surface/airborne/traction multiplier downstream of this table?
- What is the authoritative AIRBORNE/GROUNDED state?
- What state commits the route at the first fork?
- Is Stage 1 a finite graph with reused banks, a cycle, or another form of context-dependent course reuse?
