[CmdletBinding()]
param([switch]$SkipBuild)
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
$root=$PSScriptRoot
$fail=@()
function Check([bool]$ok,[string]$name){if($ok){Write-Host "$name`: PASS" -ForegroundColor Green}else{$script:fail += $name;Write-Host "$name`: FAIL" -ForegroundColor Red}}
function Get-SafePropertyValue($Object,[string]$Name,$Default=$null){
    if($null -eq $Object){return $Default}
    $p=$Object.PSObject.Properties[$Name]
    if($null -eq $p){return $Default}
    return $p.Value
}

function Normalize-ReleaseContractText([string]$Text){
    if($null -eq $Text){return ''}
    return $Text.Replace("`r`n","`n").Replace("`r","`n")
}
function Test-ChatGptHandoverContract([string]$Text,[string]$NativeVersion){
    if([string]::IsNullOrWhiteSpace($Text) -or [string]::IsNullOrWhiteSpace($NativeVersion)){return $false}
    $normalized=Normalize-ReleaseContractText $Text
    $hasVersion=$normalized -match [regex]::Escape('v'+$NativeVersion)
    $hasAuthoritative=$normalized -match '(?im)^##[ ]+Authoritative continuation[ ]*$'
    $hasNextTask=$normalized -match '(?im)^##[ ]+Immediate next task[ ]*$'
    return ($hasVersion -and $hasAuthoritative -and $hasNextTask)
}

# PowerShell parse/encoding/PS5.1/protected-variable safety is centralized in Validate-WindowsPreflight.ps1.
try{& (Join-Path $root 'Validate-WindowsPreflight.ps1');Check $true 'Windows preflight validator'}catch{Check $false 'Windows preflight validator';throw}
try{& (Join-Path $root 'Validate-Workbench.ps1');Check $true 'Workbench validator'}catch{Check $false 'Workbench validator';throw}
$workbenchValidator=Get-Content (Join-Path $root 'Validate-Workbench.ps1') -Raw
Check ($workbenchValidator -match [regex]::Escape("[regex]::Escape('Prop `$prev ''frame'' -1')") -and $workbenchValidator -notmatch [regex]::Escape('[regex]::Escape("Prop $prev ''frame'' -1")')) 'Workbench validator strict-mode literal safety'
$windowsPreflight=Join-Path $root 'Validate-WindowsPreflight.ps1'
$prepareProjectText=Get-Content (Join-Path $root 'Prepare-Project.ps1') -Raw
Check ((Test-Path $windowsPreflight -PathType Leaf) -and $prepareProjectText -match 'Validate-Release\.ps1' -and $prepareProjectText -match '-SkipBuild') 'Authoritative release preflight wired into preparation'

$rc32Release=Join-Path $root 'research\scripts\regression\regression-v06690-rc32.chqscript'
$rc32Feature=Join-Path $root 'research\scripts\regression\regression-v06690-rc32-script-parameters.chqscript'
$lanHelper=Join-Path $root 'scripts\Ensure-ChaseHQLanAccess.ps1'
Check ((Test-Path $rc32Release -PathType Leaf) -and (Test-Path $rc32Feature -PathType Leaf)) 'RC3.2 whole-release + focused regression scripts bundled'
$launcherRc32=Get-Content (Join-Path $root 'Start-ChaseHQ.ps1') -Raw
$lanHelperText=if(Test-Path $lanHelper -PathType Leaf){Get-Content $lanHelper -Raw}else{''}
Check ($launcherRc32 -match '\[switch\]\$LocalOnly' -and $launcherRc32 -match '\$effectiveLanAccess' -and $launcherRc32 -match 'Resolve-ChaseHQLanAddress' -and $launcherRc32 -match 'Ensure-ChaseHQLanAccess\.ps1' -and $launcherRc32 -match 'LAN/mobile host smoke: PASS') 'RC3.2 default LAN/mobile launch contract'
Check ((Test-Path $lanHelper -PathType Leaf) -and $launcherRc32 -match 'Ensure-ChaseHQLanAccess\.ps1' -and $lanHelperText.Contains('$frontendPrefixes') -and $lanHelperText -match 'http://127\.0\.0\.1' -and $lanHelperText -match 'http://localhost' -and $lanHelperText -match 'Test-FrontendPrefixPermission') 'RC3.2 complete frontend URL ACL prerequisite automation'
$workbenchBytes=[IO.File]::ReadAllBytes((Join-Path $root 'Start-ChaseHQWeb.ps1'))
Check (@($workbenchBytes|Where-Object{$_ -gt 127}).Count -eq 0) 'Windows PowerShell 5.1 Workbench ASCII safety'

$versionText=Get-Content (Join-Path $root 'src\version.h') -Raw;$vm=[regex]::Match($versionText,'kNativeVersion\s*=\s*"([^"]+)"');if(-not $vm.Success){throw 'Could not determine native version'};$native=$vm.Groups[1].Value
$cmake=Get-Content (Join-Path $root 'CMakeLists.txt') -Raw
$manifest=Get-Content (Join-Path $root 'research\feature-manifest.json') -Raw|ConvertFrom-Json
$api=Get-Content (Join-Path $root 'research\schema\api-actions.json') -Raw|ConvertFrom-Json
$scriptSchema=Get-Content (Join-Path $root 'research\schema\script-language.json') -Raw|ConvertFrom-Json
Check ($cmake -match 'file\(STRINGS.*src/version.h' -and $cmake -match 'CHASEHQ_NATIVE_VERSION="\$\{CHQ_RELEASE_VERSION\}"' -and [string]$manifest.build -eq $native -and [string]$api.build -eq $native -and [string]$scriptSchema.build -eq $native) 'Version/schema consistency'
$handoverContractGoodLf = "# Continue from v$native`n## Authoritative continuation`nCurrent continuation.`n## Immediate next task`nRun the next gate.`n"
$handoverContractGoodCrLf = $handoverContractGoodLf.Replace("`n","`r`n")
$handoverContractGoodCr = $handoverContractGoodLf.Replace("`n","`r")
$handoverContractMissingContinuation = "# Continue from v$native`n## Immediate next task`nRun the next gate.`n"
$handoverContractMissingNext = "# Continue from v$native`n## Authoritative continuation`nCurrent continuation.`n"
$handoverContractStaleVersion = "# Continue from v0.0.0-stale`n## Authoritative continuation`nCurrent continuation.`n## Immediate next task`nRun the next gate.`n"
Check ((Test-ChatGptHandoverContract $handoverContractGoodLf $native) -and (Test-ChatGptHandoverContract $handoverContractGoodCrLf $native) -and (Test-ChatGptHandoverContract $handoverContractGoodCr $native) -and -not(Test-ChatGptHandoverContract $handoverContractMissingContinuation $native) -and -not(Test-ChatGptHandoverContract $handoverContractMissingNext $native) -and -not(Test-ChatGptHandoverContract $handoverContractStaleVersion $native)) 'ChatGPT handover contract self-test (LF/CRLF/CR)'

