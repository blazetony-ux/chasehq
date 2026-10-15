[CmdletBinding()]
param()
$ErrorActionPreference='Stop'
Set-StrictMode -Version 2.0
$root=$PSScriptRoot
$script:fail=@()
$script:blocked=@()
function Get-OwnedSourceFiles([string]$Filter){
    return @(Get-ChildItem $root -Recurse -File -Filter $Filter | Where-Object{
        $rel=$_.FullName.Substring($root.Length).TrimStart([char]92,[char]47)
        $rel -notmatch '^(?:SDL|roms|out|evidence)[\\/]'
    })
}
function Result([bool]$Ok,[string]$Name,[string]$Detail=''){
    if($Ok){Write-Host ($Name+': PASS') -ForegroundColor Green;return}
    if([string]::IsNullOrWhiteSpace($Detail)){$script:fail += $Name}else{$script:fail += ($Name+': '+$Detail)}
    $suffix='';if(-not [string]::IsNullOrWhiteSpace($Detail)){$suffix=' - '+$Detail}
    Write-Host ($Name+': FAIL'+$suffix) -ForegroundColor Red
}

# Parse all PowerShell sources and report the complete error set.
$parseErrors=@()
foreach($f in @(Get-OwnedSourceFiles '*.ps1')){
    $tokens=$null;$errs=$null
    [System.Management.Automation.Language.Parser]::ParseFile($f.FullName,[ref]$tokens,[ref]$errs)|Out-Null
    foreach($e in @($errs)){if($null -ne $e){$parseErrors += ($f.FullName+': '+$e.Message)}}
}
$detail='';if($parseErrors.Count -ne 0){$detail=($parseErrors -join '; ')}
Result ($parseErrors.Count -eq 0) 'PowerShell parse' $detail
if($parseErrors.Count -ne 0){
    $blockedNames=@(
        'Windows script ASCII policy self-test',
        'Windows-owned script ASCII safety',
        'Workbench ASCII-safe source',
        'Batch launcher ASCII-safe source',
        'Windows PowerShell 5.1 command surface',
        'Reserved-variable rule self-test',
        'Constant/read-only PowerShell variable safety',
        'Validator strict-mode literal safety',
        'Fail-closed CTest build gate',
        'RC3.4.9 native build identity',
        'Default LAN/mobile launcher plumbing',
        'RC3.2 script parameter/evidence plumbing',
        'LAN URL ACL/firewall prerequisite helper',
        'RC3.4.5 whole-release regression wiring',
        'RC3.4.3 text-contract newline invariance',

        'RC3.4.5 chqdev controller surface',
        'Research launcher executed-argument contract',
        'Debug/Release post-build launcher binding smoke gate'
    )
    foreach($name in $blockedNames){
        $script:blocked += ($name+': PowerShell parse failed')
        Write-Host ($name+': BLOCKED - PowerShell parse failed') -ForegroundColor Yellow
    }
    Write-Host ''
    Write-Host ('WINDOWS PREFLIGHT: NOT CLEAN ('+$script:fail.Count+' FAIL, '+$script:blocked.Count+' BLOCKED)') -ForegroundColor Red
    foreach($item in $script:fail){Write-Host (' FAIL - '+$item) -ForegroundColor Red}
    foreach($item in $script:blocked){Write-Host (' BLOCKED - '+$item) -ForegroundColor Yellow}
    throw ('Windows preflight not clean ('+$script:fail.Count+' FAIL, '+$script:blocked.Count+' BLOCKED)')
}

# Windows PowerShell 5.1 source/code-page safety.
# Every project-owned Windows script must be ASCII after an optional UTF-8 BOM.
# This is intentionally broader than the Workbench-only historical check: embedded
# HTML/JavaScript inside .ps1 files is still parsed by Windows PowerShell 5.1.
function Get-AsciiSourceProblems([string]$Path,[string]$Label){
    $problems=@()
    $bytes=[IO.File]::ReadAllBytes($Path)
    $offset=0
    if($bytes.Length -ge 3 -and $bytes[0] -eq 0xEF -and $bytes[1] -eq 0xBB -and $bytes[2] -eq 0xBF){$offset=3}
    $utf8=New-Object System.Text.UTF8Encoding -ArgumentList $false,$true
    try{$text=$utf8.GetString($bytes,$offset,$bytes.Length-$offset)}catch{return @(($Label+': invalid UTF-8 after optional BOM'))}
    $line=1;$column=1
    for($i=0;$i -lt $text.Length;$i++){
        $ch=$text[$i]
        $cp=[int]$ch
        if($cp -gt 127){
            $problems += ('{0}:{1}:{2} U+{3:X4}' -f $Label,$line,$column,$cp)
        }
        if($ch -eq "`n"){$line++;$column=1}else{$column++}
    }
    return @($problems)
}

