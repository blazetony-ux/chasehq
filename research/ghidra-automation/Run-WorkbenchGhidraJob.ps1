param(
    [Parameter(Mandatory=$true)][string]$ChaseRoot,
    [Parameter(Mandatory=$true)][string]$JobId
)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
if ($JobId -cnotmatch '^ghidra-[0-9]{8}-[0-9]{6}-[a-f0-9]{8}$') { throw 'Invalid Ghidra job ID' }
$jobsRoot = Join-Path $ChaseRoot 'evidence\ghidra-jobs'
$jobDir = Join-Path $jobsRoot $JobId
if (-not (Test-Path -LiteralPath $jobDir -PathType Container)) { throw 'Missing Ghidra job directory' }
$out = Join-Path $jobDir 'result.json'
$success = $false
$summary = [ordered]@{schema='chq-ghidra-job-v1';jobId=$JobId;build='0.67.0.0-RC6.2.4';state='RUNNING';startedUtc=(Get-Date).ToUniversalTime().ToString('o');finishedUtc='';headlessProject='';evidenceZip='';cases=@();hypothesisWindows=0;error='';emulatorUnchanged=$true}
try {
    $launcher = Join-Path $PSScriptRoot 'Run-ChaseHQ-Ghidra.ps1'
    if (-not (Test-Path -LiteralPath $launcher -PathType Leaf)) { throw 'Bundled Ghidra launcher is missing' }
    # Invoke only the packaged, fixed-script headless workflow; no caller-controlled code or arguments.
    & $launcher -ChaseRoot $ChaseRoot
    $code = $LASTEXITCODE
    if ($null -ne $code -and $code -ne 0) { throw "Ghidra launcher exit code $code" }
    $runs = @(Get-ChildItem -LiteralPath (Join-Path $ChaseRoot 'evidence') -Directory -Filter 'ghidra-rea-run032-*' | Where-Object { Test-Path (Join-Path $_.FullName 'run-manifest.json') } | Sort-Object LastWriteTimeUtc -Descending)
    if ($runs.Count -lt 1 -or $runs[0].LastWriteTimeUtc -lt ([DateTimeOffset]::Parse($summary.startedUtc).UtcDateTime).AddSeconds(-2)) { throw 'Ghidra did not create a new verified run manifest for this job' }
    $manifestFile = Join-Path $runs[0].FullName 'run-manifest.json'
    $manifest = Get-Content -LiteralPath $manifestFile -Raw | ConvertFrom-Json
    if ($manifest.schema -ne 'chq-ghidra-automation-run-v1' -or [int]$manifest.ghidraExitCode -ne 0) { throw 'Ghidra manifest failed validation' }
    if ($manifest.releaseBaseline -cne '0.67.0.0-RC6.2.4') { throw 'Ghidra manifest release identity mismatch' }
    if ([int]$manifest.exportedHypothesisWindows -lt 1) { throw 'No candidate static windows exported' }
    $summary.state = 'PASS'
    $summary.headlessProject = [string]$manifest.headlessProject
    $summary.evidenceZip = $runs[0].FullName + '.zip'
    $summary.cases = @($manifest.stagedCases)
    $summary.hypothesisWindows = [int]$manifest.exportedHypothesisWindows
    $success = $true
} catch {
    $summary.state = 'FAIL'
    $summary.error = [string]$_.Exception.Message
} finally {
    $summary.finishedUtc = (Get-Date).ToUniversalTime().ToString('o')
    $summary | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $out -Encoding UTF8
}
if (-not $success) { Write-Error "GHIDRA JOB FAILED: $($summary.error)"; exit 1 }
Write-Host "GHIDRA JOB PASS: $JobId; candidate windows=$($summary.hypothesisWindows)"
