# RC5.1 sprite completion-boundary development proof. Windows PowerShell 5.1 safe.
# Runs separate headless processes: the live Workbench is NOT restarted.
# Actual Workbench run IDs come from Workbench bundles; this is a HEADLESS corpus.
param(
    [string]$ProjectRoot = (Get-Location).Path,
    [string]$BaselineRoot = '',
    [string]$CheckpointRoot = '',
    [string]$RomRoot = '',
    [string]$EvidenceRoot = ''
)
$ErrorActionPreference = 'Stop'
$project = (Resolve-Path -LiteralPath $ProjectRoot).Path
if (-not $BaselineRoot) {
    # Search from the candidate extraction directory upward. Exclude the
    # candidate's own project root even though its internal folder is RC5.
    $cursor = Split-Path -Parent $project
    for ($depth=0; $depth -lt 4; $depth++) {
        $candidate = Join-Path $cursor 'ChaseHQ-Native-v0.67.0.0-RC5'
        if ((Test-Path -LiteralPath $candidate -PathType Container) -and
            (([System.IO.Path]::GetFullPath($candidate).TrimEnd('\')) -ne $project.TrimEnd('\')) -and
            (Test-Path -LiteralPath (Join-Path $candidate 'checkpoints') -PathType Container)) {
            $BaselineRoot = $candidate
            break
        }
        $next = Split-Path -Parent $cursor
        if (-not $next -or $next -eq $cursor) { break }
        $cursor = $next
    }
}
if (-not $BaselineRoot) { $BaselineRoot = $project }
if (-not $CheckpointRoot) { $CheckpointRoot = Join-Path $BaselineRoot 'checkpoints' }
if (-not $RomRoot) { $RomRoot = Join-Path $BaselineRoot 'roms\chasehq' }
if (-not $EvidenceRoot) { $EvidenceRoot = Join-Path $BaselineRoot 'evidence\sessions' }
$exe = Join-Path $project 'out\build\x64-Debug\Debug\ChaseHQRuntime.exe'
if (-not (Test-Path -LiteralPath $exe -PathType Leaf)) { throw "Build the candidate first with .\chqdev.bat build debug; runtime missing: $exe" }
if (-not (Test-Path -LiteralPath $RomRoot -PathType Container)) { throw "ROM source not found: $RomRoot; supply -RomRoot" }
if (-not (Test-Path -LiteralPath $CheckpointRoot -PathType Container)) { throw "Checkpoint directory not found: $CheckpointRoot; supply -CheckpointRoot" }

# RC5 Workbench saves named checkpoints into the issuing run's evidence artifact
# directory, NOT necessarily into the project's top-level checkpoints folder.
# Search the recorded Workbench sessions once for actual saved checkpoint files.
# No auto-extraction, mutation or replacement of the qualified RC5 source.
$evidenceCheckpoints = @{}
if (Test-Path -LiteralPath $EvidenceRoot -PathType Container) {
    Get-ChildItem -LiteralPath $EvidenceRoot -File -Filter '*.chqstate' -Recurse -ErrorAction SilentlyContinue | ForEach-Object {
        if (-not $evidenceCheckpoints.ContainsKey($_.Name)) {
            $evidenceCheckpoints[$_.Name] = $_.FullName
        }
    }
}
function Resolve-ShadowCheckpoint([string]$Name) {
    $normal = Join-Path $CheckpointRoot $Name
    if (Test-Path -LiteralPath $normal -PathType Leaf) { return $normal }
    if ($evidenceCheckpoints.ContainsKey($Name)) { return $evidenceCheckpoints[$Name] }
    return $null
}

# Core scenarios are mandatory. The later assisted checkpoints were captured
# by Workbench and are optional; missing cases are explicitly SKIPPED.
# The class-6 input can also be reproduced from the packaged canonical frame
# 5587 by applying the exact byte intervention before its first CPU instruction.

$cases = @(
    [pscustomobject]@{Name='gameplay-2064'; File='stage1-gameplay-2064.chqstate'; End=2080; Required=$true; RoadRequired=$false; Kind='reference'},
    [pscustomobject]@{Name='class06-5587'; File='rc5-live-shadow-class06-5587.chqstate'; End=5603; Required=$true; RoadRequired=$true; Kind='experimental'},
    [pscustomobject]@{Name='predamage-6333'; File='stage1-target-immediate-pre-damage-6333.chqstate'; End=6349; Required=$true; RoadRequired=$false; Kind='reference'},
    [pscustomobject]@{Name='postfinal-9548'; File='stage1-target-post-final-hit-9548.chqstate'; End=9564; Required=$true; RoadRequired=$false; Kind='reference'},
    [pscustomobject]@{Name='road-class0-5587'; File='rc5-road-sweep-class0-5587.chqstate'; End=5603; Required=$false; RoadRequired=$false; Kind='experimental'},
    [pscustomobject]@{Name='road-class4-5587'; File='rc5-road-sweep-class4-5587.chqstate'; End=5603; Required=$false; RoadRequired=$false; Kind='experimental'},
    [pscustomobject]@{Name='road-class6-5587'; File='rc5-road-sweep-class6-5587.chqstate'; End=5603; Required=$false; RoadRequired=$true; Kind='experimental'},
    [pscustomobject]@{Name='sprite-peak-8647'; File='rc5-sprite-density-8647.chqstate'; End=8663; Required=$false; RoadRequired=$false; Kind='assisted'},
    [pscustomobject]@{Name='palette-transition-7507'; File='rc5-palette-transition-7507.chqstate'; End=7523; Required=$false; RoadRequired=$false; Kind='assisted'},
    [pscustomobject]@{Name='extended-centre-10387'; File='rc5-extended-centre-3.chqstate'; End=10403; Required=$false; RoadRequired=$false; Kind='assisted'},
    [pscustomobject]@{Name='census-centre-10787'; File='rc5-census-centre-1.chqstate'; End=10803; Required=$false; RoadRequired=$false; Kind='assisted'},
    [pscustomobject]@{Name='census-negative-11187'; File='rc5-census-negative-bias-2.chqstate'; End=11203; Required=$false; RoadRequired=$false; Kind='assisted'}
)
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
$out = Join-Path (Join-Path $project 'evidence') ('rc5.1-sprite-boundary-dev-' + $stamp)
New-Item -ItemType Directory -Force -Path $out | Out-Null
$priorRoad = [Environment]::GetEnvironmentVariable('CHQ_NATIVE_ROAD_SHADOW','Process')
$priorSprite = [Environment]::GetEnvironmentVariable('CHQ_NATIVE_SPRITE_TRANSFER_SHADOW','Process')
$rows = @()
$probeErrors = @()
$errorText = ''
$classification = 'INCOMPLETE'
try {
    [Environment]::SetEnvironmentVariable('CHQ_NATIVE_ROAD_SHADOW','1','Process')
    [Environment]::SetEnvironmentVariable('CHQ_NATIVE_SPRITE_TRANSFER_SHADOW','1','Process')
    foreach ($c in $cases) {
        $cp = Resolve-ShadowCheckpoint $c.File
        $origin = 'saved-checkpoint'
        $patchSpec = ''
        # All experimental road-material checkpoints can be rebuilt safely
        # from the same canonical precontact state, with an explicit audited
        # scheduled CPU-B byte write. This eliminates fragile dependencies on
        # where Workbench persisted its per-run artifact checkpoints.
        $materialHex = switch ($c.Name) {
            'class06-5587' { '06' }
            'road-class0-5587' { '00' }
            'road-class4-5587' { '04' }
            'road-class6-5587' { '06' }
            default { '' }
        }
        if (-not $cp -and $materialHex) {
            $cp = Resolve-ShadowCheckpoint 'stage1-target-immediate-pre-contact-5587.chqstate'
            if ($cp) {
                $origin = 'canonical-5587-with-scheduled-material-patch'
                $patchSpec = '5587:B:8:109064:' + $materialHex
                Write-Host "[SHADOW] $($c.Name): reproducing experimental material $materialHex from canonical 5587 using --patch-at-frame" -ForegroundColor Yellow
            }
        }
        if (-not $cp) {
            $state = if ($c.Required) { 'ERROR_MISSING_REQUIRED' } else { 'SKIPPED_MISSING_CHECKPOINT' }
            $msg = "$state $($c.Name): $($c.File); searched checkpoints=$CheckpointRoot and evidence=$EvidenceRoot"
            if ($c.Required) { $probeErrors += $msg }
            $rows += [pscustomobject]@{Case=$c.Name;Kind=$c.Kind;State=$state;Origin='missing';Checkpoint='';Patch='';Start=-1;End=$c.End;RoadEntries=0;RoadPass=0;RoadFail=0;SpriteEntries=0;SpritePass=0;SpriteFail=0;BulkPass=0;BulkFail=0;Incomplete=0;Rejected=0;MismatchByte=-1}
            Write-Host "[SHADOW] $msg" -ForegroundColor Yellow
            continue
        }
        $dir = Join-Path $out $c.Name
        New-Item -ItemType Directory -Force -Path $dir | Out-Null
        Write-Host "[SHADOW] $($c.Name) ($($c.Kind)) -> $($c.End); origin=$origin" -ForegroundColor Cyan
        try {
        $exeArgs = @('--roms',$RomRoot,'--load-checkpoint',$cp,'--irq4', '--frames',[string]$c.End,'--logs',$dir,'--rom-census','--native-observe-entry','A:0x4013A','--native-observe-entry','B:0x3B84')
        if ($patchSpec) { $exeArgs += @('--patch-at-frame',$patchSpec) }
        & $exe @exeArgs > (Join-Path $dir 'console-output.txt') 2>&1
        $exitCode = $LASTEXITCODE
        if ($exitCode -ne 0) { throw "Headless runtime failed for $($c.Name): exit $exitCode; see $dir" }
        $mp = Join-Path $dir 'run_manifest.json'
        $sp = Join-Path $dir 'native-live-candidate-shadow.json'
        if (-not (Test-Path -LiteralPath $mp) -or -not (Test-Path -LiteralPath $sp)) { throw "Missing headless manifest/shadow report: $dir" }
        $manifest = Get-Content -LiteralPath $mp -Raw | ConvertFrom-Json
        $shadow = Get-Content -LiteralPath $sp -Raw | ConvertFrom-Json
        if (-not $manifest.checkpointLoadVerified) { throw "Checkpoint not verified: $($c.Name)" }
        if ($patchSpec) {
            $patchLog = Join-Path $dir 'memory_patches.csv'
            if (-not (Test-Path -LiteralPath $patchLog -PathType Leaf)) { throw "Scheduled material intervention not logged: $($c.Name)" }
            $wantHex = '0x' + ([Convert]::ToInt32($patchSpec.Split(':')[4],16)).ToString('x')
            $foundPatch = @(Import-Csv -LiteralPath $patchLog | Where-Object {
                $_.frame -eq '5587' -and $_.cpu -eq 'B' -and $_.address -eq '0x109064' -and $_.new_value -eq $wantHex
            })
            if ($foundPatch.Count -ne 1) { throw "Scheduled material intervention was not applied once at frame 5587: $($c.Name)" }
        }
        if (-not ($shadow.road.enabled -and $shadow.spriteTransfer.enabled) -or $shadow.replacementEnabled) { throw "Incorrect shadow mode: $($c.Name)" }
        if ($shadow.spriteTransfer.fullComparisonPC -ne 262516 -or $shadow.spriteTransfer.bulkOnlyPC -ne 263542) { throw "Candidate does not expose corrected boundaries: $($c.Name)" }
        $valid = $shadow.spriteTransfer.entries -gt 0 -and $shadow.spriteTransfer.entries -eq $shadow.spriteTransfer.pass -and $shadow.spriteTransfer.fail -eq 0 -and -not $shadow.spriteTransfer.pending -and $shadow.spriteTransfer.incomplete -eq 0 -and $shadow.spriteTransfer.rejected -eq 0 -and $shadow.spriteTransfer.bulkPass -eq $shadow.spriteTransfer.entries -and $shadow.spriteTransfer.bulkFail -eq 0 -and $shadow.road.fail -eq 0 -and -not $shadow.road.pending -and $shadow.road.pass -eq $shadow.road.entries
        if ($c.RoadRequired -and $shadow.road.entries -eq 0) { $valid = $false }
        $state = if ($valid) { 'PASS' } else { 'FAIL' }
        $row = [pscustomobject]@{Case=$c.Name;Kind=$c.Kind;State=$state;Origin=$origin;Checkpoint=$cp;Patch=$patchSpec;Start=$manifest.startFrame;End=$manifest.endFrameExclusive;RoadEntries=$shadow.road.entries;RoadPass=$shadow.road.pass;RoadFail=$shadow.road.fail;SpriteEntries=$shadow.spriteTransfer.entries;SpritePass=$shadow.spriteTransfer.pass;SpriteFail=$shadow.spriteTransfer.fail;BulkPass=$shadow.spriteTransfer.bulkPass;BulkFail=$shadow.spriteTransfer.bulkFail;Incomplete=$shadow.spriteTransfer.incomplete;Rejected=$shadow.spriteTransfer.rejected;MismatchByte=$shadow.spriteTransfer.firstLastMismatchByte}
        $rows += $row
        $row | Format-List | Out-String | Write-Host
        # Continue through ALL scenarios on mismatches to preserve a full corpus.
        } catch {
            $msg = "Case $($c.Name) failed: $($_.Exception.Message)"
            $probeErrors += $msg
            Set-Content -LiteralPath (Join-Path $dir 'probe-error.txt') -Value $msg -Encoding UTF8
            $rows += [pscustomobject]@{Case=$c.Name;Kind=$c.Kind;State='ERROR';Origin=$origin;Checkpoint=$cp;Patch=$patchSpec;Start=-1;End=$c.End;RoadEntries=0;RoadPass=0;RoadFail=0;SpriteEntries=0;SpritePass=0;SpriteFail=0;BulkPass=0;BulkFail=0;Incomplete=0;Rejected=0;MismatchByte=-1}
            Write-Host "[SHADOW] $msg" -ForegroundColor Red
        }
    }
    $failed = @($rows | Where-Object {$_.State -eq 'FAIL'})
    $skipped = @($rows | Where-Object {$_.State -like 'SKIPPED*'})
    $classification = if ($probeErrors.Count -gt 0) { 'PROBE_ERROR' } elseif ($failed.Count -gt 0) { 'MISMATCH_OR_INCOMPLETE' } elseif ($skipped.Count -gt 0) { 'OBSERVED_MATCH_OPTIONAL_COVERAGE_GAPS' } else { 'OBSERVED_ALL_MATCH' }
} catch {
    $errorText = $_.Exception.Message
    $probeErrors += $errorText
    $classification = 'PROBE_ERROR'
    Set-Content -LiteralPath (Join-Path $out 'probe-error.txt') -Value $errorText -Encoding UTF8
    Write-Host "[SHADOW] ERROR: $errorText" -ForegroundColor Red
} finally {
    [Environment]::SetEnvironmentVariable('CHQ_NATIVE_ROAD_SHADOW',$priorRoad,'Process')
    [Environment]::SetEnvironmentVariable('CHQ_NATIVE_SPRITE_TRANSFER_SHADOW',$priorSprite,'Process')
    if ($probeErrors.Count -gt 0) { $errorText = $probeErrors -join '; '; Set-Content -LiteralPath (Join-Path $out 'probe-errors.txt') -Value $probeErrors -Encoding UTF8 }
    $rows | Export-Csv -LiteralPath (Join-Path $out 'shadow-summary.csv') -NoTypeInformation -Encoding UTF8
    $rows | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $out 'shadow-summary.json') -Encoding UTF8
    [pscustomobject]@{Schema='chq-rc5.1-sprite-completion-dev-v1';Candidate='0.67.0.0-RC5.1 shadow development corrective (not promoted)';Classification=$classification;ProjectRoot=$project;BaselineRoot=$BaselineRoot;CheckpointRoot=$CheckpointRoot;RomRoot=$RomRoot;EvidenceRoot=$EvidenceRoot;Created=(Get-Date).ToString('o');TotalCases=$cases.Count;Executed=@($rows | Where-Object {$_.State -in @('PASS','FAIL','ERROR')}).Count;Rows=$rows.Count;Error=$errorText} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $out 'probe-metadata.json') -Encoding UTF8
    $zip = "$out.zip"
    Compress-Archive -Path (Join-Path $out '*') -DestinationPath $zip -CompressionLevel Optimal -Force
    $hash = (Get-FileHash -LiteralPath $zip -Algorithm SHA256).Hash.ToLowerInvariant()
    Write-Host "[SHADOW] Classification: $classification" -ForegroundColor Yellow
    Write-Host "[SHADOW] Evidence: $zip" -ForegroundColor Green
    Write-Host "[SHADOW] SHA256: $hash"
    Write-Host '[SHADOW] Existing Workbench not restarted. No native replacement.'
}
if ($classification -eq 'PROBE_ERROR' -or $classification -eq 'MISMATCH_OR_INCOMPLETE') { throw "Shadow proof did not pass; upload the evidence ZIP. $errorText" }
