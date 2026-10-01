> **SUPERSEDED FOR CURRENT v0.65.0 WORK:** Use `RESEARCH_SCRIPTING_0.65.0.md` as the authoritative Script Console syntax reference. The current Web Console includes restricted `if/else`, inline/named list iteration, `${repeat_index}`, and other behavior not fully documented here. The implementation in `Start-ChaseHQWeb.ps1` is the final source of truth if documentation conflicts.

# v0.63.2 Research Script Automation

v0.63.2 makes the Web Workbench Script Console a first-class automation client of the Research API.

## Project rule

Once the Workbench is running, every normal safe research operation exposed by the API should also be expressible from the Script Console. When a future workflow requires repeated manual API/UI actions, prefer adding/using a scriptable action rather than prescribing a long click sequence.

Startup/recovery outside the running Workbench remains a PowerShell responsibility.

## Language

Supported control syntax:

- `set name = value`
- `${name}` substitution
- comma-separated list variables
- `for item in list` ... `end`
- `repeat N` ... `end`
- `wait MS`
- `wait-until step-complete [timeout_ms]`
- `require capability NAME`
- `assert status key=value`
- `echo TEXT`
- `zip evidence latest|NAME`
- `zip ioc-sweep latest|NAME`
- `api ACTION key=value ...`
- all existing safe native debugger commands

Use `help` and `api actions` inside the Script Console for runtime discovery.

## Safety limits

- no shell/process execution
- no arbitrary filesystem paths from scripts
- 64 KiB script source
- 1000 source lines
- 500 expanded commands
- 64 iterations per loop
- nesting depth 4
- `wait` <= 5000 ms
- `wait-until` <= 30000 ms

## Script library

Reusable scripts live under `research/scripts/`. The Workbench can list, load and save `.chqscript` files in that tree. Metadata can be embedded as `# name:`, `# purpose:` and `# checkpoint:` comments.

Bundled examples include baseline gameplay, held IOC bit sweeps, deterministic pulse sweeps, evidence capture/ZIP and handling parameter sweeps.

## Scriptable API actions

The v0.63.2 action registry covers current safe operational API capabilities including machine status/capabilities, controls, timer hold, window always-on-top, handling tuning, palette inspection/entry override, IOC held/pulse/sweep actions, checkpoints, logs, evidence, frame capture, memory reads, events and semantic sprite tails.

New API capabilities should add a stable script action at the same time unless there is a concrete reason they cannot be safely automated.
