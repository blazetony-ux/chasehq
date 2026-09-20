# Validation — v0.8 CPU Runtime

Validated on Windows x64, Visual Studio 2022 / MSVC 19.44.35229,
CMake 3.31.6, using an external local SDL3 3.2.20 source tree.

- CPU-only configuration and build: passed.
- Complete SDL3 frontend Release build: passed; SDL3.dll copied beside executable.
- Release CTest suite: passed (1 test executable with multiple behavioral checks).
- Synthetic CPU program: immediate word/byte writes to work RAM, long write to
  sprite RAM, arithmetic, STOP, IRQ4 wakeup/autovector, and RTE stack restoration.
- Full 0x80000 interleave checked against all input lane bytes.
- Missing ROM, wrong size, wrong checksum, invalid reset vector, invalid bus
  width and simultaneous runtime instance rejected.
- Big-endian byte lanes, 24-bit address wrap, ROM write protection, region
  boundary access, shared RAM alias, separate sub-CPU road space and palette
  address/data registers checked.
- Trace filtering/cap and RAM/palette dump sizes checked.
- CLI help, invalid frame count and missing-ROM error behavior checked.

The upstream opcode generator produced incorrect mask values with this compiler's
Release optimization. CMake now compiles only m68kmake with /Od. The resulting
opcode source is byte-for-byte identical to the Debug generator's output
(SHA256 EB4BB8C9CB12BF10521658F9150D092438623164A11E35C6B46DDC6AA585A3FC).
The Release emulation core remains optimized. CPU tests pass with the regenerated
source. No upstream source edits are required for this build workaround.

No commercial game ROMs were supplied in the v0.7 project ZIP. Therefore actual
Chase H.Q. boot progress, game-written RAM contents and visual runtime behavior
have NOT been verified. The SDL frontend was compiled and its help command run;
a rendered scene was not visually retested. The v0.7 graphics algorithms and
scene input handling are retained, with updated preview title text.

The runtime deliberately omits CPU B execution, actual I/O/sound/motor devices,
and hardware-driven rendering. A successful bounded run is not a playable game
or a confirmed completed arcade boot.
