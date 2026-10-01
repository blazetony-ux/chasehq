[CmdletBinding()]
param([switch]$Check,[string]$Roms,[string]$Sdl)
$ErrorActionPreference='Stop'
Set-StrictMode -Version 2.0
$root=$PSScriptRoot; Set-Location $root
function Log([string]$m){Write-Host $m}
function Resolve-RomGameRoot([string]$Source){
  if([string]::IsNullOrWhiteSpace($Source)){return $null}
  $p=[IO.Path]::GetFullPath($Source)
  if(-not(Test-Path $p -PathType Container)){throw "ROM source not found: $p"}
  if(Test-Path (Join-Path $p 'b52-132.39')){return $p}
  $child=Join-Path $p 'chasehq'
  if(Test-Path (Join-Path $child 'b52-132.39')){return $child}
  throw "ROM source is neither a Chase H.Q. ROM folder nor a ROM root containing .\chasehq (b52-132.39 not found): $p"
}
$requiredRomSizes=[ordered]@{'b52-130.36'=131072;'b52-136.29'=131072;'b52-131.37'=131072;'b52-129.30'=131072;'b52-132.39'=65536;'b52-133.55'=65536;'b52-34.5'=524288;'b52-35.7'=524288;'b52-36.9'=524288;'b52-37.11'=524288;'b52-30.4'=524288;'b52-31.6'=524288;'b52-32.8'=524288;'b52-33.10'=524288;'b52-38.34'=524288;'b52-28.4'=524288;'b52-29.27'=524288;'b52-01.7'=256;'b52-06.24'=256}
function Assert-Roms([string]$Path,[string]$Label){
  $missing=@();$bad=@();foreach($name in $requiredRomSizes.Keys){$f=Join-Path $Path $name;if(-not(Test-Path $f -PathType Leaf)){$missing+=$name;continue};$len=(Get-Item $f).Length;if($len -ne [int64]$requiredRomSizes[$name]){$bad+=("$name expected=$($requiredRomSizes[$name]) got=$len")}}
  if($missing.Count){throw "$Label is incomplete. Missing $($missing.Count) required file(s): $($missing -join ', ')"}
  if($bad.Count){throw "$Label has wrong-size ROM file(s): $($bad -join '; ')"}
  Log ("$Label validation: PASS ({0}/{0}, names + sizes)" -f $requiredRomSizes.Count)
}
Log '  - Unblocking PowerShell scripts...'
Get-ChildItem -LiteralPath $root -Recurse -Filter *.ps1 -File -ErrorAction SilentlyContinue|Unblock-File
$config=Join-Path $root 'build-local.json';$cfg=$null
if(Test-Path $config){$cfg=Get-Content $config -Raw|ConvertFrom-Json}
if(-not $Roms){$Roms=$env:CHASEHQ_ROM_SOURCE};if(-not $Roms -and $cfg){$Roms=[string]$cfg.romSource}
if(-not $Sdl){$Sdl=$env:CHASEHQ_SDL_SOURCE};if(-not $Sdl -and $cfg){$Sdl=[string]$cfg.sdlSource}
Log '  - Local dependency staging...'
$romGame=$null
if($Roms){
  $romGame=Resolve-RomGameRoot $Roms; Log "ROM source: $Roms"; Log "Resolved Chase H.Q. ROM folder: $romGame"; Assert-Roms $romGame 'ROM source'
  if(-not $Check){$dest=Join-Path $root 'roms\chasehq';New-Item -ItemType Directory -Path $dest -Force|Out-Null;$legacyNested=Join-Path $dest 'chasehq';if(Test-Path (Join-Path $legacyNested 'b52-132.39')){Log 'Removing stale nested roms\chasehq\chasehq folder from the v0.66.1 staging bug.';Remove-Item $legacyNested -Recurse -Force};Copy-Item (Join-Path $romGame '*') $dest -Recurse -Force;Assert-Roms $dest 'Staged ROM destination'}
}else{Log 'ROM source: not configured (using existing project ROMs)';$dest=Join-Path $root 'roms\chasehq';if(Test-Path $dest){Assert-Roms $dest 'Existing project ROM destination'}}
if($Sdl){
  $sdlFull=[IO.Path]::GetFullPath($Sdl);Log "SDL source: $sdlFull";if(-not(Test-Path (Join-Path $sdlFull 'CMakeLists.txt'))){throw "SDL source must be an SDL source tree containing CMakeLists.txt: $sdlFull"}
  if(-not $Check){$dest=Join-Path $root 'SDL';New-Item -ItemType Directory -Path $dest -Force|Out-Null;Copy-Item (Join-Path $sdlFull '*') $dest -Recurse -Force}
}else{Log 'SDL source: not configured (using existing .\SDL tree)';if(-not(Test-Path (Join-Path $root 'SDL\CMakeLists.txt'))){throw 'Existing .\SDL tree is missing CMakeLists.txt and no SDL source is configured.'}}
Log '  - Preparation CMake check...'
$find=(Join-Path $root 'Find-CMake.bat');cmd.exe /d /v:on /s /c "call `"$find`" ^&^& `"!CMAKE_EXE!`" --version";if($LASTEXITCODE -ne 0){exit $LASTEXITCODE}
& (Join-Path $root 'Validate-Workbench.ps1')
if($LASTEXITCODE -ne 0){exit $LASTEXITCODE}
if($Check){Log '  - CHECK ONLY - no files copied.';Log '  - Project checks complete.'}else{Log '  - Dependency staging complete.';Log '  - Project ready.'}
