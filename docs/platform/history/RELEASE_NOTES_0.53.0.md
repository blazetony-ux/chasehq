# Chase H.Q. Native v0.53.0 — Steering + TC0100SCN Diagnostics

v0.53.0 is the next meaningful diagnostic build after the v0.52.0 experiment platform.

## Added
- Deterministic IOC steering: `--steering VALUE`, repeatable `--steering-at FRAME:VALUE`, and `--steering-range START:END:VALUE`.
- Steering is applied through the same IOC steering register consumed at ports 0x0c/0x0d and is checkpoint-compatible.
- `--gameplay-state-log` writes `gameplay_state.csv` with the known authoritative state: displayed speed (0x100400 BCD), internal speed (0x10041c), distance (0x102fc0 24.8), timer, score (0x100408 BCD), IOC steering, processed steering (0x100300), accelerator/brake IOC state, turbo active (0x100212), turbos left (0x1003a2), and both candidate player-body brake-lamp palette entries (bank 64/65 pen 13). ROAD/SURFACE remain deliberately unlabelled until proven.
- TC0100SCN geometry tracer: raw ctrl0..7, decoded X/Y scroll, rowscroll address/value, screen/hardware/source coordinates, tile coordinates, flip state, and diagnostic presentation offsets by frame/scanline.
- TC0100SCN trace controls: `--tc0100scn-trace`, `--tc0100scn-trace-from`, `--tc0100scn-trace-to`, `--tc0100scn-scanlines FROM:TO[:STEP]`.

## Defaults promoted from evidence
- Equal-priority sprite-vs-sprite overlap now defaults to `higher-slot`; `--sprite-tie-break lower-slot` remains available for controlled comparison.
- BG0 diagnostic presentation candidate changes from `(0,-16)` to `(0,-4)` after the 25-case refinement and moving-frame validation. BG1 remains `(0,-20)`. These are evidence-backed diagnostic presentation values, **not hardware truth**; the new TC0100SCN tracer exists to explain/remove the need for them.
- Sprite presentation offset remains `(0,0)`; the building displacement was proven to be TC0100SCN background content, not a global sprite-positioning error.

## Process/tooling findings carried forward
- Multi-run experiments should produce structured per-run evidence and a final ZIP automatically.
- Packaging must only report success after compression itself succeeds and the ZIP exists.
- Emulator shutdown must flush/close diagnostic streams before external packaging; v0.52.0 exposed a Windows file-lock race on `targeted_palette.csv`.
- When the next development step is a run, provide the exact one-line PowerShell command immediately rather than announcing that a command will follow.

## Still outstanding
- Authoritative ON-ROAD/OFF-ROAD and SURFACE discovery (now unblocked by deterministic steering).
- Full uncropped 262-line hardware-raster diagnostic export.
- Region/pixel -> source object/tile -> RAM -> writer provenance improvements.
- Remaining debugger defect batch: exit-frame boundary, restored-start screenshot scheduling, restored-run performance accounting, stale IOC version text, run-phase final state, provenance summary counts, and explicit event-time vs frame-boundary semantics.
- Audio (Z80/TC0140SYT/YM2610), scheduler/timing, checkpoint v2, rewind/goto/session/external studio and performance optimisation remain later work.