$required=@('palette.trace.start','palette.trace.status','palette.trace.tail','palette.trace.stop','palette.trace.clear','memory.trace.start','memory.trace.status','memory.trace.tail','memory.trace.stop','memory.trace.clear','track.record.start','track.record.stop','track.record.clear','track.record.status','track.record.sample','track.svg.export','docs.index','docs.search','docs.get','docs.glossary','docs.handbook','docs.page','docs.handover','docs.export','knowledge.list','knowledge.get','knowledge.search','knowledge.related','window.status','window.always-on-top','window.fullscreen','window.scale','window.show','window.hide','window.minimize','window.restore','script.run','script.runs','timeline.record.start','timeline.record.stop','timeline.status','timeline.list','timeline.load','timeline.seek','timeline.next','timeline.prev','timeline.frames','timeline.play','timeline.trim.event','timeline.scan.memory','timeline.analyze.auto','timeline.fork-live','timeline.unload','timeline.reset','frame.snapshot.assert-overlap','sprite.order.inspect','sprite.order.set','game.sprite.order.inspect','game.sprite.order.set','course.follow.status','course.follow.configure','course.follow.start','course.follow.stop','course.follow.reset','course.survey.status','course.survey.start','course.survey.stop','track.map.export')
$web=Get-Content (Join-Path $root 'Start-ChaseHQWeb.ps1') -Raw
$docs=Get-Content (Join-Path $root 'docs\API_REFERENCE.md') -Raw
$missing=@($required|Where-Object{$web -notmatch [regex]::Escape($_) -or -not $api.actions.PSObject.Properties[$_] -or $docs -notmatch [regex]::Escape($_)})
Check ($missing.Count -eq 0) 'API/schema/docs consistency'
if($missing){Write-Host ('Missing contracts: '+($missing -join ', ')) -ForegroundColor Red}

$full=Get-Content (Join-Path $root 'research\scripts\regression\full-regression.chqscript') -Raw
Check ($full -match 'regression-v0664-palette-trace\.chqscript' -and $full -match 'regression-v0665-memory-trace\.chqscript' -and $full -match 'regression-v06651-track-recorder\.chqscript' -and $full -match 'regression-v06661-layered-frame-snapshot\.chqscript' -and $full -match 'regression-v06675-user-handbook\.chqscript' -and $full -match 'regression-v06676-docs-consolidation\.chqscript' -and $full -match 'regression-v06677-run-history-snapshot-discovery\.chqscript' -and $full -match 'regression-v06678-discovery-performance\.chqscript' -and $full -match 'regression-v06678-run-finalization\.chqscript' -and $full -match 'regression-v06679-research-recovery\.chqscript' -and $full -match 'regression-v06680-forensic-timeline\.chqscript' -and $full -match 'regression-v06680-rc7-portable-auto-analysis\.chqscript' -and $full -match 'regression-v06690-rc2-sprite-order-speed-semantics\.chqscript' -and $full -match 'regression-v06690-rc30-course-mapping-live-survey\.chqscript' -and $full -match 'regression-v06690-rc31-course-mapping-quality\.chqscript' -and $full -match 'regression-v06690-rc32\.chqscript' -and $full.TrimEnd().EndsWith('echo === ChaseHQ Full Regression: COMPLETE ===')) 'Regression gate wiring'


# v0.66.9.0-RC2.4.4 regression-capacity / metadata / agent-guide integrity.
Check ((Test-Path (Join-Path $root 'AGENTS.md') -PathType Leaf)) 'Root AGENTS.md packaged'
$scriptLimits=Get-SafePropertyValue $scriptSchema 'limits' $null
$expandedCommands=Get-SafePropertyValue $scriptLimits 'expandedCommands' 0
Check ([int]$expandedCommands -eq 1000 -and $web -match 'Expanded script exceeds 1000 commands' -and $web -match 'expandedCommands=1000') 'RC2.4.4 script expanded-command limit parity'
$researchLauncher=Get-Content (Join-Path $root 'Start-ChaseHQResearch.ps1') -Raw
Check ($researchLauncher -match 'Get-ProjectBuildVersion' -and $researchLauncher -match 'build=\$script:ProjectBuildVersion' -and $researchLauncher -notmatch "build='0\.66\.8\.0'") 'RC2.4.4 startup/session build metadata derivation'
$expectedResearchArgLine='$argLine = (($nativeLaunchArgs | ForEach-Object { Quote-Arg $_ }) -join '' '')'
$forbiddenResearchArgLine='$argLine = (($EmulatorArgs | ForEach-Object { Quote-Arg $_ }) -join '' '')'
Check ($researchLauncher.Contains($expectedResearchArgLine) -and -not $researchLauncher.Contains($forbiddenResearchArgLine) -and $researchLauncher.Contains('Start-Process -FilePath $emu -ArgumentList $argLine')) 'Research launcher executed-argument contract'
$launchSmokeMarker='if ($ValidateLaunchOnly) { $nativeLaunchArgs += ''--help'' }'
$buildDebugSmoke=Get-Content (Join-Path $root 'Build-Debug.bat') -Raw
$buildReleaseSmoke=Get-Content (Join-Path $root 'Build-Release.bat') -Raw
Check ($researchLauncher.Contains('[switch]$ValidateLaunchOnly') -and $researchLauncher.Contains($launchSmokeMarker) -and $buildDebugSmoke.Contains('-ValidateLaunchOnly') -and $buildReleaseSmoke.Contains('-ValidateLaunchOnly')) 'Debug/Release post-build launcher binding smoke gate'
Check ($full -match 'regression-v06690-rc243-snapshot-discovery\.chqscript' -and $full.TrimEnd().EndsWith('echo === ChaseHQ Full Regression: COMPLETE ===')) 'RC2.4.4 Full Regression current gate wiring'

