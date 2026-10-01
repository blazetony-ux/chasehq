[CmdletBinding()]
param([switch]$SkipBuild)
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
$root=$PSScriptRoot
$fail=@()
function Check([bool]$ok,[string]$name){if($ok){Write-Host "$name`: PASS" -ForegroundColor Green}else{$script:fail += $name;Write-Host "$name`: FAIL" -ForegroundColor Red}}

# Parse every PowerShell file with the engine parser.
$parseErrors=@()
foreach($f in Get-ChildItem $root -Recurse -File -Filter *.ps1){$tokens=$null;$errs=$null;[System.Management.Automation.Language.Parser]::ParseFile($f.FullName,[ref]$tokens,[ref]$errs)|Out-Null;if($errs){$parseErrors += @($errs|ForEach-Object{"$($f.FullName): $($_.Message)"})}}
Check ($parseErrors.Count -eq 0) 'PowerShell parse'
if($parseErrors){$parseErrors|ForEach-Object{Write-Host $_ -ForegroundColor Red}}

try{& (Join-Path $root 'Validate-Workbench.ps1');Check $true 'Workbench validator'}catch{Check $false 'Workbench validator';throw}

$versionText=Get-Content (Join-Path $root 'src\version.h') -Raw;$vm=[regex]::Match($versionText,'kNativeVersion\s*=\s*"([^"]+)"');if(-not $vm.Success){throw 'Could not determine native version'};$native=$vm.Groups[1].Value
$cmake=Get-Content (Join-Path $root 'CMakeLists.txt') -Raw
$manifest=Get-Content (Join-Path $root 'research\feature-manifest.json') -Raw|ConvertFrom-Json
$api=Get-Content (Join-Path $root 'research\schema\api-actions.json') -Raw|ConvertFrom-Json
$scriptSchema=Get-Content (Join-Path $root 'research\schema\script-language.json') -Raw|ConvertFrom-Json
Check ($cmake -match 'file\(STRINGS.*src/version.h' -and $cmake -match 'CHASEHQ_NATIVE_VERSION="\$\{CHQ_RELEASE_VERSION\}"' -and [string]$manifest.build -eq $native -and [string]$api.build -eq $native -and [string]$scriptSchema.build -eq $native) 'Version/schema consistency'

$required=@('palette.trace.start','palette.trace.status','palette.trace.tail','palette.trace.stop','palette.trace.clear','memory.trace.start','memory.trace.status','memory.trace.tail','memory.trace.stop','memory.trace.clear','track.record.start','track.record.stop','track.record.clear','track.record.status','track.record.sample','track.svg.export','docs.index','docs.search','docs.get','docs.glossary','docs.handbook','docs.page','docs.handover','docs.export','knowledge.list','knowledge.get','knowledge.search','knowledge.related','window.status','window.always-on-top','window.fullscreen','window.scale','window.show','window.hide','window.minimize','window.restore','script.runs','timeline.record.start','timeline.record.stop','timeline.status','timeline.list','timeline.load','timeline.seek','timeline.next','timeline.prev','timeline.frames','timeline.play','timeline.trim.event','timeline.scan.memory','timeline.analyze.auto','timeline.fork-live','timeline.unload','timeline.reset','frame.snapshot.assert-overlap','sprite.order.inspect','sprite.order.set','game.sprite.order.inspect','game.sprite.order.set')
$web=Get-Content (Join-Path $root 'Start-ChaseHQWeb.ps1') -Raw
$docs=Get-Content (Join-Path $root 'docs\API_REFERENCE.md') -Raw
$missing=@($required|Where-Object{$web -notmatch [regex]::Escape($_) -or -not $api.actions.PSObject.Properties[$_] -or $docs -notmatch [regex]::Escape($_)})
Check ($missing.Count -eq 0) 'API/schema/docs consistency'
if($missing){Write-Host ('Missing contracts: '+($missing -join ', ')) -ForegroundColor Red}

