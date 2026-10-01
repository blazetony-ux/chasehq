param([int]$HttpPort = 37680)
$ErrorActionPreference = 'Stop'
$base = "http://127.0.0.1:$HttpPort"
$name = 'snapshot-refresh-' + (Get-Date -Format 'yyyyMMdd-HHmmss-fff')
$body = @{ name = $name } | ConvertTo-Json -Compress
$created = Invoke-RestMethod -Method Post -Uri "$base/api/v1/frame/snapshot" -ContentType 'application/json' -Body $body
if (-not $created.ok) { throw 'Snapshot capture did not report ok=true' }
$list = Invoke-RestMethod -Method Get -Uri "$base/api/v1/frame/snapshots"
$hit = @($list.snapshots | Where-Object { $_.name -eq $created.name })
if ($hit.Count -ne 1) { throw "Immediate snapshot discovery failed: $($created.name) was not returned by /api/v1/frame/snapshots" }
Write-Host "PASS: newly captured snapshot is immediately discoverable: $($created.name)"
Write-Host "PATH: $($created.path)"