# v0.66.9.0-RC2.5 package surface / safe scripting.
$rc25=@('docs\SPRITE_OWNERSHIP.md','research\scripts\regression\regression-sprite-ownership.chqscript','research\scripts\regression\regression-v06690-rc25-script-safety.chqscript','research\scripts\graphics\tc0100scn-rowscroll-raster-mapping.chqscript','scripts\Assert-SpriteSnapshotOverlap.ps1')
$rc25Missing=@($rc25|Where-Object{-not(Test-Path (Join-Path $root $_) -PathType Leaf)})
Check ($rc25Missing.Count -eq 0) 'v0.66.9.0-RC2.5 package surface'
Check ($web -match "'frame\.snapshot\.assert-overlap'" -and $web -match "'timeline\.reset'" -and $web -match 'Schema action is not exposed through Script Console allow-list') 'RC2.5 API allow-list/schema parity wiring'
$scriptStatements=@(Get-SafePropertyValue $scriptSchema 'statements' @())
$scriptSyntax=@($scriptStatements|ForEach-Object{[string](Get-SafePropertyValue $_ 'syntax' '')})
Check ($web -match 'try requires finally' -and $web -match "kind:'try'" -and $scriptSyntax -contains 'try ... finally ... end') 'RC2.5 try/finally schema/compiler parity'
Check ($web -match 'currentSession=\(Get-SessionId\)' -and $web -match 'defaultHistorySession') 'RC2.5 current-session history default wiring'
Check ($full -match 'regression-v06690-rc25-script-safety\.chqscript' -and $full -match 'regression-sprite-ownership\.chqscript' -and $full.TrimEnd().EndsWith('echo === ChaseHQ Full Regression: COMPLETE ===')) 'RC2.5 Full Regression permanent gate wiring'
# Permanent regression dependency integrity: named forensic timelines must be
# produced by the same regression that loads them. This prevents a clean release
# tree from passing static validation while Full Regression depends on artifacts
# left behind by an unrelated/manual research run.
$regressionDir=Join-Path $root 'research\scripts\regression'
$timelineDependencyProblems=@()
foreach($regressionScript in @(Get-ChildItem $regressionDir -Filter '*.chqscript' -File -ErrorAction Stop)){
    $regText=Get-Content $regressionScript.FullName -Raw
    $starts=@([regex]::Matches($regText,'(?im)^\s*api\s+timeline\.record\.start\s+name=([A-Za-z0-9._-]+)(?:\s|$)')|ForEach-Object{$_.Groups[1].Value.ToLowerInvariant()})
    $loads=@([regex]::Matches($regText,'(?im)^\s*api\s+timeline\.load\s+name=([A-Za-z0-9._-]+)(?:\s|$)')|ForEach-Object{$_.Groups[1].Value.ToLowerInvariant()})
    foreach($timelineName in $loads){
        if($starts -notcontains $timelineName){$timelineDependencyProblems += ($regressionScript.Name+' -> '+$timelineName)}
    }
}
Check ($timelineDependencyProblems.Count -eq 0) 'Regression timeline self-containment'
if($timelineDependencyProblems){Write-Host ('External timeline dependencies: '+($timelineDependencyProblems -join '; ')) -ForegroundColor Red}

$rc25SafetyText=Get-Content (Join-Path $regressionDir 'regression-v06690-rc25-script-safety.chqscript') -Raw
Check ($rc25SafetyText -match 'timeline\.record\.start\s+name=regression-v06690-rc25-script-safety' -and $rc25SafetyText -match 'timeline\.load\s+name=regression-v06690-rc25-script-safety' -and $rc25SafetyText -notmatch 'timeline\.load\s+name=car-shadow-layer-investigation') 'RC2.5 script-safety self-contained timeline fixture'
$roadmapRc33=Get-Content (Join-Path $root 'docs\ROADMAP.md') -Raw
Check ($roadmapRc33 -match '## RC3\.3 release/build hardening backlog' -and $roadmapRc33 -match 'Permanent regressions must be self-contained' -and $roadmapRc33 -match 'three-prefix' -and $roadmapRc33 -match 'child gate failures') 'RC3.3 hardening backlog recorded'
$rc33Regression=Join-Path $root 'research\scripts\regression\regression-v06690-rc33.chqscript'
$rc33Process=Get-Content (Join-Path $root 'docs\RELEASE_PROCESS.md') -Raw
$chqdevBat=Join-Path $root 'chqdev.bat'
$chqdevConfig=Join-Path $root 'chqdev.json'
$chqdevSource=Join-Path $root 'tools\chqdev\main.go'
$chqdevPreflight=Get-Content (Join-Path $root 'tools\chqdev\preflight.go') -Raw
$chqdevQualify=Get-Content (Join-Path $root 'tools\chqdev\qualify.go') -Raw
$chqdevWin=Join-Path $root 'tools\chqdev\bin\chqdev-windows-amd64.exe'
$chqdevLinux=Join-Path $root 'tools\chqdev\bin\chqdev-linux-amd64'
$chqdevDoc=Join-Path $root 'docs\CHQDEV.md'
$legacyQualifyWrappers=@(Get-ChildItem $root -File -Filter 'Qualify-*.bat' -ErrorAction SilentlyContinue)
$legacyInvokeWrappers=@(Get-ChildItem (Join-Path $root 'tools') -File -Filter 'Invoke-RC*Qualification.ps1' -ErrorAction SilentlyContinue)
Check ((Test-Path $chqdevBat -PathType Leaf) -and (Test-Path $chqdevSource -PathType Leaf) -and $chqdevQualify -match 'qualifyCommand' -and $chqdevQualify -match 'promotable' -and $legacyQualifyWrappers.Count -eq 0 -and $legacyInvokeWrappers.Count -eq 0) 'Single canonical chqdev qualification pipeline; version-specific wrappers absent'
Check ((Test-Path $rc33Regression -PathType Leaf) -and $full -match 'regression-v06690-rc33\.chqscript') 'RC3.3 whole-release regression wiring'
Check ($rc33Process -match 'NON-NEGOTIABLE' -and $rc33Process -match 'MUST NOT qualify a mutable working tree' -and $rc33Process -match 'exact immutable ZIP') 'RC3.3 mandatory release-process contract'

$rc34Regression=Join-Path $root 'research\scripts\regression\regression-v06690-rc34.chqscript'
$rc34Test=Join-Path $root 'tests\workbench_rc34_tests.js'
$webRc34=Get-Content (Join-Path $root 'Start-ChaseHQWeb.ps1') -Raw
Check ((Test-Path $rc34Regression -PathType Leaf) -and $full -match 'regression-v06690-rc34\.chqscript') 'RC3.4 whole-release regression wiring'
Check ((Test-Path $rc34Test -PathType Leaf) -and (Test-Path (Join-Path $root 'research\web\qr-v8.js') -PathType Leaf) -and $webRc34 -match '/api/v1/mobile/pairing' -and $webRc34 -match 'ChaseHQQr\.svg' -and $webRc34 -match 'history\.replaceState' -and $webRc34 -match 'location\.replace' -and $webRc34 -match 'Recent Script Runs' -and $webRc34 -match '/api/v1/script/runs') 'RC3.4 QR pairing/mobile history parity'

$rc341Regression=Join-Path $root 'research\scripts\regression\regression-v06690-rc341.chqscript'
$rc341Test=Join-Path $root 'tests\workbench_rc341_tests.js'
$windowsPreflightText=Get-Content (Join-Path $root 'Validate-WindowsPreflight.ps1') -Raw
Check ((Test-Path $rc341Regression -PathType Leaf) -and $full -match 'regression-v06690-rc341\.chqscript') 'RC3.4.1 whole-release regression wiring'
Check ((Test-Path $rc341Test -PathType Leaf) -and $windowsPreflightText -match 'Windows script ASCII policy self-test' -and $windowsPreflightText -match 'Windows-owned script ASCII safety' -and $windowsPreflightText -match 'U\+00B7') 'RC3.4.1 Windows-script ASCII fail-closed policy'

