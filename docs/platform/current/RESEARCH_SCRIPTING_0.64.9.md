> **SUPERSEDED FOR CURRENT v0.65.0 WORK:** Use `RESEARCH_SCRIPTING_0.65.0.md`.

# v0.64.9 Research Script Console — Authoritative Current Syntax

This document is the authoritative scripting-language reference for the v0.64.9 Web Research Workbench.

**Source of truth:** the current Web Script Console implementation in `Start-ChaseHQWeb.ps1` wins over older documentation when there is a conflict. In particular, `RESEARCH_SCRIPTING_0.63.2.md` predates the restricted `if/else` support now implemented by the Web Console.

## Project rule

Once the Workbench is running, normal safe research operations should be expressed through the Script Console and stable Research API wherever practical. Prefer compact scripts using variables, lists, loops, repeats and restricted conditions rather than manually expanding repeated commands.

Startup/recovery outside the running Workbench remains a PowerShell responsibility.

## Comments and blank lines

Blank lines are ignored. Lines beginning with `#` are comments.

Recommended metadata comments for reusable scripts:

```text
# name: Human-readable title
# purpose: Research outcome
# checkpoint: canonical-state.chqstate
```

## Variables

Define a variable:

```text
set checkpoint = stage1-driving-2352.chqstate
set regions = FULL,ROAD_AREA,PLAYER_CAR,HUD_TOP,HUD_TURBO,TARGET_AREA
```

Substitute a variable with `${name}`:

```text
api checkpoint.load file=${checkpoint} run=false
```

Variable names must begin with a letter or underscore and may then contain letters, digits and underscores.

Variables are textual substitutions. The language does not currently provide a general arithmetic/expression engine.

## Lists and `for`

Comma-separated values are used as lists.

Named list variable:

```text
set regions = FULL,ROAD_AREA,PLAYER_CAR
for region in regions
    api image.compare a=baseline b=patched region=${region}
end
```

An inline comma-separated source is also accepted:

```text
for region in FULL,ROAD_AREA,PLAYER_CAR
    echo ${region}
end
```

A `for` loop is limited to 64 iterations.

## `repeat`

```text
repeat 4
    echo pass-${repeat_index}
end
```

`${repeat_index}` is supplied automatically and starts at **1**.

`repeat` accepts `0..64` iterations.

## Restricted `if` / `else`

The Web Script Console supports restricted conditions. `else` is optional.

### Status predicate

```text
if status frame >= 2656
    echo reached-frame
else
    echo before-frame
end
```

Form:

```text
if status KEY OP VALUE
```

### Memory predicate

```text
if memory cpu=A address=10A048 width=8 value = 05
    echo road-state-match
else
    echo road-state-different
end
```

Form:

```text
if memory cpu=A address=ADDR width=8|16|32 value OP HEX
```

CPU is `A` or `B`.

### Capability predicate

```text
if capability frame.snapshot
    api frame.snapshot name=baseline
else
    echo frame.snapshot unavailable
end
```

Form:

```text
if capability NAME
```

### Comparison operators

Supported operators:

```text
= != < <= > >=
```

Status relational comparisons require numeric values. Equality/inequality also work for strings.

## Nesting

`for`, `repeat` and `if` blocks may be nested, subject to the scripting safety limit of nesting depth 4.

## Timing and synchronization

Pause execution for milliseconds:

```text
wait 250
```

Range: `0..5000` ms.

Wait for a deterministic stepping operation to complete:

```text
wait-until step-complete
wait-until step-complete 10000
```

Maximum timeout: 30000 ms.

## Capability requirements

Fail the script if a capability is unavailable:

```text
require capability NAME
```

Use `if capability NAME` when the script should branch instead of fail.

## Assertions

```text
assert status paused=1
```

This is intended for validation/regression scripts where a failed condition should stop execution when **Stop on error** is enabled.

## Output

```text
echo TEXT
```

Variable substitution works inside the text.

## Research API actions

```text
api ACTION key=value ...
```

Examples:

```text
api status
api checkpoint.load file=stage1-driving-2352.chqstate run=false
api control.run-frames frames=30
api frame.capture name=baseline
api image.compare a=baseline b=patched region=PLAYER_CAR
```

Use:

```text
api actions
api schema action=ACTION_NAME
```

for runtime discovery of stable scriptable API actions and their contracts.

## Evidence ZIP helpers

```text
zip evidence latest
zip evidence NAME
zip ioc-sweep latest
zip ioc-sweep NAME
zip evidence-match PREFIX
zip evidence-match PREFIX BUNDLE_NAME
```

## Native debugger commands

All existing **safe native debugger commands** accepted by the Workbench remain valid script lines in addition to the higher-level `api` actions.

No arbitrary shell/process execution is available from scripts.

## Tokenization / quoting

Command arguments are tokenized by the Workbench's safe script tokenizer. Quote arguments when they contain whitespace. Keep filesystem access within the safe paths exposed by supported actions; scripts do not provide unrestricted filesystem access.

## Safety limits

Current v0.64.9 limits:

- 64 KiB maximum script source
- 1000 maximum source lines
- 500 maximum expanded commands
- 64 maximum iterations per `for` loop
- `repeat` range `0..64`
- nesting depth 4
- `wait` range `0..5000` ms
- `wait-until` timeout <= 30000 ms
- no arbitrary shell/process execution
- no unrestricted filesystem paths

## Important implementation note

The Web Script Console compiles `if/else`, loop/repeat structure and substitutions client-side, while individual safe commands are then executed through the Research API/script endpoint. Therefore the Web Console implementation is the authoritative reference for the language available interactively in v0.64.9.

Older documentation may describe only the earlier server-side expander subset.

## Preferred style for future work

Do **not** manually duplicate equivalent commands when the current language can express the operation with variables/lists/loops.

Preferred:

```text
set regions = FULL,ROAD_AREA,PLAYER_CAR,HUD_TOP,HUD_TURBO,TARGET_AREA

for region in regions
    api image.compare a=determinism-a b=determinism-b region=${region}
end
```

Avoid six manually repeated `api image.compare` lines unless there is a concrete reason each invocation differs.

For deterministic A/B experiments, reload the same canonical checkpoint before each branch so frame-equivalent captures can be compared without natural gameplay motion contaminating the result.
