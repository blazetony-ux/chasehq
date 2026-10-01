# Chase H.Q. Native Semantic Memory Map — v0.28

Confidence: **Confirmed** = demonstrated by trace/disassembly; **Probable** = strong behavioral evidence; **Hypothesis** = working theory; **Rejected** = disproved lead retained for history.

| Address / range | CPU | Type | Meaning | Confidence | Evidence / notes |
|---|---|---|---|---|---|
| `$10801A` | A/B shared | W/R word | CPU-A to CPU-B operating-mode selector | Confirmed | `0000` causes CPU-B IRQ to request `$0004`; `FFFF` causes persistent `$8002`. CPU A writes `FFFF` at `$4820`. |
| `$100802` | B | word | Dispatcher command/mode | Confirmed | Low 5 bits index table at `$052E`; bit 15 makes command persistent. `$8002` selects entry 2. |
| `$100804` | B | word | IRQ wake/strobe | Confirmed | IRQ writes `FFFF`; main loop waits then clears it. |
| `$100806` | B | word | Copy of current dispatcher command | Confirmed | Written immediately before dispatch. |
| `$100808` | B | word | IRQ counter | Confirmed | Incremented in IRQ and mirrored to shared `$1080A6`. |
| `$10080A` | B | word | Main-loop dispatch counter | Confirmed | Incremented after wake. |
| `$10080E` | B | long | Current live progression/position value imported from `$10A000` | Probable | Previous value saved to `$10222C`; nonnegative delta calculated at `$102CE4`. Exact gameplay unit TBD. |
| `$1011E2...` | B | word table | Road body X/control source table | Confirmed for data flow | `$4188` masks each word with `$07FF`, ORs `$8800`, and emits it as road record W2. |
| `$101384...` | B | word table | Road colour/tile source table | Confirmed for data flow | Emitted unchanged as road record W3. |
| `$101548...` | B | word table | Road left clip/control source table | Confirmed for data flow | Emitted unchanged as road record W1. |
| `$101618...` | B | word table | Road right clip/control source table | Confirmed for data flow | Emitted unchanged as road record W0. |
| `$101A58` | B | word | Pending TC0150ROD control value | Probable | Generated after live road processing; copied to `$801FFE` when strobe is set. |
| `$101A5A` | B | word | Pending-road-control ready/strobe | Probable | Set `FFFF` after generation, cleared after `$101A58` is copied to `$801FFE`. |
| `$101A5C` | B | word | Road RAM ping-pong selector | Confirmed | Toggles each generated frame; correlates with `$800000/$800800` bank selection. |
| `$101A5E` | B | long | Current road-buffer end pointer | Confirmed | Alternates `$8007F8/$800FF8`; generated records end exactly at this boundary. |
| `$800000-$801FFF` | B | TC0150ROD RAM | Road generator RAM | Confirmed | CPU B writes; no CPU A/B reads observed after generation. Hardware/rendering consumer. |
| `$800000-$8007FF` | B | road bank | 256 x 8-byte interleaved scanline records | Confirmed | One ping-pong bank. |
| `$800800-$800FFF` | B | road bank | 256 x 8-byte interleaved scanline records | Confirmed | Alternate ping-pong bank. |
| `$801FFE` | B | word | TC0150ROD road control / bank select | Confirmed | Receives pending `$101A58`; low byte also controls priority switch line in established TC0150ROD behavior. |

## TC0150ROD 8-byte scanline record

Each record is four big-endian words in CPU-visible road RAM:

| Word | Meaning | Confidence |
|---|---|---|
| W0 | Right road edge clip/control | Confirmed |
| W1 | Left road edge clip/control | Confirmed |
| W2 | Road body X offset/control | Confirmed |
| W3 | Colour bank + road-gfx tile number | Confirmed |

For the frame-1627 live writer, all 97 observed records matched: `W0=$101618[i]`, `W1=$101548[i]`, `W2=($1011E2[i]&$07FF)|$8800`, `W3=$101384[i]`.

## Important rejected interpretations

- `$100040` first word is **not** semantic gameplay state; it is scheduler delay/countdown.
- `$10801A` is **not** a numeric road command; it selects CPU-B operating mode (`0000` init/clear, `FFFF` live persistent processing).
- `$101A5A` is **not** itself a road command; it behaves as the pending-control completion/ready strobe.

