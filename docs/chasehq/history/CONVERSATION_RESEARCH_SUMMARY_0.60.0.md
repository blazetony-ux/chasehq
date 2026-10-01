# ChaseHQ-Native — Conversation & Research Consolidation through v0.59.4

This document consolidates the major technical findings, experiments, accepted hypotheses, rejected hypotheses, tool limitations, checkpoint semantics and near-term priorities established during the long-running Chase H.Q. native-emulation/reverse-engineering session leading to v0.59.4. It is intended to be readable without reconstructing the project history from chat or dozens of evidence ZIPs.

## 1. Project direction and release discipline

Chase H.Q. remains the sole target. The project has evolved from basic CPU/video bring-up into a deterministic reverse-engineering platform with reusable traces, checkpoints, structured gameplay telemetry, course surveying, collision intervention and a course-aware driving controller.

The working discipline is evidence-first:

- prefer authoritative emulated/game state over screenshot inference;
- use controlled A/B interventions to establish causality;
- retain legacy behaviour as a comparison baseline;
- keep checkpoint/state experiments reproducible;
- make diagnostics parameter-driven and reusable rather than cutting one-off builds;
- preserve the known-good Windows build layout: `out/build/x64-Debug/ChaseHQNative.exe`;
- avoid changing build-system layout casually after the v0.58.2 multi-config regression;
- cut releases at meaningful capability milestones and carry the documentation/checkpoint corpus forward.

## 2. Graphics/compositor foundation

The PROM-aware compositor is the accepted default. Earlier REFERENCE and LEGACY modes remain useful diagnostics but no longer define correctness. A visible-area correction and evidence-backed TC0100SCN background offsets were established earlier. Sprite tie-breaking is provisionally higher-slot-wins.

The apparent v0.59.2 frame-532 graphics regression was disproved by a same-frame v0.59.1/v0.59.2 evidence comparison: substantive graphics artifacts were identical. At that boot/attract point sprite RAM and road state were legitimately empty/cleared, so it is not a release regression.

Sprite tooling now includes decoded snapshots, RAM dumps, assembled sprite export, raw tile export, atlas export, per-frame sprite evidence, owner/overlap analysis, pixel provenance and semantic player-car tracking.

Stage-1 target visual identity has been correlated with sprite maps `319,320,321,322`, palette `152`.

Billboard replacement remains a future modding task. The required infrastructure is largely present, but the exact billboard implementation (OBJ sprite vs BG tile path) still needs to be identified from a visible example. Runtime replacement is preferred over ROM modification.

## 3. Authoritative gameplay state

Known gameplay state includes:

- displayed SPEED: CPU-A `0x100400` (packed BCD);
- internal/base speed: `0x10041C`;
- DISTANCE / course progression: `0x102FC0`, 24.8 fixed-point during normal progression;
- SCORE: `0x100408`, with underlying BCD accumulator `0x100488-0x10048B`;
- TIMER: `0x100200`;
- processed steering: `0x100300`;
- TURBO active: `0x100212`;
- TURBOS remaining: `0x1003A2`;
- authoritative player lateral coordinate: `0x10A044`;
- road/contact field: `0x10A048`;
- live road-boundary pair: `0x10A05C`.

Road classification is evidence-backed as:

- INTERIOR: `!(flags & 0x06)`;
- EDGE: `(flags & 0x06) == 0x02`;
- OFF_ROAD: `(flags & 0x04) != 0`.

From the boundary pair and `0x10A044`, the runtime derives road left/right, centre, width, absolute lateral error and normalized centre error. These values are now central to autopilot and mapping evaluation.

The sampled/frame-level telemetry can lag instruction-time events. This was demonstrated by score writes occurring before the later frame-sampled score-change event. Event-time traces remain authoritative when exact ordering matters.

## 4. Course/track data

The runtime course store is `0x109000-0x109FFF`, organized as 16 banks × `0x100` bytes, each bank containing 32 records × 8 bytes.

Proven record channels:

- `+0`: signed horizontal curvature/trajectory control;
- `+1`: signed vertical road-profile control.

`+2` remains unresolved. `+3` is a distinct road parameter but does not yet have a final semantic label. `+4..+7` remain unresolved.

The horizontal chain reaches a pre-perspective trajectory around `0x108400` and perspective-scaled state around `0x1012B2`. The vertical `+1` channel has been traced through intermediate state into road projection.

Course selection uses the current bank from `0x1021BC & 0x0F` and course-position-derived record selection. A ROM linkage table exists around `0xF5A4`, but runtime bank progression proves that the actual course is contextual rather than a trivial static sequence.

Observed long-run progression has included:

`0 -> 1 -> 3 -> 7 -> 15 -> 0 -> 1 -> 2 -> 3 -> 4 -> 5 -> 6 -> 7 -> 8 -> 9 ...`

