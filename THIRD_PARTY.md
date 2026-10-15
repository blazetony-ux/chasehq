## RC4.7 — Static Analysis / REA tooling candidate

See `docs/REA_GHIDRA_RC47.md` and `docs/DOCUMENTATION_AUDIT_0.67.0.0-RC4.7.md`. **Current candidate RC4.7; promoted parent RC4.6. Windows qualification pending.** Dedicated Workbench REA workspace, bounded `ghidra.wait`, Ghidra run evidence attachment, corrected result path, Java/Jython preflight and flow-edge export. No Capstone library is shipped in this candidate; source vendoring is blocked pending a verified upstream archive. No native emulator CPU replacement.

# Third-party components

## Musashi

This project includes the Musashi 68000 emulator source under `third_party/musashi`. Preserve its upstream copyright and license notices when redistributing this source tree.

## SDL3

SDL3 is **not** redistributed in this package. The `SDL/` directory contains placeholder instructions only. Copy a local SDL3 source tree into that directory before configuring the graphical target, or configure with `-DCHASEHQ_BUILD_SDL=OFF` for the CPU-only runtime.

## Chase H.Q. ROMs

No Taito ROM data is included. The runtime loads ROMs supplied locally by the user.

### RC4.7 reverse-engineering toolchain boundary

Ghidra and its optional Jython extension remain separate user-installed tools. `chqdev ghidra setup --install-jython` can install only a verified version-matching extension bundled with that separate distribution, after an explicit operator opt-in. Capstone M68K is **not shipped or linked**: no source archive was available for provenance, license and build verification in the portable build environment. The engine's Capstone support is a deferred dependency, not a claimed feature.
