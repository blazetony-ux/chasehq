## v0.45 diagnostics platform note

v0.45 adds multi-pixel regression sampling, periodic IOC stimulus, generic timed/conditional memory patches, responsive fast-forward progress/event pumping, isolated diagnostic bundles and deeper priority provenance. The known-bad graphics set currently tracks `(99,165)`, `(150,224)`, `(122,239)` and `(197,239)` so compositor changes can be checked against both the continue-screen and player-car cases in one run.

# Chase H.Q. Native — Reverse-engineering notebook (through v0.27)

This document records findings that have been demonstrated by ROM disassembly/runtime tracing. Addresses are hexadecimal unless stated otherwise.

## ROM / hardware identity

Main program ROMs: `b52-130.36`, `b52-136.29`, `b52-131.37`, `b52-129.30`; additional CPU/sound ROMs include `b52-132.39`, `b52-133.55`, `b52-137.51`, `27c256.ic17`. Graphics include TC0100SCN tile ROM `b52-29.27`, TC0150ROD road ROM `b52-28.4`, sprite banks `b52-34/35/36/37` and `b52-30/31/32/33`, and spritemap `b52-38.34`.

The MAME driver identifies dual 12 MHz M68000 CPUs, 4 MHz Z80, YM2610, TC0040IOC, TC0100SCN, TC0150ROD, TC0110PCR and TC0140SYT.

## Sprite/tile decoding

Sprite graphics use 16x16 pixels, planes `{0,16,32,48}`, x offsets 0..15, y stride 64 bits, 1024 bits / 128 bytes per tile. Tile graphics use the MAME `ROM_LOAD16_WORD_SWAP` arrangement. Sprite object layouts derived from `b52-38.34` include 128x128 8x8 chunks from bank A, 64x128 4x8 from bank B at spritemap offset `$20000`, and 32x128 2x8 from bank B at `$30000`. Zoom class comes from `raw=(zoom_x-1)&$7f`: bit `$40` -> 128x128 A, bit `$20` -> 64x128 B, neither -> 32x128 B.

## Main/sub memory maps

CPU A: ROM `$000000-$07FFFF`; work low `$100000-$107FFF`; shared `$108000-$10BFFF`; work high `$10C000-$10FFFF`; IOC `$400000-$400003`; CPU control `$800000-$800001`; sound stub `$820000-$820003`; palette `$A00000`; tilemap `$C00000`; tile control `$C20000`; sprites `$D00000`; motor stub `$E00000`.

CPU B: ROM `$000000-$01FFFF`; private RAM `$100000-$103FFF`; shared RAM alias; TC0150ROD RAM `$800000-$801FFF`.

## CPU-B command/road work

CPU-B IRQ work around `$0478-$0492` compares shared `$10801A`; when zero it writes command 4. Command table observations: command 2 enters `$06C2`; command 4 enters `$05B0`; most other tested values enter `$05AE`.

Command-4 runtime clearing reaches `$0992-$09CC`: `D1=0`, `D2=$FE`, `A4=$800000`, then `$9C8/$9CA` write two longwords per loop iteration until `DBRA` completes. Thus the very hot road writes at `$9C8/$9CA` are intentional clearing/reset work, not decoded road geometry.

Private CPU-B area `$101600-$102200` contains changing data, but a direct consumer feeding road RAM has not yet been established. `$101A58/$101A5A` behaves as a pending value + strobe; a nonzero strobe copies `$101A58` to `$801FFE` and clears the strobe.

## IOC scanner / controls

Scanner around `$2BA4`: selectors 4..0 read `$400001` into `$100140-$100144`; selectors `$0D..08` into `$100148-$10014D`; selector 4 also participates in output shadowing around `$100126`. Snapshots are copied to shared `$108040/$108044/$108048/$10804C`.

Port 2 baseline `$33`: Coin1 `$04` active high; Coin2 `$08` active high; Service `$10` active low; Brake `$20` active low. Port 3 baseline `$3F`: Turbo `$01` active low; Tilt `$02`; Calibrate `$04`; Start1 `$08` active low; Shifter `$10`; Accelerator `$20` active low in the injected XOR model used here.

