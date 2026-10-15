[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)][string]$ZipPath,
    [switch]$DeveloperHandoff
)
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
$zip=(Resolve-Path $ZipPath).Path
function New-ShortArchiveTestRoot([string]$ResolvedZipPath){
    $suffix='cq-'+[guid]::NewGuid().ToString('N').Substring(0,8)
    $candidateBases=@(
        $env:USERPROFILE,
        [IO.Path]::GetTempPath(),
        (Split-Path $ResolvedZipPath -Parent)
    ) | Where-Object {-not [string]::IsNullOrWhiteSpace([string]$_)} | Select-Object -Unique

    $usable=@()
    foreach($base in $candidateBases){
        try{
            $basePath=[IO.Path]::GetFullPath([string]$base)
            $candidate=Join-Path $basePath $suffix
            New-Item -ItemType Directory -Path $candidate -Force -ErrorAction Stop | Out-Null
            Remove-Item $candidate -Recurse -Force -ErrorAction SilentlyContinue
            $usable += [pscustomobject]@{Path=$candidate;Length=$candidate.Length}
        }catch{}
    }

    if($usable.Count -eq 0){
        throw 'No writable short extraction root is available for archive validation.'
    }

    return [string](($usable | Sort-Object Length | Select-Object -First 1).Path)
}

$temp=New-ShortArchiveTestRoot -ResolvedZipPath $zip
New-Item -ItemType Directory -Path $temp|Out-Null
try{
    Expand-Archive -LiteralPath $zip -DestinationPath $temp -Force
    $roots=@(Get-ChildItem $temp -Directory)
    if($roots.Count -ne 1){throw "Archive must contain exactly one project root; found $($roots.Count)"}
    $root=$roots[0].FullName
    $pathBudgetProbe=Join-Path $root 'evidence\sessions\20260927_235959\runs\999\artifacts\frame-snapshots\brake-lamp-palette-provenance-writer-pc-investigation\bundle-manifest.json'
    if($pathBudgetProbe.Length -ge 240){
        throw ('Archive validation extraction root is too long for the release path-budget test: '+$root+' (representative path '+$pathBudgetProbe.Length+' chars)')
    }
    $required=@('chqdev.bat','chqdev.json','CHANGELOG.md','VALIDATION.md','PACKAGE_MANIFEST.json','PACKAGE_MANIFEST.txt','Build-Debug.bat','Build-Release.bat','Start-ChaseHQ.ps1','Start-ChaseHQWeb.ps1','Start-ChaseHQResearch.ps1','Validate-WindowsPreflight.ps1','Validate-Release.ps1','Validate-Workbench.ps1','scripts\Ensure-ChaseHQLanAccess.ps1','docs\RELEASE_PROCESS.md','docs\RESEARCH_RECOVERY_PLAN.md','research\scripts\SCRIPT_CATALOG.md','research\scripts\regression\regression-v06690-rc32.chqscript','research\scripts\regression\regression-v06690-rc33.chqscript','research\scripts\regression\regression-v06690-rc34.chqscript','research\scripts\regression\regression-v06690-rc341.chqscript','research\scripts\regression\regression-v06690-rc342.chqscript','research\scripts\regression\regression-v06690-rc343.chqscript','research\scripts\regression\full-regression.chqscript','checkpoints\stage1-driving-2352.chqstate','checkpoints\stage1-gameplay-2064.chqstate','checkpoints\stage1-target-post-final-hit-9548.chqstate','checkpoints\stage1-end-level-9988.chqstate')
    $missing=@($required|Where-Object{-not(Test-Path (Join-Path $root $_) -PathType Leaf)})
    $legacyQualifiers=@(Get-ChildItem $root -File -Filter 'Qualify-*.bat' -ErrorAction SilentlyContinue)
    if($legacyQualifiers.Count){throw ('Version-specific qualification wrappers must not be packaged: '+(($legacyQualifiers|ForEach-Object{$_.Name}) -join ', '))}
    if($missing.Count){throw ('Missing required package files: '+($missing -join ', '))}
    $forbidden=@('SDL','out','.git')
    $present=@($forbidden|Where-Object{Test-Path (Join-Path $root $_)})
    if($present.Count){throw ('Forbidden packaged directories: '+($present -join ', '))}
    if(Test-Path (Join-Path $root 'roms\chasehq')){throw 'ROM payload must not be packaged'}
    if(Test-Path (Join-Path $root 'evidence\sessions')){throw 'Transient evidence sessions must not be packaged'}
    $backups=@(Get-ChildItem $root -Recurse -File|Where-Object{$_.Name -like '*.bak' -or $_.Name -like '*.pre-*.bak'})
    if($backups.Count){throw 'Backup/debug files found in package'}
    if($DeveloperHandoff -and -not(Test-Path (Join-Path $root 'build-local.json') -PathType Leaf)){throw 'Developer handoff requires build-local.json'}
    $parseErrors=@()
    foreach($f in Get-ChildItem $root -Recurse -File -Filter *.ps1){$tokens=$null;$errs=$null;[Management.Automation.Language.Parser]::ParseFile($f.FullName,[ref]$tokens,[ref]$errs)|Out-Null;if($errs){$parseErrors+=@($errs|ForEach-Object{"$($f.FullName): $($_.Message)"})}}
    if($parseErrors.Count){throw ('PowerShell parse failure(s): '+($parseErrors -join ' | '))}
    & (Join-Path $root 'Validate-Release.ps1') -SkipBuild
    $releaseValidationOk=$?
    if(-not $releaseValidationOk){throw 'Validate-Release.ps1 -SkipBuild failed inside extracted archive'}
    Write-Host "Release archive validation: PASS" -ForegroundColor Green
    Write-Host "Archive: $zip"
    Write-Host "Root: $($roots[0].Name)"
}
finally{
    Remove-Item $temp -Recurse -Force -ErrorAction SilentlyContinue
}
