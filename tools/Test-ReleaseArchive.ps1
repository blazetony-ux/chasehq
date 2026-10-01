[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)][string]$ZipPath,
    [switch]$DeveloperHandoff
)
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
$zip=(Resolve-Path $ZipPath).Path
$temp=Join-Path ([IO.Path]::GetTempPath()) ('chq-release-test-'+[guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $temp|Out-Null
try{
    Expand-Archive -LiteralPath $zip -DestinationPath $temp -Force
    $roots=@(Get-ChildItem $temp -Directory)
    if($roots.Count -ne 1){throw "Archive must contain exactly one project root; found $($roots.Count)"}
    $root=$roots[0].FullName
    $required=@('Build-Debug.bat','Start-ChaseHQ.ps1','Start-ChaseHQWeb.ps1','Validate-Release.ps1','Validate-Workbench.ps1','docs\RELEASE_PROCESS.md','docs\RESEARCH_RECOVERY_PLAN.md','research\scripts\SCRIPT_CATALOG.md','checkpoints\stage1-driving-2352.chqstate','checkpoints\stage1-gameplay-2064.chqstate','checkpoints\stage1-target-post-final-hit-9548.chqstate','checkpoints\stage1-end-level-9988.chqstate')
    $missing=@($required|Where-Object{-not(Test-Path (Join-Path $root $_) -PathType Leaf)})
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
    if($LASTEXITCODE -ne 0){throw 'Validate-Release.ps1 -SkipBuild failed inside extracted archive'}
    Write-Host "Release archive validation: PASS" -ForegroundColor Green
    Write-Host "Archive: $zip"
    Write-Host "Root: $($roots[0].Name)"
}
finally{
    Remove-Item $temp -Recurse -Force -ErrorAction SilentlyContinue
}
