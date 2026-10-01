# HTTP Research API bridge — v0.61.2

`Start-ChaseHQWeb.ps1` is now both the Web Debugger host and a localhost-only HTTP automation bridge. It deliberately consumes the native debugger API through `chqctl`; the emulator remains authoritative.

Base URL: `http://127.0.0.1:<HttpPort>/api/v1`

Implemented endpoints include `GET /status`, `GET /capabilities`, `GET /input/ports`, `GET /events`, `GET /sprites`, `GET /memory/{A|B}/{hex}?width=8|16|32`, POST control endpoints, `POST /input/ioc/pulse`, `POST /command`, `POST /evidence/capture`, and `GET /export/memory?cpu=A&start=100000&end=1000ff`.

The memory export is intentionally conservative: it performs debugger-safe byte reads and returns `application/octet-stream`. It is suitable for targeted live captures now; a future native bulk-export endpoint should replace it for large regions.

PowerShell module: `tools/powershell/ChaseHQResearch.psm1`. It uses `Invoke-RestMethod` for structured calls and `Invoke-WebRequest` for binary downloads.

Examples:

```powershell
Import-Module .\tools\powershell\ChaseHQResearch.psm1 -Force
$chq = Connect-ChaseHQ -Port 37680
Get-ChaseHQStatus $chq
Pause-ChaseHQ $chq
Invoke-ChaseHQIocPulse $chq -Port 3 -Mask 20 -Frames 3
Export-ChaseHQMemory $chq -Cpu A -Start 100000 -End 1000ff -Path .\ram.bin
```

Security: the bridge binds only to `127.0.0.1`; generic command execution remains allow-listed and rejects shell metacharacters. It is a research interface, not an Internet-facing service.