Steering input passes through `$10014C`, `$2CE0`, `$2D2E` and produces processed state at `$100300`. Digital control decode produces `$100302`.

## Credits / game start

Service can increment credit counter `$100108`. Start handling around `$17D8` tests bit 3 of `$100143`; active-low Start falls through to `$17E0`. The game-start path at `$092C` clears `$10012E`; `$0930` decrements `$100108`; `$0950` calls `$0998`. Scene/task scheduling later reaches `$189C`, `$37B2/$3872`, and scripted control task `$2210`.

Shared `$10801A` is deliberately cleared by CPU A and remains zero; it is not the missing “gameplay started” handshake originally suspected.

## Scheduler `$108A`

`$108A` saves D0-D7/A0-A6, forms a task-record address as `$100000 + word[$100100]`, stores `D1` at record+0 (`$1096`), current stack pointer at +2 (`$1098`), a saved return/context longword at +6 (`$109C`), then jumps `$1056` to schedule another task.

The scripted-control task is slot `$100080`. During its active input waits, `$1096` repeatedly writes state `0001`. At final completion `$2282` sets `D1=0`, `$2284` calls `$108A`, and `$1096` writes `0000` to `$100080`. No later `$2288` resume occurs in the observed run. The task therefore terminates.

## Script interpreter `$2210-$237C`

`$2210` clears bit 5 of `$10017C`, performs setup, yields, then scans a table starting at ROM `$2A750`. Primary bytes observed include `$2E,$37,$2D,$0B,$27`. Once the `$0B` candidate succeeds, the follow-on loop at `$224E-$2274` processes seven adjacent script bytes using `$229C` and `$22C4`.

Decoded event semantics include `$0B` accelerator press/release, `$0E` Turbo press/release, and steering-qualified events `$07/$0D`. Secondary steering validation at `$234C` uses `cmpi.b`, therefore the low byte of `$100300` is a signed 8-bit quantity.

For `$07`, steering must satisfy >= `+$40` in the relevant branch. `C:60` injection produces `$100300=$0060` and passes. For `$0D`, the relevant branch requires <= `-$40`; `C:A0` produces `$100300=$00A0`, whose low byte is signed -96 and passes.

Known successful bytes/events:

- `$2A768=$0B`: accelerator press/release
- `$2A769=$0B`: accelerator press/release
- `$2A76A=$07`: accelerator + right/positive steering (`$60` works)
- `$2A76B=$0E`: Turbo press/release
- `$2A76C=$0D`: accelerator + negative steering (`$A0` low byte works)
- `$2A76D=$0B`: accelerator press/release
- `$2A76E=$0E`: Turbo press/release
- `$2A76F=$07`: final accelerator + positive steering (`$60` works)

## Completion dispatch `$2276-$2386`

After the final event, `$2268` decrements D4 from 1 to 0; `$226A` advances A0 to `$2A770`; `$226C` branches to `$2276`. `$2276` shifts D3 left two positions; with D3=1 it becomes 4. `$2278` loads A0=`$2288`; `$227E jsr (A0,D3.w)` enters `$228C`, whose stub is `bra $237E`.

`$237E` executes `move.l #$00003800,(-$7E8E,A5)`. With A5=`$108000`, destination is `$100172`. `$2386` returns to `$2282`, which sets D1=0 and calls scheduler `$108A` at `$2284`. The scheduler then terminates slot `$100080` as described above.

A direct memory watch extended to 4000 frames found startup RAM-test accesses, `$18B4` clearing `$100172`, and `$237E` writing `$00003800`, but no subsequent CPU-A main-ROM read/write of `$100172-$100175`. Current interpretation: it is a latched state/parameter whose immediate consumer is not a direct later CPU-A access in the observed path.

## Next target

Use v0.27 arming/task tracing to remain silent until `$237E` / `$100172=$3800`, then identify which other scheduler slot changes or executes next. This should reveal the surviving task that takes ownership of the post-script gameplay transition.

## v0.28 — live TC0150ROD road pipeline resolved

Later v0.27 conditional traces supersede the earlier statement above that a direct private-RAM-to-road-RAM consumer had not been established.

