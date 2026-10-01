# Research Scripting 0.66.0

The Web Script Console implementation in `Start-ChaseHQWeb.ps1` is authoritative.

## Existing language
Variables, `${name}`, comma lists, `for`, `repeat`, `if/else`, waits, capability requirements, assertions, zip helpers and `api ACTION key=value` remain supported.

## API-backed query functions
Direct invocation prints the returned API data; assignment converts the returned collection to a comma-list suitable for `for`.

- `getregions()` -> `regions.list`
- `getprofiles()` -> `profiles.list`
- `getsprites()` -> `sprites.list`
- `getpaletteentries(bank=N)` -> `palette.entries`
- `getcheckpoints()` -> `checkpoint.list`
- `getcapabilities()` -> `capabilities`
- `getmaps()` -> `sprites.maps`
- `getpalettes()` -> `sprites.palettes`
- `getregistry()` -> `registry.list`
- `gettrackrecords(before=N,after=N)` -> `track.records`
- `getevidence()` -> `evidence.names`
- `getobjects()` -> `objects.list`
- `getaudiochannels()` -> `audio.channels`

## Self-description
Use `api platform.info`, `api features.list`, `api script.functions`, or `api schema` in future conversations rather than relying on stale documentation.

## Audio
The audio namespace is foundation-only in v0.66.0. `audio.capabilities`, `audio.channels`, `audio.events`, `audio.waveform.plan`, `audio.export.plan`, and `audio.inject.plan` describe the future contract. No runtime injection is enabled yet.

## Future parser work intentionally deferred
Rich collection helpers (`filter`, `sort`, `map`) and API-result objects are documented as future work rather than partially implemented here; they need runtime regression on the Windows workbench.

## Reuse and collection helpers
`include "relative/path.chqscript"` expands a saved library script before compilation. Includes are intentionally one-level in v0.66.0; recursive include policy is reserved for later hardening.

Collection helpers operate directly on API-backed query functions:
- `count(getregions())`
- `first(getregions())`
- `last(getregions())`
- `unique(getpalettes())`
- `contains(getprofiles(),chasehq)`

They can also be assigned with `set name = ...`.

## Experiment recorder
- `api experiment.begin name=NAME`
- `api experiment.capture label=NAME include=sprites,regions,gameplay,track`
- `api experiment.status`
- `api experiment.end`

An experiment groups screenshots and structured state under `web-evidence/experiments/<id>/manifest.json`. Automatic mutation rollback remains future work.