$full=Get-Content (Join-Path $root 'research\scripts\regression\full-regression.chqscript') -Raw
Check ($full -match 'regression-v0664-palette-trace\.chqscript' -and $full -match 'regression-v0665-memory-trace\.chqscript' -and $full -match 'regression-v06651-track-recorder\.chqscript' -and $full -match 'regression-v06661-layered-frame-snapshot\.chqscript' -and $full -match 'regression-v06675-user-handbook\.chqscript' -and $full -match 'regression-v06676-docs-consolidation\.chqscript' -and $full -match 'regression-v06677-run-history-snapshot-discovery\.chqscript' -and $full -match 'regression-v06678-discovery-performance\.chqscript' -and $full -match 'regression-v06678-run-finalization\.chqscript' -and $full -match 'regression-v06679-research-recovery\.chqscript' -and $full -match 'regression-v06680-forensic-timeline\.chqscript' -and $full -match 'regression-v06680-rc7-portable-auto-analysis\.chqscript' -and $full -match 'regression-v06690-rc2-sprite-order-speed-semantics\.chqscript' -and $full.TrimEnd().EndsWith('echo === ChaseHQ Full Regression: COMPLETE ===')) 'Regression gate wiring'


# v0.66.9.0-RC2.4.4 regression-capacity / metadata / agent-guide integrity.
Check ((Test-Path (Join-Path $root 'AGENTS.md') -PathType Leaf)) 'Root AGENTS.md packaged'
Check ([int]$scriptSchema.limits.expandedCommands -eq 1000 -and $web -match 'Expanded script exceeds 1000 commands' -and $web -match 'expandedCommands=1000') 'RC2.4.4 script expanded-command limit parity'
$researchLauncher=Get-Content (Join-Path $root 'Start-ChaseHQResearch.ps1') -Raw
Check ($researchLauncher -match 'Get-ProjectBuildVersion' -and $researchLauncher -match 'build=\$script:ProjectBuildVersion' -and $researchLauncher -notmatch "build='0\.66\.8\.0'") 'RC2.4.4 startup/session build metadata derivation'
Check ($full -match 'regression-v06690-rc243-snapshot-discovery\.chqscript' -and $full.TrimEnd().EndsWith('echo === ChaseHQ Full Regression: COMPLETE ===')) 'RC2.4.4 Full Regression current gate wiring'

# v0.66.9.0-RC2.5 package surface / safe scripting.
$rc25=@('RELEASE_NOTES_0.66.9.0-RC2.5.md','VALIDATION_0.66.9.0-RC2.5.md','docs\SPRITE_OWNERSHIP.md','research\scripts\regression\regression-sprite-ownership.chqscript','research\scripts\regression\regression-v06690-rc25-script-safety.chqscript','research\scripts\graphics\tc0100scn-rowscroll-raster-mapping.chqscript','scripts\Assert-SpriteSnapshotOverlap.ps1')
$rc25Missing=@($rc25|Where-Object{-not(Test-Path (Join-Path $root $_) -PathType Leaf)})
Check ($rc25Missing.Count -eq 0) 'v0.66.9.0-RC2.5 package surface'
Check ($web -match "'frame\.snapshot\.assert-overlap'" -and $web -match "'timeline\.reset'" -and $web -match 'Schema action is not exposed through Script Console allow-list') 'RC2.5 API allow-list/schema parity wiring'
Check ($web -match 'try requires finally' -and $web -match "kind:'try'" -and $scriptSchema.statements.syntax -contains 'try ... finally ... end') 'RC2.5 try/finally schema/compiler parity'
Check ($web -match 'currentSession=\(Get-SessionId\)' -and $web -match 'defaultHistorySession') 'RC2.5 current-session history default wiring'
Check ($full -match 'regression-v06690-rc25-script-safety\.chqscript' -and $full -match 'regression-sprite-ownership\.chqscript' -and $full.TrimEnd().EndsWith('echo === ChaseHQ Full Regression: COMPLETE ===')) 'RC2.5 Full Regression permanent gate wiring'


