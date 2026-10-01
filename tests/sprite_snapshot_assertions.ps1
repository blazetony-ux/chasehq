param([string]$OutputRoot=(Join-Path $PSScriptRoot '../out/sprite-snapshot-tests'))
$ErrorActionPreference='Stop'
Set-StrictMode -Version 2
. (Join-Path $PSScriptRoot '../scripts/Assert-SpriteSnapshotOverlap.ps1')
Add-Type -AssemblyName System.Drawing
$dir=Join-Path $OutputRoot ([guid]::NewGuid().ToString('N'))
foreach($part in 'normal/sprites','front/sources','front/sprites','back/sources','back/sprites'){New-Item -ItemType Directory -Path (Join-Path $dir $part) -Force|Out-Null}
$metadata=[ordered]@{frame=2352;sprites=@(
    [ordered]@{slot=82;map=475;asset='sprites/front.png';x=10;y=20;width=2;height=1;pixels=2},
    [ordered]@{slot=81;map=575;asset='sprites/back.png';x=11;y=20;width=2;height=1;pixels=2})}
foreach($part in 'normal','front','back'){$metadata|ConvertTo-Json -Depth 5|Set-Content (Join-Path $dir "$part/sprites/sprites.json")}
foreach($part in 'front','back'){
    $rgb=if($part -eq 'front'){[Drawing.Color]::Red}else{[Drawing.Color]::Black}
    $x=if($part -eq 'front'){10}else{11}
    $crop=New-Object Drawing.Bitmap 2,1;$solo=New-Object Drawing.Bitmap 320,240
    try {for($i=0;$i -lt 2;$i++){$crop.SetPixel($i,0,$rgb);$solo.SetPixel($x+$i,20,$rgb)}
        $crop.Save((Join-Path $dir "normal/sprites/$part.png"));$solo.Save((Join-Path $dir "$part/sources/sprites.png"))
    }finally{$crop.Dispose();$solo.Dispose()}
}
$csv=Join-Path $dir 'normal/sprite-ownership.csv'
function Write-Ownership([int]$OverlapOwner=82,[int]$Coverage=2){
    $w=[IO.StreamWriter]::new($csv)
    try{$w.WriteLine('x,y,owner,coverage');for($y=0;$y -lt 240;$y++){for($x=0;$x -lt 320;$x++){
        $owner=65535;$c=0
        if($y -eq 20){if($x -eq 10){$owner=82;$c=1};if($x -eq 11){$owner=$OverlapOwner;$c=$Coverage};if($x -eq 12){$owner=81;$c=1}}
        $w.WriteLine("$x,$y,$owner,$c")
    }}}finally{$w.Dispose()}
}
function Verify {Assert-SpriteSnapshotOverlap -Snapshot (Join-Path $dir 'normal') -Front 82 -Back 81 -FrontSolo (Join-Path $dir 'front') -BackSolo (Join-Path $dir 'back')}
function Must-Reject([string]$Pattern){$message='';try{$null=Verify}catch{$message=$_.Exception.Message};if($message -notmatch $Pattern){throw "Expected rejection '$Pattern', got '$message'"}}
Write-Ownership
$result=Verify
if($result.overlapPixels -ne 1 -or $result.frontOwnedOverlap -ne 1 -or $result.backVisibleOutsideFront -ne 1){throw 'Incorrect positive assertion counts'}
Write-Ownership -OverlapOwner 81
Must-Reject 'overwrote'
Write-Ownership -Coverage 1
Must-Reject 'coverage missing'
Write-Ownership
$metadata.sprites[0].y=36
$metadata|ConvertTo-Json -Depth 5|Set-Content (Join-Path $dir 'normal/sprites/sprites.json')
Must-Reject 'export/live source mismatch'
'PASS snapshot assertions: positive ownership, opaque black, wrong owner, missing coverage, 16-line coordinate regression'