$rc342Regression=Join-Path $root 'research\scripts\regression\regression-v06690-rc342.chqscript'
$rc342Test=Join-Path $root 'tests\workbench_rc342_tests.js'
Check ((Test-Path $rc342Regression -PathType Leaf) -and $full -match 'regression-v06690-rc342\.chqscript') 'RC3.4.2 whole-release regression wiring'
Check ((Test-Path $rc342Test -PathType Leaf) -and (Get-Content $rc342Test -Raw) -match 'handover contract negative self-test' -and (Get-Content $rc342Test -Raw) -match 'Authoritative continuation' -and (Get-Content $rc342Test -Raw) -match 'Immediate next task') 'RC3.4.2 handover-contract fail-closed policy'

$rc343Regression=Join-Path $root 'research\scripts\regression\regression-v06690-rc343.chqscript'
$rc343Test=Join-Path $root 'tests\workbench_rc343_tests.js'
$rc343TestText=if(Test-Path $rc343Test -PathType Leaf){Get-Content $rc343Test -Raw}else{''}
Check ((Test-Path $rc343Regression -PathType Leaf) -and $full -match 'regression-v06690-rc343\.chqscript') 'RC3.4.3 whole-release regression wiring'
Check ((Test-Path $rc343Test -PathType Leaf) -and $rc343TestText -match 'goodCrLf' -and $rc343TestText -match 'goodCr' -and $rc343TestText -match 'Normalize-ReleaseContractText' -and $rc343TestText -match 'current documentation matrix') 'RC3.4.3 newline/document contract fail-closed policy'

$rc344Regression=Join-Path $root 'research\scripts\regression\regression-v06690-rc344.chqscript'
$rc344Test=Join-Path $root 'tests\workbench_rc344_tests.js'
Check ((Test-Path $rc344Regression -PathType Leaf) -and $full -match 'regression-v06690-rc344\.chqscript') 'RC3.4.4 whole-release regression wiring'
Check ((Test-Path $rc344Test -PathType Leaf) -and (Test-Path $chqdevConfig -PathType Leaf) -and (Test-Path $chqdevSource -PathType Leaf) -and (Test-Path $chqdevWin -PathType Leaf) -and (Test-Path $chqdevLinux -PathType Leaf) -and (Test-Path $chqdevDoc -PathType Leaf) -and (Test-Path (Join-Path $root 'chqdev.bat') -PathType Leaf) -and (Test-Path (Join-Path $root 'chqdev.sh') -PathType Leaf)) 'RC3.4.4 chqdev package surface'
$chqdevConfigObject=$null;try{$chqdevConfigObject=Get-Content $chqdevConfig -Raw|ConvertFrom-Json}catch{}
Check ($null -ne $chqdevConfigObject -and [string]$chqdevConfigObject.release -eq $native -and [string]$chqdevConfigObject.schema -eq 'chqdev-config-v1') 'RC3.4.4 chqdev config/native identity parity'

$rc345Regression=Join-Path $root 'research\scripts\regression\regression-v06690-rc345.chqscript'
$rc345Test=Join-Path $root 'tests\workbench_rc345_tests.js'
$chqdevPackage=Get-Content (Join-Path $root 'tools\chqdev\package.go') -Raw
$chqdevDocText=Get-Content $chqdevDoc -Raw
Check ((Test-Path $rc345Regression -PathType Leaf) -and (Test-Path $rc345Test -PathType Leaf) -and $full -match 'regression-v06690-rc345\.chqscript') 'RC3.4.5 whole-release regression wiring'
Check ($chqdevPackage -match 'deterministicZipTime' -and $chqdevPackage -match 'reproducibility' -and $chqdevQualify -match 'chqdev-qualification-v2') 'RC3.4.5 deterministic package/qualification controller'
Check ($chqdevDocText -match '(?i)de facto' -and $chqdevDocText -match '(?i)authoritative build/release' -and $chqdevDocText -match '(?i)compatibility/fallback') 'RC3.4.5 chqdev authoritative process contract'

$rc346Regression=Join-Path $root 'research\scripts\regression\regression-v06690-rc346.chqscript'
$rc346Test=Join-Path $root 'tests\workbench_rc346_tests.js'
$chqdevTrust=Get-Content (Join-Path $root 'tools\chqdev\trust_common.go') -Raw
$chqdevTrustWindows=Get-Content (Join-Path $root 'tools\chqdev\trust_windows.go') -Raw
$chqdevLifecycle=Get-Content (Join-Path $root 'tools\chqdev\runtime_lifecycle.go') -Raw
Check ((Test-Path $rc346Regression -PathType Leaf) -and (Test-Path $rc346Test -PathType Leaf) -and $full -match 'regression-v06690-rc346\.chqscript') 'RC3.4.6 whole-release regression wiring'
Check ($chqdevTrust -match 'windows-trust' -and $chqdevTrustWindows -match 'Zone.Identifier' -and $chqdevLifecycle -match 'ExecutionPolicy' -and $chqdevLifecycle -match 'runtimeCheck') 'RC3.4.6 chqdev trust/runtime lifecycle surface'
Check ($chqdevDocText -match 'Zone.Identifier' -and $chqdevDocText -match 'runtime start\|stop\|restart' -and $chqdevDocText -match 'manual .*Unblock-File') 'RC3.4.6 documented trust/runtime lifecycle contract'