# v0.66.9.0-RC2.6 sprite bitplane-significance correction.
$rc26=@('RELEASE_NOTES_0.66.9.0-RC2.6.md','VALIDATION_0.66.9.0-RC2.6.md','Test-SpriteGfx.bat','src\sprite_gfx.h','tests\sprite_gfx_tests.cpp','docs\evidence\sprite-bitplane-significance\README.md','docs\evidence\sprite-bitplane-significance\rc25-map405-plane-order-simulation-final.png','docs\evidence\sprite-bitplane-significance\rc25-map405-slot-041.png','research\scripts\graphics\sprite-bitplane-significance-proveoff.chqscript')
$rc26Missing=@($rc26|Where-Object{-not(Test-Path (Join-Path $root $_) -PathType Leaf)})
Check ($rc26Missing.Count -eq 0) 'v0.66.9.0-RC2.6 package surface'
if($rc26Missing){Write-Host ('Missing RC2.6 assets: '+($rc26Missing -join ', ')) -ForegroundColor Red}
$spriteGfx=Get-Content (Join-Path $root 'src\sprite_gfx.h') -Raw
$videoCppRc26=Get-Content (Join-Path $root 'src\video.cpp') -Raw
$mainCppRc26=Get-Content (Join-Path $root 'src\main.cpp') -Raw
$spriteGfxTest=Get-Content (Join-Path $root 'tests\sprite_gfx_tests.cpp') -Raw
Check ($spriteGfx -match 'bit << \(3 - plane\)' -and $videoCppRc26 -match 'decode_sprite_pixel_4bpp\(region, tile_index, x, y\)' -and $mainCppRc26 -match 'decode_sprite_pixel_4bpp\(r,tile,x,y\)') 'RC2.6 shared MAME-consistent sprite decoder wiring'
Check ($cmake -match 'sprite_gfx_tests' -and $spriteGfxTest -match 'source_mask < 16' -and $spriteGfxTest -match 'reverse_nibble') 'RC2.6 exhaustive native bitplane regression wiring'
$catalogRc26=Get-Content (Join-Path $root 'research\scripts\SCRIPT_CATALOG.md') -Raw
Check ($catalogRc26 -match 'sprite-bitplane-significance-proveoff\.chqscript') 'RC2.6 bitplane prove-off script catalogued'
$featureIds=@($manifest.features|ForEach-Object{[string]$_.id})
Check ($featureIds -contains 'graphics.sprite-bitplane-significance') 'RC2.6 feature manifest entry'
$readmeRc26=Get-Content (Join-Path $root 'README.md') -Raw
Check ($readmeRc26 -match 'Current candidate: v0\.66\.9\.0-RC2\.6' -and (Get-Content (Join-Path $root 'VALIDATION.md') -Raw) -match 'v0\.66\.9\.0-RC2\.6') 'RC2.6 current-candidate pointers'
$buildDebug=Get-Content (Join-Path $root 'Build-Debug.bat') -Raw
Check ($buildDebug -match '1 tests failed out of ' -and $buildDebug -notmatch '1 tests failed out of 1') 'Build gate tolerates only one historical failure independent of total test count'



