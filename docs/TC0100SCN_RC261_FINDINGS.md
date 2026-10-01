# RC2.6.1 TC0100SCN findings carried into RC2.7

The supplied RC2.6.1 research conversation is the provenance for runs 009–013 and the four CLI offset runs. Those external bundles were not independently re-read in this implementation session. Source inspection independently confirmed the renderer formula and the scene-no-sprites defect. Do not turn a supplied finding into a claim of newly executed evidence.

- Physical BG0 is Graphics Lab `bg-upper`; physical BG1 is `bg-bottom` at the investigated CTRL 6 state. Logical bottom/upper naming follows CTRL 6 order and is not universally identical to physical BG numbering.
- Visible raster rows sample rowscroll entry `visible Y + 16`, modulo 512. A single entry affects its corresponding visible scanline. Cloud occlusion hid the first top-band experiment.
- Global Y response at frames 2352–2364 is coherent. Preserve `hardware Y - signed CTRL_Y - 8`, with hardware Y = visible Y +16. No new Y-sign/origin correction or presentation offset is justified.
- Map 405 cloud geometry at 2352 was independently validated against raw sprite RAM in run 009. Slots 59–62: W0 B9A 7, W1 57FF, W3 0195; palette 175, priority 0, no flips, zoomX 128, zoomY 93. Sorted X positions -110,18,146,274 have 128-pixel spacing. Do not reposition these clouds.
- The horizon/building scenery is TC0100SCN tilemap content. Map 268/269 roadside lamp-post sprites are a separate pairing; moving building sprites cannot correct the tilemap scenery.
- Standard-width RAM is 64 KiB at C00000; control is 16 bytes at C20000. BG0 rowscroll offset C000, BG1 C400, BG1 column table E000–E0FF (absolute C0E 000).
- Run 012 exported complete RAM/control at 2352 and 2360. Column table was zero at both; this only rules it out at those states. Column scroll remains unsupported in RC2.7, is reported explicitly in snapshot metadata, and has a tested zero/nonzero detector.
- At 2352, BG0 entries 0–111 were zero; entries 112–139 formed a transition; 140–187 were 01DF; entry 188 was 01CB; later entries were zero. BG1 was 01F5 through 139,01E7 at 140–187,01D8 at 188, then zero. Run 013 inverted every active visible entry on a mutable fork and produced exact baseline/BG1-only/both captures.
- Supplied run 013 comparison: BG1-only changed 27,858 raw bg-bottom pixels and 7,380 final-no-HUD pixels; adding BG0 changed 8,984 raw bg-upper pixels and yielded 13,768 changed final-no-HUD pixels overall. These are historical supplied measurements, not new RC2.7 regression results.
- Source inspection confirmed old X = screen X + signed CTRL_X + signed ROWSCROLL +16. RC2.7 implements screen X - signed CTRL_X - signed ROWSCROLL +16, modulo 512, in the shared BG0/BG1 path and its trace. The supplied investigation compared this with MAME's TC0100SCN model. A fresh retrieval of upstream was unavailable here; no new upstream verification is claimed.
- Four supplied CLI offset runs recorded bg 0=[0,0], bg 1=[0,0] and identical functional outputs. Historical -4/-20 offsets were not active. RC2.7 keeps all defaults zero.
- Requested capture artifacts at the loaded checkpoint frame were missing. Source inspection found captures were checked after advancing; RC2.7 renders/captures the loaded checkpoint before advancement and asserts required artifacts before bundling.
- `scene-no-sprites.png` equalled final in run 013. Source inspection confirmed its base already contained sprites. RC2.7 now copies the authoritative compositor state before sprite paint and applies any subsequent text pass.

The new X mode is a same-renderer old/corrected diagnostic comparison, not an independent implementation and not SHADOW VERIFIED. A screenshot looking nicer is not hardware proof. Windows canonical captures, exact reconstruction and visual/structured review remain the release promotion gate.

Preserve the RC2.6.1 sprite decoder: physical planes 0,1,2,3 map to pen bits 3,2,1,0. Keep all 16 pen tests and the permanent versioned sprite regression. No priority, palette, cloud, building-sprite or Y workaround is included.