$asciiRuleProblems=@()
$tmpAscii=Join-Path ([IO.Path]::GetTempPath()) ('chq-ascii-policy-'+[guid]::NewGuid().ToString('N')+'.ps1')
try{
    [IO.File]::WriteAllText($tmpAscii,"Write-Host 'ASCII fixture'`r`n",(New-Object System.Text.UTF8Encoding($false)))
    if(@(Get-AsciiSourceProblems $tmpAscii 'ascii-fixture').Count -ne 0){$asciiRuleProblems += 'ASCII fixture was rejected'}
    [IO.File]::WriteAllText($tmpAscii,("Write-Host 'bad"+[char]0x00B7+"fixture'`r`n"),(New-Object System.Text.UTF8Encoding($false)))
    $negative=@(Get-AsciiSourceProblems $tmpAscii 'non-ascii-fixture')
    if($negative.Count -ne 1 -or $negative[0] -notmatch 'U\+00B7'){$asciiRuleProblems += 'U+00B7 fixture was not rejected with a precise diagnostic'}
}finally{Remove-Item -LiteralPath $tmpAscii -Force -ErrorAction SilentlyContinue}
$detail='';if($asciiRuleProblems.Count -ne 0){$detail=$asciiRuleProblems -join '; '}
Result ($asciiRuleProblems.Count -eq 0) 'Windows script ASCII policy self-test' $detail

$windowsScriptProblems=@()
$seenWindowsScript=@{}
foreach($pattern in @('*.ps1','*.psm1','*.psd1','*.bat','*.cmd')){
    foreach($f in @(Get-OwnedSourceFiles $pattern)){
        if($seenWindowsScript.ContainsKey($f.FullName)){continue}
        $seenWindowsScript[$f.FullName]=$true
        $rel=$f.FullName.Substring($root.Length).TrimStart([char]92,[char]47)
        $windowsScriptProblems += @(Get-AsciiSourceProblems $f.FullName $rel)
    }
}
$detail='';if($windowsScriptProblems.Count -ne 0){$detail=$windowsScriptProblems -join '; '}
Result ($windowsScriptProblems.Count -eq 0) 'Windows-owned script ASCII safety' $detail

$webPath=Join-Path $root 'Start-ChaseHQWeb.ps1'
$webBytes=[IO.File]::ReadAllBytes($webPath)
$webNonAscii=@($webBytes|Where-Object{$_ -gt 127})
$detail='';if($webNonAscii.Count -ne 0){$detail=$webNonAscii.Count.ToString()+' non-ASCII byte(s)'}
Result ($webNonAscii.Count -eq 0) 'Workbench ASCII-safe source' $detail
$badBatch=@()
foreach($f in @(Get-OwnedSourceFiles '*.bat')){
    if(@([IO.File]::ReadAllBytes($f.FullName)|Where-Object{$_ -gt 127}).Count -ne 0){$badBatch += $f.FullName}
}
$detail='';if($badBatch.Count -ne 0){$detail=$badBatch -join '; '}
Result ($badBatch.Count -eq 0) 'Batch launcher ASCII-safe source' $detail

# Reject command surfaces that require newer PowerShell than Windows PowerShell 5.1.
$ps51Files=@('Prepare-Project.ps1','Validate-Workbench.ps1','Validate-Release.ps1','Start-ChaseHQ.ps1','Start-ChaseHQResearch.ps1','Start-ChaseHQMultiResearch.ps1','scripts\Ensure-ChaseHQLanAccess.ps1')
$ps51Problems=@()
foreach($rel in $ps51Files){
    $path=Join-Path $root $rel
    if(-not(Test-Path $path -PathType Leaf)){$ps51Problems += ($rel+' missing');continue}
    $t=Get-Content $path -Raw
    if($t -match 'ForEach-Object\s+-Parallel\b'){$ps51Problems += ($rel+' contains ForEach-Object -Parallel')}
    if($t -match 'ConvertFrom-Json\b[^\r\n]*-AsHashtable\b'){$ps51Problems += ($rel+' contains ConvertFrom-Json -AsHashtable')}
}
$detail='';if($ps51Problems.Count -ne 0){$detail=$ps51Problems -join '; '}
Result ($ps51Problems.Count -eq 0) 'Windows PowerShell 5.1 command surface' $detail

