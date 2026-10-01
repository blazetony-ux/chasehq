# Simplified launcher — v0.62.1

`Start-ChaseHQ.ps1` is now the normal entry point for day-to-day ChaseHQ research. The older `Start-ChaseHQResearch.ps1` and `Start-ChaseHQWeb.ps1` remain available for advanced/manual workflows.

## Normal use

```powershell
.\Start-ChaseHQ.ps1
```

With no arguments this starts the canonical **Gameplay** state, waits for the native Research API, starts the Web Workbench, waits for its HTTP API, and opens the browser.

Named modes:

```powershell
.\Start-ChaseHQ.ps1 -Mode Gameplay
.\Start-ChaseHQ.ps1 -Mode Boot
.\Start-ChaseHQ.ps1 -Mode Attract
.\Start-ChaseHQ.ps1 -Mode TargetPostFinalHit
.\Start-ChaseHQ.ps1 -Mode StageEnd
```

A literal checkpoint can override the selected mode:

```powershell
.\Start-ChaseHQ.ps1 -Checkpoint .\checkpoints\stage1-gameplay-2064.chqstate
```

Stop the session recorded by the launcher:

```powershell
.\Start-ChaseHQ.ps1 -Stop
```

## Canonical mode mapping

| Mode | Checkpoint / behaviour |
|---|---|
| Gameplay | `stage1-gameplay-2064.chqstate` — player gameplay research |
| Boot | no checkpoint; normal boot |
| Attract | `stage1-driving-2352.chqstate` — autonomous attract-mode driving reference |
| TargetPostFinalHit | `stage1-target-post-final-hit-9548.chqstate` |
| StageEnd | `stage1-end-level-9988.chqstate` |

The `Attract` mapping deliberately makes the semantic role explicit because the historical filename `stage1-driving-2352` can otherwise be mistaken for player-controlled gameplay.

## Advanced switches

- `-NoWeb` starts only the emulator/native Research API.
- `-NoBrowser` starts the Web Workbench without opening the system browser.
- `-Attach` connects the launcher to an already-running native Research API rather than starting another emulator.
- `-Port` and `-HttpPort` override the default native/API ports (`37600` / `37680`).
- `-PrimeCommand` and `-EmulatorArgs` remain available for targeted experiments.

The launcher writes `.chq/active-session.json` so the matching `-Stop` operation can terminate the processes it started without relying on broad process-name killing.
