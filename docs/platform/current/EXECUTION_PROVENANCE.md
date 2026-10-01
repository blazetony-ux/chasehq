# Execution Provenance Platform (v0.42)

v0.42 adds dynamic execution provenance so a memory address can be followed across callers/callees without repeatedly choosing fixed PC ranges.

## Core commands

```text
--follow-address A[:B]         follow reads and writes
--follow-read A[:B]            follow reads only
--follow-write A[:B]           follow writes only
--find-first-change A[:B]      change-only writer watch plus provenance
--trace-callstack              reconstruct observed BSR/JSR/RTS/RTE call stacks
--trace-callgraph              export dynamic caller -> callee graph
--profile-functions            export per-function counters
--provenance-from-frame N
--provenance-to-frame N
--provenance-depth N
--provenance-max N
--fast-forward-to N            SDL frontend only: suppress drawing/events/pacing before N
```

`--follow-address` automatically enables call-stack/callgraph/function profiling.

Example for the player-car shared structure:

```powershell
.\out\build\x64-Debug\ChaseHQNative.exe --scenario stage1-gameplay --frames 1800 --fast-forward-to 1600 --follow-address 10a04a --provenance-from-frame 1600 --provenance-to-frame 1650 --no-legacy-debug
```

## Outputs

- `provenance_address_flow.csv` — frame/CPU/op/PC/function/address/value/region/call stack.
- `provenance_callstack.log` — readable CALL and followed memory events.
- `provenance_callgraph.csv` — caller/callee/count table.
- `provenance_callgraph.dot` — Graphviz-compatible dynamic call graph.
- `provenance_functions.csv` — observed function instruction/read/write/call counts.
- `provenance_summary.txt` — run summary and limitations.

## How call stacks work

The runtime observes every executed instruction. A BSR/JSR is recorded as a pending call; on the next instruction hook the actual target PC is known, so indirect calls are resolved dynamically rather than guessed from disassembly. RTS/RTE/RTR pop the shadow stack. Known return addresses are also used to opportunistically re-synchronise after unusual control flow.

This is a *dynamic* call graph: it reports paths actually executed by the selected scenario, not every possible ROM function.

## Timeline acceleration

`--fast-forward-to N` still executes every CPU frame so machine state remains causally correct, but the SDL frontend does not draw, poll normal frontend events, or sleep at 60 Hz until frame N. This is the safe speed-up before true resumable save states exist.

## Save-state status

v0.42 does **not** claim full CPU/device checkpoint resume. v0.41/v0.42 hardware captures remain forensic captures. A true checkpoint format still requires serialising/restoring both Musashi CPU contexts and all private runtime/device latches in addition to RAM/palette/video state.

## v0.43 focused mode

`--follow-address`, `--follow-read`, and `--follow-write` still reconstruct shadow caller stacks but no longer accumulate/export the entire dynamic callgraph and function profile by default. This removes substantial work during narrow address investigations. Add `--provenance-full-graph` for the v0.42 global graph/profile behaviour, and `--provenance-verbose-calls` when every CALL line is required.