# Catch assignments/parameters that collide with variables this PowerShell runtime actually exposes as Constant or ReadOnly.
# Do not maintain a hand-written list of automatic variables: several are intentionally writable, and `$null = ...`
# is the canonical PowerShell output-discard pattern. Query the runtime's variable options instead.
function Test-ProtectedVariableName([string]$Name){
    if([string]::IsNullOrWhiteSpace($Name)){return $false}
    if($Name -ieq 'null'){return $false}
    $v=Get-Variable -Name $Name -ErrorAction SilentlyContinue
    if($null -eq $v){return $false}
    $opts=[System.Management.Automation.ScopedItemOptions]$v.Options
    $constant=[System.Management.Automation.ScopedItemOptions]::Constant
    $readOnly=[System.Management.Automation.ScopedItemOptions]::ReadOnly
    return ((($opts -band $constant) -ne 0) -or (($opts -band $readOnly) -ne 0))
}

# Self-test the rule before trusting it against the source tree. These expectations are valid on Windows PowerShell 5.1
# and modern PowerShell: $null is writable as a sink, while Host/PID/true/false are protected.
$ruleProblems=@()
if(Test-ProtectedVariableName 'null'){$ruleProblems += '$null incorrectly classified as protected'}
foreach($n in @('Host','PID','true','false')){if(-not(Test-ProtectedVariableName $n)){$ruleProblems += ('$'+$n+' was not classified as protected')}}
if(Test-ProtectedVariableName '__chq_preflight_probe'){$ruleProblems += 'ordinary variable incorrectly classified as protected'}
$detail='';if($ruleProblems.Count -ne 0){$detail=$ruleProblems -join '; '}
Result ($ruleProblems.Count -eq 0) 'Reserved-variable rule self-test' $detail

$astProblemSet=@{}
foreach($f in @(Get-OwnedSourceFiles '*.ps1')){
    $tok=$null;$err=$null;$tree=[System.Management.Automation.Language.Parser]::ParseFile($f.FullName,[ref]$tok,[ref]$err)
    if(@($err).Count -ne 0){continue}
    foreach($node in @($tree.FindAll({param($n) $n -is [System.Management.Automation.Language.AssignmentStatementAst]},$true))){
        $left=$node.Left;if($left -isnot [System.Management.Automation.Language.VariableExpressionAst]){continue}
        $raw=[string]$left.VariablePath.UserPath;if([string]::IsNullOrWhiteSpace($raw) -or $raw.Contains(':')){continue}
        if(Test-ProtectedVariableName $raw){$msg=($f.FullName+':'+$left.Extent.StartLineNumber+': assignment to protected $'+$raw);$astProblemSet[$msg]=$true}
    }
    foreach($node in @($tree.FindAll({param($n) $n -is [System.Management.Automation.Language.ForEachStatementAst]},$true))){
        $raw=[string]$node.Variable.VariablePath.UserPath;if([string]::IsNullOrWhiteSpace($raw) -or $raw.Contains(':')){continue}
        if(Test-ProtectedVariableName $raw){$msg=($f.FullName+':'+$node.Variable.Extent.StartLineNumber+': foreach variable uses protected $'+$raw);$astProblemSet[$msg]=$true}
    }
    foreach($node in @($tree.FindAll({param($n) $n -is [System.Management.Automation.Language.ParameterAst]},$true))){
        $raw=[string]$node.Name.VariablePath.UserPath;if([string]::IsNullOrWhiteSpace($raw) -or $raw.Contains(':')){continue}
        if(Test-ProtectedVariableName $raw){$msg=($f.FullName+':'+$node.Name.Extent.StartLineNumber+': parameter uses protected $'+$raw);$astProblemSet[$msg]=$true}
    }
}
$astProblems=@($astProblemSet.Keys|Sort-Object)
$detail='';if($astProblems.Count -ne 0){$detail=$astProblems -join '; '}
Result ($astProblems.Count -eq 0) 'Constant/read-only PowerShell variable safety' $detail