CPU A writes `$10801A=FFFF` at PC `$4820` (frame 1627 in the reproducible injected-input run). CPU-B IRQ code then writes persistent command `$8002` to `$100802`. The dispatcher masks the low five bits, indexes the table at `$052E`, and entry 2 resolves to `$06C2`.

Entry `$06C2` imports live shared state and executes `$4994`, `$0ADA`, `$1D8E`, `$2598`, `$339E`, `$3C70`, `$4188`. The large road-data path is `$2598 -> $339E -> $3C70 -> $4188`. The final writer emits interleaved four-word records into TC0150ROD RAM.

For 97 consecutive frame-1627 records the relationship was verified for every entry:

- W0 = `$101618 + i*2`
- W1 = `$101548 + i*2`
- W2 = (`$1011E2 + i*2` & `$07FF`) | `$8800`
- W3 = `$101384 + i*2`

The first records were `800C 8008 8B4C 2010`, `8010 800B 8B4E 1014`, `8014 800D 8B50 4018`. Records are 8 bytes each. 97 records occupy `$308` bytes, exactly `$8004F0-$8007F7` or `$800CF0-$800FF7`, ending at the boundary represented by `$101A5E` (`$8007F8/$800FF8`). `$101A5C` toggles the ping-pong selection each generated frame.

A subsequent CPU-read trace over both generated ranges from frames 1627-1635 produced no CPU reads. The generated table is therefore consumed by the TC0150ROD/video hardware model rather than by another 68000 routine.

The TC0150ROD record format is now identified as four interleaved scanline words: right clip/control, left clip/control, body X offset/control, and colour-bank/road-gfx tile. `$801FFE` is the road control word and selects road A/B banks. Chase H.Q. uses a renderer Y offset of -1. This explains the observed first live write at bank offset `$4F0`: with `y_offs=-1`, that record corresponds to screen line 159.

v0.28 changes the native SDL road renderer to consume this interleaved layout and control word directly instead of the earlier speculative planar interpretation.

## v0.29 SDL frontend IOC pulse fix
The v0.28 SDL frontend parsed `--pulse-ioc` but did not apply the IOC XOR masks each frame. `ChaseHQRuntime` did apply them, explaining why runtime traces reached the frame-1627 live transition while the SDL window remained on the RAM test. v0.29 mirrors the runtime pulse state machine in `main.cpp`, including IOC debug-frame tracking and pulse logging. Version strings were also updated so frontend/runtime identification is unambiguous.

## v0.30 — SDL frontend/core divergence instrumentation

The v0.29 graphical frontend reached frame 1800 while still displaying the RAM-test image and reporting zero rendered sprites. The standalone runtime, using the same scripted IOC sequence, independently confirmed the CPU-A live-mode write `$10801A:0000->FFFF` at frame 1627. v0.30 therefore adds state snapshots inside `ChaseHQNative` at frames 120, 500, 1300, 1627, 1700 and 1800. The snapshot reads the same Runtime/Bus instance passed to `Video::draw_runtime` and reports `$10801A`, `$100802`, `$100804`, `$101A5A`, `$101A5C`, `$101A5E`, `$801FFE`, plus non-zero byte counts for road and sprite RAM. This is diagnostic only; no emulation semantics are intentionally changed.


## v0.31 — frontend IRQ4 root cause and first live road image

The v0.30 frontend instrumentation showed `irq4_requests_A=0`, `irq4_requests_B=0`, and `irq_acks=0` when `ChaseHQNative` was run without `--irq4`. In that condition `$10801A` remained zero through frame 1800, no CPU-B live command was dispatched, and road/sprite non-zero counts remained zero. Re-running the same frontend with per-frame IRQ4 allowed the game to progress into the live scene. The observed frame 1800 contained HUD/background graphics, 47 sprites, and a recognisable perspective road with converging edges/lane markings. This validates the live road-RAM interpretation sufficiently to freeze its basic four-word/banked layout while sprite/scenery and mixer issues are investigated. v0.31 therefore makes frontend IRQ4 unconditional per frame; the standalone diagnostic runtime remains explicitly controlled by `--irq4`.