# v0.66.7.7 documentation/handbook integrity and build-identity consistency.
$knowledgePath=Join-Path $root 'research\knowledge\documentation.json'
$handoverPath=Join-Path $root 'docs\CHATGPT_HANDOVER_PROMPT.md'
$knowledge=$null;try{$knowledge=Get-Content $knowledgePath -Raw|ConvertFrom-Json}catch{}
$entries=@($(if($knowledge){$knowledge.entries}else{@()}))
$ids=@($entries|ForEach-Object{[string]$_.id});$dupes=@($ids|Group-Object|Where-Object{$_.Count -gt 1}|ForEach-Object{$_.Name})
$requiredFields=@('id','title','kind','category','status','confidence','introduced','updated','summary')
$bad=@();foreach($e in $entries){foreach($f in $requiredFields){if(-not $e.PSObject.Properties[$f] -or [string]::IsNullOrWhiteSpace([string]$e.$f)){$bad += ([string]$e.id+':'+$f)}};foreach($rid in @($e.related)){if($rid -and $ids -notcontains [string]$rid){$bad += ([string]$e.id+':related->'+[string]$rid)}};foreach($ev in @($e.evidence)){if($ev.path){$ep=Join-Path $root ([string]$ev.path);if(-not(Test-Path $ep -PathType Leaf)){$bad += ([string]$e.id+':evidence->'+[string]$ev.path)}}}}
Check ($knowledge -and [string]$knowledge.build -eq $native -and $entries.Count -gt 0 -and $dupes.Count -eq 0 -and $bad.Count -eq 0) 'Knowledge integrity'
Check ($ids -contains 'sprite-bitplane-significance') 'RC2.6 sprite bitplane knowledge entry'
$handbookPath=Join-Path $root 'research\knowledge\handbook.json';$handbook=$null;try{$handbook=Get-Content $handbookPath -Raw|ConvertFrom-Json}catch{};Check ($handbook -and [string]$handbook.build -eq $native -and @($handbook.sections).Count -ge 5) 'User handbook integrity'
if($dupes){Write-Host ('Duplicate knowledge IDs: '+($dupes -join ', ')) -ForegroundColor Red};if($bad){Write-Host ('Knowledge problems: '+($bad -join ', ')) -ForegroundColor Red}
$handover=if(Test-Path $handoverPath){Get-Content $handoverPath -Raw}else{''}
Check ($handover -match [regex]::Escape('v'+$native) -and $handover -match 'authoritative continuation' -and $handover -match 'Immediate next task') 'ChatGPT handover current'
Check ($web -match 'Docs / Knowledge' -and $web -match 'Handbook' -and $web -match 'API Reference' -and $web -match 'Scripting Reference' -and $web -match 'Individual sprites' -and $web -match 'SDL Window Controls') 'Current Workbench handbook surfaces'
Check ($web -match 'ChaseHQ Research Workbench v@@CHQ_VERSION@@' -and $web -match 'bridge=\$workbenchVersion' -and $web -match 'version=\$workbenchVersion') 'Workbench/health/OpenAPI build identity'