# Exact package contracts that previously failed only after extraction on Windows.
$wb=Get-Content (Join-Path $root 'Validate-Workbench.ps1') -Raw
$safePrev="[regex]::Escape('Prop `$prev ''frame'' -1')"
$unsafePrev='[regex]::Escape("Prop $prev ''frame'' -1")'
Result ($wb.Contains($safePrev) -and -not $wb.Contains($unsafePrev)) 'Validator strict-mode literal safety'
$bd=Get-Content (Join-Path $root 'Build-Debug.bat') -Raw
$br=Get-Content (Join-Path $root 'Build-Release.bat') -Raw
Result ($bd.Contains('if not "%CTEST_RC%"=="0"') -and -not $bd.Contains('KNOWN_SEGFAULT') -and -not $bd.Contains('KNOWN TEST ISSUE') -and $br.Contains('if not "%CTEST_RC%"=="0"') -and -not $br.Contains('KNOWN_SEGFAULT') -and -not $br.Contains('KNOWN TEST ISSUE')) 'Fail-closed CTest build gate'

$versionText=Get-Content (Join-Path $root 'src\version.h') -Raw
$vm=[regex]::Match($versionText,'kNativeVersion\s*=\s*"([^"]+)"')
$current='';if($vm.Success){$current=$vm.Groups[1].Value}
Result ($current -eq '0.66.9.0-RC3.4.9') 'RC3.4.9 native build identity' $current
$launcher=Get-Content (Join-Path $root 'Start-ChaseHQ.ps1') -Raw
$web=Get-Content $webPath -Raw
Result ($launcher -match '\[switch\]\$LocalOnly' -and $launcher -match '\$effectiveLanAccess' -and $launcher -match 'Resolve-ChaseHQLanAddress' -and $launcher -match 'Ensure-ChaseHQLanAccess\.ps1' -and $launcher -match 'LAN/mobile host smoke: PASS' -and $launcher -match '\?token=\$resolvedAccessToken' -and $web -match '\[switch\]\$LanAccess' -and $web -match 'TokenMatches') 'Default LAN/mobile launcher plumbing'
Result ($web -match 'Get-ChqScriptParameterDefinitions' -and $web -match 'Resolve-ChqScriptParameters' -and $web.Contains("resolvedCommands='resolved-commands.txt'") -and $web.Contains("'script.run'")) 'RC3.2 script parameter/evidence plumbing'

$lanHelper=Join-Path $root 'scripts\Ensure-ChaseHQLanAccess.ps1'
$lanHelperText=if(Test-Path $lanHelper -PathType Leaf){Get-Content $lanHelper -Raw}else{''}
Result ((Test-Path $lanHelper -PathType Leaf) -and $lanHelperText -match 'Ensure-ExactUrlAcl' -and $lanHelperText -match 'Get-UrlAclState' -and $lanHelperText -match 'ExpectedUser' -and $lanHelperText -match 'Final proof MUST be performed back in the original non-elevated process' -and $lanHelperText.Contains('$frontendPrefixes') -and $lanHelperText -match 'http://127\.0\.0\.1' -and $lanHelperText -match 'http://localhost' -and $lanHelperText -match 'Test-FrontendPrefixPermission' -and $lanHelperText -match 'foreach\(\$prefix in \$frontendPrefixes\)' -and $lanHelperText -match 'New-NetFirewallRule' -and $lanHelperText -match '-Verb RunAs') 'LAN URL ACL/firewall prerequisite helper'
$rc32Release=Join-Path $root 'research\scripts\regression\regression-v06690-rc32.chqscript'
$rc33Release=Join-Path $root 'research\scripts\regression\regression-v06690-rc33.chqscript'
$rc34Release=Join-Path $root 'research\scripts\regression\regression-v06690-rc34.chqscript'
$rc341Release=Join-Path $root 'research\scripts\regression\regression-v06690-rc341.chqscript'
$rc342Release=Join-Path $root 'research\scripts\regression\regression-v06690-rc342.chqscript'
$rc343Release=Join-Path $root 'research\scripts\regression\regression-v06690-rc343.chqscript'

