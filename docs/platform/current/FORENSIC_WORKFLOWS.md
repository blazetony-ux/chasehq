# v0.46 quick workflows

### Compositor known-bad matrix
`--investigate compositor-known-bad` combines the four established regression pixels with zero-palette, pair-order and primask-matrix experiments in one auto-zipped run. Inspect `experiments_v046/primask_matrix.csv`, `primask_matrix_summary.txt`, `experiment_pairwise_diff.csv` and the per-pixel provenance outputs.

### Compact memory watch
Use `--watch-address A:byte:100200:race_timer` for known state variables. It emits only first value + changes to `memory_watch.csv`; use broad generic traces only when discovering the address itself.

### Legacy diagnostics
Legacy hard-coded logs are disabled by default in v0.46. Opt in with `--legacy-debug`.

# v0.43 forensic workflows

## Frame breakpoints
`--break-frame N` captures mutable hardware-visible state, writes `break_frame_N/recent_execution.csv`, produces the configured graphics diagnostics at N, and stops by default. `--break-frame-continue` continues automatically; `--break-frame-wait` waits for Enter then continues. `--break-history N` controls the rolling pre-break PC history.

## Focused provenance
Address following now keeps shadow stacks but avoids global callgraph/function counters unless requested. Use `--provenance-full-graph` when a complete graph/profile is useful and `--provenance-verbose-calls` to log every call event.

## Log bundles
`--auto-zip-logs` creates a portable ZIP bundle after all streams are closed. Names include version, scenario, trace focus and timestamp. `--zip-name NAME` overrides the filename; `--zip-delete-source` removes the raw log directory only after successful packaging.

## Unattended batch runner
`--batch FILE` reads one job per line using `name|arguments`. Each job receives its own log directory and ZIP; `batch_summary.csv` records exit status. Example:

```text
car1800|--scenario stage1-gameplay --frames 1800 --fast-forward-to 1600 --gfx-trace-preset car --graphics-debug-frame 1800 --no-legacy-debug
slot44|--scenario stage1-gameplay --frames 1800 --fast-forward-to 1600 --follow-address 1027e8:1027ef --provenance-from-frame 1620 --provenance-to-frame 1650 --no-legacy-debug
```


## CLI parity and evidence packaging (v0.49.1)

Every reproducible diagnostic capability should be invokable from CLI/config as well as the interactive UI. The UI is for exploration; repeatable investigations should be expressible as copy/paste commands. Evidence-heavy runs should use isolated per-run directories and `--evidence-bundle` / `--evidence-name` so all raw structured logs, metadata and supporting captures are delivered as one ZIP.

Debugger save states are no longer limited to one quick state: F1 enables the debug overlay, digits 0-9 select a persistent state slot, F5 saves the selected slot and F9 loads it. The directory defaults to `checkpoints/` and can be changed with `--checkpoint-dir`. The initial selected slot can be set with `--checkpoint-slot`. Exact checkpoint files remain loadable through `--load-checkpoint PATH`, and scheduled saves remain available through `--save-checkpoint FRAME:PATH`.

## v0.49.2 bounded-run robustness

The first Windows/SDL v0.49.1 evidence-bundle test exposed an important workflow failure: a checkpoint-loaded bounded run could return without producing the requested evidence ZIP. v0.49.2 makes the unattended path explicit and self-reporting:

- `--exit-at-frame N` is treated as an absolute emulated-frame target, including after `--load-checkpoint`.
- startup prints the restored start frame, absolute target, and number of frames remaining;
- bounded runs print periodic progress and a definitive `RUN TARGET REACHED` line;
- runtime faults terminate through the normal finalisation path instead of leaving a non-progressing run;
- every normal evidence run writes `run_status.json` with start/final/target frames, completion state and sprite-evidence file count;
- ZIP output reports its exact path and size;
- if an exception occurs after the evidence directory has been established, an emergency partial evidence bundle is attempted with `run_failure.txt` rather than silently losing the run.

This preserves the project rule that the preferred user workflow is: run one exact CLI command, allow it to terminate itself, then attach one generated ZIP.
