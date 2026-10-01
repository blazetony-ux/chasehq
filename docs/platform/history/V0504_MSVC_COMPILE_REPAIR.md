# v0.50.4 MSVC compile repair

v0.50.4 is a narrow corrective patch for the v0.50.3 player-car semantic-object release.

The v0.50.3 archive contained two source-generation/escaping defects which GCC CPU-only tests did not exercise because the SDL frontend translation units were not part of that test target:

- `src/main.cpp`: four JSON manifest keys added for semantic/object evidence were emitted with malformed C++ string quoting, causing MSVC C3688 literal-suffix errors.
- `src/video.cpp`: `write_semantic_object_evidence_csv` had literal `\\n` source text inserted between C++ statements, causing MSVC illegal-escape and cascading syntax errors.

Both defects are corrected. There is no intended emulation, rendering, checkpoint, or semantic-object behaviour change from v0.50.3.

Validation performed in the build environment:

- configure with `-DCHASEHQ_BUILD_SDL=OFF`
- clean CPU build
- `cpu_bus_rom_tests`: pass

The Windows SDL/MSVC frontend still requires confirmation on the user's machine because the distributable source archive does not include the local SDL source tree.
