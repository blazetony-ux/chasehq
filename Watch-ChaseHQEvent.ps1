[CmdletBinding()]
param(
 [int]$Port=37600,[ValidateSet('A','B')][string]$Cpu='A',[Parameter(Mandatory)][string]$Range,
 [ValidateSet('write','read','rw')][string]$Access='write',[switch]$Change,
 [ValidateSet('checkpoint','screenshot','pause','command')][string]$Action='checkpoint',
 [string]$Output='.\evidence\triggered',[string]$Command,[int]$PollMs=100,[switch]$Once
)
$ErrorActionPreference='Stop';$root=$PSScriptRoot;$ctl=Join-Path $root 'out\build\x64-Debug\chqctl.exe';if(-not(Test-Path $ctl)){throw "Missing $ctl"}
New-Item -ItemType Directory -Path $Output -Force|Out-Null
$opts=@('watch','add',$Cpu,$Range,$Access);if($Change){$opts+='change'}
$r=& $ctl --port $Port @opts;if($LASTEXITCODE){throw $r};Write-Host $r
$last='';while($true){$e=(& $ctl --port $Port events tail 1 2>&1|Out-String).Trim();if($e -and $e -ne $last -and $e -match "CPU=$Cpu" -and $e -match 'frame=(\d+)'){$f=$Matches[1];$stamp=Get-Date -Format 'yyyyMMdd-HHmmss-fff';switch($Action){'checkpoint'{& $ctl --port $Port checkpoint save (Join-Path $Output "event-f$f-$stamp.chqstate")} 'screenshot'{& $ctl --port $Port screenshot (Join-Path $Output "event-f$f-$stamp.png")} 'pause'{& $ctl --port $Port pause} 'command'{if(-not $Command){throw '-Command required for Action command'};& $ctl --port $Port $Command}};$last=$e;if($Once){break}};Start-Sleep -Milliseconds $PollMs}