$rc344Release=Join-Path $root 'research\scripts\regression\regression-v06690-rc344.chqscript'
$rc345Release=Join-Path $root 'research\scripts\regression\regression-v06690-rc345.chqscript'
$rc346Release=Join-Path $root 'research\scripts\regression\regression-v06690-rc346.chqscript'
$rc347Release=Join-Path $root 'research\scripts\regression\regression-v06690-rc347.chqscript'
$rc348Release=Join-Path $root 'research\scripts\regression\regression-v06690-rc348.chqscript'
$rc349Release=Join-Path $root 'research\scripts\regression\regression-v06690-rc349.chqscript'
$fullRegression=Get-Content (Join-Path $root 'research\scripts\regression\full-regression.chqscript') -Raw
Result ((Test-Path $rc32Release -PathType Leaf) -and (Test-Path $rc33Release -PathType Leaf) -and (Test-Path $rc34Release -PathType Leaf) -and (Test-Path $rc341Release -PathType Leaf) -and (Test-Path $rc342Release -PathType Leaf) -and (Test-Path $rc343Release -PathType Leaf) -and (Test-Path $rc344Release -PathType Leaf) -and (Test-Path $rc345Release -PathType Leaf) -and (Test-Path $rc346Release -PathType Leaf) -and (Test-Path $rc347Release -PathType Leaf) -and (Test-Path $rc348Release -PathType Leaf) -and (Test-Path $rc349Release -PathType Leaf) -and $fullRegression -match 'regression-v06690-rc32\.chqscript' -and $fullRegression -match 'regression-v06690-rc33\.chqscript' -and $fullRegression -match 'regression-v06690-rc34\.chqscript' -and $fullRegression -match 'regression-v06690-rc341\.chqscript' -and $fullRegression -match 'regression-v06690-rc342\.chqscript' -and $fullRegression -match 'regression-v06690-rc343\.chqscript' -and $fullRegression -match 'regression-v06690-rc344\.chqscript' -and $fullRegression -match 'regression-v06690-rc345\.chqscript' -and $fullRegression -match 'regression-v06690-rc346\.chqscript' -and $fullRegression -match 'regression-v06690-rc347\.chqscript' -and $fullRegression -match 'regression-v06690-rc348\.chqscript' -and $fullRegression -match 'regression-v06690-rc349\.chqscript') 'RC3.4.9 whole-release regression wiring'
$releaseValidatorText=Get-Content (Join-Path $root 'Validate-Release.ps1') -Raw
$rc343PortableTest=Join-Path $root 'tests\workbench_rc343_tests.js'
Result ((Test-Path $rc343PortableTest -PathType Leaf) -and $releaseValidatorText -match 'Normalize-ReleaseContractText' -and $releaseValidatorText -match 'handoverContractGoodLf' -and $releaseValidatorText -match 'handoverContractGoodCrLf' -and $releaseValidatorText -match 'handoverContractGoodCr' -and $releaseValidatorText -match 'ChatGPT handover contract self-test') 'RC3.4.3 text-contract newline invariance'


$chqdevConfig=Join-Path $root 'chqdev.json'
$chqdevMain=Join-Path $root 'tools\chqdev\main.go'
$chqdevWin=Join-Path $root 'tools\chqdev\bin\chqdev-windows-amd64.exe'
$chqdevBat=Join-Path $root 'chqdev.bat'
$chqdevDoc=Join-Path $root 'docs\CHQDEV.md'
Result ((Test-Path $chqdevConfig -PathType Leaf) -and (Test-Path $chqdevMain -PathType Leaf) -and (Test-Path $chqdevWin -PathType Leaf) -and (Test-Path $chqdevBat -PathType Leaf) -and (Test-Path $chqdevDoc -PathType Leaf)) 'RC3.4.5 chqdev controller surface'
$chqdevTrustWindows=Get-Content (Join-Path $root 'tools\chqdev\trust_windows.go') -Raw
$chqdevLifecycle=Get-Content (Join-Path $root 'tools\chqdev\runtime_lifecycle.go') -Raw
Result ($chqdevTrustWindows -match 'Zone.Identifier' -and $chqdevLifecycle -match 'ExecutionPolicy' -and $chqdevLifecycle -match 'runtimeCheck') 'RC3.4.6+ chqdev Windows trust/runtime lifecycle surface'