## v0.32 — Sprite reverse-engineering workbench
Added frame-scoped logical-sprite snapshots to avoid repeated one-off diagnostic builds. Each snapshot preserves raw 4-word sprite RAM entries and records the complete current decode: signed position, X/Y zoom, format selection (128/64/32 x 128), tile/object number, colour, priority, flips, spritemap base, every chunk's map coordinate/index, current little-endian code, byte-swapped comparison code, calculated destination rectangle, and on-screen status. Raw 0x800-byte sprite RAM can be saved alongside the decoded report. A new generic `sprite` trace preset watches writes to $D00000-$D007FF and captures first-change instruction context. This deliberately records raw and interpreted values so a decoder bug cannot hide behind its own interpretation.

## v0.35 — Sprite asset export / graphics RE workbench

The sprite debugger can now preserve each stage of the current interpretation outside the live renderer. At any requested snapshot frame `--sprite-export` creates a frame directory containing a contact sheet, `sprites.csv`, per-slot manifests, an unscaled spritemap reconstruction, and a zoomed reconstruction. `--sprite-export-tiles` additionally exports every referenced 16x16 graphics chunk once. PNG backgrounds are transparent and colours are sampled from live TC0110PCR palette RAM (with the existing diagnostic fallback when an entry is still black).

This deliberately creates three fault boundaries: a bad unscaled reconstruction points toward sprite-ROM decode / B52-38 spritemap / object-bank / chunk-order assumptions; a correct unscaled object but bad zoomed object points toward zoom, flip, or chunk placement; correct exports with a bad gameplay image points toward final coordinates, clipping, priority PROM/mixer, or composition. Raw sprite RAM and CPU write tracing remain available so a visual object can be followed backwards into game logic.

## v0.35 — Chase H.Q. sprite-format correction from v0.33 evidence

The v0.33 frame-1800 export was compared with the historical MAME `chasehq_draw_sprites_16x16` implementation. This exposed several concrete frontend decode errors rather than an uncertain OBJ-B ROM failure.

Confirmed Chase H.Q. 8-byte sprite entry interpretation used by v0.35:

- W0 bits 15..9: Zoom Y; bits 8..0: Y.
- W1 bit 15: priority; bits 14..7: palette bank; bits 6..0: Zoom X.
- W2 bit 15: Flip Y; bit 14: Flip X; bits 8..0: X.
- W3 bits 10..0: spritemap/object number (11 bits). Higher W3 bits are not part of the object index.
- Sprite RAM is consumed back-to-front.
- Zoom values are incremented by one before object-format selection and chunk distribution.
- Y is adjusted by the Chase H.Q. screen offset and by `(128 - zoomY)` before chunk placement.

Rejected v0.33 interpretations:

- Palette from W3 high byte — rejected.
- Priority from W2 bit 15 — rejected; this is Flip Y.
- Flip Y from W1 bit 15 — rejected; this is priority.
- 9-bit (`0x01ff`) object number — rejected; Chase H.Q. uses 11 bits (`0x07ff`).
- Forward sprite-RAM draw order — rejected for hardware-reference rendering.

The existing 128x128 / 64x128 / 32x128 spritemap partitions, OBJ-A/OBJ-B selection and 8-row chunk geometry agree with the reference implementation. This materially narrows future sprite work: after v0.35 validation, remaining defects should be investigated in pixel decode, palette/mixer behaviour, clipping, or hardware timing rather than changing those confirmed map partitions without new evidence.

## v0.35 — reusable sprite fault-isolation matrix

The workbench now exports enough competing interpretations to test future sprite faults without recompiling. `map_entries.csv` preserves spritemap word/byte offsets, native and byte-swapped codes, tile-count bounds, zero/FFFF markers and repeated-entry evidence. `zoom_boundaries.csv` records every integer chunk boundary and flags zero-sized chunks. Full OBJ-A/OBJ-B tile atlases isolate graphics-ROM decode from spritemap/object assembly. Optional alternate reconstructions deliberately try byte-swapped map codes and the opposite decoded graphics bank; these are diagnostic counterfactuals only, not promoted hardware semantics. Per-sprite comparison PNGs make the fault boundary externally inspectable.


