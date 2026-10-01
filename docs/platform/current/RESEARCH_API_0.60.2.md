# Chase H.Q. Native v0.60.2 — Expanded Live Research API

v0.60.2 turns the v0.60.1 localhost API into a broader live reverse-engineering control surface. The design goal is to keep the emulator running while an investigator pastes short `chqctl` commands into a second console to observe, pause, alter, trace, compare and repeat.

The API remains bound to `127.0.0.1` by default and shares the same Research Workbench state as F12. It supplements, rather than removes, the launch-time diagnostics.

## New in v0.60.2

### Named snapshots

Keep multiple live experiment baselines instead of one global pause point:

```powershell
.\chqctl.exe snapshot save pre-hit
.\chqctl.exe snapshot save candidate-a
.\chqctl.exe snapshot list
.\chqctl.exe snapshot restore pre-hit
.\chqctl.exe snapshot remove candidate-a
```

### Live 68000 registers

```powershell
.\chqctl.exe regs A
.\chqctl.exe regs B D0
.\chqctl.exe reg write A D0 12345678
.\chqctl.exe reg write A PC 0077BE
```

Supported names: `D0-D7`, `A0-A7`, `PC`, `SR`, `SP`, `USP`, `ISP`.

Register writes pause controlled execution first and update the stored Musashi CPU context directly.

### Range watches with stable IDs

```powershell
.\chqctl.exe watch add A 10A080:10A0BF write change break
.\chqctl.exe watch add A 100200 16 rw
.\chqctl.exe watch list
.\chqctl.exe watch remove 3
```

A watch may cover one access width or an arbitrary inclusive address range. IDs remain stable until removal/clear.

### Reversible patch management

```powershell
.\chqctl.exe patch freeze A 10A096 16 01AA
.\chqctl.exe patch replace A 10A096 16 01AA
.\chqctl.exe patch suppress A 10A096 16
.\chqctl.exe patch list
.\chqctl.exe patch remove 2
```

`freeze` immediately writes the selected value and substitutes it on later matching writes. `replace` leaves the current value alone but substitutes the selected value on future matching writes. `suppress` discards matching writes and preserves the current value.

### Query the recent causal evidence

```powershell
.\chqctl.exe why mem A 10A096
.\chqctl.exe changed A 100000:1004FF since 9500
```

`why mem` reports the current byte, most recent writer/intervention in the Research Event buffer, and recent readers if they were watched.

`changed` collapses the recent event buffer to the latest changed write per address in a selected range.

### Configurable event history

```powershell
.\chqctl.exe events limit
.\chqctl.exe events limit 50000
```

The Research Event ring is configurable from 64 to 1,000,000 entries. Use larger histories selectively because reads over broad watched ranges can generate substantial event volume.

### IOC/input inspection and overrides

```powershell
.\chqctl.exe input ports
.\chqctl.exe input steering
.\chqctl.exe input steering set 0800
.\chqctl.exe input xor get 3
.\chqctl.exe input xor set 3 20
.\chqctl.exe input xor clear
```

This exposes the existing generic IOC steering and XOR-mask injection layer through the live API, preparing the Workbench for deterministic control experiments and replay tooling.

## Live reconfiguration of the heavyweight generic tracer

The old launch-time generic tracer is still available. v0.60.2 also exposes a live configuration path so it can be armed after the game has already reached an interesting state.

Example:

```powershell
.\chqctl.exe legacytrace reset
.\chqctl.exe legacytrace cpu A
.\chqctl.exe legacytrace mem 10A080:10A0BF w change
.\chqctl.exe legacytrace pc 007700:007850
.\chqctl.exe legacytrace trigger 10A096 change
.\chqctl.exe legacytrace context 256 512
.\chqctl.exe legacytrace max 200000
.\chqctl.exe legacytrace output .\evidence\target-live-trace.log
.\chqctl.exe legacytrace start
```

Available controls:

```text
legacytrace status
legacytrace reset
legacytrace mem START:END [r|w|rw] [change] [nonzero]
legacytrace pc START:END
legacytrace trigger START:END [write|change|nonzero]
legacytrace cpu A|B|both
legacytrace window FROM TO
legacytrace context BEFORE AFTER
legacytrace max LINES
legacytrace output PATH
legacytrace start [PATH]
legacytrace stop
```

`stop` closes the active heavyweight trace while retaining the API-side configuration. `reset` clears the configuration.

## Live provenance configuration

The established execution-provenance engine can now be configured and started while the game is already running:

```powershell
.\chqctl.exe provenance reset
.\chqctl.exe provenance follow 10A080:10A0BF
.\chqctl.exe provenance access rw
.\chqctl.exe provenance callstack on
.\chqctl.exe provenance callgraph on
.\chqctl.exe provenance max 250000
.\chqctl.exe provenance output .\evidence\target-provenance
.\chqctl.exe provenance start
```

Controls:

```text
provenance status
provenance reset
provenance follow START:END
provenance access r|w|rw
provenance window FROM TO
provenance max EVENTS
provenance callstack on|off
provenance callgraph on|off
provenance profile on|off
provenance output DIR
provenance start [DIR]
provenance stop
```

This makes the richer historical forensic tools available without relaunching ChaseHQNative.

## Synchronous command-line experiments

`run N` is asynchronous at the server boundary: the emulator must return to its frame loop to execute those frames. v0.60.2 therefore adds client-side `--wait`:

```powershell
.\chqctl.exe --wait run 30
```

The client polls `status` until `step_remaining=0`. This is the recommended form for copied command chains where the next command depends on the requested frames having actually completed.

## Script files

`chqctl` can execute a reusable command file:

```powershell
.\chqctl.exe script .\experiments\target-motion.chqscript
```

Scripts wait for controlled `run` / `step frame` commands by default. Blank lines and lines starting with `#` or `;` are ignored.

The interactive shell also supports:

```text
CHQ> source .\experiments\target-motion.chqscript
```

This is the first lightweight experiment-file mechanism; later versions can add assertions, branching and structured A/B/N trial metadata without changing the underlying API.

## Recommended live workflow

```powershell
.\chqctl.exe pause
.\chqctl.exe snapshot save baseline
.\chqctl.exe watch add A 10A080:10A0BF write change
.\chqctl.exe events limit 50000
.\chqctl.exe patch replace A 10A096 16 01AA
.\chqctl.exe --wait run 30
.\chqctl.exe changed A 10A080:10A0BF
.\chqctl.exe why mem A 10A096
.\chqctl.exe snapshot restore baseline
```

This is intentionally generic. The same loop can be applied to gameplay RAM, sprite RAM, input state, later audio/device adapters, course state, collisions or stage transitions.

## Compatibility

All established launch-time CLI diagnostics remain available. v0.60.2 does not remove `--trace-mem*`, `--trace-pc`, provenance switches, graphics/sprite diagnostics, course survey tools, checkpointing, patches or fast-forward options.
