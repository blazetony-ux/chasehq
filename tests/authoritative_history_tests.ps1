$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
$root=Split-Path $PSScriptRoot -Parent
$text=Get-Content (Join-Path $root 'Start-ChaseHQWeb.ps1') -Raw
$fn=[regex]::Match($text,'(?s)function Get-AuthoritativeScriptRuns\(\)\{.*?(?=\r?\nfunction\s+)').Value
if(-not $fn){throw 'Authoritative history function absent'}
$temp=Join-Path ([IO.Path]::GetTempPath()) ('chq-history-'+[guid]::NewGuid().ToString('N'))
$script:AuthoritativeRunHistoryCache=$null
$script:AuthoritativeRunHistoryCacheAt=$null
function Prop($o,$name,$default){if($o.PSObject.Properties[$name]){return $o.$name};return $default}
function Get-KnownEvidenceSessionRoots { return $temp }
. ([scriptblock]::Create($fn))
try {
    # Over 250 records, reverse filesystem names and mtime ordering, offsets,
    # equal instants with different UTC offsets, and a session outside page 1.
    for($i=0;$i -lt 320;$i++){
        $session=if($i -lt 3){'active'}else{'other'}
        $id=('{0:D3}' -f (319-$i))
        $dir=Join-Path $temp ($session+'/runs/'+$id)
        New-Item -ItemType Directory -Force $dir|Out-Null
        $at=[DateTimeOffset]::Parse('2026-09-30T12:00:00Z').AddMinutes($i)
        if($i%2){$at=$at.ToOffset([TimeSpan]::FromHours(1))}
        $file=Join-Path $dir 'run-metadata.json'
        @{started=$at.ToString('o');sessionId=$session;runId=$id;status='PASS';build='0.66.9.0-RC2.7';durationMs=1}|ConvertTo-Json|Set-Content $file
        (Get-Item $file).LastWriteTimeUtc=[datetime]::UtcNow.AddMinutes(-$i)
    }
    $all=@(Get-AuthoritativeScriptRuns)
    if($all.Count -ne 320){throw 'History was truncated before filtering'}
    for($i=1;$i -lt $all.Count;$i++){
        if([DateTimeOffset]::Parse([string]$all[$i-1]['at']) -lt [DateTimeOffset]::Parse([string]$all[$i]['at'])){throw 'History not newest first'}
    }
    $page=@($all|Select-Object -Skip 5 -First 7)
    if($page.Count -ne 7 -or [string]$page[0]['runId'] -ne '005'){throw 'Pagination does not select the newest matching records'}
    $active=@($all|Where-Object {$_.session -eq 'active'}|Select-Object -First 2)
    if($active.Count -ne 2 -or [string]$active[0]['runId'] -ne '317'){throw 'Session filtering lost records outside the first global page'}
    if(@($all|Where-Object {$_.version -eq '0.66.9.0-RC2.7'}).Count -ne 320){throw 'Full release version lost'}
    Write-Host 'Authoritative ordering, pagination, session and version tests: PASS'
} finally { Remove-Item $temp -Recurse -Force -ErrorAction SilentlyContinue }