### v0.30 frontend diagnostic watch set
The graphical frontend now snapshots these already-established locations from its own live Runtime instance: `$10801A` (A->B live-mode handshake), `$100802/$100804` (CPU-B command/wake), `$101A5A/$101A5C/$101A5E` (road pending/bank/boundary), and `$801FFE` (TC0150ROD control). This adds observability, not new semantic claims.


### v0.31 execution note
`ChaseHQNative` now issues IRQ4 once per emulated frontend frame by default. This is an execution/timing requirement rather than a new memory-map register. The v0.30 no-IRQ run demonstrated that CPU execution alone does not advance the normal game state into the live road/sprite scene.

### Sprite diagnostics (v0.32)
- `$D00000-$D007FF` — sprite RAM, 8-byte logical entries in the current renderer. v0.32 can snapshot the raw region and decode each entry without modifying emulation state.
- Each current logical entry is interpreted as four big-endian words W0..W3; v0.32 reports the raw words before applying any interpretation.
- B52-38 spritemap lookups now log both the current little-endian word interpretation and its byte-swapped candidate. This is diagnostic evidence only; no endian change is asserted yet.
- Confidence remains **Probable** for several sprite attribute bit meanings until correlated against known-good imagery/reference behaviour.

### v0.35 sprite evidence capture

`$D00000-$D007FF` remains the authoritative raw sprite-RAM source for the graphics workbench. v0.35 can export the raw RAM, decoded logical sprite attributes, B52-38 spritemap words, referenced graphics tiles, pre-zoom object assembly and post-zoom object assembly from the same frame. These exports are evidence captures of the *current interpretation*; they do not promote still-unverified sprite field semantics to Confirmed merely because the exporter can visualise them.

### Chase H.Q. sprite RAM `$D00000-$D007FF` — v0.35 corrected decode

| Entry word | Meaning | Confidence |
|---|---|---|
| +0 / W0 | bits 15..9 Zoom Y; bits 8..0 Y | Confirmed (reference + live RAM) |
| +2 / W1 | bit 15 priority; bits 14..7 palette bank; bits 6..0 Zoom X | Confirmed (reference) |
| +4 / W2 | bit 15 Flip Y; bit 14 Flip X; bits 8..0 X | Confirmed (reference) |
| +6 / W3 | bits 10..0 spritemap/object number | Confirmed (reference) |

Sprite list consumption is back-to-front. Object format remains selected from Zoom X: 128x128 uses OBJ A / spritemap `$00000-$3ffff`; 64x128 uses OBJ B / `$40000-$5ffff`; 32x128 uses OBJ B / `$60000-$7ffff` (byte ranges; code stores word offsets `$00000/$20000/$30000`).

### Diagnostic export fields
For each referenced B52-38 spritemap entry v0.35 records the word index, corresponding byte offset, native 16-bit code, byte-swapped comparison code, decoded graphics-bank tile count, in-range status, zero/FFFF markers and repeated-entry status. These fields are evidence and do not themselves change confidence levels of hardware semantics.


### Sprite spritemap code semantics (v0.35)
- B52-38 spritemap entries are 16-bit little-endian words.
- Effective graphics tile index: `raw_code & $3fff` (Confirmed against MAME reference behaviour).
- Each OBJ graphics region is 0x200000 bytes = 16384 16x16x4 tiles at 128 bytes/tile.
- Chase H.Q. sprite Y offset is +7 before `(128-zoomY)` anchoring.


### v0.36 video-pipeline observation note
No new emulated memory-map semantics are asserted by v0.36. The new evidence maps are renderer-side observations derived from already-mapped TC0100SCN, TC0150ROD, sprite RAM `$D00000-$D007FF`, palette RAM and B52 priority PROMs. `sprite_screen_placement.csv` always preserves the raw W0..W3 source words beside calculated coordinates and PROM decisions so renderer conclusions remain traceable back to hardware-visible state.

### v0.38 video evidence note
No new emulated address meaning is asserted by the v0.38 diagnostics. The workbench exports TC0110PCR palette entries, sprite RAM, TC0100SCN control words, TC0150ROD state and effective PROM tables side-by-side so renderer inferences can be traced back to hardware-visible values. Mask-candidate scores and PROM `block_mask` decoding remain analysis-layer hypotheses until independently proven.


