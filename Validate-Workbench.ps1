[CmdletBinding()]
param()
$ErrorActionPreference='Stop'
$root=$PSScriptRoot
$web=Join-Path $root 'Start-ChaseHQWeb.ps1'
if(-not(Test-Path $web)){throw "Missing $web"}
$text=Get-Content $web -Raw
$required=@('function setScriptRunUi','function setScriptState','function setScriptPhase','function touchScriptProgress','function reportFrontendFault','function createDiagnosticBundle','function forceResetScriptRunner','function validateScriptRunnerDom','function appendScriptOutput','function Web-RequestLog','async function runScriptText','/api/v1/frontend-ready','frontendBootstrap')
$missing=@($required|Where-Object{$text.IndexOf($_,[StringComparison]::Ordinal) -lt 0})
if($missing.Count){throw "Workbench resilience self-test failed; missing: $($missing -join ', ')"}
$manifest=Get-Content (Join-Path $root 'research\feature-manifest.json') -Raw|ConvertFrom-Json
$versionText=Get-Content (Join-Path $root 'src\version.h') -Raw
$m=[regex]::Match($versionText,'kNativeVersion\s*=\s*"([^"]+)"');if(-not$m.Success){throw 'Could not read native version'}
$nativeVersion=$m.Groups[1].Value
if([string]$manifest.build -ne $nativeVersion){throw "Version mismatch: feature manifest=$($manifest.build), native=$nativeVersion"}

if($text -notmatch "@runtime/web-listener\.log"){throw 'Workbench validation failed: runtime web-listener alias missing'}
if($text -match "logs\.tail file=@runtime/web-stdout\.log"){throw 'Workbench validation failed: stale web-stdout logs.tail example remains'}
$full=Get-Content (Join-Path $root 'research\scripts\regression\full-regression.chqscript') -Raw
$lastComplete=$full.LastIndexOf('echo === ChaseHQ Full Regression: COMPLETE ===',[StringComparison]::Ordinal)
if($lastComplete -lt 0 -or $full.Substring($lastComplete).Trim() -ne 'echo === ChaseHQ Full Regression: COMPLETE ==='){throw 'Full regression COMPLETE marker is not the final command'}
if($text -notmatch 'chq-script-run-history-v1'){throw 'Workbench validation failed: legacy history migration hook missing'}

$scripts=@(Get-ChildItem (Join-Path $root 'research\scripts') -Recurse -Filter *.chqscript -File)
if($scripts.Count -lt 1){throw 'No research scripts found'}

$scriptRoot=Join-Path $root 'research\scripts'
$includeFailures=@()
foreach($sf in $scripts){
    $raw=Get-Content $sf.FullName -Raw
    foreach($m in [regex]::Matches($raw,'(?m)^\s*include\s+"([^"]+)"\s*$')){
        $target=Join-Path $scriptRoot ($m.Groups[1].Value -replace '/','\\')
        if(-not(Test-Path $target -PathType Leaf)){$includeFailures += ($sf.FullName+': '+$m.Groups[1].Value)}
    }
}
if($includeFailures.Count){throw ('Unresolved script include(s): '+($includeFailures -join '; '))}
$metadataFailures=@()
foreach($sf in $scripts){
    $raw=Get-Content $sf.FullName -Raw
    foreach($key in @('name','category','purpose')){if($raw -notmatch ('(?mi)^#\s*'+$key+'\s*:\s*\S')){$metadataFailures += ($sf.FullName+': missing # '+$key+':')}}
    if($sf.Directory.Name -eq 'regression'){
        foreach($key in @('regression-version','regression-scope')){if($raw -notmatch ('(?mi)^#\s*'+$key+'\s*:\s*\S')){$metadataFailures += ($sf.FullName+': missing # '+$key+':')}}
    }
}
if($metadataFailures.Count){throw ('Script metadata validation failed: '+($metadataFailures -join '; '))}
$required067=@('Docs / Knowledge','Handbook','API Reference','Scripting Reference','Project Knowledge','Glossary','SDL Window Controls','Individual sprites','docs.index','docs.search','docs.handbook','docs.page','docs.handover','window.status','window.fullscreen','window.scale','window.minimize','sprites/sprites.json')
$missing067=@($required067|Where-Object{$text.IndexOf($_,[StringComparison]::Ordinal) -lt 0})
if($missing067.Count){throw ('v0.66.7.7 Workbench handbook surface missing: '+($missing067 -join ', '))}
$knowledge=Get-Content (Join-Path $root 'research\knowledge\documentation.json') -Raw|ConvertFrom-Json
if([string]$knowledge.build -ne $nativeVersion){throw "Knowledge build mismatch: $($knowledge.build) != $nativeVersion"}

