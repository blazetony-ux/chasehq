# RC5.3: PS5.1-safe deterministic native clear-loop substitution equivalence.
# Live Workbench is NEVER restarted; native replacement is experimentally opt-in.
param(
 [string]$BaselineRoot = '',
 [string]$ProjectRoot = (Get-Location).Path,
 [string]$RomRoot = ''
)
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$project=(Resolve-Path -LiteralPath $ProjectRoot).Path
if (-not $BaselineRoot) { $BaselineRoot=Join-Path (Split-Path -Parent $project) 'ChaseHQ-Native-v0.67.0.0-RC5' }
$baseline=(Resolve-Path -LiteralPath $BaselineRoot).Path
if ($baseline.TrimEnd('\') -ieq $project.TrimEnd('\')) { throw 'BaselineRoot must be the immutable, qualified RC5 parent, not this candidate.' }
if (-not $RomRoot) { $RomRoot=Join-Path $baseline 'roms\chasehq' }
if (-not (Test-Path -LiteralPath $RomRoot -PathType Container)) { throw "ROM directory missing: $RomRoot" }
$exe=Join-Path $project 'out\build\x64-Debug\Debug\ChaseHQRuntime.exe'
if (-not (Test-Path -LiteralPath $exe -PathType Leaf)) { throw "Build RC5.3 with .\chqdev.bat build debug first: $exe" }
$manifestPath=Join-Path $project 'shadow-harvest\manifest.json'
$harvestDir=Join-Path $project 'shadow-harvest\checkpoints'
$loaded=ConvertFrom-Json -InputObject (Get-Content -LiteralPath $manifestPath -Raw)
# PS5.1: DO NOT write @(Get-Content -Raw | ConvertFrom-Json): a JSON
# root array is sometimes delivered as ONE object. Enumerate explicitly.
$harvest=@()
foreach ($entry in $loaded) { $harvest+= $entry }
if ($harvest.Count -ne 9) { throw "Checkpoint corpus corrupt: expected 9 manifest records, found $($harvest.Count)" }
foreach($item in $harvest) {
 $p=Join-Path $harvestDir $item.file
 if(-not (Test-Path -LiteralPath $p -PathType Leaf)){throw "Harvested checkpoint missing: $p"}
 $file=Get-Item -LiteralPath $p
 if($file.Length -ne [int64]$item.bytes){throw "Checkpoint size mismatch: $p"}
 $hash=(Get-FileHash -LiteralPath $p -Algorithm SHA256).Hash.ToLowerInvariant()
 if($hash -ne $item.sha256){throw "Checkpoint SHA-256 mismatch: $p"}
}
Write-Host "[RC5.3] Nine original checkpoint SHA-256 hashes verified." -ForegroundColor Green
$cases=@(
 [pscustomobject]@{Name='gameplay-2064'; File='stage1-gameplay-2064.chqstate'; End=2080; Kind='reference'; NeedsClear=$false},
 [pscustomobject]@{Name='class06-5587'; File='rc5-live-shadow-class06-5587.chqstate'; End=5603; Kind='experimental'; NeedsClear=$true},
 [pscustomobject]@{Name='predamage-6333'; File='stage1-target-immediate-pre-damage-6333.chqstate'; End=6349; Kind='reference'; NeedsClear=$false},
 [pscustomobject]@{Name='postfinal-9548'; File='stage1-target-post-final-hit-9548.chqstate'; End=9564; Kind='reference'; NeedsClear=$true},
 [pscustomobject]@{Name='road-class0-5587'; File='rc5-road-sweep-class0-5587.chqstate'; End=5603; Kind='experimental'; NeedsClear=$true},
 [pscustomobject]@{Name='road-class4-5587'; File='rc5-road-sweep-class4-5587.chqstate'; End=5603; Kind='experimental'; NeedsClear=$true},
 [pscustomobject]@{Name='road-class6-5587'; File='rc5-road-sweep-class6-5587.chqstate'; End=5603; Kind='experimental'; NeedsClear=$true},
 [pscustomobject]@{Name='sprite-peak-8647'; File='rc5-sprite-density-8647.chqstate'; End=8663; Kind='assisted'; NeedsClear=$true},
 [pscustomobject]@{Name='palette-transition-7507'; File='rc5-palette-transition-7507.chqstate'; End=7523; Kind='assisted'; NeedsClear=$false},
 [pscustomobject]@{Name='extended-centre-10387'; File='rc5-extended-centre-3.chqstate'; End=10403; Kind='assisted'; NeedsClear=$false},
 [pscustomobject]@{Name='census-centre-10787'; File='rc5-census-centre-1.chqstate'; End=10803; Kind='assisted'; NeedsClear=$false},
 [pscustomobject]@{Name='census-negative-11187'; File='rc5-census-negative-bias-2.chqstate'; End=11203; Kind='assisted'; NeedsClear=$false}
)
$stamp=Get-Date -Format 'yyyyMMdd-HHmmss'
$out=Join-Path $project ('evidence\rc53-native-clear-equivalence-'+$stamp)
New-Item -ItemType Directory -Force -Path $out | Out-Null
$prior=@{}
foreach($key in @('CHQ_NATIVE_SPRITE_TRANSFER_SHADOW','CHQ_NATIVE_ROAD_SHADOW','CHQ_NATIVE_SPRITE_CLEAR_MODE','CHQ_NATIVE_SPRITE_CLEAR_EXPERIMENT_ACK')) {
 $prior[$key]=[Environment]::GetEnvironmentVariable($key,'Process')
}
$results=New-Object System.Collections.Generic.List[object]
$errors=New-Object System.Collections.Generic.List[string]
$classification='INCOMPLETE'
try {
 [Environment]::SetEnvironmentVariable('CHQ_NATIVE_SPRITE_TRANSFER_SHADOW','1','Process')
 [Environment]::SetEnvironmentVariable('CHQ_NATIVE_ROAD_SHADOW','1','Process')
 foreach($case in $cases) {
  $cp=Join-Path $harvestDir $case.File
  if(-not (Test-Path -LiteralPath $cp -PathType Leaf)) {
   $cp=Join-Path (Join-Path $baseline 'checkpoints') $case.File
  }
  if(-not (Test-Path -LiteralPath $cp -PathType Leaf)) {
   $errors.Add("Missing REQUIRED checkpoint: $($case.File)")
   $results.Add([pscustomobject]@{Case=$case.Name;Kind=$case.Kind;State='ERROR_MISSING_CHECKPOINT'})
   continue
  }
  try {
   $runs=@{}
   foreach($mode in @('compare','native')) {
    [Environment]::SetEnvironmentVariable('CHQ_NATIVE_SPRITE_CLEAR_MODE',$mode,'Process')
    [Environment]::SetEnvironmentVariable('CHQ_NATIVE_SPRITE_CLEAR_EXPERIMENT_ACK',$(if($mode -eq 'native') {'YES_UNQUALIFIED'} else {$null}),'Process')
    $dir=Join-Path (Join-Path $out $case.Name) $mode
    New-Item -ItemType Directory -Path $dir -Force | Out-Null
    $exeArgs=@('--roms',$RomRoot,'--load-checkpoint',$cp,'--irq4','--rom-census','--frames',[string]$case.End,'--logs',$dir)
    Write-Host "[RC5.3] $($case.Name) $mode -> frame $($case.End)" -ForegroundColor Cyan
    & $exe @exeArgs > (Join-Path $dir 'console-output.txt') 2>&1
    if($LASTEXITCODE -ne 0){throw "Runtime process failed in ${mode}: $LASTEXITCODE"}
    $manifest=Get-Content -LiteralPath (Join-Path $dir 'run_manifest.json') -Raw | ConvertFrom-Json
    $shadow=Get-Content -LiteralPath (Join-Path $dir 'native-live-candidate-shadow.json') -Raw | ConvertFrom-Json
    $native=Get-Content -LiteralPath (Join-Path $dir 'native-sprite-clear-substitution.json') -Raw | ConvertFrom-Json
    if(-not $manifest.checkpointLoadVerified){throw "Checkpoint load not verified for $mode"}
    if($manifest.release -ne '0.67.0.0-RC5.3'){throw "Wrong release runtime: $($manifest.release)"}
    if($native.mode -ne $mode -or $native.fullSpriteRoutineReplaced){throw "Incorrect native substitution scope or opt-in $mode"}
    if($shadow.spriteTransfer.entries -le 0 -or $shadow.spriteTransfer.entries -ne $shadow.spriteTransfer.pass -or $shadow.spriteTransfer.fail -ne 0 -or $shadow.spriteTransfer.incomplete -ne 0 -or $shadow.spriteTransfer.rejected -ne 0 -or $shadow.spriteTransfer.pending -or $shadow.spriteTransfer.bulkPass -ne $shadow.spriteTransfer.entries -or $shadow.spriteTransfer.bulkFail -ne 0){throw "Sprite shadow verification failed ($mode)"}
    if($shadow.road.entries -ne $shadow.road.pass -or $shadow.road.fail -ne 0 -or $shadow.road.pending){throw "Road shadow verification failed ($mode)"}
    if($native.readbackFailures -ne 0 -or $native.disabledAfterFailure){throw "Native guarded write failure ($mode)"}
    $runs[$mode]=[pscustomobject]@{dir=$dir;manifest=$manifest;shadow=$shadow;native=$native}
   }
   $ref=$runs['compare'];$test=$runs['native']
   if($ref.native.nativeLoopSkips -ne 0){throw 'Compare mode executed native instructions'}
   if($test.native.nativeLoopSkips -ne $test.native.nativeReachedFinalRts){throw 'Incomplete native loop skip'}
   if($case.NeedsClear -and $test.native.nativeLoopSkips -le 0){throw 'Missing required actual native execution coverage'}
   if($ref.shadow.spriteTransfer.entries -ne $test.shadow.spriteTransfer.entries){throw 'Sprite transfer count diverged'}
   if($ref.shadow.road.entries -ne $test.shadow.road.entries){throw 'Road generator count diverged'}
   $rr=Join-Path $ref.dir 'ram';$nr=Join-Path $test.dir 'ram';$compared=0
   foreach($r in @(Get-ChildItem -LiteralPath $rr -File -Filter '*.bin')) {
    $n=Join-Path $nr $r.Name
    if(-not (Test-Path -LiteralPath $n -PathType Leaf)){throw "Native RAM dump missing: $($r.Name)"}
    $hashA=(Get-FileHash -LiteralPath $r.FullName -Algorithm SHA256).Hash
    $hashB=(Get-FileHash -LiteralPath $n -Algorithm SHA256).Hash
    if($hashA -ne $hashB){throw "RAM PARITY MISMATCH: $($r.Name)"}
    $compared++
   }
   if($compared -lt 10){throw "Insufficient RAM region coverage: $compared"}
   $extract= {param([string]$path)
      $line=(Get-Content -LiteralPath $path -TotalCount 2)
      @($line | ForEach-Object {$_ -replace 'instructions=\\d+ ','instructions=SKIPPED-BY-NATIVE '})
   }
   # CPU-A instruction hits deliberately differ when the clear-loop is skipped.
   # Require PC, SR, SP, cycle counters, and CPU-B state to remain identical.
   $refHeader=Get-Content -LiteralPath (Join-Path $ref.dir 'summary.txt') -TotalCount 2
   $natHeader=Get-Content -LiteralPath (Join-Path $test.dir 'summary.txt') -TotalCount 2
   if(@($refHeader).Count -ne 2 -or @($natHeader).Count -ne 2){throw 'Missing final CPU state summary'}
   for($i=0;$i -lt 2;$i++) {
    $a=$refHeader[$i] -replace 'instructions=\d+','instructions=<excluded>'
    $b=$natHeader[$i] -replace 'instructions=\d+','instructions=<excluded>'
    if($a -ne $b){throw "CPU STATE/TIMING MISMATCH: $a versus $b"}
   }
   $results.Add([pscustomobject]@{Case=$case.Name;Kind=$case.Kind;State='PASS';SpriteEntries=$test.shadow.spriteTransfer.entries;RoadEntries=$test.shadow.road.entries;NativeLoopSkips=$test.native.nativeLoopSkips;GuardRejections=$test.native.guardRejected;RamRegions=$compared;CpuStateCyclesParity='PASS'})
   Write-Host "[RC5.3] PASS $($case.Name): $compared identical RAM regions, $($test.native.nativeLoopSkips) actual native clear-loop skips" -ForegroundColor Green
  } catch {
   $msg="$($case.Name): $($_.Exception.Message)"
   $errors.Add($msg)
   $results.Add([pscustomobject]@{Case=$case.Name;Kind=$case.Kind;State='FAIL';Reason=$msg})
   Write-Host "[RC5.3] FAIL $msg" -ForegroundColor Red
  }
 }
 $classification=if($errors.Count -gt 0){'FAIL'}else{'OBSERVED_12_CASE_EQUIVALENCE'}
} catch {
 $errors.Add($_.Exception.Message);$classification='PROBE_ERROR'
} finally {
 foreach($key in $prior.Keys){[Environment]::SetEnvironmentVariable($key,$prior[$key],'Process')}
 $results.ToArray() | Select-Object Case,Kind,State,SpriteEntries,RoadEntries,NativeLoopSkips,GuardRejections,RamRegions,CpuStateCyclesParity,Reason | Export-Csv -LiteralPath (Join-Path $out 'native-equivalence.csv') -NoTypeInformation -Encoding UTF8
 # PS5.1: use -InputObject to preserve a JSON array, even with one row.
 ConvertTo-Json -InputObject ($results.ToArray()) -Depth 8 | Set-Content -LiteralPath (Join-Path $out 'native-equivalence.json') -Encoding UTF8
 ConvertTo-Json -InputObject ([pscustomobject]@{schema='chq-rc53-native-clear-equivalence-v1';release='0.67.0.0-RC5.3';classification=$classification;cases=$cases.Count;completed=$results.Count;errorCount=$errors.Count;scope='native trailing sprite clear loop ONLY';qualified=$false}) -Depth 5 | Set-Content -LiteralPath (Join-Path $out 'probe-metadata.json') -Encoding UTF8
 if($errors.Count){$errors | Set-Content -LiteralPath (Join-Path $out 'probe-errors.txt') -Encoding UTF8}
 $zip="$out.zip";Compress-Archive -Path (Join-Path $out '*') -DestinationPath $zip -CompressionLevel Optimal -Force
 Write-Host "[RC5.3] Classification: $classification" -ForegroundColor Yellow
 Write-Host "[RC5.3] Evidence: $zip"
 Write-Host "[RC5.3] SHA256: $((Get-FileHash -LiteralPath $zip -Algorithm SHA256).Hash.ToLowerInvariant())"
 Write-Host '[RC5.3] No Workbench restart; original RC5 and candidate source not modified. Native mode was limited to isolated headless runs.'
}
if($classification -ne 'OBSERVED_12_CASE_EQUIVALENCE'){throw 'RC5.3 native equivalence FAILED/INCOMPLETE; upload the evidence ZIP. Do not promote.'}
