# Chase H.Q. Native v0.60.3 — Research Session Launcher

`Start-ChaseHQResearch.ps1` is the preferred way to start a live reverse-engineering session. It orchestrates the SDL emulator and `chqctl` so a complete session can be started and primed with one PowerShell command.

## Basic launch

```powershell
.\Start-ChaseHQResearch.ps1
```

The launcher:

1. validates `ChaseHQNative.exe`, `chqctl.exe`, the ROM directory and optional checkpoint;
2. checks that the requested API port is not already occupied by another ChaseHQ Research API;
3. starts `ChaseHQNative.exe` with the supplied launch-time diagnostics/options;
4. waits until the localhost Research API responds;
5. runs an optional `.chqscript` plus any supplied live API priming commands;
6. opens a second PowerShell window running `chqctl shell`.

The emulator remains the source of truth. The shell is simply another client of the same v0.60 Research Workbench state used by F12.

## Presets

JSON session presets live in `sessions/`.

```powershell
.\Start-ChaseHQResearch.ps1 -Preset default
```

A target-posthit example is included:

```powershell
.\Start-ChaseHQResearch.ps1 -Preset target-posthit
```

A preset may specify `Roms`, `Checkpoint`, `Port`, `EvidenceRoot`, `EmulatorArgs`, `PrimeCommands`, and `Experiment`.

## One-off parameterized session

```powershell
.\Start-ChaseHQResearch.ps1 `
  -Roms ".\roms\chasehq" `
  -Checkpoint ".\checkpoints\stage1-target-pre-final-hit-9495.chqstate" `
  -NoCollisions -InfiniteTime -UnlimitedTurbo -AutoTurbo `
  -CorneringScale 1.15 -CorneringSpeedRetain 1.00 `
  -PrimeCommand "events limit 100000","snapshot save baseline","watch add A 10A096 16 write change break"
```

Arbitrary established emulator switches can be passed through with `-EmulatorArgs`:

```powershell
.\Start-ChaseHQResearch.ps1 -EmulatorArgs @(
  "--course-survey",
  "--course-follow",
  "--course-follow-controller", "legacy"
)
```

## Prime with an experiment script

```powershell
.\Start-ChaseHQResearch.ps1 -Experiment ".\experiments\target-motion-example.chqscript"
```

The script is executed only after the API is ready. `PrimeCommand` commands are then applied in order.

## Attach to an already running emulator

```powershell
.\Start-ChaseHQResearch.ps1 -Attach
```

This skips emulator launch, verifies the API, primes the requested commands/scripts, and opens `chqctl shell`.

## Non-interactive / automation mode

Use `-NoShell` when the launcher is being used only to start and prime an automated session:

```powershell
.\Start-ChaseHQResearch.ps1 -Preset target-posthit -NoShell
```

## API port

Both emulator and client are kept on the same explicit port:

```powershell
.\Start-ChaseHQResearch.ps1 -Port 37601
```

This also permits multiple independent ChaseHQNative instances when each is given its own port and session/output paths.

## Fresh evidence directory

`-Fresh` clears and recreates the selected `-EvidenceRoot` (or `.\evidence\research-session` when omitted) before launch. It deliberately does not kill an already-running emulator; an occupied API port is treated as an error unless `-Attach` is specified.

## Design intent

This launcher is an orchestration layer, not a replacement for either interface. Existing launch-time diagnostics remain available through `-EmulatorArgs`, while all v0.60.x live API commands can be used in `PrimeCommands`, `.chqscript` files, the interactive shell, or later commands copied directly from ChatGPT.