$chqdevRunner=Get-Content (Join-Path $root 'tools\chqdev\script_runner.go') -Raw
$chqdevRegression=Get-Content (Join-Path $root 'tools\chqdev\regression.go') -Raw
$chqdevRuntimeTests=Get-Content (Join-Path $root 'tools\chqdev\runtime_test.go') -Raw
Result ($chqdevRunner -match 'defaults := scriptDefaultVars\(src\)' -and $chqdevRunner -match 'resolveQueries\(src, defaults\)' -and $chqdevRunner -match 'compileScriptLines\(strings\.Split\(src, "\\n"\), defaults\)') 'RC3.4.8 Go runner parameter-default propagation'
Result ($chqdevRuntimeTests -match 'TestGoScriptRunnerParameterDefaults' -and $chqdevRuntimeTests -match 'echo frames=4' -and $chqdevRuntimeTests -match 'control\.run-frames frames=4') 'RC3.4.8 parameter-default integration regression'
Result ($chqdevRegression -match 'go-script-console-v2' -and $chqdevRegression -match 'runPath' -and $chqdevRegression -match 'bundlePath') 'RC3.4.8 regression failure diagnostics'

$runtimeTests349=Get-Content (Join-Path $root 'tests\runtime_tests.cpp') -Raw
$chqdevBuild349=Get-Content (Join-Path $root 'tools\chqdev\build.go') -Raw
$chqdevCfg349=Get-Content (Join-Path $root 'chqdev.json') -Raw
Result (([regex]::Matches($runtimeTests349,'std::make_unique<chq::Runtime>\(').Count -eq 16) -and $runtimeTests349 -notmatch '(?m)^\s*chq::Runtime\s+[A-Za-z_]') 'RC3.4.9 Runtime test heap-allocation guard'
Result (([regex]::Matches($runtimeTests349,'tf\.close\(\);').Count -eq 6) -and $runtimeTests349 -match 'pf\.close\(\);' -and $runtimeTests349 -match 'cf\.close\(\);') 'RC3.4.9 Windows stream-lifetime guard'
Result ($chqdevBuild349 -notmatch 'known historical failure' -and $chqdevBuild349 -notmatch 'KnownCTestFailure' -and $chqdevCfg349 -notmatch 'knownCTestFailure') 'RC3.4.9 zero-tolerated-failure chqdev policy'

$researchLauncher=Get-Content (Join-Path $root 'Start-ChaseHQResearch.ps1') -Raw
$expectedArgLine='$argLine = (($nativeLaunchArgs | ForEach-Object { Quote-Arg $_ }) -join '' '')'
$forbiddenArgLine='$argLine = (($EmulatorArgs | ForEach-Object { Quote-Arg $_ }) -join '' '')'
Result ($researchLauncher.Contains('$nativeLaunchArgs = @(') -and $researchLauncher.Contains('$nativeLaunchArgs += @($EmulatorArgs)') -and $researchLauncher.Contains($expectedArgLine) -and -not $researchLauncher.Contains($forbiddenArgLine) -and $researchLauncher.Contains('Start-Process -FilePath $emu -ArgumentList $argLine')) 'Research launcher executed-argument contract'
$launchSmokeMarker='if ($ValidateLaunchOnly) { $nativeLaunchArgs += ''--help'' }'
Result ($researchLauncher.Contains('[switch]$ValidateLaunchOnly') -and $researchLauncher.Contains($launchSmokeMarker) -and $bd.Contains('-ValidateLaunchOnly') -and $bd.Contains('Running launcher process/argument smoke') -and $br.Contains('-ValidateLaunchOnly') -and $br.Contains('Running launcher process/argument smoke')) 'Debug/Release post-build launcher binding smoke gate'

if($script:fail.Count -ne 0 -or $script:blocked.Count -ne 0){
    Write-Host ''
    Write-Host ('WINDOWS PREFLIGHT: NOT CLEAN ('+$script:fail.Count+' FAIL, '+$script:blocked.Count+' BLOCKED)') -ForegroundColor Red
    foreach($item in $script:fail){Write-Host (' FAIL - '+$item) -ForegroundColor Red}
    foreach($item in $script:blocked){Write-Host (' BLOCKED - '+$item) -ForegroundColor Yellow}
    throw ('Windows preflight not clean ('+$script:fail.Count+' FAIL, '+$script:blocked.Count+' BLOCKED)')
}
Write-Host 'WINDOWS PREFLIGHT: PASS' -ForegroundColor Green