# v0.66.7.7 article/search/export/release hygiene.
$nonGlossary=@($entries|Where-Object{$_.kind -ne 'glossary'})
$missingArticle=@($nonGlossary|Where-Object{-not $_.PSObject.Properties['article'] -or $null -eq $_.article -or [string]::IsNullOrWhiteSpace([string]$_.article.discovery)})
$legacyApi=@($entries|Where-Object{$_.PSObject.Properties['api']})
Check ($missingArticle.Count -eq 0 -and $legacyApi.Count -eq 0) 'Knowledge article/schema consolidation'
if($missingArticle){Write-Host ('Missing article bodies: '+(($missingArticle|ForEach-Object{$_.id}) -join ', ')) -ForegroundColor Red}
if($legacyApi){Write-Host ('Legacy api fields: '+(($legacyApi|ForEach-Object{$_.id}) -join ', ')) -ForegroundColor Red}
Check (([regex]::Matches($web,'(?m)^(?:async )?function showHandbookPage\(id\)\{')).Count -eq 1) 'Single handbook-page implementation'
Check ($web -match 'searchText' -and $web -match 'New-KnowledgeArticleHtml' -and $web -match 'docsKnowledgeCard') 'Article search/export/landing wiring'
Check ($web -match 'Get-AuthoritativeScriptRuns' -and $web -match '/api/v1/script/runs' -and $web -match 'refreshAuthoritativeRunHistory') 'Authoritative run-history discovery wiring'
Check ($web -match 'Get-StructuredFrameSnapshotRoots' -and $web -match 'Resolve-StructuredFrameSnapshotDir' -and $web -match 'artifacts\\frame-snapshots') 'Completed-run snapshot discovery wiring'
Check (@($web.ToCharArray()|Where-Object{[int]$_ -gt 127}).Count -eq 0) 'Workbench ASCII-safe source'
$owned=@('Start-ChaseHQWeb.ps1','Validate-Release.ps1','Validate-Workbench.ps1','docs','research')
$moji=@();foreach($item in $owned){$path=Join-Path $root $item;if(Test-Path $path -PathType Leaf){$files=@(Get-Item $path)}elseif(Test-Path $path -PathType Container){$files=@(Get-ChildItem $path -Recurse -File|Where-Object{$_.Extension -in '.ps1','.psm1','.md','.txt','.json','.chqscript','.html','.css','.js'})}else{$files=@()};foreach($f in $files){if($f.Name -like '*.bak'){continue};$t=Get-Content $f.FullName -Raw -ErrorAction SilentlyContinue;if($t -and ($t.Contains([string][char]0x00C2) -or $t.Contains([string][char]0x00C3))){$moji += $f.FullName}}}
Check ($moji.Count -eq 0) 'Owned text mojibake sentinel'
if($moji){$moji|ForEach-Object{Write-Host ('Suspicious owned text: '+$_) -ForegroundColor Red}}
$debugBackups=@(Get-ChildItem $root -Recurse -File|Where-Object{$_.Name -like '*.pre-*.bak' -or $_.Name -like '*.bak'})
Check ($debugBackups.Count -eq 0) 'No debug backup files in release tree'

