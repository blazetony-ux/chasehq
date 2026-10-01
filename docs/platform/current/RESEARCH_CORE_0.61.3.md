# v0.61.3 Research Core

This candidate builds reusable research infrastructure without changing intended Chase H.Q. gameplay behaviour.

## Added
- `chq-experiment-v1` JSON experiment definitions and a PowerShell experiment runner.
- Portable Web Debugger workspace definitions.
- Memory snapshot/export helper with provenance metadata.
- Offline byte-level RAM diff engine with address/delta/XOR output and optional CSV.
- Persistent JSON annotations for frames, memory, routines, sprites, checkpoints, experiments and events.
- `/api/v1/schema` discovery document in the HTTP bridge.
- Expanded roadmap for safe-boundary snapshots, native bulk export, SSE, checkpoint genealogy, deterministic A/B/N and cross-game provider separation.

## Safety boundary
The HTTP bridge remains localhost-only. Generic command execution remains allowlisted and rejects shell metacharacters. Large RAM export remains capped until native bulk export exists.

## Experiment example
```powershell
.\tools\powershell\Invoke-ChaseHQExperiment.ps1 .\experiments\core\ioc-pulse-example.json
```

## Memory diff example
```powershell
Import-Module .\tools\powershell\ChaseHQResearch.psm1 -Force
$chq=Connect-ChaseHQ
Save-ChaseHQMemorySnapshot $chq before A 100000 100FFF
# controlled intervention
Save-ChaseHQMemorySnapshot $chq after A 100000 100FFF
Compare-ChaseHQMemoryFiles .\evidence\memory\before-A-100000-100FFF.bin .\evidence\memory\after-A-100000-100FFF.bin -BaseAddress 0x100000 -OutFile .\evidence\memory\diff.csv
```
