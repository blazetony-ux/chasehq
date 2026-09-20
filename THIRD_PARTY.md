# Third-party code and hardware references

## Musashi

Vendored unchanged from https://github.com/kstenerud/Musashi at commit
313ebf1bd9f4d0d93341eb5ce21fd8a119e9dbdd (retrieved 2026-09-20).
https://github.com/kstenerud/Musashi/tree/313ebf1bd9f4d0d93341eb5ce21fd8a119e9dbdd

Copyright Karl Stenerud. The permissive license is reproduced in the source
headers, including third_party/musashi/m68kcpu.c and m68k.h. All upstream
notices are retained. Build configuration overrides are in our CMakeLists.txt;
no upstream source modifications were made. Opcode files are generated locally.

Musashi includes John R. Hauser's SoftFloat Release 2b. Its copyright,
redistribution and derivative-work requirements are retained in
third_party/musashi/softfloat/README.txt and its source headers. SoftFloat
is included because the upstream core contains FPU helper code even though
this project selects the 68000 CPU type.

## SDL3

Not bundled. Copy your existing local SDL3 source into SDL/. SDL is licensed
under the zlib license; retain its LICENSE.txt with your copy.
Official source: https://github.com/libsdl-org/SDL

## Hardware documentation consulted

MAME's Taito Z driver: Chase H.Q. CPU A/B maps, CPU clock, IRQ4, World ROM
filenames, lane offsets, CRCs, palette format and shift configuration:
https://github.com/mamedev/mame/blob/master/src/mame/taito/taito_z.cpp

TC0110PCR address/data register semantics:
https://github.com/mamedev/mame/blob/master/src/mame/taito/tc0110pcr.cpp

References retrieved 2026-09-20. MAME source is not incorporated into this
project. The initial bus is a separate implementation based on those hardware
facts. Placeholders and deviations are identified in README.md.

No Chase H.Q. program or graphics ROMs are distributed in this ZIP.
Build-time note: the project compiles m68kmake with MSVC /Od because MSVC 19.44
Release optimization was observed to corrupt generated opcode masks. The core
itself remains optimized. See VALIDATION.md for verification.
