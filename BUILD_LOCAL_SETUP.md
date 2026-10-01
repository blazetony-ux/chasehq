# Local ROM / SDL staging

`Prepare-Project.bat` can stage user-local Chase H.Q. ROMs and an SDL3 source tree before CMake runs. Nothing is downloaded and ROMs are never included in release packages.

## Option 1: local config (recommended)
Copy `build-local.example.json` to `build-local.json` and edit the paths. `build-local.json` is git-ignored.

## Option 2: environment variables
- `CHASEHQ_ROM_SOURCE`
- `CHASEHQ_SDL_SOURCE`

## Option 3: command line
```bat
Prepare-Project.bat --roms "D:\Arcade\ROMs\chasehq" --sdl "D:\DevDeps\SDL3"
```

Priority: command line > environment > `build-local.json` > existing project files.

Use `Prepare-Project.bat --check` to validate configuration without copying files.

## v0.66.7.9 developer handoff rule

For the Windows development handoff used in this project, carry the working `build-local.json` forward so `Build-Debug.bat` can immediately stage ROM/SDL dependencies from the established local paths. Do **not** compensate for a missing local config by bundling the full SDL source tree into every candidate ZIP.

The source/research handoff package should remain small: no SDL tree, no `out`, no ROM payload and no transient evidence. Public/source redistribution may omit the machine-specific `build-local.json`; the developer handoff used for continuous local testing may include it intentionally.
