param(
  [string]$Exe = ".\out\build\x64-Debug\ChaseHQNative.exe",
  [string]$Roms = ".\roms\chasehq",
  [string]$Checkpoint = ".\checkpoints\stage1-driving-2352.chqstate",
  [int]$CaptureFrame = 2400,
  [int[]]$Values = @(-32,-16,0,16,32),
  [string]$EvidenceRoot = ".\evidence\sprite_xy_sweep"
)
$ErrorActionPreference = "Stop"
$zip = "$EvidenceRoot.zip"
if(Test-Path $EvidenceRoot){Remove-Item $EvidenceRoot -Recurse -Force}
if(Test-Path $zip){Remove-Item $zip -Force}
New-Item -ItemType Directory -Force -Path $EvidenceRoot | Out-Null
$index = @()
foreach($y in $Values){ foreach($x in $Values){
  $tag = "sprite_x${x}_y${y}"
  $run = Join-Path $EvidenceRoot $tag
  Write-Host "`n===== $tag =====" -ForegroundColor Cyan
  & $Exe --roms $Roms --load-checkpoint $Checkpoint --exit-at-frame ($CaptureFrame+1) --layer-offset "bg0:0:-16" --layer-offset "bg1:0:-20" --layer-offset "sprites:${x}:${y}" --screenshot-at $CaptureFrame --logs $run
  if($LASTEXITCODE -ne 0){ throw "Run $tag failed with exit code $LASTEXITCODE" }
  $index += [pscustomobject]@{run=$tag;sprite_x=$x;sprite_y=$y;capture_frame=$CaptureFrame;bg0_y=-16;bg1_y=-20}
}}
$index | ConvertTo-Json | Set-Content (Join-Path $EvidenceRoot "experiment_index.json") -Encoding UTF8
Compress-Archive -Path "$EvidenceRoot\*" -DestinationPath $zip -Force
Write-Host "`nDONE - upload: $zip" -ForegroundColor Green