## v0.35 — sprite exporter correction and MAME parity audit

The v0.34 capture exposed a debugger-side arithmetic error: a 16x16x4 sprite tile is 1024 **bits** (128 bytes), but the exporter divided the 0x200000-byte OBJ region by 1024 bytes and therefore reported only 2048 tiles instead of the correct 16384. Consequently v0.34 falsely marked normal spritemap codes such as $3708 as out of range and omitted them from `assembled_unscaled.png`. v0.35 uses `(bytes*8)/1024`, records both raw and hardware-effective `code & $3fff`, and exports the full 16384-tile OBJ A/B atlases.

A line-by-line audit against MAME's `chasehq_draw_sprites_16x16` also corrected the Chase H.Q. sprite vertical offset from +3 to +7 and preserves the documented hardware behaviour where upper spritemap-code bits are discarded (`code & $3fff`). A raw `$ffff` map word therefore resolves to tile `$3fff` rather than being silently skipped; MAME notes this behaviour is used by mask sprites. Sprite RAM traversal was already back-to-front and the 128x128 / 64x128 / 32x128 map formulas and integer zoom boundaries already matched the reference implementation.


## v0.36 — final-video evidence chain

The v0.35 frame-1700 exports established that current OBJ-A/OBJ-B selection, spritemap byte order, effective `code & $3fff` tile addressing and logical object reconstruction can produce coherent cars, trees and roadside objects. v0.36 therefore moves the main fault-isolation boundary downstream.

For each requested frame the frontend now records: source-layer renders, incremental composition, current PROM-mask output, a no-PROM legacy comparison, PROM address/output visualisations, final sprite-owner pixels, sprite overlap density and per-slot screen placement. Per-slot analysis separates (a) candidate non-transparent pixels, (b) pixels accepted against the pre-sprite road/BG layer mask, (c) pixels blocked by those layers, (d) pixels remaining as final sprite owner and (e) pixels subsequently lost to sprite overlap. This is designed to distinguish placement/zoom faults from layer-mask/PROM faults without further instrumentation builds.

## v0.38 — priority, mask and palette laboratory
The v0.36 frame-1700 capture demonstrated coherent source sprites and highlighted strong pairwise overlap around the player-car region. v0.38 adds exact pair-overlap measurement, heuristic mask/control-object ranking, palette-bank forensics, complete effective PROM truth-table export, address-bit activity, per-frame PROM usage and controlled “suppress/force priority” comparison renders. No heuristic result is treated as hardware truth; raw sprite words, PROM bytes and palette entries remain the primary evidence.


## v0.38 — palette fallback removal and Chase H.Q. reference priority path

The v0.37 frame-1800 palette capture proved that the runtime was replacing many legitimate TC0110PCR `$0000` entries with synthetic debug colours. That made the player-car companion sprite (slot 44, palette bank 70) appear as a large green overlay and invalidated colour-based mask hypotheses. v0.38 removes that fallback from normal rendering and sprite export: `$0000` now decodes to black.

The same audit showed that the experimental PROM mask path was not rejecting sprite pixels against the scene in the established Chase H.Q. manner. v0.38 therefore adds a separate reference compositor based on the known Taito Z Chase H.Q. draw order and priority-bitmap model. The reference path uses upper BG priority value 1, TC0150ROD priorities 1/2, text priority 4, and sprite masks `0xf0`/`0xfc` selected by W1 bit 15. The PROM path is retained as an experimental comparison rather than silently treated as authoritative.

Evidence status: palette fallback bug **Confirmed**; Chase H.Q. sprite priority bit location **Confirmed**; reference priority-mask model **Reference parity implementation**; exact physical B52-01 PROM wiring remains **Unresolved/experimental**.


## v0.39 — forensic instrumentation
Added write-provenance for every TC0110PCR entry, per-sprite pen/palette correlation, focused slots 44/45 experiments, bottom-screen source comparison, and priority-class histograms. This is designed to distinguish a genuine special sprite operation from an ordinary sprite rendered with incomplete palette/priority semantics.

## v0.40 — Parameterised graphics-forensics framework