$rc347Regression=Join-Path $root 'research\scripts\regression\regression-v06690-rc347.chqscript'
$rc347Test=Join-Path $root 'tests\workbench_rc347_tests.js'
$rc348Regression=Join-Path $root 'research\scripts\regression\regression-v06690-rc348.chqscript'
$rc348Test=Join-Path $root 'tests\workbench_rc348_tests.js'
$rc349Regression=Join-Path $root 'research\scripts\regression\regression-v06690-rc349.chqscript'
$rc349Test=Join-Path $root 'tests\workbench_rc349_tests.js'
$chqdevRunner=Get-Content (Join-Path $root 'tools\chqdev\script_runner.go') -Raw
$chqdevRegression=Get-Content (Join-Path $root 'tools\chqdev\regression.go') -Raw
$chqdevRuntimeTests=Get-Content (Join-Path $root 'tools\chqdev\runtime_test.go') -Raw
Check ((Test-Path $rc347Regression -PathType Leaf) -and (Test-Path $rc347Test -PathType Leaf) -and $full -match 'regression-v06690-rc347\.chqscript') 'RC3.4.7 fresh qualification / Go runner carry-forward'
Check ((Test-Path $rc348Regression -PathType Leaf) -and (Test-Path $rc348Test -PathType Leaf) -and $full -match 'regression-v06690-rc348\.chqscript') 'RC3.4.8 whole-release regression wiring'
Check ($chqdevRunner -match 'defaults := scriptDefaultVars\(src\)' -and $chqdevRunner -match 'resolveQueries\(src, defaults\)' -and $chqdevRunner -match 'compileScriptLines\(strings\.Split\(src, "\\n"\), defaults\)') 'RC3.4.8 Go runner parameter-default propagation'
Check ($chqdevRuntimeTests -match 'TestGoScriptRunnerParameterDefaults' -and $chqdevRuntimeTests -match 'echo frames=4' -and $chqdevRuntimeTests -match 'control\.run-frames frames=4') 'RC3.4.8 parameter-default integration regression'
Check ($chqdevRegression -match 'go-script-console-v2' -and $chqdevRegression -match 'runPath' -and $chqdevRegression -match 'bundlePath' -and $chqdevQualify -match 'reportFailureDetail') 'RC3.4.8 fail-closed regression diagnostics'
Check ((Test-Path $rc349Regression -PathType Leaf) -and (Test-Path $rc349Test -PathType Leaf) -and $full -match 'regression-v06690-rc349\.chqscript') 'RC3.4.9 clean-CTest whole-release regression wiring'
$runtimeTests349=Get-Content (Join-Path $root 'tests\runtime_tests.cpp') -Raw
$chqdevBuild349=Get-Content (Join-Path $root 'tools\chqdev\build.go') -Raw
$chqdevCfg349=Get-Content (Join-Path $root 'chqdev.json') -Raw
Check (([regex]::Matches($runtimeTests349,'std::make_unique<chq::Runtime>\(').Count -eq 16) -and $runtimeTests349 -notmatch '(?m)^\s*chq::Runtime\s+[A-Za-z_]') 'RC3.4.9 runtime_tests large Runtime objects are heap allocated'
Check (([regex]::Matches($runtimeTests349,'tf\.close\(\);').Count -eq 6) -and $runtimeTests349 -match 'pf\.close\(\);' -and $runtimeTests349 -match 'cf\.close\(\);') 'RC3.4.9 Windows input streams close before cleanup'
Check ($chqdevBuild349 -notmatch 'known historical failure' -and $chqdevBuild349 -notmatch 'KnownCTestFailure' -and $chqdevCfg349 -notmatch 'knownCTestFailure') 'RC3.4.9 chqdev CTest policy has zero tolerated failures'





# v0.66.9.2-RC3 track semantics / evidence-isolation candidate.
$rc92Regression=Join-Path $root 'research\scripts\regression\regression-v06692-rc3.chqscript'
$rc92Test=Join-Path $root 'tests\workbench_v06692_rc3_tests.js'
$trackDoc92=Join-Path $root 'docs\chasehq\discoveries\STAGE1_TRACK_RECORD_0.66.9.2-RC3.md'
Check ((Test-Path $rc92Regression -PathType Leaf) -and (Test-Path $rc92Test -PathType Leaf) -and (Test-Path $trackDoc92 -PathType Leaf) -and $full -match 'regression-v06692-rc3\.chqscript') 'v0.66.9.2-RC3 track semantics/evidence isolation surface'
Check ($web -match 'roadProcessingSelector' -and $web -match 'paletteTransitionSelector' -and $web -match 'environmentTransitionSelector' -and $web -match 'featureParameterSemantic' -and $web -match '\$surfaceSignature=\$roadProcessingSignature' -and $web -match 'surfaceStableSignature=\$roadProcessingSignature') 'v0.66.9.2-RC3 track semantic/signature wiring'
Check ($web -match "if\(\$lop -eq 'reset'\)\{\$script:LastLegacyTracePath=\$null\}" -and $web -match 'LastWriteTimeUtc' -and $web -match "Collect-LegacyTraceArtifact \(Join-Path \$runRoot 'artifacts'\) \$started") 'v0.66.9.2-RC3 legacytrace run-isolation wiring'
Check ($launcherRc32 -match '\$projectBuildVersion' -and $launcherRc32 -notmatch "build = '0\.66\.9\.0'" -and $web -match 'native-startup-phase\.txt' -and $web -match 'diagnostic-state\.json') 'v0.66.9.2-RC3 session/diagnostic metadata wiring'
Check ($web -match "mode='ATTRACT MODE'" -and $web -match "mode='PLAYER GAMEPLAY'") 'v0.66.9.2-RC3 checkpoint context distinction'
$legacyRootReleaseFiles=@(Get-ChildItem $root -File | Where-Object {$_.Name -match '^(BUILD_NOTES_|HOTFIX_NOTES_|RELEASE_NOTES_|VALIDATION_|PACKAGE_MANIFEST_|Qualify-)'})
Check ((Test-Path (Join-Path $root 'CHANGELOG.md') -PathType Leaf) -and (Test-Path (Join-Path $root 'VALIDATION.md') -PathType Leaf) -and $legacyRootReleaseFiles.Count -eq 0 -and $chqdevPreflight -match 'checkReleaseRootHygiene') 'v0.66.9.2-RC3 consolidated release-root hygiene'

# v0.66.9.0-RC2.6 sprite bitplane-significance correction.
$rc26=@('Test-SpriteGfx.bat','src\sprite_gfx.h','tests\sprite_gfx_tests.cpp','docs\evidence\sprite-bitplane-significance\README.md','docs\evidence\sprite-bitplane-significance\rc25-map405-plane-order-simulation-final.png','docs\evidence\sprite-bitplane-significance\rc25-map405-slot-041.png','research\scripts\graphics\sprite-bitplane-significance-proveoff.chqscript')
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
Check ($readmeRc26 -match [regex]::Escape($native) -and (Get-Content (Join-Path $root 'VALIDATION.md') -Raw) -match [regex]::Escape($native)) 'Current candidate pointers'
$buildDebug=Get-Content (Join-Path $root 'Build-Debug.bat') -Raw
$buildRelease=Get-Content (Join-Path $root 'Build-Release.bat') -Raw
$cliEvidenceTest=Get-Content (Join-Path $root 'tests\cli_evidence_tests.ps1') -Raw
Check ($buildDebug -notmatch 'KNOWN_SEGFAULT' -and $buildDebug -notmatch 'KNOWN TEST ISSUE' -and $buildDebug -match 'if not "%CTEST_RC%"=="0"') 'Debug build gate treats every CTest failure as fatal'
Check ($buildRelease -notmatch 'KNOWN_SEGFAULT' -and $buildRelease -notmatch 'KNOWN TEST ISSUE' -and $buildRelease -match 'if not "%CTEST_RC%"=="0"') 'Release build gate treats every CTest failure as fatal'
$currentCliIdentity='$m.build -ne ''0.66.9.0-RC3.4.9'''
$expectedCliIdentity='$m.build -ne ''{0}''' -f $native
Check ($currentCliIdentity -eq $expectedCliIdentity) 'Release validator CLI identity follows native build identity'
$staleCliIdentity='$m.build -ne ''0.66.9.0-RC2.7'''
Check ($cliEvidenceTest.Contains($currentCliIdentity) -and -not $cliEvidenceTest.Contains($staleCliIdentity)) 'CLI evidence test asserts current RC3.4.9 build identity'



