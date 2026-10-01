[CmdletBinding(DefaultParameterSetName='Start')]
param(
    [Parameter(ParameterSetName='Start')]
    [ValidateSet('Gameplay','Boot','Attract','TargetPostFinalHit','StageEnd')]
    [string]$Mode = 'Gameplay',

    [Parameter(ParameterSetName='Start')]
    [string]$Checkpoint,

    [Parameter(ParameterSetName='Start')]
    [string]$Roms = '.\roms\chasehq',

    [Parameter(ParameterSetName='Start')]
    [int]$Port = 37600,

    [Parameter(ParameterSetName='Start')]
    [int]$HttpPort = 37680,

    [Parameter(ParameterSetName='Start')]
    [switch]$NoWeb,

    [Parameter(ParameterSetName='Start')]
    [switch]$NoBrowser,

    [Parameter(ParameterSetName='Start')]
    [switch]$Attach,

    [Parameter(ParameterSetName='Start')]
    [string[]]$PrimeCommand = @(),

    [Parameter(ParameterSetName='Start')]
    [string[]]$EmulatorArgs = @(),

    [Parameter(ParameterSetName='Start')]
    [switch]$Restart,

    [Parameter(ParameterSetName='Health', Mandatory=$true)]
    [switch]$HealthCheck,

    [Parameter(ParameterSetName='Diagnostics', Mandatory=$true)]
    [switch]$Diagnostics,

    [Parameter(ParameterSetName='Stop', Mandatory=$true)]
    [switch]$Stop,

    [Parameter(ParameterSetName='Support', Mandatory=$true)]
    [switch]$SupportBundle
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version 2.0

$root = if ($PSScriptRoot) { (Resolve-Path $PSScriptRoot).Path } else { (Get-Location).Path }
Set-Location $root
$runtimeDir = Join-Path $root '.chq'
$sessionFile = Join-Path $runtimeDir 'active-session.json'
$startupFile = Join-Path $runtimeDir 'startup-session.json'

function Stop-RecordedProcess([object]$Id, [string]$Label) {
    # PowerShell unwraps Nullable[int] values to plain System.Int32 objects, so
    # accessing .Value is not reliable under StrictMode.  Normalise whatever
    # came from JSON (number/string/null) into an integer instead.
    if ($null -eq $Id) { return }
    $pidValue = 0
    if (-not [int]::TryParse([string]$Id, [ref]$pidValue)) { return }
    if ($pidValue -le 0) { return }

    $p = Get-Process -Id $pidValue -ErrorAction SilentlyContinue
    if ($null -ne $p) {
        Write-Host "Stopping $Label (PID $pidValue)..." -ForegroundColor Yellow
        Stop-Process -Id $pidValue -Force -ErrorAction SilentlyContinue
    } else {
        Write-Host "$Label PID $pidValue is no longer running." -ForegroundColor DarkGray
    }
}

function Read-RecordedSession {
    if (-not (Test-Path $sessionFile)) { return $null }
    try { return (Get-Content $sessionFile -Raw | ConvertFrom-Json) } catch { throw "Could not read recorded session file '$sessionFile': $($_.Exception.Message)" }
}
function Stop-RecordedSession([switch]$Quiet) {
    $s = Read-RecordedSession
    if ($null -eq $s -and (Test-Path $startupFile)) { try { $s=Get-Content $startupFile -Raw|ConvertFrom-Json } catch {} }
    if ($null -eq $s) { if(-not $Quiet){Write-Host 'No recorded ChaseHQ session or startup attempt is active.' -ForegroundColor Yellow}; return $null }
    $webRecordedPid = if ($s.PSObject.Properties.Name -contains 'webPid') { $s.webPid } else { $null }
    $emuRecordedPid = if ($s.PSObject.Properties.Name -contains 'emulatorPid') { $s.emulatorPid } else { $null }
    Stop-RecordedProcess $webRecordedPid 'Web Workbench'
    Stop-RecordedProcess $emuRecordedPid 'ChaseHQNative'
    Remove-Item $sessionFile,$startupFile -Force -ErrorAction SilentlyContinue
    if(-not $Quiet){Write-Host 'ChaseHQ session stopped.' -ForegroundColor Green}
    return $s
}
function Test-TcpPortOpen([int]$TestPort) {
    try { $c=New-Object Net.Sockets.TcpClient; $ar=$c.BeginConnect('127.0.0.1',$TestPort,$null,$null); if(-not $ar.AsyncWaitHandle.WaitOne(250)){$c.Close();return $false};$c.EndConnect($ar);$c.Close();return $true } catch { return $false }
}
function Wait-PortsReleased([int[]]$Ports,[int]$TimeoutSeconds=10) {
    $deadline=(Get-Date).AddSeconds($TimeoutSeconds)
    do { $busy=@($Ports|Where-Object{$_ -gt 0 -and (Test-TcpPortOpen $_)}); if($busy.Count -eq 0){return};Start-Sleep -Milliseconds 200 } while((Get-Date)-lt $deadline)
    throw ('Timed out waiting for port(s) to be released: '+($busy -join ', '))
}
function Stop-StaleWorkbenchHosts([int[]]$Ports) {
    try {
        $requested=@{}
        foreach($p in @($Ports|Where-Object{$_ -gt 0}|Select-Object -Unique)){$requested[[int]$p]=$true}
        $escaped=[regex]::Escape($root)
        $hosts=@(Get-CimInstance Win32_Process -ErrorAction SilentlyContinue | Where-Object {
            $_.ProcessId -gt 4 -and $_.CommandLine -and $_.CommandLine -match 'Start-ChaseHQWeb\.ps1'
        })
        foreach($h in $hosts){
            $cmd=[string]$h.CommandLine
            $hostHttpPort=0
            $m=[regex]::Match($cmd,'(?i)(?:^|\s)-HttpPort\s+["'']?(\d+)["'']?(?=\s|$)')
            if($m.Success){[void][int]::TryParse($m.Groups[1].Value,[ref]$hostHttpPort)}
            $matchesRequested=$false
            if($hostHttpPort -gt 0){
                $matchesRequested=$requested.ContainsKey($hostHttpPort) -or $requested.ContainsKey($hostHttpPort+1)
            } elseif($cmd -match $escaped) {
                # Fallback for very old launchers that did not expose -HttpPort in their command line.
                $matchesRequested=$true
            }
            if($matchesRequested){
                Write-Host "Stopping stale Web Workbench host (PID $($h.ProcessId), HttpPort $hostHttpPort)..." -ForegroundColor Yellow
                Stop-Process -Id ([int]$h.ProcessId) -Force -ErrorAction SilentlyContinue
            }
        }
    } catch {}
}
function Stop-PortOwners([int[]]$Ports) {
    $owners=@()
    foreach($port in @($Ports|Where-Object{$_ -gt 0}|Select-Object -Unique)){
        try{$owners += @(Get-NetTCPConnection -LocalPort $port -State Listen -ErrorAction SilentlyContinue|Select-Object -ExpandProperty OwningProcess)}catch{}
    }
    if(@($owners|Where-Object{$_ -eq 4}).Count -gt 0){
        # HTTP.sys reports listeners as PID 4/System. Never attempt to terminate System;
        # stop the user-mode Workbench host that owns the HTTP request queue instead.
        Stop-StaleWorkbenchHosts -Ports $Ports
        Start-Sleep -Milliseconds 500
    }
    foreach($pidValue in @($owners|Where-Object{$_ -gt 4}|Select-Object -Unique)){
        $proc=Get-Process -Id $pidValue -ErrorAction SilentlyContinue
        if($proc){Write-Host "Stopping process holding ChaseHQ port (PID $pidValue, $($proc.ProcessName))..." -ForegroundColor Yellow;Stop-Process -Id $pidValue -Force -ErrorAction SilentlyContinue}
    }
}
function Show-SessionHealth([switch]$Detailed) {
    $s=Read-RecordedSession
    $source='active session'
    if($null -eq $s -and (Test-Path $startupFile)){try{$s=Get-Content $startupFile -Raw|ConvertFrom-Json;$source='latest startup attempt'}catch{}}
    if($null -eq $s){Write-Host 'No recorded ChaseHQ session or startup attempt is available.' -ForegroundColor Yellow;return}
    Write-Host "=== ChaseHQ $source health ===" -ForegroundColor Cyan
    foreach($name in @('build','state','phase','mode','started','updated','checkpoint','evidence','message','diagnosticZip')){if($s.PSObject.Properties.Name -contains $name){$v=$s.$name;if($null -ne $v -and [string]$v -ne ''){Write-Host (('{0,-13} {1}' -f ($name+':'),$v))}}}
    foreach($pair in @(@('Emulator','emulatorPid'),@('Workbench','webPid'))){if($s.PSObject.Properties.Name -contains $pair[1]){$pidValue=0;$ok=[int]::TryParse([string]$s.($pair[1]),[ref]$pidValue);$alive=$ok -and $pidValue -gt 0 -and $null -ne (Get-Process -Id $pidValue -ErrorAction SilentlyContinue);Write-Host (('{0,-12} PID {1,-7} {2}' -f $pair[0],$pidValue,$(if($alive){'RUNNING'}else{'NOT RUNNING'}))) -ForegroundColor $(if($alive){'Green'}else{'Yellow'})}}
    if($s.PSObject.Properties.Name -contains 'httpPort' -and $s.httpPort){try{$r=Invoke-WebRequest -UseBasicParsing -Uri "http://127.0.0.1:$($s.httpPort)/api/health" -TimeoutSec 2;$j=$r.Content|ConvertFrom-Json;Write-Host "Workbench:   HTTP $($r.StatusCode), scripts=$($j.scriptCount), frontend=$($j.frontendBootstrap)" -ForegroundColor Green}catch{Write-Host "Workbench:   unavailable on $($s.httpPort): $($_.Exception.Message)" -ForegroundColor Yellow}}
    if($Detailed){foreach($f in 'web-startup.log','web-listener.log','web-stdout.log','web-stderr.log','web-command-diagnostics.log','web-frontend-errors.log'){ $fp=Join-Path $runtimeDir $f;if(Test-Path $fp){Write-Host "`n--- $f (tail 40) ---" -ForegroundColor DarkCyan;Get-Content $fp -Tail 40 -ErrorAction SilentlyContinue}};if($s.PSObject.Properties.Name -contains 'evidence' -and $s.evidence){foreach($f in 'native-stdout.log','native-stderr.log'){$fp=Join-Path ([string]$s.evidence) $f;if(Test-Path $fp){Write-Host "`n--- $f (tail 40) ---" -ForegroundColor DarkCyan;Get-Content $fp -Tail 40 -ErrorAction SilentlyContinue}}}}
}

function New-SupportBundle {
    New-Item -ItemType Directory -Path $runtimeDir -Force|Out-Null
    $stamp=Get-Date -Format 'yyyyMMdd-HHmmss-fff';$outDir=Join-Path $root 'evidence\diagnostics';New-Item -ItemType Directory -Path $outDir -Force|Out-Null;$tmp=Join-Path $runtimeDir ('support-'+$stamp);New-Item -ItemType Directory -Path $tmp -Force|Out-Null
    try{
      foreach($f in @('active-session.json','startup-session.json','web-startup.log','web-listener.log','web-stdout.log','web-stderr.log','web-command-diagnostics.log','web-frontend-errors.log','frontend-ready.json')){$src=Join-Path $runtimeDir $f;if(Test-Path $src){Copy-Item $src (Join-Path $tmp $f) -Force}}
      foreach($f in @('build-local.json','research\feature-manifest.json','src\version.h')){$src=Join-Path $root $f;if(Test-Path $src){$dst=Join-Path $tmp (($f -replace '[\\/]','_'));Copy-Item $src $dst -Force}}
      $s=Read-RecordedSession;if($null -eq $s -and (Test-Path $startupFile)){try{$s=Get-Content $startupFile -Raw|ConvertFrom-Json}catch{}}
      if($s -and $s.PSObject.Properties.Name -contains 'evidence' -and $s.evidence -and (Test-Path $s.evidence)){foreach($f in @('native-stdout.log','native-stderr.log','session.json','run_phase.txt')){$src=Join-Path ([string]$s.evidence) $f;if(Test-Path $src){Copy-Item $src (Join-Path $tmp $f) -Force}}}
      $ports=[ordered]@{native37600=(Test-TcpPortOpen 37600);web37680=(Test-TcpPortOpen 37680);backend37681=(Test-TcpPortOpen 37681);created=(Get-Date).ToString('o')};$ports|ConvertTo-Json|Set-Content (Join-Path $tmp 'ports.json') -Encoding UTF8
      $zip=Join-Path $outDir ('support-bundle-'+$stamp+'.zip');Compress-Archive -Path (Join-Path $tmp '*') -DestinationPath $zip -Force;Write-Host "Support bundle: $zip" -ForegroundColor Green;return $zip
    }finally{Remove-Item $tmp -Recurse -Force -ErrorAction SilentlyContinue}
}

if ($PSCmdlet.ParameterSetName -eq 'Stop') { Stop-RecordedSession | Out-Null; exit 0 }
if ($PSCmdlet.ParameterSetName -eq 'Health') { Show-SessionHealth; exit 0 }
if ($PSCmdlet.ParameterSetName -eq 'Diagnostics') { Show-SessionHealth -Detailed; exit 0 }
if ($PSCmdlet.ParameterSetName -eq 'Support') { New-SupportBundle | Out-Null; exit 0 }
if ($Restart) {
    Write-Host 'Restart requested: stopping recorded session first...' -ForegroundColor Yellow
    $old=Stop-RecordedSession -Quiet
    $ports=@($Port,$HttpPort,$HttpPort+1)
    if($old){if($old.nativePort){$ports+=[int]$old.nativePort};if($old.httpPort){$ports+=[int]$old.httpPort;$ports+=([int]$old.httpPort+1)}}
    try{Wait-PortsReleased ($ports|Select-Object -Unique) 3}catch{
        Write-Host 'Recorded processes did not release ports; stopping actual port owners...' -ForegroundColor Yellow
        Stop-PortOwners ($ports|Select-Object -Unique)
        Wait-PortsReleased ($ports|Select-Object -Unique) 10
    }
    Write-Host 'Previous session stopped and ports released.' -ForegroundColor Green
}

if ($Port -lt 1 -or $Port -gt 65535) { throw '-Port must be 1..65535.' }
if ($HttpPort -lt 1 -or $HttpPort -gt 65534) { throw '-HttpPort must be 1..65534 (v0.65.0 reserves HttpPort+1 for the private Web backend).' }
if ($Port -eq $HttpPort) { throw '-Port and -HttpPort must be different.' }
if ($Port -eq ($HttpPort + 1)) { throw '-Port must not equal HttpPort+1 (reserved for the private Web backend).' }

$modeCheckpoints = @{
    Gameplay           = '.\checkpoints\stage1-gameplay-2064.chqstate'
    Boot               = $null
    Attract            = '.\checkpoints\stage1-driving-2352.chqstate'
    TargetPostFinalHit = '.\checkpoints\stage1-target-post-final-hit-9548.chqstate'
    StageEnd           = '.\checkpoints\stage1-end-level-9988.chqstate'
}

$selectedCheckpoint = $Checkpoint
if ([string]::IsNullOrWhiteSpace($selectedCheckpoint)) {
    $selectedCheckpoint = $modeCheckpoints[$Mode]
}

$research = Join-Path $root 'Start-ChaseHQResearch.ps1'
$web = Join-Path $root 'Start-ChaseHQWeb.ps1'
if (-not (Test-Path $research)) { throw "Missing launcher: $research" }
if (-not $NoWeb -and -not (Test-Path $web)) { throw "Missing web launcher: $web" }

$beforePids = @((Get-Process -Name 'ChaseHQNative' -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Id))

# IMPORTANT: use a hashtable for named-parameter splatting.  An array here is
# positional splatting in PowerShell and would bind '-Checkpoint' to -Port.
$researchArgs = @{
    NoShell     = $true
    Port        = $Port
    Roms        = $Roms
    SessionName = ('launcher-' + $Mode.ToLowerInvariant())
    StartupRecordPath = $startupFile
}
if ($Attach) { $researchArgs.Attach = $true }
if ($selectedCheckpoint) { $researchArgs.Checkpoint = $selectedCheckpoint }
if ($PrimeCommand.Count -gt 0) { $researchArgs.PrimeCommand = @($PrimeCommand) }
if ($EmulatorArgs.Count -gt 0) { $researchArgs.EmulatorArgs = @($EmulatorArgs) }

Write-Host ''
Write-Host '=== ChaseHQ Native one-command launcher ===' -ForegroundColor Cyan
Write-Host "Mode:       $Mode"
if ($selectedCheckpoint) { Write-Host "Checkpoint: $selectedCheckpoint" } else { Write-Host 'Checkpoint: none (boot)' }
Write-Host "Native API: http://127.0.0.1:$Port"
if (-not $NoWeb) { Write-Host "Workbench:  http://127.0.0.1:$HttpPort/" }
Write-Host ''

Remove-Item $startupFile -Force -ErrorAction SilentlyContinue
try { & $research @researchArgs; if ($LASTEXITCODE -ne 0) { throw "Research launcher failed with exit code $LASTEXITCODE." } } catch { Write-Host ''; Write-Host 'LAUNCH FAILED - automatic diagnostics follow' -ForegroundColor Red; Show-SessionHealth -Detailed; throw }

$after = @(Get-Process -Name 'ChaseHQNative' -ErrorAction SilentlyContinue)
$emulatorPid = 0
if(Test-Path $startupFile){try{$sr=Get-Content $startupFile -Raw|ConvertFrom-Json;if($sr.emulatorPid){$emulatorPid=[int]$sr.emulatorPid}}catch{}}
if (-not $Attach -and $emulatorPid -le 0) {
    $new = @($after | Where-Object { $_.Id -notin $beforePids } | Sort-Object StartTime -Descending)
    if ($new.Count -gt 0) { $emulatorPid = [int]$new[0].Id }
    elseif ($after.Count -eq 1) { $emulatorPid = [int]$after[0].Id }
}

$webPid = 0
try {
if (-not $NoWeb) {
    # Fail early if another listener already owns the requested HTTP port.
    try {
        $existing = Invoke-WebRequest -UseBasicParsing -Uri "http://127.0.0.1:$HttpPort/api/v1/status" -TimeoutSec 1
        if ($existing.StatusCode -eq 200) {
            throw "A ChaseHQ Web Workbench is already responding on port $HttpPort. Stop it first or choose -HttpPort."
        }
    } catch {
        if ($_.Exception.Message -like 'A ChaseHQ Web Workbench*') { throw }
    }

    New-Item -ItemType Directory -Path $runtimeDir -Force | Out-Null
    $webStdout = Join-Path $runtimeDir 'web-stdout.log'
    $webStderr = Join-Path $runtimeDir 'web-stderr.log'
    $webListener = Join-Path $runtimeDir 'web-listener.log'
    Remove-Item $webStdout,$webStderr,$webListener,(Join-Path $runtimeDir 'frontend-ready.json') -Force -ErrorAction SilentlyContinue
    $sessionRoot = $null
    $sessionsDir = Join-Path $root 'evidence\sessions'
    if (Test-Path $sessionsDir) {
        $latestSession = Get-ChildItem $sessionsDir -Directory -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending | Select-Object -First 1
        if ($latestSession) { $sessionRoot = $latestSession.FullName }
    }
    $webArgs = @('-NoLogo','-NoProfile','-ExecutionPolicy','Bypass','-File',$web,'-Port',[string]$Port,'-HttpPort',[string]$HttpPort)
    if ($sessionRoot) { $webArgs += @('-SessionRoot',$sessionRoot,'-EvidenceRoot',(Join-Path $sessionRoot 'evidence')) }
    $wp = Start-Process -FilePath 'powershell.exe' -ArgumentList $webArgs -WorkingDirectory $root -WindowStyle Hidden -RedirectStandardOutput $webStdout -RedirectStandardError $webStderr -PassThru
    $webPid = [int]$wp.Id
    try{if(Test-Path $startupFile){$sr=Get-Content $startupFile -Raw|ConvertFrom-Json;$sr|Add-Member -NotePropertyName webPid -NotePropertyValue $webPid -Force;$sr|Add-Member -NotePropertyName httpPort -NotePropertyValue $HttpPort -Force;$sr.phase='WEB_WAIT';$sr.updated=(Get-Date).ToString('o');$sr|ConvertTo-Json -Depth 8|Set-Content $startupFile -Encoding UTF8}}catch{}

    Write-Host "Waiting for Web Workbench on 127.0.0.1:$HttpPort..." -ForegroundColor Yellow
    $ready = $false
    $deadline = (Get-Date).AddSeconds(30)
    while ((Get-Date) -lt $deadline) {
        if ($wp.HasExited) { break }
        try {
            $healthProbe = Invoke-WebRequest -UseBasicParsing -Uri "http://127.0.0.1:$HttpPort/api/health" -TimeoutSec 1
            $statusProbe = Invoke-WebRequest -UseBasicParsing -Uri "http://127.0.0.1:$HttpPort/api/v1/status" -TimeoutSec 1
            $scriptsProbe = Invoke-WebRequest -UseBasicParsing -Uri "http://127.0.0.1:$HttpPort/api/v1/scripts" -TimeoutSec 1
            $rootProbe = Invoke-WebRequest -UseBasicParsing -Uri "http://127.0.0.1:$HttpPort/" -TimeoutSec 1
            $hj=$healthProbe.Content|ConvertFrom-Json;$sj=$scriptsProbe.Content|ConvertFrom-Json
            if ($healthProbe.StatusCode -eq 200 -and $statusProbe.StatusCode -eq 200 -and $rootProbe.StatusCode -eq 200 -and @($sj.scripts).Count -gt 0) { $ready = $true; break }
        } catch {}
        Start-Sleep -Milliseconds 250
    }
    Start-Sleep -Milliseconds 500
    $wp.Refresh()
    if (-not $ready -or $wp.HasExited) {
        $out = if (Test-Path $webListener) { Get-Content $webListener -Raw -ErrorAction SilentlyContinue } elseif (Test-Path $webStdout) { Get-Content $webStdout -Raw -ErrorAction SilentlyContinue } else { '' }
        $err = if (Test-Path $webStderr) { Get-Content $webStderr -Raw -ErrorAction SilentlyContinue } else { '' }
        if($wp.HasExited){$err = ('Workbench process exit code: '+$wp.ExitCode+"`n"+$err)}
        if (-not $wp.HasExited) { Stop-Process -Id $webPid -Force -ErrorAction SilentlyContinue }
        throw "Web Workbench failed to stay ready on port $HttpPort.`n--- stdout ---`n$out`n--- stderr ---`n$err"
    }
    Write-Host "Web Workbench ready (PID $webPid)." -ForegroundColor Green
    try{if(Test-Path $startupFile){$sr=Get-Content $startupFile -Raw|ConvertFrom-Json;$sr.phase='WEB_READY';$sr.updated=(Get-Date).ToString('o');$sr|ConvertTo-Json -Depth 8|Set-Content $startupFile -Encoding UTF8}}catch{}
}
} catch {
    $msg=$_.Exception.Message;try{if(Test-Path $startupFile){$sr=Get-Content $startupFile -Raw|ConvertFrom-Json;$sr|Add-Member -NotePropertyName state -NotePropertyValue 'failed' -Force;$sr|Add-Member -NotePropertyName phase -NotePropertyValue 'WEB_WAIT' -Force;$sr|Add-Member -NotePropertyName message -NotePropertyValue $msg -Force;$sr.updated=(Get-Date).ToString('o');$sr|ConvertTo-Json -Depth 8|Set-Content $startupFile -Encoding UTF8}}catch{};Write-Host '';Write-Host 'WEB STARTUP FAILED - automatic diagnostics follow' -ForegroundColor Red;Show-SessionHealth -Detailed;New-SupportBundle|Out-Null;throw
}

New-Item -ItemType Directory -Path $runtimeDir -Force | Out-Null
[ordered]@{
    schema = 'chq-active-session-v1'
    build = '0.66.9.0'
    state = 'ready'
    phase = 'READY'
    started = (Get-Date).ToString('o')
    mode = $Mode
    checkpoint = $selectedCheckpoint
    nativePort = $Port
    httpPort = if ($NoWeb) { $null } else { $HttpPort }
    emulatorPid = $emulatorPid
    webPid = $webPid
    evidence = $(if(Test-Path $startupFile){try{(Get-Content $startupFile -Raw|ConvertFrom-Json).evidence}catch{$null}}else{$null})
} | ConvertTo-Json -Depth 4 | Set-Content -Path $sessionFile -Encoding UTF8

if (-not $NoWeb -and -not $NoBrowser) {
    $bootstrapNonce=[DateTimeOffset]::UtcNow.ToUnixTimeMilliseconds(); Start-Process "http://127.0.0.1:$HttpPort/?bootstrap=$bootstrapNonce"
    Write-Host 'Waiting for browser frontend bootstrap self-test...' -ForegroundColor Yellow
    $frontReady=$false;$frontDeadline=(Get-Date).AddSeconds(20)
    while((Get-Date)-lt $frontDeadline){try{$h=(Invoke-WebRequest -UseBasicParsing -Uri "http://127.0.0.1:$HttpPort/api/health" -TimeoutSec 1).Content|ConvertFrom-Json;if($h.frontendBootstrap -eq 'ready'){$frontReady=$true;break}}catch{};Start-Sleep -Milliseconds 300}
    if(-not $frontReady){Write-Host 'FRONTEND BOOTSTRAP DID NOT REPORT READY.' -ForegroundColor Red;$ff=Join-Path $runtimeDir 'web-frontend-errors.log';if(Test-Path $ff){Write-Host '--- frontend errors ---' -ForegroundColor Yellow;Get-Content $ff -Tail 50};try{$ar=Get-Content $sessionFile -Raw|ConvertFrom-Json;$ar.state='failed';$ar.phase='FRONTEND_BOOTSTRAP';$ar|Add-Member -NotePropertyName message -NotePropertyValue 'Browser frontend bootstrap did not report ready' -Force;$ar|ConvertTo-Json -Depth 8|Set-Content $sessionFile -Encoding UTF8}catch{};Write-Host 'Run .\Start-ChaseHQ.ps1 -Diagnostics for the full snapshot.' -ForegroundColor Yellow;New-SupportBundle|Out-Null;throw 'Workbench server started, but browser frontend bootstrap self-test failed or timed out.'}
    Write-Host 'Browser frontend bootstrap: READY' -ForegroundColor Green
    try{if(Test-Path $startupFile){$sr=Get-Content $startupFile -Raw|ConvertFrom-Json;$sr.state='ready';$sr.phase='READY';$sr.updated=(Get-Date).ToString('o');$sr|ConvertTo-Json -Depth 8|Set-Content $startupFile -Encoding UTF8}}catch{}
}

Write-Host ''
Write-Host 'READY' -ForegroundColor Green
Write-Host "  Emulator:   $(if($emulatorPid){'PID ' + $emulatorPid}else{'attached/existing'})"
Write-Host "  Native API: 127.0.0.1:$Port"
if (-not $NoWeb) { Write-Host "  Workbench:  http://127.0.0.1:$HttpPort/" }
Write-Host "  Mode:       $Mode"
if ($selectedCheckpoint) { Write-Host "  Checkpoint: $selectedCheckpoint" }
Write-Host ''
Write-Host 'Stop this recorded session with:' -ForegroundColor DarkGray
Write-Host '  .\Start-ChaseHQ.ps1 -Stop' -ForegroundColor DarkGray
Write-Host 'Restart cleanly with:' -ForegroundColor DarkGray
Write-Host '  .\Start-ChaseHQ.ps1 -Restart' -ForegroundColor DarkGray
Write-Host 'Health/diagnostics:' -ForegroundColor DarkGray
Write-Host '  .\Start-ChaseHQ.ps1 -HealthCheck    # or -Diagnostics' -ForegroundColor DarkGray
Write-Host '  .\Start-ChaseHQ.ps1 -SupportBundle' -ForegroundColor DarkGray
