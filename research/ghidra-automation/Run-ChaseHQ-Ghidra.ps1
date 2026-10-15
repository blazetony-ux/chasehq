param(
    [string]$GhidraHome = '',
    [string]$RomPath = "$env:USERPROFILE\Downloads\cpu_main.bin",
    [string]$ChaseRoot = '',
    [string]$ProjectParent = 'C:\Projects\GhidraResearch',
    [string[]]$CandidatePcs = @('0x00820C')
)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

if ([string]::IsNullOrWhiteSpace($ChaseRoot)) {
    $ChaseRoot = (Get-Location).Path
}
if (-not (Test-Path -LiteralPath (Join-Path $ChaseRoot 'chqdev.bat') -PathType Leaf)) {
    throw "Run from the extracted ChaseHQ project directory, or pass -ChaseRoot: $ChaseRoot"
}
if (-not (Test-Path -LiteralPath $RomPath -PathType Leaf)) {
    throw "CPU-A ROM export not found: $RomPath"
}
if ((Get-Item -LiteralPath $RomPath).Length -ne 524288) {
    throw "CPU-A ROM must be exactly 524288 bytes"
}
$expected = '5CF1C9E786A39795EBD8E99C8E8BE0D2398CC4F74221067CF2844945308D6C5A'
$sha = (Get-FileHash -LiteralPath $RomPath -Algorithm SHA256).Hash.ToUpperInvariant()
if ($sha -cne $expected) {
    throw "Wrong CPU-A ROM. Expected $expected; received $sha"
}
$stream = [IO.File]::OpenRead($RomPath)
try {
    $header = New-Object byte[] 8
    if ($stream.Read($header,0,8) -ne 8) { throw 'Failed to read reset vectors' }
} finally {
    $stream.Dispose()
}
if ([BitConverter]::ToString($header) -cne '00-10-3F-00-00-00-01-40') {
    throw "Unexpected reset-vector bytes: $([BitConverter]::ToString($header))"
}

$headless = $null
if (-not [string]::IsNullOrWhiteSpace($GhidraHome)) {
    $headless = Join-Path $GhidraHome 'support\analyzeHeadless.bat'
    if (-not (Test-Path -LiteralPath $headless -PathType Leaf)) {
        throw "Ghidra launcher not found: $headless"
    }
} else {
    foreach ($root in @($env:GHIDRA_HOME,'C:\Tools\Ghidra','C:\Tools',"$env:USERPROFILE\Downloads",'C:\Program Files\Ghidra')) {
        if ([string]::IsNullOrWhiteSpace($root)) { continue }
        if (-not (Test-Path -LiteralPath $root -PathType Container)) { continue }
        $hit = Get-ChildItem -LiteralPath $root -Recurse -File -Filter 'analyzeHeadless.bat' -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($hit) { $headless = $hit.FullName; break }
    }
    if (-not $headless) {
        throw 'Cannot find analyzeHeadless.bat. Supply -GhidraHome "C:\path\to\ghidra_12.1.4_PUBLIC"'
    }
}

