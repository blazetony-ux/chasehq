# v0.65.0 Research Script Console — Authoritative Current Syntax

The implementation in `Start-ChaseHQWeb.ps1` is authoritative if this document and code ever disagree.

## Core syntax

```text
# comment
set name = value
${name}

for item in listVariable
    ...
end

repeat N
    ... ${repeat_index} ...
end

if status KEY OP VALUE
    ...
else
    ...
end

if memory cpu=A address=ADDR width=8|16|32 value OP HEX
    ...
end

if capability NAME
    ...
end
```

`OP` is `=`, `!=`, `<`, `<=`, `>` or `>=`.

Other safe commands: `wait`, `wait-until step-complete`, `require capability`, `assert status`, `echo`, `zip ...`, `api ACTION key=value ...`, plus the allow-listed native debugger commands.

## API-backed query functions

These functions are resolved from the current Research API rather than from hard-coded script knowledge:

```text
getregions()
getsprites()
getmaps()
getpalettes()
getpaletteentries(bank=N)
getcheckpoints()
getcapabilities()
```

Assignment returns an iterable comma-separated value list:

```text
set regions = getregions()
for region in regions
    api image.compare a=control b=test region=${region}
end
```

Variables already assigned earlier in the script can be used in query arguments:

```text
set bank = 75
set entries = getpaletteentries(bank=${bank})
```

Calling a query function directly prints its authoritative backing API result, which is intentional for future diagnosis:

```text
getregions()
getsprites()
getcapabilities()
```

The API remains authoritative. Script query functions are ergonomic wrappers only.

## Current v0.65.0 limits

- source: 64 KiB
- source lines: 1000
- expanded commands: 500
- loop/repeat iterations: 256
- nesting depth: 4
- `wait`: 0..5000 ms
- `wait-until`: up to 30000 ms

## Live graphics research API

```text
api sprites.list
api sprite.inspect slot=44
api sprite.override slot=44 map=182
api sprite.override slot=44 palette=77
api sprite.override slot=44 visible=false
api sprite.override.clear
api sprite.visual slot=44 mode=solo
api sprite.visual slot=44 mode=hide
api sprite.visual slot=44 mode=flash
api sprite.visual.clear
```

Overrides are presentation-only and do not modify emulated game RAM.

## Image/state helpers

```text
api regions.list
api palette.entries bank=75
api state.snapshot include=sprites,regions,gameplay,track
api image.compare a=NAME b=NAME region=FULL
api image.diff a=NAME b=NAME region=FULL output=diff-name
```

`image.compare` and `image.diff` accept ordinary `frame.capture` names, evidence names, and structured `frame.snapshot` names.

## Regression rule

Every new Script Console syntax feature or scriptable Research API action must update/add regression coverage under `research/scripts/regression/` in the same release.
