# ChaseHQ-Native v0.8 — CPU Runtime

Replacement source project based on the supplied v0.7 Scene Renderer.
This is an initial main-68000 runtime, **not a playable emulator**.
The v0.7 road/sprite renderer and scene controls are preserved. Its scene is
still synthetic and does not yet consume the CPU's sprite, palette or road RAM.
The window title explicitly identifies this preview.

## Windows setup

1. Install Visual Studio 2022 with **Desktop development with C++**, Windows SDK,
   and CMake tools. Open an **x64 Native Tools Command Prompt for VS 2022**.
2. Extract this ZIP to a new folder. Copy the contents of your working v0.7
   `SDL` folder into this project's `SDL` folder. `SDL/CMakeLists.txt` must exist.
   Use SDL3 source, not SDL2 or a prebuilt development library.
3. Copy your extracted v0.7 ROM files into `roms/chasehq`.
   Add the four World-set main CPU ROMs listed below if they are not already there.
4. From the folder containing this README:

```bat
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
build\Release\ChaseHQNative.exe
```

Musashi is included in `third_party/musashi`. CMake builds its opcode generator
locally; no Git, FetchContent, network download or separate CPU installation is
needed. SDL3.dll is copied beside the frontend executable automatically.

The default ROM directory is the project's absolute `roms/chasehq` path recorded
at configure time, so launching from another working directory works. Reconfigure
if you move the source folder, or pass `--roms "D:\Games\roms\chasehq"`.
The legacy first positional ROM-directory argument also works.

## Main CPU ROMs (MAME World `chasehq` set)

| File | Bytes | Region offset / byte lane | CRC32 |
| --- | ---: | --- | --- |
| b52-130.36 | 131072 | 0x00000 even | 4e7beb46 |
| b52-136.29 | 131072 | 0x00001 odd | 2f414df0 |
| b52-131.37 | 131072 | 0x40000 even | aa945d83 |
| b52-129.30 | 131072 | 0x40001 odd | 0eaebc08 |

Each pair is interleaved byte by byte into a big-endian 0x80000-byte region;
there is no extra word swap. All four file sizes and CRCs must match. Missing,
short, corrupted or renamed regional-set ROMs produce explicit errors. Other
regional revisions need an explicit ROM manifest update; they are not guessed.
No game ROMs are included.

The SDL scene additionally requires the same ten graphics ROMs as v0.7:
`b52-34.5`, `b52-35.7`, `b52-36.9`, `b52-37.11`, `b52-30.4`,
`b52-31.6`, `b52-32.8`, `b52-33.10`, `b52-38.34`, `b52-28.4`.
Each graphics file is 524288 bytes; existing v0.7 size validation is preserved.

## CPU-only build and execution

This target needs only the four main CPU ROMs, without SDL or graphics assets:

```bat
cmake -S . -B build-cpu -G "Visual Studio 17 2022" -A x64 -DCHASEHQ_BUILD_SDL=OFF
cmake --build build-cpu --config Release --parallel
ctest --test-dir build-cpu -C Release --output-on-failure
build-cpu\Release\ChaseHQRuntime.exe --frames 120 --logs logs\first-boot
```

Both executables use the same runtime. By default they execute at most 120 slices
of 200000 cycles each (nominal 12 MHz / 60 Hz; instruction completion can overshoot
slightly). The CPU-only target exits after the budget. The SDL target runs one
slice per preview frame, saves diagnostics at the budget, and keeps the scene
interactive until Esc. Closing early saves the partial run.

Optional arguments:

- `--frames N`: 1 to 36000 slices. Default 120. There is no unbounded mode.
- `--roms PATH`: ROM directory.
- `--logs PATH`: diagnostic directory, relative to the launch directory unless
  absolute. Default `logs`. Existing files at that location are overwritten;
  use a fresh directory for each comparison run.
- `--trace-all`: include ROM reads/instruction fetches in the capped bus trace.
- `--irq4`: inject a simplified level-4 interrupt once per slice, held until
  autovector acknowledgement. Off by default for initial reset-path inspection.
  This is diagnostic timing, not a complete video/dual-CPU scheduler.
- `--scene-only`: SDL target only; original v0.7 scene without main CPU ROMs.
- `--help`: print options and exit.

```bat
build\Release\ChaseHQNative.exe --scene-only
build\Release\ChaseHQRuntime.exe --frames 600 --irq4 --logs logs\irq-run
```

Scene controls: arrows adjust curve/horizon; Shift increases adjustment;
A/D changes road-ROM phase; R toggles road; T toggles sprites; Home resets the
scene; Esc quits. Home does not reset the CPU. Restart for a fresh CPU run.