$handbook=Get-Content (Join-Path $root 'research\knowledge\handbook.json') -Raw|ConvertFrom-Json
if([string]$handbook.build -ne $nativeVersion){throw "Handbook build mismatch: $($handbook.build) != $nativeVersion"}
if(@($handbook.sections).Count -lt 5){throw 'Handbook content is unexpectedly sparse'}
if($text -notmatch '/api/v1/docs/handbook' -or $text -notmatch '/api/v1/schema/actions' -or $text -notmatch 'showApiAction' -or $text -notmatch 'showScriptReference'){throw 'Workbench handbook/live-reference wiring is incomplete'}
$handover=Get-Content (Join-Path $root 'docs\CHATGPT_HANDOVER_PROMPT.md') -Raw
if($handover -notmatch [regex]::Escape('v'+$nativeVersion)){throw 'ChatGPT handover version is stale'}

# v0.66.7.7 handbook/search/export consolidation checks.
if(([regex]::Matches($text,'(?m)^(?:async )?function showHandbookPage\(id\)\{')).Count -ne 1){throw 'Workbench validation failed: showHandbookPage must have exactly one implementation'}
if($text -notmatch 'searchText' -or $text -notmatch 'New-KnowledgeArticleHtml' -or $text -notmatch 'docsKnowledgeCard'){throw 'Workbench validation failed: consolidated knowledge search/export/landing wiring is incomplete'}
if(@($text.ToCharArray()|Where-Object{[int]$_ -gt 127}).Count -ne 0){throw 'Workbench validation failed: Start-ChaseHQWeb.ps1 must remain ASCII-safe for Windows PowerShell 5.1'}
if($text -notmatch 'Get-AuthoritativeScriptRuns' -or $text -notmatch '/api/v1/script/runs' -or $text -notmatch 'refreshAuthoritativeRunHistory'){throw 'Workbench validation failed: authoritative run-history discovery wiring missing'}
if($text -notmatch 'Finalize-ChqScriptRun' -or $text -notmatch 'finalizeRun:true' -or $text -notmatch 'transient:true'){throw 'Workbench validation failed: single-finalization/transient script-run wiring missing'}
$runFn=[regex]::Match($text,'(?s)function Invoke-ChqScriptRun\(.*?\n\}\nfunction Finalize-ChqScriptRun').Value;if([string]::IsNullOrWhiteSpace($runFn) -or $runFn -match 'Compress-Archive'){throw 'Workbench validation failed: per-command script run still performs archive compression'}
$clientRun=[regex]::Match($text,"(?s)async function runScriptText\(.*?\n\}\nfunction legacyCopyNow").Value;if([string]::IsNullOrWhiteSpace($clientRun)){throw 'Workbench validation failed: browser runScriptText not found'}
if($clientRun -notmatch "(?s)if\(ticker\)\{clearInterval\(ticker\);ticker=null\}.*setScriptPhase\('FINALIZING'" ){throw 'Workbench validation failed: script ticker is not stopped before FINALIZING'}
if($clientRun -notmatch "(?s)setScriptPhase\('FINALIZING'.*finalizeServerRun\('PASS'\).*recordRun\('PASS'.*setScriptPhase\('DONE'" ){throw 'Workbench validation failed: DONE/COMPLETE is not published after finalization/history recording'}
if(-not(Test-Path (Join-Path $root 'research\scripts\regression\regression-v06678-run-finalization.chqscript'))){throw 'Workbench validation failed: run-finalization regression missing'}
if($text -notmatch 'Get-StructuredFrameSnapshotRoots' -or $text -notmatch 'Resolve-StructuredFrameSnapshotDir' -or $text -notmatch 'artifacts\\frame-snapshots'){throw 'Workbench validation failed: completed-run snapshot discovery wiring missing'}

