[CmdletBinding()]
param()
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
$web=Join-Path $root 'Start-ChaseHQWeb.ps1'
$tokens=$null;$parseErrors=$null
$ast=[System.Management.Automation.Language.Parser]::ParseFile($web,[ref]$tokens,[ref]$parseErrors)
if($parseErrors){throw ('Start-ChaseHQWeb.ps1 parse failure: '+(@($parseErrors|ForEach-Object{$_.Message}) -join '; '))}
$node=@($ast.FindAll({param($n) $n -is [System.Management.Automation.Language.FunctionDefinitionAst]},$true)|Where-Object{$_.Name -eq 'Signed16'}|Select-Object -First 1)
if($node.Count -ne 1){throw 'Could not isolate Signed16 from Start-ChaseHQWeb.ps1'}
. ([scriptblock]::Create($node[0].Extent.Text))
$cases=@(
    @{raw=0x0000;expected=0},
    @{raw=0x7FFF;expected=32767},
    @{raw=0x8000;expected=-32768},
    @{raw=0xFEF7;expected=-265},
    @{raw=0xFFFF;expected=-1}
)
foreach($c in $cases){$actual=Signed16 ([uint32]$c.raw);if([int]$actual -ne [int]$c.expected){throw ('Signed16 failed raw=0x{0:X4}: expected {1}, got {2}' -f $c.raw,$c.expected,$actual)}}
$roadLeft=0x7470;$roadRight=0x8B90;$delta=Signed16 (($roadRight-$roadLeft)-band 0xFFFF)
if($delta -ne 5920){throw "Track road-width signed conversion failed: expected 5920, got $delta"}
$lateral=0x7EF7;$roadCentre=0x8000;$latErr=Signed16 (($lateral-$roadCentre)-band 0xFFFF)
if($latErr -ne -265){throw "Track lateral-error signed conversion failed: expected -265, got $latErr"}
$text=Get-Content $web -Raw
if($text -match '\[int16\]\s*\([^\r\n]*-band\s+0xFFFF'){throw 'Unsafe direct [int16] reinterpret cast over masked 16-bit track arithmetic remains'}
if($text -notmatch [regex]::Escape('$delta=Signed16 (($roadRight-$roadLeft)-band 0xFFFF)') -or $text -notmatch [regex]::Escape('$latErr=Signed16 (($lateral-$roadCentre)-band 0xFFFF)')){throw 'Track-state signed arithmetic is not routed through Signed16'}
Write-Host 'Workbench track signed-16 reinterpretation regression: PASS' -ForegroundColor Green
