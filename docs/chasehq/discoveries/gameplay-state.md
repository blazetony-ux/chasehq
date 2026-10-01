# Discovery record — gameplay state

**Status:** mixed CONFIRMED / OPEN

## Outcome
Evidence-backed gameplay values currently include:

| State | Source | Encoding | Confidence |
|---|---|---|---|
| Display speed | CPU-A `0x100400` | 16-bit packed BCD | CONFIRMED |
| Internal speed / physics | CPU-A `0x10041C` | 16-bit raw | CONFIRMED |
| Score | CPU-A `0x100408` | 32-bit packed BCD payload | CONFIRMED |
| Distance | CPU-A `0x102FC0` | 32-bit 24.8 fixed point, divide by 256 | CONFIRMED |
| Road classification | CPU-A `0x10A048` | flags; bit `0x04` = OFF_ROAD, `(flags & 0x06)==0x02` = EDGE, otherwise INTERIOR | CONFIRMED |
| Road boundaries | CPU-A `0x10A05C` | two 16-bit wrap-safe lateral bounds | CONFIRMED |
| Player lateral coordinate | CPU-A `0x10A044` | 16-bit wrap-safe coordinate | CONFIRMED |
| Processed turbo input | CPU-A `0x100303` bit 0 | boolean bit | CONFIRMED |
| Turbo active | CPU-A `0x10040F` bit 1 | boolean bit | CONFIRMED |
| Turbos remaining | CPU-A `0x1003A2` | 16-bit count | CONFIRMED |
| Turbo active-duration counter | CPU-A `0x100414` | 16-bit frame counter | CONFIRMED |
| Turbo expiry threshold | `0x00D2` in the tested canonical Stage-1 configuration | 210 emulated frames | CONFIRMED for tested configuration |
| Brake-lamp visual state | palette entries `0x40D/0x41D`, pen 13 | dark/bright red transition | PROBABLE visual proxy |
| Raw steering input | CPU-A 0x10014C | IOC signed 12-bit / byte-swapped RAM representation | CONFIRMED |
| Processed steering | CPU-A 0x100300 | signed 16-bit; centre 0, left negative, right positive | CONFIRMED |
| Authoritative target health/damage | unresolved | — | OPEN |

## Current IOC P3 control map
- `0x01` — TURBO — CONFIRMED. The processed bit appears at `0x100303` bit 0; code at `0x00846E` tests it and `0x008486` sets bit 1 of `0x10040F`.
- `0x02` — triggers TILT — CONFIRMED. Do not over-name the physical cabinet signal beyond the observed game behaviour.
- `0x10` — BRAKE — HIGH CONFIDENCE from controlled accelerator+brake suppression.
- `0x20` — ACCELERATOR — CONFIRMED.
- `0x04`, `0x08`, `0x40`, `0x80` — no steering effect observed in the tested Stage-1 conditions; not proven globally unused.

## Turbo causal validation
From `stage1-gameplay-2064.chqstate`, turbo activation changed `0x1003A2` from 3 to 2, set `0x10040F` to `0x02`, and started `0x100414`. The counter then incremented exactly one tick per emulated frame. At `0x00D2` the active state expired. A second activation changed the remaining count from 2 to 1 and restarted the counter. This establishes the count/active/timer semantics directly rather than inferring them from HUD graphics.

## Important distinction
A displayed/HUD value is not automatically the authoritative gameplay value. Display speed and internal speed are deliberately documented separately. The Workbench Gameplay Registry exposes source/confidence so visual proxies do not silently become authoritative state.

## Next tests
Steering is confirmed and now bound through the authentic IOC path in v0.66.2. Continue target health/damage/defeat, end-level transition, palette/colour accuracy and track branch/topology research.

## v0.66.9.0-RC2 speed-control clarification
The Workbench Game Lab now displays both fields together. `0x100400` is the 16-bit packed-BCD HUD/display speed; `0x10041C` is the 16-bit raw internal/physics speed. Setting/freezing `0x10041C` is a physics experiment and is not expected to make the HUD show the same decimal number. The actual top-speed clamp/target remains unresolved.