if($text -notmatch 'async function applyScriptLibraryFilter' -or $text -notmatch "if\(!loadedScriptPath\)throw Error\('Script library discovered entries but did not auto-load the selected script'\)"){throw 'Workbench validation failed: Script Library auto-load/bootstrap guard missing'}
if($text -notmatch 'Wait-ChqPaused\(\[int\]\$minimumFrame,\[int\]\$timeoutMs=900000,\[int\]\$stallTimeoutMs=15000\)' -or $text -notmatch 'no_progress_ms'){throw 'Workbench validation failed: progress-aware long frame wait missing'}
if($text -notmatch 'longOp\?3700000:10000' -or $text -notmatch '\?3700:8;'){throw 'Workbench validation failed: long-operation browser/proxy budget missing'}
if($text -match 'web-evidence'){throw 'Workbench validation failed: obsolete web-evidence path remains'}
if($text -match "script-runs"){throw 'Workbench validation failed: obsolete script-runs path remains'}
$requiredRunRoot="Join-Path "+'$sessionPath'+" 'runs'";if($text.IndexOf($requiredRunRoot,[StringComparison]::Ordinal) -lt 0){throw 'Workbench validation failed: per-session runs root missing'}

# v0.66.8.0 RC3: offline timeline analysis must remain streaming and non-destructive.
$trimFn=[regex]::Match($text,'(?s)function Trim-ForensicTimelineEvent\(.*?(?=\r?\nfunction\s+)').Value
$scanFn=[regex]::Match($text,'(?s)function Analyze-ForensicTimelineWrites\(.*?(?=\r?\nfunction\s+)').Value
if([string]::IsNullOrWhiteSpace($trimFn) -or $trimFn -notmatch '\[IO\.File\]::OpenText\(\$writes\)' -or $trimFn -match 'Remove-Item \$fp' -or $trimFn -match 'Export-Csv \$writes' -or $trimFn -match '\$rows\s*\+='){throw 'Workbench validation failed: timeline event-window implementation is not streaming/non-destructive'}
if([string]::IsNullOrWhiteSpace($scanFn) -or $scanFn -notmatch '\[IO\.File\]::OpenText\(\$csv\)' -or $scanFn -match '@\(Import-Csv \$csv\)'){throw 'Workbench validation failed: timeline memory analysis is not streaming'}

Write-Host 'Script includes: PASS' -ForegroundColor Green
Write-Host 'Script metadata: PASS' -ForegroundColor Green
Write-Host 'Session/run layout: PASS' -ForegroundColor Green
Write-Host "Workbench static resilience validation: PASS" -ForegroundColor Green
Write-Host "Version consistency: PASS ($nativeVersion)" -ForegroundColor Green
Write-Host "Script library: PASS ($($scripts.Count) scripts)" -ForegroundColor Green

# v0.66.8.0 RC7: derived analysis belongs to the active run and automated correlation/playback are wired.
if($text -notmatch 'function Get-TimelineAnalysisDirectory' -or $text -notmatch 'function Invoke-TimelineAutomatedAnalysis' -or $text -notmatch "'timeline\.analyze\.auto'" -or $text -notmatch "'timeline\.play'"){throw 'Workbench validation failed: RC7 portable timeline analysis/playback wiring missing'}
if($scanFn -notmatch 'Get-TimelineAnalysisDirectory \$full -Create'){throw 'Workbench validation failed: timeline scan does not target current-run analysis artifacts'}

# v0.66.8.0 RC7.3: browser long-run memory resilience and history filtering.
if($text -notmatch 'id="scriptThumbs" type="checkbox"> Show thumbnails'){throw 'Workbench validation failed: thumbnails must default OFF'}
if($text -notmatch 'MAX_LIVE_OUTPUT_BLOCKS=120' -or $text -notmatch 'MAX_ARTIFACT_THUMBNAILS=24' -or $text -notmatch 'function trimScriptOutputDom'){throw 'Workbench validation failed: bounded browser output/artifact rendering missing'}
if($text -notmatch 'id="scriptHistoryStatus"' -or $text -notmatch 'deriveReleaseVersion' -or $text -notmatch 'releaseVersion'){throw 'Workbench validation failed: RC-aware version/status run-history filters missing'}


# v0.66.8.0 RC7.4: bounded run-history payloads / browser response cap.
if($text -notmatch "chq-script-run-list-v2" -or $text -notmatch "api script.runs limit=25 status=PASS" -or $text -notmatch "Browser display truncated:"){throw 'Workbench validation failed: RC7.4 bounded run-history/response safeguards missing'}
$runHistoryFn=[regex]::Match($text,'(?s)function Get-AuthoritativeScriptRuns\(\)\{.*?(?=\r?\nfunction\s+)').Value
if([string]::IsNullOrWhiteSpace($runHistoryFn) -or $runHistoryFn -match 'ReadAllText\(\$console' -or $runHistoryFn -match 'sourceText='){throw 'Workbench validation failed: authoritative run-history list still embeds heavy console/source payloads'}

