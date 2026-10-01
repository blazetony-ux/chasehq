# ChaseHQ Native - Experimental Track Scripts

These scripts are intended for v0.64.8.4+ Research Workbench.

## Purpose
They provide tangible causal experiments against the live mutable Stage 1
track cache around 0x109000 while the game's ATTRACT-mode driver is active.

## Current evidence basis
- 0x10A05A is the current 8-byte track-record offset.
- 0x109000... is the live mutable track-page/cache.
- Track record +0 is signed horizontal curvature:
  - negative = LEFT
  - positive = RIGHT
  - zero = straight/no horizontal curvature.
- At stage1-driving-2352, the current record is 0x1090A8 and the next is 0x1090B0.
- The loader may repopulate the cache at later page transitions, so these are
  intentionally local experiments rather than claims of permanent ROM-level replacement.

## Recommended order
1. track-flat-4-records.chqscript
2. restore-canonical-track-state.chqscript
3. track-strong-right-4-records.chqscript
4. restore-canonical-track-state.chqscript
5. track-strong-left-4-records.chqscript
6. restore-canonical-track-state.chqscript
7. track-synthetic-slalom-8-records.chqscript

Watch both the SDL window and Track View.

## Safety / recovery
All experiments begin from stage1-driving-2352.chqstate.
To reset at any time:
- pause/stop the emulator if needed
- run restore-canonical-track-state.chqscript

## Future extension
Once a long-lived track-page experiment is proven, the next step is an automated
track-injection runner that rewrites upcoming records ahead of the car and logs
track.state + screenshots throughout the run.
