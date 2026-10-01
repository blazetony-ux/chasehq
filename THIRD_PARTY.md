# Third-party components

## Musashi

This project includes the Musashi 68000 emulator source under `third_party/musashi`. Preserve its upstream copyright and license notices when redistributing this source tree.

## SDL3

SDL3 is **not** redistributed in this package. The `SDL/` directory contains placeholder instructions only. Copy a local SDL3 source tree into that directory before configuring the graphical target, or configure with `-DCHASEHQ_BUILD_SDL=OFF` for the CPU-only runtime.

## Chase H.Q. ROMs

No Taito ROM data is included. The runtime loads ROMs supplied locally by the user.
