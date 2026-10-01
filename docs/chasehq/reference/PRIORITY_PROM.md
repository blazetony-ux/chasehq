# Priority PROM reverse-engineering reference

## Purpose
This file is the maintained evidence notebook for the B52 priority PROM path. v0.38 does **not** assert that the current address wiring or output-bit interpretation is final. The workbench therefore exports the raw 256-entry tables, exact per-frame addresses used, raw outputs, address-bit activity and the current candidate `block_mask` separately.

## v0.38 evidence products
- `prom_truth_tables.csv`: all 256 addresses for the sprite/mix and road tables.
- `prom_mix_effective.bin`, `prom_road_effective.bin`: exact bytes used by the native lookup tables for external comparison.
- `prom_frame_usage.csv`: addresses actually exercised by Road A, Road B and sprites in a captured frame.
- `prom_bit_activity.csv`: high/toggle counts for each candidate address bit. A bit that never varies is immediately visible.
- `31_prom_address_bits.png`, `32_prom_address_output.png`: spatial views of the chosen address and raw output.
- `34_prom_vs_legacy_difference.png`: where current PROM-mask composition differs from the legacy approximation.

## Current interpretation status
The sprite-side PROM output is currently expanded into blocking classes for bottom BG, Road A, Road B and upper BG. That interpretation remains **Hypothesis/Probable**, not Confirmed. Raw PROM values must be treated as the authoritative evidence until address pins and output semantics are proven.

## Investigation method
1. Identify the exact sprite pixel and logical slot.
2. Read its `prom_addr`, `prom_out`, source and layer mask from `pixel_pipeline.csv`.
3. Cross-check the address in `prom_truth_tables.csv`.
4. Inspect which candidate address bits are actually varying in `prom_bit_activity.csv`.
5. Compare normal, forced-priority and suppression experiment images.
6. Promote a signal interpretation to Confirmed only when it agrees across multiple objects/frames and with hardware/reference evidence.


## v0.38 interpretation boundary

The B52 PROM dumps and truth-table experiments remain valuable reverse-engineering evidence, but they are no longer the default final compositor. The Chase H.Q. reference renderer provides a known behavioural baseline using the priority bitmap and sprite masks `0xf0`/`0xfc`. `--mixer prom` remains available so PROM hypotheses can be compared against that baseline without rebuilding.