### v0.38 video-priority semantics

Sprite RAM W1 bit 15 selects one of two reference priority masks (`0xf0` / `0xfc`). TC0110PCR raw value `$0000` is rendered as black; no normal-runtime debug colour substitution remains. These are renderer semantics, not new RAM addresses.

## v0.42 retained semantic discoveries

| Address | CPU | Access | Meaning | Confidence | Evidence |
|---|---|---|---|---|---|
| `$10A040` | A/B shared | R/W | player/car shared object-state block base used by Stage 1 setup/render path | Probable | initialised by `$007Bxx`, consumed by `$042Cxx/$046Dxx/$0470xx` |
| `$10A04A` | A/B shared | R/W | object `+$0A`, literal `$0050` initial render/config field | Confirmed | `$007BA2 MOVE.W #$50,($A,A3)` |
| `$10A068` | A/B shared | R/W | object `+$28`, literal `$2000`, consumed by primary car sprite construction | Confirmed | `$007BA8`, later read by `$046DA6` |
| `$10A070` | A/B shared | R/W | object `+$30`, table-derived byte (`$FF` at Stage 1 activation) | Confirmed value/path, semantic role unknown | `$042C86` |
| `$10A076` | A/B shared | R/W | object `+$36`, copy of `$10A04A`, consumed by secondary car component | Confirmed | `$042C72`, `$04703A` |
| `$101858` | A work | R/W | literal `$2380` state/control field set during gameplay activation | Confirmed value, role unknown | `$040B12` |
| `$102688` | A work | R/W | probable base of 8-byte software sprite staging table | Probable | slot offset mapping to `$D00000` copy routine |


## v0.43 retained high-confidence sprite path
- `$1027E8-$1027EF` — slot 44 software staging entry; generated by `$0470AC/$0470AE`; **Confirmed**.
- `$1027F0-$1027F7` — slot 45 primary-car staging entry; generated by `$046DE8/$046DEA`; **Confirmed**.
- `$D00160-$D00167` — slot 44 hardware sprite RAM destination, direct copy from staging by `$04056A/$04056C`; **Confirmed**.
- slot 44 palette bank 70/pen 4 entry `$0464` is deliberately written `$0000`; global zero-colour transparency is **Rejected**.

## v0.44 evidence note - player-car sprite pair

No address-map changes. The following evidence is preserved for diagnostics: slot 44 staging `$1027E8-$1027EF` is generated by `$0470AC/$0470AE` and copied to hardware sprite RAM `$D00160-$D00167` by `$04056A/$04056C`; slot 45 staging `$1027F0-$1027F7` is generated by `$046DE8/$046DEA`. The slot-44 record at the investigated stage is `FEB2 237F 0060 0252`; slot 45 is `FEB2 207F 0060 0204`.

## v0.47 workflow note
The confirmed Stage 1 BCD race countdown at CPU-A $100200 can be used with `--capture-on-change`, `--capture-on-value`, patch-relative capture and checkpoint-assisted investigations. This does not change its existing Confirmed classification or the documented natural timeout path.


## Evidence-backed gameplay state (v0.51.2 documentation update)

| CPU A address | Width/encoding | Meaning | Evidence note |
|---|---:|---|---|
| `0x100400` | 16-bit packed BCD | Display speed km/h | writer PC `0x0087BA`, function `0x00876A` |
| `0x100408` | 32-bit packed BCD payload | Score | writer PC `0x0082AA` |
| `0x10041C` | 16-bit raw | Internal speed/physics state | writer PC `0x008700` |
| `0x100212` | 16-bit boolean (`0/FFFF`) | Turbo active | observed exact activation/deactivation windows |
| `0x1003A2` | 16-bit count | Turbos remaining | observed `3 -> 2 -> 1` across two activations |
| `0x102FC0` | 32-bit 24.8 fixed point | Distance | divide by 256; writer PC `0x00108A` |

Player-car brake lamps are currently best observed from palette state rather than a proven dedicated gameplay register: entries `0x40D/0x41D`, pen 13, change from dark red (`~0x000E`) to bright red (`~0x18BE`) while braking.

On-road/off-road state is not yet identified. `0x101A58/0x101A5A` did not prove to be player road-contact state in the attract/driving experiment.