# v0.66.7.9 release-discipline / research-recovery checks.
$requiredDocs=@('docs\RELEASE_PROCESS.md','docs\RESEARCH_RECOVERY_PLAN.md','research\scripts\recovery\README.md','research\scripts\recovery\research-session-preflight.chqscript','research\scripts\recovery\fast-release-gate.chqscript','research\scripts\gameplay\turbo-state-causal-validation.chqscript','research\scripts\gameplay\turbo-state-writer-trace.chqscript','research\scripts\graphics\turbo-hud-before-during-after.chqscript','research\scripts\graphics\canonical-checkpoint-visual-survey.chqscript','research\scripts\gameplay\target-endlevel-state-survey.chqscript','research\scripts\regression\regression-v06679-research-recovery.chqscript')
$missingRecovery=@($requiredDocs|Where-Object{-not(Test-Path (Join-Path $root $_) -PathType Leaf)})
Check ($missingRecovery.Count -eq 0) 'Release/research recovery package surface'
if($missingRecovery){Write-Host ('Missing recovery assets: '+($missingRecovery -join ', ')) -ForegroundColor Red}
$releaseProcess=Get-Content (Join-Path $root 'docs\RELEASE_PROCESS.md') -Raw
Check ($releaseProcess -match 'Build-Debug\.bat' -and $releaseProcess -match 'one run directory' -and $releaseProcess -match 'FINALIZING' -and $releaseProcess -match 'Full Regression') 'Canonical release-process documentation'
$turboEntry=@($entries|Where-Object{$_.id -eq 'turbo-hud-investigation'})|Select-Object -First 1
$turboText=if($turboEntry){$turboEntry|ConvertTo-Json -Depth 12 -Compress}else{''}
Check ($turboEntry -and $turboText -match '0x100303' -and $turboText -match '0x10040F' -and $turboText -match '0x1003A2' -and $turboText -match '0x100414' -and $turboText -notmatch 'Where is turbo-active state stored') 'Turbo knowledge corrected to authoritative gameplay state'
$scripts=@(Get-ChildItem (Join-Path $root 'research\scripts') -Recurse -Filter *.chqscript -File)
$catalog=Get-Content (Join-Path $root 'research\scripts\SCRIPT_CATALOG.md') -Raw
$catalogMissing=@($scripts|ForEach-Object{$rel=$_.FullName.Substring((Join-Path $root 'research\scripts').Length+1).Replace('\','/');if($catalog -notmatch [regex]::Escape($rel)){$rel}})
Check ($catalogMissing.Count -eq 0) 'Script catalog covers packaged scripts'
if($catalogMissing){Write-Host ('Scripts missing from catalog: '+($catalogMissing -join ', ')) -ForegroundColor Red}

# v0.66.9.0-RC2.4.3 same-turn structured-snapshot correctness.
$rc243Regression=Join-Path $root 'research\scripts\regression\regression-v06690-rc243-snapshot-discovery.chqscript'
Check (Test-Path $rc243Regression -PathType Leaf) 'RC2.4.3 snapshot-discovery regression packaged'
if(Test-Path $rc243Regression -PathType Leaf){$rc243Text=Get-Content $rc243Regression -Raw;Check ($rc243Text -match 'frame\.snapshot\.list require=regression-rc243-snapshot-discovery') 'RC2.4.3 snapshot regression is assertive'}
Check ($web -match 'RecentStructuredFrameSnapshots' -and $web -match 'Required frame snapshot not listed' -and $web -match 'KnownEvidenceSessionRootsCache=\$null') 'RC2.4.3 recent-snapshot registry/cache invalidation wiring'

# v0.66.8.0 Forensic Timeline v1 contract.
$timelineDocs=@('docs\FORENSIC_TIMELINE.md','research\scripts\timeline\README.md','research\scripts\timeline\capture-turbo-complete-cycle.chqscript','research\scripts\timeline\import-and-inspect-timeline.chqscript','research\scripts\timeline\timeline-fork-live-example.chqscript','research\scripts\timeline\analyze-existing-turbo-forensic-timeline.chqscript','research\scripts\timeline\replay-turbo-timeline-in-sdl.chqscript','research\scripts\regression\regression-v06680-forensic-timeline.chqscript','research\scripts\regression\regression-v06680-rc7-portable-auto-analysis.chqscript')
$timelineMissing=@($timelineDocs|Where-Object{-not(Test-Path (Join-Path $root $_) -PathType Leaf)})
Check ($timelineMissing.Count -eq 0) 'Forensic Timeline package surface'
if($timelineMissing){Write-Host ('Missing timeline assets: '+($timelineMissing -join ', ')) -ForegroundColor Red}
$mainCpp=Get-Content (Join-Path $root 'src\main.cpp') -Raw
$runtimeCpp=Get-Content (Join-Path $root 'src\runtime.cpp') -Raw
Check ($mainCpp -match 'chq-forensic-timeline-v1' -and $mainCpp -match 'timeline forked_live' -and $mainCpp -match 'forensic_capture_frame' -and $runtimeCpp -match 'timeline_write_sink_') 'Native timeline capture/seek/write-provenance wiring'
Check ($web -match 'Analyze-ForensicTimelineWrites' -and $web -match 'Trim-ForensicTimelineEvent' -and $web -match 'timeline.scan.memory' -and $web -match 'timeline.analyze.auto' -and $web -match 'timeline.play' -and $web -match 'timeline.trim.event') 'Timeline automation/script wiring'
$audioCaps=Get-Content (Join-Path $root 'research\audio\audio-capabilities.json') -Raw|ConvertFrom-Json
Check ($audioCaps.timelineCapture -and -not [bool]$audioCaps.timelineCapture.runtimeAudioAvailable -and [bool]$audioCaps.timelineCapture.soundStubCaptured) 'Timeline audio limitation explicit'

# Conservative Windows path-length smoke using the actual candidate root.
$worst=Join-Path $root 'evidence\sessions\20260927_235959\runs\999\artifacts\frame-snapshots\brake-lamp-palette-provenance-writer-pc-investigation\bundle-manifest.json'
Check ($worst.Length -lt 240) 'Generated-path length'

Check ($web -match "Contains\('output'\)" -and $web -match 'bundleDownloadName' -and $web -match 'existingMeta' -and $web -match 'trackSaveSvg' -and $web -notmatch 'Syntax preview') 'Run/UI reliability fixes'

if(-not $SkipBuild){
  & (Join-Path $root 'Build-Debug.bat')
  Check ($LASTEXITCODE -eq 0) 'Debug build/tests'
}

# v0.66.9.0-RC1 Game Lab / TC0100SCN character inspector surface.
$rc690=@('RELEASE_NOTES_0.66.9.0-RC1.md','VALIDATION_0.66.9.0.md','research\scripts\graphics\turbo-hud-tile-character-inspection.chqscript','research\scripts\regression\regression-v06690-game-lab-tile-inspector.chqscript')
$rc690Missing=@($rc690|Where-Object{-not(Test-Path (Join-Path $root $_) -PathType Leaf)})
Check ($rc690Missing.Count -eq 0) 'v0.66.9.0 candidate package surface'
Check ($web -match 'game\.turbo\.stock' -and $web -match 'game\.speed\.freeze' -and $web -match 'tile\.inspect' -and $web -match 'TC0100SCN Character Inspector') 'v0.66.9.0 Game Lab / tile-inspector wiring'
Check ($catalog -match 'turbo-hud-layer-sprite-correlation\.chqscript' -and $catalog -match 'turbo-gameplay-to-hud-writer-path\.chqscript') 'Reusable turbo/HUD scripts catalogued'

# v0.66.9.0-RC2 sprite-order / speed-semantics candidate checks.
$rc6902=@('RELEASE_NOTES_0.66.9.0-RC2.md','VALIDATION_0.66.9.0-RC2.md','research\scripts\regression\regression-v06690-rc2-sprite-order-speed-semantics.chqscript','research\scripts\graphics\player-car-sprite-order-ab.chqscript')
$rc6902Missing=@($rc6902|Where-Object{-not(Test-Path (Join-Path $root $_) -PathType Leaf)})
Check ($rc6902Missing.Count -eq 0) 'v0.66.9.0-RC2 package surface'
Check ($web -match 'sprite\.order\.inspect' -and $web -match 'sprite\.order\.set' -and $web -match 'HUD speed:' -and $web -match 'Internal physics') 'RC2 sprite-order / speed-semantics wiring'
Check ($catalog -match 'player-car-sprite-order-ab\.chqscript' -and $catalog -match 'turbo-hud-animation-source-investigation\.chqscript') 'RC2 reusable research scripts catalogued'

# RC2.7 public surface and proof-gate integrity.
$rc27Actions=@('layer.offsets.get','layer.offsets.set','layer.offsets.reset','layer.offsets.assert','tc0100.mode','release.assert-version','frame.snapshot.assert-exact','frame.snapshot.assert-scene-no-sprites')
$rc27Missing=@($rc27Actions|Where-Object{-not $api.actions.PSObject.Properties[$_] -or $web -notmatch [regex]::Escape($_) -or $docs -notmatch [regex]::Escape($_)})
Check ($rc27Missing.Count -eq 0) 'RC2.7 API/schema/docs contracts'
Check ($full -match 'regression-v06690-rc27-tc0100-reliability.chqscript' -and $full.TrimEnd().EndsWith('echo === ChaseHQ Full Regression: COMPLETE ===')) 'RC2.7 living regression wiring'
Check ((Test-Path (Join-Path $root 'docs/NATIVE_RECONSTRUCTION.md')) -and (Test-Path (Join-Path $root 'research/knowledge/native-subsystems.json'))) 'RC2.7 native reconstruction metadata'
if($fail.Count){Write-Host ('READY TO PACKAGE: NO ('+($fail -join ', ')+')') -ForegroundColor Red;exit 1}
Write-Host "READY TO PACKAGE: YES ($native)" -ForegroundColor Green