## Diagnostics

`bus.log` records CPU space, last instruction PC, read/write, access width,
address, value and region. The default omits ROM reads, but includes RAM/device
reads, writes (including rejected ROM writes), and unmapped accesses. At 20000
entries it writes a truncation notice; counters continue without logging more.
Long operations are represented as the accesses issued by Musashi; some CPU
operations naturally appear as two 16-bit accesses. Values are hexadecimal.

`summary.txt` reports final PC, SR, stack, cycles, instruction count and per-region
**byte-access** counters. Instruction counts exclude cycles spent stopped.
`ram/` contains big-endian byte dumps of work/shared RAM, sprites, tilemaps,
controls, palette registers, a separate 8192-byte `palette.bin`, and road RAM.
Stub writes are retained for inspection even when their read behavior is fixed.
Road RAM should remain zero when only CPU A runs; that is expected.

Exit codes: 0 = diagnostic budget/normal exit, 1 = setup, ROM, file or argument
error, 2 = execution left the supported executable address ranges. Exit 0 does
not establish that the original game has booted. Waiting for CPU B, sound,
inputs, or a hardware status transition is possible. STOP and exception handling
are performed by Musashi. No synthetic success responses are inserted to skip
handshakes. A PC guard stops further slices when execution leaves ROM/work RAM;
this is a diagnostic guard rather than emulated bus-error hardware.

## Initial memory abstraction

All addresses below are hexadecimal, inclusive, 24-bit; words are big-endian.
Unmapped reads return all ones, writes are ignored and counted. ROM is read-only.

| CPU A range | Implementation |
| --- | --- |
| 000000–07ffff | Reconstructed program ROM |
| 100000–107fff | Work RAM |
| 108000–10bfff | Shared RAM, aliased by the CPU B bus view |
| 10c000–10ffff | Work RAM |
| 400000–400003 | I/O/watchdog placeholder; reads FF, writes retained |
| 800000–800001 | CPU control latch placeholder; no CPU B reset action yet |
| 820000–820003 | Sound placeholder; odd-byte reads 00, high lanes FF |
| a00000–a00007 | TC0110PCR-style indirect palette registers |
| c00000–c0ffff | Tilemap RAM placeholder |
| c20000–c2000f | Tile control storage |
| d00000–d007ff | Sprite RAM |
| e00000–e003ff | Motor interface storage placeholder |

Palette address is written at a00000 (low 12 bits, Chase H.Q. shift=0).
Data is read/written at a00002. There are 4096 raw xBGR555 entries, no automatic
index increment; other palette-register reads return 00ff. Byte writes merge
big-endian lanes. Raster timing and palette rendering are not connected yet.

The reserved CPU B bus view provides private RAM at 100000–103fff, the shared
alias at 108000–10bfff, and **road RAM at 800000–801fff**. CPU A's 800000 address
is its CPU control latch, not road RAM. The bus API accepts `BusSpace::Sub` so this
separation is testable now. CPU B program loading and execution are not implemented.

## Architecture and next work

- `cpu_rom.*`: validated World ROM loader and lane reconstruction.
- `runtime.*`: bus, backing RAM, stubs, logging, Musashi adapter and run budget.
- `runtime_main.cpp`: CPU-only executable.
- `machine.*`, `rom_loader.*`, `video.*`: preserved v0.7 graphics pipeline.
- `main.cpp`: SDL scene plus bounded CPU execution.
- `tests/runtime_tests.cpp`: ROM interleave, bus semantics and synthetic 68000 tests.

Musashi is configured for a 68000, with address errors, trace exceptions,
instruction callbacks and interrupt acknowledge enabled. Its state is global;
only one runtime instance may exist at a time. This is not thread-safe. A future
CPU B implementation must use explicit context save/restore and a scheduler.

Next work: execute CPU B and reproduce shared-RAM communication, implement real
I/O and sound handshakes, feed hardware RAM into the renderer, and add accurate
interrupt/device scheduling. This project does not claim a working attract mode.

## Provenance and validation

Based on the user-provided `ChaseHQ-Native-v0.7-Scene-Renderer.zip`.
The sprite and road decoding algorithms are unchanged.
Musashi snapshot: `313ebf1bd9f4d0d93341eb5ce21fd8a119e9dbdd`.
Original CPU and SoftFloat notices are retained under `third_party/musashi`.
See `THIRD_PARTY.md` for reference links and licensing locations.
See `VALIDATION.md` for the checks performed for this release.
