[CmdletBinding()]
param(
    [string]$Preset,
    [string]$Session,
    [string]$Roms = ".\roms\chasehq",
    [string]$Checkpoint,
    [string]$Experiment,
    [string[]]$EmulatorArgs = @(),
    [string[]]$PrimeCommand = @(),
    [int]$Port = 37600,
    [switch]$Attach,
    [switch]$NoShell,
    [switch]$Fresh,
    [switch]$NoBuildCheck,
    [switch]$NoCollisions,
    [switch]$InfiniteTime,
    [switch]$UnlimitedTurbo,
    [switch]$AutoTurbo,
    [double]$CorneringScale = 0,
    [double]$CorneringSpeedRetain = 0,
    [string]$CourseFollowController,
    [string]$EvidenceRoot,
    [string]$SessionName = "research",
    [switch]$NoEvidenceManifest,
    [int]$ApiTimeoutSeconds = 20,
    [string]$StartupRecordPath
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version 2.0

function Resolve-Root {
    if ($PSScriptRoot) { return (Resolve-Path $PSScriptRoot).Path }
    return (Get-Location).Path
}

function Get-ProjectBuildVersion([string]$Root) {
    $versionPath = Join-Path $Root 'src\version.h'
    if (Test-Path $versionPath -PathType Leaf) {
        $m = [regex]::Match((Get-Content $versionPath -Raw), 'kNativeVersion\s*=\s*"([^"]+)"')
        if ($m.Success) { return $m.Groups[1].Value }
    }
    $cmakePath = Join-Path $Root 'CMakeLists.txt'
    if (Test-Path $cmakePath -PathType Leaf) {
        $m = [regex]::Match((Get-Content $cmakePath -Raw), 'project\s*\(\s*ChaseHQNative\s+VERSION\s+([0-9.]+)', [Text.RegularExpressions.RegexOptions]::IgnoreCase)
        if ($m.Success) { return $m.Groups[1].Value }
    }
    return 'unknown'
}

function Quote-Arg([string]$Value) {
    if ($null -eq $Value) { return '""' }
    if ($Value -notmatch '[\s"]') { return $Value }
    return '"' + ($Value -replace '(\\*)"', '$1$1\"' -replace '(\\+)$', '$1$1') + '"'
}

function Test-Api([string]$Ctl, [int]$ApiPort) {
    try {
        & $Ctl --port $ApiPort status *> $null
        return ($LASTEXITCODE -eq 0)
    } catch { return $false }
}

function Invoke-Ctl([string]$Ctl, [int]$ApiPort, [string]$Command) {
    Write-Host "CHQ> $Command" -ForegroundColor Cyan
    $parts = @('--port', [string]$ApiPort)
    # Use chqctl's script parser for complex commands so quoted paths remain intact.
    $tmp = Join-Path $env:TEMP ("chq-prime-{0}-{1}.chqscript" -f $PID, [Guid]::NewGuid().ToString('N'))
    try {
        Set-Content -Path $tmp -Value $Command -Encoding ASCII
        & $Ctl @parts script $tmp
        if ($LASTEXITCODE -ne 0) { throw "chqctl command failed ($LASTEXITCODE): $Command" }
    } finally {
        Remove-Item $tmp -Force -ErrorAction SilentlyContinue
    }
}

function Get-SessionProperty($obj, [string]$name) {
    if ($null -eq $obj) { return $null }
    $prop = $obj.PSObject.Properties[$name]
    if ($null -eq $prop) { return $null }
    return $prop.Value
}


function Write-StartupRecord([string]$State,[string]$Phase,[object]$Process=$null,[string]$Message='',[string]$DiagnosticZip='') {
    if ([string]::IsNullOrWhiteSpace($StartupRecordPath)) { return }
    $dir=Split-Path $StartupRecordPath -Parent;if($dir){New-Item -ItemType Directory -Path $dir -Force|Out-Null}
    $pidValue=$null;$exitCode=$null
    if($Process){try{$pidValue=[int]$Process.Id}catch{};try{$Process.Refresh();if($Process.HasExited){$exitCode=[int]$Process.ExitCode}}catch{}}
    [ordered]@{schema='chq-startup-session-v1';build=$script:ProjectBuildVersion;updated=(Get-Date).ToString('o');state=$State;phase=$Phase;nativePort=$Port;emulatorPid=$pidValue;evidence=$evidencePath;roms=$Roms;checkpoint=$Checkpoint;message=$Message;exitCode=$exitCode;diagnosticZip=$DiagnosticZip}|ConvertTo-Json -Depth 6|Set-Content $StartupRecordPath -Encoding UTF8
}
function Tail-IfExists([string]$Path,[int]$Lines=80){if(Test-Path $Path){return (Get-Content $Path -Tail $Lines -ErrorAction SilentlyContinue|Out-String).Trim()}return ''}
function New-StartupBundle([string]$Reason,[object]$Process=$null){
    try{
      $diagDir=Join-Path $evidencePath 'diagnostics';New-Item -ItemType Directory -Path $diagDir -Force|Out-Null
      $stamp=Get-Date -Format 'yyyyMMdd-HHmmss-fff';$tmp=Join-Path $diagDir ('tmp-'+$stamp);New-Item -ItemType Directory -Path $tmp -Force|Out-Null
      Set-Content (Join-Path $tmp 'failure.txt') $Reason -Encoding UTF8
      foreach($f in @('native-stdout.log','native-stderr.log','session.json')){$src=Join-Path $evidencePath $f;if(Test-Path $src){Copy-Item $src (Join-Path $tmp $f) -Force}}
      if($StartupRecordPath -and (Test-Path $StartupRecordPath)){Copy-Item $StartupRecordPath (Join-Path $tmp 'startup-session.json') -Force}
      $zip=Join-Path $diagDir ('startup-failure-'+$stamp+'.zip');Compress-Archive -Path (Join-Path $tmp '*') -DestinationPath $zip -Force;Remove-Item $tmp -Recurse -Force -ErrorAction SilentlyContinue;return $zip
    }catch{return ''}
}
function Merge-SessionObject($obj) {
    if ($null -eq $obj) { return }
    $v = Get-SessionProperty $obj 'Roms'; if ($v) { $script:Roms = [string]$v }
    $v = Get-SessionProperty $obj 'Checkpoint'; if ($v) { $script:Checkpoint = [string]$v }
    $v = Get-SessionProperty $obj 'Experiment'; if ($v) { $script:Experiment = [string]$v }
    $v = Get-SessionProperty $obj 'Port'; if ($v) { $script:Port = [int]$v }
    $v = Get-SessionProperty $obj 'EvidenceRoot'; if ($v) { $script:EvidenceRoot = [string]$v }
    $v = Get-SessionProperty $obj 'EmulatorArgs'; if ($v) { $script:EmulatorArgs += @($v | ForEach-Object { [string]$_ }) }
    $v = Get-SessionProperty $obj 'PrimeCommands'; if ($v) { $script:PrimeCommand += @($v | ForEach-Object { [string]$_ }) }
}

$root = Resolve-Root
$script:ProjectBuildVersion = Get-ProjectBuildVersion $root
Set-Location $root

# v0.61.0: every launch can own an evidence namespace and machine-readable manifest.
$sessionStamp = Get-Date -Format 'yyyyMMdd_HHmmss'
$safeSessionName = ($SessionName -replace '[^A-Za-z0-9_.-]', '-')
if (-not $EvidenceRoot) { $EvidenceRoot = '.\evidence\sessions' }
$evidenceBase = if ([IO.Path]::IsPathRooted($EvidenceRoot)) { $EvidenceRoot } else { Join-Path $root $EvidenceRoot }
$evidencePath = Join-Path $evidenceBase $sessionStamp
if (-not $Attach) {
    $candidate=$evidencePath;$suffix=1
    while(Test-Path $candidate){$candidate=Join-Path $evidenceBase ($sessionStamp+'_'+('{0:D2}' -f $suffix));$suffix++}
    $evidencePath=$candidate
    New-Item -ItemType Directory -Path $evidencePath -Force | Out-Null
}

if ($Port -lt 1 -or $Port -gt 65535) { throw '-Port must be 1..65535.' }

if ($Preset) {
    $presetPath = Join-Path $root ("sessions\{0}.json" -f $Preset)
    if (-not (Test-Path $presetPath)) { throw "Preset not found: $presetPath" }
    Merge-SessionObject (Get-Content $presetPath -Raw | ConvertFrom-Json)
}
if ($Session) {
    $sessionPath = if ([IO.Path]::IsPathRooted($Session)) { $Session } else { Join-Path $root $Session }
    if (-not (Test-Path $sessionPath)) { throw "Session file not found: $sessionPath" }
    Merge-SessionObject (Get-Content $sessionPath -Raw | ConvertFrom-Json)
}

$emu = Join-Path $root 'out\build\x64-Debug\ChaseHQNative.exe'
$ctl = Join-Path $root 'out\build\x64-Debug\chqctl.exe'
if(-not(Test-Path $emu)){$emu=Join-Path $root 'out\build\x64-Debug\Debug\ChaseHQNative.exe'}
if(-not(Test-Path $ctl)){$ctl=Join-Path $root 'out\build\x64-Debug\Debug\chqctl.exe'}

if (-not $NoBuildCheck) {
    if (-not (Test-Path $emu)) { throw "Missing emulator: $emu`nBuild the project first." }
    if (-not (Test-Path $ctl)) { throw "Missing debugger client: $ctl`nBuild the project first." }
}

if (-not $Attach) {
    if (Test-Api $ctl $Port) { throw "A ChaseHQ Research API is already responding on port $Port. Use -Attach or choose another -Port." }

    $romPath = if ([IO.Path]::IsPathRooted($Roms)) { $Roms } else { Join-Path $root $Roms }
    if (-not (Test-Path $romPath)) { throw "ROM directory not found: $romPath" }
    $requiredRom = Join-Path $romPath 'b52-132.39'
    if (-not (Test-Path $requiredRom -PathType Leaf)) { throw "Required Chase H.Q. ROM missing before launch: $requiredRom. Run .\Prepare-Project.bat first." }

    $args = @('--roms', $romPath, '--debug-api-port', [string]$Port, '--logs', $evidencePath)
    if ($Checkpoint) {
        $cp = if ([IO.Path]::IsPathRooted($Checkpoint)) { $Checkpoint } else { Join-Path $root $Checkpoint }
        if (-not (Test-Path $cp)) { throw "Checkpoint not found: $cp" }
        $args += @('--load-checkpoint', $cp)
    }
    if ($NoCollisions) { $args += '--no-collisions' }
    if ($InfiniteTime) { $args += '--infinite-time' }
    if ($UnlimitedTurbo) { $args += '--unlimited-turbo' }
    if ($AutoTurbo) { $args += '--auto-turbo' }
    if ($CorneringScale -gt 0) { $args += @('--cornering-scale', [string]::Format([Globalization.CultureInfo]::InvariantCulture, '{0:0.####}', $CorneringScale)) }
    if ($CorneringSpeedRetain -gt 0) { $args += @('--cornering-speed-retain', [string]::Format([Globalization.CultureInfo]::InvariantCulture, '{0:0.####}', $CorneringSpeedRetain)) }
    if ($CourseFollowController) { $args += @('--course-follow-controller', $CourseFollowController) }
    $args += $EmulatorArgs

    if ($Fresh) {
        $freshPath = $evidencePath
        if (Test-Path $freshPath) { Remove-Item $freshPath -Recurse -Force }
        New-Item -ItemType Directory -Path $freshPath -Force | Out-Null
    }

    Write-Host "Launching ChaseHQNative on Research API port $Port..." -ForegroundColor Green
    Write-Host ("  " + $emu + " " + (($args | ForEach-Object { Quote-Arg $_ }) -join ' ')) -ForegroundColor DarkGray
    $argLine = (($args | ForEach-Object { Quote-Arg $_ }) -join ' ')
    $nativeStdout=Join-Path $evidencePath 'native-stdout.log';$nativeStderr=Join-Path $evidencePath 'native-stderr.log'
    Write-StartupRecord 'starting' 'NATIVE_LAUNCH' $null
    $script:NativeProcess = Start-Process -FilePath $emu -ArgumentList $argLine -WorkingDirectory $root -RedirectStandardOutput $nativeStdout -RedirectStandardError $nativeStderr -PassThru
    Write-Host "Native PID: $($script:NativeProcess.Id)" -ForegroundColor DarkGray
    Write-StartupRecord 'starting' 'API_WAIT' $script:NativeProcess
}

Write-Host "Waiting for Research API on 127.0.0.1:$Port..." -ForegroundColor Yellow
$deadline = (Get-Date).AddSeconds($ApiTimeoutSeconds);$apiReady=$false
while ((Get-Date) -lt $deadline) {
    if (Test-Api $ctl $Port) { $apiReady=$true; break }
    if(-not $Attach -and $script:NativeProcess){$script:NativeProcess.Refresh();if($script:NativeProcess.HasExited){$code=$script:NativeProcess.ExitCode;$u=[BitConverter]::ToUInt32([BitConverter]::GetBytes([int]$code),0);$reason="Native process exited before API readiness. Exit code=$code (0x$('{0:X8}' -f $u)).";$out=Tail-IfExists (Join-Path $evidencePath 'native-stdout.log');$err=Tail-IfExists (Join-Path $evidencePath 'native-stderr.log');$zip=New-StartupBundle $reason $script:NativeProcess;Write-StartupRecord 'failed' 'API_WAIT' $script:NativeProcess $reason $zip;Write-Host 'STARTUP FAILED' -ForegroundColor Red;Write-Host $reason -ForegroundColor Red;if($out){Write-Host "--- native stdout ---`n$out"};if($err){Write-Host "--- native stderr ---`n$err"};if($zip){Write-Host "Diagnostics: $zip" -ForegroundColor Yellow};throw $reason}}
    Start-Sleep -Milliseconds 250
}
if (-not $apiReady) { $reason="Research API did not become ready within $ApiTimeoutSeconds seconds on port $Port.";$zip=New-StartupBundle $reason $(if($Attach){$null}else{$script:NativeProcess});Write-StartupRecord 'failed' 'API_WAIT' $(if($Attach){$null}else{$script:NativeProcess}) $reason $zip;Write-Host 'STARTUP FAILED' -ForegroundColor Red;if($zip){Write-Host "Diagnostics: $zip" -ForegroundColor Yellow};throw $reason }
Write-StartupRecord 'starting' 'NATIVE_READY' $(if($Attach){$null}else{$script:NativeProcess})
Write-Host 'Research API ready.' -ForegroundColor Green

if ($Experiment) {
    $exp = if ([IO.Path]::IsPathRooted($Experiment)) { $Experiment } else { Join-Path $root $Experiment }
    if (-not (Test-Path $exp)) { throw "Experiment script not found: $exp" }
    Write-Host "Priming experiment: $exp" -ForegroundColor Green
    & $ctl --port $Port script $exp
    if ($LASTEXITCODE -ne 0) { throw "Experiment priming failed with exit code $LASTEXITCODE." }
}

foreach ($cmd in $PrimeCommand) { Invoke-Ctl $ctl $Port $cmd }

if (-not $NoEvidenceManifest -and -not $Attach) {
    $manifest = [ordered]@{
        schema = 'chq-research-session-v1'; build = $script:ProjectBuildVersion; created = (Get-Date).ToString('o');
        session = $safeSessionName; port = $Port; evidence = $evidencePath; roms = $Roms;
        checkpoint = $Checkpoint; experiment = $Experiment; emulatorArgs = @($EmulatorArgs); primeCommands = @($PrimeCommand)
    }
    $manifest | ConvertTo-Json -Depth 6 | Set-Content (Join-Path $evidencePath 'session.json') -Encoding UTF8
}

Write-Host ''
Write-Host 'Research session ready.' -ForegroundColor Green
Write-Host "API: 127.0.0.1:$Port"
if (-not $Attach) { Write-Host "Evidence: $evidencePath" }
Write-Host "Control client: $ctl"
Write-Host "Example: .\out\build\x64-Debug\chqctl.exe --port $Port status"

if (-not $NoShell) {
    $shellCommand = "& '" + ($ctl -replace "'", "''") + "' --port $Port shell"
    Write-Host 'Opening chqctl interactive shell...' -ForegroundColor Green
    Start-Process -FilePath 'powershell.exe' -ArgumentList @('-NoExit', '-NoLogo', '-Command', $shellCommand) -WorkingDirectory $root | Out-Null
}