# v0.66.9.0-RC1: Game Lab + TC0100SCN text-character inspector.
if($text -notmatch 'Chase H\.Q\. Experimental / Game Lab' -or $text -notmatch 'gameTurboStock' -or $text -notmatch 'gameSpeedFreeze'){throw 'Workbench validation failed: v0.66.9.0 Game Lab UI wiring missing'}
foreach($a in @('game.experiment.status','game.turbo.stock','game.turbo.active','game.turbo.fire','game.speed.set','game.speed.freeze','game.speed.clear','tile.inspect')){if($text.IndexOf("'$a'",[StringComparison]::Ordinal) -lt 0){throw "Workbench validation failed: missing action $a"}}
if($text -notmatch 'TC0100SCN Character Inspector' -or $text -notmatch 'function Get-Tc0100TextCharacter' -or $text -notmatch '0xC06000'){throw 'Workbench validation failed: TC0100SCN RAM character inspector missing'}
if(-not(Test-Path (Join-Path $root 'research\scripts\regression\regression-v06690-game-lab-tile-inspector.chqscript'))){throw 'Workbench validation failed: v0.66.9.0 focused regression missing'}
Write-Host 'v0.66.9.0 Game Lab / tile inspector wiring: PASS' -ForegroundColor Green


# v0.66.9.0-RC2: sprite order/map fixes, speed semantics and strict native ERR propagation.
foreach($a in @('sprite.order.inspect','sprite.order.set','game.sprite.order.inspect','game.sprite.order.set')){if($text.IndexOf("'$a'",[StringComparison]::Ordinal) -lt 0){throw "Workbench validation failed: missing RC2 action $a"}}
if($text -notmatch 'HUD speed:' -or $text -notmatch 'Internal physics' -or $text -notmatch 'preservedRun'){throw 'Workbench validation failed: Game Lab speed semantics/run-state preservation missing'}
if($text -notmatch "requires slot=N or map=N" -or $text -notmatch "No active sprite uses map="){throw 'Workbench validation failed: sprite.map.inspect map selector fix missing'}
if($text -notmatch "match '\^ERR" ){throw 'Workbench validation failed: native ERR script failure propagation missing'}
if($text -notmatch 'f\{2:D6\}'){throw 'Workbench validation failed: frame-safe tile.inspect artifact naming missing'}

# v0.66.9.0-RC2.4.4 script capacity.
if($text -notmatch "Expanded script exceeds 1000 commands" -or $text -notmatch "expandedCommands=1000"){throw 'Workbench validation failed: RC2.4.4 expanded-command limit parity missing'}
Write-Host 'v0.66.9.0-RC2.4.4 script capacity: PASS' -ForegroundColor Green

# v0.66.9.0-RC2.5 sprite/script lifecycle reliability.
if($text -notmatch "'frame\.snapshot\.assert-overlap'" -or $text -notmatch "'timeline\.reset'" -or $text -notmatch 'Schema action is not exposed through Script Console allow-list'){throw 'Workbench validation failed: RC2.5 Script Console/schema parity wiring missing'}
if($text -notmatch "try requires finally" -or $text -notmatch "kind:'try'" -or $text -notmatch 'ignoreAbort=false' -or $text -notmatch "return compileClientScript\(text,localVars\)"){throw 'Workbench validation failed: RC2.5 try/finally/nested-block compiler wiring missing'}
if($text -notmatch 'TimelineResetCheckpoint' -or $text -notmatch "'timeline.reset'"){throw 'Workbench validation failed: RC2.5 timeline.reset transaction wiring missing'}
if($text -notmatch 'currentSession=\(Get-SessionId\)' -or $text -notmatch 'defaultHistorySession' -or $text -notmatch 'historyDefaultApplied'){throw 'Workbench validation failed: RC2.5 current-session history default wiring missing'}
if($text -notmatch "scriptPath:src.path\|\|'ad-hoc'"){throw 'Workbench validation failed: RC2.5 diagnostic script source attribution fix missing'}
if(-not(Test-Path (Join-Path $root 'research\scripts\regression\regression-v06690-rc25-script-safety.chqscript'))){throw 'Workbench validation failed: RC2.5 script-safety regression missing'}
if($full -notmatch 'regression-v06690-rc25-script-safety\.chqscript' -or $full -notmatch 'regression-sprite-ownership\.chqscript'){throw 'Workbench validation failed: RC2.5 permanent regression wiring missing'}
Write-Host 'v0.66.9.0-RC2.5 script lifecycle / action parity: PASS' -ForegroundColor Green