$candidateTokens = @()
foreach ($token in $CandidatePcs) {
    if ($token -cnotmatch '^0x[0-9a-fA-F]{1,6}$') { throw "Invalid CPU-A candidate PC: $token" }
    $address = [Convert]::ToInt32($token.Substring(2), 16)
    if ($address -lt 8 -or $address -ge 0x80000 -or ($address % 2) -ne 0) { throw "CPU-A candidate out of ROM bounds or unaligned: $token" }
    $canonical = '0x{0:X6}' -f $address
    if ($candidateTokens -cnotcontains $canonical) { $candidateTokens += $canonical }
}
if ($candidateTokens.Count -gt 64) { throw 'Maximum 64 requested Ghidra sites per run' }
$release = (Get-Content -LiteralPath (Join-Path $ChaseRoot 'chqdev.json') -Raw | ConvertFrom-Json).release
if ($release -cnotmatch '^0\.67\.0\.0-RC6\.2\.1$') { throw "Unexpected RC6.2.1 Ghidra release identity: $release" }
$stamp = (Get-Date -Format 'yyyyMMdd-HHmmss-fff')
$runId = 'ghidra-rea-run032-' + $stamp
$evidenceDir = Join-Path $ChaseRoot ('evidence\' + $runId)
$exportDir = Join-Path $evidenceDir 'analysis'
New-Item -ItemType Directory -Path $exportDir -Force | Out-Null
New-Item -ItemType Directory -Path $ProjectParent -Force | Out-Null
$projectName = 'ChaseHQ_REA_Auto_' + $stamp.Replace('-','_')
$headlessLog = Join-Path $evidenceDir 'headless-console.txt'
$scriptLog = Join-Path $evidenceDir 'ghidra-script.log'
$analysisLog = Join-Path $evidenceDir 'ghidra-analysis.log'

Write-Host "CPU-A SHA256 verified: $sha"
Write-Host "Ghidra headless: $headless"
Write-Host "Separate project: $projectName"
Write-Host 'Starting headless Motorola 68000 analysis (no GUI interaction required)...'

$originalPreference = $ErrorActionPreference
$ErrorActionPreference = 'Continue'
try {
    & $headless $ProjectParent $projectName -import $RomPath -processor '68000:BE:32:default' -cspec default -loader BinaryLoader -loader-baseAddr '0x000000' -preScript 'ChaseHQSeed68000.py' @candidateTokens -postScript 'ChaseHQExport68000.py' $exportDir $sha @candidateTokens -scriptPath $PSScriptRoot -analysisTimeoutPerFile 900 -max-cpu 2 -log $analysisLog -scriptlog $scriptLog *> $headlessLog
    $processExitCode = $LASTEXITCODE
} finally {
    $ErrorActionPreference = $originalPreference
}

$staticFile = Join-Path $exportDir 'chq-ghidra-static.json'
$findingsFile = Join-Path $exportDir 'chq-ghidra-findings.json'
if ($processExitCode -ne 0 -or -not (Test-Path -LiteralPath $staticFile) -or -not (Test-Path -LiteralPath $findingsFile)) {
    throw "Ghidra automation failed (exit $processExitCode). Inspect: $headlessLog"
}
if (-not (Select-String -LiteralPath $headlessLog -Pattern 'CHQ_GHIDRA_EXPORT_OK' -Quiet)) {
    throw "Ghidra export confirmation missing. Inspect $headlessLog"
}
$record = Get-Content -LiteralPath $staticFile -Raw | ConvertFrom-Json
if ($record.schema -ne 'chq-rea-static-export-v1' -or $record.cpu -ne 'A' -or $record.imageBase -ne '0x000000' -or $record.romSha256.ToUpperInvariant() -cne $sha) {
    throw 'Export provenance does not match the verified ROM and CPU-A address map'
}
$windows = @($record.functions)
$findings = Get-Content -LiteralPath $findingsFile -Raw | ConvertFrom-Json
foreach ($pc in $candidateTokens) {
    $present = @($findings.sites | Where-Object { $_.pc -ceq $pc -and $null -ne $_.instruction })
    if ($present.Count -lt 1) { throw "Ghidra did not disassemble requested CPU-A candidate $pc" }
}

if ($windows.Count -lt 1 -or $windows.Count -gt 4096) {
    throw 'No acceptable static hypothesis windows in the export'
}
$imports = Join-Path $ChaseRoot 'research\rea\imports'
New-Item -ItemType Directory -Path $imports -Force | Out-Null
$cases = @('contact-5587','damage-6333','post-final-9548')
$staged = @()
foreach ($case in $cases) {
    $old = 'rc45-' + $case
    $name = 'ghidra-' + $case
    $sourceTsv = Join-Path $imports ($old + '.census.tsv')
    if (-not (Test-Path -LiteralPath $sourceTsv)) {
        Write-Warning "Missing Run031 real census: $sourceTsv (skipping $case crosswalk staging)"
        continue
    }
    $destJson = Join-Path $imports ($name + '.static.json')
    $destTsv = Join-Path $imports ($name + '.census.tsv')
    Copy-Item -LiteralPath $staticFile -Destination $destJson -Force
    Copy-Item -LiteralPath $sourceTsv -Destination $destTsv -Force
    $staged += $name
}
$manifest = [ordered]@{
    schema = 'chq-ghidra-automation-run-v1'
    releaseBaseline = $release
    requestedCandidatePcs = @($candidateTokens)
    cpu = 'A'
    romSha256 = $sha.ToLowerInvariant()
    ghidraLauncher = $headless
    headlessProject = (Join-Path $ProjectParent $projectName)
    language = '68000:BE:32:default'
    imageBase = '0x000000'
    ghidraExitCode = $processExitCode
    exportedHypothesisWindows = $windows.Count
    stagedCases = @($staged)
    confidence = 'hypothesis: no function boundaries independently reviewed'
    nativeRoutineReplacement = $false
    immutableReleaseArchiveChanged = $false
}
$manifest | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $evidenceDir 'run-manifest.json') -Encoding UTF8
$upload = Join-Path $ChaseRoot ('evidence\' + $runId + '.zip')
Compress-Archive -LiteralPath $evidenceDir -DestinationPath $upload -CompressionLevel Optimal -Force
Write-Host "GHIDRA STATIC ANALYSIS COMPLETE - $($windows.Count) hypothesis windows"
if ($staged.Count -eq 0) {
    Write-Warning 'No Run031 census files were found; upload the report and we will stage the crosswalk separately.'
} else {
    Write-Host "Workbench REA cases: $($staged -join ', ')"
}
Write-Host "UPLOAD EVIDENCE ZIP: $upload"