# v0.66.7.7 documentation/handbook integrity and build-identity consistency.
$knowledgePath=Join-Path $root 'research\knowledge\documentation.json'
$handoverPath=Join-Path $root 'docs\CHATGPT_HANDOVER_PROMPT.md'
$knowledge=$null;try{$knowledge=Get-Content $knowledgePath -Raw|ConvertFrom-Json}catch{}
$entries=@($(if($knowledge){$knowledge.entries}else{@()}))
$ids=@($entries|ForEach-Object{[string]$_.id});$dupes=@($ids|Group-Object|Where-Object{$_.Count -gt 1}|ForEach-Object{$_.Name})
$requiredFields=@('id','title','kind','category','status','confidence','introduced','updated','summary')
$bad=@()
foreach($e in $entries){
    foreach($f in $requiredFields){
        if(-not $e.PSObject.Properties[$f] -or [string]::IsNullOrWhiteSpace([string]$e.$f)){
            $bad += ([string]$e.id+':'+$f)
        }
    }
    $relatedRefs=@()
    if($e.PSObject.Properties['related']){$relatedRefs=@($e.related)}
    foreach($rid in $relatedRefs){
        if($rid -and $ids -notcontains [string]$rid){$bad += ([string]$e.id+':related->'+[string]$rid)}
    }
    $evidenceRefs=@()
    if($e.PSObject.Properties['evidence']){$evidenceRefs=@($e.evidence)}
    foreach($ev in $evidenceRefs){
        if($null -ne $ev -and $ev.PSObject.Properties['path'] -and -not [string]::IsNullOrWhiteSpace([string]$ev.path)){
            $ep=Join-Path $root ([string]$ev.path)
            if(-not(Test-Path $ep -PathType Leaf)){$bad += ([string]$e.id+':evidence->'+[string]$ev.path)}
        }
    }
}
Check ($knowledge -and [string]$knowledge.build -eq $native -and $entries.Count -gt 0 -and $dupes.Count -eq 0 -and $bad.Count -eq 0) 'Knowledge integrity'
Check ($ids -contains 'sprite-bitplane-significance') 'RC2.6 sprite bitplane knowledge entry'
$handbookPath=Join-Path $root 'research\knowledge\handbook.json';$handbook=$null;try{$handbook=Get-Content $handbookPath -Raw|ConvertFrom-Json}catch{};Check ($handbook -and [string]$handbook.build -eq $native -and @($handbook.sections).Count -ge 5) 'User handbook integrity'
if($dupes){Write-Host ('Duplicate knowledge IDs: '+($dupes -join ', ')) -ForegroundColor Red};if($bad){Write-Host ('Knowledge problems: '+($bad -join ', ')) -ForegroundColor Red}
$handover=if(Test-Path $handoverPath){Get-Content $handoverPath -Raw}else{''}
Check (Test-ChatGptHandoverContract $handover $native) 'ChatGPT handover current'
Check ($web -match 'Docs / Knowledge' -and $web -match 'Handbook' -and $web -match 'API Reference' -and $web -match 'Scripting Reference' -and $web -match 'Individual sprites' -and $web -match 'SDL Window Controls') 'Current Workbench handbook surfaces'
Check ($web -match 'ChaseHQ Research Workbench v@@CHQ_VERSION@@' -and $web -match 'bridge=\$workbenchVersion' -and $web -match 'version=\$workbenchVersion') 'Workbench/health/OpenAPI build identity'


