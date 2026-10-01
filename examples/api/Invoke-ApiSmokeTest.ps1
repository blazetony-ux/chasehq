param([int]$HttpPort=37680)
Import-Module "$PSScriptRoot\..\..\tools\powershell\ChaseHQResearch.psm1" -Force
$c=Connect-ChaseHQ -Port $HttpPort
Get-ChaseHQStatus $c | Format-List
Get-ChaseHQCapabilities $c | Format-List
Get-ChaseHQMemory $c -Cpu A -Address 10A096 -Width 16 | Format-List
Invoke-ChaseHQIocPulse $c -Port 3 -Mask 20 -Frames 3 | Format-List