This strongly motivates a graph representation with forks/branches/rejoins rather than a single centreline. The actual route-selection variable and first branch-commit frame are still unproven.

A later run entered a terminal/sentinel course state after the Stage-1 target defeat sequence. This must not be treated as proof that the underlying track data physically ends: the transition is likely stage/encounter-completion driven. Mapping mode should eventually be able to suppress stage completion to test whether meaningful course data continues.

## 5. Steering and handling model

Injected steering and game steering were mapped as signed 12-bit values with bit 11 as sign. Representative proven values include +32→+12, +64→+51, +96→+96 and symmetric negative values.

A major v0.59.x discovery is the handling/cornering transform around CPU-A `0x008Axx`. The game uses a ROM table around `0x00AD46` containing paired coefficients. The table index is derived from turn/steering state. Both coefficients are multiplied by internal speed.

Authoritative instruction-time capture points are:

- `0x008AAA`: forward coefficient still in D4 before multiply;
- `0x008AAC`: forward component in D4 after multiply/shift;
- `0x008AB2`: lateral coefficient still in D3 before multiply;
- `0x008AB4`: lateral component in D3 after multiply/shift.

Observed examples include:

- straight / low turn: forward `0x0100` (1.000), lateral `0`;
- stronger turn: forward `0x00FE` (~0.992), lateral `36` (~0.141);
- large turn: forward `0x00F4` (~0.953), lateral `77` (~0.301).

v0.59.3 introduced reversible runtime handling overrides after table load but before multiplication, leaving the ROM and table-selection logic intact.

Controlled A/B sweeps established that increasing lateral authority improves road retention but too much causes long overshoot/recovery episodes. Across the longer fine sweep, `--cornering-scale 1.15 --cornering-speed-retain 1.00` was the best tested mapping baseline, with approximately 89.7% INTERIOR and only ~8.1% OFF_ROAD over that window. Higher scales (1.20-1.35) produced worse long excursions even when some short-window metrics looked attractive.

The handling telemetry bug in v0.59.2, where a forward product was mislabelled as lateral coefficient, was fixed in v0.59.3 by capturing the registers at their true lifetime points.

## 6. Autopilot evolution

The v0.57 closed-loop controller uses live road centre and car lateral position plus course-curvature feed-forward. It substantially improved over earlier open-loop surveying but remained imperfect.

A predictive controller with deeper look-ahead, PD correction, steering slew limiting and curve-aware speed control was implemented and A/B tested. It was smoother but materially worse at road holding: it entered fewer but much longer off-road excursions. A hybrid recovery variant also remained inadequate.

The preferred architecture is now:

1. learned course-position feed-forward from a prior teacher run;
2. authoritative live road-centre correction at all times;
3. aggressive recovery that temporarily overrides learned feed-forward when strongly displaced;
4. future explicit branch selection once route state is proven.

`driver_profile.csv` is the groundwork for this approach. A long teacher pass with 1.15× cornering recorded thousands of dense course-position samples.

The car should target the geometric centre of the track by default. Apex/racing-line bias is intentionally not the mapping objective. At forks, the desired centre should transition to the centre of the selected branch.

## 7. Collision system and collision-free mapping

The first Stage-1 target collision revealed a physical lateral shove at `0x00A156`, writing player lateral coordinate `0x10A044`. Subsequent long traces exposed the wider response family:

- `0x00A142`: lateral shove one direction;
- `0x00A156`: lateral shove opposite direction;
- `0x00A1BE`: alternate lateral response;
- `0x00A1C4`: alternate lateral response;
- `0x00A200`: explicit speed penalty that replaces internal speed with 75% of its previous value.

v0.59.1+ `--no-collisions` suppresses these proven physical consequences while leaving collision/contact detection, interaction state, scoring and object behaviour alive. This is intentionally evidence-scoped; other response families may still exist.

The original target contact chain remains valuable:

`target/player proximity -> 009Cxx test -> 009E2A interaction -> type/event 9 -> score/event handling -> physical response`.

`0x10A089 bit 5` is a strong object-local contact-state candidate.

## 8. Stage-1 target / pursuit object

The Stage-1 target is tracked through the object record at `0x10A080` (0x40-byte object stride). Its visual identity has been correlated to sprite maps 319-322/palette 152.

Target-related state currently includes:

- target longitudinal position: `0x10A080`;
- target-local status byte: `0x10A089`;
- target forward movement/speed magnitude candidate: `0x10A092`;
- target movement-control/deceleration parameter: `0x10A096`;
- interaction type: `0x10042E`;
- interaction timer: `0x1002C6`;
- interaction/phase counter: `0x1002AE`.

