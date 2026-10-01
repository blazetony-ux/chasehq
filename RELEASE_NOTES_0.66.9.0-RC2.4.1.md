# ChaseHQ-Native v0.66.9.0-RC2.4.1

RC2.4.1 is a launcher hotfix for `Start-ChaseHQ.ps1 -Restart` when a previous ChaseHQ Web Workbench from another extracted build still owns the standard HTTP port through HTTP.sys.

## Restart cleanup fix

`Get-NetTCPConnection` reports PowerShell `HttpListener` listeners as PID 4/System. RC2.4 correctly refused to terminate PID 4, but its fallback stale-host search was restricted to the current build folder. A Workbench left running from RC2.3 or another adjacent build could therefore keep port 37680 open and cause `-Restart` to time out.

The stale-host cleanup is now **port-scoped rather than build-folder-scoped**. It identifies `Start-ChaseHQWeb.ps1` processes by their `-HttpPort` argument and stops only hosts whose public port or private backend port overlaps the ports requested by the new session. Other ChaseHQ Workbench instances using different ports are left alone.

No emulator, TC0100SCN renderer, Research API, schema or `.chqscript` behavior changed from RC2.4.
