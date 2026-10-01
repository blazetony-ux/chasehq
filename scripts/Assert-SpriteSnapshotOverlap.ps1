# Shared offline/Workbench assertion; no emulator or host is started here.
function Assert-SpriteSnapshotOverlap {
    param([Parameter(Mandatory=$true)][string]$Snapshot,
          [Parameter(Mandatory=$true)][int]$Front,
          [Parameter(Mandatory=$true)][int]$Back,
          [Parameter(Mandatory=$true)][string]$FrontSolo,
          [Parameter(Mandatory=$true)][string]$BackSolo)
    $ErrorActionPreference='Stop'
    Add-Type -AssemblyName System.Drawing
    if($Front -eq $Back){throw 'Front and back must be different sprites'}
    $meta=Get-Content (Join-Path $Snapshot 'sprites/sprites.json') -Raw|ConvertFrom-Json
    $frontRows=@($meta.sprites|Where-Object {[int]$_.slot -eq $Front})
    $backRows=@($meta.sprites|Where-Object {[int]$_.slot -eq $Back})
    if($frontRows.Count -ne 1 -or $backRows.Count -ne 1){throw 'Required sprite asset absent or ambiguous'}
    $frontAsset=$frontRows[0];$backAsset=$backRows[0]
    $bitmaps=@()
    try {
        $assets=@($frontAsset,$backAsset);$soloDirs=@($FrontSolo,$BackSolo);$masks=@()
        for($n=0;$n -lt 2;$n++) {
            $asset=$assets[$n]
            $soloMeta=Get-Content (Join-Path $soloDirs[$n] 'sprites/sprites.json') -Raw|ConvertFrom-Json
            if([int]$soloMeta.frame -ne [int]$meta.frame){throw 'Solo and aggregate frames differ'}
            $crop=[Drawing.Bitmap]::FromFile((Join-Path $Snapshot $asset.asset));$bitmaps+=,$crop
            $solo=[Drawing.Bitmap]::FromFile((Join-Path $soloDirs[$n] 'sources/sprites.png'));$bitmaps+=,$solo
            if($solo.Width -ne 320 -or $solo.Height -ne 240){throw 'Unexpected source dimensions'}
            if($crop.Width -ne [int]$asset.width -or $crop.Height -ne [int]$asset.height){throw 'Crop metadata dimensions disagree'}
            $mask=New-Object 'bool[]' 76800;$count=0
            for($y=0;$y -lt 240;$y++){for($x=0;$x -lt 320;$x++){
                $cx=$x-[int]$asset.x;$cy=$y-[int]$asset.y
                $inCrop=$cx -ge 0 -and $cy -ge 0 -and $cx -lt $crop.Width -and $cy -lt $crop.Height
                $c=if($inCrop){$crop.GetPixel($cx,$cy)}else{[Drawing.Color]::FromArgb(0)}
                $s=$solo.GetPixel($x,$y)
                if(($c.A -ne $s.A) -or ($c.A -ne 0 -and $c.ToArgb() -ne $s.ToArgb())){throw "Sprite $($asset.slot) export/live source mismatch at $x,$y"}
                if($c.A -ne 0){$mask[$y*320+$x]=$true;$count++}
            }}
            if($count -eq 0 -or $count -ne [int]$asset.pixels){throw 'Empty sprite or incorrect exported pixel count'}
            $masks+=,$mask
        }
        $rows=Import-Csv (Join-Path $Snapshot 'sprite-ownership.csv')
        if($rows.Count -ne 76800){throw 'Incomplete ownership map'}
        $overlap=0;$frontWins=0;$backVisible=0;$seen=New-Object 'bool[]' 76800
        foreach($r in $rows){
            $x=[int]$r.x;$y=[int]$r.y
            if($x -lt 0 -or $x -ge 320 -or $y -lt 0 -or $y -ge 240){throw 'Invalid ownership coordinate'}
            $i=$y*320+$x;if($seen[$i]){throw 'Duplicate ownership coordinate'};$seen[$i]=$true
            $owner=[int]$r.owner;$coverage=[int]$r.coverage
            if($masks[0][$i] -and $masks[1][$i]){
                $overlap++
                if($coverage -lt 2){throw "Overlap coverage missing at $x,$y"}
                if($owner -eq $Back){throw "Back sprite $Back overwrote front sprite $Front at $x,$y"}
                if($owner -eq $Front){$frontWins++}
            } elseif($masks[1][$i] -and $owner -eq $Back){$backVisible++}
        }
        if($overlap -eq 0 -or $frontWins -eq 0 -or $backVisible -eq 0){throw "Vacuous overlap assertion: overlap=$overlap frontWins=$frontWins backVisible=$backVisible"}
        return [ordered]@{ok=$true;frame=[int]$meta.frame;front=$Front;back=$Back;frontMap=[int]$frontAsset.map;backMap=[int]$backAsset.map;overlapPixels=$overlap;frontOwnedOverlap=$frontWins;backVisibleOutsideFront=$backVisible;exportCoordinatesExact=$true}
    } finally {foreach($b in $bitmaps){$b.Dispose()}}
}