`0x1002AE` must NOT be labelled simple health. Controlled experiments proved it can underflow without being a conventional HP variable. However, in the natural final-hit sequence it participates in the terminal interaction: it reaches zero, then the decisive type-9 interaction decrements `0 -> FFFF`, taking a special negative branch.

The final-hit trace established:

- natural `0x1002AE: 0000 -> FFFF` at the decisive interaction;
- special branch writes `0x1002D8 = 0x1400`, but an A/B experiment proved this value alone does not drive target slowdown;
- `0x1002D4/0x1002D6` form short periodic sub-state pulses, but A/B suppression did not alter the defeated target behaviour;
- a distinct defeated-motion routine begins around `0x00778C`;
- that routine sets bits 0 and 2 of `0x10018D`;
- it examines target/player separation;
- it can force `0x10A096 = 0x0080` and initialize `0x1002D0 = 0x0096` (150 frames);
- while `0x10A096` remains in defeated-motion control, `0x10A092` decreases steadily and the target visibly slows.

A decisive A/B experiment restored `0x10A096` from `0x0080` to `0x01AA` after defeated mode had begun. The target's `0x10A092` then stopped following the normal descending sequence and stabilized around `0x01A9-0x01AC`, while the global defeat flags/timer remained active. This causally proves that `0x10A096` controls the defeated target's deceleration/motion behaviour.

Current labels therefore are:

- `0x10018D bits 0+2`: strong TARGET DEFEAT / TRANSITION MODE candidate;
- `0x1002D0`: 150-frame post-defeat transition timer candidate;
- `0x10A096`: target longitudinal movement-control/deceleration parameter (causally supported);
- `0x10A092`: target forward movement/speed magnitude candidate (strongly supported).

These are exposed on the v0.59.4 TARGET debug page and added to `target_state.csv` with candidate-oriented names rather than being mislabelled as health.

## 9. Stage completion / level-skip direction

A Stage-1 completion run showed normal course progression stop, a later semantic `0xFFFFFFFF` terminal course state, then end-level scoring/transition behaviour. Importantly, no simple literal `FFFFFFFF` write to `0x102FC0` was observed at the relevant instant; the exporter is representing a semantic terminal state, not necessarily a raw distance write.

Therefore a future stage skip should not be implemented as `distance = FFFFFFFF`. The safer research chain is:

`target defeated mode -> target slowdown/transition -> stage-complete state -> terminal course state -> end-level screen -> next stage`.

A future `--force-stage-complete` or stage-skip option should reproduce the game's own companion flags/timers rather than forcing one address.

This work also provides a potential mapping feature: suppress the stage-complete transition while preserving course progression, allowing the mapper to test whether useful course topology exists beyond normal Stage-1 completion.

## 10. Canonical checkpoints

Bundled canonical checkpoints in v0.59.4 include:

- `stage1-gameplay-2064.chqstate` — early player gameplay/research checkpoint;
- `stage1-driving-2352.chqstate` — autonomous-driving/road research checkpoint;
- `stage1-target-post-final-hit-9548.chqstate` — user-confirmed state after the decisive hit, with target entering slowdown;
- `stage1-end-level-9988.chqstate` — user-confirmed end-of-level screen.

A highly valuable `stage1-target-approach-7060` checkpoint has been validated in previous working trees but was not present in the v0.59.3 distribution supplied to this build environment. It should be copied/generated into the distributable corpus when its validated bytes are available.

A local 9495 pre-final-hit checkpoint was created during experiments, but its bytes were not included in the uploaded evidence archive; do not claim it is bundled until supplied.

Planned CHQSTATE v2 remains backward-compatible and should add optional metadata plus an embedded PNG preview. This is intentionally separate from restore-critical state.

## 11. Debugger / tracing limitations discovered

Important tool limitations found through real experiments:

- `--trace-mem-change` did not reliably expose a known score write that explicit `--trace-mem-w` did;
- frame-sampled changes can lag event-time writes;
- executable/ROM patch reporting can say a patch succeeded while CPU instruction fetch still executes original ROM bytes;
- `--patch-when` failed to react to a write/value that explicit tracing observed;
- `--patch-at-frame` works reliably for ordinary RAM when timed after the producer write;
- checkpoint/save CLI arguments must be treated exactly as implemented (`--save-checkpoint FRAME:PATH`);
- `--fast-forward-status` expects a numeric interval;
- graphics state capture and screenshots are different concepts;
- quiet fast-forward deliberately suppresses much forensic bookkeeping before the target frame.

These limitations belong on the debugger roadmap because reliable intervention is essential for branch forcing, target-health experiments and future stage-skip work.

## 12. Performance observations

Before quiet fast-forward, heavy diagnostic bookkeeping reduced a long run to roughly realtime (~56 FPS in an earlier benchmark). Quiet fast-forward raised this to roughly 270-290 FPS (~4.5-4.9× realtime) while preserving deterministic target-frame behaviour.