# v0.66.9.0-RC2.9: target-health promotion, research controls, SDL pause UX and long-run parity.
if($nativeVersion -in @('0.66.9.0-RC2.9','0.66.9.0-RC3.0')){
    foreach($cp in @(
        'stage1-target-immediate-pre-contact-5587.chqstate',
        'stage1-target-first-physical-contact-5588.chqstate',
        'stage1-target-immediate-pre-damage-6333.chqstate',
        'stage1-target-first-damage-6334.chqstate'
    )){if(-not(Test-Path (Join-Path $root ('checkpoints\'+$cp)))){throw "Workbench validation failed: RC2.9 checkpoint missing: $cp"}}
    if($text -notmatch "'game\.target\.one-hit'" -or $text -notmatch 'gameTargetOneHit' -or $text -notmatch 'One-hit Target Kill'){throw 'Workbench validation failed: RC2.9 one-hit target API/UI wiring missing'}
    if($text -notmatch 'Pause/Break toggles pause/resume' -or $text -notmatch 'panel w7"><h2>Current SDL frame' -or $text -notmatch 'panel w5"><h2>SDL Window Controls' -or $text -notmatch '@media\(max-width:1200px\)'){throw 'Workbench validation failed: RC2.9 SDL pause/responsive Dashboard wiring missing'}
    if($text -notmatch "'control\.run-frames'=\[ordered\]@\{params=@\('frames=1\.\.100000 required','timeout=1000\.\.3600000 optional default 900000 ms'\)" ){throw 'Workbench validation failed: RC2.9 run-frames schema/default timeout mismatch'}
    if(-not(Test-Path (Join-Path $root 'research\scripts\regression\regression-v06690-rc29-target-health-tooling.chqscript'))){throw 'Workbench validation failed: RC2.9 focused regression missing'}
    $apiDoc=Get-Content (Join-Path $root 'docs\API_REFERENCE.md') -Raw
    if($apiDoc -notmatch 'default is 900000 ms' -or $apiDoc -notmatch 'game\.target\.one-hit'){throw 'Workbench validation failed: RC2.9 API documentation is stale'}
    $gameplayDoc=Get-Content (Join-Path $root 'docs\chasehq\discoveries\gameplay-state.md') -Raw
    if($gameplayDoc -notmatch '0x1002AE' -or $gameplayDoc -notmatch 'FFFF'){throw 'Workbench validation failed: RC2.9 target-health documentation is stale'}
    Write-Host 'v0.66.9.0-RC2.9 target health / SDL UX / timeout parity: PASS' -ForegroundColor Green
}


# v0.66.9.0-RC3.0: live course-mapping / survey parity and evidence surfaces.
if($nativeVersion -eq '0.66.9.0-RC3.0'){
    foreach($needle in @("'course.follow.status'","'course.follow.configure'","'course.survey.start'","'course.survey.stop'","'track.map.export'",'Course Mapping / Live Survey','UNCLASSIFIED_DYNAMIC_OBJECT','surfaceSignature','carX','roadLeftX')){if($text -notmatch [regex]::Escape($needle)){throw "Workbench validation failed: RC3.0 mapping surface missing: $needle"}}
    foreach($f in @('stage1-course-mapping-shakedown-600.chqscript','stage1-course-mapping-3600.chqscript','stage1-course-mapping-negative-bias.chqscript','stage1-course-mapping-positive-bias.chqscript')){if(-not(Test-Path (Join-Path $root ('research\scripts\track\surveys\'+$f)))){throw "Workbench validation failed: RC3.0 survey script missing: $f"}}
    if(-not(Test-Path (Join-Path $root 'research\scripts\regression\regression-v06690-rc30-course-mapping-live-survey.chqscript'))){throw 'Workbench validation failed: RC3.0 focused regression missing'}
    $apiDoc=Get-Content (Join-Path $root 'docs\API_REFERENCE.md') -Raw
    if($apiDoc -notmatch 'course\.follow\.configure' -or $apiDoc -notmatch 'track\.map\.export'){throw 'Workbench validation failed: RC3.0 API documentation is stale'}
    Write-Host 'v0.66.9.0-RC3.0 live course mapping / survey parity: PASS' -ForegroundColor Green
}
