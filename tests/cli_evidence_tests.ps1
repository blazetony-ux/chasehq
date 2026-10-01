param([Parameter(Mandatory=$true)][string]$Exe)
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
$temp=Join-Path ([IO.Path]::GetTempPath()) ('chq-evidence-'+[guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Force $temp|Out-Null
try {
    $logs=Join-Path $temp 'run';$zip=Join-Path $temp 'run.zip';$shots=Join-Path $temp 'external-shots'
    & $Exe (Join-Path $root 'roms/chasehq') --load-checkpoint (Join-Path $root 'checkpoints/stage1-driving-2352.chqstate') --graphics-debug-frame 2352 --graphics-export-raw-maps --screenshot-at 2352 --screenshot-dir $shots --exit-at-frame 2353 --logs $logs --evidence-bundle --zip-name $zip
    if($LASTEXITCODE -ne 0){throw 'CLI evidence run failed'}
    $expanded=Join-Path $temp 'expanded';Expand-Archive $zip $expanded
    $files=@(Get-ChildItem $expanded -Recurse -File)
    if(-not @($files|Where-Object {$_.Name -eq 'frame_00002352.png'}).Count){throw 'Checkpoint-frame screenshot absent from ZIP'}
    if(-not @($files|Where-Object {$_.FullName -match 'graphics_frame_2352'}).Count){throw 'Checkpoint-frame graphics diagnostics absent from ZIP'}
    if(-not @($files|Where-Object {$_.FullName -match 'graphics_frame_2352' -and $_.Extension -eq '.png'}).Count){throw 'Graphics PNG absent from ZIP'}
    $manifest=@($files|Where-Object {$_.Name -eq 'evidence-manifest.json'})
    if($manifest.Count -ne 1){throw 'Evidence identity manifest absent or ambiguous'}
    $m=Get-Content $manifest[0].FullName -Raw|ConvertFrom-Json
    if($m.build -ne '0.66.9.0-RC2.7' -or $m.startFrame -ne 2352){throw 'Evidence identity incorrect'}
    Write-Host 'CLI checkpoint-frame capture and evidence completeness: PASS'
} finally { Remove-Item $temp -Recurse -Force -ErrorAction SilentlyContinue }
