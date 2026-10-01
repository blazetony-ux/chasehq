param(
  [string]$Exe = ".\out\build\x64-Debug\ChaseHQNative.exe",
  [string]$Roms = ".\roms\chasehq",
  [string]$Checkpoint = ".\ui_quick.chqstate",
  [string]$Plan = ".\tools\sprite_probe_batch.csv",
  [string]$BatchName = "sprite-probe-batch"
)
$ErrorActionPreference = "Stop"
if (!(Test-Path $Exe)) { throw "Executable not found: $Exe" }
if (!(Test-Path $Checkpoint)) { throw "Checkpoint not found: $Checkpoint" }
if (!(Test-Path $Plan)) { throw "Plan not found: $Plan" }
$rows = Import-Csv $Plan
if (!$rows -or $rows.Count -eq 0) { throw "Batch plan contains no rows" }
$produced = @()
foreach ($r in $rows) {
  $from = [int]$r.from
  $to = [int]$r.to
  $every = if ($r.evidence_every) {[int]$r.evidence_every} else {5}
  $shot = if ($r.screenshot_every) {[int]$r.screenshot_every} else {10}
  $exit = $to + 1
  $name = "$BatchName-$($r.name)"
  Write-Host "=== $name : $($r.selector) frames $from-$to ==="
  $args = @($Roms,
    "--load-checkpoint", $Checkpoint,
    "--sprite-evidence-every", "$every",
    "--sprite-evidence-from", "$from",
    "--sprite-evidence-to", "$to",
    "--sprite-solo", $r.selector,
    "--diagnostic-background", "black",
    "--screenshot-every", "$shot",
    "--screenshot-from", "$from",
    "--screenshot-to", "$to",
    "--exit-at-frame", "$exit",
    "--evidence-bundle",
    "--evidence-name", $name)
  & $Exe @args
  if ($LASTEXITCODE -ne 0) { throw "$name failed with exit code $LASTEXITCODE" }
  $zip = Join-Path ".\evidence" ($name + ".zip")
  if (Test-Path $zip) { $produced += $zip }
}
if ($produced.Count -gt 0) {
  $batchZip = Join-Path ".\evidence" ($BatchName + "-attachments.zip")
  if (Test-Path $batchZip) { Remove-Item $batchZip -Force }
  Compress-Archive -Path $produced -DestinationPath $batchZip -CompressionLevel Optimal
  Write-Host "Batch attachment ready: $batchZip"
}
