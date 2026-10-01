# ChaseHQ-Native v0.64.7.1.1 — Research API Reliability & Memory Tooling

v0.64.7.1.1 is a focused Research Workbench reliability release built from the validated v0.64.6 baseline after live reverse-engineering exposed several debugger-interface bottlenecks.

## Script Console
- Adds deliberately restricted `if` / optional `else` blocks in the Web Workbench Script Console.
- Supported predicates: `status KEY OP VALUE`, `memory cpu=A|B address=... width=... value OP HEX`, and `capability NAME`.
- Operators are limited to `= != < <= > >=`; there is no arbitrary expression evaluator or shell execution.
- Existing `for`, `repeat`, variables and safety limits remain intact.

## Request reliability
- Frontend proxy uses operation-sensitive backend timeouts: ordinary reads retain the short failure window while long script, deterministic-frame, checkpoint-load and IOC-sweep operations receive up to 60 seconds.
- Browser requests use matching long-operation timeouts, avoiding the false `DEGRADED` / 10-second abort observed with deterministic research runs.

## Memory tooling and discovery
- `memory.export` now accepts either `address + length` or legacy `start + end`.
- Addresses accept plain hexadecimal or `0x` form; lengths accept decimal or `0x` hexadecimal.
- `output=evidence\...` is supported and sandboxed beneath the evidence root.
- New `memory.read-range` action returns compact contiguous hex dumps without requiring a file export.
- `api schema action=...` now exposes action-specific contracts and examples.
- `help ACTION` returns action-specific help rather than silently falling back to generic Script v2 help.

## Gameplay / track registry corrections
- Authoritative packed-BCD score is `0x100488`; `0x100408` is retained as the derived display/masked copy.
- Course position is `0x10A040`; rejected `0x102FC0` is no longer presented as distance.
- Road classification uses `0x10A048 & 0x06`: 00 on-road, 04 left off-road, 02 right off-road, 06 extreme/combined.
- Live track-record offset is `0x10A05A`, in 8-byte records rooted at `0x109000`; packed geometry remains `0x10A05C`.
- The Gameplay Registry no longer performs the unsafe signed-to-UInt16 road-centre conversion that could make one semantic field take down the entire registry response.

## Deterministic research checkpoint
The attract-mode road transition remains a useful regression observation from `stage1-driving-2352.chqstate`: frame 2656 has `0x10A048=0x01`; frame 2657 has `0x10A048=0x05` (left off-road). The checkpoint remains explicitly ATTRACT MODE, not player gameplay.


## v0.64.7.1 corrective patch
- Restores Script Console output rendering (`appendScriptOutput` was missing in v0.64.7).
- Copy/Download Output now refuse empty output and report copied/downloaded character counts.
- Adds native `readrange` debugger command; `memory.read-range` no longer spawns one `chqctl` process per byte.
- `memory.export` now uses one native bulk read and parses it into the binary export.
- While an intentional Script Console command owns the serialized backend, the header shows `BUSY` rather than a false `DEGRADED` state.
