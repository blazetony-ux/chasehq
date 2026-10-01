
## v0.45 multi-pixel and regression-set sampling

- `--gfx-trace-pixels LIST` adds multiple exact screen pixels in one argument. Both `99:165,150:224` and tuple-like forms are accepted.
- `--pixel-provenance-pixels LIST` does the same while enabling causal pixel provenance.
- `--gfx-trace-regression-set known-bad` loads the current Chase H.Q. compositor regression coordinates `(99,165)`, `(150,224)`, `(122,239)`, `(197,239)`.
- Explicit pixel lists are never silently truncated by `--pixel-provenance-max`; that option caps only automatic sampling.
- Config-file aliases are `pixels=...`, `pixel_provenance_pixels=...`, and `regression_set=known-bad`.

# Parameterised Graphics Tracing (v0.40)

v0.40 applies the generic memory tracer philosophy to graphics reverse engineering: one build, reusable targeted filters, presets and config files, with machine-readable evidence instead of repeated source changes.

## CLI filters

All `--gfx-trace-*` filters are repeatable unless they represent a single scalar value.

- `--gfx-trace-slot N` — logical sprite slot 0..255.
- `--gfx-trace-pair A:B` — focus a sprite interaction pair.
- `--gfx-trace-palette-bank N` — TC0110PCR palette bank 0..255.
- `--gfx-trace-palette-entry N` — exact palette entry 0..4095.
- `--gfx-trace-pen N` — source pen 0..15.
- `--gfx-trace-prom-addr A[:B]` — PROM address/range, hexadecimal 00..ff.
- `--gfx-trace-prom-value HEX` — exact PROM output value.
- `--gfx-trace-priority N` — decoded reference-priority value.
- `--gfx-trace-pixel X:Y` — exact output pixel.
- `--gfx-trace-region X1:Y1:X2:Y2` — rectangular screen area.
- `--gfx-trace-layer road|bg0|bg1|sprite|text` — layer focus metadata; repeatable.
- `--gfx-trace-road-priority N` — raw road probe/priority value.
- `--gfx-trace-zero-palette` — include uses of hardware palette value `$0000`.
- `--gfx-trace-anomalies` — restrict the targeted per-pixel file to suspicious conditions.
- `--gfx-trace-from-frame N` / `--gfx-trace-to-frame N` — trace window.
- `--gfx-trace-max N` — targeted CSV line cap.
- `--gfx-trace-preset NAME` — predefined filter sets.
- `--gfx-trace-config FILE` — load `key=value` settings.

## Presets

- `car`: slots 44/45, palette banks 64/70, entry 1124, car-area region and zero-palette investigation.
- `palette`: active player-car palette banks and zero-palette analysis.
- `priority`: all major layer classes with anomaly focus.
- `prom`: complete PROM address range.
- `road-priority`: road-layer anomaly investigation.
- `compositor`: all major layers over the lower half of the screen.

## Config file format

Example `graphics_trace.cfg`:

```ini
preset=car
from_frame=1690
to_frame=1800
slot=44
slot=45
pair=44:45
palette_bank=64
palette_bank=70
palette_entry=1124
pen=4
region=64:150:255:239
prom_addr=00:ff
zero_palette=true
anomalies=true
max=100000
```

Repeat keys to add more filters.

## Output

At each requested graphics diagnostic frame the parameterised tracer writes:

- `graphics_trace_config.txt` — resolved active filter set.
- `targeted_pixel_pipeline.csv` — filtered pixel-level compositor state.
- `targeted_palette.csv` — selected palette entries plus write provenance.
- `targeted_sprite_slots.csv` — selected logical sprite state.
- `targeted_prom_usage.csv` — selected PROM addresses/outputs and usage counts.
- `targeted_anomalies.csv` — overlap and road-priority-collapse candidates.
- `targeted_trace_summary.txt` — compact counts and line-cap status.

These outputs accompany the existing full v0.39/v0.38 compositor, sprite, palette and PROM diagnostic bundle.

## Design principle for future subsystems

The same architecture should be used for input/control, audio, sound-CPU, timing, device communication and later hardware RE: reusable CLI filters, presets, config files, frame/time windows, event caps, provenance, anomaly detection and machine-readable export before adding one-off source-level diagnostics.

## v0.44 controlled compositor experiments

The following switches are diagnostic only and never alter the default gameplay renderer:

- `--experiment-slot-zero-transparent N`
- `--experiment-palette-zero-transparent N`
- `--experiment-sprite-pair-order A:B`
- `--experiment-sprite-order ascending|descending`
- `--experiment-layer-order bottom,upper,road,sprites,text`
- `--experiment-layer-matrix`

Experiment images and `experiment_comparison.csv` are written to `graphics_frame_N/experiments_v044/`. The comparison CSV records whole-frame changed-pixel counts and changed pixels within any requested pixel/region scope.

Explicit pixel/region requests now take precedence over preset auto-selection. Pair auto-selection uses high-information overlap ranking and emits `interesting_pixels.csv`.


## v0.51.2 layer-position experiments

Use repeatable `--layer-offset LAYER:X:Y` for presentation-only alignment experiments. Targets: `bg0`, `bg1`, `text`, `sprites`, `road`, `all`. Negative Y moves the rendered layer upward. Values are recorded in run metadata and may also be supplied in a master debug config as `layer_offset=...`.

RC2.4 defaults BG0/BG1 to `0:0`. Earlier negative BG defaults were evidence-era presentation compensation for a TC0100SCN Y-scroll sign error that is now corrected in the renderer. Explicit offsets remain valid for controlled geometry hypotheses and are still presentation-only.
