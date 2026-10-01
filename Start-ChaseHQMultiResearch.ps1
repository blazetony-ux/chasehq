[CmdletBinding()]
param(
    [int]$Instances = 2,
    [int]$BasePort = 37600,
    [int]$BaseHttpPort = 37680,
    [switch]$StartWeb,
    [string]$Checkpoint,
    [string]$Roms = '.\roms\chasehq',
    [string]$SessionName = 'ab',
    [string[]]$EmulatorArgs = @(),
    [switch]$NoShell
)
$ErrorActionPreference='Stop'
if($Instances -lt 2 -or $Instances -gt 16){ throw '-Instances must be 2..16.' }
$root=$PSScriptRoot
$instanceInfo=@()
for($i=0;$i -lt $Instances;$i++){
    $port=$BasePort+$i
    $name='{0}-{1}' -f $SessionName,[char](65+$i)
    $params=@('-NoLogo','-ExecutionPolicy','Bypass','-File',(Join-Path $root 'Start-ChaseHQResearch.ps1'),'-Port',$port,'-SessionName',$name,'-Roms',$Roms)
    if($Checkpoint){$params+=@('-Checkpoint',$Checkpoint)}
    if($NoShell){$params+='-NoShell'}
    foreach($a in $EmulatorArgs){$params+=@('-EmulatorArgs',$a)}
    Start-Process powershell.exe -ArgumentList $params -WorkingDirectory $root | Out-Null
    Start-Sleep -Milliseconds 350
    $httpPort=$BaseHttpPort+$i
    if($StartWeb){ Start-Process powershell.exe -ArgumentList @('-NoLogo','-ExecutionPolicy','Bypass','-File',(Join-Path $root 'Start-ChaseHQWeb.ps1'),'-Port',$port,'-HttpPort',$httpPort) -WorkingDirectory $root | Out-Null }
    $instanceInfo += [pscustomobject]@{name=$name;debugPort=$port;httpPort=$httpPort;api=('http://127.0.0.1:{0}/api/v1' -f $httpPort)}
}
$manifest=[pscustomobject]@{schema='chq-instances-v1';created=(Get-Date).ToString('o');instances=$instanceInfo}
$manifest|ConvertTo-Json -Depth 5|Set-Content (Join-Path $root 'instances.json') -Encoding UTF8
Write-Host "Started $Instances research instances on ports $BasePort..$($BasePort+$Instances-1)."
if($StartWeb){Write-Host "HTTP APIs: $BaseHttpPort..$($BaseHttpPort+$Instances-1)"}
Write-Host "Instance manifest: $(Join-Path $root 'instances.json')"