Heavy forensic runs with broad memory/PC tracing and callstack/context capture can still fall to only ~4-5 FPS and appear to saturate one host logical core. The safe future performance direction is to keep emulated CPUs/scheduler deterministic while moving formatting, logging, PNG/export and other post-snapshot diagnostics to worker threads. Direct CPU-A/CPU-B host-thread parallelism is high-risk and should not precede profiling/scheduler validation.

A future `--perf-profile` should attribute time to CPU-A, CPU-B, scheduler/bus, road, renderer, trace collection, callstack processing, serialization and export.

## 13. Debug UI redesign in v0.59.4

The original F1 panel became too crowded as authoritative telemetry accumulated. v0.59.4 changes the overlay to four pages, cycled with F11 (Shift+F11 goes backwards):

1. DRIVING — speed/distance/score, road state, boundaries, width, centre, car position, normalized centre error, curve/grade, course bank/record and assists.
2. HANDLING — steering/curve, centre error, native/applied handling coefficients, forward/lateral components and override status.
3. TARGET — target/player longitudinal positions, separation, contact flags, target movement/speed, motion control, interaction state, defeat flags and defeat timer.
4. SYSTEM — CPU PCs, checkpoint slot and interactive debugger key reference.

This keeps live gameplay visible and makes each page semantically focused instead of continuously extending one narrow column.

## 14. Immediate next priorities after v0.59.4

1. Windows-build/accept the paged UI and new target-state CSV columns.
2. Preserve/restore the 7060 target-approach checkpoint in the distributable checkpoint corpus.
3. Use the post-final-hit and end-level checkpoints to continue proving the exact stage-complete trigger.
4. Test whether forcing the `0x10018D` defeat-mode flags plus required companion state can initiate the normal transition without a final hit; do not implement stage skip until this is deterministic.
5. Resume learned profile replay using 1.15× cornering and always-on road-centre correction.
6. Identify authoritative AIRBORNE/GROUNDED state and integrate it into controller/mapping telemetry.
7. Locate the first actual route/fork commit and create a canonical fork checkpoint.
8. Run left/right from the same fork state, discover the route selector and construct a branch/rejoin graph.
9. Only after stable mapping, revisit billboard replacement and broader graphics mods.
10. Add safe performance profiling/asynchronous diagnostics before considering threaded emulated CPUs.

## 15. Confidence labels

The project should continue distinguishing:

- PROVEN / causal: intervention changes the downstream behaviour in the predicted way;
- STRONG CANDIDATE: repeated temporal/code-path evidence but not yet isolated causally;
- DERIVED: mathematically calculated from proven underlying values;
- UNKNOWN: observed field without enough semantic evidence.

This is especially important for target health/defeat state: `0x1002AE` is not to be called HP, while `0x10A096` has direct causal support as a target defeated-motion control.

---

# v0.60.0 tooling direction and implementation

After the Stage-1 target-defeat investigation, the project deliberately shifted from frequent narrow diagnostic builds to a broad live remediation workbench. The requested end-state is not merely "trace more": it is **detect an event, pause, alter emulated state directly, run a controlled amount, inspect the result, restore the exact same baseline, and repeat without rebuilding**.

The architectural goal is deliberately subsystem-neutral: memory, sprites, future controls/IOC, audio, timing, CPU/device communication, tilemaps, road hardware and semantic gameplay state should all become event producers feeding reusable trigger, intervention, provenance, diff and experiment layers.

v0.60.0 implements the first substantial slice of that architecture:

- F12 dedicated Research Workbench, separate from F1 gameplay telemetry;
- automatic pause + temporary CHQSTATE baseline;
- arbitrary live RAM edit on CPU A/B with selectable 8/16/32-bit width;
- write-once, freeze and write-suppression interventions;
- watched writes/changes with automatic pause at a safe frame boundary;
- rolling memory-event history including PC and old/new value;
- CPU-local single-instruction step plus controlled frame stepping;
- F9 rewind to the same pause baseline for repeated counterfactual trials;
- semantic sprite-change detection and recent sprite-RAM writer-PC correlation;
- interactive evidence export of research events, active patches and sprite changes.

The enemy hit bar is the intended first high-value test. Instead of guessing a health address, the new workflow can use the visible bar change as an anchor, identify which decoded sprite changed, correlate the responsible sprite-RAM writer PC, then live-patch candidate source state and repeat from the same baseline.

Important v0.60.0 boundaries: watch breaks are currently safe-frame rather than mid-frame; freeze/suppress interception matches exact access width/address; full A/B/N automatic ranking, deep source-read provenance, async event serialization and non-memory adapters are still future layers. These are explicitly tracked rather than hidden behind version claims.