# v0.66.7.7 article/search/export/release hygiene.
$nonGlossary=@($entries|Where-Object{$_.kind -ne 'glossary'})
$missingArticle=@()
foreach($entry in $nonGlossary){
    $article=Get-SafePropertyValue $entry 'article' $null
    if($null -eq $article -or $article -is [string]){$missingArticle += $entry;continue}
    $discovery=Get-SafePropertyValue $article 'discovery' ''
    if([string]::IsNullOrWhiteSpace([string]$discovery)){$missingArticle += $entry}
}
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
$utf8Strict=New-Object System.Text.UTF8Encoding -ArgumentList $false,$true
$moji=@()
$invalidUtf8=@()
foreach($item in $owned){
    $path=Join-Path $root $item
    if(Test-Path $path -PathType Leaf){
        $files=@(Get-Item $path)
    }elseif(Test-Path $path -PathType Container){
        $files=@(Get-ChildItem $path -Recurse -File|Where-Object{$_.Extension -in '.ps1','.psm1','.md','.txt','.json','.chqscript','.html','.css','.js'})
    }else{
        $files=@()
    }
    foreach($f in $files){
        if($f.Name -like '*.bak'){continue}
        try{
            $bytes=[IO.File]::ReadAllBytes($f.FullName)
            $t=$utf8Strict.GetString($bytes)
        }catch{
            $invalidUtf8 += $f.FullName
            continue
        }
        if($t -and ($t.Contains([string][char]0x00C2) -or $t.Contains([string][char]0x00C3))){
            $moji += $f.FullName
        }
    }
}
Check ($invalidUtf8.Count -eq 0) 'Owned text strict UTF-8 decoding'
if($invalidUtf8){$invalidUtf8|ForEach-Object{Write-Host ('Invalid UTF-8 owned text: '+$_) -ForegroundColor Red}}
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
$scriptRoot=Join-Path $root 'research\scripts'
$scriptPaths=@($scripts|ForEach-Object{$_.FullName.Substring($scriptRoot.Length+1).Replace('\','/')})
$catalog=Get-Content (Join-Path $root 'research\scripts\SCRIPT_CATALOG.md') -Raw
$catalogPaths=@([regex]::Matches($catalog,'(?m)^\|\s*([^|\r\n]+\.chqscript)\s*\|')|ForEach-Object{$_.Groups[1].Value.Trim()})
$catalogMissing=@($scriptPaths|Where-Object{$catalogPaths -notcontains $_})
$catalogExtras=@($catalogPaths|Where-Object{$scriptPaths -notcontains $_})
$catalogDuplicates=@($catalogPaths|Group-Object|Where-Object{$_.Count -gt 1}|ForEach-Object{$_.Name})
Check ($catalogMissing.Count -eq 0 -and $catalogExtras.Count -eq 0 -and $catalogDuplicates.Count -eq 0 -and $catalogPaths.Count -eq $scriptPaths.Count) 'Script catalog covers packaged scripts'
if($catalogMissing){Write-Host ('Scripts missing from catalog rows: '+($catalogMissing -join ', ')) -ForegroundColor Red}
if($catalogExtras){Write-Host ('Catalog rows without packaged scripts: '+($catalogExtras -join ', ')) -ForegroundColor Red}
if($catalogDuplicates){Write-Host ('Duplicate catalog script rows: '+($catalogDuplicates -join ', ')) -ForegroundColor Red}

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
$timelineCapture=Get-SafePropertyValue $audioCaps 'timelineCapture' $null
$runtimeAudioAvailable=Get-SafePropertyValue $timelineCapture 'runtimeAudioAvailable' $null
$soundStubCaptured=Get-SafePropertyValue $timelineCapture 'soundStubCaptured' $null
Check ($null -ne $timelineCapture -and $false -eq [bool]$runtimeAudioAvailable -and $true -eq [bool]$soundStubCaptured) 'Timeline audio limitation explicit'

# Conservative Windows path-length smoke using the actual candidate root.
$worst=Join-Path $root 'evidence\sessions\20260927_235959\runs\999\artifacts\frame-snapshots\brake-lamp-palette-provenance-writer-pc-investigation\bundle-manifest.json'
Check ($worst.Length -lt 240) 'Generated-path length'

Check ($web -match "Contains\('output'\)" -and $web -match 'bundleDownloadName' -and $web -match 'existingMeta' -and $web -match 'trackSaveSvg' -and $web -notmatch 'Syntax preview') 'Run/UI reliability fixes'

if(-not $SkipBuild){
  & (Join-Path $root 'Build-Debug.bat')
  Check ($LASTEXITCODE -eq 0) 'Debug build/tests'
}

# v0.66.9.0-RC1 Game Lab / TC0100SCN character inspector surface.
$rc690=@('research\scripts\graphics\turbo-hud-tile-character-inspection.chqscript','research\scripts\regression\regression-v06690-game-lab-tile-inspector.chqscript')
$rc690Missing=@($rc690|Where-Object{-not(Test-Path (Join-Path $root $_) -PathType Leaf)})
Check ($rc690Missing.Count -eq 0) 'v0.66.9.0 candidate package surface'
Check ($web -match 'game\.turbo\.stock' -and $web -match 'game\.speed\.freeze' -and $web -match 'tile\.inspect' -and $web -match 'TC0100SCN Character Inspector') 'v0.66.9.0 Game Lab / tile-inspector wiring'
Check ($catalog -match 'turbo-hud-layer-sprite-correlation\.chqscript' -and $catalog -match 'turbo-gameplay-to-hud-writer-path\.chqscript') 'Reusable turbo/HUD scripts catalogued'

# v0.66.9.0-RC2 sprite-order / speed-semantics candidate checks.
$rc6902=@('research\scripts\regression\regression-v06690-rc2-sprite-order-speed-semantics.chqscript','research\scripts\graphics\player-car-sprite-order-ab.chqscript')
$rc6902Missing=@($rc6902|Where-Object{-not(Test-Path (Join-Path $root $_) -PathType Leaf)})
Check ($rc6902Missing.Count -eq 0) 'v0.66.9.0-RC2 package surface'
Check ($web -match 'sprite\.order\.inspect' -and $web -match 'sprite\.order\.set' -and $web -match 'HUD speed:' -and $web -match 'Internal physics') 'RC2 sprite-order / speed-semantics wiring'
Check ($catalog -match 'player-car-sprite-order-ab\.chqscript' -and $catalog -match 'turbo-hud-animation-source-investigation\.chqscript') 'RC2 reusable research scripts catalogued'

# RC2.7 public surface and proof-gate integrity.
$rc27Actions=@('layer.offsets.get','layer.offsets.set','layer.offsets.reset','layer.offsets.assert','tc0100.mode','release.assert-version','frame.snapshot.assert-exact','frame.snapshot.assert-scene-no-sprites')
$rc27Missing=@($rc27Actions|Where-Object{-not $api.actions.PSObject.Properties[$_] -or $web -notmatch [regex]::Escape($_) -or $docs -notmatch [regex]::Escape($_)})
Check ($rc27Missing.Count -eq 0) 'RC2.7 API/schema/docs contracts'
Check ((Test-Path (Join-Path $root 'research/scripts/regression/regression-v06690-rc28-tc0100-reliability.chqscript') -PathType Leaf) -and $full -match 'regression-v06690-rc28-tc0100-reliability\.chqscript' -and $full.TrimEnd().EndsWith('echo === ChaseHQ Full Regression: COMPLETE ===')) 'TC0100 reliability living regression wiring (RC2.8 supersedes RC2.7)'
Check ((Test-Path (Join-Path $root 'docs/NATIVE_RECONSTRUCTION.md')) -and (Test-Path (Join-Path $root 'research/knowledge/native-subsystems.json'))) 'RC2.7 native reconstruction metadata'
# v0.66.9.4-RC2 native-reconstruction / asset-tooling candidate checks.
$rc6942Regression=Join-Path $root 'research\scripts\regression\regression-v06694-rc2.chqscript'
$rc6942Test=Join-Path $root 'tests\workbench_v06694_rc2_tests.js'
$rc6942Audit=Join-Path $root 'docs\DOCUMENTATION_AUDIT_0.66.9.4-RC2.md'
Check ((Test-Path $rc6942Regression -PathType Leaf) -and (Test-Path $rc6942Test -PathType Leaf) -and (Test-Path $rc6942Audit -PathType Leaf) -and $full -match 'regression-v06694-rc2\.chqscript') 'v0.66.9.4-RC2 focused/full regression package surface'
Check ($web -match 'Native Reconstruction Lab' -and $web -match 'backend\.sprite\.set' -and $web -match 'sprite\.map-override' -and $web -match 'text\.charset\.export' -and $web -match 'hud-layout-card') 'v0.66.9.4-RC2 Workbench native/asset/HUD surface'
Check ((Test-Path (Join-Path $root 'checkpoints\stage2-gameplay-7500.chqstate') -PathType Leaf) -and (Test-Path (Join-Path $root 'checkpoints\stage2-enemy-10514.chqstate') -PathType Leaf)) 'v0.66.9.4-RC2 Stage-2 checkpoint corpus'


# v0.66.9.4-RC3.1.3 qualification-orchestration correction.
$rc694313Test=Join-Path $root 'tests\workbench_v06694_rc313_tests.js'
$rc694313Regression=Join-Path $root 'research\scripts\regression\regression-v06694-rc313.chqscript'
$rc694313Audit=Join-Path $root 'docs\DOCUMENTATION_AUDIT_0.66.9.4-RC3.1.3.md'
$chqRuntime=Get-Content (Join-Path $root 'tools\chqdev\runtime.go') -Raw
$chqLife=Get-Content (Join-Path $root 'tools\chqdev\runtime_lifecycle.go') -Raw
$chqQualify=Get-Content (Join-Path $root 'tools\chqdev\qualify.go') -Raw
Check ((Test-Path $rc694313Test -PathType Leaf) -and (Test-Path $rc694313Regression -PathType Leaf) -and (Test-Path $rc694313Audit -PathType Leaf)) 'v0.66.9.4-RC3.1.3 historical promotion-source package surface'
Check ($chqRuntime -match 'func runtimeStartupCheck' -and $chqRuntime -match 'func runtimeHistoryCheck' -and $chqRuntime -match 'runtimeHistoryHTTPTimeout = 30 \* time\.Second') 'RC3.1.3 split lightweight startup / authoritative history readiness'
Check ($chqLife -match 'runtimeStartupCheck\(root, cfg, ""\)' -and $chqQualify -match 'Authoritative run-history gate START' -and $chqQualify -match 'readinessFailureDetail\(ready\)') 'RC3.1.3 qualification sequencing/diagnostic wiring'

# v0.66.9.4 final promotion identity / regression guard.
$final694Test=Join-Path $root 'tests\workbench_v06694_tests.js'
$final694Regression=Join-Path $root 'research\scripts\regression\regression-v06694.chqscript'
$final694Audit=Join-Path $root 'docs\DOCUMENTATION_AUDIT_0.66.9.4.md'
Check ((Test-Path $final694Test -PathType Leaf) -and (Test-Path $final694Regression -PathType Leaf) -and (Test-Path $final694Audit -PathType Leaf) -and $full -match 'regression-v06694\.chqscript' -and $full -notmatch 'include "regression/regression-v06694-rc313\.chqscript"') 'v0.66.9.4 final focused/full/promotion package surface'

# v0.66.9.5-RC1.1 evidence-driven corrective / diagnostics candidate checks.
$rc6951Test=Join-Path $root 'tests\workbench_v06695_rc11_tests.js'
$rc6951Regression=Join-Path $root 'research\scripts\regression\regression-v06695-rc11.chqscript'
$rc6951Audit=Join-Path $root 'docs\DOCUMENTATION_AUDIT_0.66.9.5-RC1.1.md'
$semanticObjects=Get-Content (Join-Path $root 'research\semantic-objects.json') -Raw
$videoSource=Get-Content (Join-Path $root 'src\video.cpp') -Raw
$runtimeSource=Get-Content (Join-Path $root 'src\runtime.cpp') -Raw
Check ((Test-Path $rc6951Test -PathType Leaf) -and (Test-Path $rc6951Regression -PathType Leaf) -and (Test-Path $rc6951Audit -PathType Leaf) -and $full -match 'regression-v06695-rc11\.chqscript') 'v0.66.9.5-RC1.1 focused/full/package surface'
Check ($web -match 'frame\.snapshot\.parity' -and $web -match 'sprite\.asset-resolve' -and $web -match 'events\.clear') 'v0.66.9.5-RC1.1 snapshot/resolver/run-local-event API wiring'
Check ($web -match "\$allowedFirst=@\('backend'," -and $web -match "cmd\('backend sprites status'\)" -and $web -match "cmd\('backend sprites set " -and $web -match "cmd\('backend sprites reset-verify'\)") 'v0.66.9.5-RC1.1 Native Reconstruction Lab direct-command web-bridge parity'
Check ($web -match '10041C' -and $web -match 'width=32' -and $web -match 'value -shl 16') 'v0.66.9.5-RC1.1 fixed-point 32-bit speed-freeze wiring'
Check ($semanticObjects -match '509' -and $videoSource -match '509' -and $runtimeSource -match 'target_contact' -and $runtimeSource -match '108082') 'v0.66.9.5-RC1.1 semantic player / target-contact / coupled-beacon source wiring'


# v0.66.9.5-RC2 ROM Sprite/Vehicle Atlas + semantic vehicle evidence checks.
$rc6952Test=Join-Path $root 'tests\workbench_v06695_rc2_tests.js'
$rc6952Regression=Join-Path $root 'research\scripts\regression\regression-v06695-rc2.chqscript'
$rc6952Audit=Join-Path $root 'docs\DOCUMENTATION_AUDIT_0.66.9.5-RC2.md'
$mainSource=Get-Content (Join-Path $root 'src\main.cpp') -Raw
Check ((Test-Path $rc6952Test -PathType Leaf) -and (Test-Path $rc6952Regression -PathType Leaf) -and (Test-Path $rc6952Audit -PathType Leaf) -and $full -match 'regression-v06695-rc2\.chqscript') 'v0.66.9.5-RC2 focused/full/package surface'
Check ($web -match 'ROM Sprite / Vehicle Atlas' -and $web -match 'sprite\.rom-atlas\.export' -and $web -match '/api/v1/sprite/rom-atlas/export') 'v0.66.9.5-RC2 Workbench ROM-atlas UI/API wiring'
Check ($mainSource -match 'chq-rom-sprite-atlas-v1' -and $mainSource -match 'b52-38\.34' -and $mainSource -match 'obj-b64' -and $mainSource -match 'effective_map=') 'v0.66.9.5-RC2 native ROM decode + source/effective sprite inspection'
Check ($semanticObjects -match '530' -and $semanticObjects -match '536' -and $semanticObjects -match '598' -and $semanticObjects -match '600' -and $semanticObjects -match '617' -and $semanticObjects -match '78') 'v0.66.9.5-RC2 semantic vehicle-family additions'
Check ((Get-Content $rc6952Regression -Raw) -match 'beacon_live_present=1' -and (Get-Content $rc6952Regression -Raw) -match 'beacon_live_present=0') 'v0.66.9.5-RC2 latency-aware beacon focused gate'

# v0.66.9.5-RC3 final Native Promotion Framework / tooling-freeze checks.
$rc6953Test=Join-Path $root 'tests\workbench_v06695_rc3_tests.js'
$rc6953Regression=Join-Path $root 'research\scripts\regression\regression-v06695-rc3.chqscript'
$rc6953Corpus=Join-Path $root 'research\scripts\graphics\native-promotion-sprites.chqscript'
$rc6953Audit=Join-Path $root 'docs\DOCUMENTATION_AUDIT_0.66.9.5-RC3.md'
$nativeKnowledge=Get-Content (Join-Path $root 'research\knowledge\native-subsystems.json') -Raw
Check ((Test-Path $rc6953Test -PathType Leaf) -and (Test-Path $rc6953Regression -PathType Leaf) -and (Test-Path $rc6953Corpus -PathType Leaf) -and (Test-Path $rc6953Audit -PathType Leaf) -and $full -match 'regression-v06695-rc3\.chqscript') 'v0.66.9.5-RC3 focused/full/native-promotion package surface'
Check ($videoSource -match 'NativeOnly' -and $videoSource -match 'original_fallbacks' -and $videoSource -match 'first-sprite-mismatch' -and $mainSource -match 'promotion-status') 'v0.66.9.5-RC3 native-only hard-disable / divergence evidence native wiring'
Check ($web -match 'Native Promotion Framework' -and $web -match 'nativeSpriteNativeOnly' -and $web -match 'native\.promotion\.assert' -and $web -match '/api/v1/native/progress') 'v0.66.9.5-RC3 Workbench Native Progress / promotion API wiring'
Check ($nativeKnowledge -match 'chq-native-promotion-framework-v1' -and $nativeKnowledge -match 'nativeOnlySupported' -and $nativeKnowledge -match 'Canonical promotion corpus') 'v0.66.9.5-RC3 native maturity/promotion knowledge wiring'

if($fail.Count){Write-Host ('READY TO PACKAGE: NO ('+($fail -join ', ')+')') -ForegroundColor Red;exit 1}
Write-Host "READY TO PACKAGE: YES ($native)" -ForegroundColor Green