The graphics investigation is now parameter-driven rather than build-driven. Sprite slots/pairs, TC0110PCR banks/entries/pens, PROM addresses/values, priority classes, individual pixels, screen regions and raw road-priority values can be selected at runtime. Presets and `key=value` config files allow repeatable investigations without recompilation. The first intended use is the confirmed slot-44/45 + palette-bank-70 case, while the same framework is designed to remain useful for later compositor and road-priority work.

This establishes a project-wide RE rule: future input, audio and timing investigations should receive similarly targetable diagnostics before resorting to repeated bespoke builds.

## v0.41 methodology milestone
The proven Stage 1 IOC pulse sequence is now a named scenario (`stage1-gameplay`). Graphics trace presets are intentionally separate from game-state setup. v0.41 also adds subsystem event categories, per-frame IOC capture, mutable hardware-state snapshots and self-describing run manifests so evidence can be reproduced and compared across builds.

## v0.42 provenance milestone

The debugging workflow is now address-centric rather than PC-range-centric. `--follow-address` records every selected read/write with the executing PC, dynamically reconstructed current function and call chain. This is intended to replace repeated manual traces used during the slot 44 investigation.

Latest confirmed player-car chain retained from v0.41 investigations:
- shared object base `$10A040` is initialised at frame 1623;
- `$007BA2` writes literal `$0050` to `$10A04A`;
- `$007BA8` writes literal `$2000` to `$10A068`;
- `$042C72` copies `$10A04A` to `$10A076`;
- `$04703A` consumes `$10A076` in the deliberate secondary player-car render path;
- `$047082 ORI.W #$2300,D6` explicitly selects the slot-44 attribute/palette bits;
- slot 44 and slot 45 are independently generated then copied from the software sprite staging table to hardware sprite RAM.

Rejected explanations remain: stale hardware sprite RAM, boot leftovers, transfer corruption, and accidental duplication.


## v0.43 compositor provenance milestone
The CPU-side slot-44/45 chain is considered established through hardware sprite RAM. v0.43 therefore traces the final renderer decision instead of adding more game-code PC windows. Pair 44/45 pixel provenance records source pen, palette raw value, reference primask decision and final winner so the remaining black-overlay issue can be localized to sprite decode/palette/priority/compositor semantics.

## v0.44 accumulated evidence from v0.43 graphics forensics

Confirmed: player-car slot 44 is deliberately constructed by the game, copied unchanged through software staging to hardware sprite RAM, and is a coherent companion layer to slot 45. At frame 1800 slot 44 uses palette bank 70 and source pen 4 exclusively for its 1024 visible source pixels; palette entry `$0464` is `$0000`. Bank 70 is static in the observed gameplay interval, while slot 45 / bank 64 receives live palette updates. Rendering slot-44 zero-valued palette pixels as transparent is visually equivalent to suppressing slot 44 for the sampled frame, but this remains an experimental hypothesis rather than a default hardware rule.

Confirmed diagnostic issue from v0.43: explicit pixel/region selections did not consistently control final-pixel provenance, and PROM-address reporting could disagree between frame-level and targeted paths. v0.44 fixes the former and makes further PROM-wiring verification a first-class diagnostic target. Exact B52-01 physical input wiring remains unresolved/experimental.

## v0.47 repeated-investigation workflow
Known gameplay states can now be reached more efficiently with memory patching, relative captures and lightweight checkpoints. Use event-triggered captures to anchor evidence to state changes instead of guessing absolute frame numbers. Bundles now validate artifacts and avoid PowerShell ZIP sharing failures.

## v0.48.0 interactive investigation workflow

The SDL frontend can now be used as a live inspection console. F1 exposes known machine/game state while the emulator runs continuously. The first state-aware variable is the confirmed race timer (`$100200` packed BCD, `$100201` fractional). Debugger writes/freezes are explicit interventions and must not be mistaken for native game behaviour in evidence reports.

Current sprite RAM is decoded every rendered frame for the interactive object list. Mouse selection uses the final sprite-owner bitmap first, then bounding-box fallback, so a visually clicked object can be connected directly to its RAM slot/map/palette/priority. This is intended to shorten the path from visual anomaly to focused reproducible CLI experiment.
