[CmdletBinding()]
param()
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
$web=Join-Path $root 'Start-ChaseHQWeb.ps1'
$tokens=$null;$parseErrors=$null
$ast=[System.Management.Automation.Language.Parser]::ParseFile($web,[ref]$tokens,[ref]$parseErrors)
if($parseErrors){throw ('Start-ChaseHQWeb.ps1 parse failure: '+(@($parseErrors|ForEach-Object{$_.Message}) -join '; '))}
$needed=@('Prop','New-TrackSpriteCandidates','Assign-TrackObjectIds','Get-TrackObjectCandidates')
foreach($name in $needed){
    $node=@($ast.FindAll({param($n) $n -is [System.Management.Automation.Language.FunctionDefinitionAst]},$true)|Where-Object{$_.Name -eq $name}|Select-Object -First 1)
    if($node.Count -ne 1){throw "Could not isolate function $name from Start-ChaseHQWeb.ps1"}
    . ([scriptblock]::Create($node[0].Extent.Text))
}
$script:TrackObjectNextId=1
$script:TrackObjectPreviousGroups=@()
$orderedProbe=[ordered]@{VisibleBottom=123;Map=410};if([int](Prop $orderedProbe 'visibleBottom' -1)-ne 123 -or [int](Prop $orderedProbe 'map' -1)-ne 410){throw 'Prop() failed production OrderedDictionary/case-insensitive lookup'}
$sprites=@(
    [ordered]@{slot=10;map=410;palette=177;x=20;y=100;width=24;height=24;visiblePixels=220;visibleLeft=21;visibleTop=101;visibleRight=43;visibleBottom=123},
    [ordered]@{slot=11;map=433;palette=153;x=120;y=110;width=32;height=32;visiblePixels=330;visibleLeft=122;visibleTop=112;visibleRight=151;visibleBottom=141},
    [ordered]@{slot=12;map=326;palette=153;x=200;y=120;width=40;height=40;visiblePixels=440;visibleLeft=202;visibleTop=122;visibleRight=239;visibleBottom=159}
)
$first=@(Get-TrackObjectCandidates -sprites $sprites)
if($first.Count -ne 3){throw "Object-candidate helper collapsed candidate array: expected 3, got $($first.Count)"}
foreach($c in $first){
    foreach($field in @('memberCount','slots','maps','palettes','sourceSlot','sourceMap','sourcePalette','left','top','right','bottom','width','height','centreX','centreY','visiblePixels','trackId','trackingMethod','trackingConfidence')){
        if($null -eq $c.PSObject.Properties[$field]){throw "Object candidate is missing required field: $field"}
    }
    if([string]::IsNullOrWhiteSpace([string]$c.maps) -or [string]::IsNullOrWhiteSpace([string]$c.palettes)){throw 'Object candidate lost map/palette evidence'}
    if([int]$c.memberCount -ne 1){throw 'Conservative per-sprite candidate policy changed unexpectedly'}
}
$firstIds=@($first|ForEach-Object{[string]$_.trackId})
$moved=@(
    [ordered]@{slot=10;map=410;palette=177;x=22;y=102;width=24;height=24;visiblePixels=218;visibleLeft=23;visibleTop=103;visibleRight=45;visibleBottom=125},
    [ordered]@{slot=11;map=433;palette=153;x=122;y=112;width=32;height=32;visiblePixels=328;visibleLeft=124;visibleTop=114;visibleRight=153;visibleBottom=143},
    [ordered]@{slot=12;map=326;palette=153;x=202;y=122;width=40;height=40;visiblePixels=438;visibleLeft=204;visibleTop=124;visibleRight=241;visibleBottom=161}
)
$second=@(Get-TrackObjectCandidates -sprites $moved)
if($second.Count -ne 3){throw "Second object-candidate sample collapsed: expected 3, got $($second.Count)"}
$reused=@($second|Where-Object{$firstIds -contains [string]$_.trackId})
if($reused.Count -ne 3){throw "Temporal tracking did not preserve expected IDs: reused $($reused.Count)/3"}
if(@($second|Where-Object{$_.trackingMethod -ne 'MAP_PALETTE_NEAREST_SCREENSPACE'}).Count -ne 0){throw 'Expected map/palette nearest screen-space tracking on second mock sample'}
Write-Host 'Workbench object candidate OrderedDictionary/array/enrichment/tracking regression: PASS' -ForegroundColor Green
