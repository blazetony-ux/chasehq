[CmdletBinding()]
param([int]$Port=37600,[int]$HttpPort=37680,[string]$EvidenceRoot='.\evidence\session-evidence',[string]$SessionRoot='')
$ErrorActionPreference='Stop'
$root=$PSScriptRoot
$versionSource=Get-Content (Join-Path $root 'src\version.h') -Raw
$workbenchVersion=[regex]::Match($versionSource,'kNativeVersion\s*=\s*"([^"]+)"').Groups[1].Value
if(-not $workbenchVersion){throw 'Authoritative release version missing from src/version.h'}
. (Join-Path $root 'scripts/Assert-SpriteSnapshotOverlap.ps1')
$workspaceRoot=[IO.Path]::GetFullPath((Split-Path $root -Parent))
$evidenceSessionsRoot=Join-Path $root 'evidence\sessions'
$ctl=Join-Path $root 'out\build\x64-Debug\chqctl.exe'
if(-not(Test-Path $ctl)){$ctl=Join-Path $root 'out\build\x64-Debug\Debug\chqctl.exe'}
if(-not(Test-Path $ctl)){throw "Missing debugger client under out\build\x64-Debug (root or Debug subfolder). Build the project first."}
$publicHttpPort=$HttpPort
if($HttpPort -ge 65535){throw '-HttpPort must be 1..65534 because v0.66.9.0 reserves HttpPort+1 for the private backend.'}
$backendHttpPort=$HttpPort+1
if($backendHttpPort -eq $Port){throw 'HttpPort+1 conflicts with the native debug API Port; choose a different -HttpPort.'}
$runtimeDiagDir=Join-Path $root '.chq';New-Item -ItemType Directory -Path $runtimeDiagDir -Force|Out-Null
$runtimeStartupDiag=Join-Path $runtimeDiagDir 'web-startup.log'
function Startup-Log([string]$Message){try{Add-Content -Path $runtimeStartupDiag -Value ((Get-Date).ToString('o')+' '+$Message) -Encoding UTF8}catch{}}
Startup-Log ('BEGIN build='+$workbenchVersion+' public='+$publicHttpPort+' backend='+$backendHttpPort+' native='+$Port)
$listener=[Net.HttpListener]::new()
$backendPrefix="http://127.0.0.1:$backendHttpPort/"
$listener.Prefixes.Add($backendPrefix)
try{$listener.Start();Startup-Log ('BACKEND_READY '+$backendPrefix)}catch{Startup-Log ('BACKEND_FAIL '+$_.Exception.ToString());throw}
Write-Host "ChaseHQ Web backend: $backendPrefix (public port $publicHttpPort; debug API port $Port)"
$script:ChqRequestSeq=0
# v0.66.9.0 carries forward the shared Track View recorder. The browser UI and Research Script API
# operate on this same server-side state, so scripted surveys do not require the
# Track View tab to be open.
$script:TrackRecorderEnabled=$false
$script:TrackRecorderSamples=@()
$script:TrackRecorderMaxSamples=5000
$script:CourseSurveyActive=$false
$script:CourseSurveyRestore=$null
$script:TimelineResetCheckpoint=$null
$runtimeDiagDir=Join-Path $root '.chq';New-Item -ItemType Directory -Path $runtimeDiagDir -Force|Out-Null
$runtimeCommandDiag=Join-Path $runtimeDiagDir 'web-command-diagnostics.log'
$runtimeFrontendDiag=Join-Path $runtimeDiagDir 'web-frontend-errors.log'
$commandMutexName=('Local\ChaseHQNative-WebCmd-'+$Port)
$script:ChqCommandMutex=New-Object System.Threading.Mutex($false,$commandMutexName)
function Invoke-Chq([string[]]$cmd){
    $script:ChqRequestSeq++
    $req=$script:ChqRequestSeq
    if($null -eq $cmd -or $cmd.Count -eq 0 -or [string]::IsNullOrWhiteSpace([string]$cmd[0])){
        $msg=('['+(Get-Date).ToString('o')+"] request=$req REJECTED empty debugger command")
        Add-Content -Path $runtimeCommandDiag -Value $msg -Encoding UTF8
        throw "Debugger command rejected before transport: empty command (request $req)"
    }
    $locked=$false
    try{
        $locked=$script:ChqCommandMutex.WaitOne(30000)
        if(-not $locked){
            $line=('['+(Get-Date).ToString('o')+"] request=$req TIMEOUT waiting for command dispatcher command="+($cmd -join ' '))
            Add-Content -Path $runtimeCommandDiag -Value $line -Encoding UTF8
            throw "Timed out waiting for native command dispatcher (request $req)"
        }
        $result=(& $ctl --port $Port @cmd 2>&1 | Out-String).Trim()
        if($result -match 'ERR\s+empty command' -or [string]::IsNullOrWhiteSpace($result)){
            $caller=((Get-PSCallStack | Select-Object -Skip 1 -First 4 | ForEach-Object {$_.FunctionName}) -join ' <- ')
            $line=('['+(Get-Date).ToString('o')+"] request=$req caller=$caller command="+($cmd -join ' ')+" response="+$(if($result){$result}else{'<EMPTY>'})+" recovery=retry-once")
            Add-Content -Path $runtimeCommandDiag -Value $line -Encoding UTF8
            Start-Sleep -Milliseconds 15
            $retry=(& $ctl --port $Port @cmd 2>&1 | Out-String).Trim()
            if($retry -and $retry -notmatch 'ERR\s+empty command'){
                Add-Content -Path $runtimeCommandDiag -Value ('['+(Get-Date).ToString('o')+"] request=$req RECOVERED retry command="+($cmd -join ' ')) -Encoding UTF8
                $result=$retry
            } else {
                Add-Content -Path $runtimeCommandDiag -Value ('['+(Get-Date).ToString('o')+"] request=$req RETRY_FAILED response="+$(if($retry){$retry}else{'<EMPTY>'})) -Encoding UTF8
            }
        }
        return $result
    } finally {
        if($locked){try{$script:ChqCommandMutex.ReleaseMutex()|Out-Null}catch{}}
    }
}

$script:WebLogStarted = Get-Date
function Web-Stamp {
    $now=Get-Date
    $elapsed=$now-$script:WebLogStarted
    return ('[{0} +{1:00}:{2:00}:{3:00}.{4:000}]' -f $now.ToString('HH:mm:ss.fff'),[int]$elapsed.TotalHours,$elapsed.Minutes,$elapsed.Seconds,$elapsed.Milliseconds)
}
$runtimeListenerLog=Join-Path $runtimeDiagDir 'web-listener.log'
function Web-Log([string]$Message,[ConsoleColor]$Color=[ConsoleColor]::Gray){ $line=(Web-Stamp)+' '+$Message;Write-Host $line -ForegroundColor $Color;try{Add-Content -Path $runtimeListenerLog -Value $line -Encoding UTF8}catch{} }
function Web-RequestLog([string]$Method,[string]$Path,[long]$Ms){$line=(Web-Stamp)+(" [WEB] {0} {1} {2}ms" -f $Method,$Path,$Ms);try{Add-Content -Path $runtimeListenerLog -Value $line -Encoding UTF8}catch{};$routine=@('/api/status','/api/input/ports','/api/events','/api/sprites','/api/v1/frame.png');if($Ms -ge 250 -or $routine -notcontains $Path){Write-Host $line -ForegroundColor $(if($Ms -ge 250){[ConsoleColor]::DarkYellow}else{[ConsoleColor]::Gray})}}
function Send($ctx,[string]$body,[string]$type='text/plain; charset=utf-8',[int]$status=200){
    $b=[Text.Encoding]::UTF8.GetBytes($body)
    try {
        $ctx.Response.StatusCode=$status
        $ctx.Response.ContentType=$type
        $ctx.Response.ContentEncoding=[Text.Encoding]::UTF8
        $ctx.Response.ContentLength64=$b.Length
        $ctx.Response.OutputStream.Write($b,0,$b.Length)
    } catch [System.IO.IOException] {
        Web-Log "[WEB] client disconnected while sending $($ctx.Request.HttpMethod) $($ctx.Request.Url.AbsolutePath): $($_.Exception.Message)" DarkYellow
    } catch [System.Net.HttpListenerException] {
        Web-Log "[WEB] HTTP client disconnected while sending $($ctx.Request.HttpMethod) $($ctx.Request.Url.AbsolutePath): $($_.Exception.Message)" DarkYellow
    } catch [System.ObjectDisposedException] {
        Web-Log "[WEB] response already closed for $($ctx.Request.HttpMethod) $($ctx.Request.Url.AbsolutePath)" DarkYellow
    } finally {
        try { $ctx.Response.OutputStream.Close() } catch {}
        try { $ctx.Response.Close() } catch {}
    }
}
function SafeName([string]$s){ if($s -notmatch '^[A-Za-z0-9_.-]{1,64}$'){throw 'Invalid name'}; $s }
function SendJson($ctx,$obj,[int]$status=200){ Send $ctx ($obj|ConvertTo-Json -Depth 12 -Compress) 'application/json; charset=utf-8' $status }
function ReadJson($req){$sr=[IO.StreamReader]::new($req.InputStream,$req.ContentEncoding);try{$t=$sr.ReadToEnd()}finally{$sr.Dispose()};if([string]::IsNullOrWhiteSpace($t)){return [pscustomobject]@{}};$t|ConvertFrom-Json}
function Prop($o,[string]$n,$d=$null){if($null -eq $o){return $d};$p=$o.PSObject.Properties[$n];if($null -eq $p){return $d};$p.Value}
function Parse-Kv([string]$raw){$o=[ordered]@{raw=$raw};foreach($m in [regex]::Matches($raw,'(?m)(?:^|\s)([A-Za-z_][A-Za-z0-9_.-]*)=([^\s]+)')){$v=$m.Groups[2].Value;if($v -match '^-?\d+$'){$v=[long]$v};$o[$m.Groups[1].Value]=$v};[pscustomobject]$o}
function SendBytes($ctx,[byte[]]$bytes,[string]$name){$ctx.Response.StatusCode=200;$ctx.Response.ContentType='application/octet-stream';$ctx.Response.AddHeader('Content-Disposition',('attachment; filename="'+$name+'"'));$ctx.Response.ContentLength64=$bytes.Length;$ctx.Response.OutputStream.Write($bytes,0,$bytes.Length);$ctx.Response.Close()}
function SendBinary($ctx,[byte[]]$bytes,[string]$type){try{$ctx.Response.StatusCode=200;$ctx.Response.ContentType=$type;$ctx.Response.ContentLength64=$bytes.Length;$ctx.Response.OutputStream.Write($bytes,0,$bytes.Length)}finally{try{$ctx.Response.OutputStream.Close()}catch{};try{$ctx.Response.Close()}catch{}}}
function Hex-U32([string]$s){$s=([string]$s).Trim();if($s -match '^(?i)0x([0-9a-f]{1,8})$'){$s=$Matches[1]};if($s -notmatch '^[0-9A-Fa-f]{1,8}$'){throw "Invalid hexadecimal address: $s"};[Convert]::ToUInt32($s,16)}
function Count-U32([string]$s){$s=([string]$s).Trim();if($s -match '^(?i)0x([0-9a-f]{1,8})$'){return [Convert]::ToUInt32($Matches[1],16)};$n=0;if([uint32]::TryParse($s,[ref]$n)){return [uint32]$n};if($s -match '^[0-9A-Fa-f]{1,8}$'){return [Convert]::ToUInt32($s,16)};throw "Invalid length/count: $s"}
function Resolve-EvidenceOutput([string]$name,[string]$defaultName){
    if([string]::IsNullOrWhiteSpace($name)){$name=$defaultName}
    $rel=([string]$name).Replace('\','/').TrimStart('/')
    if($rel.StartsWith('evidence/',[StringComparison]::OrdinalIgnoreCase)){$rel=$rel.Substring(9)}
    if($rel -match '(^|/)\.\.(/|$)' -or $rel.IndexOfAny([char[]]':*?"<>|') -ge 0){throw 'Invalid evidence output path'}
    $full=[IO.Path]::GetFullPath((Join-Path $evidencePath $rel.Replace('/','\')))
    $base=[IO.Path]::GetFullPath($evidencePath).TrimEnd([IO.Path]::DirectorySeparatorChar)+[IO.Path]::DirectorySeparatorChar
    if(-not $full.StartsWith($base,[StringComparison]::OrdinalIgnoreCase)){throw 'Evidence output escapes evidence root'}
    New-Item -ItemType Directory -Path (Split-Path $full) -Force|Out-Null
    return $full
}
function Resolve-CheckpointFile([string]$name){
    if([string]::IsNullOrWhiteSpace($name)){throw 'Checkpoint file is required'}
    $leaf=[IO.Path]::GetFileName($name)
    if($leaf -ne $name){throw 'Checkpoint must be a packaged filename, not an arbitrary path'}
    if($leaf -notmatch '^[A-Za-z0-9_.-]+\.chqstate$'){throw 'Invalid checkpoint filename'}
    $full=Join-Path $checkpointRoot $leaf
    if(-not(Test-Path $full)){throw "Checkpoint not found: $leaf"}
    return $full
}
function Resolve-CheckpointSaveFile([string]$name){
    if([string]::IsNullOrWhiteSpace($name)){throw 'Checkpoint file is required'}
    $leaf=[IO.Path]::GetFileName($name)
    if($leaf -ne $name){throw 'Checkpoint must be a packaged filename, not an arbitrary path'}
    if(-not $leaf.EndsWith('.chqstate',[StringComparison]::OrdinalIgnoreCase)){$leaf += '.chqstate'}
    if($leaf -notmatch '^[A-Za-z0-9_.-]+\.chqstate$'){throw 'Invalid checkpoint filename'}
    return (Join-Path $checkpointRoot $leaf)
}
function Wait-ChqPaused([int]$minimumFrame,[int]$timeoutMs=900000,[int]$stallTimeoutMs=15000){
    $sw=[Diagnostics.Stopwatch]::StartNew();$last=$null;$lastFrame=[int64]-1;$lastProgressMs=[int64]0
    do{
        Start-Sleep -Milliseconds 20
        $st=Parse-Kv (Invoke-Chq @('status'));$last=$st
        $paused=([string]$st.paused -eq '1');$frame=[int64]$st.frame;$remaining=[int64](Prop $st 'step_remaining' 0)
        if($frame -gt $lastFrame){$lastFrame=$frame;$lastProgressMs=$sw.ElapsedMilliseconds}
        if($paused -and $frame -ge $minimumFrame){return $st}
        # An explicit pause/Stop clears step_remaining. Treat that as an out-of-band cancellation.
        if($paused -and $remaining -eq 0 -and $frame -lt $minimumFrame){throw "Run aborted at frame $frame before target $minimumFrame"}
        if(($sw.ElapsedMilliseconds-$lastProgressMs) -ge $stallTimeoutMs){throw "Stalled waiting for frame $minimumFrame (last_frame=$frame step_remaining=$remaining no_progress_ms=$($sw.ElapsedMilliseconds-$lastProgressMs))"}
    }while($sw.ElapsedMilliseconds -lt $timeoutMs)
    $lf=if($last){Prop $last 'frame' '?'}else{'?'};$lr=if($last){Prop $last 'step_remaining' '?'}else{'?'}
    throw "Timed out waiting for frame $minimumFrame (last_frame=$lf step_remaining=$lr elapsed_ms=$($sw.ElapsedMilliseconds))"
}
function Get-CheckpointCatalog(){
    $items=@()
    foreach($f in Get-ChildItem $checkpointRoot -Filter '*.chqstate' -File | Sort-Object Name){
        $metaPath=[IO.Path]::ChangeExtension($f.FullName,'.json');$meta=$null
        if(Test-Path $metaPath){try{$meta=Get-Content $metaPath -Raw|ConvertFrom-Json}catch{}}
        $items += [ordered]@{file=$f.Name;name=$(if($meta){[string](Prop $meta 'name' $f.BaseName)}else{$f.BaseName});title=$(if($meta){[string](Prop $meta 'name' $f.BaseName)}else{$f.BaseName});frame=$(if($meta){Prop $meta 'frame' $null}else{$null});role=$(if($meta){[string](Prop $meta 'role' '')}else{''});description=$(if($meta){[string](Prop $meta 'description' '')}else{''});recommendedUse=$(if($meta){[string](Prop $meta 'recommended_use' '')}else{''})}
    }
    return @($items)
}
function Get-SessionLogFiles(){
    $items=@()
    $runtimeWebOut=Join-Path (Join-Path $root '.chq') 'web-stdout.log';$runtimeWebErr=Join-Path (Join-Path $root '.chq') 'web-stderr.log'
    if(Test-Path $runtimeWebOut){$items+='@runtime/web-stdout.log'}
    if(Test-Path $runtimeWebErr){$items+='@runtime/web-stderr.log'}
    if(Test-Path $runtimeListenerLog){$items+='@runtime/web-listener.log'}
    if(Test-Path $runtimeCommandDiag){$items+='@runtime/web-command-diagnostics.log'}
    if(Test-Path $runtimeFrontendDiag){$items+='@runtime/web-frontend-errors.log'}
    if($sessionPath -and (Test-Path $sessionPath)){
        $files=Get-ChildItem $sessionPath -File -Recurse -ErrorAction SilentlyContinue | Where-Object {$_.Extension -in '.log','.txt','.csv'} | Sort-Object FullName
        $items += @($files|ForEach-Object{$_.FullName.Substring($sessionPath.Length).TrimStart([char]'\',[char]'/') -replace '\\','/'})
    }
    return @($items)
}
$evidencePath=if([IO.Path]::IsPathRooted($EvidenceRoot)){$EvidenceRoot}else{Join-Path $root $EvidenceRoot};New-Item -ItemType Directory -Path $evidencePath -Force|Out-Null
$sessionPath=if([string]::IsNullOrWhiteSpace($SessionRoot)){$null}elseif([IO.Path]::IsPathRooted($SessionRoot)){$SessionRoot}else{Join-Path $root $SessionRoot}
$checkpointRoot=(Resolve-Path (Join-Path $root 'checkpoints')).Path
$script:currentCheckpointFile=''
$scriptLibraryRoot=Join-Path $root 'research\scripts';New-Item -ItemType Directory -Path $scriptLibraryRoot -Force|Out-Null
$previewPath=Join-Path (Join-Path $root '.chq') 'web-preview.png';New-Item -ItemType Directory -Path (Split-Path $previewPath) -Force|Out-Null
$gameProfileRoot=Join-Path $root 'games\chasehq'
$profilesRoot=Join-Path $root 'games'
$activeProfileId='chasehq'

$script:RecentStructuredFrameSnapshots=@{}
$featureManifestPath=Join-Path $root 'research\feature-manifest.json'
$audioProfilePath=Join-Path $root 'research\audio\audio-capabilities.json'
$semanticObjectsPath=Join-Path $root 'research\knowledge\semantic-objects.json'
$documentationKnowledgePath=Join-Path $root 'research\knowledge\documentation.json'
$documentationHandbookPath=Join-Path $root 'research\knowledge\handbook.json'
$docsRoot=Join-Path $root 'docs'
$docsEvidenceRoot=Join-Path $docsRoot 'evidence'
$docsExportRoot=Join-Path $evidencePath 'documentation-exports';New-Item -ItemType Directory -Path $docsExportRoot -Force|Out-Null
function Get-DocumentationExportRoot(){ $r=Join-Path $evidencePath 'documentation-exports'; New-Item -ItemType Directory -Path $r -Force|Out-Null; return $r }
$chatgptHandoverPath=Join-Path $docsRoot 'CHATGPT_HANDOVER_PROMPT.md'
function Get-DocumentationKnowledge(){
    if(-not(Test-Path $documentationKnowledgePath)){throw 'Documentation knowledge source is missing'}
    return (Get-Content $documentationKnowledgePath -Raw|ConvertFrom-Json)
}
function Get-DocumentationHandbook(){
    if(-not(Test-Path $documentationHandbookPath)){throw 'Documentation handbook source is missing'}
    return (Get-Content $documentationHandbookPath -Raw -Encoding UTF8|ConvertFrom-Json)
}
function Get-DocumentationHandbookPage([string]$id){
    if([string]::IsNullOrWhiteSpace($id)){throw 'Handbook page id is required'}
    foreach($section in @((Get-DocumentationHandbook).sections)){foreach($page in @($section.pages)){if([string]$page.id -eq $id){return [ordered]@{section=[ordered]@{id=$section.id;title=$section.title};page=$page}}}}
    throw "Handbook page not found: $id"
}
function Get-DocumentationEntries(){return @((Get-DocumentationKnowledge).entries)}
function Get-DocumentationEntry([string]$id){
    if([string]::IsNullOrWhiteSpace($id)){throw 'Documentation id is required'}
    $entry=@(Get-DocumentationEntries|Where-Object{[string]$_.id -eq $id}|Select-Object -First 1)
    if(-not $entry){throw "Documentation entry not found: $id"}
    return $entry[0]
}
function Search-DocumentationEntries([string]$query='',[string]$category='',[string]$status=''){
    $q=([string]$query).Trim().ToLowerInvariant();$cat=([string]$category).Trim();$st=([string]$status).Trim()
    $items=@(Get-DocumentationEntries)
    if($cat){$items=@($items|Where-Object{[string]$_.category -eq $cat})}
    if($st){$items=@($items|Where-Object{[string]$_.status -eq $st})}
    if($q){$items=@($items|Where-Object{$article=if($_.article){$_.article|ConvertTo-Json -Depth 12 -Compress}else{''};$hay=([string]$_.title+' '+[string]$_.summary+' '+([string]::Join(' ',@($_.tags)))+' '+[string]$_.category+' '+[string]$_.id+' '+$article+' '+([string]::Join(' ',@($_.docs)))+' '+([string]::Join(' ',@($_.scripts)))+' '+([string]::Join(' ',@($_.apiActions))));$hay.ToLowerInvariant().Contains($q)})}
    return @($items|Sort-Object category,title)
}
function Get-DocumentationIndex(){
    $k=Get-DocumentationKnowledge;$entries=@($k.entries)
    return [ordered]@{schema='chq-docs-index-v1';build=$workbenchVersion;count=$entries.Count;categories=@($entries|ForEach-Object{$_.category}|Sort-Object -Unique);statuses=@($entries|ForEach-Object{$_.status}|Sort-Object -Unique);entries=@($entries|ForEach-Object{$article=if($_.article){$_.article|ConvertTo-Json -Depth 12 -Compress}else{''};$searchText=([string]$_.title+' '+[string]$_.summary+' '+([string]::Join(' ',@($_.tags)))+' '+[string]$_.category+' '+[string]$_.id+' '+$article+' '+([string]::Join(' ',@($_.docs)))+' '+([string]::Join(' ',@($_.scripts)))+' '+([string]::Join(' ',@($_.apiActions))));[ordered]@{id=$_.id;title=$_.title;kind=$_.kind;category=$_.category;status=$_.status;confidence=$_.confidence;introduced=$_.introduced;updated=$_.updated;summary=$_.summary;tags=@($_.tags);searchText=$searchText}})}
}
function Get-DocumentationRelated([string]$id){$e=Get-DocumentationEntry $id;$ids=@($e.related);return @(Get-DocumentationEntries|Where-Object{$ids -contains [string]$_.id}|Sort-Object title)}
function Resolve-DocumentationEvidence([string]$relative){
    if([string]::IsNullOrWhiteSpace($relative)){throw 'Evidence path is required'}
    $rel=$relative.Replace('/','\').TrimStart('\')
    if($rel.StartsWith('docs\evidence\',[StringComparison]::OrdinalIgnoreCase)){$rel=$rel.Substring(14)}
    if($rel -match '(^|\\)\.\.(\\|$)'){throw 'Evidence path escapes documentation evidence root'}
    $base=[IO.Path]::GetFullPath($docsEvidenceRoot).TrimEnd([IO.Path]::DirectorySeparatorChar)+[IO.Path]::DirectorySeparatorChar
    $full=[IO.Path]::GetFullPath((Join-Path $docsEvidenceRoot $rel))
    if(-not $full.StartsWith($base,[StringComparison]::OrdinalIgnoreCase)){throw 'Evidence path escapes documentation evidence root'}
    if(-not(Test-Path $full -PathType Leaf)){throw "Documentation evidence not found: $relative"}
    return $full
}
function Html-Escape([string]$text){return [Net.WebUtility]::HtmlEncode([string]$text)}
function Evidence-DataUri([string]$relative){
    $full=Resolve-DocumentationEvidence $relative;$ext=[IO.Path]::GetExtension($full).ToLowerInvariant();if($ext -notin '.png','.jpg','.jpeg','.webp'){return $null};$mime=if($ext -eq '.png'){'image/png'}elseif($ext -eq '.webp'){'image/webp'}else{'image/jpeg'};return ('data:'+ $mime +';base64,'+[Convert]::ToBase64String([IO.File]::ReadAllBytes($full)))
}
function New-DocumentationHtml([object[]]$entries,[string]$title='Chase H.Q. Native Documentation',[string]$evidenceMode='key'){
    $cards=New-Object Text.StringBuilder
    foreach($e in @($entries)){
        $null=$cards.Append('<section class="entry"><div class="meta"><span>'+$(Html-Escape $e.category)+'</span><span>'+$(Html-Escape $e.status)+'</span><span>updated '+$(Html-Escape $e.updated)+'</span></div><h2>'+$(Html-Escape $e.title)+'</h2><p>'+$(Html-Escape $e.summary)+'</p>')
        if(@($e.tags).Count){$null=$cards.Append('<p class="tags">'+((@($e.tags)|ForEach-Object{'<code>'+$(Html-Escape $_)+'</code>'}) -join ' ')+'</p>')}
        if($evidenceMode -ne 'none' -and @($e.evidence).Count){$null=$cards.Append('<div class="evidence">');$evs=@($e.evidence);if($evidenceMode -eq 'key'){$evs=@($evs|Select-Object -First 2)};foreach($ev in $evs){if([string]$ev.type -eq 'image'){try{$uri=Evidence-DataUri ([string]$ev.path);if($uri){$null=$cards.Append('<figure><img src="'+$uri+'"><figcaption>'+$(Html-Escape $ev.caption)+'</figcaption></figure>')}}catch{}}else{$null=$cards.Append('<div class="artifact"><b>'+$(Html-Escape $ev.type)+'</b> '+$(Html-Escape $ev.caption)+'</div>')}};$null=$cards.Append('</div>')}
        $docs=@($e.docs|Where-Object{ -not [string]::IsNullOrWhiteSpace([string]$_) });if($docs.Count){$null=$cards.Append('<p class="small"><b>Docs:</b> '+$(Html-Escape ([string]::Join(', ',@($e.docs))))+'</p>')}
        $null=$cards.Append('</section>')
    }
    return '<!doctype html><html><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>'+$(Html-Escape $title)+'</title><style>:root{color-scheme:dark}body{margin:0;background:#070a0e;color:#edf4fa;font-family:Segoe UI,system-ui,sans-serif}.hero{padding:30px;border-bottom:8px repeating-linear-gradient(90deg,#fff 0 24px,#111 24px 48px);background:linear-gradient(135deg,#0d1520,#131820)}main{max-width:1100px;margin:auto;padding:24px}.entry{background:#10161d;border:1px solid #2a3744;margin:0 0 16px;padding:18px}.entry h2{margin:8px 0}.meta{display:flex;gap:8px;flex-wrap:wrap}.meta span,code{background:#0b1924;border:1px solid #345064;padding:4px 7px;font-size:11px}.evidence{display:flex;gap:12px;flex-wrap:wrap;margin-top:12px}figure{margin:0;max-width:460px}img{max-width:100%;image-rendering:pixelated;border:1px solid #394959;background:#000}figcaption,.small{color:#aab9c7;font-size:12px}.artifact{border-left:3px solid #2698ff;padding:8px;background:#0d1319}@media print{body{background:#fff;color:#111}.entry{background:#fff;border-color:#aaa;break-inside:avoid}.meta span,code{background:#eee;color:#111}.small,figcaption{color:#444}}</style></head><body><div class="hero"><h1>'+$(Html-Escape $title)+'</h1><p>Generated from ChaseHQ-Native '+$workbenchVersion+' machine-readable knowledge.</p></div><main>'+$cards.ToString()+'</main></body></html>'
}
function New-HandbookBlockHtml($block){
    $type=[string](Prop $block 'type' '')
    if($type -eq 'p'){return '<p>'+$(Html-Escape ([string](Prop $block 'text' '')))+'</p>'}
    if($type -eq 'callout'){return '<div class="callout"><b>'+$(Html-Escape ([string](Prop $block 'title' 'Note')))+'</b><div>'+$(Html-Escape ([string](Prop $block 'text' '')))+'</div></div>'}
    if($type -eq 'list' -or $type -eq 'steps'){$tag=if($type -eq 'steps'){'ol'}else{'ul'};$items=((@($block.items)|ForEach-Object{'<li>'+$(Html-Escape ([string]$_))+'</li>'}) -join '');return '<'+$tag+'>'+$items+'</'+$tag+'>'}
    if($type -eq 'code'){$title=if(Prop $block 'title' $null){'<h4>'+$(Html-Escape ([string]$block.title))+'</h4>'}else{''};return $title+'<pre>'+$(Html-Escape ([string](Prop $block 'text' '')))+'</pre>'}
    if($type -eq 'table'){$h='<table><thead><tr>'+((@($block.headers)|ForEach-Object{'<th>'+$(Html-Escape ([string]$_))+'</th>'}) -join '')+'</tr></thead><tbody>';foreach($row in @($block.rows)){$h+='<tr>'+((@($row)|ForEach-Object{'<td>'+$(Html-Escape ([string]$_))+'</td>'}) -join '')+'</tr>'};return $h+'</tbody></table>'}
    return ''
}
function New-KnowledgeArticleHtml($article){
    if($null -eq $article){return ''}
    $b=[Text.StringBuilder]::new()
    if($article.discovery){$null=$b.Append('<h5>What we discovered</h5><p>'+$(Html-Escape $article.discovery)+'</p>')}
    if($article.significance){$null=$b.Append('<h5>Why it matters</h5><p>'+$(Html-Escape $article.significance)+'</p>')}
    if(@($article.confirmedValues).Count){
        $null=$b.Append('<h5>Confirmed values</h5><table><tr><th>Item</th><th>Confirmed value</th></tr>')
        foreach($v in @($article.confirmedValues)){$null=$b.Append('<tr><td>'+$(Html-Escape $v.name)+'</td><td><code>'+$(Html-Escape $v.value)+'</code></td></tr>')}
        $null=$b.Append('</table>')
    }
    if(@($article.method).Count){
        $null=$b.Append('<h5>How it was proven</h5><ol>')
        foreach($v in @($article.method)){$null=$b.Append('<li>'+$(Html-Escape $v)+'</li>')}
        $null=$b.Append('</ol>')
    }
    if(@($article.reproduction).Count){
        $null=$b.Append('<h5>How to reproduce</h5><ol>')
        foreach($v in @($article.reproduction)){$null=$b.Append('<li>'+$(Html-Escape $v)+'</li>')}
        $null=$b.Append('</ol>')
    }
    if(@($article.openQuestions).Count){
        $null=$b.Append('<h5>Open questions / next research</h5><ul>')
        foreach($v in @($article.openQuestions)){$null=$b.Append('<li>'+$(Html-Escape $v)+'</li>')}
        $null=$b.Append('</ul>')
    }
    return $b.ToString()
}
function New-FullHandbookHtml([string]$evidenceMode='key'){
    $hb=Get-DocumentationHandbook;$apiActions=$scriptActionSchemas;$scriptSchema=Get-ScriptLanguageSchema;$knowledge=@(Get-DocumentationEntries)
    $toc=New-Object Text.StringBuilder;$body=New-Object Text.StringBuilder
    foreach($section in @($hb.sections)){
        $null=$toc.Append('<a href="#sec-'+$(Html-Escape $section.id)+'">'+$(Html-Escape $section.title)+'</a>')
        $null=$body.Append('<section class="chapter" id="sec-'+$(Html-Escape $section.id)+'"><h2>'+$(Html-Escape $section.title)+'</h2>')
        foreach($page in @($section.pages)){
            $null=$body.Append('<article><h3>'+$(Html-Escape $page.title)+'</h3><p class="lede">'+$(Html-Escape $page.summary)+'</p>')
            foreach($block in @($page.blocks)){$null=$body.Append((New-HandbookBlockHtml $block))}
            if([string]$page.generated -eq 'api-reference'){
                $null=$body.Append('<h4>Complete generated action reference</h4>')
                foreach($name in @($apiActions.Keys|Sort-Object)){$a=$apiActions[$name];$null=$body.Append('<div class="ref"><h4><code>'+$(Html-Escape $name)+'</code></h4><p>'+$(Html-Escape $a.description)+'</p>');if(@($a.params).Count){$null=$body.Append('<table><tr><th>Parameter</th></tr>'+((@($a.params)|ForEach-Object{'<tr><td><code>'+$(Html-Escape $_)+'</code></td></tr>'}) -join '')+'</table>')};$null=$body.Append('<pre>'+$(Html-Escape $a.example)+'</pre></div>')}
            }elseif([string]$page.generated -eq 'script-reference'){
                $null=$body.Append('<table><tr><th>Syntax</th><th>Purpose</th></tr>');foreach($st in @($scriptSchema.statements)){$null=$body.Append('<tr><td><code>'+$(Html-Escape $st.syntax)+'</code></td><td>'+$(Html-Escape $st.description)+'</td></tr>')};$null=$body.Append('</table><h4>Query functions</h4><p>'+((@($scriptSchema.queryFunctions)|ForEach-Object{'<code>'+$(Html-Escape $_)+'</code>'}) -join ' ')+'</p>')
            }elseif([string]$page.generated -eq 'knowledge-browser'){
                foreach($e in @($knowledge|Where-Object{$_.kind -ne 'glossary'})){
                    $null=$body.Append('<div class="ref"><h4>'+$(Html-Escape $e.title)+'</h4><div class="meta">'+$(Html-Escape $e.category)+' | '+$(Html-Escape $e.status)+' | updated '+$(Html-Escape $e.updated)+'</div><p>'+$(Html-Escape $e.summary)+'</p>')
                    $null=$body.Append((New-KnowledgeArticleHtml $e.article))
                    $apiActions=@($e.apiActions|Where-Object{ -not [string]::IsNullOrWhiteSpace([string]$_) });if($apiActions.Count){$null=$body.Append('<h5>Relevant API actions</h5><p>'+(($apiActions|ForEach-Object{'<code>'+$(Html-Escape $_)+'</code>'}) -join ' ')+'</p>')}
                    $scripts=@($e.scripts|Where-Object{ -not [string]::IsNullOrWhiteSpace([string]$_) });if($scripts.Count){$null=$body.Append('<h5>Scripts</h5><ul>'+(($scripts|ForEach-Object{'<li><code>'+$(Html-Escape $_)+'</code></li>'}) -join '')+'</ul>')}
                    $checkpoints=@($e.checkpoints|Where-Object{ -not [string]::IsNullOrWhiteSpace([string]$_) });if($checkpoints.Count){$null=$body.Append('<h5>Checkpoints</h5><ul>'+(($checkpoints|ForEach-Object{'<li><code>'+$(Html-Escape $_)+'</code></li>'}) -join '')+'</ul>')}
                    $docs=@($e.docs|Where-Object{ -not [string]::IsNullOrWhiteSpace([string]$_) });if($docs.Count){$null=$body.Append('<h5>Source documentation</h5><ul>'+(($docs|ForEach-Object{'<li><code>'+$(Html-Escape $_)+'</code></li>'}) -join '')+'</ul>')}
                    if($evidenceMode -ne 'none' -and @($e.evidence).Count){
                        $evs=@($e.evidence)
                        if($evidenceMode -eq 'key'){$evs=@($evs|Select-Object -First 2)}
                        foreach($ev in $evs){
                            if([string]$ev.type -eq 'image'){
                                try{$uri=Evidence-DataUri ([string]$ev.path);if($uri){$null=$body.Append('<figure><img src="'+$uri+'"><figcaption>'+$(Html-Escape $ev.caption)+'</figcaption></figure>')}}catch{}
                            }else{
                                $null=$body.Append('<p class="meta">'+$(Html-Escape $ev.type)+': '+$(Html-Escape $ev.caption)+' | '+$(Html-Escape $ev.path)+'</p>')
                            }
                        }
                    }
                    $null=$body.Append('</div>')
                }
            }elseif([string]$page.generated -eq 'glossary'){
                $null=$body.Append('<dl class="glossary">');foreach($e in @($knowledge|Where-Object{$_.kind -eq 'glossary'}|Sort-Object title)){$null=$body.Append('<dt>'+$(Html-Escape $e.title)+'</dt><dd>'+$(Html-Escape $e.summary)+'</dd>')};$null=$body.Append('</dl>')
            }
            $null=$body.Append('</article>')
        }
        $null=$body.Append('</section>')
    }
    $css=':root{color-scheme:dark}*{box-sizing:border-box}body{margin:0;background:#080b0f;color:#edf4fa;font-family:Segoe UI,system-ui,sans-serif;line-height:1.55}.hero{padding:34px;border-bottom:8px repeating-linear-gradient(90deg,#fff 0 24px,#111 24px 48px);background:linear-gradient(135deg,#0d1520,#171b22)}.hero h1{font:italic 46px Impact,Haettenschweiler,sans-serif;margin:0}.layout{max-width:1500px;margin:auto;display:grid;grid-template-columns:260px 1fr;gap:18px;padding:22px}nav{position:sticky;top:12px;align-self:start;border:1px solid #2a3744;background:#0f151c;padding:10px}nav a{display:block;color:#a9c8e6;text-decoration:none;padding:6px 8px}.chapter{border:1px solid #2a3744;background:#0f151c;margin-bottom:16px;padding:18px}.chapter>h2{font-size:26px;border-bottom:1px solid #2a3744;padding-bottom:8px}article{margin:18px 0 28px}article h3{font-size:20px}.lede,.meta,figcaption{color:#aab9c7}.ref{border-left:3px solid #2698ff;background:#0c1218;padding:10px 13px;margin:10px 0}pre,code{font-family:ui-monospace,Consolas,monospace}pre{background:#080e14;border:1px solid #293846;padding:11px;white-space:pre-wrap;overflow:auto}code{background:#091018;border:1px solid #223341;padding:1px 4px}table{width:100%;border-collapse:collapse;margin:8px 0 14px}th,td{border:1px solid #2b3945;padding:7px;text-align:left;vertical-align:top}th{background:#17212a}.callout{border-left:4px solid #ffc43d;background:#111821;padding:10px 12px;margin:10px 0}.glossary{display:grid;grid-template-columns:180px 1fr;gap:1px;background:#273440}.glossary dt,.glossary dd{margin:0;background:#0e151c;padding:8px 10px}.glossary dt{font-weight:800}figure{max-width:620px;margin:10px 0}img{max-width:100%;image-rendering:pixelated;border:1px solid #394959}@media(max-width:850px){.layout{grid-template-columns:1fr}nav{position:relative}}@media print{body{background:white;color:#111}.hero,.chapter,.ref,.callout,nav{background:white}.layout{display:block}nav{display:none}.chapter{border:0;page-break-before:auto}pre,code{background:#f4f4f4;color:#111}.lede,.meta,figcaption{color:#444}}'
    return '<!doctype html><html><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>'+$(Html-Escape $hb.title)+'</title><style>'+$css+'</style></head><body><header class="hero"><h1>'+$(Html-Escape $hb.title)+'</h1><p>ChaseHQ-Native v'+$workbenchVersion+' | generated from the same handbook, API/schema and project-knowledge sources used by the Workbench.</p></header><div class="layout"><nav><b>Contents</b>'+$toc.ToString()+'</nav><main>'+$body.ToString()+'</main></div></body></html>'
}

function Find-EdgeExecutable(){
    $candidates=@((Join-Path ${env:ProgramFiles(x86)} 'Microsoft\Edge\Application\msedge.exe'),(Join-Path $env:ProgramFiles 'Microsoft\Edge\Application\msedge.exe'),(Join-Path $env:LOCALAPPDATA 'Microsoft\Edge\Application\msedge.exe'))
    foreach($x in $candidates){if($x -and (Test-Path $x -PathType Leaf)){return $x}}
    return $null
}
function Export-Documentation([string]$format='html',[string]$id='',[string]$query='',[string]$evidence='key',[string]$output=''){
    $format=$format.ToLowerInvariant();if($format -notin 'html','pdf'){throw 'format must be html or pdf'}
    if($evidence -notin 'none','key','full'){throw 'evidence must be none, key or full'}
    $items=if($id){@((Get-DocumentationEntry $id))}elseif($query){@(Search-DocumentationEntries $query)}else{@()}
    if(($id -or $query) -and $items.Count -eq 0){throw 'No documentation entries matched export'}
    $stamp=(Get-Date).ToString('yyyyMMdd-HHmmss');$base=if($output){[IO.Path]::GetFileNameWithoutExtension((SafeName ([IO.Path]::GetFileName($output))))}elseif($id){'docs-'+(SafeName $id)}else{'chasehq-user-research-handbook-'+$stamp}
    $exportRoot=Get-DocumentationExportRoot;$htmlPath=Join-Path $exportRoot ($base+'.html');if($id -or $query){New-DocumentationHtml $items 'Chase H.Q. Native - Documentation / Knowledge' $evidence|Set-Content $htmlPath -Encoding UTF8}else{New-FullHandbookHtml $evidence|Set-Content $htmlPath -Encoding UTF8}
    if($format -eq 'html'){return [ordered]@{ok=$true;format='html';path=$htmlPath;name=[IO.Path]::GetFileName($htmlPath);download='/api/v1/docs/export/'+[IO.Path]::GetFileName($htmlPath)+'/download';count=$(if($id -or $query){$items.Count}else{@((Get-DocumentationHandbook).sections).Count})}}
    $edge=Find-EdgeExecutable;if(-not $edge){throw 'Microsoft Edge was not found; PDF export requires Edge headless printing on Windows. HTML export remains available.'}
    $pdfPath=Join-Path $exportRoot ($base+'.pdf');$uri='file:///'+($htmlPath.Replace('\','/'));$args=@('--headless','--disable-gpu','--no-pdf-header-footer',('--print-to-pdf='+$pdfPath),$uri);$proc=Start-Process -FilePath $edge -ArgumentList $args -PassThru -Wait -WindowStyle Hidden;if($proc.ExitCode -ne 0 -or -not(Test-Path $pdfPath)){throw "Edge PDF export failed with exit code $($proc.ExitCode)"}
    return [ordered]@{ok=$true;format='pdf';path=$pdfPath;name=[IO.Path]::GetFileName($pdfPath);download='/api/v1/docs/export/'+[IO.Path]::GetFileName($pdfPath)+'/download';count=$(if($id -or $query){$items.Count}else{@((Get-DocumentationHandbook).sections).Count})}
}
$script:activeExperiment=$null
function New-DiagnosticBundle($Payload){
    $stamp=(Get-Date).ToString('yyyyMMdd-HHmmss-fff')
    $diagRoot=Join-Path $evidencePath 'diagnostics';New-Item -ItemType Directory -Path $diagRoot -Force|Out-Null
    $tmp=Join-Path $runtimeDiagDir ('diag-'+$stamp);New-Item -ItemType Directory -Path $tmp -Force|Out-Null
    try{
        $fault=[ordered]@{schema='chq-diagnostic-bundle-v1';created=(Get-Date).ToString('o');build=$workbenchVersion;phase=[string](Prop $Payload 'phase' '');context=[string](Prop $Payload 'context' '');message=[string](Prop $Payload 'message' '');scriptPath=[string](Prop $Payload 'scriptPath' '');session=$sessionPath}
        $fault|ConvertTo-Json -Depth 8|Set-Content (Join-Path $tmp 'fault.json') -Encoding UTF8
        [string](Prop $Payload 'script' '')|Set-Content (Join-Path $tmp 'script.chqscript') -Encoding UTF8
        [string](Prop $Payload 'output' '')|Set-Content (Join-Path $tmp 'script-output.txt') -Encoding UTF8
        try{(Invoke-Chq @('status'))|Set-Content (Join-Path $tmp 'native-status.txt') -Encoding UTF8}catch{($_.Exception.ToString())|Set-Content (Join-Path $tmp 'native-status-error.txt') -Encoding UTF8}
        try{$shot=Join-Path $tmp 'frame.png';$null=Invoke-Chq @('screenshot',$shot)}catch{}
        foreach($f in @('web-startup.log','web-listener.log','web-stdout.log','web-stderr.log','web-command-diagnostics.log','web-frontend-errors.log','active-session.json','startup-session.json','frontend-ready.json')){$src=Join-Path $runtimeDiagDir $f;if(Test-Path $src){try{Copy-Item $src (Join-Path $tmp $f) -Force}catch{}}}
        if($sessionPath -and (Test-Path $sessionPath)){foreach($f in @('bus.log','prom_mixer.log','road_probe.log','road_ram_layout.log','run_phase.txt')){$src=Join-Path $sessionPath $f;if(Test-Path $src){try{Copy-Item $src (Join-Path $tmp $f) -Force}catch{}}}}
        $zip=Join-Path $diagRoot ('diagnostic-'+$stamp+'.zip');Compress-Archive -Path (Join-Path $tmp '*') -DestinationPath $zip -Force
        return [ordered]@{ok=$true;zip=$zip;name=[IO.Path]::GetFileName($zip)}
    }finally{Remove-Item $tmp -Recurse -Force -ErrorAction SilentlyContinue}
}
$registryPath=Join-Path $gameProfileRoot 'gameplay-registry.json'
$nextStepsPath=Join-Path $gameProfileRoot 'next-steps.json'
$imageRegionsPath=Join-Path $gameProfileRoot 'image-regions.json'
function Get-GameProfile([string]$id='chasehq'){
    if([string]::IsNullOrWhiteSpace($id)){$id='chasehq'}
    if($id -notmatch '^[A-Za-z0-9_.-]{1,64}$'){throw 'Invalid game profile id'}
    $file=Join-Path (Join-Path $profilesRoot $id) 'profile.json'
    if(-not(Test-Path $file)){throw "Game profile not found: $id"}
    $o=Get-Content $file -Raw|ConvertFrom-Json
    [ordered]@{profile=$o;active=($id -eq $activeProfileId);activationSupported=($id -eq 'chasehq');descriptor=$file}
}
function Get-GameProfiles{
    $items=@()
    foreach($d in Get-ChildItem $profilesRoot -Directory -ErrorAction SilentlyContinue|Sort-Object Name){
        $file=Join-Path $d.FullName 'profile.json';if(-not(Test-Path $file)){continue}
        try{$o=Get-Content $file -Raw|ConvertFrom-Json;$items += [ordered]@{id=[string](Prop $o 'id' $d.Name);displayName=[string](Prop $o 'displayName' $d.Name);hardware=[string](Prop $o 'hardware' '');role=[string](Prop $o 'role' '');status=[string](Prop $o 'status' '');active=([string](Prop $o 'id' $d.Name) -eq $activeProfileId);activationSupported=([string](Prop $o 'id' $d.Name) -eq 'chasehq')}}catch{}
    }
    return @($items)
}
$proxySource=@"
using System;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Web.Script.Serialization;

public sealed class ChaseHQFrontProxy : IDisposable {
    readonly HttpListener listener = new HttpListener();
    readonly HttpClient client = new HttpClient();
    readonly string backend;
    readonly string scriptRoot;
    readonly string nextSteps;
    readonly string imageRegions;
    readonly string runtimeRoot;
    readonly string sessionRoot;
    readonly JavaScriptSerializer json = new JavaScriptSerializer();
    readonly CancellationTokenSource cts = new CancellationTokenSource();
    readonly DateTime startedUtc = DateTime.UtcNow;
    Task loop;
    public ChaseHQFrontProxy(int publicPort,int backendPort,string scriptRoot,string nextSteps,string imageRegions,string runtimeRoot,string sessionRoot) {
        backend = "http://127.0.0.1:"+backendPort+"/";
        this.scriptRoot=Path.GetFullPath(scriptRoot);
        this.nextSteps=nextSteps; this.imageRegions=imageRegions; this.runtimeRoot=runtimeRoot; this.sessionRoot=sessionRoot ?? "";
        client.Timeout=Timeout.InfiniteTimeSpan;
        listener.Prefixes.Add("http://127.0.0.1:"+publicPort+"/");
        listener.Prefixes.Add("http://localhost:"+publicPort+"/");
    }
    public void Start(){ listener.Start(); loop=Task.Run((Func<Task>)AcceptLoop); }
    async Task AcceptLoop(){
        while(listener.IsListening && !cts.IsCancellationRequested){
            HttpListenerContext ctx=null;
            try { ctx=await listener.GetContextAsync().ConfigureAwait(false); }
            catch { if(!listener.IsListening) break; continue; }
            var captured=ctx;
            Task backgroundTask = Task.Factory.StartNew(delegate { Handle(captured).GetAwaiter().GetResult(); }, CancellationToken.None, TaskCreationOptions.DenyChildAttach, TaskScheduler.Default);
        }
    }
    static async Task<byte[]> ReadBody(HttpListenerRequest r){ using(var ms=new MemoryStream()){ await r.InputStream.CopyToAsync(ms).ConfigureAwait(false); return ms.ToArray(); } }
    static async Task Reply(HttpListenerContext c,int status,string type,byte[] data){
        try { c.Response.StatusCode=status; c.Response.ContentType=type; c.Response.ContentLength64=data.Length; await c.Response.OutputStream.WriteAsync(data,0,data.Length).ConfigureAwait(false); }
        catch {} finally { try{c.Response.OutputStream.Close();}catch{} try{c.Response.Close();}catch{} }
    }
    static byte[] Utf8(string s) { return Encoding.UTF8.GetBytes(s ?? ""); }
    async Task JsonReply(HttpListenerContext c,object o,int status=200){ await Reply(c,status,"application/json; charset=utf-8",Utf8(json.Serialize(o))).ConfigureAwait(false); }
    string SafeScriptPath(string rel,bool save=false){
        if(string.IsNullOrWhiteSpace(rel)) throw new Exception("Script path is required");
        rel=rel.Replace('\\','/').TrimStart('/');
        if(rel.Contains("../") || rel.StartsWith("..") || rel.IndexOfAny(new[]{':','*','?','\"','<','>','|'})>=0) throw new Exception("Invalid script library path");
        if(!rel.EndsWith(".chqscript",StringComparison.OrdinalIgnoreCase)) rel += ".chqscript";
        var full=Path.GetFullPath(Path.Combine(scriptRoot,rel.Replace('/',Path.DirectorySeparatorChar)));
        var prefix=scriptRoot.TrimEnd(Path.DirectorySeparatorChar)+Path.DirectorySeparatorChar;
        if(!full.StartsWith(prefix,StringComparison.OrdinalIgnoreCase)) throw new Exception("Script path escapes library root");
        if(!save && !File.Exists(full)) throw new Exception("Script not found: "+rel);
        return full;
    }
    object ListScripts(){
        var list=new List<Dictionary<string,object>>();
        if(!Directory.Exists(scriptRoot)) return new Dictionary<string,object>{{"scripts",list}};
        foreach(var f in Directory.GetFiles(scriptRoot,"*.chqscript",SearchOption.AllDirectories).OrderBy(x=>x)){
            var rel=f.Substring(scriptRoot.Length).TrimStart('\\','/').Replace('\\','/');
            string name=Path.GetFileNameWithoutExtension(f),purpose="",checkpoint="",category="",status="",requires="";
            foreach(var line in File.ReadLines(f).Take(16)){
                var t=line.Trim(); if(!t.StartsWith("#")) continue; t=t.Substring(1).Trim();
                int k=t.IndexOf(':'); if(k<0) continue; var key=t.Substring(0,k).Trim().ToLowerInvariant(); var val=t.Substring(k+1).Trim();
                if(key=="name") name=val; else if(key=="purpose") purpose=val; else if(key=="checkpoint") checkpoint=val; else if(key=="category") category=val; else if(key=="status") status=val; else if(key=="requires") requires=val;
            }
            list.Add(new Dictionary<string,object>{{"path",rel},{"name",name},{"purpose",purpose},{"checkpoint",checkpoint},{"category",category},{"status",status},{"requires",requires},{"modified",File.GetLastWriteTime(f).ToString("o")}});
        }
        return new Dictionary<string,object>{{"scripts",list},{"servedBy","concurrent-front"}};
    }
    static string TailFileShared(string full,int lines){
        if(!File.Exists(full)) return "";
        var all=new List<string>();
        using(var fs=new FileStream(full,FileMode.Open,FileAccess.Read,FileShare.ReadWrite|FileShare.Delete))
        using(var sr=new StreamReader(fs,Encoding.UTF8,true)){ string line; while((line=sr.ReadLine())!=null) all.Add(line); }
        return string.Join("\n",all.Skip(Math.Max(0,all.Count-lines)));
    }
    async Task HandleLocal(HttpListenerContext c,string path){
        try {
            if(path=="/api/health") { var up=(long)Math.Max(0,(DateTime.UtcNow-startedUtc).TotalSeconds); var readyFile=Path.Combine(runtimeRoot,"frontend-ready.json"); var scriptCount=Directory.Exists(scriptRoot)?Directory.GetFiles(scriptRoot,"*.chqscript",SearchOption.AllDirectories).Length:0; await JsonReply(c,new Dictionary<string,object>{{"ok",true},{"product","ChaseHQ-Native"},{"bridge","$workbenchVersion"},{"web","concurrent-front"},{"backend",backend},{"startedUtc",startedUtc.ToString("o")},{"uptimeSeconds",up},{"scriptCount",scriptCount},{"frontendBootstrap",File.Exists(readyFile)?"ready":"pending"}}); return; }
            if(path=="/api/v1/frontend-ready" && c.Request.HttpMethod=="POST") { var body=await ReadBody(c.Request); File.WriteAllBytes(Path.Combine(runtimeRoot,"frontend-ready.json"),body.Length>0?body:Utf8("{\"ok\":true}")); await JsonReply(c,new Dictionary<string,object>{{"ok",true},{"state","ready"},{"servedBy","concurrent-front"}}); return; }
            if(path=="/api/v1/scripts" && c.Request.HttpMethod=="GET") { await JsonReply(c,ListScripts()); return; }
            if(path=="/api/v1/scripts/load" && c.Request.HttpMethod=="POST") {
                var body=Encoding.UTF8.GetString(await ReadBody(c.Request)); var d=json.Deserialize<Dictionary<string,object>>(body);
                var rel=d!=null && d.ContainsKey("path")?Convert.ToString(d["path"]):""; var full=SafeScriptPath(rel,false);
                var rr=full.Substring(scriptRoot.Length).TrimStart('\\','/').Replace('\\','/');
                await JsonReply(c,new Dictionary<string,object>{{"path",rr},{"script",File.ReadAllText(full)},{"servedBy","concurrent-front"}}); return;
            }
            if(path=="/api/v1/scripts/save" && c.Request.HttpMethod=="POST") {
                var body=Encoding.UTF8.GetString(await ReadBody(c.Request)); var d=json.Deserialize<Dictionary<string,object>>(body);
                var rel=d!=null && d.ContainsKey("path")?Convert.ToString(d["path"]):""; var script=d!=null && d.ContainsKey("script")?Convert.ToString(d["script"]):"";
                var full=SafeScriptPath(rel,true); Directory.CreateDirectory(Path.GetDirectoryName(full)); File.WriteAllText(full,script,new UTF8Encoding(false));
                var rr=full.Substring(scriptRoot.Length).TrimStart('\\','/').Replace('\\','/'); await JsonReply(c,new Dictionary<string,object>{{"ok",true},{"path",rr},{"servedBy","concurrent-front"}}); return;
            }
            if(path=="/api/v1/next-steps" && c.Request.HttpMethod=="GET") { await Reply(c,200,"application/json; charset=utf-8",File.ReadAllBytes(nextSteps)); return; }
            if(path=="/api/v1/image/regions" && c.Request.HttpMethod=="GET") { await Reply(c,200,"application/json; charset=utf-8",File.ReadAllBytes(imageRegions)); return; }
            if(path=="/api/v1/logs" && c.Request.HttpMethod=="GET") {
                var files=new List<string>(); var wo=Path.Combine(runtimeRoot,"web-stdout.log"); var we=Path.Combine(runtimeRoot,"web-stderr.log"); var wl=Path.Combine(runtimeRoot,"web-listener.log"); var wc=Path.Combine(runtimeRoot,"web-command-diagnostics.log"); var wf=Path.Combine(runtimeRoot,"web-frontend-errors.log"); if(File.Exists(wo))files.Add("@runtime/web-stdout.log"); if(File.Exists(we))files.Add("@runtime/web-stderr.log"); if(File.Exists(wl))files.Add("@runtime/web-listener.log"); if(File.Exists(wc))files.Add("@runtime/web-command-diagnostics.log"); if(File.Exists(wf))files.Add("@runtime/web-frontend-errors.log");
                if(!string.IsNullOrEmpty(sessionRoot) && Directory.Exists(sessionRoot)) foreach(var f in Directory.GetFiles(sessionRoot,"*",SearchOption.AllDirectories).Where(f=>new[]{".log",".txt",".csv"}.Contains(Path.GetExtension(f).ToLowerInvariant())).OrderBy(f=>f)) files.Add(f.Substring(sessionRoot.Length).TrimStart('\\','/').Replace('\\','/'));
                await JsonReply(c,new Dictionary<string,object>{{"files",files},{"servedBy","concurrent-front"}}); return;
            }
            if(path=="/api/v1/logs/tail" && c.Request.HttpMethod=="GET") {
                var q=System.Web.HttpUtility.ParseQueryString(c.Request.Url.Query); var file=q["file"]??""; int lines=80; int.TryParse(q["lines"],out lines); if(lines<1)lines=1;if(lines>2000)lines=2000;
                string full;if(file=="@runtime/web-stdout.log")full=Path.Combine(runtimeRoot,"web-stdout.log");else if(file=="@runtime/web-stderr.log")full=Path.Combine(runtimeRoot,"web-stderr.log");else if(file=="@runtime/web-listener.log")full=Path.Combine(runtimeRoot,"web-listener.log");else if(file=="@runtime/web-command-diagnostics.log")full=Path.Combine(runtimeRoot,"web-command-diagnostics.log");else if(file=="@runtime/web-frontend-errors.log")full=Path.Combine(runtimeRoot,"web-frontend-errors.log");else { if(string.IsNullOrEmpty(sessionRoot))throw new Exception("No active session path"); full=Path.GetFullPath(Path.Combine(sessionRoot,file.Replace('/',Path.DirectorySeparatorChar))); var pre=Path.GetFullPath(sessionRoot).TrimEnd(Path.DirectorySeparatorChar)+Path.DirectorySeparatorChar;if(!full.StartsWith(pre,StringComparison.OrdinalIgnoreCase))throw new Exception("Log path escapes session"); }
                var text=TailFileShared(full,lines); await Reply(c,200,"text/plain; charset=utf-8",Utf8(text)); return;
            }
            await Proxy(c).ConfigureAwait(false);
        } catch(Exception ex) { JsonReply(c,new Dictionary<string,object>{{"ok",false},{"error",ex.Message},{"servedBy","concurrent-front"}},400).GetAwaiter().GetResult(); }
    }
    async Task Proxy(HttpListenerContext c){
        var target=new Uri(backend.TrimEnd('/')+c.Request.RawUrl);
        using(var m=new HttpRequestMessage(new HttpMethod(c.Request.HttpMethod),target)){
            if(c.Request.HasEntityBody){ var body=await ReadBody(c.Request); m.Content=new ByteArrayContent(body); if(!string.IsNullOrEmpty(c.Request.ContentType))m.Content.Headers.TryAddWithoutValidation("Content-Type",c.Request.ContentType); }
            foreach(string h in c.Request.Headers.AllKeys){ if(h.Equals("Host",StringComparison.OrdinalIgnoreCase)||h.Equals("Content-Length",StringComparison.OrdinalIgnoreCase)||h.Equals("Content-Type",StringComparison.OrdinalIgnoreCase))continue; try{m.Headers.TryAddWithoutValidation(h,c.Request.Headers[h]);}catch{} }
            int timeoutSeconds=(c.Request.RawUrl.StartsWith("/api/v1/script",StringComparison.OrdinalIgnoreCase)||c.Request.RawUrl.StartsWith("/api/v1/control/run-frames",StringComparison.OrdinalIgnoreCase)||c.Request.RawUrl.StartsWith("/api/v1/checkpoints/load",StringComparison.OrdinalIgnoreCase)||c.Request.RawUrl.StartsWith("/api/v1/input/ioc/sweep",StringComparison.OrdinalIgnoreCase))?3700:8;
            try{
                using(var timeoutCts=new CancellationTokenSource(TimeSpan.FromSeconds(timeoutSeconds)))
                using(var r=await client.SendAsync(m,HttpCompletionOption.ResponseContentRead,timeoutCts.Token).ConfigureAwait(false)){
                    var data=await r.Content.ReadAsByteArrayAsync().ConfigureAwait(false); var type=r.Content.Headers.ContentType!=null?r.Content.Headers.ContentType.ToString():"application/octet-stream";
                    if(r.Content.Headers.ContentDisposition!=null) try{c.Response.AddHeader("Content-Disposition",r.Content.Headers.ContentDisposition.ToString());}catch{}
                    await Reply(c,(int)r.StatusCode,type,data).ConfigureAwait(false);
                }
            } catch(TaskCanceledException){ JsonReply(c,new Dictionary<string,object>{{"ok",false},{"error","Backend request timed out after "+timeoutSeconds+"s"},{"state","DEGRADED"},{"path",c.Request.RawUrl}},504).GetAwaiter().GetResult(); }
            catch(Exception ex){ JsonReply(c,new Dictionary<string,object>{{"ok",false},{"error",ex.Message},{"state","DEGRADED"},{"path",c.Request.RawUrl}},502).GetAwaiter().GetResult(); }
        }
    }
    async Task Handle(HttpListenerContext c){ var path=c.Request.Url.AbsolutePath; await HandleLocal(c,path).ConfigureAwait(false); }
    public void Dispose(){ cts.Cancel(); try{listener.Stop();}catch{} try{listener.Close();}catch{} client.Dispose(); }
}
"@
try{
    if(-not ('ChaseHQFrontProxy' -as [type])){Add-Type -TypeDefinition $proxySource -ReferencedAssemblies @('System.Net.Http.dll','System.Web.Extensions.dll','System.Web.dll') -Language CSharp}
    Startup-Log 'FRONT_TYPE_READY'
}catch{Startup-Log ('FRONT_TYPE_FAIL '+$_.Exception.ToString());throw}
$runtimeRoot=Join-Path $root '.chq'
try{
    $frontProxy=[ChaseHQFrontProxy]::new($publicHttpPort,$backendHttpPort,$scriptLibraryRoot,$nextStepsPath,$imageRegionsPath,$runtimeRoot,$sessionPath)
    $frontProxy.Start()
    Startup-Log ('FRONT_READY http://127.0.0.1:'+$publicHttpPort+'/')
}catch{Startup-Log ('FRONT_FAIL '+$_.Exception.ToString());throw}
Remove-Item (Join-Path $runtimeRoot 'frontend-ready.json') -Force -ErrorAction SilentlyContinue
Remove-Item $runtimeFrontendDiag -Force -ErrorAction SilentlyContinue
Web-Log "[START] Workbench v$workbenchVersion" Cyan
Web-Log "[START] Front listener: http://127.0.0.1:$publicHttpPort" Cyan
Web-Log "[START] Backend bridge: http://127.0.0.1:$backendHttpPort" Cyan
Web-Log "[START] Script root: $scriptLibraryRoot" Cyan
Web-Log ("[START] Scripts discovered: " + @(Get-ChildItem $scriptLibraryRoot -Recurse -Filter *.chqscript -File -ErrorAction SilentlyContinue).Count) Green
Web-Log "[START] Native debugger: 127.0.0.1:$Port" Cyan
Write-Host "ChaseHQ Concurrent Web Frontend: http://127.0.0.1:$publicHttpPort/ (backend $backendHttpPort)"
function Read-ChqValue([string]$cpu,[string]$address,[int]$width,[string]$context=''){ $r=Invoke-Chq @('read',$cpu,$address,[string]$width);if($r -notmatch 'value=0x([0-9A-Fa-f]+)'){$tag=if([string]::IsNullOrWhiteSpace($context)){''}else{" [$context]"};throw "Memory read failed$tag cpu=$cpu address=$address width=${width}: $r"};return [Convert]::ToUInt32($Matches[1],16)}
function Bcd-Value([uint32]$v,[int]$digits=4){$n=0;$mul=1;for($i=0;$i -lt $digits;$i++){$n += (($v -shr ($i*4))-band 0xF)*$mul;$mul*=10};return $n}
function Signed16([uint32]$v){$w=[uint16]($v-band 0xFFFF);if($w-band 0x8000){return [int]$w-0x10000};return [int]$w}
function Signed8([uint32]$v){$b=[int]($v-band 0xFF);if($b-band 0x80){return $b-256};return $b}
function Get-LiveGameplayRegistry(){
    $meta=Get-Content $registryPath -Raw|ConvertFrom-Json
    $speed=Read-ChqValue A '100400' 16;$internal=Read-ChqValue A '10041C' 32
    $score=Read-ChqValue A '100488' 32;$scoreDisplay=Read-ChqValue A '100408' 32;$peakSpeed=Read-ChqValue A '10048C' 16
    $course=Read-ChqValue A '10A040' 32;$moveDelta=Read-ChqValue A '10A012' 16;$car=Read-ChqValue A '10A044' 16
    $rf=Read-ChqValue A '10A048' 8;$trackPage=Read-ChqValue A '10A058' 8;$trackRoute=Read-ChqValue A '10A059' 8;$trackOffset=Read-ChqValue A '10A05A' 16;$geometry=Read-ChqValue A '10A05C' 32
    $recordAddr=0x109000+$trackOffset;$record=@();for($i=0;$i -lt 8;$i++){$record += (Read-ChqValue A ('{0:X}' -f ($recordAddr+$i)) 8)}
    $curve=Signed8 $record[0];$shapeTarget=Signed8 $record[1]
    $shapeSmooth=Signed8 (Read-ChqValue A '10A04E' 8);$renderSelector=Read-ChqValue A '10A04A' 16;$renderCached=Read-ChqValue A '10A076' 16
    $edgeState=Signed8 (Read-ChqValue A '10A04C' 8);$steerProcessed=Signed16 (Read-ChqValue A '100300' 16);$steerRawMem=Read-ChqValue A '10014C' 16
    $roadMask=$rf-band 0x06;$road=if($roadMask-eq 0){'ON_ROAD'}elseif($roadMask-eq 4){'OFF_ROAD_LEFT'}elseif($roadMask-eq 2){'OFF_ROAD_RIGHT'}else{'OFF_ROAD_EXTREME'}
    $curveLabel=if($curve-lt 0){'LEFT'}elseif($curve-gt 0){'RIGHT'}else{'STRAIGHT'}
    $renderBand=if($shapeSmooth-le -5){'NEGATIVE'}elseif($shapeSmooth-ge 5){'POSITIVE'}else{'NEUTRAL'}
    $turboInput=Read-ChqValue A '100303' 8;$turboState=Read-ChqValue A '10040F' 8;$turbos=Read-ChqValue A '1003A2' 16;$turboTimer=Read-ChqValue A '100414' 16;$targetHealth=Read-ChqValue A '1002AE' 16
    $values=@{
      display_speed=[ordered]@{value=(Bcd-Value $speed 4);raw=('0x{0:X4}' -f $speed);unit='km/h'}
      internal_speed=[ordered]@{value=('0x{0:X8}' -f $internal);raw=([string]$internal)}
      score=[ordered]@{value=(Bcd-Value $score 8);raw=('0x{0:X8}' -f $score)}
      score_display_copy=[ordered]@{value=(Bcd-Value $scoreDisplay 8);raw=('0x{0:X8}' -f $scoreDisplay)}
      peak_display_speed=[ordered]@{value=(Bcd-Value $peakSpeed 4);raw=('0x{0:X4}' -f $peakSpeed);unit='km/h'}
      course_position=[ordered]@{value=$course;raw=('0x{0:X8}' -f $course);unit='course units'}
      longitudinal_delta=[ordered]@{value=$moveDelta;raw=('0x{0:X4}' -f $moveDelta);unit='course units/frame'}
      lateral_position=[ordered]@{value=$car;raw=('0x{0:X4}' -f $car)}
      road_state=[ordered]@{value=$road;raw=('0x{0:X2}' -f $rf)}
      track_record_offset=[ordered]@{value=('0x{0:X4}' -f $trackOffset);raw=('record #{0} | addr 0x{1:X6}' -f ([int]($trackOffset/8)),$recordAddr)}
      track_page=[ordered]@{value=$trackPage;raw=('0x{0:X2}' -f $trackPage)}
      track_route=[ordered]@{value=$trackRoute;raw=('0x{0:X2}' -f $trackRoute)}
      road_geometry=[ordered]@{value=('0x{0:X8}' -f $geometry);raw=('0x{0:X8}' -f $geometry)}
      track_curvature=[ordered]@{value=($curveLabel+' '+$(if($curve-ge 0){'+'+[string]$curve}else{[string]$curve}));raw=('record +0 = 0x{0:X2}' -f $record[0])}
      track_shape_target=[ordered]@{value=$shapeTarget;raw=('record +1 = 0x{0:X2}' -f $record[1])}
      track_shape_smooth=[ordered]@{value=$shapeSmooth;raw=('0x{0:X2}' -f (Read-ChqValue A '10A04E' 8))}
      player_render_band=[ordered]@{value=$renderBand;raw=('thresholds <=-5 / -4..+4 / >=+5')}
      player_render_selector=[ordered]@{value=('0x{0:X4}' -f $renderSelector);raw=('bank={0} sub=0x{1:X2}' -f (($renderSelector-band 0xC0)-shr 6),($renderSelector-band 0x1F))}
      player_render_selector_cached=[ordered]@{value=('0x{0:X4}' -f $renderCached);raw=('bank={0} sub=0x{1:X2}' -f (($renderCached-band 0xC0)-shr 6),($renderCached-band 0x1F))}
      selected_edge_state=[ordered]@{value=$edgeState;raw=('0x{0:X2}' -f (Read-ChqValue A '10A04C' 8))}
      steering_processed=[ordered]@{value=$steerProcessed;raw=('0x{0:X4}' -f ($steerProcessed-band 0xFFFF));unit=$(if($steerProcessed-lt 0){'LEFT'}elseif($steerProcessed-gt 0){'RIGHT'}else{'CENTRE'})}
      steering_raw=[ordered]@{value=('0x{0:X3}' -f (((($steerRawMem-band 0xFF)-shl 8)-bor (($steerRawMem-shr 8)-band 0xFF))-band 0x0FFF));raw=('RAM 0x{0:X4}' -f $steerRawMem);unit='signed-12-bit IOC'}
      current_track_record=[ordered]@{value=(@($record|ForEach-Object{'{0:X2}' -f $_}) -join ' ');raw=('0x{0:X6} | +0..+7' -f $recordAddr)}
      turbo_input=[ordered]@{value=(($turboInput-band 1)-ne 0);raw=('0x{0:X2}' -f $turboInput)}
      turbo_active=[ordered]@{value=(($turboState-band 2)-ne 0);raw=('0x{0:X2}' -f $turboState)}
      turbos_remaining=[ordered]@{value=$turbos;raw=('0x{0:X4}' -f $turbos)}
      turbo_timer=[ordered]@{value=$turboTimer;raw=('0x{0:X4}' -f $turboTimer)}
      target_health=[ordered]@{value=$(if($targetHealth-eq 0xFFFF){'DEFEATED'}else{[int]$targetHealth});raw=('0x{0:X4}' -f $targetHealth);unit=$(if($targetHealth-eq 0xFFFF){'terminal sentinel'}elseif($targetHealth-eq 0){'one final damaging hit remains'}else{'remaining-hit counter'})}
    }
    $items=@();foreach($e in $meta.entries){$v=$values[[string]$e.id];$items += [ordered]@{id=$e.id;label=$e.label;group=$e.group;source=$e.source;confidence=$e.confidence;value=$(if($v){$v.value}else{$null});raw=$(if($v){$v.raw}else{$null});unit=$(if($v){Prop $v 'unit' $null}else{$null});note=$(Prop $e 'note' $null)}}
    $context=[ordered]@{checkpoint=$script:currentCheckpointFile;mode=$(if($script:currentCheckpointFile -eq 'stage1-driving-2352.chqstate'){'ATTRACT MODE'}else{'unknown/live'});role=$(if($script:currentCheckpointFile -eq 'stage1-driving-2352.chqstate'){'attract_graphics_reference'}else{''})}
    return [ordered]@{schema=$meta.schema;game=$meta.game;context=$context;items=$items}
}
function Get-LiveTrackState(){
    $course=Read-ChqValue A '10A040' 32 'track.state/course_position';$lateral=Read-ChqValue A '10A044' 16 'track.state/lateral_position';$rf=Read-ChqValue A '10A048' 8 'track.state/road_flags'
    $page=Read-ChqValue A '10A058' 8 'track.state/page';$route=Read-ChqValue A '10A059' 8 'track.state/route';$offset=Read-ChqValue A '10A05A' 16 'track.state/record_offset';$geometry=Read-ChqValue A '10A05C' 32 'track.state/geometry'
    $recordAddr=0x109000+$offset;$bytes=@();for($i=0;$i -lt 8;$i++){$bytes += (Read-ChqValue A ('{0:X}' -f ($recordAddr+$i)) 8 ("track.state/record_byte_"+$i))}
    $roadMask=$rf-band 0x06;$road=if($roadMask-eq 0){'ON_ROAD'}elseif($roadMask-eq 4){'OFF_ROAD_LEFT'}elseif($roadMask-eq 2){'OFF_ROAD_RIGHT'}else{'OFF_ROAD_EXTREME'}
    $curve=Signed8 $bytes[0];$shape=Signed8 $bytes[1];$smooth=Signed8 (Read-ChqValue A '10A04E' 8 'track.state/shape_smooth')
    $band=if($smooth-le -5){'NEGATIVE'}elseif($smooth-ge 5){'POSITIVE'}else{'NEUTRAL'}
    $roadLeft=[int](($geometry-shr 16)-band 0xFFFF);$roadRight=[int]($geometry-band 0xFFFF);$delta=[int16](($roadRight-$roadLeft)-band 0xFFFF);$roadCentre=[int](($roadLeft+[int]($delta/2))-band 0xFFFF);$roadWidth=[Math]::Abs([int]$delta)
    $latErr=[int16](($lateral-$roadCentre)-band 0xFFFF);$latNorm=if($roadWidth-gt 0){[Math]::Round(([double]$latErr/([double]$roadWidth*0.5)),6)}else{0.0}
    $roadCtrl=Read-ChqValue B '801FFE' 16 'track.state/road_control'
    # Preserve representative TC0150ROD renderer state near the lower road view. This is raw evidence, not a material label.
    $roadLine=220;$aWordBase=(($roadCtrl-band 0x0300)-shl 2);$bWordBase=($roadCtrl-band 0x0C00)
    function Read-RoadLine([int]$wordBase,[string]$tag){$ra=0x800000+(($wordBase+($roadLine*4))*2);$rw=@();for($ri=0;$ri-lt 4;$ri++){$rw += Read-ChqValue B ('{0:X}' -f ($ra+$ri*2)) 16 ("track.state/$tag/$ri")};return $rw}
    $roadA=Read-RoadLine $aWordBase 'road_a';$roadB=Read-RoadLine $bWordBase 'road_b'
    $courseSurface=('course:{0:X2}-{1:X2}-{2:X2}-{3:X2}-{4:X2}-{5:X2}' -f $bytes[2],$bytes[3],$bytes[4],$bytes[5],$bytes[6],$bytes[7])
    $roadRenderSignature=('tc0150:{0:X4}:A-{1:X4}-{2:X4}:B-{3:X4}-{4:X4}' -f $roadCtrl,$roadA[2],$roadA[3],$roadB[2],$roadB[3])
    $surfaceSignature=$courseSurface+'|'+$roadRenderSignature
    return [ordered]@{coursePosition=$course;distance=$course;coursePositionRaw=('0x{0:X8}' -f $course);lateralPosition=$lateral;lateralRaw=('0x{0:X4}' -f $lateral);roadState=$road;flags=('0x{0:X2}' -f $rf);trackPage=$page;trackRoute=$route;trackOffset=$offset;trackOffsetRaw=('0x{0:X4}' -f $offset);recordIndex=[int]($offset/8);recordAddress=('0x{0:X6}' -f $recordAddr);recordBytes=(@($bytes|ForEach-Object{'{0:X2}' -f $_}) -join ' ');geometry=('0x{0:X8}' -f $geometry);roadLeft=$roadLeft;roadCentre=$roadCentre;roadRight=$roadRight;roadWidth=$roadWidth;lateralError=$latErr;lateralErrorNormalized=$latNorm;geometryCode=('0x{0:X2}' -f $bytes[3]);eventCode=('0x{0:X2}' -f $bytes[4]);visualSelector=('0x{0:X2}' -f $bytes[5]);featureTrigger=('0x{0:X2}' -f $bytes[6]);controlFlags=('0x{0:X2}' -f $bytes[7]);surfaceSignature=$surfaceSignature;courseSurfaceSignature=$courseSurface;roadRenderSignature=$roadRenderSignature;roadControl=('0x{0:X4}' -f $roadCtrl);roadSampleLine=$roadLine;roadARecord=(@($roadA|ForEach-Object{'{0:X4}' -f $_}) -join ' ');roadBRecord=(@($roadB|ForEach-Object{'{0:X4}' -f $_}) -join ' ');surfaceSemantic='UNCLASSIFIED';surfaceConfidence='OBSERVED_RAW_SIGNATURE';shapeTarget=$shape;shapeTargetRaw=('0x{0:X2}' -f $bytes[1]);shapeSmoothed=$smooth;gradientBand=$band;flagsType=('0x{0:X2}' -f $bytes[2]);curve=$curve;curveDirection=$(if($curve-lt 0){'LEFT'}elseif($curve-gt 0){'RIGHT'}else{'STRAIGHT'});semanticNote='record +0 horizontal curvature is confirmed; surfaceSignature preserves raw course-record selectors without assigning asphalt/dirt semantics'}
}
function Get-CourseFollowState(){
    $kv=Parse-Kv (Invoke-Chq @('course','status'))
    return [ordered]@{schema='chq-course-follow-live-v1';enabled=([int](Prop $kv 'enabled' 0)-ne 0);controller=[string](Prop $kv 'controller' 'hybrid');steer=[int](Prop $kv 'steer' 32);deadzone=[int](Prop $kv 'deadzone' 7);lookahead=[int](Prop $kv 'lookahead' 8);lateralKp=[double](Prop $kv 'lateral_kp' 0.006);lateralKd=[double](Prop $kv 'lateral_kd' 0.012);lateralMax=[int](Prop $kv 'lateral_max' 48);lateralDeadzone=[int](Prop $kv 'lateral_deadzone' 96);lateralBias=[int](Prop $kv 'lateral_bias' 0);slew=[int](Prop $kv 'slew' 8);speedControl=([int](Prop $kv 'speed_control' 0)-ne 0);steeringTarget=[int](Prop $kv 'steering_target' 0);curveNow=[int](Prop $kv 'curve_now' 0);curveAhead=[int](Prop $kv 'curve_ahead' 0);curvePredict=[int](Prop $kv 'curve_predict' 0);feedforward=[int](Prop $kv 'feedforward' 0);pTerm=[int](Prop $kv 'p_term' 0);dTerm=[int](Prop $kv 'd_term' 0);correction=[int](Prop $kv 'correction' 0);speedTarget=[int](Prop $kv 'speed_target' 390);accelCommand=([int](Prop $kv 'accel_cmd' 0)-ne 0);brakeCommand=([int](Prop $kv 'brake_cmd' 0)-ne 0);mode=[string](Prop $kv 'mode' 'IDLE');roadLeft=[string](Prop $kv 'road_left' '0x0');roadCentre=[string](Prop $kv 'road_centre' '0x0');targetLateral=[string](Prop $kv 'target_lateral' '0x0');roadRight=[string](Prop $kv 'road_right' '0x0');roadWidth=[int](Prop $kv 'road_width' 0);carLateral=[string](Prop $kv 'car_lateral' '0x0');lateralError=[int](Prop $kv 'lateral_error' 0);lateralErrorNormalized=[double](Prop $kv 'lateral_error_norm' 0);lateralSide=[string](Prop $kv 'lateral_side' 'CENTRE')}
}
function Set-CourseFollowConfig($args){
    $cmd=@('course','configure')
    $map=[ordered]@{controller='controller';steer='steer';deadzone='deadzone';lookahead='lookahead';lateralKp='lateral-kp';lateralKd='lateral-kd';lateralMax='lateral-max';lateralDeadzone='lateral-deadzone';bias='bias';lateralBias='bias';slew='slew';speedControl='speed-control'}
    foreach($key in $map.Keys){if($args.ContainsKey($key)){$v=[string]$args[$key];if($key-eq 'speedControl'){$v=if(Bool-Arg $args $key $false){'on'}else{'off'}};$cmd += @($map[$key],$v)}}
    if($cmd.Count-le 2){return Get-CourseFollowState}
    $raw=Invoke-Chq $cmd;if($raw -like 'ERR*'){throw $raw};return Get-CourseFollowState
}
function Start-CourseFollow(){ $raw=Invoke-Chq @('course','start');if($raw-like'ERR*'){throw $raw};return Get-CourseFollowState }
function Stop-CourseFollow(){ $raw=Invoke-Chq @('course','stop');if($raw-like'ERR*'){throw $raw};return Get-CourseFollowState }
function Reset-CourseFollow(){ $raw=Invoke-Chq @('course','reset');if($raw-like'ERR*'){throw $raw};return Get-CourseFollowState }
function Start-CourseSurvey([bool]$suppressCollision=$true,[bool]$holdTimer=$true){
    if($script:CourseSurveyActive){throw 'Course survey already active'}
    $timer=Parse-Kv (Invoke-Chq @('timer','status'));$collision=Parse-Kv (Invoke-Chq @('collision','response','get'));$course=Get-CourseFollowState
    $script:CourseSurveyRestore=[ordered]@{timerFrozen=([int](Prop $timer 'timer_frozen' 0)-ne 0);collisionEnabled=([int](Prop $collision 'response_enabled' 1)-ne 0);courseEnabled=[bool]$course.enabled}
    if($holdTimer){$null=Invoke-Chq @('timer','freeze')}
    if($suppressCollision){$null=Invoke-Chq @('collision','response','set','0')}
    $null=Start-CourseFollow
    $null=Clear-TrackRecorder;$null=Start-TrackRecorder
    $script:CourseSurveyActive=$true
    return [ordered]@{schema='chq-course-survey-live-v1';active=$true;course=(Get-CourseFollowState);track=(Get-TrackRecorderStatus);collisionSuppressed=$suppressCollision;timerHeld=$holdTimer}
}
function Stop-CourseSurvey(){
    $null=Stop-TrackRecorder;$null=Stop-CourseFollow
    if($script:CourseSurveyRestore){if(-not [bool]$script:CourseSurveyRestore.timerFrozen){$null=Invoke-Chq @('timer','resume')};if([bool]$script:CourseSurveyRestore.collisionEnabled){$null=Invoke-Chq @('collision','response','reset')};if([bool]$script:CourseSurveyRestore.courseEnabled){$null=Start-CourseFollow}}
    $script:CourseSurveyActive=$false;$script:CourseSurveyRestore=$null
    return [ordered]@{schema='chq-course-survey-live-v1';active=$false;course=(Get-CourseFollowState);track=(Get-TrackRecorderStatus)}
}
function Clear-TrackRecorder(){
    $script:TrackRecorderSamples=@()
    return Get-TrackRecorderSnapshot $false
}
function Get-TrackRecorderStatus(){
    $samples=@($script:TrackRecorderSamples);$count=$samples.Count
    $firstFrame=$null;$lastFrame=$null;$minX=$null;$maxX=$null;$minY=$null;$maxY=$null
    if($count -gt 0){
        $firstFrame=$samples[0].frame;$lastFrame=$samples[-1].frame
        $xs=@($samples|ForEach-Object{[double]$_.x});$ys=@($samples|ForEach-Object{[double]$_.y})
        $minX=($xs|Measure-Object -Minimum).Minimum;$maxX=($xs|Measure-Object -Maximum).Maximum
        $minY=($ys|Measure-Object -Minimum).Minimum;$maxY=($ys|Measure-Object -Maximum).Maximum
    }
    $surfaces=@($samples|ForEach-Object{$_.surfaceSignature}|Where-Object{$_}|Sort-Object -Unique)
    return [ordered]@{schema='chq-track-recorder-v2';enabled=[bool]$script:TrackRecorderEnabled;courseSurvey=[bool]$script:CourseSurveyActive;sampleCount=$count;segmentCount=[Math]::Max(0,$count-1);firstFrame=$firstFrame;lastFrame=$lastFrame;surfaceSignatureCount=$surfaces.Count;minX=$minX;maxX=$maxX;minY=$minY;maxY=$maxY;maxSamples=$script:TrackRecorderMaxSamples}
}
function Get-TrackRecorderSnapshot([bool]$includeSamples=$true){
    $o=Get-TrackRecorderStatus
    if($includeSamples){$o['samples']=@($script:TrackRecorderSamples)}
    return $o
}
function Add-TrackRecorderSample([bool]$includeSamples=$true){
    $t=Get-LiveTrackState;$st=Parse-Kv (Invoke-Chq @('status'));$course=Get-CourseFollowState
    $samples=@($script:TrackRecorderSamples);$prev=if($samples.Count){$samples[-1]}else{$null}
    $x=if($prev){[double]$prev.x}else{350.0};$y=if($prev){[double]$prev.y}else{470.0};$heading=if($prev){[double]$prev.heading}else{0.0}
    if($prev){
        $dd=[Math]::Max(0.0,[Math]::Min(20.0,[Math]::Abs([double]$t.distance-[double]$prev.distance)))
        $heading += [double]$t.curve * 0.0008 * [Math]::Max(1.0,$dd)
        $x += [Math]::Sin($heading) * [Math]::Max(1.0,$dd) * 3.0
        $y -= [Math]::Cos($heading) * [Math]::Max(1.0,$dd) * 3.0
    }
    # Projection is for visual overlay only. Authoritative road/car lateral units remain preserved separately.
    $halfRoadPx=18.0;$nx=[Math]::Cos($heading);$ny=[Math]::Sin($heading);$carOffsetPx=[Math]::Max(-54.0,[Math]::Min(54.0,[double]$t.lateralErrorNormalized*$halfRoadPx))
    $leftX=$x-$nx*$halfRoadPx;$leftY=$y-$ny*$halfRoadPx;$rightX=$x+$nx*$halfRoadPx;$rightY=$y+$ny*$halfRoadPx;$carX=$x+$nx*$carOffsetPx;$carY=$y+$ny*$carOffsetPx
    $speedRaw=Read-ChqValue A '100400' 16 'track.record/display_speed';$scoreRaw=Read-ChqValue A '100488' 32 'track.record/score';$steer=Signed16 (Read-ChqValue A '100300' 16 'track.record/steering_processed')
    $target=[ordered]@{classification='CONFIRMED_TARGET_CAR';record='0x10A080-0x10A0BF';longitudinal=Read-ChqValue A '10A080' 32 'track.record/target_longitudinal';lateral=Read-ChqValue A '10A084' 16 'track.record/target_lateral';localPulse=Read-ChqValue A '10A089' 8 'track.record/target_local_pulse';speed=Read-ChqValue A '10A092' 16 'track.record/target_speed';setpoint=Read-ChqValue A '10A096' 16 'track.record/target_setpoint';remainingHits=Read-ChqValue A '1002AE' 16 'track.record/target_health'}
    $sprites=@();foreach($sp in @(Get-LiveSprites)){if([int]$sp.visiblePixels-gt 0){$sprites += [ordered]@{classification='UNCLASSIFIED_DYNAMIC_OBJECT';slot=[int]$sp.slot;map=[int]$sp.map;palette=[int]$sp.palette;x=[int]$sp.x;y=[int]$sp.y;width=[int]$sp.width;height=[int]$sp.height;priority=[int]$sp.priority;visiblePixels=[int64]$sp.visiblePixels}}}
    $point=[ordered]@{};foreach($k in $t.Keys){$point[$k]=$t[$k]};$point['frame']=[int64](Prop $st 'frame' 0);$point['x']=[Math]::Round($x,4);$point['y']=[Math]::Round($y,4);$point['heading']=$heading
    $point['roadLeftX']=[Math]::Round($leftX,4);$point['roadLeftY']=[Math]::Round($leftY,4);$point['roadRightX']=[Math]::Round($rightX,4);$point['roadRightY']=[Math]::Round($rightY,4);$point['carX']=[Math]::Round($carX,4);$point['carY']=[Math]::Round($carY,4)
    $point['displaySpeed']=[int](Bcd-Value $speedRaw 4);$point['displaySpeedRaw']=('0x{0:X4}' -f $speedRaw);$point['score']=[int64](Bcd-Value $scoreRaw 8);$point['steeringProcessed']=$steer;$point['courseFollow']=$course;$point['target']=$target;$point['visibleSprites']=$sprites
    $script:TrackRecorderSamples += [pscustomobject]$point
    if($script:TrackRecorderSamples.Count -gt $script:TrackRecorderMaxSamples){$script:TrackRecorderSamples=@($script:TrackRecorderSamples|Select-Object -Last $script:TrackRecorderMaxSamples)}
    $result=Get-TrackRecorderSnapshot $includeSamples
    if(-not $includeSamples){$result['latest']=$script:TrackRecorderSamples[-1]}
    return $result
}
function Start-TrackRecorder(){ $script:TrackRecorderEnabled=$true; return Get-TrackRecorderSnapshot $false }
function Stop-TrackRecorder(){ $script:TrackRecorderEnabled=$false; return Get-TrackRecorderSnapshot $false }
function Export-TrackRecorderSvg([string]$requestedName=''){
    $samples=@($script:TrackRecorderSamples);if($samples.Count -lt 2){throw 'No recorded track data to export; record at least two samples first'}
    $base=if([string]::IsNullOrWhiteSpace($requestedName)){('track-'+(Get-Date -Format 'yyyyMMdd-HHmmss'))}else{SafeName $requestedName}
    $dir=Join-Path $evidencePath 'track-exports';New-Item -ItemType Directory -Path $dir -Force|Out-Null
    $name=$base;$file=Join-Path $dir ($name+'.svg');$n=1;while(Test-Path $file){$name=$base+'-'+$n;$n++;$file=Join-Path $dir ($name+'.svg')}
    $xs=@($samples|ForEach-Object{[double]$_.x});$ys=@($samples|ForEach-Object{[double]$_.y});$minX=($xs|Measure-Object -Minimum).Minimum;$maxX=($xs|Measure-Object -Maximum).Maximum;$minY=($ys|Measure-Object -Minimum).Minimum;$maxY=($ys|Measure-Object -Maximum).Maximum
    $pad=70.0;$w=[Math]::Max(100.0,($maxX-$minX)+2*$pad);$h=[Math]::Max(100.0,($maxY-$minY)+2*$pad);$vx=$minX-$pad;$vy=$minY-$pad
    function Path-For($items,[string]$xName,[string]$yName){$pp=@();for($i=0;$i-lt$items.Count;$i++){$q=$items[$i];$pp += ('{0} {1:F2} {2:F2}' -f $(if($i){'L'}else{'M'}),[double]$q.$xName,[double]$q.$yName)};return ($pp-join' ')}
    $centre=Path-For $samples 'x' 'y';$left=Path-For $samples 'roadLeftX' 'roadLeftY';$right=Path-For $samples 'roadRightX' 'roadRightY';$car=Path-For $samples 'carX' 'carY'
    $lines=New-Object Text.StringBuilder
    for($i=1;$i -lt $samples.Count;$i++){$a=$samples[$i-1];$b=$samples[$i];$colour=if($b.gradientBand -eq 'NEGATIVE'){'#60a5fa'}elseif($b.gradientBand -eq 'POSITIVE'){'#fb923c'}else{'#d1d5db'};$title=[System.Security.SecurityElement]::Escape(('frame {0} page {1}/{2} record #{3} curve={4} surface={5} speed={6}km/h road={7}' -f $b.frame,$b.trackRoute,$b.trackPage,$b.recordIndex,$b.curve,$b.surfaceSignature,$b.displaySpeed,$b.roadState));$null=$lines.AppendLine(('  <line x1="{0:F2}" y1="{1:F2}" x2="{2:F2}" y2="{3:F2}" stroke="{4}" stroke-width="4" stroke-linecap="round"><title>{5}</title></line>' -f [double]$a.x,[double]$a.y,[double]$b.x,[double]$b.y,$colour,$title))}
    $last=$samples[-1]
    $xml=@"
<?xml version="1.0" encoding="UTF-8"?>
<svg xmlns="http://www.w3.org/2000/svg" viewBox="$('{0:F2} {1:F2} {2:F2} {3:F2}' -f $vx,$vy,$w,$h)" width="1400" height="1040">
 <rect x="$('{0:F2}' -f $vx)" y="$('{0:F2}' -f $vy)" width="$('{0:F2}' -f $w)" height="$('{0:F2}' -f $h)" fill="#080b10"/>
 <path d="$left" fill="none" stroke="#94a3b8" stroke-width="1.5" opacity="0.70"/>
 <path d="$right" fill="none" stroke="#94a3b8" stroke-width="1.5" opacity="0.70"/>
 <path d="$centre" fill="none" stroke="#6b7280" stroke-width="1.5" opacity="0.35"/>
$($lines.ToString()) <path d="$car" fill="none" stroke="#facc15" stroke-width="2.25" opacity="0.95"><title>Player driven trajectory projection</title></path>
 <circle cx="$('{0:F2}' -f [double]$last.carX)" cy="$('{0:F2}' -f [double]$last.carY)" r="6" fill="#f8fafc"/>
</svg>
"@
    $xml.TrimStart()|Set-Content -Path $file -Encoding UTF8
    return [ordered]@{ok=$true;name=$name;path=$file;schema='chq-track-map-svg-v2';sampleCount=$samples.Count;segmentCount=$samples.Count-1;contains=@('road-left','road-centre','road-right','player-trajectory');download=('/api/v1/track/svg/'+$name+'/download')}
}
function Export-TrackMappingDataset([string]$requestedName=''){
    $samples=@($script:TrackRecorderSamples);if($samples.Count -lt 2){throw 'No recorded track data to export; record at least two samples first'}
    $base=if([string]::IsNullOrWhiteSpace($requestedName)){('course-map-'+(Get-Date -Format 'yyyyMMdd-HHmmss'))}else{SafeName $requestedName};$root=Join-Path $evidencePath 'track-maps';New-Item -ItemType Directory -Force -Path $root|Out-Null
    $name=$base;$dir=Join-Path $root $name;$n=1;while(Test-Path $dir){$name=$base+'-'+$n;$n++;$dir=Join-Path $root $name};New-Item -ItemType Directory -Force -Path $dir|Out-Null
    $surfaceGroups=@($samples|Group-Object surfaceSignature|ForEach-Object{[pscustomobject][ordered]@{signature=$_.Name;semantic='UNCLASSIFIED';confidence='OBSERVED_RAW_SIGNATURE';samples=$_.Count;firstFrame=$_.Group[0].frame;lastFrame=$_.Group[-1].frame;firstCoursePosition=$_.Group[0].coursePosition;lastCoursePosition=$_.Group[-1].coursePosition;exampleRecordBytes=$_.Group[0].recordBytes;roadRenderSignature=$_.Group[0].roadRenderSignature;roadControl=$_.Group[0].roadControl;roadARecord=$_.Group[0].roadARecord;roadBRecord=$_.Group[0].roadBRecord;visualSelector=$_.Group[0].visualSelector;geometryCode=$_.Group[0].geometryCode;eventCode=$_.Group[0].eventCode}})
    $entities=@();foreach($q in $samples){$t=$q.target;$entities += [pscustomobject][ordered]@{frame=$q.frame;coursePosition=$q.coursePosition;kind='CONFIRMED_TARGET_CAR';source='0x10A080-0x10A0BF';longitudinal=$t.longitudinal;lateral=$t.lateral;map=$null;slot=$null;palette=$null;screenX=$null;screenY=$null};foreach($sp in @($q.visibleSprites)){$entities += [pscustomobject][ordered]@{frame=$q.frame;coursePosition=$q.coursePosition;kind='UNCLASSIFIED_DYNAMIC_OBJECT';source='live-sprite-census';longitudinal=$null;lateral=$null;map=$sp.map;slot=$sp.slot;palette=$sp.palette;screenX=$sp.x;screenY=$sp.y;screenW=$sp.width;screenH=$sp.height;priority=$sp.priority;visiblePixels=$sp.visiblePixels}}}
    $sampleCsv=Join-Path $dir 'track-samples.csv';$samples|Select-Object frame,coursePosition,displaySpeed,score,trackRoute,trackPage,recordIndex,recordAddress,recordBytes,curve,shapeTarget,shapeSmoothed,roadState,roadLeft,roadCentre,roadRight,roadWidth,lateralPosition,lateralError,lateralErrorNormalized,steeringProcessed,surfaceSignature,courseSurfaceSignature,roadRenderSignature,roadControl,roadARecord,roadBRecord,x,y,roadLeftX,roadLeftY,roadRightX,roadRightY,carX,carY|Export-Csv $sampleCsv -NoTypeInformation -Encoding UTF8
    $carCsv=Join-Path $dir 'car-trajectory.csv';$samples|Select-Object frame,coursePosition,displaySpeed,lateralPosition,lateralError,lateralErrorNormalized,steeringProcessed,roadState,carX,carY,@{n='courseFollowMode';e={$_.courseFollow.mode}},@{n='courseFollowTarget';e={$_.courseFollow.steeringTarget}},@{n='courseFollowBias';e={$_.courseFollow.lateralBias}}|Export-Csv $carCsv -NoTypeInformation -Encoding UTF8
    $entityCsv=Join-Path $dir 'track-entities.csv';$entities|Export-Csv $entityCsv -NoTypeInformation -Encoding UTF8
    $surfaceCsv=Join-Path $dir 'track-surfaces.csv';$surfaceGroups|Export-Csv $surfaceCsv -NoTypeInformation -Encoding UTF8
    $json=Join-Path $dir 'track-map.json';[ordered]@{schema='chq-course-map-v1';build=$workbenchVersion;created=(Get-Date).ToString('o');name=$name;status='OBSERVED';semanticsPolicy='Unknown dynamic sprites and raw surface signatures remain unclassified until evidence supports promotion.';sampleCount=$samples.Count;samples=$samples;surfaces=$surfaceGroups;entities=$entities}|ConvertTo-Json -Depth 16|Set-Content $json -Encoding UTF8
    $svgResult=Export-TrackRecorderSvg ($name+'-overview');$svgCopy=Join-Path $dir 'track-map.svg';Copy-Item $svgResult.path $svgCopy -Force
    $readme=Join-Path $dir 'README.txt';@('ChaseHQ-Native course mapping dataset',('build='+$workbenchVersion),('samples='+$samples.Count),'track-samples.csv: authoritative sampled road/course state plus map projection','car-trajectory.csv: player driven line and autonomous-driver telemetry','track-entities.csv: confirmed target-car state plus unclassified live sprite census','track-surfaces.csv: raw surface signatures; semantic material names intentionally unassigned','track-map.json: reconstructable master dataset','track-map.svg: road edges/centreline + player trajectory visualisation')|Set-Content $readme -Encoding UTF8
    return [ordered]@{ok=$true;schema='chq-course-map-export-v1';name=$name;path=$dir;json=$json;samples=$sampleCsv;trajectory=$carCsv;entities=$entityCsv;surfaces=$surfaceCsv;svg=$svgCopy;sampleCount=$samples.Count;entityRows=$entities.Count;surfaceSignatures=$surfaceGroups.Count}
}
function Resolve-ResearchImage([string]$name,[string]$hudMode='normal'){
    $safe=SafeName $name;$mode=([string]$hudMode).ToLowerInvariant();if($mode -notin 'normal','hidden','only','exclude'){throw 'hud must be normal, hidden/exclude, or only'};if($mode -eq 'exclude'){$mode='hidden'}
    $snapDir=Join-Path (Join-Path $evidencePath 'frame-snapshots') $safe;$snapFile=if($mode -eq 'hidden'){'final-no-hud.png'}elseif($mode -eq 'only'){'hud-only.png'}else{'final.png'}
    $candidates=@();if($mode -eq 'normal'){$candidates+=@((Join-Path $evidencePath ($safe+'.png')),(Join-Path (Join-Path $evidencePath $safe) 'screenshot.png'))};$candidates+=(Join-Path $snapDir $snapFile)
    foreach($x in $candidates){if(Test-Path $x -PathType Leaf){return $x}};if($mode -ne 'normal'){throw "HUD mode '$mode' requires a layered structured frame snapshot: $safe"};throw "Image not found: $safe"
}
function Compare-ResearchImages([string]$aName,[string]$bName,[string]$regionName='FULL',[string]$outputName='',[string]$hudMode='normal'){
    Add-Type -AssemblyName System.Drawing
    $aPath=Resolve-ResearchImage $aName $hudMode;$bPath=Resolve-ResearchImage $bName $hudMode;$regions=(Get-Content $imageRegionsPath -Raw|ConvertFrom-Json).regions;$r=Prop $regions $regionName $null;if($null -eq $r){throw "Unknown image region: $regionName"}
    $a=[Drawing.Bitmap]::new($aPath);$b=[Drawing.Bitmap]::new($bPath);try{if($a.Width -ne $b.Width -or $a.Height -ne $b.Height){throw 'Image dimensions differ'};$x0=[int]$r.x;$y0=[int]$r.y;$w=[math]::Min([int]$r.w,$a.Width-$x0);$h=[math]::Min([int]$r.h,$a.Height-$y0);$changed=0;$sum=0L;$minX=999999;$minY=999999;$maxX=-1;$maxY=-1;$diff=$null;if($outputName){$diff=[Drawing.Bitmap]::new($a.Width,$a.Height)}
        for($y=$y0;$y -lt $y0+$h;$y++){for($x=$x0;$x -lt $x0+$w;$x++){$ca=$a.GetPixel($x,$y);$cb=$b.GetPixel($x,$y);$d=[math]::Abs($ca.R-$cb.R)+[math]::Abs($ca.G-$cb.G)+[math]::Abs($ca.B-$cb.B);$sum+=$d;if($d -ne 0){$changed++;$minX=[math]::Min($minX,$x);$minY=[math]::Min($minY,$y);$maxX=[math]::Max($maxX,$x);$maxY=[math]::Max($maxY,$y)};if($diff){$v=[math]::Min(255,[int]($d/3));$diff.SetPixel($x,$y,[Drawing.Color]::FromArgb($v,$v,$v))}}}
        $total=$w*$h;$out=$null;if($diff){$out=Join-Path $evidencePath ((SafeName $outputName)+'.png');$diff.Save($out,[Drawing.Imaging.ImageFormat]::Png);$diff.Dispose()}
        return [ordered]@{a=$aName;b=$bName;region=$regionName;hud=$hudMode;pixels=$total;changedPixels=$changed;changedPercent=[math]::Round(100.0*$changed/[math]::Max(1,$total),4);meanAbsRgb=[math]::Round($sum/[math]::Max(1,3*$total),4);identical=($changed-eq0);boundingBox=$(if($changed){[ordered]@{x=$minX;y=$minY;w=$maxX-$minX+1;h=$maxY-$minY+1}}else{$null});diffPath=$out}
    }finally{$a.Dispose();$b.Dispose()}
}
$html=@'
<!doctype html><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>ChaseHQ Web Debugger</title>
<style>
:root{color-scheme:dark}*{box-sizing:border-box}body{font:14px system-ui;margin:0;background:#101216;color:#e8e8e8}header{position:sticky;top:0;z-index:2;background:#181b21;border-bottom:1px solid #343944;padding:10px 14px;display:flex;gap:8px;align-items:center;flex-wrap:wrap}button,input,select{background:#242933;color:#eee;border:1px solid #454c59;border-radius:5px;padding:7px}button{cursor:pointer}button:hover{background:#303744}button.primary{border-color:#6b86b8}.badge{padding:5px 8px;border-radius:12px;background:#242933}.ok{color:#86efac}.warn{color:#fcd34d}.grid{padding:12px;display:grid;grid-template-columns:repeat(12,minmax(0,1fr));gap:10px}.panel{background:#181b21;border:1px solid #343944;border-radius:7px;min-width:0}.panel h2{font-size:13px;margin:0;padding:8px 10px;border-bottom:1px solid #343944}.body{padding:9px}.w4{grid-column:span 4}.w5{grid-column:span 5}.w6{grid-column:span 6}.w7{grid-column:span 7}.w8{grid-column:span 8}.w12{grid-column:span 12}pre{margin:0;white-space:pre-wrap;overflow:auto;max-height:310px;font:12px ui-monospace,monospace}.row{display:flex;gap:6px;flex-wrap:wrap;margin-bottom:7px;align-items:center}.row input{min-width:0}.game-lab-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(280px,1fr));gap:12px;margin:10px 0}.game-lab-card{border:1px solid #343944;border-radius:7px;background:#111821;padding:12px}.game-lab-card h3{margin:0 0 5px;font-size:13px;color:#eaf3fb}.game-lab-card .row{gap:9px;margin:9px 0}.game-lab-card button{padding:9px 11px}.game-lab-card .state-line{font:12px ui-monospace,monospace;color:#b7c8d7;background:#0b1016;border:1px solid #293440;border-radius:5px;padding:7px 9px;margin:7px 0}.game-lab-card .hint{font-size:11px;color:#8fa4b7;line-height:1.4}.game-lab-card .toggle.on{background:#244b38;border-color:#4f9d78}.tile-inspector-grid{display:grid;grid-template-columns:minmax(250px,360px) minmax(0,1fr);gap:12px}.tile-preview{width:256px;height:256px;image-rendering:pixelated;background:#080b0f;border:1px solid #343944}.tile-pen-grid{font:12px ui-monospace,monospace;white-space:pre}@media(max-width:900px){.tile-inspector-grid{grid-template-columns:1fr}}.history{width:100%;height:80px;background:#11151b;border:1px solid #343944}.pin{display:grid;grid-template-columns:1fr auto;gap:5px;margin:4px 0}.muted{color:#9ca3af}.preview{width:100%;height:auto;image-rendering:pixelated;background:#000;border:1px solid #343944;display:block}.quick{font:12px ui-monospace,monospace}.good{color:#86efac}.bad{color:#fca5a5}a{color:#93c5fd}textarea{width:100%;min-height:180px;background:#11151b;color:#eee;border:1px solid #454c59;border-radius:5px;padding:8px;font:12px ui-monospace,monospace;resize:vertical}.evidence-list{display:grid;gap:6px}.evidence-item{border:1px solid #343944;border-radius:5px;padding:7px;cursor:pointer}.evidence-item:hover{background:#202630}.evidence-item.selected{border-color:#7aa2f7;background:#20293a}.evidence-state{display:inline-block;margin-left:6px;padding:2px 6px;border-radius:10px;background:#242933;font-size:11px}.evidence-shot{width:100%;max-width:520px;image-rendering:pixelated;border:1px solid #343944;background:#000}@media(max-width:1200px){.grid{grid-template-columns:repeat(6,minmax(0,1fr))}.w4{grid-column:span 3}.w5,.w6,.w7,.w8,.w12{grid-column:span 6}}@media(max-width:700px){header{position:relative;padding:8px}.tabs{position:relative;top:auto;overflow-x:auto;flex-wrap:nowrap}.tabs button{flex:0 0 auto}.grid{padding:7px;grid-template-columns:1fr}.w4,.w5,.w6,.w7,.w8,.w12,.panel{grid-column:1!important}.history-row{grid-template-columns:1fr 1fr}.row>*{max-width:100%}.row input,.row select{min-width:0!important;width:100%}.evidence-shot{max-width:100%}}
.tabs{position:sticky;top:53px;z-index:2;background:#12161d;border-bottom:1px solid #343944;padding:7px 12px;display:flex;gap:6px;flex-wrap:wrap}.tabbtn.active{background:#334155;border-color:#7aa2f7}.panel.tab-hidden{display:none}.toggle.on{background:#244b38;border-color:#4ade80}.registry-group{grid-column:1/-1;font-weight:700;font-size:15px;margin-top:8px;padding:7px 4px;border-bottom:1px solid #343944}.registry-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(260px,1fr));gap:8px}.registry-card{border:1px solid #343944;border-radius:6px;padding:9px;background:#12161d}.registry-card .value{font:18px ui-monospace,monospace;margin:4px 0}.confidence{display:inline-block;font-size:10px;padding:2px 6px;border-radius:9px;background:#242933}.confidence.confirmed{color:#86efac}.confidence.high{color:#fde68a}.confidence.unknown{color:#9ca3af}.next-item{border-left:3px solid #475569;padding:8px 10px;margin:6px 0;background:#12161d}.next-item[data-status="IN PROGRESS"]{border-left-color:#60a5fa}.next-item[data-status="CONFIRMED"]{border-left-color:#4ade80}.next-item[data-status="BLOCKED"]{border-left-color:#f59e0b}.track-wrap{display:grid;grid-template-columns:minmax(0,2fr) minmax(220px,1fr);gap:10px}.track-svg{width:100%;height:520px;background:#080b10;border:1px solid #343944;border-radius:6px}.script-shell{display:grid;grid-template-columns:1fr;gap:10px}.ioc-table{width:100%;border-collapse:collapse;font:12px ui-monospace,monospace}.ioc-table td,.ioc-table th{border-bottom:1px solid #29303a;padding:4px 6px;text-align:right}.ioc-table th:first-child,.ioc-table td:first-child{text-align:left}.sprite-table{width:100%;border-collapse:collapse;font:12px ui-monospace,monospace}.sprite-table th,.sprite-table td{border-bottom:1px solid #29303a;padding:5px 7px;text-align:right}.sprite-table th:first-child,.sprite-table td:first-child{text-align:left}.sprite-changed{background:#332b16}.sprite-new{background:#173326}.sprite-stale{opacity:.55}.sprite-badge{font-size:10px;padding:2px 5px;border-radius:8px;background:#242933}.graphics-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(220px,1fr));gap:8px}.layer-stack{position:relative;background:#000;border:1px solid #343944;display:inline-block;max-width:100%}.layer-stack img{position:absolute;inset:0;width:100%;image-rendering:pixelated}.layer-stack img:first-child{position:relative}.layer-toggle{font-size:11px}@media(max-width:900px){.script-shell,.track-wrap{grid-template-columns:1fr}.tabs{top:92px}}

.script-output-wrap{grid-column:1/-1;border-top:1px solid #343944;padding-top:8px}.script-output-toolbar{display:flex;gap:6px;align-items:center;flex-wrap:wrap;margin-bottom:6px}.script-output{width:100%;height:420px;max-height:none;resize:vertical;overflow:auto;background:#0b0f15;border:1px solid #343944;padding:8px}.script-command-block{border-bottom:1px solid #1f2630;padding:0 0 8px;margin:0 0 8px}.script-command-text{margin:0;max-height:none;white-space:pre;font:12px ui-monospace,monospace}.script-output.wrap .script-command-text{white-space:pre-wrap;word-break:break-word}.inline-artifacts{display:flex;gap:8px;flex-wrap:wrap;margin-top:6px;padding-left:12px;border-left:2px solid #334155}.inline-artifact img{display:block;max-width:220px;max-height:160px;image-rendering:pixelated;background:#000;border:1px solid #343944;cursor:zoom-in}.script-output-wrap.maximized{position:fixed;inset:64px 10px 10px;z-index:30;background:#181b21;border:1px solid #64748b;border-radius:8px;padding:10px}.script-output-wrap.maximized .script-output{height:calc(100vh - 150px)}.artifact-grid{display:flex;gap:8px;flex-wrap:wrap;margin-top:8px}.artifact-summary{width:100%;border:1px solid #343944;border-radius:6px;padding:8px 10px;background:#10151c;color:#c8d2dc;font:12px ui-monospace,monospace}.artifact-summary b{color:#edf4fa}.artifact-note{width:100%;color:#9ca3af;font-size:11px}.artifact-card{border:1px solid #343944;border-radius:6px;padding:6px;background:#12161d;max-width:220px}.artifact-card img{display:block;max-width:200px;max-height:140px;image-rendering:pixelated;background:#000;cursor:zoom-in}.artifact-missing{color:#fca5a5;font-size:11px}.script-output-elided{padding:5px 8px;margin:0 0 8px;border:1px dashed #485365;color:#9ca3af;background:#10151c;font:11px ui-monospace,monospace}.img-modal{display:none;position:fixed;inset:0;z-index:80;background:rgba(0,0,0,.86);align-items:center;justify-content:center;padding:24px}.img-modal.open{display:flex}.img-modal-card{max-width:96vw;max-height:96vh;background:#11151b;border:1px solid #64748b;border-radius:8px;padding:10px;display:flex;flex-direction:column;gap:8px}.img-modal img{max-width:92vw;max-height:80vh;object-fit:contain;image-rendering:pixelated;background:#000}.img-modal-toolbar{display:flex;gap:8px;align-items:center;flex-wrap:wrap}.script-history{margin-top:8px;border-top:1px solid #343944;padding-top:8px}.history-row{display:grid;grid-template-columns:170px 90px 110px 190px 1fr 90px;gap:8px;padding:6px;border-bottom:1px solid #252b35;font:12px ui-monospace,monospace;cursor:pointer}.history-row:hover{background:#171d26}.history-row.selected{background:#20293a;outline:1px solid #4f6b99}.history-actions{display:flex;gap:6px;align-items:center;flex-wrap:wrap;margin-top:8px;padding-top:8px;border-top:1px solid #343944}.docs-shell{display:grid;grid-template-columns:minmax(260px,360px) minmax(0,1fr);gap:10px}.docs-list{max-height:650px;overflow:auto;border:1px solid #343944;background:#0d1117}.docs-item{padding:9px 10px;border-bottom:1px solid #252b35;cursor:pointer}.docs-item:hover,.docs-item.selected{background:#1b2634}.docs-item b{display:block}.docs-meta{display:flex;gap:5px;flex-wrap:wrap;margin-top:4px}.docs-chip{font-size:10px;padding:2px 6px;border:1px solid #37506a;border-radius:10px;background:#0b1924;color:#c9ebff}.docs-detail{min-height:360px}.docs-detail h2{font-size:24px;padding:0;border:0}.docs-evidence{display:grid;grid-template-columns:repeat(auto-fit,minmax(220px,1fr));gap:9px;margin:10px 0}.docs-evidence figure{margin:0;border:1px solid #343944;background:#10151c;padding:6px}.docs-evidence img{width:100%;image-rendering:pixelated;background:#000}.docs-evidence figcaption{font-size:11px;color:#aab3bf;margin-top:5px}.docs-checker{height:6px;background:repeating-linear-gradient(90deg,#f5f5f5 0 24px,#111 24px 48px);margin:-9px -9px 10px}.docs-title{font-family:Impact,Haettenschweiler,"Arial Narrow Bold",sans-serif;font-style:italic;letter-spacing:.04em}.docs-status-confirmed{color:#86efac}.docs-status-open,.docs-status-provisional{color:#fde68a}.docs-status-implemented-awaiting-proof{color:#93c5fd}.docs-modebar{display:flex;gap:6px;flex-wrap:wrap;margin-bottom:10px}.docs-modebar button.on{background:#23456b;border-color:#5ba7ff}.docs-nav-group{border-bottom:1px solid #26303b;padding:6px 0}.docs-nav-title{padding:6px 10px;color:#89a5bf;font-size:11px;text-transform:uppercase;letter-spacing:.08em;font-weight:800}.docs-nav-item{display:block;width:100%;text-align:left;border:0;border-radius:0;background:transparent;padding:7px 11px;color:#d6e2ec;cursor:pointer}.docs-nav-item:hover{background:#182333}.docs-nav-item.mono{font-family:ui-monospace,Consolas,monospace;font-size:12px}.docs-page-head{display:flex;justify-content:space-between;gap:20px;align-items:flex-start;border-bottom:1px solid #2a3541;padding-bottom:12px;margin-bottom:16px}.docs-lede{font-size:15px;color:#b7c8d7;max-width:900px}.docs-detail{line-height:1.58}.docs-detail h3{margin:22px 0 8px;color:#eaf3fb}.docs-code{border:1px solid #334453;background:#090f15;margin:9px 0 15px}.docs-code-head{display:flex;justify-content:space-between;align-items:center;padding:5px 8px;border-bottom:1px solid #283541;color:#8ea6bb;font-size:11px}.docs-code pre{margin:0;border:0;background:transparent;white-space:pre-wrap}.docs-callout{border-left:4px solid #2698ff;background:#101923;padding:11px 13px;margin:12px 0}.docs-callout.warning{border-color:#ffc43d}.docs-callout b{display:block;margin-bottom:4px}.docs-bullets{padding-left:24px}.docs-bullets li{margin:5px 0}.docs-table{width:100%;border-collapse:collapse;margin:9px 0 16px;font-size:12px}.docs-table th,.docs-table td{border:1px solid #303d49;padding:7px 9px;text-align:left;vertical-align:top}.docs-table th{background:#17212a}.docs-table-wrap{overflow:auto}.docs-card-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(160px,1fr));gap:9px;margin:12px 0}.docs-card{border:1px solid #303d49;background:#101820;padding:12px}.docs-big{font-size:28px;font-weight:800;margin-top:6px}.docs-pills{display:flex;gap:6px;flex-wrap:wrap}.docsInlineLink,.docsApiLink,.docsRelated{cursor:pointer}.docs-glossary{display:grid;grid-template-columns:180px 1fr;gap:1px;background:#2a3744;border:1px solid #2a3744}.docs-glossary dt,.docs-glossary dd{margin:0;background:#10171f;padding:8px 10px}.docs-glossary dt{font-weight:800}.docs-search-tip{color:#8fa4b7;font-size:11px;margin-left:auto}@media(max-width:900px){.docs-shell{grid-template-columns:1fr}.docs-page-head{display:block}.docs-glossary{grid-template-columns:1fr}}
</style>
<header><strong>ChaseHQ Research Workbench v@@CHQ_VERSION@@</strong><span id="conn" class="badge">connecting</span><span id="frame" class="badge">frame ?</span><span id="uptime" class="badge">uptime ?</span><button id="runPauseToggle" class="toggle">Emulation: ?</button><button data-cmd="step frame 1">Step frame</button><button id="timerToggle" class="toggle">Timer hold: OFF</button><button id="topToggle" class="toggle">SDL top: OFF</button><button id="capture">Capture evidence</button><label>Refresh <select id="rate"><option value="250">250 ms</option><option value="500">500 ms</option><option selected value="1000">1 s</option><option value="2000">2 s</option></select></label></header><nav class="tabs"><button class="tabbtn active" data-tab="dashboard">Dashboard</button><button class="tabbtn" data-tab="script">Script Console</button><button class="tabbtn" data-tab="registry">Gameplay Registry</button><button class="tabbtn" data-tab="track">Track View</button><button class="tabbtn" data-tab="graphics">Graphics Lab</button><button class="tabbtn" data-tab="docs">Docs / Knowledge</button><button class="tabbtn" data-tab="experimental">Experimental</button><button class="tabbtn" data-tab="evidence">Evidence</button><button class="tabbtn" data-tab="next">Next Steps</button><button class="tabbtn" data-tab="diagnostics">Logs / Diagnostics</button></nav>
<div class="grid">
<section class="panel w7"><h2>Current SDL frame</h2><div class="body"><img id="preview" class="preview" alt="Current ChaseHQ SDL frame"><div class="row" style="margin-top:7px"><button id="previewNow">Refresh frame</button><label>Auto <select id="previewRate"><option value="0">Off</option><option value="500">0.5 s</option><option selected value="1000">1 s</option><option value="2000">2 s</option><option value="5000">5 s</option></select></label><span id="previewStamp" class="muted"></span></div></div></section>
<section class="panel w5"><h2>SDL Window Controls</h2><div class="body"><div class="row"><button id="sdlShow">Show</button><button id="sdlHide">Hide</button><button id="sdlMinimize">Minimize</button><button id="sdlRestore">Restore</button></div><div class="row"><button id="sdlFullscreen" class="toggle">Fullscreen: ?</button><button id="sdlTop" class="toggle">Always on top: ?</button><label>Scale <select id="sdlScale"><option>1</option><option>2</option><option selected>3</option><option>4</option><option>5</option><option>6</option><option>7</option><option>8</option></select></label></div><div class="row"><button id="sdlPause">Pause</button><button id="sdlResume">Resume</button><button id="sdlStep">Step frame</button><button id="sdlSnapshot">Layered snapshot</button></div><div id="sdlWindowState" class="state-line">SDL state unknown</div><div class="muted"><b>SDL hotkey:</b> Pause/Break toggles pause/resume immediately during manual play. All runtime/window actions remain exposed through the Research API and .chqscript.</div></div></section>
<section class="panel w6"><h2>Machine status</h2><div class="body"><pre id="status"></pre><canvas id="hist" class="history" width="500" height="80"></canvas><div class="muted">Rolling browser history: research events.</div></div></section>
<section class="panel w6"><h2>Canonical checkpoints</h2><div class="body"><div class="row"><select id="checkpointSelect" style="min-width:280px"></select><button id="checkpointRefresh">Refresh list</button><button id="checkpointLoadRun" class="primary">Load & run</button><button id="checkpointLoadPause">Load paused</button></div><pre id="checkpointInfo"></pre></div></section>
<section class="panel w6"><h2>CPU registers</h2><div class="body"><div class="row"><button id="regsA">CPU-A</button><button id="regsB">CPU-B</button></div><pre id="regs"></pre></div></section>
<section class="panel w6"><h2>IOC quick controls</h2><div class="body"><pre id="ports"></pre><div class="row"><input id="pport" value="3" size="3"><input id="pmask" value="20" size="4"><input id="pframes" value="3" size="4"><button id="pulse">Pulse custom</button></div><div class="muted">Known Stage-1 scenario pulses (labels are hardware values, not yet semantic control names):</div><div class="row quick"><button data-pulse="2,10,5">P2:10 pulse x5</button><button data-pulse="3,08,5">P3:08 pulse x5</button><button data-pulse="3,20,20">P3:20 pulse x20</button><button data-pulse="3,01,20">P3:01 pulse x20</button><button data-pulse="12,60,70">PC:60 pulse x70</button><button data-pulse="12,A0,70">PC:A0 pulse x70</button></div><div class="muted">Single-bit discovery on port 3:</div><div class="row quick"><button data-bit="01">01</button><button data-bit="02">02</button><button data-bit="04">04</button><button data-bit="08">08</button><button data-bit="10">10</button><button data-bit="20">20</button><button data-bit="40">40</button><button data-bit="80">80</button></div></div></section>
<section class="panel w6"><h2>Automated IOC bit sweep</h2><div class="body"><div class="row"><label>Port <input id="sweepPort" type="number" min="0" max="15" value="3" style="width:5em"></label><label>Pulse frames <input id="sweepPulse" type="number" min="1" value="8" style="width:6em"></label><label>Observe frames <input id="sweepObserve" type="number" min="1" value="90" style="width:7em"></label></div><div class="row"><input id="sweepMasks" value="01,02,04,08,10,20,40,80" style="flex:1"><button id="sweepRun" class="primary">Run checkpoint-based sweep</button></div><div class="muted">Each mask starts from the selected canonical checkpoint, runs deterministically, captures a screenshot/status/events, then resets. The game timer therefore cannot spoil later candidates.</div><pre id="sweepResult"></pre></div></section>
<section class="panel w6"><h2>Memory inspector / pins</h2><div class="body"><div class="row"><select id="cpu"><option>A</option><option>B</option></select><input id="addr" value="10A096"><select id="width"><option>8</option><option selected>16</option><option>32</option></select><button id="read">Read</button><button id="why">Why?</button><button id="pin">Pin</button></div><pre id="mem"></pre><div id="pins"></div></div></section>
<section class="panel w6"><h2>Snapshots</h2><div class="body"><div class="row"><input id="snap" value="web-mark"><button id="snapSave">Save snapshot</button><button id="snapRestore">Restore</button><button id="snapList">List</button></div><pre id="snaps"></pre></div></section>
<section class="panel w6"><h2>Driving tuning - live</h2><div class="body"><div class="row"><label>Cornering <input id="corner" type="number" min="0" max="4" step="0.01" value="1.00"></label><label>Speed retain <input id="retain" type="number" min="0" max="2" step="0.01" value="1.00"></label><button id="handlingApply">Apply live</button><button id="handlingReset">Restore authentic</button></div><pre id="handling"></pre></div></section>
<section class="panel w6"><h2>Palette Lab - live TC0110PCR</h2><div class="body"><div class="row"><label>View bank <input id="palBank" type="number" min="0" max="255" value="0" style="width:6em"></label><button id="palPrev">Prev</button><button id="palLoad">Load view</button><button id="palNext">Next</button><button id="palClear">Restore all entries</button></div><div class="muted">Bank selector changes the inspected bank only; it does not force a hardware palette bank.</div><div id="palGrid" style="display:grid;grid-template-columns:repeat(8,minmax(0,1fr));gap:4px;margin-top:7px"></div><div class="row" style="margin-top:7px"><input id="palIndex" placeholder="entry 0..4095"><input id="palRaw" placeholder="raw hex e.g. 7FFF"><button id="palFreeze">Freeze entry</button><button id="palRestore">Restore saved original</button></div><pre id="palette"></pre></div></section>
<section class="panel w6"><h2>Controller / Gamepad monitor</h2><div class="body"><div id="gamepad" class="muted">Press a controller button to allow browser gamepad access.</div><pre id="gamepadRaw"></pre><div class="row" style="margin-top:7px"><button id="mapStart">Controller mapping wizard</button><button id="mapCancel" disabled>Cancel</button><button id="gamepadSteeringToggle">Live steering: OFF</button></div><div id="mapPrompt" class="muted">Directional steering now requires the correct sign as well as movement threshold.</div><pre id="mapResult"></pre></div></section>
<section class="panel w6"><h2>IOC held-state controller bridge</h2><div class="body"><div class="row"><input id="holdPort" type="number" min="0" max="15" value="3" style="width:5em"><input id="holdMask" value="00" style="width:5em"><button id="holdSet">Set XOR mask</button><button id="holdClear">Clear all IOC overrides</button></div><pre id="holdState"></pre></div></section>
<section class="panel w6"><h2>Research events</h2><div class="body"><div class="row"><button id="clearEvents">Clear</button></div><pre id="events"></pre></div></section>
<section class="panel w6"><h2>Semantic sprite timeline</h2><div class="body"><pre id="sprites"></pre></div></section>
<section class="panel w6"><h2>Watches & patches</h2><div class="body"><div class="row"><button id="watches">Watches</button><button id="patches">Patches</button></div><pre id="rules"></pre></div></section>
<section class="panel w6"><h2>Session log tail</h2><div class="body"><div class="row"><select id="logSelect" style="min-width:260px"></select><label>Lines <input id="logLines" type="number" value="80" min="10" max="1000" style="width:6em"></label><button id="logRefresh">Refresh</button><label><input id="logAuto" type="checkbox"> Auto</label></div><pre id="logTail">Select a session log.</pre></div></section>
<section class="panel w12" id="registryPanel"><h2>Gameplay Registry</h2><div class="body"><div class="row"><button id="registryRefresh">Refresh now</button><label><input id="registryAuto" type="checkbox" checked> Live refresh</label><span class="muted">Evidence-backed game semantics; confidence and source remain visible.</span></div><div id="registryContext" class="muted" style="margin:8px 0"></div><div id="registryGrid" class="registry-grid"></div></div></section>
<section class="panel w12" id="trackPanel"><h2>Course Mapping / Live Survey</h2><div class="body"><div class="game-lab-grid"><div class="game-lab-card"><h3>Autonomous course follower</h3><div id="courseFollowState" class="state-line">Follower: loading...</div><div class="row"><label>Controller <select id="courseController"><option value="hybrid" selected>hybrid</option><option value="predictive">predictive</option><option value="legacy">legacy</option><option value="profile">profile</option></select></label><label>Bias <input id="courseBias" type="number" min="-8192" max="8192" value="0" style="width:8em"></label><label>Lookahead <input id="courseLookahead" type="number" min="0" max="8" value="8" style="width:6em"></label><label><input id="courseSpeedControl" type="checkbox" checked> Speed control</label></div><div class="row"><button id="courseApply">Apply</button><button id="courseStart" class="primary">Start follower</button><button id="courseStop">Stop follower</button><button id="courseReset">Reset controller</button></div><div class="hint">Live runtime control; no emulator restart required. Bias is a signed target offset from the geometric road centre and is intended for controlled branch surveys.</div></div><div class="game-lab-card"><h3>Survey session</h3><div id="courseSurveyState" class="state-line">Survey: OFF</div><div class="row"><button id="courseSurveyStart" class="primary">Start clean survey</button><button id="courseSurveyStop">Stop / restore</button><button id="trackMapExport">Export mapping dataset</button></div><div class="hint">Start clean survey enables the follower, holds time, suppresses only proven collision response writes while preserving detection, clears the shared recorder, and starts recording. Use Script Console for deterministic long surveys.</div></div></div><div class="row"><button id="trackToggle" class="toggle">Track recording: OFF</button><button id="trackClear">Clear path</button><button id="trackFit">Fit track</button><button id="trackSaveSvg">Save SVG</button><label><input id="trackAutoFit" type="checkbox" checked> Auto fit</label><label><input id="trackShowRoad" type="checkbox" checked> Road edges</label><label><input id="trackShowCarLine" type="checkbox" checked> Car line</label><span id="trackStatus" class="muted">Road geometry and player line are recorded separately; surface and dynamic-object semantics remain evidence scoped.</span></div><div class="row" style="margin:8px 0"><span class="muted">+1 band:</span><span style="color:#60a5fa">&#9632; Negative</span><span style="color:#d1d5db">&#9632; Neutral</span><span style="color:#fb923c">&#9632; Positive</span><span style="color:#facc15">&#9632; Player trajectory</span></div><div class="track-wrap"><svg id="trackSvg" class="track-svg" viewBox="0 0 700 520" preserveAspectRatio="xMidYMid meet"><g id="trackScene"><path id="trackRoadLeft" d="" fill="none" stroke="#94a3b8" stroke-width="1.3" opacity="0.7"/><path id="trackRoadRight" d="" fill="none" stroke="#94a3b8" stroke-width="1.3" opacity="0.7"/><path id="trackPath" d="" fill="none" stroke="#6b7280" stroke-width="1.5" opacity="0.35"/><g id="trackSegments"></g><path id="trackCarLine" d="" fill="none" stroke="#facc15" stroke-width="2.2" opacity="0.95"/><circle id="trackCar" cx="350" cy="470" r="6" fill="#f8fafc"/></g></svg><pre id="trackTelemetry">Track recorder disabled.</pre></div></div></section>
<section class="panel w12" id="experimentalPanel"><h2>Chase H.Q. Experimental / Game Lab</h2><div class="body"><div class="muted">Reversible, research-oriented gameplay controls backed by the same API used by scripts. Values are authoritative emulated state, not screenshot inference.</div><div class="game-lab-grid"><div class="game-lab-card"><h3>Turbo experiments</h3><div id="gameTurboState" class="state-line">Remaining: ? | Active: ? | Timer: ? / 210</div><div class="row"><button id="gameTurboFire" class="primary">Fire turbo</button><button id="gameTurboStock" class="toggle">Infinite stock: OFF</button><button id="gameTurboActive" class="toggle">Infinite active: OFF</button><button id="gameTurboResetTimer">Reset timer</button></div><div class="row"><label>Remaining <input id="gameTurboRemaining" type="number" min="0" max="99" value="3" style="width:6em"></label><button id="gameTurboSetRemaining">Apply count</button></div><div class="hint">Infinite stock freezes CPU-A 0x1003A2. Infinite active freezes the turbo duration timer at 0x100414; use Fire turbo to enter the normal activation path.</div></div><div class="game-lab-card"><h3>Speed experiments</h3><div id="gameSpeedState" class="state-line">HUD speed: ? | Internal physics speed: ?</div><div class="row"><label>Internal physics <input id="gameSpeedValue" type="number" min="0" max="65535" value="602" style="width:8em"></label><button id="gameSpeedSet">Set once</button><button id="gameSpeedFreeze" class="toggle">Freeze internal speed: OFF</button><button id="gameSpeedClear">Clear speed override</button></div><div class="hint"><b>Important:</b> CPU-A 0x10041C is the confirmed raw internal/physics speed. The dashboard/HUD speed is a separate packed-BCD field at 0x100400, so the numeric values are not expected to match. Game Lab mutations preserve the prior RUNNING/PAUSED state; low-level memory.write still pauses by design. The actual top-speed clamp/target remains unresolved.</div></div><div class="game-lab-card"><h3>Target experiments</h3><div id="gameTargetState" class="state-line">Target remaining-hit counter: ?</div><div class="row"><button id="gameTargetOneHit" class="toggle">One-hit Target Kill: OFF</button></div><div class="hint"><b>Stage 1 special target only.</b> Research/cheat intervention. When enabled, the confirmed CPU-A 0x1002AE counter is armed to zero immediately before authentic damage writer PC 0xA112; the game then performs its genuine 0000-&gt;FFFF terminal decrement and PC 0xA118 defeat path. It does not fake the defeat state.</div></div><div class="game-lab-card"><h3>Sprite ordering experiment</h3><div id="gameSpriteOrderState" class="state-line">Same-priority tie-break: ?</div><div class="row"><button id="gameSpriteHigher" class="toggle">Ascending (higher-slot)</button><button id="gameSpriteLower" class="toggle">Descending (lower-slot)</button><button id="gameSpriteOrderInspect">Inspect order</button></div><div class="hint">Reversible compositor experiment for same-priority sprites. Default traversal is descending. Each nontransparent candidate reserves its pixel; subsequent entries cannot overwrite it. Legacy mode names select traversal, not the winner. Use captured timelines for A/B comparison.</div></div><div class="game-lab-card"><h3>Semantic IOC controls</h3><div class="row"><button data-sem-pulse="01">Turbo pulse</button><button data-sem-pulse="02">TILT pulse</button><button data-sem-pulse="10">Brake pulse</button><button data-sem-pulse="20">Accelerator pulse</button></div><div class="row"><label><input type="checkbox" data-sem-hold="01"> Hold Turbo</label><label><input type="checkbox" data-sem-hold="10"> Hold Brake</label><label><input type="checkbox" data-sem-hold="20"> Hold Accelerator</label></div><div class="row"><button id="semClear">Clear semantic holds</button></div><div class="hint">Semantic holds own the debugger XOR layer on IOC P3 while enabled.</div></div><div class="game-lab-card"><h3>Deterministic execution / disassembly</h3><div class="row"><label>Frames <input id="runFramesCount" type="number" value="60" min="1" max="100000" style="width:8em"></label><button id="runFrames">Run exact frames</button></div><div class="row"><label>CPU <select id="disCpu"><option>A</option><option>B</option></select></label><input id="disAddr" value="008460"><input id="disCount" type="number" value="32" min="1" max="256" style="width:6em"><button id="disRun">Disassemble</button></div></div></div><pre id="experimentalResult"></pre><h3>Live IOC layers</h3><div id="iocLiveTable"></div></div></section>
<section class="panel w12" id="tileInspectorPanel"><h2>TC0100SCN Character Inspector</h2><div class="body"><div class="tile-inspector-grid"><div><div class="row"><label>Text character <input id="tileInspectCode" value="0C" style="width:7em"></label><label>Palette bank <input id="tileInspectPalette" type="number" min="0" max="255" value="0" style="width:7em"></label><label>Scale <select id="tileInspectScale"><option>4</option><option selected>8</option><option>16</option><option>24</option></select></label><button id="tileInspectRun" class="primary">Inspect</button></div><div class="row"><button data-tile-preset="7C,0">Normal turbo 7C</button><button data-tile-preset="7D,0">7D</button><button data-tile-preset="7E,0">7E</button><button data-tile-preset="7F,0">7F</button><button data-tile-preset="0C,0">Active turbo 0C</button><button data-tile-preset="0D,0">0D</button><button data-tile-preset="0E,0">0E</button><button data-tile-preset="0F,0">0F</button></div><div class="muted">Current implementation inspects TC0100SCN RAM text characters (256 x 8x8, 2bpp) from tilemap character RAM at 0xC06000. PNG, raw pen matrix and JSON metadata are exported as artifacts. Background ROM-tile inspection remains a future extension.</div></div><div><img id="tileInspectPreview" class="tile-preview" alt="TC0100SCN tile preview" style="display:none"><pre id="tileInspectResult">No character inspected yet.</pre></div></div></div></section>
<section class="panel w12" id="nextPanel"><h2>Next Steps</h2><div class="body"><div class="muted">High-level diagnosis still outstanding; detailed evidence remains in the Registry, Experimental tools and project docs.</div><div id="nextSteps"></div></div></section>
<section class="panel w12" id="diagnosticsPanel"><h2>Logs / Diagnostics</h2><div class="body"><div class="row"><span id="healthNative" class="badge">Native: ?</span><span id="healthWeb" class="badge">Web: OK</span><span id="lastApi" class="badge">Last API: ?</span><button id="diagRefresh">Refresh</button><label><input id="diagAuto" type="checkbox" checked> Live tail</label><button id="diagCopy">Copy log</button></div><div class="row"><select id="diagLogSelect" style="min-width:320px"></select><label>Lines <input id="diagLines" type="number" value="150" min="20" max="2000" style="width:7em"></label></div><pre id="diagRequests">No request timings yet.</pre><pre id="diagTail">Select a log.</pre></div></section>
<section class="panel w12" id="spriteMonitorPanel"><h2>Live Sprite Monitor</h2><div class="body"><div class="row"><label>Filter <input id="spriteFilter" placeholder="slot/map/palette/position"></label><label><input id="spriteChangedOnly" type="checkbox"> changed only</label><label><input id="spriteAuto" type="checkbox" checked> auto refresh</label><button id="spriteRefresh">Refresh now</button><button id="spriteClearHistory">Clear history</button><span id="spriteSummary" class="muted"></span></div><div class="row"><b>SDL live preview</b><label>Slot <input id="spriteLiveSlot" type="number" min="0" value="44" style="width:6em"></label><label>Map <input id="spriteLiveMap" type="number" min="0" placeholder="keep" style="width:7em"></label><label>Palette <input id="spriteLivePalette" type="number" min="0" placeholder="keep" style="width:7em"></label><button id="spriteLiveApply">Apply override</button><button id="spriteLiveInspect">Inspect</button><button id="spriteLiveSolo">Solo</button><button id="spriteLiveHide">Hide</button><button id="spriteLiveFlash">Flash</button><button id="spriteLiveClear">Clear preview</button></div><div class="muted" style="margin-bottom:7px">Overrides are presentation-only: SDL renders the actual emulated scene with the selected sprite map/palette/visibility modified. Solo/hide/flash can be eyeballed live without changing game RAM. Click a row to select its slot.</div><div style="overflow:auto;max-height:520px"><table class="sprite-table"><thead><tr><th>Slot</th><th>Frame</th><th>Map</th><th>Palette</th><th>X</th><th>Y</th><th>Changes</th></tr></thead><tbody id="spriteRows"></tbody></table></div><pre id="spriteHistory" style="margin-top:8px;max-height:180px">Click a sprite row to inspect its recent history.</pre></div></section>
<section class="panel w12" id="frameSnapshotPanel"><h2>Structured Frame Snapshot</h2><div class="body"><div class="row"><input id="frameSnapshotName" placeholder="optional snapshot name" style="min-width:260px"><button id="frameSnapshotCapture" class="primary">Capture layered snapshot</button><button id="frameSnapshotList">List snapshots</button><select id="frameSnapshotSelect" style="min-width:240px"></select><select id="frameSnapshotMode"><option value="final.png">Normal</option><option value="final-no-hud.png">HUD hidden</option><option value="hud-only.png">HUD only</option><option value="layers">Layer reconstruction</option><option value="sprites">Individual sprites</option></select><button id="frameSnapshotView">View</button></div><div class="muted">v2 snapshots contain exact alpha contribution layers, raw hardware-source PNGs, reconstruction metadata, semantic sprites, and source-aware HUD variants.</div><div class="graphics-grid" style="margin-top:8px"><div><canvas id="frameSnapshotPreview" class="preview" width="320" height="240"></canvas><div id="frameSnapshotLayerToggles" style="margin-top:7px"></div></div><pre id="frameSnapshotResult">No structured snapshot captured yet.</pre></div></div></section>
<section class="panel w12" id="imageAnalysisPanel"><h2>Frame / Image Analysis</h2><div class="body"><div class="row"><input id="imgA" placeholder="baseline frame name"><input id="imgB" placeholder="test frame name"><select id="imgRegion"><option>FULL</option><option>HUD_TOP</option><option>ROAD_AREA</option><option>PLAYER_CAR</option><option>HUD_TURBO</option><option>TARGET_AREA</option></select><select id="imgHud"><option value="normal">HUD normal</option><option value="hidden">Ignore HUD</option><option value="only">HUD only</option></select><button id="imgCompare">Compare</button><input id="imgDiffName" placeholder="diff output name"><button id="imgDiff">Create diff PNG</button></div><div class="muted">HUD-aware comparison uses source-isolated structured snapshots; <b>Ignore HUD</b> does not mask a fixed rectangle.</div><pre id="imgResult"></pre></div></section>
<section class="panel w12" id="evidencePanel"><h2>Evidence Viewer</h2><div class="body"><div class="row"><button id="evidenceRefresh">Refresh captures</button><select id="evidenceSelect" style="min-width:300px"></select><button id="evidenceView">View</button><button id="evidenceLoad">Load state paused</button><button id="evidenceZip">Create ZIP</button><a id="evidenceDownload" href="#" style="display:none"><button type="button">Download selected ZIP</button></a></div><div id="evidenceSelected" class="muted" style="margin-bottom:7px">Selected: none</div><div class="row"><label>Bundle prefix <input id="evidenceBundlePrefix" value="ioc-p3-" style="min-width:180px"></label><input id="evidenceBundleName" placeholder="optional bundle name" style="min-width:180px"><button id="evidenceBundle">Create experiment bundle</button><a id="evidenceBundleDownload" href="#" style="display:none"><button type="button">Download bundle ZIP</button></a></div><div class="muted" style="margin-bottom:7px">Click a capture card to select it. Captures with a ready ZIP can be downloaded immediately; prefix bundles package a whole experiment in one ZIP.</div><div id="evidenceList" class="evidence-list"></div><img id="evidenceShot" class="evidence-shot" alt="Evidence screenshot" style="display:none;margin-top:8px"><pre id="evidenceDetail">No evidence selected.</pre></div></section>
<section class="panel w12" id="scriptPanel"><h2>Research Script Console</h2><div class="body"><div class="script-shell" id="scriptShell"><div><div class="row"><b>Script library</b><label>Category <select id="scriptLibraryCategory" style="min-width:150px"><option value="">All categories</option></select></label><label>Version <select id="scriptLibraryVersion" style="min-width:130px"><option value="">All versions</option></select></label><label>Regression view <select id="scriptLibraryScope" style="min-width:130px"><option value="current">Current build</option><option value="permanent">Permanent</option><option value="all">All</option></select></label><label>Search <input id="scriptLibrarySearch" placeholder="name, purpose or path" style="min-width:210px"></label><button id="scriptLibraryRefresh">Refresh library</button></div><div class="row" style="margin-top:6px"><select id="scriptLibrarySelect" style="min-width:520px;max-width:100%"></select><button id="scriptLibraryLoad">Reload</button><input id="scriptLibraryName" placeholder="category/my-script.chqscript" style="min-width:240px"><button id="scriptLibrarySave">Save as</button><button id="scriptHelp">Syntax help</button></div><div id="scriptLibraryInfo" class="muted" style="margin:6px 0 8px">Scripts are grouped by category. Selecting one auto-loads it.</div><textarea id="scriptText" spellcheck="false"></textarea><div class="row" style="margin-top:7px"><button id="scriptValidate">Validate</button><button id="scriptRun" class="primary">Run script</button><button id="scriptRunSelection">Run selection</button><button id="scriptRunLine">Run current line</button><button id="scriptCancel" disabled>Stop script</button><button id="scriptForceReset" title="Reset the browser-side script runner if it becomes stuck">Force reset runner</button><span id="scriptState" class="badge">Script: IDLE</span><span id="scriptRunnerHealth" class="badge ok">Runner: IDLE</span><label><input id="scriptStop" type="checkbox" checked> Stop on error</label></div></div><div class="script-output-wrap" id="scriptOutputWrap"><div class="script-output-toolbar"><b>Run output</b><button id="scriptCopy">Copy output</button><button id="scriptClear">Clear output</button><button id="scriptDownload">Download output</button><label><input id="scriptWrap" type="checkbox" checked> Wrap lines</label><label><input id="scriptThumbs" type="checkbox"> Show thumbnails</label><button id="scriptMax">Maximize output</button></div><pre id="scriptProgress"></pre><div id="scriptResult" class="script-output wrap"></div><div id="scriptArtifacts" class="artifact-grid"></div><div class="script-history"><div class="row"><b>Recent script runs</b><label>Version <select id="scriptHistoryVersion"><option value="">All versions</option></select></label><label>Status <select id="scriptHistoryStatus"><option value="">All statuses</option><option value="PASS">PASS</option><option value="FAIL">FAIL</option><option value="CANCELLED">CANCELLED</option></select></label><label>Session <select id="scriptHistorySession"><option value="">All sessions</option></select></label><button id="scriptHistoryRefresh">Refresh history</button><button id="scriptHistoryCurrent">Current session</button><button id="scriptHistoryClear">Clear local history</button></div><div id="scriptHistory"></div><div id="scriptHistoryActions" class="history-actions"><span class="muted">Select a run to view actions.</span></div></div></div></div></div></section>
<section class="panel w12" id="docsKnowledgePanel"><h2>Docs / Knowledge</h2><div class="body"><div class="docs-checker"></div><div class="docs-modebar"><button class="on" data-docs-mode="handbook">Handbook</button><button data-docs-mode="api">API Reference</button><button data-docs-mode="script">Scripting Reference</button><button data-docs-mode="knowledge">Project Knowledge</button><button data-docs-mode="glossary">Glossary</button><span class="docs-search-tip">User manual + generated live references + evidence-backed project knowledge</span></div><div class="row"><input id="docsSearch" placeholder="Search handbook, API actions, scripting, findings and glossary..." style="min-width:360px;flex:1"><button id="docsRefresh">Refresh</button><button id="docsCopyHandover">Copy ChatGPT handover</button></div><div class="row"><label>Evidence in export <select id="docsExportEvidence"><option value="none">None</option><option value="key" selected>Key evidence</option><option value="full">Full evidence</option></select></label><button id="docsExportHtml">Export full handbook HTML</button><button id="docsExportPdf">Export full handbook PDF</button><span id="docsExportResult" class="muted"></span></div><div class="docs-shell"><div id="docsList" class="docs-list"></div><article id="docsDetail" class="docs-detail"><h2 class="docs-title">CHASE H.Q. NATIVE USER & RESEARCH HANDBOOK</h2><p class="muted">Loading user documentation and live references...</p></article></div></div></section>
<section class="panel w12"><h2>API discovery</h2><div class="body"><div class="row"><a href="/api/docs/" target="_blank"><button type="button">Interactive API docs</button></a><a href="/api/v1/openapi.json" target="_blank"><button type="button">OpenAPI JSON</button></a><a href="/api/health" target="_blank"><button type="button">Health</button></a></div></div></section>
<section class="panel w12"><h2>Command console</h2><div class="body"><div class="row"><input id="command" style="flex:1" placeholder="e.g. changed A 100000:1004FF since 9500"><button id="runCmd">Run safe command</button></div><pre id="console">Web console is restricted to debugger/research commands exposed by the bridge.</pre></div></section>
</div>
<div id="imageModal" class="img-modal"><div class="img-modal-card"><div class="img-modal-toolbar"><b id="imageModalName">Image</b><button id="imageModalPrev">Previous</button><button id="imageModalNext">Next</button><a id="imageModalDownload" href="#"><button type="button">Download</button></a><button id="imageModalClose">Close</button></div><img id="imageModalImg" alt="Artifact preview"></div></div>
<script>
const $=id=>document.getElementById(id),H=[],pins=[],requestHistory=[];let timer,previewTimer,logTimer,currentGamepad=null,mapState=null,timerHeld=false,topHeld=false,currentTab='dashboard',semanticHoldMask=0,gamepadSteeringEnabled=false,lastGamepadSteer=null,lastGamepadSteerAt=0,trackRecording=false,trackSamples=[],refreshBusy=false,lastPaused=null,scriptAbort=false,scriptRunning=false,scriptPlainOutput='',modalImages=[],modalIndex=0,spriteState=new Map(),spriteHistoryBySlot=new Map(),scriptPhase='IDLE',scriptLastProgressAt=Date.now(),scriptLastContext='',scriptLastFault='';const SCRIPT_WATCHDOG_MS=30000;
function formatUptime(sec){sec=Math.max(0,Math.floor(Number(sec)||0));const d=Math.floor(sec/86400);sec%=86400;const h=Math.floor(sec/3600);sec%=3600;const m=Math.floor(sec/60),ss=sec%60;return (d?d+'d ':'')+String(h).padStart(2,'0')+':'+String(m).padStart(2,'0')+':'+String(ss).padStart(2,'0')}
async function api(path,opt={}){const started=performance.now(),ctrl=new AbortController(),longOp=/^\/api\/v1\/(script|control\/run-frames|checkpoints\/load|input\/ioc\/sweep)/i.test(path),timeoutMs=opt.timeoutMs??(longOp?3700000:10000),fetchOpt={...opt};delete fetchOpt.timeoutMs;const timeout=setTimeout(()=>ctrl.abort('Request timed out after '+Math.round(timeoutMs/1000)+'s: '+path),timeoutMs);try{const r=await fetch(path,{...fetchOpt,signal:ctrl.signal});const t=await r.text();const ms=Math.round(performance.now()-started);requestHistory.push({path,ms,ok:r.ok,at:new Date().toLocaleTimeString(undefined,{hour12:false,hour:'2-digit',minute:'2-digit',second:'2-digit',fractionalSecondDigits:3})});if(requestHistory.length>40)requestHistory.shift();if($('lastApi'))$('lastApi').textContent=`Last API: ${ms} ms`;if(!r.ok)throw Error(t);return t}catch(e){const ms=Math.round(performance.now()-started);const msg=(e?.name==='AbortError'||String(e?.message||'').includes('aborted'))?('Request timed out after '+Math.round(timeoutMs/1000)+'s: '+path):String(e?.message||e);requestHistory.push({path,ms,ok:false,at:new Date().toLocaleTimeString(undefined,{hour12:false,hour:'2-digit',minute:'2-digit',second:'2-digit',fractionalSecondDigits:3}),error:msg});if(requestHistory.length>40)requestHistory.shift();throw Error(msg)}finally{clearTimeout(timeout)}}
function parseJsonResponse(raw,context){const t=String(raw??'').trim();if(!t)throw Error(`${context}: empty JSON response`);try{return JSON.parse(t)}catch(e){const sample=t.length>240?t.slice(0,240)+'...':t;throw Error(`${context}: invalid/truncated JSON (${e.message}); raw=${sample}`)}}
const tabGroups={
 dashboard:['Current SDL frame','SDL Window Controls','Machine status','Canonical checkpoints','CPU registers','Memory inspector / pins','Snapshots','Research events','Semantic sprite timeline','Watches & patches','Session log tail','API discovery','Command console'],
 script:['Research Script Console'],registry:['Gameplay Registry'],track:['Live SVG Track View'],graphics:['Live Sprite Monitor','Structured Frame Snapshot'],docs:['Docs / Knowledge'],
 experimental:['IOC quick controls','Automated IOC bit sweep','Driving tuning - live','Palette Lab - live TC0110PCR','Controller / Gamepad monitor','IOC held-state controller bridge','Chase H.Q. Experimental / Game Lab'],
 evidence:['Frame / Image Analysis','Evidence Viewer'],next:['Next Steps'],diagnostics:['Logs / Diagnostics']};
function setTab(name){currentTab=name;document.querySelectorAll('.tabbtn').forEach(b=>b.classList.toggle('active',b.dataset.tab===name));document.querySelectorAll('section.panel').forEach(sec=>{const title=sec.querySelector('h2')?.textContent||'';sec.classList.toggle('tab-hidden',!(tabGroups[name]||[]).includes(title))});if(name==='registry')loadRegistry();if(name==='next')loadNextSteps();if(name==='track'){refreshTrackRecorder();if(trackRecording)sampleTrack();}if(name==='graphics'){refreshSpriteMonitor();loadFrameSnapshots(false);}if(name==='docs')loadDocsKnowledge();if(name==='experimental')refreshIocTable();if(name==='diagnostics')refreshDiagnostics();}
document.querySelectorAll('.tabbtn').forEach(b=>b.onclick=()=>setTab(b.dataset.tab));
function cmd(c){return api('/api/cmd',{method:'POST',headers:{'Content-Type':'text/plain'},body:c})}
function parseStatus(s){let o={};for(const m of s.matchAll(/([A-Za-z_][A-Za-z0-9_.-]*)=([^\s]+)/g))o[m[1]]=m[2];return o}
async function refresh(){if(refreshBusy)return;if(scriptRunning){$('conn').textContent='BUSY';$('conn').className='badge warn';return}refreshBusy=true;try{try{const h=JSON.parse(await api('/api/health'));$('uptime').textContent='uptime '+formatUptime(h.uptimeSeconds)}catch(_){$('uptime').textContent='uptime ?'}const s=await api('/api/status'),p=await api('/api/input/ports'),e=await api('/api/events'),sp=await api('/api/sprites');$('status').textContent=s;$('ports').textContent=p;$('events').textContent=e;$('sprites').textContent=sp;const st=parseStatus(s);$('sdlFullscreen').dataset.on=st.fullscreen;$('sdlTop').dataset.on=st.always_on_top;$('sdlFullscreen').textContent='Fullscreen: '+(st.fullscreen==='1'?'ON':'OFF');$('sdlTop').textContent='Always on top: '+(st.always_on_top==='1'?'ON':'OFF');if(st.window_scale)$('sdlScale').value=st.window_scale;$('frame').textContent='frame '+(st.frame||'?');$('conn').textContent=st.paused==='1'?'PAUSED':'RUNNING';$('conn').className='badge ok';lastPaused=st.paused==='1';$('runPauseToggle').textContent=lastPaused?'Emulation: PAUSED':'Emulation: RUNNING';$('runPauseToggle').classList.toggle('on',lastPaused);timerHeld=st.timer_frozen==='1';$('timerToggle').textContent=timerHeld?'Timer hold: ON':'Timer hold: OFF';$('timerToggle').classList.toggle('on',timerHeld);topHeld=st.always_on_top==='1';$('topToggle').textContent=topHeld?'SDL top: ON':'SDL top: OFF';$('topToggle').classList.toggle('on',topHeld);H.push({f:+(st.frame||0),e:+(st.events||0)});if(H.length>120)H.shift();draw();await refreshPins();if(currentTab==='registry'&&$('registryAuto')?.checked)loadRegistry();if(currentTab==='track'&&trackRecording)sampleTrack();if(currentTab==='graphics'&&$('spriteAuto')?.checked)refreshSpriteMonitor(sp);if(currentTab==='experimental')refreshIocTable()}catch(x){$('conn').textContent='DEGRADED';$('conn').className='badge warn';$('status').textContent='Web frontend responsive; native/backend request failed: '+x.message}finally{refreshBusy=false}}
function draw(){const c=$('hist'),x=c.getContext('2d'),w=c.width,h=c.height;x.clearRect(0,0,w,h);if(H.length<2)return;const vals=H.map(v=>v.e),mx=Math.max(1,...vals);x.strokeStyle='#7aa2f7';x.beginPath();H.forEach((v,i)=>{const px=i*w/(H.length-1),py=h-(v.e/mx)*(h-4)-2;i?x.lineTo(px,py):x.moveTo(px,py)});x.stroke()}
async function refreshPins(){$('pins').innerHTML='';for(const p of pins){let v='';try{v=await cmd(`read ${p.cpu} ${p.addr} ${p.width}`)}catch(e){v=e.message}const d=document.createElement('div');d.className='pin';const pre=document.createElement('pre');pre.textContent=v;const b=document.createElement('button');b.textContent='x';b.onclick=()=>{pins.splice(pins.indexOf(p),1);refreshPins()};d.append(pre,b);$('pins').append(d)}}
function restart(){clearInterval(timer);timer=setInterval(refresh,+$('rate').value)}
document.querySelectorAll('[data-cmd]').forEach(b=>b.onclick=async()=>{await cmd(b.dataset.cmd);refresh()});$('runPauseToggle').onclick=async()=>{try{await cmd(lastPaused?'resume':'pause');await refresh()}catch(e){$('console').textContent=e.message}};
$('timerToggle').onclick=async()=>{$('console').textContent=await cmd(timerHeld?'timer resume':'timer freeze');await refresh()};$('topToggle').onclick=async()=>{$('console').textContent=await cmd(topHeld?'window top off':'window top on');await refresh()};
async function sdlControl(command){try{$('console').textContent=await cmd(command);await refresh();const st=parseStatus(await cmd('window status'));$('sdlFullscreen').textContent='Fullscreen: '+(st.fullscreen==='1'?'ON':'OFF');$('sdlFullscreen').dataset.on=st.fullscreen;$('sdlTop').textContent='Always on top: '+(st.always_on_top==='1'?'ON':'OFF');$('sdlTop').dataset.on=st.always_on_top;if(st.scale)$('sdlScale').value=st.scale;$('sdlWindowState').textContent='visible='+st.visible+' minimized='+st.minimized+' scale='+st.scale}catch(e){$('console').textContent='SDL control failed: '+e.message}}
for(const [id,command] of Object.entries({sdlShow:'window show',sdlHide:'window hide',sdlMinimize:'window minimize',sdlRestore:'window restore',sdlPause:'pause',sdlResume:'resume',sdlStep:'step frame 1'}))$(id).onclick=()=>sdlControl(command);
$('sdlFullscreen').onclick=()=>sdlControl('window fullscreen '+($('sdlFullscreen').dataset.on==='1'?'off':'on'));
$('sdlTop').onclick=()=>sdlControl('window top '+($('sdlTop').dataset.on==='1'?'off':'on'));
$('sdlSnapshot').onclick=async()=>{try{const result=await api('/api/v1/frame/snapshot',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({name:'window-frame-'+Date.now()})});$('console').textContent=result;await loadFrameSnapshots(false)}catch(e){$('console').textContent=e.message}};
$('sdlScale').onchange=()=>sdlControl('window scale '+$('sdlScale').value);
$('regsA').onclick=async()=>$('regs').textContent=await cmd('regs A');$('regsB').onclick=async()=>$('regs').textContent=await cmd('regs B');
$('read').onclick=async()=>$('mem').textContent=await cmd(`read ${$('cpu').value} ${$('addr').value} ${$('width').value}`);$('why').onclick=async()=>$('mem').textContent=await cmd(`why mem ${$('cpu').value} ${$('addr').value}`);$('pin').onclick=()=>{pins.push({cpu:$('cpu').value,addr:$('addr').value,width:$('width').value});refreshPins()};
$('pulse').onclick=async()=>$('console').textContent=await cmd(`input pulse ${$('pport').value} ${$('pmask').value} ${$('pframes').value}`);document.querySelectorAll('[data-pulse]').forEach(b=>b.onclick=async()=>{const [p,m,f]=b.dataset.pulse.split(',');$('console').textContent=await cmd(`input pulse ${p} ${m} ${f}`)});document.querySelectorAll('[data-bit]').forEach(b=>b.onclick=async()=>{$('pport').value='3';$('pmask').value=b.dataset.bit;$('pframes').value='8';$('console').textContent=await cmd(`input pulse 3 ${b.dataset.bit} 8`)});
$('clearEvents').onclick=async()=>{await cmd('events clear');refresh()};$('watches').onclick=async()=>$('rules').textContent=await cmd('watch list');$('patches').onclick=async()=>$('rules').textContent=await cmd('patch list');
$('snapSave').onclick=async()=>$('snaps').textContent=await cmd('snapshot save '+$('snap').value);$('snapRestore').onclick=async()=>$('snaps').textContent=await cmd('snapshot restore '+$('snap').value);$('snapList').onclick=async()=>$('snaps').textContent=await cmd('snapshot list');
$('runCmd').onclick=async()=>{try{$('console').textContent=await cmd($('command').value)}catch(e){$('console').textContent=e.message}};$('command').addEventListener('keydown',e=>{if(e.key==='Enter')$('runCmd').click()});
async function loadHandling(){const t=await cmd('handling get');$('handling').textContent=t;const q=parseStatus(t);if(q.cornering_scale)$('corner').value=q.cornering_scale;if(q.speed_retain)$('retain').value=q.speed_retain}
$('handlingApply').onclick=async()=>{$('handling').textContent=await cmd(`handling set ${$('corner').value} ${$('retain').value}`)};$('handlingReset').onclick=async()=>{$('handling').textContent=await cmd('handling reset');await loadHandling()};
function palColor(raw){let v=parseInt(raw,16)||0;let e=x=>((x&31)<<3)|((x&31)>>2);return `rgb(${e(v)},${e(v>>5)},${e(v>>10)})`}
async function loadPalette(){let b=Math.max(0,Math.min(255,+$('palBank').value||0));$('palBank').value=b;let t=await cmd('palette bank '+b);$('palette').textContent=t;let q=parseStatus(t),g=$('palGrid');g.innerHTML='';for(let i=0;i<16;i++){let raw=(q['pen'+i]||'0x0').replace(/^0x/i,'');let d=document.createElement('button');d.type='button';d.style.background=palColor(raw);d.style.minHeight='42px';d.style.color='#fff';d.style.textShadow='0 1px 2px #000';d.textContent=`${i}: ${raw}`;d.onclick=()=>{$('palIndex').value=b*16+i;$('palRaw').value=raw};g.appendChild(d)}}
$('palLoad').onclick=loadPalette;$('palPrev').onclick=()=>{$('palBank').value=Math.max(0,+$('palBank').value-1);loadPalette()};$('palNext').onclick=()=>{$('palBank').value=Math.min(255,+$('palBank').value+1);loadPalette()};$('palFreeze').onclick=async()=>{$('palette').textContent=await cmd(`palette freeze ${$('palIndex').value} ${$('palRaw').value}`);loadPalette()};$('palRestore').onclick=async()=>{$('palette').textContent=await cmd(`palette restore ${$('palIndex').value}`);loadPalette()};$('palClear').onclick=async()=>{$('palette').textContent=await cmd('palette clear');loadPalette()};
$('holdSet').onclick=async()=>{$('holdState').textContent=await cmd(`input xor set ${$('holdPort').value} ${$('holdMask').value}`)};$('holdClear').onclick=async()=>{$('holdState').textContent=await cmd('input xor clear')};
function gamepadSnapshot(g){return {axes:g.axes.map(Number),buttons:g.buttons.map(b=>({value:+b.value,pressed:!!b.pressed}))}}
function detectMappingInput(g,base,action){const snap=gamepadSnapshot(g);let best=null;if(action==='steering-left'||action==='steering-right'){const want=action==='steering-left'?-1:1;for(let i=0;i<snap.axes.length;i++){const v=snap.axes[i],d=v-(base.axes[i]||0);if((want<0?v<=-.55:v>=.55)&&Math.abs(d)>.35&&(!best||Math.abs(v)>Math.abs(best.value)))best={kind:'axis',index:i,value:v,delta:d}}}else{for(let i=0;i<snap.buttons.length;i++){const b=snap.buttons[i],bb=base.buttons[i]||{value:0,pressed:false};const d=b.value-bb.value;if((b.pressed||b.value>.55)&&d>.35)best={kind:'button',index:i,value:b.value,pressed:b.pressed}}if(!best){for(let i=0;i<snap.axes.length;i++){const d=snap.axes[i]-(base.axes[i]||0);if(Math.abs(d)>.65&&(!best||Math.abs(d)>Math.abs(best.delta||0)))best={kind:'axis',index:i,value:snap.axes[i],delta:d}}}}return best}
const mapActions=['steering-left','steering-right','accelerator','brake','turbo','start','credit'];function mapPrompt(){if(!mapState)return;const a=mapActions[mapState.step];$('mapPrompt').textContent=`[${mapState.step+1}/${mapActions.length}] Move/press ONLY: ${a.toUpperCase().replace('-',' ')}. Hold until detected.`}
$('mapStart').onclick=()=>{if(!currentGamepad){$('mapResult').textContent='No controller detected yet.';return}mapState={step:0,controllerId:currentGamepad.id,started:new Date().toISOString(),mapping:{},samples:[],base:gamepadSnapshot(currentGamepad),armedAt:performance.now()+500};$('mapStart').disabled=true;$('mapCancel').disabled=false;$('mapResult').textContent='';mapPrompt()};$('mapCancel').onclick=()=>{mapState=null;$('mapStart').disabled=false;$('mapCancel').disabled=true;$('mapPrompt').textContent='Mapping cancelled.'};
async function finishMapping(){const payload={schema:'chq-controller-map-v1',controllerId:mapState.controllerId,created:new Date().toISOString(),mapping:mapState.mapping,samples:mapState.samples};try{const r=await api('/api/v1/controller/mapping',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(payload)});const j=JSON.parse(r);$('mapResult').textContent=`Saved mapping: ${j.path}\n`+JSON.stringify(payload,null,2)}catch(e){$('mapResult').textContent='Mapping save failed: '+e.message}mapState=null;$('mapStart').disabled=false;$('mapCancel').disabled=true;$('mapPrompt').textContent='Mapping complete. Review the saved values before binding to IOC.'}
function steeringRaw12FromAxis(v){v=Math.max(-1,Math.min(1,Number(v)||0));if(Math.abs(v)<0.12)v=0;const signed=Math.round(v*96);return signed<0?(0x1000+signed)&0x0FFF:signed&0x0FFF}
$('gamepadSteeringToggle').onclick=async()=>{gamepadSteeringEnabled=!gamepadSteeringEnabled;$('gamepadSteeringToggle').textContent='Live steering: '+(gamepadSteeringEnabled?'ON':'OFF');$('gamepadSteeringToggle').classList.toggle('on',gamepadSteeringEnabled);if(!gamepadSteeringEnabled){lastGamepadSteer=0;try{await cmd('input steering set 0000')}catch(_){}}};
function pollGamepad(){const gs=navigator.getGamepads?navigator.getGamepads():[],g=[...gs].find(Boolean);currentGamepad=g||null;if(g){if(gamepadSteeringEnabled&&g.axes.length){const raw=steeringRaw12FromAxis(g.axes[0]),now=performance.now();if(raw!==lastGamepadSteer&&now-lastGamepadSteerAt>35){lastGamepadSteer=raw;lastGamepadSteerAt=now;cmd('input steering set '+raw.toString(16).toUpperCase().padStart(4,'0')).catch(()=>{})}}$('gamepad').textContent=g.id;$('gamepadRaw').textContent='axes: '+g.axes.map((v,i)=>`${i}=${v.toFixed(3)}`).join('  ')+'\nbuttons: '+g.buttons.map((b,i)=>b.pressed||b.value?`${i}=${b.value.toFixed(2)}${b.pressed?'*':''}`:null).filter(Boolean).join('  ');if(mapState&&performance.now()>=mapState.armedAt){const a=mapActions[mapState.step],hit=detectMappingInput(g,mapState.base,a);if(hit){mapState.mapping[a]=hit;mapState.samples.push({action:a,detected:hit,at:new Date().toISOString(),raw:gamepadSnapshot(g)});mapState.step++;if(mapState.step>=mapActions.length)finishMapping();else{mapState.base=gamepadSnapshot(g);mapState.armedAt=performance.now()+700;mapPrompt()}}}}requestAnimationFrame(pollGamepad)}
function refreshPreview(){if(scriptRunning)return;const img=$('preview');img.src='/api/v1/frame.png?t='+Date.now();img.onload=()=>{$('previewStamp').textContent='updated '+new Date().toLocaleTimeString()};img.onerror=()=>{$('previewStamp').textContent='preview unavailable'}}function restartPreview(){clearInterval(previewTimer);const n=+$('previewRate').value;if(n>0)previewTimer=setInterval(refreshPreview,n)}$('previewNow').onclick=refreshPreview;$('previewRate').onchange=restartPreview;
async function loadCheckpoints(){try{const j=JSON.parse(await api('/api/v1/checkpoints'));const sel=$('checkpointSelect');sel.innerHTML='';for(const c of j.checkpoints){const o=document.createElement('option');o.value=c.file;o.textContent=(c.title||c.file)+(c.role?` | ${c.role}`:'');sel.appendChild(o)}$('checkpointInfo').textContent=j.checkpoints.map(c=>`${c.file}\n  ${c.role||''} ${c.description||''}`).join('\n')}catch(e){$('checkpointInfo').textContent=e.message}}async function loadSelected(run){const file=$('checkpointSelect').value;if(!file)return;try{const j=JSON.parse(await api('/api/v1/checkpoints/load',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({file,run})}));$('checkpointInfo').textContent=JSON.stringify(j,null,2);refreshPreview();refresh()}catch(e){$('checkpointInfo').textContent=e.message}}$('checkpointRefresh').onclick=loadCheckpoints;$('checkpointLoadRun').onclick=()=>loadSelected(true);$('checkpointLoadPause').onclick=()=>loadSelected(false);
$('sweepRun').onclick=async()=>{if(!$('checkpointSelect').value){$('sweepResult').textContent='Select a checkpoint first.';return}const body={checkpoint:$('checkpointSelect').value,port:+$('sweepPort').value,masks:$('sweepMasks').value.split(',').map(x=>x.trim()).filter(Boolean),pulseFrames:+$('sweepPulse').value,observeFrames:+$('sweepObserve').value};$('sweepRun').disabled=true;$('sweepResult').textContent='Running deterministic IOC sweep...';try{const j=JSON.parse(await api('/api/v1/input/ioc/sweep',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)}));$('sweepResult').textContent=`Completed: ${j.path}\n`+j.results.map(r=>`${r.mask}: frame ${r.frame} screenshot=${r.screenshot}`).join('\n');refreshPreview()}catch(e){$('sweepResult').textContent='Sweep failed: '+e.message}finally{$('sweepRun').disabled=false}};
async function loadLogs(){try{const j=JSON.parse(await api('/api/v1/logs'));const sel=$('logSelect'),old=sel.value;sel.innerHTML='';for(const f of j.files){const o=document.createElement('option');o.value=f;o.textContent=f;sel.appendChild(o)}if([...sel.options].some(o=>o.value===old))sel.value=old}catch(e){$('logTail').textContent=e.message}}async function tailLog(){const f=$('logSelect').value;if(!f)return;try{$('logTail').textContent=await api('/api/v1/logs/tail?file='+encodeURIComponent(f)+'&lines='+encodeURIComponent($('logLines').value))}catch(e){$('logTail').textContent=e.message}}function restartLog(){clearInterval(logTimer);if($('logAuto').checked)logTimer=setInterval(tailLog,1000)}$('logRefresh').onclick=tailLog;$('logAuto').onchange=restartLog;$('logSelect').onchange=tailLog;
async function loadRegistry(){if(!$('registryGrid'))return;try{const j=JSON.parse(await api('/api/v1/gameplay/registry'));const g=$('registryGrid');g.innerHTML='';const cx=j.context||{};$('registryContext').textContent=`Context: ${cx.mode||'unknown'}${cx.checkpoint?' | checkpoint '+cx.checkpoint:''}${cx.role?' | '+cx.role:''}`;let lastGroup='';for(const x of j.items){if((x.group||'Other')!==lastGroup){lastGroup=x.group||'Other';const h=document.createElement('div');h.className='registry-group';h.textContent=lastGroup;g.appendChild(h)}const d=document.createElement('div');d.className='registry-card';const c=(x.confidence||'').toLowerCase().replace(' ','-');d.innerHTML=`<div><b>${escapeHtml(x.label)}</b> <span class="confidence ${c.includes('confirmed')?'confirmed':c.includes('high')?'high':'unknown'}">${escapeHtml(x.confidence)}</span></div><div class="value">${escapeHtml(String(x.value??'-'))}${x.unit?' '+escapeHtml(x.unit):''}${x.limit!=null?' / '+x.limit:''}</div><div class="muted">${escapeHtml(x.source)}${x.raw?' | '+escapeHtml(x.raw):''}${x.note?' | '+escapeHtml(x.note):''}</div>`;g.appendChild(d)}}catch(e){$('registryGrid').textContent=e.message}}
function escapeHtml(v){return String(v).replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]))}
$('registryRefresh').onclick=loadRegistry;
let docsIndex=null,docsHandbook=null,docsApiSchema=null,docsScriptSchema=null,docsSelectedId='',docsMode='handbook';
function docsStatusClass(s){return 'docs-status-'+String(s||'').toLowerCase().replace(/[^a-z0-9]+/g,'-')}
function docsCode(text,lang='text'){return `<div class="docs-code"><div class="docs-code-head"><span>${escapeHtml(lang)}</span><button class="docsCopyCode" data-code="${encodeURIComponent(String(text))}">Copy</button></div><pre>${escapeHtml(text)}</pre></div>`}
function bindDocsCopy(){document.querySelectorAll('.docsCopyCode').forEach(b=>b.onclick=async()=>{const t=decodeURIComponent(b.dataset.code||'');const r=await copyTextRobust(t);b.textContent=r.ok?'Copied':'Select';setTimeout(()=>b.textContent='Copy',900)})}
function renderDocsBlocks(blocks=[]){let h='';for(const b of blocks){if(b.type==='p')h+=`<p>${escapeHtml(b.text||'')}</p>`;else if(b.type==='callout')h+=`<div class="docs-callout ${escapeHtml(b.tone||'info')}"><b>${escapeHtml(b.title||'Note')}</b><div>${escapeHtml(b.text||'')}</div></div>`;else if(b.type==='list')h+=`<ul class="docs-bullets">${(b.items||[]).map(x=>`<li>${escapeHtml(x)}</li>`).join('')}</ul>`;else if(b.type==='steps')h+=`<ol class="docs-bullets">${(b.items||[]).map(x=>`<li>${escapeHtml(x)}</li>`).join('')}</ol>`;else if(b.type==='code')h+=`${b.title?`<h3>${escapeHtml(b.title)}</h3>`:''}${docsCode(b.text||'',b.language||'text')}`;else if(b.type==='table'){h+='<div class="docs-table-wrap"><table class="docs-table"><thead><tr>'+((b.headers||[]).map(x=>`<th>${escapeHtml(x)}</th>`).join(''))+'</tr></thead><tbody>'+((b.rows||[]).map(r=>'<tr>'+r.map(x=>`<td>${escapeHtml(x)}</td>`).join('')+'</tr>').join(''))+'</tbody></table></div>'}}return h}
function apiGroup(name){const p=String(name).split('.')[0];return ({control:'Runtime control',window:'SDL / Window',memory:'Memory',register:'CPU registers',patch:'Patching',game:'Game Lab',tile:'Graphics / Tiles',checkpoint:'Checkpoints',frame:'Frames / snapshots',image:'Image analysis',sprite:'Sprites',sprites:'Sprites',palette:'Palette',track:'Track / Road',docs:'Documentation',knowledge:'Documentation',audio:'Audio',input:'Input / IOC',evidence:'Evidence',experiment:'Experiments',profile:'Profiles',profiles:'Profiles',schema:'Schema / discovery',script:'Scripting',gameplay:'Gameplay state',collision:'Collision',tuning:'Tuning'}[p]||'Core / Other')}
function docsSetMode(mode){docsMode=mode;document.querySelectorAll('[data-docs-mode]').forEach(b=>b.classList.toggle('on',b.dataset.docsMode===mode));$('docsSearch').value='';renderDocsNav();if(mode==='handbook')showHandbookPage('welcome');else if(mode==='api')showApiLanding();else if(mode==='script')showScriptReference();else if(mode==='knowledge')showKnowledgeLanding();else showGlossaryLanding()}
function renderDocsNav(){const root=$('docsList');root.innerHTML='';const q=$('docsSearch').value.trim().toLowerCase();if(q){return renderDocsSearch(q)}
 if(docsMode==='handbook'){for(const s of (docsHandbook?.sections||[])){const g=document.createElement('div');g.className='docs-nav-group';g.innerHTML=`<div class="docs-nav-title">${escapeHtml(s.title)}</div>`;for(const p of (s.pages||[])){const d=document.createElement('button');d.className='docs-nav-item';d.textContent=p.title;d.onclick=()=>showHandbookPage(p.id);g.appendChild(d)}root.appendChild(g)}return}
 if(docsMode==='api'){const groups={};for(const n of Object.keys(docsApiSchema?.actions||{}).sort()){const g=apiGroup(n);(groups[g]??=[]).push(n)}for(const [g,names] of Object.entries(groups)){const box=document.createElement('div');box.className='docs-nav-group';box.innerHTML=`<div class="docs-nav-title">${escapeHtml(g)}</div>`;for(const n of names){const b=document.createElement('button');b.className='docs-nav-item mono';b.textContent=n;b.onclick=()=>showApiAction(n);box.appendChild(b)}root.appendChild(box)}return}
 if(docsMode==='script'){const g=document.createElement('div');g.className='docs-nav-group';g.innerHTML='<div class="docs-nav-title">Language</div>';const ov=document.createElement('button');ov.className='docs-nav-item';ov.textContent='Complete language reference';ov.onclick=showScriptReference;g.appendChild(ov);(docsScriptSchema?.statements||[]).forEach((x,i)=>{const b=document.createElement('button');b.className='docs-nav-item mono';b.textContent=x.syntax;b.onclick=()=>showScriptStatement(i);g.appendChild(b)});const qf=document.createElement('button');qf.className='docs-nav-item';qf.textContent='Query functions';qf.onclick=showScriptReference;g.appendChild(qf);root.appendChild(g);return}
 const items=(docsIndex?.entries||[]).filter(x=>docsMode==='glossary'?x.kind==='glossary':x.kind!=='glossary');for(const x of items){const d=document.createElement('button');d.className='docs-item';d.innerHTML=`<b>${escapeHtml(x.title)}</b><div class="muted">${escapeHtml(x.summary)}</div>`;d.onclick=()=>showDocsEntry(x.id);root.appendChild(d)}
}
function renderDocsSearch(q){const root=$('docsList');root.innerHTML='';let groups=[];const pages=[];for(const s of (docsHandbook?.sections||[]))for(const p of (s.pages||[])){if(`${p.title} ${p.summary} ${JSON.stringify(p.blocks||[])}`.toLowerCase().includes(q))pages.push({section:s.title,page:p})}if(pages.length)groups.push(['Handbook',pages.map(x=>({label:x.page.title,sub:x.section,go:()=>showHandbookPage(x.page.id)}))]);const acts=Object.entries(docsApiSchema?.actions||{}).filter(([n,x])=>`${n} ${x.description} ${(x.params||[]).join(' ')}`.toLowerCase().includes(q));if(acts.length)groups.push(['API actions',acts.map(([n,x])=>({label:n,sub:x.description,go:()=>showApiAction(n)}))]);const st=(docsScriptSchema?.statements||[]).map((x,i)=>({x,i})).filter(o=>`${o.x.syntax} ${o.x.description}`.toLowerCase().includes(q));if(st.length)groups.push(['Script language',st.map(o=>({label:o.x.syntax,sub:o.x.description,go:()=>showScriptStatement(o.i)}))]);const kn=(docsIndex?.entries||[]).filter(x=>`${x.title} ${x.summary} ${(x.tags||[]).join(' ')} ${x.category} ${x.searchText||''}`.toLowerCase().includes(q));if(kn.length)groups.push(['Project knowledge',kn.map(x=>({label:x.title,sub:x.summary,go:()=>showDocsEntry(x.id)}))]);for(const [title,items] of groups){const g=document.createElement('div');g.className='docs-nav-group';g.innerHTML=`<div class="docs-nav-title">${escapeHtml(title)} (${items.length})</div>`;for(const x of items){const b=document.createElement('button');b.className='docs-item';b.innerHTML=`<b>${escapeHtml(x.label)}</b><div class="muted">${escapeHtml(x.sub||'')}</div>`;b.onclick=x.go;g.appendChild(b)}root.appendChild(g)}if(!groups.length)root.innerHTML='<div class="muted" style="padding:12px">No handbook, API, scripting or knowledge matches.</div>'}
function docsPageHeader(title,summary,badges=[]){return `<div class="docs-page-head"><div><h2 class="docs-title">${escapeHtml(title)}</h2>${summary?`<p class="docs-lede">${escapeHtml(summary)}</p>`:''}</div>${badges.length?`<div class="docs-meta">${badges.map(x=>`<span class="docs-chip">${escapeHtml(x)}</span>`).join('')}</div>`:''}</div>`}
async function showHandbookPage(id){for(const s of (docsHandbook?.sections||[])){const p=(s.pages||[]).find(x=>x.id===id);if(!p)continue;if(p.generated==='api-reference')return docsSetMode('api');if(p.generated==='script-reference')return docsSetMode('script');if(p.generated==='knowledge-browser')return docsSetMode('knowledge');if(p.generated==='glossary')return docsSetMode('glossary');if(id==='chatgpt-handover'){try{const t=await api('/api/v1/docs/handover');$('docsDetail').innerHTML=docsPageHeader(p.title,p.summary,['Project Continuation',`v${docsHandbook.build}`])+`<div class="docs-callout info"><b>Start a new ChatGPT conversation</b><div>Upload the latest ChaseHQ-Native ZIP, then paste the complete authoritative continuation prompt below. The ZIP plus this prompt are the continuation source.</div></div><div class="row" style="margin:12px 0"><button id="docsCopyFullHandover" class="primary">Copy full handover</button><button id="docsDownloadHandover">Download handover .md</button><span id="docsHandoverResult" class="muted"></span></div><pre id="docsHandoverText" class="docs-code" style="max-height:620px;overflow:auto;white-space:pre-wrap;user-select:text">${escapeHtml(t)}</pre><h3>About this handover</h3>${renderDocsBlocks(p.blocks||[])}`;$('docsCopyFullHandover').onclick=async()=>{const r=await copyTextRobust(t);$('docsHandoverResult').textContent=r.ok?'Full handover copied.':'Copy failed - select the text below.'};$('docsDownloadHandover').onclick=()=>{const blob=new Blob([t],{type:'text/markdown;charset=utf-8'}),u=URL.createObjectURL(blob),a=document.createElement('a');a.href=u;a.download='CHATGPT_HANDOVER_PROMPT.md';document.body.appendChild(a);a.click();a.remove();setTimeout(()=>URL.revokeObjectURL(u),1000)};return}catch(e){$('docsDetail').textContent='Handover load failed: '+e.message;return}}$('docsDetail').innerHTML=docsPageHeader(p.title,p.summary,[s.title,`v${docsHandbook.build}`])+renderDocsBlocks(p.blocks||[]);bindDocsCopy();return}}
function showApiLanding(){
 const actions=docsApiSchema?.actions||{},groups={};
 for(const n of Object.keys(actions).sort()){const g=apiGroup(n);(groups[g]??=[]).push(n)}
 let h=docsPageHeader('Research API Reference','Complete scriptable Research API contract generated from the authoritative schema in this build.',[`v${docsApiSchema?.build||'?'}`,`${Object.keys(actions).length} actions`]);
 h+=`<div class="docs-callout info"><b>Two interfaces</b><div>This page documents the scriptable Research API used as <code>api ACTION key=value</code> from .chqscript. The localhost HTTP API is a separate transport used by the Workbench and external clients.</div></div>`;
 h+=`<div class="docs-callout info"><b>Discover, do not guess</b><div>Use <code>api actions</code>, <code>api schema.all</code>, <code>api schema action=NAME</code> or <code>api script.schema</code>. The reference below is generated from that same live contract.</div></div>`;
 h+=`<div class="docs-card-grid">${Object.entries(groups).sort().map(([g,n])=>`<div class="docs-card"><b>${escapeHtml(g)}</b><div class="docs-big">${n.length}</div><span class="muted">actions</span></div>`).join('')}</div>`;
 h+='<h2 style="margin-top:24px">Complete action reference</h2>';
 for(const [group,names] of Object.entries(groups).sort()){
   h+=`<h3 class="docs-section-title">${escapeHtml(group)} <span class="muted">(${names.length})</span></h3>`;
   for(const name of names){
     const x=actions[name]||{},params=x.params||[];
     h+=`<details class="docs-api-entry" style="border:1px solid #2a3744;background:#0e151c;margin:8px 0;padding:10px 12px"><summary style="cursor:pointer"><code style="font-size:14px">${escapeHtml(name)}</code> - ${escapeHtml(x.description||'')}</summary><div style="padding:12px 4px 4px">`;
     h+=`<p>${escapeHtml(x.description||'No description supplied.')}</p>`;
     h+='<h4>Parameters</h4>';
     if(params.length){h+=`<table class="docs-table"><thead><tr><th>Name / values</th><th>Requirement / notes</th></tr></thead><tbody>${params.map(p=>{const a=parseParam(p);return `<tr><td><code>${escapeHtml(a[0])}</code></td><td>${escapeHtml(a[1])}</td></tr>`}).join('')}</tbody></table>`}else h+='<p class="muted">No parameters.</p>';
     h+=`<h4>Example</h4>${docsCode(x.example||`api ${name}`,'chqscript')}`;
     h+=`<div class="row"><button class="docsApiOpen" data-api="${escapeHtml(name)}">Open dedicated action page</button></div></div></details>`;
   }
 }
 $('docsDetail').innerHTML=h;
 document.querySelectorAll('.docsApiOpen').forEach(b=>b.onclick=()=>showApiAction(b.dataset.api));
 bindDocsCopy();
}
function parseParam(p){const m=String(p).match(/^(\S+)\s+(.*)$/);return m?[m[1],m[2]]:[p,'']}
function showApiAction(name){const x=docsApiSchema?.actions?.[name];if(!x)return;$('docsDetail').innerHTML=docsPageHeader(name,x.description,[apiGroup(name),`build ${docsApiSchema.build}`])+`<h3>Parameters</h3>${(x.params||[]).length?`<table class="docs-table"><thead><tr><th>Name / values</th><th>Requirement / notes</th></tr></thead><tbody>${x.params.map(p=>{const a=parseParam(p);return `<tr><td><code>${escapeHtml(a[0])}</code></td><td>${escapeHtml(a[1])}</td></tr>`}).join('')}</tbody></table>`:'<p class="muted">No parameters.</p>'}<h3>Example</h3>${docsCode(x.example||`api ${name}`,'chqscript')}<h3>Availability</h3><ul class="docs-bullets"><li>Research Script: <code>api ${escapeHtml(name)} ...</code></li><li>Machine-readable contract: <code>api schema action=${escapeHtml(name)}</code></li></ul>`;bindDocsCopy()}
function exampleForSyntax(s){if(s.startsWith('set '))return 'set checkpoint = stage1-driving-2352.chqstate';if(s.startsWith('${'))return 'echo Loaded ${checkpoint}';if(s.startsWith('repeat'))return 'repeat 3\n  api control.step-frame\n  echo step ${repeat_index}\nend';if(s.startsWith('for '))return 'set banks = 64,65\nfor bank in banks\n  api palette.bank bank=${bank}\nend';if(s.startsWith('if status'))return 'if status paused=1\n  echo machine is paused\nelse\n  echo machine is running\nend';if(s.startsWith('if capability'))return 'if capability window.always_on_top\n  api window.always_on_top enabled=true\nend';if(s.startsWith('try'))return 'api timeline.reset\ntry\n  api timeline.load name=example\n  # experiment\nfinally\n  api timeline.reset\nend';if(s.startsWith('api '))return 'api frame.snapshot name=study sprites=individual';if(s.startsWith('include'))return 'include "common/load-stage1.chqscript"';if(s.startsWith('wait '))return 'wait 250';if(s.startsWith('assert'))return 'assert status paused=1';if(s.startsWith('require'))return 'require capability frame.snapshot';if(s.startsWith('echo'))return 'echo Starting deterministic capture';if(s.startsWith('zip evidence-match'))return 'zip evidence-match turbo- turbo-study';if(s.startsWith('zip evidence'))return 'zip evidence latest';return s}
function showScriptStatement(i){const x=docsScriptSchema?.statements?.[i];if(!x)return;$('docsDetail').innerHTML=docsPageHeader(x.syntax,x.description,['Research Script',docsScriptSchema.engine])+`<h3>Syntax</h3>${docsCode(x.syntax,'chqscript')}<h3>Example</h3>${docsCode(exampleForSyntax(x.syntax),'chqscript')}<p class="muted">The authoritative parser limits and complete statement list are shown on the Complete language reference page.</p>`;bindDocsCopy()}
function showScriptReference(){
 const s=docsScriptSchema||{},limits=s.limits||{},statements=s.statements||[],queries=s.queryFunctions||[];
 let h=docsPageHeader('Research Script Language Reference','Complete live language reference generated from the authoritative script schema in this build.',[s.engine||'',`v${s.build||'?'}`,`${statements.length} statements`]);
 h+=`<div class="docs-callout info"><b>What .chqscript is for</b><div>Research Script is a small deterministic orchestration language for repeatable emulator research. Use language statements for flow control and <code>api ACTION key=value</code> for emulator/debug operations.</div></div>`;
 h+='<h2>Quick start</h2>';
 h+=docsCode('# Load a known state\nset checkpoint = stage1-driving-2352.chqstate\napi checkpoint.load file=${checkpoint} run=false\nrequire capability frame.snapshot\napi frame.snapshot name=quick-study sprites=individual\necho Capture complete','chqscript');
 h+='<h2>Complete statement reference</h2>';
 for(let i=0;i<statements.length;i++){
   const x=statements[i];
   h+=`<details class="docs-api-entry" style="border:1px solid #2a3744;background:#0e151c;margin:8px 0;padding:10px 12px"><summary style="cursor:pointer"><code>${escapeHtml(x.syntax)}</code> - ${escapeHtml(x.description||'')}</summary><div style="padding:12px 4px 4px"><h4>Purpose</h4><p>${escapeHtml(x.description||'')}</p><h4>Syntax</h4>${docsCode(x.syntax,'chqscript')}<h4>Example</h4>${docsCode(exampleForSyntax(x.syntax),'chqscript')}<div class="row"><button class="docsScriptOpen" data-script-index="${i}">Open dedicated statement page</button></div></div></details>`;
 }
 h+='<h2>Query functions</h2>';
 if(queries.length){h+=`<div class="docs-callout info"><b>Query expressions</b><div>These query functions are exposed by the authoritative script schema for use where supported by the language.</div></div><div class="docs-pills">${queries.map(x=>`<code>${escapeHtml(typeof x==='string'?x:(x.name||JSON.stringify(x)))}</code>`).join(' ')}</div>`}else h+='<p class="muted">No query functions are declared by this build.</p>';
 h+='<h2>Parser and execution limits</h2>';
 if(Object.keys(limits).length){h+=`<table class="docs-table"><thead><tr><th>Limit</th><th>Value</th></tr></thead><tbody>${Object.entries(limits).map(([k,v])=>`<tr><td><code>${escapeHtml(k)}</code></td><td>${escapeHtml(v)}</td></tr>`).join('')}</tbody></table>`}else h+='<p class="muted">No parser limits reported.</p>';
 h+='<h2>Calling the Research API</h2>';
 h+=`<p>Most emulator operations are intentionally not separate language keywords. Invoke the documented Research API using <code>api ACTION key=value</code>.</p>${docsCode('api actions\napi schema.all\napi schema action=frame.snapshot\napi frame.snapshot name=study sprites=individual','chqscript')}<div class="row"><button id="docsOpenApiFromScript">Open complete API reference</button></div>`;
 h+='<h2>Language discovery</h2>';
 h+=docsCode('api script.schema','chqscript');
 $('docsDetail').innerHTML=h;
 document.querySelectorAll('.docsScriptOpen').forEach(b=>b.onclick=()=>showScriptStatement(+b.dataset.scriptIndex));
 const apiBtn=$('docsOpenApiFromScript');if(apiBtn)apiBtn.onclick=()=>docsSetMode('api');
 bindDocsCopy();
}
function renderEvidence(x){let ev='';for(const e of (x.evidence||[])){if(e.type==='image'){let rel=String(e.path||'').replace(/^docs\/evidence\//,'');ev+=`<figure><img src="/api/v1/docs/evidence/${encodeURI(rel)}" alt="${escapeHtml(e.caption||e.id)}"><figcaption>${escapeHtml(e.caption||'')}<br>${escapeHtml(e.build?'Build '+e.build:'')}</figcaption></figure>`}else ev+=`<div class="artifact-card"><b>${escapeHtml(e.type||'artifact')}</b><div>${escapeHtml(e.caption||'')}</div><code>${escapeHtml(e.path||'')}</code></div>`}return ev}
function renderKnowledgeArticle(a){
 if(!a)return '';
 let h='';
 if(a.discovery)h+=`<h3>What we discovered</h3><p>${escapeHtml(a.discovery)}</p>`;
 if(a.significance)h+=`<h3>Why it matters</h3><p>${escapeHtml(a.significance)}</p>`;
 if((a.confirmedValues||[]).length)h+=`<h3>Confirmed values</h3><table class="docs-table"><thead><tr><th>Item</th><th>Confirmed value</th></tr></thead><tbody>${a.confirmedValues.map(v=>`<tr><td>${escapeHtml(v.name)}</td><td><code>${escapeHtml(v.value)}</code></td></tr>`).join('')}</tbody></table>`;
 if((a.method||[]).length)h+=`<h3>How it was proven</h3><ol class="docs-bullets">${a.method.map(v=>`<li>${escapeHtml(v)}</li>`).join('')}</ol>`;
 if((a.reproduction||[]).length)h+=`<h3>How to reproduce</h3><ol class="docs-bullets">${a.reproduction.map(v=>`<li>${escapeHtml(v)}</li>`).join('')}</ol>`;
 if((a.openQuestions||[]).length)h+=`<h3>Open questions / next research</h3><ul class="docs-bullets">${a.openQuestions.map(v=>`<li>${escapeHtml(v)}</li>`).join('')}</ul>`;
 return h
}
async function showDocsEntry(id){
 try{
  const x=JSON.parse(await api('/api/v1/docs/get/'+encodeURIComponent(id)));
  docsSelectedId=id;
  const rel=(x.related||[]).map(r=>`<button class="docsRelated" data-id="${escapeHtml(r)}">${escapeHtml(r)}</button>`).join(' '),ev=renderEvidence(x);
  $('docsDetail').innerHTML=
   docsPageHeader(x.title,x.summary,[x.category,x.status,`confidence: ${x.confidence||'?'}`,`updated ${x.updated||'?'}`])+
   renderKnowledgeArticle(x.article)+
   `${ev?`<h3>Evidence</h3><div class="docs-evidence">${ev}</div>`:''}`+
   `${(x.apiActions||[]).length?`<h3>Relevant API actions</h3><div class="docs-pills">${x.apiActions.map(a=>`<button class="docsApiLink" data-api="${escapeHtml(a)}"><code>${escapeHtml(a)}</code></button>`).join(' ')}</div>`:''}`+
   `${(x.scripts||[]).length?`<h3>Scripts</h3><ul class="docs-bullets">${x.scripts.map(v=>`<li><code>${escapeHtml(v)}</code></li>`).join('')}</ul>`:''}`+
   `${(x.checkpoints||[]).length?`<h3>Checkpoints</h3><ul class="docs-bullets">${x.checkpoints.map(v=>`<li><code>${escapeHtml(v)}</code></li>`).join('')}</ul>`:''}`+
   `${(x.docs||[]).length?`<h3>Source documentation</h3><ul class="docs-bullets">${x.docs.map(v=>`<li><code>${escapeHtml(v)}</code></li>`).join('')}</ul>`:''}`+
   `${rel?`<h3>Related knowledge</h3><div class="docs-pills">${rel}</div>`:''}`;
  document.querySelectorAll('.docsRelated').forEach(b=>b.onclick=()=>showDocsEntry(b.dataset.id));
  document.querySelectorAll('.docsApiLink').forEach(b=>b.onclick=()=>{docsSetMode('api');showApiAction(b.dataset.api)})
 }catch(e){$('docsDetail').textContent=e.message}
}
function showKnowledgeLanding(){
 const items=(docsIndex?.entries||[]).filter(x=>x.kind!=='glossary'),counts={};for(const x of items)counts[x.status]=(counts[x.status]||0)+1;
 const groups=[['Confirmed findings',['confirmed']],['Provisional findings',['provisional']],['Active / open investigations',['open','planned']],['Implemented awaiting proof',['implemented-awaiting-proof']],['Historical / superseded',['historical','superseded']]];
 let h=docsPageHeader('Project Knowledge','Evidence-backed findings, concepts, open investigations and feature state. Metadata is supporting context; the article and evidence are the primary documentation.',[`${items.length} entries`]);
 h+=`<div class="docs-card-grid">${Object.entries(counts).sort().map(([k,v])=>`<div class="docs-card"><b>${escapeHtml(k)}</b><div class="docs-big">${v}</div></div>`).join('')}</div>`;
 for(const [title,statuses] of groups){const list=items.filter(x=>statuses.includes(x.status));if(!list.length)continue;h+=`<h3>${escapeHtml(title)}</h3><div class="docs-card-grid">${list.map(x=>`<button class="docs-card docsKnowledgeCard" data-id="${escapeHtml(x.id)}" style="text-align:left"><b>${escapeHtml(x.title)}</b><div class="muted">${escapeHtml(x.category)} | ${escapeHtml(x.status)}${x.confidence?` | confidence: ${escapeHtml(x.confidence)}`:''}</div><div>${escapeHtml(x.summary)}</div></button>`).join('')}</div>`}
 h+='<p class="muted">Select any card or an article on the left to open the complete finding, evidence, reproduction steps and related research.</p>';
 $('docsDetail').innerHTML=h;document.querySelectorAll('.docsKnowledgeCard').forEach(b=>b.onclick=()=>showDocsEntry(b.dataset.id))
}
function showGlossaryLanding(){const items=(docsIndex?.entries||[]).filter(x=>x.kind==='glossary');$('docsDetail').innerHTML=docsPageHeader('Glossary','Quick, human-readable definitions. Select a term on the left for related evidence and references.',[`${items.length} terms`])+`<dl class="docs-glossary">${items.map(x=>`<dt>${escapeHtml(x.title)}</dt><dd>${escapeHtml(x.summary)}</dd>`).join('')}</dl>`}
async function loadDocsKnowledge(){try{const [idx,hb,apiS,scriptS]=await Promise.all([api('/api/v1/docs/index'),api('/api/v1/docs/handbook'),api('/api/v1/schema/actions'),api('/api/v1/schema/script')]);docsIndex=JSON.parse(idx);docsHandbook=JSON.parse(hb);docsApiSchema=JSON.parse(apiS);docsScriptSchema=JSON.parse(scriptS);renderDocsNav();showHandbookPage('welcome')}catch(e){$('docsDetail').textContent='Documentation load failed: '+e.message}}
async function exportDocs(format){$('docsExportResult').textContent='Exporting full handbook...';try{const body={format,evidence:$('docsExportEvidence').value};const j=JSON.parse(await api('/api/v1/docs/export',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body),timeoutMs:70000}));$('docsExportResult').innerHTML=`Created ${escapeHtml(j.name)} | <a href="${escapeHtml(j.download)}">download</a>`}catch(e){$('docsExportResult').textContent='Export failed: '+e.message}}
$('docsRefresh').onclick=loadDocsKnowledge;$('docsSearch').oninput=renderDocsNav;document.querySelectorAll('[data-docs-mode]').forEach(b=>b.onclick=()=>docsSetMode(b.dataset.docsMode));$('docsCopyHandover').onclick=async()=>{try{const t=await api('/api/v1/docs/handover');await navigator.clipboard.writeText(t);$('docsExportResult').textContent='Authoritative ChatGPT handover copied.'}catch(e){$('docsExportResult').textContent=e.message}};$('docsExportHtml').onclick=()=>exportDocs('html');$('docsExportPdf').onclick=()=>exportDocs('pdf');

async function loadNextSteps(){try{const j=JSON.parse(await api('/api/v1/next-steps'));const root=$('nextSteps');root.innerHTML='';for(const x of j.items){const d=document.createElement('div');d.className='next-item';d.dataset.status=x.status;d.innerHTML=`<b>${escapeHtml(x.title)}</b> <span class="badge">${escapeHtml(x.status)}</span><div class="muted">${escapeHtml(x.detail)}</div>`;root.appendChild(d)}}catch(e){$('nextSteps').textContent=e.message}}
// Native text format stores repeated xor/scenario keys; parse a richer table directly instead.
async function refreshIocTable(){try{const raw=await api('/api/input/ports');const toks=[...raw.matchAll(/port(\d+)=([^\s]+)\s+xor=([^\s]+)\s+scenario=([^\s]+)\s+effective_xor=([^\s]+)\s+reads=([^\s]+)/g)];let h='<table class="ioc-table"><tr><th>Port</th><th>Base</th><th>Scenario XOR</th><th>Debugger XOR</th><th>Effective XOR</th><th>Reads</th></tr>';for(const m of toks)h+=`<tr><td>${(+m[1]).toString(16).toUpperCase()}</td><td>${m[2]}</td><td>${m[4]}</td><td>${m[3]}</td><td>${m[5]}</td><td>${m[6]}</td></tr>`;h+='</table>';$('iocLiveTable').innerHTML=h}catch(e){$('iocLiveTable').textContent=e.message}}
document.querySelectorAll('[data-sem-pulse]').forEach(b=>b.onclick=async()=>{$('experimentalResult').textContent=await cmd(`input pulse 3 ${b.dataset.semPulse} 3`)});
async function gameAction(action,args={}){const raw=await api('/api/v1/game/action',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({action,...args})});return JSON.parse(raw)}
function fmtHex(v,w=4){let n=Number(v);return Number.isFinite(n)?'0x'+n.toString(16).toUpperCase().padStart(w,'0'):String(v)}
async function refreshGameLab(){try{const j=await gameAction('status');const q=j.state||{};$('gameTurboState').textContent=`Remaining: ${q.turbosRemaining ?? '?'} | Active: ${q.turboActive?'YES':'NO'} | Timer: ${fmtHex(q.turboTimer??0)} / 0x00D2`;$('gameSpeedState').textContent=`HUD speed: ${q.displaySpeed ?? '?'} km/h (${q.displaySpeedRaw ?? '?'}) | Internal physics: ${q.internalSpeed ?? q.currentSpeed ?? '?'} (${fmtHex(q.internalSpeed??q.currentSpeed??0)})`;if($('gameTargetState'))$('gameTargetState').textContent=`Target remaining-hit counter: ${q.targetHealth ?? '?'} (${q.targetHealthRaw ?? '?'}) | zero = one final damaging hit`;if($('gameSpriteOrderState'))$('gameSpriteOrderState').textContent=`Same-priority tie-break: ${q.spriteTieBreak||'?'}`;$('gameTurboStock').classList.toggle('on',!!q.infiniteTurboStock);$('gameTurboStock').textContent=`Infinite stock: ${q.infiniteTurboStock?'ON':'OFF'}`;$('gameTurboActive').classList.toggle('on',!!q.infiniteTurboActive);$('gameTurboActive').textContent=`Infinite active: ${q.infiniteTurboActive?'ON':'OFF'}`;$('gameSpeedFreeze').classList.toggle('on',!!q.speedFrozen);$('gameSpeedFreeze').textContent=`Freeze internal speed: ${q.speedFrozen?'ON':'OFF'}`;if($('gameTargetOneHit')){$('gameTargetOneHit').classList.toggle('on',!!q.targetOneHit);$('gameTargetOneHit').textContent=`One-hit Target Kill: ${q.targetOneHit?'ON':'OFF'}`}$('gameSpriteHigher')?.classList.toggle('on',q.spriteTieBreak==='higher-slot');$('gameSpriteLower')?.classList.toggle('on',q.spriteTieBreak==='lower-slot')}catch(e){$('experimentalResult').textContent='Game Lab refresh failed: '+e.message}}
$('gameTurboFire').onclick=async()=>{try{$('experimentalResult').textContent=JSON.stringify(await gameAction('turbo.fire'),null,2);setTimeout(refreshGameLab,120)}catch(e){$('experimentalResult').textContent=e.message}};
$('gameTurboStock').onclick=async()=>{try{const on=!$('gameTurboStock').classList.contains('on');$('experimentalResult').textContent=JSON.stringify(await gameAction('turbo.stock',{enabled:on,value:+$('gameTurboRemaining').value||3}),null,2);await refreshGameLab()}catch(e){$('experimentalResult').textContent=e.message}};
$('gameTurboActive').onclick=async()=>{try{const on=!$('gameTurboActive').classList.contains('on');$('experimentalResult').textContent=JSON.stringify(await gameAction('turbo.active',{enabled:on}),null,2);await refreshGameLab()}catch(e){$('experimentalResult').textContent=e.message}};
$('gameTurboResetTimer').onclick=async()=>{try{$('experimentalResult').textContent=JSON.stringify(await gameAction('turbo.timer.reset'),null,2);await refreshGameLab()}catch(e){$('experimentalResult').textContent=e.message}};
$('gameTurboSetRemaining').onclick=async()=>{try{$('experimentalResult').textContent=JSON.stringify(await gameAction('turbo.remaining.set',{value:+$('gameTurboRemaining').value}),null,2);await refreshGameLab()}catch(e){$('experimentalResult').textContent=e.message}};
$('gameSpeedSet').onclick=async()=>{try{$('experimentalResult').textContent=JSON.stringify(await gameAction('speed.set',{value:+$('gameSpeedValue').value}),null,2);await refreshGameLab()}catch(e){$('experimentalResult').textContent=e.message}};
$('gameSpeedFreeze').onclick=async()=>{try{const on=!$('gameSpeedFreeze').classList.contains('on');$('experimentalResult').textContent=JSON.stringify(await gameAction('speed.freeze',{enabled:on,value:+$('gameSpeedValue').value}),null,2);await refreshGameLab()}catch(e){$('experimentalResult').textContent=e.message}};
$('gameSpeedClear').onclick=async()=>{try{$('experimentalResult').textContent=JSON.stringify(await gameAction('speed.clear'),null,2);await refreshGameLab()}catch(e){$('experimentalResult').textContent=e.message}};
$('gameTargetOneHit').onclick=async()=>{try{const on=!$('gameTargetOneHit').classList.contains('on');$('experimentalResult').textContent=JSON.stringify(await gameAction('target.one-hit',{enabled:on}),null,2);await refreshGameLab()}catch(e){$('experimentalResult').textContent=e.message}};
$('gameSpriteHigher').onclick=async()=>{try{$('experimentalResult').textContent=JSON.stringify(await gameAction('sprite.order.set',{mode:'higher-slot'}),null,2);await refreshGameLab()}catch(e){$('experimentalResult').textContent=e.message}};
$('gameSpriteLower').onclick=async()=>{try{$('experimentalResult').textContent=JSON.stringify(await gameAction('sprite.order.set',{mode:'lower-slot'}),null,2);await refreshGameLab()}catch(e){$('experimentalResult').textContent=e.message}};
$('gameSpriteOrderInspect').onclick=async()=>{try{$('experimentalResult').textContent=JSON.stringify(await gameAction('sprite.order.inspect'),null,2)}catch(e){$('experimentalResult').textContent=e.message}};
async function inspectTile(){try{const body={action:'tile.inspect',code:$('tileInspectCode').value,palette:+$('tileInspectPalette').value||0,scale:+$('tileInspectScale').value||8};const raw=await api('/api/v1/tile/inspect',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)}),j=JSON.parse(raw);$('tileInspectResult').textContent=JSON.stringify(j.result,null,2);const img=$('tileInspectPreview');if(j.result?.png){img.src=artifactUrl(j.result.png)+'&t='+Date.now();img.style.display='block'}else img.style.display='none'}catch(e){$('tileInspectResult').textContent=e.message}}
$('tileInspectRun').onclick=inspectTile;document.querySelectorAll('[data-tile-preset]').forEach(b=>b.onclick=()=>{const [c,p]=b.dataset.tilePreset.split(',');$('tileInspectCode').value=c;$('tileInspectPalette').value=p;inspectTile()});

async function applySemanticHolds(){const masks=[...document.querySelectorAll('[data-sem-hold]:checked')].map(x=>parseInt(x.dataset.semHold,16));semanticHoldMask=masks.reduce((a,b)=>a|b,0);$('experimentalResult').textContent=semanticHoldMask?await cmd(`input xor set 3 ${semanticHoldMask.toString(16).toUpperCase().padStart(2,'0')}`):await cmd('input xor clear');refreshIocTable()}
document.querySelectorAll('[data-sem-hold]').forEach(x=>x.onchange=applySemanticHolds);$('semClear').onclick=async()=>{document.querySelectorAll('[data-sem-hold]').forEach(x=>x.checked=false);semanticHoldMask=0;$('experimentalResult').textContent=await cmd('input xor clear');refreshIocTable()};
refreshGameLab();setInterval(refreshGameLab,1500);
$('runFrames').onclick=async()=>{try{const j=JSON.parse(await api('/api/v1/control/run-frames',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({frames:+$('runFramesCount').value})}));$('experimentalResult').textContent=JSON.stringify(j,null,2);refresh();refreshPreview()}catch(e){$('experimentalResult').textContent=e.message}};
$('disRun').onclick=async()=>{try{const q=`?cpu=${encodeURIComponent($('disCpu').value)}&address=${encodeURIComponent($('disAddr').value)}&count=${encodeURIComponent($('disCount').value)}`;$('experimentalResult').textContent=(JSON.parse(await api('/api/v1/cpu/disassemble'+q))).text}catch(e){$('experimentalResult').textContent=e.message}};
function svgPathFor(items,xName,yName){return items.map((p,i)=>`${i?'L':'M'} ${Number(p[xName]??p.x).toFixed(1)} ${Number(p[yName]??p.y).toFixed(1)}`).join(' ')}
function renderTrackRecorder(j){
 trackRecording=!!j.enabled;trackSamples=Array.isArray(j.samples)?j.samples:trackSamples;$('trackToggle').textContent=trackRecording?'Track recording: ON':'Track recording: OFF';$('trackToggle').classList.toggle('on',trackRecording);
 $('trackPath').setAttribute('d',svgPathFor(trackSamples,'x','y'));$('trackRoadLeft').setAttribute('d',svgPathFor(trackSamples,'roadLeftX','roadLeftY'));$('trackRoadRight').setAttribute('d',svgPathFor(trackSamples,'roadRightX','roadRightY'));$('trackCarLine').setAttribute('d',svgPathFor(trackSamples,'carX','carY'));
 const showRoad=$('trackShowRoad')?.checked!==false;$('trackRoadLeft').style.display=showRoad?'':'none';$('trackRoadRight').style.display=showRoad?'':'none';$('trackCarLine').style.display=$('trackShowCarLine')?.checked===false?'none':'';
 const seg=$('trackSegments');seg.replaceChildren();const colour=b=>b==='NEGATIVE'?'#60a5fa':b==='POSITIVE'?'#fb923c':'#d1d5db';for(let i=1;i<trackSamples.length;i++){const a=trackSamples[i-1],b=trackSamples[i],ln=document.createElementNS('http://www.w3.org/2000/svg','line');ln.setAttribute('x1',a.x);ln.setAttribute('y1',a.y);ln.setAttribute('x2',b.x);ln.setAttribute('y2',b.y);ln.setAttribute('stroke',colour(b.gradientBand));ln.setAttribute('stroke-width','4');ln.setAttribute('stroke-linecap','round');const title=document.createElementNS('http://www.w3.org/2000/svg','title');title.textContent=`frame ${b.frame} route/page ${b.trackRoute}/${b.trackPage} record #${b.recordIndex} curve=${b.curve} speed=${b.displaySpeed} road=${b.roadState} surface=${b.surfaceSignature}`;ln.appendChild(title);seg.appendChild(ln)}
 const last=trackSamples.at(-1);$('trackCar').setAttribute('cx',last?.carX??last?.x??350);$('trackCar').setAttribute('cy',last?.carY??last?.y??470);$('trackStatus').textContent=`Recorder v2: ${j.enabled?'ON':'OFF'} | samples ${j.sampleCount??trackSamples.length} | surfaces ${j.surfaceSignatureCount??'?'} | survey ${j.courseSurvey?'ON':'OFF'}`;if($('trackAutoFit')?.checked)fitTrack(false)
}
async function refreshTrackRecorder(){try{const j=JSON.parse(await api('/api/v1/track/record'));renderTrackRecorder(j);if(trackSamples.length)$('trackTelemetry').textContent=JSON.stringify(trackSamples.at(-1),null,2);else $('trackTelemetry').textContent='No recorded track samples.'}catch(e){$('trackTelemetry').textContent=e.message}}
async function resetTrack(){try{const j=JSON.parse(await api('/api/v1/track/record/clear',{method:'POST'}));trackSamples=[];renderTrackRecorder({...j,samples:[]});$('trackSvg').setAttribute('viewBox','0 0 700 520')}catch(e){$('trackTelemetry').textContent=e.message}}
$('trackClear').onclick=resetTrack;$('trackToggle').onclick=async()=>{try{const j=JSON.parse(await api(trackRecording?'/api/v1/track/record/stop':'/api/v1/track/record/start',{method:'POST'}));renderTrackRecorder({...j,samples:trackSamples});if(j.enabled)await sampleTrack()}catch(e){$('trackTelemetry').textContent=e.message}};
$('trackFit').onclick=()=>fitTrack(true);$('trackSaveSvg').onclick=saveTrackSvg;$('trackAutoFit').onchange=()=>{if($('trackAutoFit').checked)fitTrack(false)};$('trackShowRoad').onchange=()=>renderTrackRecorder({enabled:trackRecording,samples:trackSamples,sampleCount:trackSamples.length});$('trackShowCarLine').onchange=()=>renderTrackRecorder({enabled:trackRecording,samples:trackSamples,sampleCount:trackSamples.length});
function fitTrack(tight=false){const svg=$('trackSvg');if(!svg||!trackSamples.length){if(svg)svg.setAttribute('viewBox','0 0 700 520');return}const xs=trackSamples.flatMap(p=>[+p.x,+p.roadLeftX,+p.roadRightX,+p.carX].filter(Number.isFinite)),ys=trackSamples.flatMap(p=>[+p.y,+p.roadLeftY,+p.roadRightY,+p.carY].filter(Number.isFinite));let minX=Math.min(...xs),maxX=Math.max(...xs),minY=Math.min(...ys),maxY=Math.max(...ys);const pad=50;let w=(maxX-minX)+pad*2,h=(maxY-minY)+pad*2;if(!tight){w=Math.max(700,w);h=Math.max(520,h)}const cx=(minX+maxX)/2,cy=(minY+maxY)/2;svg.setAttribute('viewBox',`${(cx-w/2).toFixed(1)} ${(cy-h/2).toFixed(1)} ${w.toFixed(1)} ${h.toFixed(1)}`)}
async function saveTrackSvg(){try{const name=`chasehq-track-${new Date().toISOString().replace(/[:.]/g,'-')}`;const j=JSON.parse(await api('/api/v1/track/svg/export',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({name})}));const a=document.createElement('a');a.href=j.download;a.download=(j.name||name)+'.svg';a.click();$('trackStatus').textContent=`Saved SVG: ${j.name} | samples ${j.sampleCount}` }catch(e){$('trackStatus').textContent=e.message}}
async function sampleTrack(){if(!trackRecording)return;try{const j=JSON.parse(await api('/api/v1/track/record/sample',{method:'POST'}));renderTrackRecorder(j);if(trackSamples.length)$('trackTelemetry').textContent=JSON.stringify(trackSamples.at(-1),null,2)}catch(e){$('trackTelemetry').textContent=e.message}}
async function refreshCourseFollow(){try{const j=JSON.parse(await api('/api/v1/course/follow'));$('courseFollowState').textContent=`Follower: ${j.enabled?'ON':'OFF'} | ${j.controller}/${j.mode} | target ${j.steeringTarget} | centre ${j.roadCentre} | car ${j.carLateral} | error ${j.lateralError} (${Number(j.lateralErrorNormalized||0).toFixed(3)}) | speed target ${j.speedTarget}`;$('courseStart').classList.toggle('on',!!j.enabled);$('courseController').value=j.controller||'hybrid';$('courseBias').value=j.lateralBias??0;$('courseLookahead').value=j.lookahead??8;$('courseSpeedControl').checked=!!j.speedControl}catch(e){$('courseFollowState').textContent=e.message}}
async function configureCourse(){const body={controller:$('courseController').value,bias:+$('courseBias').value,lookahead:+$('courseLookahead').value,speedControl:$('courseSpeedControl').checked};const j=JSON.parse(await api('/api/v1/course/follow/configure',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)}));await refreshCourseFollow();return j}
$('courseApply').onclick=async()=>{try{await configureCourse()}catch(e){$('courseFollowState').textContent=e.message}};$('courseStart').onclick=async()=>{try{await configureCourse();await api('/api/v1/course/follow/start',{method:'POST'});await refreshCourseFollow()}catch(e){$('courseFollowState').textContent=e.message}};$('courseStop').onclick=async()=>{try{await api('/api/v1/course/follow/stop',{method:'POST'});await refreshCourseFollow()}catch(e){$('courseFollowState').textContent=e.message}};$('courseReset').onclick=async()=>{try{await api('/api/v1/course/follow/reset',{method:'POST'});await refreshCourseFollow()}catch(e){$('courseFollowState').textContent=e.message}};
$('courseSurveyStart').onclick=async()=>{try{await configureCourse();const j=JSON.parse(await api('/api/v1/course/survey/start',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({collisionResponse:false,timerHold:true})}));$('courseSurveyState').textContent='Survey: ON | timer held | collision response suppressed | recorder cleared/started';renderTrackRecorder({...j.track,samples:[]});await sampleTrack();await refreshCourseFollow()}catch(e){$('courseSurveyState').textContent=e.message}};$('courseSurveyStop').onclick=async()=>{try{const j=JSON.parse(await api('/api/v1/course/survey/stop',{method:'POST'}));$('courseSurveyState').textContent='Survey: OFF / prior timer+collision state restored';renderTrackRecorder({...j.track,samples:trackSamples});await refreshCourseFollow()}catch(e){$('courseSurveyState').textContent=e.message}};
$('trackMapExport').onclick=async()=>{try{const name=`stage1-course-map-${new Date().toISOString().replace(/[:.]/g,'-')}`;const j=JSON.parse(await api('/api/v1/track/map/export',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({name})}));$('trackStatus').textContent=`Mapping dataset: ${j.name} | samples ${j.sampleCount} | entity rows ${j.entityRows} | surface signatures ${j.surfaceSignatures}` }catch(e){$('trackStatus').textContent=e.message}};
refreshCourseFollow();setInterval(()=>{if(currentTab==='track')refreshCourseFollow()},1500);
function highlightScript(){}
function selectedScript(){const e=$('scriptText');return e.value.slice(e.selectionStart,e.selectionEnd)}function currentScriptLine(){const e=$('scriptText'),p=e.selectionStart,a=e.value.lastIndexOf('\n',p-1)+1,b=e.value.indexOf('\n',p);return e.value.slice(a,b<0?e.value.length:b)}
function substituteVars(line,vars){return line.replace(/\$\{([A-Za-z_][A-Za-z0-9_]*)\}/g,(m,k)=>vars[k]??m)}
function substituteVars(text,vars){return text.replace(/\$\{([A-Za-z_][A-Za-z0-9_]*)\}/g,(m,n)=>{if(!(n in vars))throw Error(`Undefined variable: ${n}`);return vars[n]})}
async function queryFunctionValueUncached(expr){
 const m=expr.trim().match(/^(getregions|getprofiles|getsprites|getpaletteentries|getcheckpoints|getcapabilities|getmaps|getpalettes|getregistry|gettrackrecords|getevidence|getobjects|getaudiochannels)\((.*)\)$/i);if(!m)throw Error('Unknown query function: '+expr);
 const fn=m[1].toLowerCase(),arg=m[2].trim();
 async function call(action,args=''){const body={script:`api ${action}${args?' '+args:''}`,stopOnError:true,validateOnly:false,transient:true},raw=await preflightWithTimeout(api('/api/v1/script',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)}),`preflight ${action}`),j=parseJsonResponse(raw,`preflight ${action}`);if(!j.ok)throw Error(j.results?.[0]?.error||('Query failed: '+action));const out=j.results?.[0]?.output;if(out===undefined||out===null)throw Error(`preflight ${action}: response contained no output`);return out}
 if(fn==='getregions'){const a=parseJsonResponse(await call('regions.list'),'getregions');return {list:a.map(x=>x.name),printed:JSON.stringify(a,null,2),action:'regions.list'}}
 if(fn==='getprofiles'){const a=parseJsonResponse(await call('profiles.list'),'getprofiles');return {list:a.map(x=>x.id),printed:JSON.stringify(a,null,2),action:'profiles.list'}}
 if(fn==='getregistry'){const a=parseJsonResponse(await call('registry.list'),'getregistry');return {list:a.map(x=>x.id),printed:JSON.stringify(a,null,2),action:'registry.list'}}
 if(fn==='gettrackrecords'){const args=arg?arg.replace(/,/g,' '):'';const a=parseJsonResponse(await call('track.records',args),'gettrackrecords');return {list:a.map(x=>x.index),printed:JSON.stringify(a,null,2),action:'track.records'+(args?' '+args:'')}}
 if(fn==='getevidence'){const a=parseJsonResponse(await call('evidence.names'),'getevidence');return {list:a.map(x=>x.name),printed:JSON.stringify(a,null,2),action:'evidence.names'}}
 if(fn==='getobjects'){const a=parseJsonResponse(await call('objects.list'),'getobjects');return {list:a.map(x=>x.id||x.name),printed:JSON.stringify(a,null,2),action:'objects.list'}}
 if(fn==='getaudiochannels'){const a=parseJsonResponse(await call('audio.channels'),'getaudiochannels');return {list:a.map(x=>x.id),printed:JSON.stringify(a,null,2),action:'audio.channels'}}

 if(fn==='getsprites'){const a=parseJsonResponse(await call('sprites.list'),'getsprites');return {list:a.map(x=>x.slot),printed:JSON.stringify(a,null,2),action:'sprites.list'}}
 if(fn==='getmaps'||fn==='getpalettes'){const action=fn==='getmaps'?'sprites.maps':'sprites.palettes',v=parseJsonResponse(await call(action),fn);return {list:v,printed:JSON.stringify(v,null,2),action}}
 if(fn==='getpaletteentries'){const bm=arg.match(/^bank\s*=\s*(\d+)$/i);if(!bm)throw Error('getpaletteentries expects bank=N');const a=parseJsonResponse(await call('palette.entries',`bank=${bm[1]}`),'getpaletteentries');return {list:a.map(x=>x.index),printed:JSON.stringify(a,null,2),action:`palette.entries bank=${bm[1]}`}}
 if(fn==='getcapabilities'){const raw=await call('capabilities'),v=raw.replace(/^OK\s*/,'').trim().split(/\s+/).filter(Boolean);return {list:v,printed:raw,action:'capabilities'}}
 if(fn==='getcheckpoints'){const raw=await call('checkpoint.list');let v=[];try{const j=JSON.parse(raw);v=(Array.isArray(j)?j:(j.checkpoints||[])).map(x=>typeof x==='string'?x:(x.file||x.name)).filter(Boolean)}catch(_){v=raw.split(/\r?\n/).map(x=>(x.match(/(?:file|name)=([^\s]+)/)||[])[1]).filter(Boolean)}return {list:v,printed:raw,action:'checkpoint.list'}}
}


let preflightQueryCache=null,preflightQueryTotal=0,preflightQueryDone=0;
async function preflightWithTimeout(promise,label,ms=8000){let timer,cancelTimer;touchScriptProgress(label);try{return await Promise.race([promise,new Promise((_,reject)=>{timer=setTimeout(()=>reject(Error(`${label}: timed out after ${ms} ms`)),ms)}),new Promise((_,reject)=>{cancelTimer=setInterval(()=>{if(scriptAbort){clearInterval(cancelTimer);reject(Error('STOPPED_BY_USER'))}},50)})])}finally{if(timer)clearTimeout(timer);if(cancelTimer)clearInterval(cancelTimer)}}
function preflightKey(expr){return String(expr||'').trim().replace(/\s+/g,' ').toLowerCase()}
async function queryFunctionValue(expr){
 const key=preflightKey(expr);
 if(preflightQueryCache&&preflightQueryCache.has(key))return preflightQueryCache.get(key);
 if(preflightQueryCache){preflightQueryDone++;setScriptPhase('PREPARING',expr);setScriptState(`PREFLIGHT ${preflightQueryDone}/${Math.max(preflightQueryTotal,preflightQueryDone)} - ${expr}`,'warn');$('scriptProgress').textContent=`PREFLIGHT ${preflightQueryDone} / ${Math.max(preflightQueryTotal,preflightQueryDone)}\nCurrent query: ${expr}`}
 const value=await preflightWithTimeout(queryFunctionValueUncached(expr),`query ${expr}`);
 if(preflightQueryCache)preflightQueryCache.set(key,value);
 return value
}

async function collectionExpressionValue(expr){
 const t=expr.trim();let m=t.match(/^(count|first|last|unique)\((.+)\)$/i);if(m){const q=await queryFunctionValue(m[2].trim()),a=q.list;if(m[1].toLowerCase()==='count')return {value:String(a.length),printed:String(a.length)};if(m[1].toLowerCase()==='first')return {value:String(a[0]??''),printed:String(a[0]??'')};if(m[1].toLowerCase()==='last')return {value:String(a.at(-1)??''),printed:String(a.at(-1)??'')};const u=[...new Set(a.map(String))];return {value:u.join(','),printed:JSON.stringify(u,null,2)}}
 m=t.match(/^contains\((.+),\s*([^,()]+)\)$/i);if(m){const q=await queryFunctionValue(m[1].trim()),v=m[2].trim(),ok=q.list.map(String).includes(v);return {value:ok?'true':'false',printed:String(ok)}}
 throw Error('Unknown collection expression: '+expr)
}

async function resolveScriptQueryFunctions(source){
 const includeLines=source.replace(/\r/g,'').split('\n');let expanded=[];
 for(const ln of includeLines){
   const im=ln.trim().match(/^include\s+\"([^\"]+)\"$/i);
   if(im){
     setScriptPhase('PREPARING','include '+im[1]);setScriptState(`PREFLIGHT include - ${im[1]}`,'warn');$('scriptProgress').textContent=`PREFLIGHT\nLoading include: ${im[1]}`;
     const raw=await preflightWithTimeout(api('/api/v1/scripts/load',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({path:im[1]})}),`include ${im[1]}`);
     const j=parseJsonResponse(raw,`include ${im[1]}`);if(typeof j.script!=='string')throw Error(`include ${im[1]}: script text missing`);
     expanded.push(`# included ${im[1]}`);expanded.push(j.script)
   }else expanded.push(ln)
 }
 source=expanded.join('\n');
 const qre=/\b(getregions|getprofiles|getsprites|getpaletteentries|getcheckpoints|getcapabilities|getmaps|getpalettes|getregistry|gettrackrecords|getevidence|getobjects|getaudiochannels)\([^()]*\)/gi;
 const uniqueQueries=[...new Set([...source.matchAll(qre)].map(m=>preflightKey(m[0])))];
 preflightQueryCache=new Map();preflightQueryTotal=uniqueQueries.length;preflightQueryDone=0;
 $('scriptProgress').textContent=`PREFLIGHT 0 / ${preflightQueryTotal}`;
 if(preflightQueryTotal===0){
   $('scriptProgress').textContent='PREFLIGHT 0 / 0 - no dynamic queries; continuing immediately';
   preflightQueryCache=null;preflightQueryTotal=0;preflightQueryDone=0;
   return source;
 }
 try{
   const lines=source.replace(/\r/g,'').split('\n'),out=[],vars={};
   const sub=x=>x.replace(/\$\{([A-Za-z_][A-Za-z0-9_]*)\}/g,(m,k)=>k in vars?vars[k]:m);
   for(let i=0;i<lines.length;i++){
     const raw=lines[i],t=raw.trim();
     let cm=t.match(/^set\s+([A-Za-z_][A-Za-z0-9_]*)\s*=\s*((?:count|first|last|unique|contains)\(.*\))\s*$/i);
     if(cm){const c=await collectionExpressionValue(sub(cm[2]));vars[cm[1]]=c.value;out.push(`set ${cm[1]} = ${c.value}`);continue}
     cm=t.match(/^((?:count|first|last|unique|contains)\(.*\))\s*$/i);
     if(cm){const c=await collectionExpressionValue(sub(cm[1]));out.push(`echo ${c.printed.replace(/\n/g,' ')}`);continue}
     let m=t.match(/^set\s+([A-Za-z_][A-Za-z0-9_]*)\s*=\s*((?:getregions|getprofiles|getsprites|getpaletteentries|getcheckpoints|getcapabilities|getmaps|getpalettes|getregistry|gettrackrecords|getevidence|getobjects|getaudiochannels)\(.*\))\s*$/i);
     if(m){const expr=sub(m[2]),q=await queryFunctionValue(expr),value=q.list.join(',');vars[m[1]]=value;out.push(`set ${m[1]} = ${value}`);continue}
     m=t.match(/^set\s+([A-Za-z_][A-Za-z0-9_]*)\s*=\s*(.*)$/i);if(m){vars[m[1]]=sub(m[2].trim());out.push(raw);continue}
     m=t.match(/^((?:getregions|getprofiles|getsprites|getpaletteentries|getcheckpoints|getcapabilities|getmaps|getpalettes|getregistry|gettrackrecords|getevidence|getobjects|getaudiochannels)\(.*\))\s*$/i);if(m){const q=await queryFunctionValue(sub(m[1]));out.push(`api ${q.action}`);continue}
     out.push(raw)
   }
   return out.join('\n')
 }finally{preflightQueryCache=null;preflightQueryTotal=0;preflightQueryDone=0}
}

function compileClientScript(source,seedVars={}){
 const lines=source.replace(/\r/g,'').split('\n'),vars={...seedVars},out=[];
 function findBlock(open){let depth=0,elseAt=-1,finallyAt=-1;for(let j=open+1;j<lines.length;j++){const q=lines[j].trim();if(/^(for\s|repeat\s|if\s|try$)/i.test(q))depth++;else if(q==='end'){if(depth===0)return {end:j,elseAt,finallyAt};depth--}else if(q==='else'&&depth===0)elseAt=j;else if(q==='finally'&&depth===0)finallyAt=j}throw Error(`line ${open+1}: missing end`)}
 function compileSlice(a,b,localVars){const text=lines.slice(a,b).join('\n');return compileClientScript(text,localVars)}
 for(let i=0;i<lines.length;i++){
   let t=lines[i].trim();if(!t||t.startsWith('#'))continue;if(t==='end'||t==='else'||t==='finally')throw Error(`line ${i+1}: unexpected ${t}`);
   let m=t.match(/^set\s+([A-Za-z_][A-Za-z0-9_]*)\s*=\s*(.*)$/i);if(m){vars[m[1]]=substituteVars(m[2].trim(),vars);continue}
   m=t.match(/^for\s+([A-Za-z_][A-Za-z0-9_]*)\s+in\s+(.+)$/i);if(m){const blk=findBlock(i),name=m[1],src=substituteVars(m[2].trim(),vars),list=(vars[src]??src).split(',').map(x=>x.trim()).filter(Boolean);if(list.length>256)throw Error(`line ${i+1}: for loop exceeds 256 iterations`);for(const v of list)out.push(...compileSlice(i+1,blk.end,{...vars,[name]:v}));i=blk.end;continue}
   m=t.match(/^repeat\s+(\d+)$/i);if(m){const n=+m[1];if(n<0||n>256)throw Error(`line ${i+1}: repeat out of range`);const blk=findBlock(i);for(let k=1;k<=n;k++)out.push(...compileSlice(i+1,blk.end,{...vars,repeat_index:k}));i=blk.end;continue}
   m=t.match(/^if\s+(.+)$/i);if(m){const blk=findBlock(i);if(blk.finallyAt>=0)throw Error(`line ${i+1}: finally is only valid inside try`);const pred=substituteVars(m[1].trim(),vars),thenEnd=blk.elseAt>=0?blk.elseAt:blk.end,thenItems=compileSlice(i+1,thenEnd,{...vars}),elseItems=blk.elseAt>=0?compileSlice(blk.elseAt+1,blk.end,{...vars}):[];out.push({kind:'if',line:i+1,predicate:pred,thenItems,elseItems});i=blk.end;continue}
   if(/^try$/i.test(t)){const blk=findBlock(i);if(blk.finallyAt<0)throw Error(`line ${i+1}: try requires finally`);if(blk.elseAt>=0)throw Error(`line ${i+1}: else is not valid inside try`);const bodyItems=compileSlice(i+1,blk.finallyAt,{...vars}),finallyItems=compileSlice(blk.finallyAt+1,blk.end,{...vars});out.push({kind:'try',line:i+1,bodyItems,finallyItems});i=blk.end;continue}
   out.push({kind:'command',line:i+1,command:substituteVars(t,vars)})
 }
 function count(items){let n=0;for(const x of items){if(x.kind==='if')n+=count(x.thenItems)+count(x.elseItems);else if(x.kind==='try')n+=count(x.bodyItems)+count(x.finallyItems);else n++}return n}if(count(out)>1000)throw Error('Expanded script exceeds 1000 commands');return out
}
function parseComparable(v){const s=String(v).trim();if(/^0x[0-9a-f]+$/i.test(s))return parseInt(s,16);if(/^-?\d+$/.test(s))return Number(s);return s}
function compareValues(a,op,b){a=parseComparable(a);b=parseComparable(b);if(typeof a==='number'&&typeof b==='number'){if(op==='=')return a===b;if(op==='!=')return a!==b;if(op==='<')return a<b;if(op==='<=')return a<=b;if(op==='>')return a>b;if(op==='>=')return a>=b}if(op==='=')return String(a)===String(b);if(op==='!=')return String(a)!==String(b);throw Error('Relational comparison requires numeric values')}
async function evaluateScriptPredicate(pred){let m=pred.match(/^status\s+([A-Za-z_][A-Za-z0-9_.-]*)\s*(=|!=|<=|>=|<|>)\s*(\S+)$/i);if(m){const st=parseStatus(await api('/api/status'));if(!(m[1] in st))throw Error('Unknown status key: '+m[1]);return compareValues(st[m[1]],m[2],m[3])}m=pred.match(/^memory\s+cpu=(A|B)\s+address=([0-9A-Fa-fx]+)\s+width=(8|16|32)\s+value\s*(=|!=|<=|>=|<|>)\s*(0x[0-9A-Fa-f]+|[0-9A-Fa-f]+)$/i);if(m){const body={script:`api memory.read cpu=${m[1]} address=${m[2]} width=${m[3]}`,stopOnError:true,validateOnly:false,transient:true},j=JSON.parse(await api('/api/v1/script',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)})),raw=j.results?.[0]?.output||'';const vm=raw.match(/value=0x([0-9A-Fa-f]+)/);if(!vm)throw Error('Memory predicate read failed');return compareValues(parseInt(vm[1],16),m[4],parseInt(m[5].replace(/^0x/i,''),16))}m=pred.match(/^capability\s+(\S+)$/i);if(m){const body={script:'api capabilities',stopOnError:true,validateOnly:false,transient:true},j=JSON.parse(await api('/api/v1/script',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)})),raw=j.results?.[0]?.output||'';return new RegExp('(^|\\s)'+m[1].replace(/[.*+?^${}()|[\]\\]/g,'\\$&')+'(\\s|$)','i').test(raw)}throw Error('Invalid if predicate. Use: if status KEY OP VALUE | if memory cpu=A address=ADDR width=8|16|32 value OP HEX | if capability NAME')}
async function executeClientCommand(item,validateOnly=false,ignoreAbort=false){const line=item.command.trim();if(/^wait\s+\d+$/i.test(line)){const ms=+line.split(/\s+/)[1];if(ms<0||ms>5000)throw Error('wait must be 0..5000 ms');if(validateOnly)return `VALID wait ${ms}`;const end=performance.now()+ms;while(performance.now()<end){if(scriptAbort&&!ignoreAbort)throw Error('STOPPED_BY_USER');await new Promise(r=>setTimeout(r,Math.min(100,end-performance.now())))}return `OK waited ${ms}ms`}const sm=classifyScriptSource(serverRunSource,'run'),body={script:line,stopOnError:true,validateOnly,runId:serverRunId,runSource:serverRunSource,runName:sm.name||sm.label,runPurpose:sm.purpose||'',sourceType:sm.type,sourcePath:sm.path||''};const raw=await api('/api/v1/script',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)});const j=JSON.parse(raw);if(j.runId)serverRunId=j.runId;if(j.runPath)serverRunPath=j.runPath;if(j.bundle)serverRunBundle=j.bundle;if(j.bundleDownloadName)serverRunBundleName=j.bundleDownloadName;if(j.build)serverRunBuild=j.build;if(j.sessionId)serverRunSession=j.sessionId;if(!j.ok)throw Error(j.results?.[0]?.error||'Command failed');const out=j.results?.[0]?.output||'OK';if(out.length>200000)return `${out.slice(0,120000)}\n\n[Browser display truncated: ${out.length.toLocaleString()} characters; complete command output remains in the authoritative run console/bundle.]`;return out}
async function finalizeServerRun(status){if(!serverRunId)return null;const sm=classifyScriptSource(serverRunSource,'run'),body={script:'',stopOnError:true,validateOnly:false,runId:serverRunId,runSource:serverRunSource,runName:sm.name||sm.label,runPurpose:sm.purpose||'',sourceType:sm.type,sourcePath:sm.path||'',finalizeRun:true,runStatus:status};const j=JSON.parse(await api('/api/v1/script',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body),timeoutMs:60000}));if(j.runPath)serverRunPath=j.runPath;if(j.bundle)serverRunBundle=j.bundle;if(j.bundleDownloadName)serverRunBundleName=j.bundleDownloadName;if(j.build)serverRunBuild=j.build;if(j.sessionId)serverRunSession=j.sessionId;if(!j.ok&&status==='PASS')throw Error(j.error||'Run finalization failed');return j}
function countScriptCommands(items){let n=0;for(const x of items){if(x.kind==='if')n+=countScriptCommands(x.thenItems)+countScriptCommands(x.elseItems);else if(x.kind==='try')n+=countScriptCommands(x.bodyItems)+countScriptCommands(x.finallyItems);else n++}return n}
const SCRIPT_HISTORY_KEY='chq-script-run-history-v3';let authoritativeRunHistory=[];let currentRun=null;let selectedHistoryRunId=null;let defaultHistorySession='';let currentHistoryVersion='';let historyDefaultApplied=false;let loadedScriptPath='';let loadedScriptText='';let serverRunId='';let serverRunSource='';let serverRunPath='';let serverRunBundle='';let serverRunBundleName='';let serverRunBuild='@@CHQ_VERSION@@';let serverRunSession='';
function scriptMetadata(text){const m={};for(const line of String(text||'').replace(/\r/g,'').split('\n').slice(0,30)){const q=line.match(/^#\s*(name|purpose|category)\s*:\s*(.+?)\s*$/i);if(q)m[q[1].toLowerCase()]=q[2].trim()}return m}
function extractArtifacts(text){const out=[];const re=/[A-Za-z]:\\+(?:[^"\r\n])*?\.(?:png|zip|json|txt|csv|chqstate|bin|wav)/gi;for(const m of String(text||'').matchAll(re)){let x=m[0].replace(/\\\\+/g,'\\');if(!out.includes(x))out.push(x)}return out}
function artifactUrl(path,download=false){return '/api/v1/artifact?path='+encodeURIComponent(path)+(download?'&download=1':'')}
function openImageModal(paths,index=0){modalImages=paths.filter(p=>/\.png$/i.test(p));if(!modalImages.length)return;modalIndex=Math.max(0,Math.min(index,modalImages.length-1));const p=modalImages[modalIndex],name=p.split(/[\\/]/).pop();$('imageModalName').textContent=`${name} (${modalIndex+1}/${modalImages.length})`;$('imageModalImg').src=artifactUrl(p);$('imageModalDownload').href=artifactUrl(p,true);$('imageModalDownload').setAttribute('download',name);$('imageModal').classList.add('open')}
function closeImageModal(){$('imageModal').classList.remove('open');$('imageModalImg').src=''}
const MAX_LIVE_OUTPUT_BLOCKS=120,MAX_ARTIFACT_THUMBNAILS=24;
function artifactKind(path){const m=String(path||'').match(/\.([A-Za-z0-9]+)$/);return m?m[1].toUpperCase():'OTHER'}
function renderArtifacts(paths){const g=$('scriptArtifacts');g.innerHTML='';paths=[...new Set((paths||[]).filter(Boolean))];if(!paths.length)return;const counts={};for(const p of paths){const k=artifactKind(p);counts[k]=(counts[k]||0)+1}const summary=document.createElement('div');summary.className='artifact-summary';summary.innerHTML=`<b>${paths.length} artifact${paths.length===1?'':'s'}</b> - `+Object.keys(counts).sort().map(k=>`${escapeHtml(k)} ${counts[k]}`).join(' | ');g.appendChild(summary);if(!$('scriptThumbs').checked){const note=document.createElement('div');note.className='artifact-note';note.textContent='Individual artifact cards are hidden by default to keep long runs lightweight. Enable Show thumbnails to preview a bounded set; the authoritative run bundle retains every file.';g.appendChild(note);return}const images=paths.filter(p=>/\.png$/i.test(p)),nonImages=paths.filter(p=>!/\.png$/i.test(p)).slice(0,12),shownImages=images.slice(0,MAX_ARTIFACT_THUMBNAILS),shown=[...nonImages,...shownImages];for(const p of shown){const c=document.createElement('div');c.className='artifact-card';const name=p.split(/[\\/]/).pop();if(/\.png$/i.test(p)){const img=document.createElement('img');img.loading='lazy';img.src=artifactUrl(p);img.alt=name;img.onerror=()=>{img.style.display='none';const m=document.createElement('div');m.className='artifact-missing';m.textContent='Preview unavailable';c.insertBefore(m,c.firstChild)};img.onclick=()=>openImageModal(images,images.indexOf(p));c.appendChild(img)}const t=document.createElement('div');t.textContent=name;c.appendChild(t);const a=document.createElement('a');a.href=artifactUrl(p,true);a.download=name;a.textContent='Download';c.appendChild(a);g.appendChild(c)}if(paths.length>shown.length){const note=document.createElement('div');note.className='artifact-note';note.textContent=`Showing ${shown.length} of ${paths.length} artifacts (${shownImages.length} image previews max). Use the run bundle for the complete artifact set.`;g.appendChild(note)}}
function trimScriptOutputDom(){const el=$('scriptResult'),blocks=[...el.querySelectorAll('.script-command-block')];if(blocks.length<=MAX_LIVE_OUTPUT_BLOCKS)return;const remove=blocks.length-MAX_LIVE_OUTPUT_BLOCKS;for(let i=0;i<remove;i++)blocks[i].remove();let marker=el.querySelector('.script-output-elided');if(!marker){marker=document.createElement('div');marker.className='script-output-elided';el.prepend(marker)}const prior=Number(marker.dataset.count||0)+remove;marker.dataset.count=String(prior);marker.textContent=`${prior} older live output block${prior===1?'':'s'} elided from the browser view. Complete output remains in the authoritative run log/bundle.`}
function renderOutputBlock(text){const el=$('scriptResult'),block=document.createElement('div');block.className='script-command-block';const pre=document.createElement('pre');pre.className='script-command-text';pre.textContent=String(text??'');block.appendChild(pre);const arts=extractArtifacts(text),imgs=arts.filter(p=>/\.png$/i.test(p));if(imgs.length&&$('scriptThumbs').checked){const row=document.createElement('div');row.className='inline-artifacts';for(const p of imgs.slice(0,6)){const w=document.createElement('div');w.className='inline-artifact';const img=document.createElement('img');img.loading='lazy';img.src=artifactUrl(p);img.alt=p.split(/[\\/]/).pop();img.onerror=()=>{img.style.display='none';const m=document.createElement('span');m.className='artifact-missing';m.textContent='Preview unavailable';w.prepend(m)};img.onclick=()=>openImageModal(imgs,imgs.indexOf(p));w.appendChild(img);const label=document.createElement('div');label.textContent=img.alt;w.appendChild(label);row.appendChild(w)}block.appendChild(row)}el.appendChild(block);trimScriptOutputDom();el.scrollTop=el.scrollHeight}
function renderPlainOutput(text){const el=$('scriptResult');el.innerHTML='';scriptPlainOutput=String(text||'');for(const part of scriptPlainOutput.split(/\n\n+/)){if(part.trim())renderOutputBlock(part)}}
function appendScriptOutput(text){const part=String(text??'');if(!part)return;scriptPlainOutput+=(scriptPlainOutput?'\n\n':'')+part;renderOutputBlock(part)}
function deriveReleaseVersion(r){return String(r?.version||r?.build||'unknown')}
function normalizeLegacyRun(r){r={...r};r.version=String(r.version||r.build||'unknown');r.releaseVersion=r.version;if(r.serverKey)r.id=r.serverKey;if(!r.session)r.session='unknown';if(!r.sourceType)r.sourceType='legacy';return r}
function getRunHistory(){try{let h=JSON.parse(localStorage.getItem(SCRIPT_HISTORY_KEY)||'[]').map(normalizeLegacyRun);const old2=JSON.parse(localStorage.getItem('chq-script-run-history-v2')||'[]'),old1=JSON.parse(localStorage.getItem('chq-script-run-history-v1')||'[]');const all=[...authoritativeRunHistory.map(normalizeLegacyRun),...h,...old2.map(normalizeLegacyRun),...old1.map(normalizeLegacyRun)],key=r=>(r.serverKey||((r.session&&r.runId)?`${r.session}/${r.runId}`:''))?`run:${r.serverKey||`${r.session}/${r.runId}`}`:`local:${r.id||''}|${r.at||''}|${r.script||''}|${r.duration||''}`,merged=[],seen=new Set();for(const r of all){const k=key(r);if(seen.has(k))continue;seen.add(k);merged.push(r)}merged.sort((a,b)=>Date.parse(b.at||0)-Date.parse(a.at||0));saveRunHistory(merged.filter(r=>!r.serverKey));return merged}catch(e){$('scriptProgress').textContent='Local history cache could not be read: '+e.message;return authoritativeRunHistory||[]}}
async function refreshAuthoritativeRunHistory(){try{
 const meta=JSON.parse(await api('/api/v1/script/runs?limit=1'));
 defaultHistorySession=String(meta.currentSession||'');currentHistoryVersion=String(meta.build||'');
 if(!historyDefaultApplied){historyDefaultApplied=true;if(defaultHistorySession){const q=$('scriptHistorySession');q.innerHTML='<option value="">All sessions</option><option>'+escapeHtml(defaultHistorySession)+'</option>';q.value=defaultHistorySession}}
 const f=currentHistoryFilters(),query=new URLSearchParams({limit:'100'});if(f.session)query.set('session',f.session);if(f.version)query.set('version',f.version);if(f.status)query.set('status',f.status);
 const j=JSON.parse(await api('/api/v1/script/runs?'+query));authoritativeRunHistory=Array.isArray(j.runs)?j.runs.map(normalizeLegacyRun):[];renderRunHistory();return authoritativeRunHistory
 }catch(e){$('scriptProgress').textContent='Run-history refresh failed: '+e.message;return []}}
function saveRunHistory(h){try{const compact=h.slice(0,50).map(({output,sourceText,artifacts,...r},i)=>i<5?{...r,output:String(output||'').slice(0,32768),sourceText:String(sourceText||'').slice(0,65536),artifacts:(artifacts||[]).slice(0,100)}:r);localStorage.setItem(SCRIPT_HISTORY_KEY,JSON.stringify(compact))}catch(e){$('scriptProgress').textContent='Local history cache unavailable: '+e.message}}
function sessionFromPath(p){const m=String(p||'').replace(/\\/g,'/').match(/\/evidence\/sessions\/([^/]+)/i);return m?m[1]:''}
function currentHistoryFilters(){return {version:$('scriptHistoryVersion')?.value||'',status:$('scriptHistoryStatus')?.value||'',session:$('scriptHistorySession')?.value||''}}
function refreshHistoryFilters(h){const vs=[...new Set([...h.map(r=>r.releaseVersion||deriveReleaseVersion(r)),currentHistoryVersion].filter(Boolean))].sort().reverse(),ss=[...new Set([...h.map(r=>r.session).filter(Boolean),...(defaultHistorySession?[defaultHistorySession]:[])])];const v=$('scriptHistoryVersion'),q=$('scriptHistorySession'),pv=v?.value||'',pq=q?.value||'';if(v){v.innerHTML='<option value="">All versions</option>'+vs.map(x=>`<option>${escapeHtml(x)}</option>`).join('');v.value=vs.includes(pv)?pv:''}if(q){q.innerHTML='<option value="">All sessions</option>'+ss.map(x=>`<option>${escapeHtml(x)}</option>`).join('');if(!historyDefaultApplied&&defaultHistorySession&&ss.includes(defaultHistorySession)){q.value=defaultHistorySession;historyDefaultApplied=true}else q.value=ss.includes(pq)?pq:''}}
function renderSelectedRunActions(r){const a=$('scriptHistoryActions');a.innerHTML='';if(!r){a.innerHTML='<span class="muted">Select a run to view actions.</span>';return}const addButton=(label,fn)=>{const b=document.createElement('button');b.textContent=label;b.onclick=fn;a.appendChild(b)};addButton('View source',async()=>{renderPlainOutput(r.serverKey?await api(artifactUrl(r.runPath+'/script.chqscript')):(r.sourceText||''))});addButton('Reload source',async()=>{$('scriptText').value=r.serverKey?await api(artifactUrl(r.runPath+'/script.chqscript')):(r.sourceText||'');loadedScriptPath='';loadedScriptText='';highlightScript();$('scriptText').focus()});addButton('Download source',async()=>{const source=r.serverKey?await api(artifactUrl(r.runPath+'/script.chqscript')):(r.sourceText||'');const blob=new Blob([source],{type:'text/plain;charset=utf-8'}),x=document.createElement('a');x.href=URL.createObjectURL(blob);x.download=`${r.name||'script-run'}.chqscript`;x.click();setTimeout(()=>URL.revokeObjectURL(x.href),1000)});if(r.artifacts?.length)addButton('Open artifacts',()=>renderArtifacts(r.artifacts));addButton('Open in Evidence',()=>{document.querySelector('[data-tab=\"evidence\"]').click();loadEvidence(false)});if(r.runPath)addButton('Open artifact location',async()=>{try{await api('/api/v1/open-folder',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({path:r.runPath})})}catch(x){renderPlainOutput('Open artifact location failed: '+x.message)}});if(r.bundle){const x=document.createElement('a');x.textContent='Download bundle';x.href=artifactUrl(r.bundle,true)+(r.bundleDownloadName?'&name='+encodeURIComponent(r.bundleDownloadName):'');x.setAttribute('download',r.bundleDownloadName||String(r.bundle).split(/[\\/]/).pop());a.appendChild(x)}}
function renderRunHistory(){const el=$('scriptHistory'),h=getRunHistory();refreshHistoryFilters(h);const f=currentHistoryFilters(),rows=h.filter(r=>(!f.version||(r.releaseVersion||deriveReleaseVersion(r))===f.version)&&(!f.status||String(r.status||'').toUpperCase()===f.status)&&(!f.session||r.session===f.session));el.innerHTML=rows.length?'':'No recorded runs for this filter.';for(const r of rows){const d=document.createElement('div');d.className='history-row'+(selectedHistoryRunId===r.id?' selected':'');const desc=r.purpose?`<br><small class="muted">${escapeHtml(r.purpose)}</small>`:'';const rv=r.releaseVersion||deriveReleaseVersion(r);d.innerHTML=`<span>${escapeHtml(r.at)}</span><span class="${r.status==='PASS'?'good':'bad'}">${escapeHtml(r.status)}</span><span>${escapeHtml(rv||r.version||'unknown')}</span><span title="${escapeHtml(r.session||'')}">${escapeHtml(r.session||'unknown')}</span><span><b>${escapeHtml((r.sourceType||'legacy').toUpperCase())}</b> ${escapeHtml(r.name||r.script||'ad-hoc')} (${r.commands||0} commands, ${Number.isFinite(+r.artifactCount)?r.artifactCount:(r.artifacts||[]).length} artifacts)${desc}<br><small class="muted">${r.sourceHash?escapeHtml(r.sourceHash.slice(0,16))+'...':''}</small></span><span>${escapeHtml(r.duration)}s</span>`;d.onclick=async()=>{selectedHistoryRunId=r.id;const output=r.serverKey?await api(artifactUrl(r.runPath+'/console-output.txt')):(r.output||'');renderPlainOutput(output);r.artifacts=r.artifacts?.length?r.artifacts:extractArtifacts(output);renderArtifacts(r.artifacts);renderRunHistory();renderSelectedRunActions(r)};el.appendChild(d)}const sel=rows.find(r=>r.id===selectedHistoryRunId);renderSelectedRunActions(sel||null)}
function classifyScriptSource(text,mode='run'){const meta=scriptMetadata(text),cur=String(text||'');if(mode==='selection')return {type:'selection',label:meta.name||'selection',...meta};if(mode==='line')return {type:'line',label:meta.name||'current line',...meta};if(loadedScriptPath&&cur===loadedScriptText)return {type:'library',label:meta.name||loadedScriptPath,path:loadedScriptPath,...meta};if(loadedScriptPath)return {type:'modified',label:meta.name||`modified from ${loadedScriptPath}`,path:loadedScriptPath,...meta};return {type:'pasted',label:meta.name||'Pasted / ad-hoc script',...meta}}
async function recordRun(status,duration,commands,output,scriptText='',mode='run'){const artifacts=extractArtifacts(output),src=classifyScriptSource(scriptText,mode),hash=await hashScriptText(scriptText);const h=getRunHistory();h.unshift({id:Date.now(),at:new Date().toISOString(),status,script:src.label,name:src.name||src.label,purpose:src.purpose||'',sourceType:src.type,sourcePath:src.path||'',sourceHash:hash,duration,commands,artifacts,output,sourceText:String(scriptText||''),session:serverRunSession||'unknown',version:serverRunBuild||'@@CHQ_VERSION@@',runId:serverRunId,runPath:serverRunPath,bundle:serverRunBundle,bundleDownloadName:serverRunBundleName});saveRunHistory(h);selectedHistoryRunId=h[0].id;renderRunHistory();renderArtifacts(artifacts);await refreshAuthoritativeRunHistory();selectedHistoryRunId=serverRunSession+'/'+serverRunId;renderRunHistory()}
async function hashScriptText(text){try{const b=new TextEncoder().encode(String(text||'')),h=await crypto.subtle.digest('SHA-256',b);return [...new Uint8Array(h)].map(x=>x.toString(16).padStart(2,'0')).join('')}catch{return ''}}
function setScriptRunUi(running,validateOnly=false){for(const id of ['scriptRun','scriptValidate','scriptRunSelection','scriptRunLine']){const el=$(id);if(el)el.disabled=!!running}const cancel=$('scriptCancel');if(cancel)cancel.disabled=!running;const force=$('scriptForceReset');if(force)force.disabled=false}
function setScriptState(text,kind=''){const el=$('scriptState');if(el){el.textContent='Script: '+text;el.className='badge '+kind}}
function setScriptPhase(phase,context=''){scriptPhase=phase;scriptLastContext=context||scriptLastContext;scriptLastProgressAt=Date.now();const h=$('scriptRunnerHealth');if(h){h.textContent=`Runner: ${phase}${context?' | '+context:''}`;h.className='badge '+(['FAILED','STALLED'].includes(phase)?'warn':phase==='IDLE'||phase==='DONE'?'ok':'')}}
function touchScriptProgress(context=''){scriptLastProgressAt=Date.now();if(context)scriptLastContext=context;const h=$('scriptRunnerHealth');if(h&&scriptRunning)h.textContent=`Runner: ${scriptPhase}${scriptLastContext?' | '+scriptLastContext:''}`}
async function reportFrontendFault(error,extra={}){const e=error instanceof Error?error:Error(String(error));scriptLastFault=e.message;const src=classifyScriptSource($('scriptText')?.value||'','run');const payload={created:new Date().toISOString(),build:'@@CHQ_VERSION@@',phase:scriptPhase,context:scriptLastContext,script:src.label,sourceType:src.type,message:e.message,stack:e.stack||'',lastApi:requestHistory.at(-1)||null,...extra};try{await fetch('/api/v1/frontend-log',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(payload)})}catch(_){}return payload}
async function createDiagnosticBundle(error){const e=error instanceof Error?error:Error(String(error));try{const src=classifyScriptSource($('scriptText')?.value||'','run');const raw=await api('/api/v1/diagnostics/bundle',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({phase:scriptPhase,context:scriptLastContext,message:e.message,scriptPath:src.path||'ad-hoc',script:$('scriptText')?.value||'',output:scriptPlainOutput}),timeoutMs:15000});return parseJsonResponse(raw,'diagnostic bundle')}catch(_){return null}}
function forceResetScriptRunner(reason='manual reset'){scriptAbort=true;scriptRunning=false;preflightQueryCache=null;preflightQueryTotal=0;preflightQueryDone=0;setScriptRunUi(false,false);setScriptPhase('IDLE','');setScriptState('IDLE','');const p=$('scriptProgress');if(p)p.textContent=`Runner reset: ${reason}`;const c=$('conn');if(c&&c.textContent==='BUSY'){c.textContent='RECOVERED';c.className='badge warn'}}
function validateScriptRunnerDom(){const ids=['scriptRun','scriptValidate','scriptRunSelection','scriptRunLine','scriptCancel','scriptForceReset','scriptState','scriptRunnerHealth','scriptProgress','scriptResult','scriptText'];const missing=ids.filter(id=>!$(id));if(missing.length)throw Error('Script runner DOM self-test failed; missing: '+missing.join(', '));for(const name of ['setScriptRunUi','setScriptPhase','touchScriptProgress','reportFrontendFault','createDiagnosticBundle','forceResetScriptRunner','runScriptText']){if(eval('typeof '+name)!=='function')throw Error('Script runner helper self-test failed: '+name)}return true}
window.addEventListener('error',e=>{reportFrontendFault(e.error||Error(e.message),{source:'window.error',file:e.filename,line:e.lineno,column:e.colno});if(scriptRunning){setScriptPhase('FAILED','frontend exception');setScriptState('FAILED frontend','warn');forceResetScriptRunner('frontend exception: '+e.message)}});
window.addEventListener('unhandledrejection',e=>{const err=e.reason instanceof Error?e.reason:Error(String(e.reason));reportFrontendFault(err,{source:'unhandledrejection'});if(scriptRunning){setScriptPhase('FAILED','unhandled promise');setScriptState('FAILED frontend','warn');forceResetScriptRunner('unhandled promise: '+err.message)}});
setInterval(()=>{if(!scriptRunning)return;const age=Date.now()-scriptLastProgressAt;if(age>SCRIPT_WATCHDOG_MS&&scriptPhase!=='STALLED'){setScriptPhase('STALLED',scriptLastContext||'no progress');const p=$('scriptProgress');if(p)p.textContent+=`\nWATCHDOG: no script-runner progress for ${Math.round(age/1000)}s. Use Stop script or Force reset runner.`}},5000);
async function runScriptText(text,validateOnly=false,mode='run'){
 if(scriptRunning){renderPlainOutput('A script is already running.');return}
 const originalText=String(text||'');serverRunId='';serverRunSource=originalText;serverRunPath='';serverRunBundle='';serverRunBundleName='';serverRunBuild='@@CHQ_VERSION@@';serverRunSession='';scriptRunning=true;scriptAbort=false;setScriptRunUi(true,validateOnly);setScriptPhase('PREPARING','query preflight');setScriptState('PREFLIGHT...','warn');
 let commands=[],ticker=null,started=performance.now(),done=0,current='',total=0;
 try{
   text=await resolveScriptQueryFunctions(text);if(scriptAbort)throw Error('STOPPED_BY_USER');touchScriptProgress('compile');
   commands=compileClientScript(text);if(scriptAbort)throw Error('STOPPED_BY_USER');
   $('scriptResult').innerHTML='';scriptPlainOutput='';total=countScriptCommands(commands);started=performance.now();
   setScriptPhase(validateOnly?'VALIDATING':'RUNNING','starting');setScriptState(validateOnly?'VALIDATING...':'RUNNING...','warn');
   ticker=setInterval(()=>{const sec=((performance.now()-started)/1000).toFixed(1);setScriptState(`${validateOnly?'VALIDATING':'RUNNING'}... ${sec}s`,'warn');if(current)$('scriptProgress').textContent=`${validateOnly?'VALIDATING':'RUNNING'} ${done} / ${total}\nCurrent: ${current}\nElapsed: ${sec} s`;touchScriptProgress(current||'waiting')},250);
   async function runItems(items,validate,cleanupMode=false){for(const item of items){if(scriptAbort&&!cleanupMode)throw Error('STOPPED_BY_USER');if(item.kind==='if'){let take=true;if(!validate)take=await evaluateScriptPredicate(item.predicate);appendScriptOutput(`[${item.line}] if ${item.predicate}\n${validate?'VALID predicate':('condition='+take)}`);touchScriptProgress('if '+item.predicate);if(validate){await runItems(item.thenItems,true,cleanupMode);await runItems(item.elseItems,true,cleanupMode)}else await runItems(take?item.thenItems:item.elseItems,false,cleanupMode);continue}if(item.kind==='try'){appendScriptOutput(`[${item.line}] try\n${validate?'VALID try/finally':'enter try'}`);let primary=null,cleanupError=null;try{await runItems(item.bodyItems,validate,cleanupMode)}catch(e){primary=e}try{await runItems(item.finallyItems,validate,true)}catch(e){cleanupError=e;appendScriptOutput(`[${item.line}] finally\nCLEANUP ERROR: ${e.message}`)}if(primary){if(cleanupError)primary.cleanupError=cleanupError.message;throw primary}if(cleanupError)throw cleanupError;continue}done++;current=item.command;touchScriptProgress(current);try{const out=await executeClientCommand(item,validate,cleanupMode);appendScriptOutput(`[${item.line}] ${item.command}\n${out}`)}catch(e){if(scriptAbort&&!cleanupMode)e=Error('STOPPED_BY_USER');appendScriptOutput(`[${item.line}] ${item.command}\n${e.message==='STOPPED_BY_USER'?'STOPPED BY USER':'ERROR: '+e.message}`);e.scriptLine=item.line;if(cleanupMode||$('scriptStop').checked||e.message==='STOPPED_BY_USER')throw e}}}
   await runItems(commands,validateOnly);
   if(ticker){clearInterval(ticker);ticker=null}
   const sec=((performance.now()-started)/1000).toFixed(1);
   if(validateOnly){setScriptPhase('DONE',`${done} executed`);$('scriptProgress').textContent=`VALIDATION COMPLETE ${done} commands in ${sec} s`;setScriptState(`VALIDATION COMPLETE ${sec}s`,'ok')}
   else{setScriptPhase('FINALIZING','one run bundle');setScriptState(`FINALIZING... ${sec}s`,'warn');$('scriptProgress').textContent=`FINALIZING ${done} executed commands after ${sec} s\nCreating one authoritative run bundle...`;touchScriptProgress('finalizing one run bundle');await finalizeServerRun('PASS');await recordRun('PASS',sec,done,scriptPlainOutput,originalText,mode);setScriptPhase('DONE',`${done} executed`);$('scriptProgress').textContent=`COMPLETE ${done} executed commands in ${sec} s\nFinal run bundle created.`;setScriptState(`COMPLETE ${sec}s`,'ok');setTimeout(async()=>{const warnings=[];try{await refresh()}catch(e){warnings.push('refresh: '+e.message)}try{refreshPreview()}catch(e){warnings.push('preview: '+e.message)}try{await loadEvidence(true)}catch(e){warnings.push('evidence: '+e.message)}if(warnings.length)$('scriptProgress').textContent+=`\nPost-run warning: ${warnings.join(' | ')}`},0)}
 }catch(e){
   if(ticker){clearInterval(ticker);ticker=null}
   const sec=((performance.now()-started)/1000).toFixed(1);if(e.message==='STOPPED_BY_USER'){setScriptPhase('CANCELLED','user');$('scriptProgress').textContent=`STOPPED after ${sec} s`;setScriptState(`STOPPED ${sec}s`,'warn');if(!validateOnly&&serverRunId){try{await finalizeServerRun('CANCELLED')}catch(fe){$('scriptProgress').textContent+=`\nFinalization warning: ${fe.message}`}}}else{setScriptPhase('FAILED',e.scriptLine?'line '+e.scriptLine:'runner');$('scriptProgress').textContent=`FAILED${e.scriptLine?' line '+e.scriptLine:''} after ${sec} s: ${e.message}`;setScriptState(`FAILED${e.scriptLine?' line '+e.scriptLine:''}`,'warn');await reportFrontendFault(e,{source:'runScriptText',command:current,done,total});const diag=await createDiagnosticBundle(e);if(diag?.zip){appendScriptOutput(`DIAGNOSTIC ZIP: ${diag.zip}`);$('scriptProgress').textContent+=`\nDiagnostic bundle: ${diag.zip}`}if(!validateOnly){if(serverRunId){try{await finalizeServerRun('FAIL')}catch(fe){$('scriptProgress').textContent+=`\nFinalization warning: ${fe.message}`}}await recordRun('FAIL',sec,done,scriptPlainOutput,originalText,mode)}}
 }finally{if(ticker)clearInterval(ticker);scriptRunning=false;setScriptRunUi(false,false);if(['PREPARING','RUNNING','VALIDATING','STALLED'].includes(scriptPhase))setScriptPhase('IDLE','')}
}
function legacyCopyNow(text){let ta=null;try{ta=document.createElement('textarea');ta.value=text;ta.style.position='fixed';ta.style.left='0';ta.style.top='0';ta.style.width='2px';ta.style.height='2px';ta.style.opacity='0.01';ta.style.zIndex='99999';document.body.appendChild(ta);ta.focus({preventScroll:true});ta.select();ta.setSelectionRange(0,ta.value.length);const ok=!!(document.execCommand&&document.execCommand('copy'));return {ok,method:'legacy copy',error:ok?'':'document.execCommand(copy) returned false'}}catch(e){return {ok:false,error:e?.message||String(e)}}finally{if(ta&&ta.parentNode)ta.parentNode.removeChild(ta)}}
async function copyTextRobust(text){const legacy=legacyCopyNow(text);if(legacy.ok)return legacy;let apiError='';if(navigator.clipboard&&navigator.clipboard.writeText){try{await navigator.clipboard.writeText(text);return {ok:true,method:'Clipboard API'}}catch(e){apiError=e?.message||String(e)}}return {ok:false,error:[legacy.error,apiError].filter(Boolean).join(' | ')}}
function selectPreText(el){try{const sel=window.getSelection(),range=document.createRange();range.selectNodeContents(el);sel.removeAllRanges();sel.addRange(range);el.scrollIntoView({block:'nearest'});return true}catch(_){return false}}
let evidenceCatalog=[];
function renderEvidenceCards(){const list=$('evidenceList'),selected=$('evidenceSelect').value;list.innerHTML='';for(const e of evidenceCatalog.slice(0,16)){const d=document.createElement('div');d.className='evidence-item'+(e.name===selected?' selected':'');const title=document.createElement('b');title.textContent=e.name;const state=document.createElement('span');state.className='evidence-state '+(e.zipReady?'good':'muted');state.textContent=e.zipReady?'ZIP ready':'not packaged';const meta=document.createElement('div');meta.className='muted';meta.textContent=`frame ${e.frame??'?'} | ${e.created||''}`;d.append(title,state,meta);d.onclick=()=>{$('evidenceSelect').value=e.name;viewEvidence()};list.appendChild(d)}}
function parseSpriteLines(raw){const rows=[];for(const line of String(raw||'').split(/\r?\n/)){const m=line.match(/frame=(\d+)\s+slot=(\d+).*?map=([^\s]+)->([^\s]+).*?pal=([^\s]+)->([^\s]+).*?pos=([^,\s]+),([^\s]+)->([^,\s]+),([^\s]+)/);if(!m)continue;rows.push({frame:+m[1],slot:+m[2],map:m[4],pal:m[6],x:+m[9],y:+m[10],raw:line})}return rows}
function renderSpriteMonitor(rows){const latest=new Map();for(const r of rows){const prev=latest.get(r.slot);if(!prev||r.frame>=prev.frame)latest.set(r.slot,r)}const filter=($('spriteFilter')?.value||'').toLowerCase(),changedOnly=$('spriteChangedOnly')?.checked;const body=$('spriteRows');if(!body)return;body.innerHTML='';let changedCount=0;const sorted=[...latest.values()].sort((a,b)=>a.slot-b.slot);for(const r of sorted){const old=spriteState.get(r.slot),changes=[];if(old){if(old.map!==r.map)changes.push(`map ${old.map}->${r.map}`);if(old.pal!==r.pal)changes.push(`pal ${old.pal}->${r.pal}`);if(old.x!==r.x||old.y!==r.y)changes.push(`pos ${old.x},${old.y}->${r.x},${r.y}`)}else changes.push('new');if(changes.length)changedCount++;const hist=spriteHistoryBySlot.get(r.slot)||[];if(!old||changes.length){hist.push({...r,changes:[...changes]});if(hist.length>40)hist.shift();spriteHistoryBySlot.set(r.slot,hist)}const hay=`${r.slot} ${r.map} ${r.pal} ${r.x} ${r.y} ${changes.join(' ')}`.toLowerCase();if(filter&&!hay.includes(filter))continue;if(changedOnly&&!changes.length)continue;const tr=document.createElement('tr');tr.className=!old?'sprite-new':changes.length?'sprite-changed':'';tr.innerHTML=`<td>${r.slot}</td><td>${r.frame}</td><td>${r.map}</td><td>${r.pal}</td><td>${r.x}</td><td>${r.y}</td><td>${changes.join(', ')||'-'}</td>`;tr.onclick=()=>{$('spriteLiveSlot').value=r.slot;const h=spriteHistoryBySlot.get(r.slot)||[];$('spriteHistory').textContent=`slot ${r.slot}\n`+h.slice().reverse().map(x=>`f=${x.frame} map=${x.map} pal=${x.pal} pos=${x.x},${x.y} ${x.changes?.join(', ')||''}`).join('\n')};body.appendChild(tr)}spriteState=latest;$('spriteSummary').textContent=`${sorted.length} latest slots | ${changedCount} changed/new this refresh`}
async function refreshSpriteMonitor(existingRaw=null){try{const raw=existingRaw??await api('/api/sprites');renderSpriteMonitor(parseSpriteLines(raw))}catch(e){if($('spriteHistory'))$('spriteHistory').textContent='Sprite monitor error: '+e.message}}
$('spriteRefresh').onclick=()=>refreshSpriteMonitor();$('spriteFilter').oninput=()=>refreshSpriteMonitor();$('spriteChangedOnly').onchange=()=>refreshSpriteMonitor();$('spriteClearHistory').onclick=()=>{spriteState=new Map();spriteHistoryBySlot=new Map();$('spriteHistory').textContent='Sprite history cleared.';refreshSpriteMonitor()};
async function runSpriteResearch(command){try{const body={script:command,stopOnError:true,validateOnly:false},j=JSON.parse(await api('/api/v1/script',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)}));if(!j.ok)throw Error(j.results?.[0]?.error||'Sprite action failed');$('spriteHistory').textContent=j.results?.[0]?.output||'OK';setTimeout(()=>refreshSpriteMonitor(),80)}catch(e){$('spriteHistory').textContent='Sprite action error: '+e.message}}
$('spriteLiveInspect').onclick=()=>runSpriteResearch(`api sprite.inspect slot=${+$('spriteLiveSlot').value}`);
$('spriteLiveApply').onclick=()=>{const slot=+$('spriteLiveSlot').value,m=$('spriteLiveMap').value,p=$('spriteLivePalette').value;let q=`api sprite.override slot=${slot}`;if(m!=='')q+=` map=${+m}`;if(p!=='')q+=` palette=${+p}`;runSpriteResearch(q)};
$('spriteLiveSolo').onclick=()=>runSpriteResearch(`api sprite.visual slot=${+$('spriteLiveSlot').value} mode=solo`);
$('spriteLiveHide').onclick=()=>runSpriteResearch(`api sprite.visual slot=${+$('spriteLiveSlot').value} mode=hide`);
$('spriteLiveFlash').onclick=()=>runSpriteResearch(`api sprite.visual slot=${+$('spriteLiveSlot').value} mode=flash`);
$('spriteLiveClear').onclick=async()=>{await runSpriteResearch('api sprite.override.clear');await runSpriteResearch('api sprite.visual.clear')};

async function loadFrameSnapshots(selectNewest=false){try{const j=JSON.parse(await api('/api/v1/frame/snapshots')),sel=$('frameSnapshotSelect'),old=sel.value;sel.innerHTML='';for(const x of j.snapshots){const o=document.createElement('option');o.value=x.name;o.textContent=`${x.name}${x.frame!=null?' | f'+x.frame:''}${x.origin==='run'?` | ${x.tree} | ${x.session}/r${x.runId}`:''}`;sel.appendChild(o)}if(!selectNewest&&[...sel.options].some(o=>o.value===old))sel.value=old;$('frameSnapshotResult').textContent=JSON.stringify(j,null,2);return j}catch(e){$('frameSnapshotResult').textContent=e.message}}
let frameLayerState=[];
async function drawFrameLayers(name){const canvas=$('frameSnapshotPreview'),ctx=canvas.getContext('2d');ctx.clearRect(0,0,canvas.width,canvas.height);ctx.fillStyle='#101010';ctx.fillRect(0,0,canvas.width,canvas.height);for(const x of frameLayerState){if(!x.visible)continue;const img=new Image();await new Promise((res,rej)=>{img.onload=res;img.onerror=rej;img.src=`/api/v1/frame/snapshot/${encodeURIComponent(name)}/asset/${x.rel}?t=${Date.now()}`});ctx.save();ctx.globalAlpha=x.opacity;ctx.drawImage(img,0,0,canvas.width,canvas.height);ctx.restore()}}
async function drawSnapshotAsset(name,rel){const canvas=$('frameSnapshotPreview'),ctx=canvas.getContext('2d'),img=new Image();await new Promise((res,rej)=>{img.onload=res;img.onerror=rej;img.src=`/api/v1/frame/snapshot/${encodeURIComponent(name)}/asset/${rel}?t=${Date.now()}`});ctx.clearRect(0,0,canvas.width,canvas.height);ctx.drawImage(img,0,0,canvas.width,canvas.height)}
function renderLayerControls(name){const root=$('frameSnapshotLayerToggles');root.innerHTML='';for(const [i,x] of frameLayerState.entries()){const row=document.createElement('div');row.className='row';const cb=document.createElement('input');cb.type='checkbox';cb.checked=x.visible;cb.onchange=()=>{x.visible=cb.checked;drawFrameLayers(name)};const label=document.createElement('span');label.textContent=x.rel.split('/').at(-1);label.style.minWidth='150px';const op=document.createElement('input');op.type='range';op.min='0';op.max='1';op.step='.05';op.value=String(x.opacity);op.oninput=()=>{x.opacity=+op.value;drawFrameLayers(name)};const solo=document.createElement('button');solo.textContent='Solo';solo.onclick=()=>{frameLayerState.forEach((q,j)=>q.visible=j===i);renderLayerControls(name);drawFrameLayers(name)};row.append(cb,label,op,solo);root.appendChild(row)}const all=document.createElement('button');all.textContent='Show all';all.onclick=()=>{frameLayerState.forEach(q=>{q.visible=true;q.opacity=1});renderLayerControls(name);drawFrameLayers(name)};const raw=document.createElement('button');raw.textContent='Raw text/HUD source';raw.onclick=()=>drawSnapshotAsset(name,'sources/text-hud.png');root.append(all,raw)}
let frameSpriteState=[];
async function drawFrameSprites(name){const canvas=$('frameSnapshotPreview'),ctx=canvas.getContext('2d'),base=new Image();await new Promise((res,rej)=>{base.onload=res;base.onerror=rej;base.src=`/api/v1/frame/snapshot/${encodeURIComponent(name)}/asset/scene-no-sprites.png?t=${Date.now()}`});ctx.clearRect(0,0,canvas.width,canvas.height);ctx.drawImage(base,0,0,canvas.width,canvas.height);for(const sp of frameSpriteState){if(!sp.visible)continue;const img=new Image();await new Promise((res,rej)=>{img.onload=res;img.onerror=rej;img.src=`/api/v1/frame/snapshot/${encodeURIComponent(name)}/asset/${sp.asset}?t=${Date.now()}`});ctx.save();ctx.globalAlpha=sp.opacity;ctx.drawImage(img,sp.x,sp.y,sp.width,sp.height);ctx.restore()}}
function renderSpriteControls(name){const root=$('frameSnapshotLayerToggles');root.innerHTML='';for(const [i,sp] of frameSpriteState.entries()){const row=document.createElement('div');row.className='row';const cb=document.createElement('input');cb.type='checkbox';cb.checked=sp.visible;cb.onchange=()=>{sp.visible=cb.checked;drawFrameSprites(name)};const label=document.createElement('span');label.textContent=`slot ${sp.slot} | map ${sp.map} | pal ${sp.palette} | pri ${sp.priority}`;label.style.minWidth='260px';const op=document.createElement('input');op.type='range';op.min='0';op.max='1';op.step='.05';op.value=String(sp.opacity);op.oninput=()=>{sp.opacity=+op.value;drawFrameSprites(name)};const solo=document.createElement('button');solo.textContent='Solo';solo.onclick=()=>{frameSpriteState.forEach((q,j)=>q.visible=j===i);renderSpriteControls(name);drawFrameSprites(name)};const hide=document.createElement('button');hide.textContent='Hide';hide.onclick=()=>{sp.visible=false;renderSpriteControls(name);drawFrameSprites(name)};row.append(cb,label,op,solo,hide);root.appendChild(row)}const all=document.createElement('button');all.textContent='Show all sprites';all.onclick=()=>{frameSpriteState.forEach(q=>{q.visible=true;q.opacity=1});renderSpriteControls(name);drawFrameSprites(name)};root.appendChild(all)}
async function viewFrameSnapshot(){const name=$('frameSnapshotSelect').value;if(!name)return;try{const m=JSON.parse(await api(`/api/v1/frame/snapshot/${encodeURIComponent(name)}/manifest`)),mode=$('frameSnapshotMode').value;$('frameSnapshotResult').textContent=JSON.stringify(m,null,2);if(mode==='layers'){frameLayerState=(m.reconstructionData?.reconstruction?.layers||[]).map(rel=>({rel,visible:true,opacity:1}));renderLayerControls(name);await drawFrameLayers(name)}else if(mode==='sprites'){const sj=JSON.parse(await api(`/api/v1/frame/snapshot/${encodeURIComponent(name)}/asset/sprites/sprites.json`));frameSpriteState=(sj.sprites||[]).map(x=>({...x,visible:true,opacity:1}));renderSpriteControls(name);await drawFrameSprites(name)}else{$('frameSnapshotLayerToggles').innerHTML='';await drawSnapshotAsset(name,mode)}}catch(e){$('frameSnapshotResult').textContent=e.message}}
$('frameSnapshotCapture').onclick=async()=>{try{const name=$('frameSnapshotName').value.trim();const j=JSON.parse(await api('/api/v1/frame/snapshot',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({name})}));$('frameSnapshotResult').textContent=JSON.stringify(j,null,2);await loadFrameSnapshots(true);$('frameSnapshotSelect').value=j.name;await viewFrameSnapshot()}catch(e){$('frameSnapshotResult').textContent=e.message}};
$('frameSnapshotList').onclick=()=>loadFrameSnapshots(false);$('frameSnapshotView').onclick=viewFrameSnapshot;$('frameSnapshotMode').onchange=viewFrameSnapshot;
async function loadEvidence(selectNewest=false){try{const j=JSON.parse(await api('/api/v1/evidence'));evidenceCatalog=j.captures||[];const sel=$('evidenceSelect'),old=sel.value;sel.innerHTML='';for(const e of evidenceCatalog){const o=document.createElement('option');o.value=e.name;o.textContent=`${e.name} | frame ${e.frame??'?'} | ${e.zipReady?'ZIP ready':'not packaged'}`;sel.appendChild(o)}if(!selectNewest&&[...sel.options].some(o=>o.value===old))sel.value=old;else if(sel.options.length)sel.selectedIndex=0;renderEvidenceCards();if(sel.value)await viewEvidence();else{$('evidenceSelected').textContent='Selected: none';$('evidenceDownload').style.display='none'}}catch(e){$('evidenceDetail').textContent=e.message}}
async function viewEvidence(){const n=$('evidenceSelect').value;if(!n)return;try{const j=JSON.parse(await api('/api/v1/evidence/'+encodeURIComponent(n)));$('evidenceSelected').textContent=`Selected: ${n} | ${j.zipReady?'ZIP ready':'ZIP not created'}`;$('evidenceDetail').textContent=JSON.stringify(j.manifest,null,2);$('evidenceShot').src='/api/v1/evidence/'+encodeURIComponent(n)+'/screenshot.png?t='+Date.now();$('evidenceShot').style.display='block';$('evidenceDownload').style.display=j.zipReady?'inline':'none';$('evidenceZip').textContent=j.zipReady?'Recreate ZIP':'Create ZIP';if(j.zipReady)$('evidenceDownload').href='/api/v1/evidence/'+encodeURIComponent(n)+'/download';renderEvidenceCards()}catch(e){$('evidenceDetail').textContent=e.message}}
$('evidenceSelect').onchange=viewEvidence;$('evidenceRefresh').onclick=()=>loadEvidence(false);$('evidenceView').onclick=viewEvidence;$('evidenceLoad').onclick=async()=>{const n=$('evidenceSelect').value;if(!n)return;try{$('evidenceDetail').textContent=await api('/api/v1/evidence/'+encodeURIComponent(n)+'/load',{method:'POST'});await refresh();refreshPreview()}catch(e){$('evidenceDetail').textContent=e.message}};$('evidenceZip').onclick=async()=>{const n=$('evidenceSelect').value;if(!n)return;try{const j=JSON.parse(await api('/api/v1/evidence/'+encodeURIComponent(n)+'/zip',{method:'POST'}));$('evidenceDetail').textContent=`ZIP ready: ${j.path}`;await loadEvidence(false);$('evidenceSelect').value=n;await viewEvidence()}catch(e){$('evidenceDetail').textContent=e.message}};$('evidenceBundle').onclick=async()=>{try{const prefix=$('evidenceBundlePrefix').value.trim(),name=$('evidenceBundleName').value.trim();const j=JSON.parse(await api('/api/v1/evidence/bundle',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({prefix,name})}));$('evidenceDetail').textContent=`Experiment bundle ready: ${j.path}\nCaptures: ${j.count}\n${(j.captures||[]).join('\n')}`;$('evidenceBundleDownload').href=j.download;$('evidenceBundleDownload').style.display='inline'}catch(e){$('evidenceDetail').textContent=e.message}};
let scriptLibraryItems=[];
function scriptCategoryOf(x){return (x.category||String(x.path||'').split('/')[0]||'uncategorized').toLowerCase()}
function updateScriptLibraryInfo(){const sel=$('scriptLibrarySelect'),x=scriptLibraryItems.find(v=>v.path===sel.value);if(!x){$('scriptLibraryInfo').textContent='No script selected.';return}const bits=[`Category: ${scriptCategoryOf(x)}`,`Path: ${x.path}`];if(x.status)bits.push(`Status: ${x.status}`);if(x.checkpoint)bits.push(`Checkpoint: ${x.checkpoint}`);if(x.requires)bits.push(`Requires: ${x.requires}`);$('scriptLibraryInfo').textContent=bits.join('  |  ')}
function renderScriptLibrary(){const sel=$('scriptLibrarySelect'),cat=$('scriptLibraryCategory').value.trim().toLowerCase(),ver=$('scriptLibraryVersion')?.value||'',scope=$('scriptLibraryScope')?.value||'current',q=$('scriptLibrarySearch').value.trim().toLowerCase(),old=sel.value;sel.innerHTML='';const currentBuild='@@CHQ_VERSION@@';const filtered=scriptLibraryItems.filter(x=>{if(cat&&scriptCategoryOf(x)!==cat)return false;if(ver&&(x.regressionVersion||'')!==ver)return false;if(scriptCategoryOf(x)==='regression'){const rs=(x.regressionScope||'permanent').toLowerCase();if(scope==='permanent'&&rs!=='permanent')return false;if(scope==='current'&&rs!=='permanent'&&(x.regressionVersion||'')!==currentBuild)return false}return !q||`${x.name||''} ${x.purpose||''} ${x.path||''} ${x.regressionVersion||''}`.toLowerCase().includes(q)});const groups=new Map();for(const x of filtered){const c=scriptCategoryOf(x);if(!groups.has(c))groups.set(c,[]);groups.get(c).push(x)}for(const c of [...groups.keys()].sort()){const g=document.createElement('optgroup');g.label=`${c.toUpperCase()} (${groups.get(c).length})`;for(const x of groups.get(c).sort((a,b)=>(a.name||a.path).localeCompare(b.name||b.path))){const o=document.createElement('option');o.value=x.path;o.textContent=(x.name||x.path)+(x.purpose?` - ${x.purpose}`:'');g.appendChild(o)}sel.appendChild(g)}if([...sel.options].some(o=>o.value===old))sel.value=old;updateScriptLibraryInfo()}
async function loadScriptLibrary(autoLoad=true){try{const j=JSON.parse(await api('/api/v1/scripts'));scriptLibraryItems=Array.isArray(j.scripts)?j.scripts:[];const cat=$('scriptLibraryCategory'),oldCat=cat.value;cat.innerHTML='<option value="">All categories</option>';for(const c of [...new Set(scriptLibraryItems.map(scriptCategoryOf))].sort()){const o=document.createElement('option');o.value=c;o.textContent=`${c.toUpperCase()} (${scriptLibraryItems.filter(x=>scriptCategoryOf(x)===c).length})`;cat.appendChild(o)}if([...cat.options].some(o=>o.value===oldCat))cat.value=oldCat;const v=$('scriptLibraryVersion'),oldVer=v?.value||'';if(v){const versions=[...new Set(scriptLibraryItems.map(x=>x.regressionVersion).filter(Boolean))].sort().reverse();v.innerHTML='<option value="">All versions</option>'+versions.map(x=>`<option>${escapeHtml(x)}</option>`).join('');if(versions.includes(oldVer))v.value=oldVer}renderScriptLibrary();if(autoLoad&&$('scriptLibrarySelect').value&&$('scriptLibrarySelect').value!==loadedScriptPath)await loadLibraryScript()}catch(e){renderPlainOutput('Script library: '+e.message)}}
async function applyScriptLibraryFilter(){renderScriptLibrary();if($('scriptLibrarySelect').value&&$('scriptLibrarySelect').value!==loadedScriptPath)await loadLibraryScript()}
async function loadLibraryScript(){const path=$('scriptLibrarySelect').value;if(!path)return;const b=$('scriptLibraryLoad');b.disabled=true;b.textContent='Loading...';try{const j=JSON.parse(await api('/api/v1/scripts/load',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({path})}));if(typeof j.script!=='string')throw Error('Script text missing from response');$('scriptText').value=j.script;loadedScriptPath=j.path;loadedScriptText=j.script;highlightScript();$('scriptLibraryName').value=j.path;renderPlainOutput(`Loaded ${j.path}`);$('scriptText').focus();updateScriptLibraryInfo()}catch(e){renderPlainOutput('Load failed: '+e.message)}finally{b.disabled=false;b.textContent='Load'}}
async function saveLibraryScript(){let path=$('scriptLibraryName').value.trim();if(!path){renderPlainOutput('Enter a library path such as ioc/my-script.chqscript');return}try{const j=JSON.parse(await api('/api/v1/scripts/save',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({path,script:$('scriptText').value})}));renderPlainOutput(`Saved ${j.path}`);await loadScriptLibrary(false);$('scriptLibrarySelect').value=j.path;await loadLibraryScript()}catch(e){renderPlainOutput(e.message)}}
async function scriptHelp(){try{const j=JSON.parse(await api('/api/v1/script/help'));renderPlainOutput(j.text)}catch(e){renderPlainOutput(e.message)}}
$('scriptText').addEventListener('paste',()=>{loadedScriptPath='';loadedScriptText=''});$('scriptValidate').onclick=()=>runScriptText($('scriptText').value,true,'validate');$('scriptRun').onclick=()=>runScriptText($('scriptText').value,false,'run');$('scriptRunSelection').onclick=()=>{const t=$('scriptText'),sel=t.value.slice(t.selectionStart,t.selectionEnd);if(!sel.trim()){renderPlainOutput('No script text selected.');return}runScriptText(sel,false,'selection')};$('scriptRunLine').onclick=()=>{const t=$('scriptText'),pos=t.selectionStart,a=t.value.lastIndexOf('\n',Math.max(0,pos-1))+1,b=t.value.indexOf('\n',pos),line=t.value.slice(a,b<0?t.value.length:b);if(!line.trim()){renderPlainOutput('Current line is empty.');return}runScriptText(line,false,'line')};$('scriptLibraryRefresh').onclick=loadScriptLibrary;$('scriptLibraryLoad').onclick=loadLibraryScript;$('scriptLibrarySelect').onchange=()=>{updateScriptLibraryInfo();loadLibraryScript()};$('scriptLibraryCategory').onchange=applyScriptLibraryFilter;$('scriptLibraryVersion').onchange=applyScriptLibraryFilter;$('scriptLibraryScope').onchange=applyScriptLibraryFilter;$('scriptLibrarySearch').oninput=applyScriptLibraryFilter;$('scriptLibrarySave').onclick=saveLibraryScript;$('scriptHelp').onclick=scriptHelp;$('scriptCancel').onclick=async()=>{scriptAbort=true;setScriptPhase('CANCELLING',scriptLastContext);setScriptState('CANCELLING...','warn');$('scriptProgress').textContent='Cancelling preflight/execution and pausing emulation...';try{await fetch('/api/v1/control/pause',{method:'POST'})}catch(_){} };$('scriptForceReset').onclick=()=>forceResetScriptRunner('manual force reset');$('scriptCopy').onclick=async()=>{const el=$('scriptResult'),text=scriptPlainOutput;if(!text){$('scriptProgress').textContent='Nothing to copy: script output is empty.';return}const r=await copyTextRobust(text);if(r.ok){$('scriptProgress').textContent=`Copied ${text.length} characters to clipboard (${r.method}).`}else{const selected=selectPreText(el);$('scriptProgress').textContent=`Clipboard copy was blocked by the browser (${r.error}). ${selected?'Output selected - press Ctrl+C.':'Use Download output instead.'}`}};$('scriptClear').onclick=()=>{$('scriptResult').innerHTML='';scriptPlainOutput='';$('scriptProgress').textContent='';renderArtifacts([])};$('scriptWrap').onchange=()=>$('scriptResult').classList.toggle('wrap',$('scriptWrap').checked);$('scriptThumbs').onchange=()=>{renderPlainOutput(scriptPlainOutput);renderArtifacts(extractArtifacts(scriptPlainOutput))};$('scriptMax').onclick=()=>{const w=$('scriptOutputWrap');w.classList.toggle('maximized');$('scriptMax').textContent=w.classList.contains('maximized')?'Restore output':'Maximize output'};$('scriptHistoryClear').onclick=()=>{localStorage.removeItem(SCRIPT_HISTORY_KEY);localStorage.removeItem('chq-script-run-history-v1');localStorage.removeItem('chq-script-run-history-v2');refreshAuthoritativeRunHistory()};$('scriptHistoryVersion').onchange=refreshAuthoritativeRunHistory;$('scriptHistoryStatus').onchange=refreshAuthoritativeRunHistory;$('scriptHistorySession').onchange=refreshAuthoritativeRunHistory;$('scriptHistoryRefresh').onclick=refreshAuthoritativeRunHistory;$('scriptHistoryCurrent').onclick=async()=>{await refreshAuthoritativeRunHistory();$('scriptHistoryVersion').value='';$('scriptHistoryStatus').value='';$('scriptHistorySession').value=defaultHistorySession;await refreshAuthoritativeRunHistory()};$('imageModalClose').onclick=closeImageModal;$('imageModal').onclick=e=>{if(e.target===$('imageModal'))closeImageModal()};$('imageModalPrev').onclick=()=>openImageModal(modalImages,(modalIndex-1+modalImages.length)%modalImages.length);$('imageModalNext').onclick=()=>openImageModal(modalImages,(modalIndex+1)%modalImages.length);document.addEventListener('keydown',e=>{if(e.key==='Escape')closeImageModal()});$('scriptDownload').onclick=()=>{const text=scriptPlainOutput;if(!text){$('scriptProgress').textContent='Nothing to download: script output is empty.';return}const blob=new Blob([text],{type:'text/plain;charset=utf-8'}),a=document.createElement('a');a.href=URL.createObjectURL(blob);a.download='chasehq-script-output.txt';a.click();$('scriptProgress').textContent=`Downloaded ${text.length} characters.`;setTimeout(()=>URL.revokeObjectURL(a.href),1000)};
function renderDiagnostics(){if(!$('diagRequests'))return;$('diagRequests').textContent=requestHistory.slice().reverse().map(r=>`${r.at} ${r.ok?'OK':'ERR'} ${r.ms} ms  ${r.path}${r.error?'  '+r.error:''}`).join('\n')||'No request timings yet.'}
async function refreshDiagnostics(){renderDiagnostics();try{const j=JSON.parse(await api('/api/health'));$('healthNative').textContent='Native: '+(j.native?.raw||'OK');$('healthNative').className='badge ok';$('healthWeb').textContent='Web: OK';$('healthWeb').className='badge ok';const l=JSON.parse(await api('/api/v1/logs'));const sel=$('diagLogSelect'),old=sel.value;sel.innerHTML='';for(const f of l.files){const o=document.createElement('option');o.value=f;o.textContent=f;sel.appendChild(o)}if([...sel.options].some(o=>o.value===old))sel.value=old;if(sel.value)$('diagTail').textContent=await api('/api/v1/logs/tail?file='+encodeURIComponent(sel.value)+'&lines='+encodeURIComponent($('diagLines').value))}catch(e){$('healthWeb').textContent='Web: DEGRADED';$('healthWeb').className='badge warn';$('diagTail').textContent=e.message}renderDiagnostics()}
$('diagRefresh').onclick=refreshDiagnostics;$('diagLogSelect').onchange=refreshDiagnostics;$('diagCopy').onclick=async()=>{const r=await copyTextRobust($('diagTail').textContent);if(!r.ok)$('diagTail').textContent+='\n\nCopy failed: '+r.error};setInterval(()=>{if(currentTab==='diagnostics'&&$('diagAuto')?.checked)refreshDiagnostics()},1000);
$('capture').onclick=async()=>{try{$('console').textContent='Capturing evidence...';const r=await api('/api/v1/evidence/capture',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({name:'evidence-'+Date.now()})});const j=JSON.parse(r);$('console').textContent=`Evidence captured\n${j.path}\nZIP: ${j.zip||'not created'}\nMachine state restored: ${j.resumed?'RUNNING':'PAUSED'}`;await refresh();refreshPreview();loadLogs();loadEvidence(true)}catch(e){$('console').textContent='Evidence capture failed: '+e.message}};
$('rate').onchange=restart;(async()=>{try{validateScriptRunnerDom();setScriptPhase('IDLE','self-test OK');requestAnimationFrame(pollGamepad);loadHandling();loadPalette();loadCheckpoints();loadLogs();loadEvidence();await loadScriptLibrary();if(!loadedScriptPath)throw Error('Script library discovered entries but did not auto-load the selected script');loadNextSteps();highlightScript();await refreshAuthoritativeRunHistory();setTab('dashboard');await refresh();restart();refreshPreview();restartPreview();await fetch('/api/v1/frontend-ready',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({ok:true,build:'@@CHQ_VERSION@@',created:new Date().toISOString(),scripts:scriptLibraryItems.length})});}catch(e){await reportFrontendFault(e,{source:'startup-self-test'});setScriptState('FAILED self-test','warn');setScriptPhase('FAILED','self-test');$('conn').textContent='FRONTEND FAILED';$('conn').className='badge warn';}})();
</script>
'@
$html=$html.Replace('@@CHQ_VERSION@@',$workbenchVersion)
$allowedFirst=@('layer-offset','tc0100-mode','memory','handling','tuning','palette','timer','window','status','pause','resume','step','run','read','write','mem','regs','reg','watch','patch','events','why','changed','input','sprites','snapshot','baseline','screenshot','checkpoint','research','trace','legacytrace','provenance','version','capabilities','disasm','disassemble')
function Invoke-SafeCommand([string]$line){$line=$line.Trim();if($line.Length -gt 512){throw 'Command too long'};$parts=$line -split '\s+';if($parts.Count -lt 1 -or $allowedFirst -notcontains $parts[0].ToLowerInvariant()){throw 'Command not allowed by web bridge'}; if($line -match '[;&|`$<>]'){throw 'Shell metacharacters are not allowed'}; Invoke-Chq $parts}
function Resolve-ScriptLibraryPath([string]$relative,[switch]$ForSave){
    if([string]::IsNullOrWhiteSpace($relative)){throw 'Script path is required'}
    $relative=$relative.Replace('\','/').TrimStart('/')
    if($relative -match '(^|/)\.\.(/|$)' -or $relative -notmatch '^[A-Za-z0-9_.\-/]+$'){throw 'Invalid script library path'}
    if(-not $relative.EndsWith('.chqscript',[StringComparison]::OrdinalIgnoreCase)){$relative += '.chqscript'}
    $full=[IO.Path]::GetFullPath((Join-Path $scriptLibraryRoot $relative.Replace('/','\')))
    $base=[IO.Path]::GetFullPath($scriptLibraryRoot)+[IO.Path]::DirectorySeparatorChar
    if(-not $full.StartsWith($base,[StringComparison]::OrdinalIgnoreCase)){throw 'Script path escapes library root'}
    if(-not $ForSave -and -not(Test-Path $full -PathType Leaf)){throw "Script not found: $relative"}
    return [pscustomobject]@{Relative=$relative;Full=$full}
}
function Get-ScriptLibrary(){
    $items=@()
    foreach($f in Get-ChildItem $scriptLibraryRoot -Filter '*.chqscript' -File -Recurse -ErrorAction SilentlyContinue | Sort-Object FullName){
        $rel=$f.FullName.Substring($scriptLibraryRoot.Length).TrimStart([char]'\',[char]'/').Replace('\','/')
        $head=@(Get-Content $f.FullName -Encoding UTF8 -TotalCount 24 -ErrorAction SilentlyContinue);$name=$f.BaseName;$purpose='';$checkpoint='';$category='';$status='';$requires='';$regressionVersion='';$regressionScope=''
        foreach($l in $head){
            if($l -match '^\s*#\s*name\s*:\s*(.+)$'){$name=$Matches[1].Trim()}
            if($l -match '^\s*#\s*purpose\s*:\s*(.+)$'){$purpose=$Matches[1].Trim()}
            if($l -match '^\s*#\s*checkpoint\s*:\s*(.+)$'){$checkpoint=$Matches[1].Trim()}
            if($l -match '^\s*#\s*category\s*:\s*(.+)$'){$category=$Matches[1].Trim()}
            if($l -match '^\s*#\s*status\s*:\s*(.+)$'){$status=$Matches[1].Trim()}
            if($l -match '^\s*#\s*requires\s*:\s*(.+)$'){$requires=$Matches[1].Trim()}
            if($l -match '^\s*#\s*regression-version\s*:\s*(.+)$'){$regressionVersion=$Matches[1].Trim()}
            if($l -match '^\s*#\s*regression-scope\s*:\s*(.+)$'){$regressionScope=$Matches[1].Trim().ToLowerInvariant()}
        }
        if([string]::IsNullOrWhiteSpace($category)){$category=($rel -split '/')[0]}
        if($category -ieq 'regression' -and [string]::IsNullOrWhiteSpace($regressionScope)){$regressionScope='permanent'}
        $items += [ordered]@{path=$rel;name=$name;purpose=$purpose;checkpoint=$checkpoint;category=$category;status=$status;requires=$requires;regressionVersion=$regressionVersion;regressionScope=$regressionScope;modified=$f.LastWriteTime.ToString('o')}
    }
    return @($items)
}
function Substitute-ScriptVariables([string]$text,[hashtable]$vars){
    return [regex]::Replace($text,'\$\{([A-Za-z_][A-Za-z0-9_]*)\}',{param($m)$n=$m.Groups[1].Value;if(-not $vars.ContainsKey($n)){throw "Undefined variable: $n"};[string]$vars[$n]})
}
function Find-ScriptBlockEnd([string[]]$lines,[int]$bodyStart,[int]$limit){
    $depth=0
    for($j=$bodyStart;$j -lt $limit;$j++){
        $t=$lines[$j].Trim()
        if($t -match '^(for\s+|repeat\s+)'){$depth++}
        elseif($t -eq 'end'){if($depth -eq 0){return $j};$depth--}
    }
    throw 'Missing end for loop block'
}
function Expand-ScriptRange([string[]]$lines,[int]$start,[int]$limit,[hashtable]$vars,[string[]]$context,[ref]$expandedCount,[int]$depth=0){
    if($depth -gt 4){throw 'Loop nesting exceeds 4 levels'}
    $out=@();$i=$start
    while($i -lt $limit){
        $raw=$lines[$i];$lineNo=$i+1;$line=$raw.Trim();$i++
        if(-not $line -or $line.StartsWith('#')){continue}
        if($line -eq 'end'){throw "Unexpected end at line $lineNo"}
        if($line -match '^set\s+([A-Za-z_][A-Za-z0-9_]*)\s*=\s*(.*)$'){
            $name=$Matches[1];$value=Substitute-ScriptVariables $Matches[2].Trim() $vars;$vars[$name]=$value;continue
        }
        if($line -match '^for\s+([A-Za-z_][A-Za-z0-9_]*)\s+in\s+(.+)$'){
            $var=$Matches[1];$source=$Matches[2].Trim();$end=Find-ScriptBlockEnd $lines $i $limit
            $resolved=if($vars.ContainsKey($source)){[string]$vars[$source]}else{Substitute-ScriptVariables $source $vars}
            $items=@($resolved -split ','|ForEach-Object{$_.Trim()}|Where-Object{$_ -ne ''})
            if($items.Count -gt 256){throw 'for loop exceeds 256 iterations'}
            foreach($item in $items){$v=@{};foreach($k in $vars.Keys){$v[$k]=$vars[$k]};$v[$var]=$item;$ctx=@($context)+("$var=$item");$out += @(Expand-ScriptRange $lines $i $end $v $ctx $expandedCount ($depth+1))}
            $i=$end+1;continue
        }
        if($line -match '^repeat\s+(.+)$'){
            $nText=Substitute-ScriptVariables $Matches[1].Trim() $vars;$n=0
            if(-not [int]::TryParse($nText,[ref]$n) -or $n -lt 0 -or $n -gt 256){throw 'repeat count must be 0..256'}
            $end=Find-ScriptBlockEnd $lines $i $limit
            for($r=1;$r -le $n;$r++){$v=@{};foreach($k in $vars.Keys){$v[$k]=$vars[$k]};$v['repeat_index']=$r;$ctx=@($context)+("repeat=$r/$n");$out += @(Expand-ScriptRange $lines $i $end $v $ctx $expandedCount ($depth+1))}
            $i=$end+1;continue
        }
        $cmd=Substitute-ScriptVariables $line $vars;$expandedCount.Value++
        if($expandedCount.Value -gt 1000){throw 'Expanded script exceeds 1000 commands'}
        $out += [pscustomobject]@{Line=$lineNo;Command=$cmd;Context=($context -join ' ')}
    }
    return @($out)
}
function Expand-ChqScript([string]$script){
    if($script.Length -gt 65536){throw 'Script too long (max 64 KiB)'}
    $lines=@($script -split "`r?`n");if($lines.Count -gt 1000){throw 'Script has more than 1000 source lines'}
    $vars=@{};$count=0;$ref=[ref]$count;$items=@(Expand-ScriptRange $lines 0 $lines.Count $vars @() $ref 0)
    return [pscustomobject]@{Items=$items;Count=$count}
}
function Split-ScriptTokens([string]$line){
    $tokens=@();foreach($m in [regex]::Matches($line,'"(?:[^"\\]|\\.)*"|\S+')){$v=$m.Value;if($v.Length -ge 2 -and $v[0] -eq '"' -and $v[$v.Length-1] -eq '"'){$v=$v.Substring(1,$v.Length-2) -replace '\\"','"'};$tokens += $v};return @($tokens)
}
function Script-Args([string[]]$tokens,[int]$start){$h=@{};for($i=$start;$i -lt $tokens.Count;$i++){if($tokens[$i] -notmatch '^([A-Za-z_][A-Za-z0-9_.-]*)=(.*)$'){throw "Expected key=value argument: $($tokens[$i])"};$h[$Matches[1]]=$Matches[2]};return $h}
function Bool-Arg($map,[string]$name,[bool]$default=$false){if(-not $map.ContainsKey($name)){return $default};$v=([string]$map[$name]).ToLowerInvariant();if($v -in '1','true','yes','on'){return $true};if($v -in '0','false','no','off'){return $false};throw "$name must be true/false"}
function Latest-EvidenceDir([string]$kind){
    $dirs=Get-ChildItem $evidencePath -Directory -ErrorAction SilentlyContinue
    if($kind -eq 'ioc-sweep'){$dirs=$dirs|Where-Object{$_.Name -like 'ioc-sweep-*'}}else{$dirs=$dirs|Where-Object{$_.Name -notlike 'ioc-sweep-*' -and (Test-Path (Join-Path $_.FullName 'state.chqstate'))}}
    $d=$dirs|Sort-Object LastWriteTime -Descending|Select-Object -First 1;if(-not $d){throw "No $kind output found"};return $d
}
function Zip-ResearchOutput([string]$kind,[string]$name='latest'){
    if($kind -notin 'evidence','ioc-sweep'){throw 'zip target must be evidence or ioc-sweep'}
    $d=if($name -eq 'latest'){Latest-EvidenceDir $kind}else{$safe=SafeName $name;$x=Join-Path $evidencePath $safe;if(-not(Test-Path $x -PathType Container)){throw "Output not found: $safe"};Get-Item $x}
    $zip=Join-Path $evidencePath ($d.Name+'.zip');if(Test-Path $zip){Remove-Item $zip -Force};Compress-Archive -Path (Join-Path $d.FullName '*') -DestinationPath $zip -CompressionLevel Optimal
    return "OK zip path=$zip"
}
function Zip-EvidencePrefix([string]$prefix,[string]$bundleName=''){
    $prefix=SafeName $prefix
    $dirs=@(Get-ChildItem $evidencePath -Directory -ErrorAction SilentlyContinue | Where-Object {$_.Name.StartsWith($prefix,[StringComparison]::OrdinalIgnoreCase)} | Sort-Object Name)
    if($dirs.Count -eq 0){throw "No evidence captures match prefix: $prefix"}
    if([string]::IsNullOrWhiteSpace($bundleName)){$bundleName=($prefix.TrimEnd('-','_','.')+'-bundle')}
    $bundleName=SafeName $bundleName
    $zip=Join-Path $evidencePath ($bundleName+'.zip');if(Test-Path $zip){Remove-Item $zip -Force}
    Compress-Archive -Path @($dirs.FullName) -DestinationPath $zip -CompressionLevel Optimal
    return [ordered]@{ok=$true;name=$bundleName;prefix=$prefix;count=$dirs.Count;captures=@($dirs.Name);path=$zip;download=("/api/v1/evidence-bundle/$bundleName/download")}
}
function New-EvidenceCapture([string]$name){
    $name=SafeName $name;$dir=Join-Path $evidencePath $name;New-Item -ItemType Directory -Path $dir -Force|Out-Null
    $beforeRaw=Invoke-Chq @('status');$before=Parse-Kv $beforeRaw;$wasPaused=([string]$before.paused -eq '1');$pauseResult=$null;$resumeResult=$null
    try{
        if(-not $wasPaused){$pauseResult=Invoke-Chq @('pause')}
        $captureStatus=Invoke-Chq @('status');$checkpoint=Invoke-Chq @('checkpoint','save',(Join-Path $dir 'state.chqstate'));$ev=Invoke-Chq @('events','save',(Join-Path $dir 'events.csv'));$sp=Invoke-Chq @('sprites','save',(Join-Path $dir 'sprites.csv'));$shot=Invoke-Chq @('screenshot',(Join-Path $dir 'screenshot.png'))
        $manifest=[ordered]@{schema='chq-evidence-v2';name=$name;created=(Get-Date).ToString('o');debugPort=$Port;httpPort=$HttpPort;wasPaused=$wasPaused;statusBefore=$beforeRaw;statusCaptured=$captureStatus;pause=$pauseResult;checkpoint=$checkpoint;events=$ev;sprites=$sp;screenshot=$shot}
        $manifest|ConvertTo-Json -Depth 8|Set-Content (Join-Path $dir 'manifest.json') -Encoding UTF8
    }finally{if(-not $wasPaused){$resumeResult=Invoke-Chq @('resume')}}
    $manifest.resume=$resumeResult;$manifest|ConvertTo-Json -Depth 8|Set-Content (Join-Path $dir 'manifest.json') -Encoding UTF8
    $zip=Join-Path $evidencePath ($name+'.zip');if(Test-Path $zip){Remove-Item $zip -Force};Compress-Archive -Path (Join-Path $dir '*') -DestinationPath $zip -CompressionLevel Optimal
    return [ordered]@{ok=$true;name=$name;path=$dir;zip=$zip;download=('/api/v1/evidence/'+$name+'/download');resumed=(-not $wasPaused);manifest=$manifest}
}
function Invoke-IocSweepCore([string]$file,[int]$iocPort,[string[]]$masks,[int]$pulseFrames,[int]$observeFrames){
    $full=Resolve-CheckpointFile $file;$before=Parse-Kv (Invoke-Chq @('status'));$wasPaused=([string]$before.paused -eq '1');$stamp=Get-Date -Format 'yyyyMMdd-HHmmss';$dir=Join-Path $evidencePath ("ioc-sweep-p{0}-{1}" -f $iocPort,$stamp);New-Item -ItemType Directory -Path $dir -Force|Out-Null;$results=@()
    try{
        foreach($m0 in $masks){
            $m=([string]$m0).Trim().ToUpperInvariant();if($m -notmatch '^[0-9A-F]{1,2}$'){throw "Invalid mask $m"}
            $load=Invoke-Chq @('checkpoint','load',$full);$start=Parse-Kv (Invoke-Chq @('status'));$startFrame=[int]$start.frame;$pulse=Invoke-Chq @('input','pulse',('{0:X}' -f $iocPort),$m,[string]$pulseFrames);$step=Invoke-Chq @('step','frame',[string]$observeFrames);$done=Wait-ChqPaused ($startFrame+$observeFrames) 12000
            $shotPath=Join-Path $dir ("mask-$m.png");$shot=Invoke-Chq @('screenshot',$shotPath);$events=Invoke-Chq @('events','tail','100');$events|Set-Content (Join-Path $dir ("mask-$m-events.txt")) -Encoding UTF8
            $results += [ordered]@{mask=$m;frame=[int]$done.frame;screenshot=$shotPath;load=$load;pulse=$pulse;step=$step}
        }
    }finally{$null=Invoke-Chq @('checkpoint','load',$full);if(-not $wasPaused){$null=Invoke-Chq @('resume')}}
    $manifest=[ordered]@{schema='chq-ioc-sweep-v1';created=(Get-Date).ToString('o');checkpoint=$file;port=$iocPort;pulseFrames=$pulseFrames;observeFrames=$observeFrames;masks=@($masks);results=$results};$manifest|ConvertTo-Json -Depth 10|Set-Content (Join-Path $dir 'manifest.json') -Encoding UTF8
    return [ordered]@{ok=$true;path=$dir;results=$results}
}
function New-StructuredFrameSnapshot([string]$requestedName=''){
    $st=Parse-Kv (Invoke-Chq @('status'))
    $base=if([string]::IsNullOrWhiteSpace($requestedName)){('frame-snapshot-'+(Get-Date -Format 'yyyyMMdd-HHmmss-fff'))}else{SafeName $requestedName}
    $snapRoot=Join-Path $evidencePath 'frame-snapshots';New-Item -ItemType Directory -Path $snapRoot -Force|Out-Null
    $name=$base;$i=1;$dir=Join-Path $snapRoot $name;while(Test-Path $dir){$name=$base+'-'+$i;$i++;$dir=Join-Path $snapRoot $name};New-Item -ItemType Directory -Path $dir -Force|Out-Null
    $csv=Join-Path $dir 'sprites.csv';$txt=Join-Path $dir 'sprites-tail.txt'
    $layered=Invoke-Chq @('graphics','snapshot',$dir);$spriteSave=Invoke-Chq @('sprites','save',$csv);$spriteTail=Invoke-Chq @('sprites','tail','250','semantic');$spriteTail|Set-Content $txt -Encoding UTF8
    $reconPath=Join-Path $dir 'reconstruction.json';if(-not(Test-Path $reconPath)){throw 'Native layered snapshot did not create reconstruction.json'};$recon=Get-Content $reconPath -Raw|ConvertFrom-Json
    $manifest=[ordered]@{schema='chq-frame-snapshot-v2';name=$name;created=(Get-Date).ToString('o');build=$workbenchVersion;frame=(Prop $st 'frame' $null);paused=(Prop $st 'paused' $null);screenshot='final.png';hudHidden='final-no-hud.png';hudOnly='hud-only.png';layers='layers/';sources='sources/';reconstruction='reconstruction.json';palette='palette.csv';spriteOwnership='sprite-ownership.csv';sprites='sprites.csv';spriteTail='sprites-tail.txt';spriteDetails='sprites/sprites.json';sceneNoSprites='scene-no-sprites.png';captureResult=$layered;spriteSaveResult=$spriteSave;compositor=(Prop $recon 'compositor' 'unknown');hudModel=(Prop $recon 'hudModel' 'unknown');notes=@('Layer contribution PNGs use alpha and reconstruct the selected compositor output in manifest order.','HUD isolation suppresses the TC0100SCN text layer at source; no fixed rectangle mask is used.','Raw source PNGs are forensic views and are separate from exact reconstruction contribution layers.','Per-sprite cropped transparent assets are exploratory forensic views; aggregate sprites layer remains authoritative for exact reconstruction.')}
    $manifest|ConvertTo-Json -Depth 10|Set-Content (Join-Path $dir 'manifest.json') -Encoding UTF8
    # A capture mutates the snapshot tree. Keep an explicit same-process registry entry as the
    # authoritative fast path, then invalidate all disk-discovery caches. This avoids relying on
    # evidence-root scope/cache timing for a frame.snapshot -> frame.snapshot.list sequence.
    $runId=''
    try{
        $frameSnapshotsDir=Split-Path $dir -Parent
        $artifactDir=Split-Path $frameSnapshotsDir -Parent
        $runDir=Split-Path $artifactDir -Parent
        if((Split-Path (Split-Path $runDir -Parent) -Leaf) -eq 'runs'){$runId=Split-Path $runDir -Leaf}
    }catch{}
    if(-not $script:RecentStructuredFrameSnapshots){$script:RecentStructuredFrameSnapshots=@{}}
    $script:RecentStructuredFrameSnapshots[$name]=[pscustomobject]@{name=$name;frame=$manifest.frame;created=$manifest.created;path=$dir;origin=$(if($runId){'run'}else{'active'});session=(Get-SessionId);runId=$runId;tree='evidence';lastWrite=(Get-Item $dir).LastWriteTime}
    $script:StructuredFrameSnapshotsCache=$null;$script:StructuredFrameSnapshotsCacheAt=$null
    $script:SnapshotRootsCache=$null;$script:SnapshotRootsCacheAt=$null
    $script:KnownEvidenceSessionRootsCache=$null
    return [ordered]@{ok=$true;name=$name;path=$dir;frame=$manifest.frame;screenshot=(Join-Path $dir 'final.png');hudHidden=(Join-Path $dir 'final-no-hud.png');hudOnly=(Join-Path $dir 'hud-only.png');manifest=(Join-Path $dir 'manifest.json');reconstruction=$reconPath;viewer=('/api/v1/frame/snapshot/'+$name+'/asset/final.png')}
}
function Get-KnownEvidenceSessionRoots(){
    if($script:KnownEvidenceSessionRootsCache){return @($script:KnownEvidenceSessionRootsCache)}
    $roots=@();if(Test-Path $evidenceSessionsRoot -PathType Container){$roots += (Get-Item $evidenceSessionsRoot).FullName}
    if(Test-Path $workspaceRoot -PathType Container){foreach($v in Get-ChildItem $workspaceRoot -Directory -Filter 'ChaseHQ-Native-v*' -ErrorAction SilentlyContinue){$r=Join-Path $v.FullName 'evidence\sessions';if(Test-Path $r -PathType Container){$roots += (Get-Item $r).FullName}}}
    $script:KnownEvidenceSessionRootsCache=@($roots|Sort-Object -Unique);return @($script:KnownEvidenceSessionRootsCache)
}
function Get-StructuredFrameSnapshotRoots(){
    $now=Get-Date;if($script:SnapshotRootsCache -and $script:SnapshotRootsCacheAt -and ($now-$script:SnapshotRootsCacheAt).TotalSeconds -lt 10){return @($script:SnapshotRootsCache)}
    $roots=@();$live=Join-Path $evidencePath 'frame-snapshots';if(Test-Path $live -PathType Container){$roots += [ordered]@{path=$live;origin='active';session=$(Get-SessionId);runId='';tree=(Split-Path $root -Leaf)}}
    foreach($sessionsRoot in @(Get-KnownEvidenceSessionRoots)){$tree=Split-Path (Split-Path $sessionsRoot -Parent) -Leaf;foreach($session in Get-ChildItem $sessionsRoot -Directory -ErrorAction SilentlyContinue){$runs=Join-Path $session.FullName 'runs';if(-not(Test-Path $runs -PathType Container)){continue};foreach($run in Get-ChildItem $runs -Directory -ErrorAction SilentlyContinue){$r=Join-Path $run.FullName 'artifacts\frame-snapshots';if(Test-Path $r -PathType Container){$roots += [ordered]@{path=$r;origin='run';session=$session.Name;runId=$run.Name;tree=$tree}}}}}
    $script:SnapshotRootsCache=@($roots|Sort-Object path -Unique);$script:SnapshotRootsCacheAt=$now;return @($script:SnapshotRootsCache)
}
function Resolve-StructuredFrameSnapshotDir([string]$name){
    $safe=SafeName $name
    if($script:RecentStructuredFrameSnapshots -and $script:RecentStructuredFrameSnapshots.ContainsKey($safe)){
        $recent=[string]$script:RecentStructuredFrameSnapshots[$safe].path
        if(Test-Path $recent -PathType Container){return (Get-Item $recent).FullName}
    }
    $matches=@();foreach($r in @(Get-StructuredFrameSnapshotRoots)){$d=Join-Path ([string]$r.path) $safe;if(Test-Path $d -PathType Container){$matches += Get-Item $d}}
    if(@($matches).Count -eq 0){$script:SnapshotRootsCache=$null;$script:SnapshotRootsCacheAt=$null;$script:KnownEvidenceSessionRootsCache=$null;foreach($r in @(Get-StructuredFrameSnapshotRoots)){$d=Join-Path ([string]$r.path) $safe;if(Test-Path $d -PathType Container){$matches += Get-Item $d}}}
    if(@($matches).Count -eq 0){throw "Frame snapshot not found: $safe"};return (@($matches)|Sort-Object LastWriteTime -Descending|Select-Object -First 1).FullName
}
function Get-StructuredFrameSnapshots(){
    $now=Get-Date
    $items=@()
    if($script:StructuredFrameSnapshotsCache -and $script:StructuredFrameSnapshotsCacheAt -and ($now-$script:StructuredFrameSnapshotsCacheAt).TotalSeconds -lt 5){
        foreach($x in @($script:StructuredFrameSnapshotsCache)){$items += [pscustomobject]@{name=$x.name;frame=$x.frame;created=$x.created;path=$x.path;origin=$x.origin;session=$x.session;runId=$x.runId;tree=$x.tree;lastWrite=$(if(Test-Path ([string]$x.path)){(Get-Item ([string]$x.path)).LastWriteTime}else{[datetime]::MinValue})}}
    }else{
        foreach($r in @(Get-StructuredFrameSnapshotRoots)){foreach($d in Get-ChildItem ([string]$r.path) -Directory -ErrorAction SilentlyContinue){$mp=Join-Path $d.FullName 'manifest.json';$m=$null;if(Test-Path $mp){try{$m=Get-Content $mp -Raw -Encoding UTF8|ConvertFrom-Json}catch{}};$items += [pscustomobject]@{name=$d.Name;frame=$(if($m){$m.frame}else{$null});created=$(if($m){$m.created}else{$d.LastWriteTime.ToString('o')});path=$d.FullName;origin=[string]$r.origin;session=[string]$r.session;runId=[string]$r.runId;tree=[string]$r.tree;lastWrite=$d.LastWriteTime}}}
    }
    if($script:RecentStructuredFrameSnapshots){foreach($x in @($script:RecentStructuredFrameSnapshots.Values)){if(Test-Path ([string]$x.path) -PathType Container){$items += $x}}}
    $out=@();foreach($g in $items|Group-Object name){$x=@($g.Group|Sort-Object lastWrite -Descending|Select-Object -First 1)[0];$out += [ordered]@{name=$x.name;frame=$x.frame;created=$x.created;path=$x.path;origin=$x.origin;session=$x.session;runId=$x.runId;tree=$x.tree}}
    $out=@($out|Sort-Object created -Descending)
    $script:StructuredFrameSnapshotsCache=$out;$script:StructuredFrameSnapshotsCacheAt=$now
    return @($out)
}
function Get-AuthoritativeScriptRuns(){
    $now=Get-Date;if($script:AuthoritativeRunHistoryCache -and $script:AuthoritativeRunHistoryCacheAt -and ($now-$script:AuthoritativeRunHistoryCacheAt).TotalSeconds -lt 10){return @($script:AuthoritativeRunHistoryCache)}
    $metaFiles=@();foreach($sessionsRoot in @(Get-KnownEvidenceSessionRoots)){foreach($session in Get-ChildItem $sessionsRoot -Directory -ErrorAction SilentlyContinue){$runs=Join-Path $session.FullName 'runs';if(-not(Test-Path $runs -PathType Container)){continue};foreach($run in Get-ChildItem $runs -Directory -ErrorAction SilentlyContinue){$mf=Join-Path $run.FullName 'run-metadata.json';if(Test-Path $mf -PathType Leaf){$metaFiles += Get-Item $mf}}}}
    $metaFiles=@($metaFiles|Sort-Object FullName -Unique);$out=@()
    foreach($mf in $metaFiles){$run=$mf.Directory;try{$m=Get-Content $mf.FullName -Raw -Encoding UTF8|ConvertFrom-Json}catch{continue};$metaStatus=[string](Prop $m 'status' '');if($metaStatus -eq 'RUNNING'){continue};$bundle=Join-Path $run.FullName 'bundle.zip';$manifestPath=Join-Path $run.FullName 'bundle-manifest.json';$artifactCount=0;$artifactTypes=@{};if(Test-Path $manifestPath -PathType Leaf){try{$bm=Get-Content $manifestPath -Raw -Encoding UTF8|ConvertFrom-Json;foreach($rel in @($bm.files)){if([string]$rel -like 'artifacts\*'){$artifactCount++;$ext=[IO.Path]::GetExtension([string]$rel).TrimStart('.').ToUpperInvariant();if(-not$ext){$ext='OTHER'};if(-not$artifactTypes.ContainsKey($ext)){$artifactTypes[$ext]=0};$artifactTypes[$ext]++}}}catch{}}
        $started=[string](Prop $m 'started' $run.LastWriteTime.ToString('o'));$parsedAt=[DateTimeOffset]::MinValue;if(-not [DateTimeOffset]::TryParse($started,[ref]$parsedAt)){$started=$mf.LastWriteTimeUtc.ToString('o')};$sessionId=[string](Prop $m 'sessionId' (Split-Path (Split-Path $run.FullName -Parent) -Leaf));$out += [ordered]@{id=$started;serverKey=($sessionId+'/'+[string](Prop $m 'runId' $run.Name));at=$started;status=$(if($metaStatus){$metaStatus}elseif([bool](Prop $m 'ok' $false)){'PASS'}else{'FAIL'});version=[string](Prop $m 'build' 'unknown');session=$sessionId;sourceType=[string](Prop $m 'sourceType' 'server');name=[string](Prop $m 'name' 'Script run');purpose=[string](Prop $m 'purpose' '');sourcePath=[string](Prop $m 'sourcePath' '');duration=[math]::Round(([double](Prop $m 'durationMs' 0))/1000,3);commands=[int](Prop $m 'commands' 0);artifactCount=$artifactCount;artifactTypes=$artifactTypes;runId=[string](Prop $m 'runId' $run.Name);runPath=$run.FullName;bundle=$(if(Test-Path $bundle -PathType Leaf){$bundle}else{''});bundleDownloadName=[string](Prop $m 'bundleDownloadName' 'bundle.zip')}
    }
    $script:AuthoritativeRunHistoryCache=@($out|Sort-Object @{Expression={[DateTimeOffset]::Parse([string]$_['at']).UtcDateTime.Ticks};Descending=$true},@{Expression={[string]$_['serverKey']};Descending=$true});$script:AuthoritativeRunHistoryCacheAt=$now;return @($script:AuthoritativeRunHistoryCache)
}
function Get-ForensicTimelines(){
    $items=@();$roots=@()
    $live=Join-Path $evidencePath 'timelines';if(Test-Path $live -PathType Container){$roots += [ordered]@{path=$live;origin='active';session=(Get-SessionId);runId=''}}
    $packaged=Join-Path $root 'research\timelines';if(Test-Path $packaged -PathType Container){$roots += [ordered]@{path=$packaged;origin='packaged';session='';runId=''}}
    foreach($sessionsRoot in @(Get-KnownEvidenceSessionRoots)){foreach($session in Get-ChildItem $sessionsRoot -Directory -ErrorAction SilentlyContinue){$runs=Join-Path $session.FullName 'runs';if(-not(Test-Path $runs -PathType Container)){continue};foreach($run in Get-ChildItem $runs -Directory -ErrorAction SilentlyContinue){$r=Join-Path $run.FullName 'artifacts\timelines';if(Test-Path $r -PathType Container){$roots += [ordered]@{path=$r;origin='run';session=$session.Name;runId=$run.Name}}}}}
    foreach($r in $roots){foreach($d in Get-ChildItem ([string]$r.path) -Directory -Filter '*.chqtimeline' -ErrorAction SilentlyContinue){$m=$null;$mp=Join-Path $d.FullName 'manifest.json';if(Test-Path $mp){try{$m=Get-Content $mp -Raw -Encoding UTF8|ConvertFrom-Json}catch{}};$items += [ordered]@{name=$d.BaseName;path=$d.FullName;origin=$r.origin;session=$r.session;runId=$r.runId;build=$(if($m){[string](Prop $m 'build' '')}else{''});startFrame=$(if($m){Prop $m 'startFrame' $null}else{$null});endFrame=$(if($m){Prop $m 'endFrame' $null}else{$null});frameCount=$(if($m){Prop $m 'frameCount' $null}else{$null});writeCount=$(if($m){Prop $m 'writeCount' $null}else{$null});state=$(if($m){[string](Prop $m 'state' '')}else{''});modified=$d.LastWriteTime.ToString('o')}}}
    return @($items|Sort-Object modified -Descending)
}
function Resolve-ForensicTimeline([string]$nameOrPath){
    if([string]::IsNullOrWhiteSpace($nameOrPath)){throw 'timeline name/path is required'}
    if(Test-Path $nameOrPath -PathType Container){return (Resolve-Path $nameOrPath).Path}
    $safe=SafeName ([IO.Path]::GetFileNameWithoutExtension($nameOrPath));$matches=@(Get-ForensicTimelines|Where-Object{$_.name -eq $safe -or [IO.Path]::GetFileName($_.path) -eq ($safe+'.chqtimeline')})
    if($matches.Count -eq 0){throw "Forensic timeline not found: $nameOrPath"};return [string]$matches[0].path
}
function Get-TimelineTargetValue([string]$hexValue,[uint32]$writeAddress,[int]$writeWidth,[uint32]$targetAddress,[int]$targetWidth){
    $writeBytes=[int]($writeWidth/8);$targetBytes=[int]($targetWidth/8);if($writeBytes -lt 1 -or $targetBytes -lt 1){throw 'timeline widths must be byte aligned'}
    $writeEnd=$writeAddress+$writeBytes-1;$targetEnd=$targetAddress+$targetBytes-1;if($writeAddress -gt $targetAddress -or $writeEnd -lt $targetEnd){return $null}
    $raw=[Convert]::ToUInt64(([string]$hexValue).Replace('0x',''),16);$offset=[int]($targetAddress-$writeAddress);$shift=[int](($writeBytes-($offset+$targetBytes))*8);$valueMask=if($targetWidth -eq 32){[uint64]0xFFFFFFFF}else{([uint64]1 -shl $targetWidth)-1};return [uint32](($raw -shr $shift)-band$valueMask)
}
function Trim-ForensicTimelineEvent([string]$timelinePath,[string]$cpu,[string]$address,[int]$width,[string]$maskText,[int]$pre=1,[int]$post=1){
    # Historical API name retained for compatibility. RC3 no longer trims or rewrites the
    # authoritative recording. It performs one streaming pass over writes.csv, records the
    # rising/falling edge window under analysis/, and leaves frames/writes/checkpoints intact.
    $full=Resolve-ForensicTimeline $timelinePath;$writes=Join-Path $full 'writes.csv';$framesCsv=Join-Path $full 'frames.csv';$manifest=Join-Path $full 'manifest.json';if(-not(Test-Path $writes)){throw 'Timeline write stream missing'};if(-not(Test-Path $framesCsv)){throw 'Timeline frame index missing'}
    $cpu=$cpu.ToUpperInvariant();if($cpu -notin 'A','B'){throw 'cpu must be A or B'};if($width -notin 8,16,32){throw 'width must be 8, 16 or 32'};$target=Hex-U32 $address;$addr=('0x{0:X}' -f $target);$mask=Hex-U32 $maskText
    $rise=$null;$fall=$null;$riseSource=$null;$fallSource=$null;$scanned=0
    $sr=[IO.File]::OpenText($writes)
    try{
        $null=$sr.ReadLine() # header
        while(-not $sr.EndOfStream){
            $line=$sr.ReadLine();if([string]::IsNullOrWhiteSpace($line)){continue};$scanned++;$c=$line.Split(',');if($c.Length -lt 9){continue};if((([string]$c[2]).Trim().Trim([char]34)) -ne $cpu){continue}
            $wa=Hex-U32 (([string]$c[4]).Trim().Trim([char]34));$ww=[int](([string]$c[5]).Trim().Trim([char]34));$old=Get-TimelineTargetValue (([string]$c[6]).Trim().Trim([char]34)) $wa $ww $target $width;$new=Get-TimelineTargetValue (([string]$c[7]).Trim().Trim([char]34)) $wa $ww $target $width;if($null-eq$old -or $null-eq$new){continue}
            $frame=[int](([string]$c[1]).Trim().Trim([char]34));$ob=(($old -band $mask) -ne 0);$nb=(($new -band $mask) -ne 0)
            if($null-eq$rise -and -not$ob -and $nb){$rise=$frame;$riseSource=[pscustomobject]@{frame=$frame;old=[uint32]$old;new=[uint32]$new;pc=[string]$c[3];writeAddress=[string]$c[4];writeWidth=$ww};continue}
            if($null-ne$rise -and $null-eq$fall -and $ob -and -not$nb){$fall=$frame;$fallSource=[pscustomobject]@{frame=$frame;old=[uint32]$old;new=[uint32]$new;pc=[string]$c[3];writeAddress=[string]$c[4];writeWidth=$ww};break}
        }
    }finally{$sr.Dispose()}
    if($null-eq$rise){throw 'No rising edge found in timeline write stream'};if($null-eq$fall){throw 'No falling edge found after rising edge'};$from=[Math]::Max(0,$rise-$pre);$to=$fall+$post
    $frames=@(Import-Csv $framesCsv);$keep=@($frames|Where-Object{[int]$_.frame -ge $from -and [int]$_.frame -le $to});if($keep.Count -eq 0){throw 'Event window contains no recorded frames'}
    $sourceStart=if($frames.Count){[int]$frames[0].frame}else{$from};$sourceEnd=if($frames.Count){[int]$frames[-1].frame}else{$to}
    $event=[ordered]@{schema='chq-timeline-event-window-v1';timeline=$full;cpu=$cpu;address=$addr;width=$width;mask=('0x{0:X}' -f $mask);riseFrame=$rise;fallFrame=$fall;viewStart=[int]$keep[0].frame;viewEnd=[int]$keep[-1].frame;preFrames=$pre;postFrames=$post;sourceStart=$sourceStart;sourceEnd=$sourceEnd;sourceFrameCount=$frames.Count;viewFrameCount=$keep.Count;riseWriter=$riseSource.pc;riseWriteAddress=$riseSource.writeAddress;riseWriteWidth=$riseSource.writeWidth;fallWriter=$fallSource.pc;fallWriteAddress=$fallSource.writeAddress;fallWriteWidth=$fallSource.writeWidth;writesScannedUntilFall=$scanned;destructive=$false;created=(Get-Date).ToString('o')}
    $analysis=Get-TimelineAnalysisDirectory $full -Create;$eventsDir=Join-Path $analysis 'events';New-Item -ItemType Directory -Force -Path $eventsDir|Out-Null;$tag=('event-{0}-{1}-m{2}' -f $cpu,('{0:X}' -f $target),('{0:X}' -f $mask));$eventPath=Join-Path $eventsDir ($tag+'.json');$latest=Join-Path $analysis 'event-window.json';$json=$event|ConvertTo-Json -Depth 8;[IO.File]::WriteAllText($eventPath,$json,[Text.UTF8Encoding]::new($false));[IO.File]::WriteAllText($latest,$json,[Text.UTF8Encoding]::new($false))
    return [ordered]@{schema='chq-timeline-event-window-v1';timeline=$full;riseFrame=$rise;fallFrame=$fall;startFrame=[int]$keep[0].frame;endFrame=[int]$keep[-1].frame;frames=$keep.Count;sourceFrames=$frames.Count;writesScanned=$scanned;destructive=$false;eventFile=$eventPath;event=$event}
}

function Get-TimelineKnownSemantic([string]$cpu,[string]$address){
    if($cpu -ne 'A'){return ''}
    switch($address.ToUpperInvariant()){
        '0X100303'{return 'turbo_input'}
        '0X10040F'{return 'turbo_active'}
        '0X1003A2'{return 'turbos_remaining_hi'}
        '0X1003A3'{return 'turbos_remaining_lo'}
        '0X100414'{return 'turbo_timer_hi'}
        '0X100415'{return 'turbo_timer_lo'}
        default{return ''}
    }
}
function Get-TimelineAnalysisDirectory([string]$timelinePath,[switch]$Create){
    $safe=SafeName ([IO.Path]::GetFileNameWithoutExtension($timelinePath));$base=Join-Path $evidencePath 'timeline-analysis';$dir=Join-Path $base $safe
    if($Create){New-Item -ItemType Directory -Force -Path $dir|Out-Null}
    return $dir
}
function Convert-TimelineGroupsToCandidates($groups,[int]$triggerFrame,[int]$stopFrame,[int]$window){
    $out=@();$eventSpan=if($triggerFrame -ge 0 -and $stopFrame -ge $triggerFrame){$stopFrame-$triggerFrame+1}else{0}
    foreach($g in $groups.Values){
        $score=0;$firstDelta=if($triggerFrame-ge 0){[Math]::Abs([int]$g.firstFrame-$triggerFrame)}else{999999};$lastDelta=if($stopFrame-ge 0){[Math]::Abs([int]$g.lastFrame-$stopFrame)}else{999999};$edgeMatch=($firstDelta-le$window -and $lastDelta-le$window)
        if($g.triggerHits){$score+=100+20*$g.triggerHits};if($g.stopHits){$score+=80+15*$g.stopHits};if($edgeMatch){$score+=250}
        if($g.final -eq $g.initial){$score+=40;$class=if($edgeMatch){'edge-bounded-restored'}else{'returns-to-initial'}}
        elseif($g.changes -eq 1 -and $g.final -lt $g.initial){$score+=30;$class='single-decrement'}
        elseif($g.changes -eq 1 -and $g.final -gt $g.initial){$score+=25;$class='single-increment'}
        elseif($edgeMatch){$score+=60;$class='edge-bounded-state'}
        elseif($g.frames.Count -ge 8){$score+=20;$class='temporal-counter/state'}else{$class='multi-change'}
        if($eventSpan -gt 0 -and $g.changes -gt ($eventSpan*4)){$score-=100;if(-not$edgeMatch){$class='background-high-frequency'}}
        $known=Get-TimelineKnownSemantic ([string]$g.cpu) ([string]$g.address);if($known){$score+=500}
        $out += [pscustomobject]@{cpu=$g.cpu;address=$g.address;width=$g.width;changes=$g.changes;changedFrames=$g.frames.Count;firstFrame=$g.firstFrame;lastFrame=$g.lastFrame;firstDelta=$firstDelta;lastDelta=$lastDelta;edgeMatch=$edgeMatch;initial=('0x{0:X}' -f $g.initial);final=('0x{0:X}' -f $g.final);triggerHits=$g.triggerHits;stopHits=$g.stopHits;writers=(($g.writers|Sort-Object)-join ' ');writerCount=$g.writers.Count;classification=$class;knownSemantic=$known;score=$score}
    }
    return @($out|Sort-Object @{Expression='score';Descending=$true},@{Expression='changes';Descending=$true},cpu,address)
}
function Analyze-ForensicTimelineWrites([string]$timelinePath,[int]$triggerFrame=-1,[int]$stopFrame=-1,[int]$window=1){
    $full=Resolve-ForensicTimeline $timelinePath;$mp=Join-Path $full 'manifest.json';$currentAnalysis=Get-TimelineAnalysisDirectory $full;$eventPath=Join-Path $currentAnalysis 'event-window.json';$legacyEventPath=Join-Path $full 'analysis\event-window.json';$event=$null
    if(Test-Path $eventPath -PathType Leaf){try{$event=Get-Content $eventPath -Raw -Encoding UTF8|ConvertFrom-Json}catch{}}elseif(Test-Path $legacyEventPath -PathType Leaf){try{$event=Get-Content $legacyEventPath -Raw -Encoding UTF8|ConvertFrom-Json}catch{}}
    if(($triggerFrame -lt 0 -or $stopFrame -lt 0) -and $event){if($triggerFrame -lt 0){$triggerFrame=[int]$event.riseFrame};if($stopFrame -lt 0){$stopFrame=[int]$event.fallFrame}}
    if(($triggerFrame -lt 0 -or $stopFrame -lt 0) -and (Test-Path $mp)){try{$mm=Get-Content $mp -Raw -Encoding UTF8|ConvertFrom-Json;if($mm.event){if($triggerFrame -lt 0){$triggerFrame=[int]$mm.event.riseFrame};if($stopFrame -lt 0){$stopFrame=[int]$mm.event.fallFrame}}}catch{}}
    $scanFrom=-1;$scanTo=-1
    if($triggerFrame -ge 0 -and $stopFrame -ge 0){$scanFrom=[Math]::Max(0,$triggerFrame-$window);$scanTo=$stopFrame+$window}
    elseif($event){$scanFrom=[int]$event.viewStart;$scanTo=[int]$event.viewEnd}
    $csv=Join-Path $full 'writes.csv';if(-not(Test-Path $csv -PathType Leaf)){throw 'Timeline write stream missing'}
    $groups=@{};$byteGroups=@{};$writerGroups=@{};$rowsScanned=0;$rowsInWindow=0;$changedRows=0
    $sr=[IO.File]::OpenText($csv)
    try{
        $null=$sr.ReadLine()
        while(-not $sr.EndOfStream){
            $line=$sr.ReadLine();if([string]::IsNullOrWhiteSpace($line)){continue};$rowsScanned++;$c=$line.Split(',');if($c.Length -lt 9){continue};$f=[int](([string]$c[1]).Trim().Trim([char]34));if($scanFrom -ge 0 -and $f -lt $scanFrom){continue};if($scanTo -ge 0 -and $f -gt $scanTo){break};$rowsInWindow++;if([int](([string]$c[8]).Trim().Trim([char]34)) -ne 1){continue};$changedRows++
            $cpu=([string]$c[2]).Trim().Trim([char]34);$pc=([string]$c[3]).Trim().Trim([char]34);$addr=([string]$c[4]).Trim().Trim([char]34);$bits=[int](([string]$c[5]).Trim().Trim([char]34));$oldText=([string]$c[6]).Trim().Trim([char]34);$newText=([string]$c[7]).Trim().Trim([char]34);$old=[Convert]::ToUInt64($oldText.Replace('0x',''),16);$new=[Convert]::ToUInt64($newText.Replace('0x',''),16);$key=$cpu+':'+$addr+':'+$bits
            if(-not $groups.ContainsKey($key)){$groups[$key]=[ordered]@{cpu=$cpu;address=$addr;width=$bits;changes=0;firstFrame=$f;lastFrame=$f;initial=$old;final=$new;writers=New-Object 'System.Collections.Generic.HashSet[string]';triggerHits=0;stopHits=0;frames=New-Object 'System.Collections.Generic.HashSet[int]'}}
            $g=$groups[$key];$g.changes++;$g.lastFrame=$f;$g.final=$new;$null=$g.writers.Add($pc);$null=$g.frames.Add($f);if($triggerFrame -ge 0 -and [Math]::Abs($f-$triggerFrame) -le $window){$g.triggerHits++};if($stopFrame -ge 0 -and [Math]::Abs($f-$stopFrame) -le $window){$g.stopHits++}
            $wk=$cpu+':'+$pc;if(-not $writerGroups.ContainsKey($wk)){$writerGroups[$wk]=[ordered]@{cpu=$cpu;pc=$pc;writes=0;frames=New-Object 'System.Collections.Generic.HashSet[int]';addresses=New-Object 'System.Collections.Generic.HashSet[string]'}};$wg=$writerGroups[$wk];$wg.writes++;$null=$wg.frames.Add($f);$null=$wg.addresses.Add($addr)
            $wa=Hex-U32 $addr;$bytes=[int]($bits/8);for($i=0;$i -lt $bytes;$i++){$shift=[int](($bytes-$i-1)*8);$ob=[uint32](($old -shr $shift)-band 0xFF);$nb=[uint32](($new -shr $shift)-band 0xFF);if($ob -eq $nb){continue};$ba=$wa+$i;$baText=('0x{0:X}' -f $ba);$bk=$cpu+':'+$baText;if(-not $byteGroups.ContainsKey($bk)){$byteGroups[$bk]=[ordered]@{cpu=$cpu;address=$baText;width=8;changes=0;firstFrame=$f;lastFrame=$f;initial=$ob;final=$nb;writers=New-Object 'System.Collections.Generic.HashSet[string]';triggerHits=0;stopHits=0;frames=New-Object 'System.Collections.Generic.HashSet[int]'}};$bg=$byteGroups[$bk];$bg.changes++;$bg.lastFrame=$f;$bg.final=$nb;$null=$bg.writers.Add($pc);$null=$bg.frames.Add($f);if($triggerFrame -ge 0 -and [Math]::Abs($f-$triggerFrame) -le $window){$bg.triggerHits++};if($stopFrame -ge 0 -and [Math]::Abs($f-$stopFrame) -le $window){$bg.stopHits++}}
        }
    }finally{$sr.Dispose()}
    $ranked=Convert-TimelineGroupsToCandidates $groups $triggerFrame $stopFrame $window;$bytesRanked=Convert-TimelineGroupsToCandidates $byteGroups $triggerFrame $stopFrame $window
    $writers=@();foreach($w in $writerGroups.Values){$writers += [pscustomobject]@{cpu=$w.cpu;pc=$w.pc;changedWrites=$w.writes;changedFrames=$w.frames.Count;addressCount=$w.addresses.Count;addresses=(($w.addresses|Sort-Object)-join ' ')}};$writers=@($writers|Sort-Object @{Expression='changedWrites';Descending=$true},@{Expression='addressCount';Descending=$true},cpu,pc)
    $analysis=Get-TimelineAnalysisDirectory $full -Create;$transactionCsv=Join-Path $analysis 'memory-write-candidates.csv';$byteCsv=Join-Path $analysis 'memory-byte-candidates.csv';$rankedJson=Join-Path $analysis 'ranked-candidates.json';$writerJson=Join-Path $analysis 'writer-groups.json';$report=Join-Path $analysis 'report.html';$sourceJson=Join-Path $analysis 'source.json'
    $sourceManifest=Join-Path $full 'manifest.json';$sourceInfo=[ordered]@{schema='chq-timeline-analysis-source-v1';timeline=$full;timelineName=[IO.Path]::GetFileNameWithoutExtension($full);analysisBuild=$workbenchVersion;created=(Get-Date).ToString('o');triggerFrame=$triggerFrame;stopFrame=$stopFrame;scanStartFrame=$scanFrom;scanEndFrame=$scanTo;window=$window;manifestSha256=$(if(Test-Path $sourceManifest){(Get-FileHash $sourceManifest -Algorithm SHA256).Hash}else{''});writesSha256=(Get-FileHash $csv -Algorithm SHA256).Hash};$sourceInfo|ConvertTo-Json -Depth 6|Set-Content $sourceJson -Encoding UTF8
    $ranked|Export-Csv $transactionCsv -NoTypeInformation -Encoding UTF8;$bytesRanked|Export-Csv $byteCsv -NoTypeInformation -Encoding UTF8;$bytesRanked|Select-Object -First 300|ConvertTo-Json -Depth 6|Set-Content $rankedJson -Encoding UTF8;$writers|Select-Object -First 300|ConvertTo-Json -Depth 6|Set-Content $writerJson -Encoding UTF8
    $sb=New-Object Text.StringBuilder;$null=$sb.Append('<!doctype html><html><head><meta charset="utf-8"><title>ChaseHQ Forensic Timeline Analysis</title><style>body{font:14px system-ui;background:#10151b;color:#e7edf4;margin:24px}table{border-collapse:collapse;width:100%;margin:12px 0 28px}th,td{border:1px solid #34414d;padding:6px;text-align:left}th{background:#18222c}code{color:#9ed2ff}.meta{color:#aab8c5}</style></head><body>');$null=$sb.Append('<h1>ChaseHQ Forensic Timeline Analysis</h1><p class="meta">Timeline: '+[Net.WebUtility]::HtmlEncode($full)+' | rows scanned: '+$rowsScanned+' | rows in event window: '+$rowsInWindow+' | changed writes: '+$changedRows+' | trigger frame: '+$triggerFrame+' | stop frame: '+$stopFrame+'</p>');$null=$sb.Append('<h2>Top byte-address candidates</h2><table><tr><th>Score</th><th>CPU</th><th>Address</th><th>Changes</th><th>Frames</th><th>Initial</th><th>Final</th><th>Class</th><th>Writers</th></tr>');foreach($x in @($bytesRanked|Select-Object -First 100)){$null=$sb.Append('<tr><td>'+$x.score+'</td><td>'+$x.cpu+'</td><td><code>'+$x.address+'</code></td><td>'+$x.changes+'</td><td>'+$x.changedFrames+'</td><td><code>'+$x.initial+'</code></td><td><code>'+$x.final+'</code></td><td>'+$x.classification+'</td><td><code>'+[Net.WebUtility]::HtmlEncode($x.writers)+'</code></td></tr>')};$null=$sb.Append('</table><h2>Top writer PCs</h2><table><tr><th>CPU</th><th>PC</th><th>Changed writes</th><th>Frames</th><th>Addresses</th></tr>');foreach($x in @($writers|Select-Object -First 100)){$null=$sb.Append('<tr><td>'+$x.cpu+'</td><td><code>'+$x.pc+'</code></td><td>'+$x.changedWrites+'</td><td>'+$x.changedFrames+'</td><td>'+[Net.WebUtility]::HtmlEncode($x.addresses)+'</td></tr>')};$null=$sb.Append('</table></body></html>');[IO.File]::WriteAllText($report,$sb.ToString(),[Text.UTF8Encoding]::new($false))
    return [ordered]@{schema='chq-timeline-write-analysis-v3';timeline=$full;analysisRoot=$analysis;source=$sourceJson;rowsScanned=$rowsScanned;rowsInWindow=$rowsInWindow;changedWrites=$changedRows;changedTransactions=$ranked.Count;changedByteAddresses=$bytesRanked.Count;writerGroups=$writers.Count;triggerFrame=$triggerFrame;stopFrame=$stopFrame;scanStartFrame=$scanFrom;scanEndFrame=$scanTo;window=$window;transactionCsv=$transactionCsv;byteCsv=$byteCsv;ranked=$rankedJson;writers=$writerJson;report=$report;top=@($bytesRanked|Select-Object -First 25);topWriters=@($writers|Select-Object -First 25)}
}

function Invoke-TimelineAutomatedAnalysis([string]$timelinePath,[int]$triggerFrame,[int]$stopFrame,[int]$window=1,[int]$top=20){
    if($triggerFrame -lt 0 -or $stopFrame -lt $triggerFrame){throw 'triggerFrame/stopFrame must define a valid event interval'}
    if($top -lt 1 -or $top -gt 100){throw 'top must be 1..100'}
    $full=Resolve-ForensicTimeline $timelinePath;$analysis=Analyze-ForensicTimelineWrites $full $triggerFrame $stopFrame $window;$analysisRoot=[string]$analysis.analysisRoot
    $disasmDir=Join-Path $analysisRoot 'disassembly';New-Item -ItemType Directory -Force -Path $disasmDir|Out-Null
    $pairs=@{};foreach($c in @($analysis.top)|Select-Object -First $top){foreach($pc in @(([string]$c.writers)-split '\s+'|Where-Object{$_})){if($pc -match '^0x[0-9A-Fa-f]+$'){$pairs[([string]$c.cpu+':'+$pc)]=[ordered]@{cpu=[string]$c.cpu;pc=$pc}}}}
    $disassembly=@();foreach($pair in @($pairs.Values)|Select-Object -First 24){$safePc=([string]$pair.pc).Replace('0x','');$file=Join-Path $disasmDir (([string]$pair.cpu)+'-'+$safePc+'.txt');try{$text=Invoke-Chq @('disasm',[string]$pair.cpu,[string]$pair.pc,'24');[IO.File]::WriteAllText($file,[string]$text,[Text.UTF8Encoding]::new($false));$disassembly += [ordered]@{cpu=$pair.cpu;pc=$pair.pc;file=$file;ok=$true}}catch{$disassembly += [ordered]@{cpu=$pair.cpu;pc=$pair.pc;file='';ok=$false;error=$_.Exception.Message}}}
    $loaded=$false;$milestones=@();$comparisons=@()
    try{
        $null=Invoke-Chq @('timeline','load',$full);$loaded=$true
        $framesRaw=[string](Invoke-Chq @('timeline','frames'));$available=New-Object 'System.Collections.Generic.HashSet[int]';foreach($x in @($framesRaw -split '\s+'|Select-Object -Skip 1)){if($x -match '^\d+$'){$null=$available.Add([int]$x)}}
        $mid=[int][Math]::Floor(($triggerFrame+$stopFrame)/2);$wanted=@([ordered]@{role='before';frame=[Math]::Max(0,$triggerFrame-$window)},[ordered]@{role='active';frame=$triggerFrame},[ordered]@{role='mid';frame=$mid},[ordered]@{role='expiry';frame=$stopFrame},[ordered]@{role='after';frame=$stopFrame+$window})
        $prefix=SafeName ([IO.Path]::GetFileNameWithoutExtension($full));$stateDir=Join-Path $analysisRoot 'milestones';New-Item -ItemType Directory -Force -Path $stateDir|Out-Null
        foreach($w in $wanted){$f=[int]$w.frame;if(-not$available.Contains($f)){continue};$null=Invoke-Chq @('timeline','seek',[string]$f);$state=[ordered]@{schema='chq-timeline-milestone-v1';role=$w.role;frame=$f;status=(Parse-Kv (Invoke-Chq @('status')));gameplay=(Get-LiveGameplayRegistry);track=(Get-LiveTrackState);sprites=@(Get-LiveSprites)};$stateFile=Join-Path $stateDir ($w.role+'-f'+$f+'.json');$state|ConvertTo-Json -Depth 12|Set-Content $stateFile -Encoding UTF8;$snapName=SafeName ($prefix+'-'+$w.role+'-f'+$f);$snap=New-StructuredFrameSnapshot $snapName;$milestones += [ordered]@{role=$w.role;frame=$f;state=$stateFile;snapshot=$snapName;snapshotPath=[string]$snap.path}}
        $before=@($milestones|Where-Object{$_.role -eq 'before'}|Select-Object -First 1);$active=@($milestones|Where-Object{$_.role -eq 'active'}|Select-Object -First 1);$expiry=@($milestones|Where-Object{$_.role -eq 'expiry'}|Select-Object -First 1);$after=@($milestones|Where-Object{$_.role -eq 'after'}|Select-Object -First 1)
        foreach($pair in @(@{a=$before;b=$active;label='before-vs-active'},@{a=$active;b=$expiry;label='active-vs-expiry'},@{a=$expiry;b=$after;label='expiry-vs-after'})){if(@($pair.a).Count -and @($pair.b).Count){foreach($hud in @('normal','only','hidden')){try{$cmp=Compare-ResearchImages ([string]$pair.a[0].snapshot) ([string]$pair.b[0].snapshot) 'HUD_TURBO' '' $hud;$comparisons += [ordered]@{pair=$pair.label;hud=$hud;result=$cmp}}catch{$comparisons += [ordered]@{pair=$pair.label;hud=$hud;error=$_.Exception.Message}}}}}
    }finally{if($loaded){try{$null=Invoke-Chq @('timeline','unload')}catch{}}}
    $corrFile=Join-Path $analysisRoot 'graphics-correlations.json';$comparisons|ConvertTo-Json -Depth 12|Set-Content $corrFile -Encoding UTF8;$milestoneFile=Join-Path $analysisRoot 'milestones.json';$milestones|ConvertTo-Json -Depth 8|Set-Content $milestoneFile -Encoding UTF8;$disasmFile=Join-Path $analysisRoot 'disassembly-index.json';$disassembly|ConvertTo-Json -Depth 6|Set-Content $disasmFile -Encoding UTF8
    $summary=[ordered]@{schema='chq-timeline-auto-analysis-v1';timeline=$full;analysisRoot=$analysisRoot;triggerFrame=$triggerFrame;stopFrame=$stopFrame;window=$window;changedByteAddresses=$analysis.changedByteAddresses;writerGroups=$analysis.writerGroups;topCandidates=@($analysis.top|Select-Object -First $top);disassembly=$disassembly;milestones=$milestones;graphicsCorrelations=$comparisons;source=$analysis.source;report=$analysis.report};$summaryFile=Join-Path $analysisRoot 'automated-analysis.json';$summary|ConvertTo-Json -Depth 14|Set-Content $summaryFile -Encoding UTF8
    return [ordered]@{schema='chq-timeline-auto-analysis-v1';ok=$true;timeline=$full;analysisRoot=$analysisRoot;summary=$summaryFile;report=$analysis.report;ranked=$analysis.ranked;writers=$analysis.writers;graphics=$corrFile;milestones=$milestoneFile;disassembly=$disasmFile;changedByteAddresses=$analysis.changedByteAddresses;writerGroups=$analysis.writerGroups;top=@($analysis.top|Select-Object -First 10)}
}

function Get-ResearchRegions(){
    $j=Get-Content $imageRegionsPath -Raw|ConvertFrom-Json
    $out=@();foreach($prop in $j.regions.PSObject.Properties){$r=$prop.Value;$out += [ordered]@{name=$prop.Name;x=[int]$r.x;y=[int]$r.y;w=[int]$r.w;h=[int]$r.h}}
    return @($out)
}
function Get-LiveSprites(){
    $raw=Invoke-Chq @('sprites','list');$out=@();foreach($line in @($raw -split "`r?`n")|Select-Object -Skip 1){$kv=Parse-Kv $line;if($null -ne (Prop $kv 'slot' $null)){$out += [ordered]@{slot=[int](Prop $kv 'slot' 0);map=[int](Prop $kv 'map' 0);palette=[int](Prop $kv 'palette' 0);x=[int](Prop $kv 'x' 0);y=[int](Prop $kv 'y' 0);width=[int](Prop $kv 'width' 0);height=[int](Prop $kv 'height' 0);priority=[int](Prop $kv 'priority' 0);prioritySource='sprite word1 bit15';drawOrder=[int](Prop $kv 'slot' 0);visiblePixels=[int64](Prop $kv 'visible_pixels' 0);visibleLeft=[int](Prop $kv 'visible_left' 0);visibleTop=[int](Prop $kv 'visible_top' 0);visibleRight=[int](Prop $kv 'visible_right' 0);visibleBottom=[int](Prop $kv 'visible_bottom' 0)}}};return @($out)
}
function Get-PaletteEntryList([int]$bank){if($bank -lt 0 -or $bank -gt 255){throw 'bank must be 0..255'};$out=@();for($i=0;$i -lt 16;$i++){$out += [ordered]@{bank=$bank;pen=$i;index=($bank*16+$i)}};return @($out)}
function Get-ResearchStateSnapshot([string]$include='sprites,regions,gameplay,track'){
    $want=@($include -split ','|ForEach-Object{$_.Trim().ToLowerInvariant()}|Where-Object{$_});$o=[ordered]@{schema='chq-state-snapshot-v1';created=(Get-Date).ToString('o');status=(Parse-Kv (Invoke-Chq @('status')))}
    if($want -contains 'sprites'){$o.sprites=Get-LiveSprites};if($want -contains 'regions'){$o.regions=Get-ResearchRegions};if($want -contains 'gameplay'){$o.gameplay=Get-LiveGameplayRegistry};if($want -contains 'track'){$o.track=Get-LiveTrackState};return $o
}

function Get-PlatformInfo(){
    $profile=Get-GameProfile $activeProfileId
    $features=if(Test-Path $featureManifestPath){Get-Content $featureManifestPath -Raw|ConvertFrom-Json}else{$null}
    return [ordered]@{schema='chq-platform-info-v1';product='ChaseHQ-Native';build=$workbenchVersion;scriptEngine='chq-research-script-v2';activeProfile=$activeProfileId;hardware='taitoz';debugPort=$Port;httpPort=$HttpPort;session=$sessionPath;featureManifest=$(if($features){[string](Prop $features 'schema' '')}else{''});audioFoundation=$true;audioRuntimeInjection=$false;forensicTimeline='chq-forensic-timeline-v1';timelineAudioRuntime=$false}
}
function Get-FeatureManifest(){if(-not(Test-Path $featureManifestPath)){throw 'Feature manifest not found'};$m=Get-Content $featureManifestPath -Raw|ConvertFrom-Json;if([string](Prop $m 'build' '') -ne $workbenchVersion){throw "Feature manifest build mismatch: manifest=$([string](Prop $m 'build' '')) runtime=$workbenchVersion"};return $m}
function Get-SemanticObjects(){if(-not(Test-Path $semanticObjectsPath)){return @()};$j=Get-Content $semanticObjectsPath -Raw|ConvertFrom-Json;return @($j.objects)}
function Get-AudioCapabilities(){if(-not(Test-Path $audioProfilePath)){throw 'Audio capability descriptor not found'};return Get-Content $audioProfilePath -Raw|ConvertFrom-Json}
function Get-AudioChannels(){ $a=Get-AudioCapabilities; return @($a.channels) }
function Get-LiveRegistryItems(){ $g=Get-LiveGameplayRegistry; return @($g.items) }
function Get-TrackRecords([int]$before=4,[int]$after=12){
    if($before -lt 0 -or $before -gt 64 -or $after -lt 0 -or $after -gt 64){throw 'before/after must be 0..64'}
    $t=Get-LiveTrackState;$idx=[int]$t.recordIndex;$out=@();$start=[Math]::Max(0,$idx-$before);$end=$idx+$after
    for($i=$start;$i -le $end;$i++){$addr=0x109000+($i*8);$b=@();for($k=0;$k -lt 8;$k++){$b += [int](Read-ChqValue 'A' ('{0:X}' -f ($addr+$k)) 8)};$curve=Signed8 $b[0];$shape=Signed8 $b[1];$out += [ordered]@{index=$i;current=($i -eq $idx);address=('0x{0:X6}' -f $addr);bytes=(($b|ForEach-Object{'{0:X2}' -f $_}) -join ' ');curve=$curve;shapeTarget=$shape;flagsType=('0x{0:X2}' -f $b[2]);geometryCode=('0x{0:X2}' -f $b[3]);eventCode=('0x{0:X2}' -f $b[4]);visualSelector=('0x{0:X2}' -f $b[5]);featureTrigger=('0x{0:X2}' -f $b[6]);controlFlags=('0x{0:X2}' -f $b[7])}}
    return @($out)
}
function Get-EvidenceNames(){ $items=@();foreach($d in Get-ChildItem $evidencePath -Directory -ErrorAction SilentlyContinue|Sort-Object LastWriteTime -Descending){$items += [ordered]@{name=$d.Name;modified=$d.LastWriteTime.ToString('o');path=$d.FullName}};return @($items) }


function Begin-ResearchExperiment([string]$name=''){
    if($script:activeExperiment){throw 'An experiment is already active'}
    $id=if([string]::IsNullOrWhiteSpace($name)){('experiment-'+(Get-Date -Format 'yyyyMMdd-HHmmss'))}else{SafeName $name}
    $base=Join-Path $evidencePath 'experiments';New-Item -ItemType Directory -Force -Path $base|Out-Null;$dir=Join-Path $base $id;$n=1;while(Test-Path $dir){$id=($id+'-'+$n);$n++;$dir=Join-Path $base $id};New-Item -ItemType Directory -Force -Path $dir|Out-Null
    $script:activeExperiment=[ordered]@{schema='chq-experiment-v1';id=$id;created=(Get-Date).ToString('o');build=$workbenchVersion;profile=$activeProfileId;startStatus=(Parse-Kv (Invoke-Chq @('status')));captures=@();path=$dir}
    $script:activeExperiment|ConvertTo-Json -Depth 10|Set-Content (Join-Path $dir 'manifest.json') -Encoding UTF8
    return $script:activeExperiment
}
function Capture-ResearchExperiment([string]$label='capture',[string]$include='sprites,regions,gameplay,track'){
    if(-not $script:activeExperiment){throw 'No active experiment'};$safe=SafeName $label;$dir=$script:activeExperiment.path;$png=Join-Path $dir ($safe+'.png');$json=Join-Path $dir ($safe+'-state.json');$shot=Invoke-Chq @('screenshot',$png);$state=Get-ResearchStateSnapshot $include;$state|ConvertTo-Json -Depth 14|Set-Content $json -Encoding UTF8;$cap=[ordered]@{label=$safe;created=(Get-Date).ToString('o');frame=(Prop $state.status 'frame' $null);screenshot=$png;state=$json;screenshotResult=$shot};$script:activeExperiment.captures += $cap;$script:activeExperiment|ConvertTo-Json -Depth 12|Set-Content (Join-Path $dir 'manifest.json') -Encoding UTF8;return $cap
}
function End-ResearchExperiment(){if(-not $script:activeExperiment){throw 'No active experiment'};$script:activeExperiment.ended=(Get-Date).ToString('o');$script:activeExperiment.endStatus=(Parse-Kv (Invoke-Chq @('status')));$m=$script:activeExperiment;$m|ConvertTo-Json -Depth 12|Set-Content (Join-Path $m.path 'manifest.json') -Encoding UTF8;$script:activeExperiment=$null;return $m}


# v0.66.9.0 Chase H.Q.-specific reversible Game Lab helpers.
$script:GameLabPatchIds=@{}
function Remove-GameLabPatch([string]$key){
    if(-not $script:GameLabPatchIds.ContainsKey($key)){return 'OK no tracked patch'}
    $id=[string]$script:GameLabPatchIds[$key];$script:GameLabPatchIds.Remove($key)
    $r=Invoke-Chq @('patch','remove',$id);if($r -like 'ERR*'){return "OK tracked patch already absent id=$id"};return $r
}
function Set-GameLabFreeze([string]$key,[string]$address,[int]$width,[uint32]$value){
    if($script:GameLabPatchIds.ContainsKey($key)){$null=Remove-GameLabPatch $key}
    $hex=('{0:X}' -f $value);$r=Invoke-Chq @('patch','freeze','A',$address,[string]$width,$hex)
    if($r -notmatch 'id=(\d+)'){throw "Could not determine patch id: $r"};$script:GameLabPatchIds[$key]=[int]$Matches[1];return $r
}
function Test-GameLabWasRunning(){
    $s=Parse-Kv (Invoke-Chq @('status'))
    return ([int](Prop $s 'paused' 1) -eq 0)
}
function Restore-GameLabRunState([bool]$wasRunning){
    if($wasRunning){$null=Invoke-Chq @('resume')}
}
function Get-GameLabState(){
    $remaining=Read-ChqValue 'A' '1003A2' 16 'game-lab turbos remaining'
    $activeRaw=Read-ChqValue 'A' '10040F' 8 'game-lab turbo active'
    $timer=Read-ChqValue 'A' '100414' 16 'game-lab turbo timer'
    $internalSpeed=Read-ChqValue 'A' '10041C' 16 'game-lab internal physics speed'
    $displaySpeedRaw=Read-ChqValue 'A' '100400' 16 'game-lab HUD display speed'
    $targetHealth=Read-ChqValue 'A' '1002AE' 16 'game-lab target remaining-hit counter'
    $targetCtl=Invoke-Chq @('target','one-hit','status')
    $targetOneHit=($targetCtl -match 'target_one_hit=1')
    $order=(Invoke-Chq @('sprites','tie-break'))
    $tie=if($order -match 'tie_break=([^\s]+)'){$Matches[1]}else{'unknown'}
    return [ordered]@{turbosRemaining=[int]$remaining;turboActive=(($activeRaw -band 2) -ne 0);turboActiveRaw=[int]$activeRaw;turboTimer=[int]$timer;turboExpiry=0xD2;currentSpeed=[int]$internalSpeed;internalSpeed=[int]$internalSpeed;displaySpeed=[int](Bcd-Value $displaySpeedRaw 4);displaySpeedRaw=('0x{0:X4}' -f $displaySpeedRaw);targetHealth=$(if($targetHealth-eq 0xFFFF){'DEFEATED'}else{[int]$targetHealth});targetHealthRaw=('0x{0:X4}' -f $targetHealth);targetOneHit=[bool]$targetOneHit;spriteTieBreak=$tie;infiniteTurboStock=$script:GameLabPatchIds.ContainsKey('turbo-stock');infiniteTurboActive=$script:GameLabPatchIds.ContainsKey('turbo-active');speedFrozen=$script:GameLabPatchIds.ContainsKey('speed')}
}
function Get-Tc0100TextCharacter([string]$codeText,[int]$paletteBank=0,[int]$scale=8){
    $code=[int](Hex-U32 $codeText)
    if($code -lt 0 -or $code -gt 255){throw 'code must be 00..FF'}
    if($paletteBank -lt 0 -or $paletteBank -gt 255){throw 'palette must be 0..255'}
    if($scale -lt 1 -or $scale -gt 32){throw 'scale must be 1..32'}

    # TC0100SCN text characters are 256 RAM-defined 8x8 2bpp glyphs. Existing
    # renderer provenance maps character code N to 16 bytes at C06000 + N*16.
    $addr=0xC06000+($code*16)
    $dump=Invoke-Chq @('readrange','A',('{0:X}' -f $addr),'16')
    $bytes=New-Object 'System.Collections.Generic.List[byte]'
    foreach($line in ($dump -split "`r?`n")){
        if($line -match '^[0-9A-Fa-f]{8}:\s*(.*)$'){
            foreach($h in ($Matches[1] -split '\s+')){
                if($h -match '^[0-9A-Fa-f]{2}$'){$bytes.Add([Convert]::ToByte($h,16))}
            }
        }
    }
    if($bytes.Count -ne 16){throw "Expected 16 character bytes, got $($bytes.Count)"}

    $pens=@()
    for($y=0;$y -lt 8;$y++){
        $row=@();$b0=$bytes[$y*2];$b1=$bytes[$y*2+1]
        for($x=0;$x -lt 8;$x++){
            $bit=7-$x
            $p0=($b0 -shr $bit) -band 1
            $p1=($b1 -shr $bit) -band 1
            $row+=($p0 -bor ($p1 -shl 1))
        }
        $pens+=,@($row)
    }

    $pal=Parse-Kv (Invoke-Chq @('palette','bank',[string]$paletteBank))
    $rawCols=@()
    for($i=0;$i -lt 4;$i++){
        $k='pen'+$i
        $rawCols+=,[int](Hex-U32 ([string](Prop $pal $k '0')))
    }

    $dir=Join-Path $evidencePath 'tile-inspect'
    New-Item -ItemType Directory -Force -Path $dir|Out-Null
    $st=Parse-Kv (Invoke-Chq @('status'));$curFrame=[int](Prop $st 'frame' 0)
    $stem=('tc0100scn-text-{0:X2}-p{1:D3}-f{2:D6}' -f $code,$paletteBank,$curFrame)
    $png=Join-Path $dir ($stem+'.png')
    $metaPath=Join-Path $dir ($stem+'.json')
    $pensPath=Join-Path $dir ($stem+'-pens.txt')

    Add-Type -AssemblyName System.Drawing -ErrorAction Stop
    $bmp=New-Object -TypeName System.Drawing.Bitmap -ArgumentList (8*$scale),(8*$scale),([System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    try{
        for($y=0;$y -lt 8;$y++){
            for($x=0;$x -lt 8;$x++){
                $pen=[int]$pens[$y][$x]
                $raw=[int]$rawCols[$pen]
                $rv=$raw -band 31;$gv=($raw -shr 5) -band 31;$bv=($raw -shr 10) -band 31
                $r=($rv -shl 3) -bor ($rv -shr 2)
                $g=($gv -shl 3) -bor ($gv -shr 2)
                $b=($bv -shl 3) -bor ($bv -shr 2)
                $a=if($pen -eq 0){0}else{255}
                $c=[System.Drawing.Color]::FromArgb($a,$r,$g,$b)
                for($yy=0;$yy -lt $scale;$yy++){
                    for($xx=0;$xx -lt $scale;$xx++){$bmp.SetPixel($x*$scale+$xx,$y*$scale+$yy,$c)}
                }
            }
        }
        $bmp.Save($png,[System.Drawing.Imaging.ImageFormat]::Png)
    } finally {$bmp.Dispose()}

    $penLines=@();foreach($row in $pens){$penLines+=(@($row|ForEach-Object{[string]$_}) -join ' ')}
    Set-Content $pensPath ($penLines -join "`r`n") -Encoding ASCII
    $meta=[ordered]@{
        schema='chq-tc0100scn-character-v1';build=$workbenchVersion;source='TC0100SCN RAM text character';
        code=$code;codeHex=('0x{0:X2}' -f $code);address=('0x{0:X6}' -f $addr);
        bytes=@($bytes|ForEach-Object{('{0:X2}' -f $_)});paletteBank=$paletteBank;
        paletteRaw=@($rawCols|ForEach-Object{('0x{0:X4}' -f $_)});transparentPen=0;
        width=8;height=8;bpp=2;scale=$scale;pens=$pens;png=$png;pensFile=$pensPath;metadata=$metaPath
    }
    $meta|ConvertTo-Json -Depth 10|Set-Content $metaPath -Encoding UTF8
    return $meta
}

$scriptApiActions=(@('release.assert-version','frame.snapshot.assert-exact','frame.snapshot.assert-scene-no-sprites','layer.offsets.assert','layer.offsets.get','layer.offsets.set','layer.offsets.reset','tc0100.mode','status','capabilities','input.ports','events.tail','sprites.tail','memory.read','memory.read-range','memory.write','memory.trace.start','memory.trace.status','memory.trace.tail','memory.trace.stop','memory.trace.clear','register.read','register.write','patch.freeze','patch.replace','patch.suppress','patch.list','patch.remove','patch.clear','cpu.disassemble','control.pause','control.resume','control.step-frame','control.run-frames','control.abort','control.step-instruction','timer.hold','window.status','window.always-on-top','window.fullscreen','window.scale','window.show','window.hide','window.minimize','window.restore','tuning.handling.get','tuning.handling.set','tuning.handling.reset','collision.state','collision.response.get','collision.response.set','collision.response.reset','input.steering.get','input.steering.set','palette.bank','palette.entry.set','palette.entry.restore','palette.overrides.clear','palette.trace.start','palette.trace.status','palette.trace.tail','palette.trace.stop','palette.trace.clear','input.ioc.xor','input.ioc.clear','input.ioc.pulse','input.ioc.sweep','checkpoint.list','checkpoint.load','checkpoint.save','logs.list','logs.tail','evidence.capture','evidence.list','evidence.view','evidence.load','evidence.zip','frame.capture','frame.snapshot','frame.snapshot.assert-overlap','frame.snapshot.list','image.compare','image.diff','image.regions','regions.list','profiles.list','profile.get','sprites.list','sprites.maps','sprites.palettes','sprite.inspect','sprite.map.inspect','sprite.order.inspect','sprite.order.set','sprite.override','sprite.override.clear','sprite.visual','sprite.visual.clear','palette.entries','state.snapshot','gameplay.registry','track.state','track.record.start','track.record.stop','track.record.clear','track.record.status','track.record.sample','track.svg.export','track.map.export','course.follow.status','course.follow.configure','course.follow.start','course.follow.stop','course.follow.reset','course.survey.status','course.survey.start','course.survey.stop','next-steps','memory.export','schema','platform.info','features.list','script.functions','registry.list','track.records','objects.list','audio.capabilities','audio.channels','audio.events','audio.waveform.plan','audio.export.plan','audio.inject.plan','evidence.names','docs.index','docs.search','docs.get','docs.glossary','docs.handbook','docs.page','docs.handover','docs.export','knowledge.list','knowledge.get','knowledge.search','knowledge.related','experiment.begin','experiment.capture','experiment.status','experiment.end','timeline.record.start','timeline.record.stop','timeline.status','timeline.list','timeline.load','timeline.seek','timeline.next','timeline.prev','timeline.frames','timeline.play','timeline.trim.event','timeline.scan.memory','timeline.analyze.auto','timeline.fork-live','timeline.unload','timeline.reset','schema.all','script.schema','script.runs','game.experiment.status','game.target.one-hit','game.turbo.stock','game.turbo.active','game.turbo.fire','game.turbo.timer.reset','game.turbo.remaining.set','game.speed.set','game.speed.freeze','game.speed.clear','game.sprite.order.inspect','game.sprite.order.set','tile.inspect','ui.prompt') | Select-Object -Unique)
$scriptActionSchemas=[ordered]@{
 'game.experiment.status'=[ordered]@{params=@();example='api game.experiment.status';description='Read Chase H.Q.-specific Game Lab state including target remaining-hit counter, one-hit target mode, turbo/current speed and reversible override toggles'}
 'game.target.one-hit'=[ordered]@{params=@('enabled=true|false required');example='api game.target.one-hit enabled=true';description='Enable/disable the Stage 1 special-target one-hit research cheat. When enabled, the confirmed 0x1002AE counter is armed to zero immediately before authentic damage PC 0xA112 so the genuine terminal underflow/defeat path executes.'}
 'game.turbo.stock'=[ordered]@{params=@('enabled=true|false required','value=0..65535 optional default 3');example='api game.turbo.stock enabled=true value=3';description='Enable/disable an infinite turbo-stock freeze on confirmed CPU-A 0x1003A2'}
 'game.turbo.active'=[ordered]@{params=@('enabled=true|false required');example='api game.turbo.active enabled=true';description='Enable/disable infinite active turbo by freezing confirmed duration timer CPU-A 0x100414 at zero; activation still uses the normal turbo path'}
 'game.turbo.fire'=[ordered]@{params=@('frames=N optional default 3');example='api game.turbo.fire frames=3';description='Pulse the confirmed IOC P3 turbo input (mask 0x01)'}
 'game.turbo.timer.reset'=[ordered]@{params=@();example='api game.turbo.timer.reset';description='Reset the confirmed turbo duration timer at CPU-A 0x100414 to zero'}
 'game.turbo.remaining.set'=[ordered]@{params=@('value=0..65535 required');example='api game.turbo.remaining.set value=3';description='Set the confirmed turbos-remaining value at CPU-A 0x1003A2 once'}
 'game.speed.set'=[ordered]@{params=@('value=0..65535 required');example='api game.speed.set value=602';description='Set raw internal/physics speed CPU-A 0x10041C once while preserving the prior run/pause state; HUD speed at 0x100400 is a separate packed-BCD display field'}
 'game.speed.freeze'=[ordered]@{params=@('enabled=true|false required','value=0..65535 optional; current internal speed used when enabling if omitted');example='api game.speed.freeze enabled=true value=700';description='Enable/disable a reversible freeze on raw internal/physics speed CPU-A 0x10041C while preserving the prior run/pause state'}
 'game.speed.clear'=[ordered]@{params=@();example='api game.speed.clear';description='Remove only the Game Lab current-speed freeze, leaving unrelated research patches untouched'}
 'tile.inspect'=[ordered]@{params=@('code=00..FF required (hex)','palette=0..255 optional default 0','scale=1..32 optional default 8');example='api tile.inspect code=0C palette=0 scale=8';description='Inspect/export a TC0100SCN RAM text character as raw 2bpp pens, palette-aware PNG and JSON metadata'}
 'status'=[ordered]@{params=@();example='api status';description='Read machine/frame status'}
 'capabilities'=[ordered]@{params=@();example='api capabilities';description='List native debugger capabilities'}
 'memory.read'=[ordered]@{params=@('cpu=A|B required','address=HEX required','width=8|16|32 optional default 16');example='api memory.read cpu=A address=10A048 width=8';description='Read one value'}
 'memory.read-range'=[ordered]@{params=@('cpu=A|B required','address=HEX required','length=COUNT required, decimal or 0xHEX, max 65536');example='api memory.read-range cpu=A address=109000 length=512';description='Read a contiguous byte range and return a compact hex dump'}
 'memory.write'=[ordered]@{params=@('cpu=A|B required','address=HEX required','width=8|16|32 optional default 16','value=HEX required','expect=HEX optional compare-before-write');example='api memory.write cpu=A address=10A04A width=16 value=0051 expect=0011';description='Debugger-safe write; pauses machine and reports old/new values'}
 'memory.trace.start'=[ordered]@{params=@('cpu=A|B required','address=HEX required','length=COUNT optional default 1','width=8|16|32|any optional default any','limit=64..1000000 optional default 4096');example='api memory.trace.start cpu=A address=0x100303 length=1 width=8 limit=256';description='Start bounded generic memory-write tracing for an address/range; captures exact writer frame, CPU, PC, width, old/new value, changed flag and write count'}
 'memory.trace.status'=[ordered]@{params=@();example='api memory.trace.status';description='Read generic memory-write trace configuration, event count and limit'}
 'memory.trace.tail'=[ordered]@{params=@('count=N optional default 20');example='api memory.trace.tail count=50';description='Return recent matching memory-write events including unchanged writes'}
 'memory.trace.stop'=[ordered]@{params=@();example='api memory.trace.stop';description='Stop generic memory-write tracing while retaining captured events'}
 'memory.trace.clear'=[ordered]@{params=@();example='api memory.trace.clear';description='Clear captured generic memory-write trace events and reset trace write count'}
 'register.read'=[ordered]@{params=@('cpu=A|B required','name=D0..D7|A0..A7|PC|SR|SP optional');example='api register.read cpu=A name=D0';description='Read one register or complete CPU register set'}
 'register.write'=[ordered]@{params=@('cpu=A|B required','name=REGISTER required','value=HEX required');example='api register.write cpu=A name=D0 value=00000001';description='Write a CPU register while paused'}
 'patch.freeze'=[ordered]@{params=@('cpu=A|B required','address=HEX required','width=8|16|32 required','value=HEX required','expect=HEX optional');example='api patch.freeze cpu=A address=10A04A width=16 value=0051';description='Immediately write and continuously freeze a value; returns patch ID'}
 'patch.replace'=[ordered]@{params=@('cpu=A|B required','address=HEX required','width=8|16|32 required','value=HEX required');example='api patch.replace cpu=A address=10A04A width=16 value=0051';description='Replace matching game writes with a fixed value'}
 'patch.suppress'=[ordered]@{params=@('cpu=A|B required','address=HEX required','width=8|16|32 required');example='api patch.suppress cpu=A address=10A04A width=16';description='Suppress writes and preserve current value'}
 'patch.list'=[ordered]@{params=@();example='api patch.list';description='List active live patches'}
 'patch.remove'=[ordered]@{params=@('id=N required');example='api patch.remove id=1';description='Remove one live patch'}
 'patch.clear'=[ordered]@{params=@();example='api patch.clear';description='Remove all live patches'}
 'memory.export'=[ordered]@{params=@('cpu=A|B required','address=HEX + length=COUNT OR start=HEX + end=HEX','output=relative evidence path optional','name=basename optional legacy alias','max 1 MiB');example='api memory.export cpu=A address=109000 length=512 output=evidence\\stage1-track-109000.bin';description='Export a contiguous debugger-safe byte range under the evidence root'}
 'cpu.disassemble'=[ordered]@{params=@('cpu=A|B optional default A','address=HEX required','count=N optional default 16');example='api cpu.disassemble cpu=A address=008120 count=64';description='Read-only disassembly'}
 'control.run-frames'=[ordered]@{params=@('frames=1..100000 required','timeout=1000..3600000 optional default 900000 ms');example='api control.run-frames frames=100 timeout=900000';description='Run exactly N emulated frames and pause; cancellation-aware with configurable progress-aware timeout'}
 'control.abort'=[ordered]@{params=@();example='api control.abort';description='Out-of-band pause/cancel of active frame execution'}
 'checkpoint.load'=[ordered]@{params=@('file=packaged .chqstate required','run=true|false optional default false');example='api checkpoint.load file=stage1-gameplay-2064.chqstate run=false';description='Load a packaged deterministic checkpoint'}
 'checkpoint.save'=[ordered]@{params=@('file=packaged filename required','name=display name optional','role=role optional','description=text optional');example='api checkpoint.save file=stage1-gradient-test.chqstate role=research';description='Save current deterministic state into packaged checkpoints and write metadata'}
 'frame.snapshot'=[ordered]@{params=@('name=optional snapshot name');example='api frame.snapshot name=stage1-bad-foreground';description='Capture reconstructable layered frame snapshot with alpha contribution layers, raw sources, HUD variants, semantic sprite export and manifest'}
 'frame.snapshot.assert-overlap'=[ordered]@{params=@('name=NAME required','front=SLOT required','back=SLOT required','frontSolo=NAME required','backSolo=NAME required');example='api frame.snapshot.assert-overlap name=normal front=82 back=81 frontSolo=body backSolo=shadow';description='Assert non-vacuous front ownership, uncovered back visibility, overlap coverage and exact per-sprite coordinates against solo runtime source renders; throws on mismatch'}
 'frame.snapshot.list'=[ordered]@{params=@('require=NAME optional; throw if NAME is not present in the returned list');example='api frame.snapshot.list require=stage1-study';description='List structured frame snapshots, optionally asserting that a named snapshot is immediately discoverable'}
 'regions.list'=[ordered]@{params=@();example='api regions.list';description='Return authoritative image regions as structured JSON'}
 'profiles.list'=[ordered]@{params=@();example='api profiles.list';description='List packaged game-profile descriptors; metadata only and does not switch runtime'}
 'profile.get'=[ordered]@{params=@('id=profile id optional default chasehq');example='api profile.get id=sci';description='Read one packaged game profile and whether it is active/activatable'}
 'sprites.list'=[ordered]@{params=@();example='api sprites.list';description='Return current live composed sprite records as structured JSON'}
 'sprites.maps'=[ordered]@{params=@();example='api sprites.maps';description='Return unique live sprite map IDs'}
 'sprites.palettes'=[ordered]@{params=@();example='api sprites.palettes';description='Return unique live sprite palette IDs'}
 'sprite.inspect'=[ordered]@{params=@('slot=N required');example='api sprite.inspect slot=44';description='Inspect one currently active sprite slot'}
 'sprite.map.inspect'=[ordered]@{params=@('slot=N optional','map=N optional; resolves all currently active matching slots');example='api sprite.map.inspect map=475';description='Inspect raw spritemap/chunk provenance and raw pen histogram for an active sprite slot or all active slots using a requested map'}
 'sprite.order.inspect'=[ordered]@{params=@();example='api sprite.order.inspect';description='Report the exact current sprite traversal/tie-break and resolved active draw order'}
 'sprite.order.set'=[ordered]@{params=@('mode=higher-slot|lower-slot required');example='api sprite.order.set mode=lower-slot';description='Reversibly switch same-priority sprite tie-breaking at runtime for compositor A/B experiments'}
 'game.sprite.order.inspect'=[ordered]@{params=@();example='api game.sprite.order.inspect';description='Game Lab alias for the current sprite-order report'}
 'game.sprite.order.set'=[ordered]@{params=@('mode=higher-slot|lower-slot required');example='api game.sprite.order.set mode=lower-slot';description='Game Lab alias for reversible same-priority sprite ordering experiment'}
 'schema.all'=[ordered]@{params=@();example='api schema.all';description='Return every Script API action contract from the authoritative in-process schema'}
 'script.schema'=[ordered]@{params=@();example='api script.schema';description='Return the authoritative Research Script v2 grammar and limits'}
 'script.runs'=[ordered]@{params=@('limit=N optional default 25 max 100','offset=N optional default 0','status=PASS|FAIL|CANCELLED optional','version=VERSION optional','session=SESSION optional');example='api script.runs limit=25 status=PASS';description='Return a bounded summary page of authoritative completed script runs; large console/source/artifact payloads are not embedded'}
 'ui.prompt'=[ordered]@{params=@('message=TEXT required; underscores render as spaces','pause=true|false optional default true','top=true|false optional default true','timeout=1000..3600000 optional default 300000');example='api ui.prompt message=Hold_DOWN_then_press_ENTER pause=true top=true';description='Show an SDL-native prompt overlay and wait for Enter/Escape while physical gameplay keys remain live'}
 'sprite.override'=[ordered]@{params=@('slot=N required','map=N optional','palette=N optional','visible=true|false optional');example='api sprite.override slot=44 map=182';description='Presentation-only live SDL sprite override; omitted fields retain original values'}
 'sprite.override.clear'=[ordered]@{params=@();example='api sprite.override.clear';description='Clear all live sprite presentation overrides'}
 'sprite.visual'=[ordered]@{params=@('slot=N required','mode=solo|hide|flash required');example='api sprite.visual slot=44 mode=flash';description='Live SDL sprite isolation/highlight mode'}
 'sprite.visual.clear'=[ordered]@{params=@();example='api sprite.visual.clear';description='Clear sprite solo/hide/flash presentation mode'}
 'palette.entries'=[ordered]@{params=@('bank=0..255 required');example='api palette.entries bank=75';description='Return the 16 palette entry indices for a bank'}
 'state.snapshot'=[ordered]@{params=@('include=sprites,regions,gameplay,track optional');example='api state.snapshot include=sprites,regions,gameplay,track';description='Capture coherent structured research state from the current paused instant'}
 'input.ports'=[ordered]@{params=@();example='api input.ports';description='Read current IOC/input port state'}
 'events.tail'=[ordered]@{params=@('count=N optional default 100');example='api events.tail count=50';description='Return recent research events'}
 'sprites.tail'=[ordered]@{params=@('count=N optional default 100');example='api sprites.tail count=100';description='Return recent semantic sprite-change events'}
 'control.pause'=[ordered]@{params=@();example='api control.pause';description='Pause emulation'}
 'control.resume'=[ordered]@{params=@();example='api control.resume';description='Resume emulation'}
 'control.step-frame'=[ordered]@{params=@('frames=N optional default 1');example='api control.step-frame frames=1';description='Advance paused emulation by N frames'}
 'control.step-instruction'=[ordered]@{params=@('cpu=A|B optional default A');example='api control.step-instruction cpu=A';description='Step one CPU instruction'}
 'timer.hold'=[ordered]@{params=@('enabled=true|false optional default true');example='api timer.hold enabled=true';description='Freeze or resume the game timer helper'}
 'window.always-on-top'=[ordered]@{params=@('enabled=true|false optional default true');example='api window.always-on-top enabled=true';description='Toggle SDL always-on-top'}
 'layer.offsets.get'=[ordered]@{params=@();example='api layer.offsets.get';description='Read live presentation offsets for all five layers'}
 'layer.offsets.set'=[ordered]@{params=@('layer=bg0|bg1|text|sprites|road|all required','x=-4096..4096 required','y=-4096..4096 required');example='api layer.offsets.set layer=bg0 x=0 y=4';description='Set reversible live presentation offsets; hardware state is unchanged'}
 'layer.offsets.reset'=[ordered]@{params=@('layer=bg0|bg1|text|sprites|road|all optional default all');example='api layer.offsets.reset layer=all';description='Reset presentation offsets to zero'}
 'tc0100.mode'=[ordered]@{params=@('mode=legacy|corrected optional');example='api tc0100.mode mode=corrected';description='Select diagnostic old/new X transform; this is not independent shadow verification'}
 'frame.snapshot.assert-scene-no-sprites'=[ordered]@{params=@('name=NAME required');example='api frame.snapshot.assert-scene-no-sprites name=rc27-corrected';description='Require exact reconstruction, visible sprite ownership and changed no-sprites pixels'}
 'layer.offsets.assert'=[ordered]@{params=@('layer=bg0|bg1|text|sprites|road required','x=INTEGER required','y=INTEGER required');example='api layer.offsets.assert layer=bg0 x=0 y=4';description='Assert live layer offsets against expected values'}
 'release.assert-version'=[ordered]@{params=@();example='api release.assert-version';description='Require native status, Workbench and feature manifest to share the full release label'}
 'frame.snapshot.assert-exact'=[ordered]@{params=@('name=NAME required','requireColumnZero=true|false optional default false');example='api frame.snapshot.assert-exact name=rc27-corrected';description='Require exact=true, verifiedExact=true and zero reconstruction mismatches'}
 'window.status'=[ordered]@{params=@();example='api window.status';description='Read live SDL window state'}
 'window.fullscreen'=[ordered]@{params=@('enabled=true|false optional default true');example='api window.fullscreen enabled=true';description='Toggle SDL fullscreen'}
 'window.scale'=[ordered]@{params=@('scale=1..8 required');example='api window.scale scale=4';description='Set SDL window integer scale and leave fullscreen'}
 'window.show'=[ordered]@{params=@();example='api window.show';description='Show SDL window'}
 'window.hide'=[ordered]@{params=@();example='api window.hide';description='Hide SDL window'}
 'window.minimize'=[ordered]@{params=@();example='api window.minimize';description='Minimize SDL window'}
 'window.restore'=[ordered]@{params=@();example='api window.restore';description='Restore SDL window'}
 'tuning.handling.get'=[ordered]@{params=@();example='api tuning.handling.get';description='Read handling override plus instruction-time turn/table/speed/lateral coefficient telemetry when valid'}
 'tuning.handling.set'=[ordered]@{params=@('cornering=NUMBER optional default 1','speedRetain=NUMBER optional default 1');example='api tuning.handling.set cornering=1.15 speedRetain=1.0';description='Set live handling multipliers'}
 'tuning.handling.reset'=[ordered]@{params=@();example='api tuning.handling.reset';description='Restore authentic handling values'}
 'collision.state'=[ordered]@{params=@();example='api collision.state';description='Read evidence-scoped collision-response suppression state and counters; detection remains live'}
 'collision.response.get'=[ordered]@{params=@();example='api collision.response.get';description='Read whether authentic collision response writes are enabled'}
 'collision.response.set'=[ordered]@{params=@('enabled=true|false required');example='api collision.response.set enabled=false';description='Enable/disable only proven collision response writes while preserving detection/event logic'}
 'collision.response.reset'=[ordered]@{params=@();example='api collision.response.reset';description='Restore authentic collision response'}
 'input.steering.get'=[ordered]@{params=@();example='api input.steering.get';description='Read raw IOC steering injection (signed 12-bit encoding, centre=0000)'}
 'input.steering.set'=[ordered]@{params=@('value=HEX required');example='api input.steering.set value=0060';description='Set raw IOC steering injection; use 0000 centre, 0060 right, 0FA0 left for proven full steering'}
 'palette.bank'=[ordered]@{params=@('bank=N required');example='api palette.bank bank=75';description='Inspect one 16-pen palette bank'}
 'palette.entry.set'=[ordered]@{params=@('index=N required','raw=HEX required');example='api palette.entry.set index=1209 raw=7FFF';description='Freeze one live palette entry'}
 'palette.entry.restore'=[ordered]@{params=@('index=N required');example='api palette.entry.restore index=1209';description='Return one palette entry to game control'}
 'palette.overrides.clear'=[ordered]@{params=@();example='api palette.overrides.clear';description='Clear all palette freezes'}
 'palette.trace.start'=[ordered]@{params=@('indices=comma-separated palette indices 0..4095 required','limit=64..1000000 optional default 4096');example='api palette.trace.start indices=1037,1053 limit=4096';description='Start bounded semantic TC0110PCR write tracing for selected palette entries; captures frame, CPU, PC, old/new value and write count'}
 'palette.trace.status'=[ordered]@{params=@();example='api palette.trace.status';description='Read palette trace enabled state, selected indices, event count and limit'}
 'palette.trace.tail'=[ordered]@{params=@('count=N optional default 20');example='api palette.trace.tail count=50';description='Return recent matching palette-write events including frame, CPU, PC, index/bank/pen, old/new value, changed flag and write count'}
 'palette.trace.stop'=[ordered]@{params=@();example='api palette.trace.stop';description='Stop palette-write capture while retaining captured events'}
 'palette.trace.clear'=[ordered]@{params=@();example='api palette.trace.clear';description='Clear captured palette-write events without changing the selected trace filter'}
 'input.ioc.xor'=[ordered]@{params=@('port=N required','mask=HEX required');example='api input.ioc.xor port=3 mask=20';description='Set held IOC XOR mask'}
 'input.ioc.clear'=[ordered]@{params=@();example='api input.ioc.clear';description='Clear held IOC XOR masks'}
 'input.ioc.pulse'=[ordered]@{params=@('port=N required','mask=HEX required','frames=N optional default 3');example='api input.ioc.pulse port=3 mask=20 frames=3';description='Pulse IOC mask for emulated frames'}
 'input.ioc.sweep'=[ordered]@{params=@('checkpoint=FILE optional','port=N optional default 3','masks=comma list optional','pulseFrames=N optional','observeFrames=N optional');example='api input.ioc.sweep checkpoint=stage1-gameplay-2064.chqstate port=3 masks=01,02,04';description='Run deterministic IOC mask sweep and collect evidence'}
 'checkpoint.list'=[ordered]@{params=@();example='api checkpoint.list';description='List packaged checkpoints'}
 'logs.list'=[ordered]@{params=@();example='api logs.list';description='List active-session logs'}
 'logs.tail'=[ordered]@{params=@('file=RELATIVE required','lines=N optional default 80');example='api logs.tail file=@runtime/web-listener.log lines=80';description='Tail a log using the exact identifier returned by logs.list'}
 'evidence.capture'=[ordered]@{params=@('name=NAME optional');example='api evidence.capture name=experiment-a';description='Capture checkpoint/events/sprites/screenshot evidence bundle'}
 'evidence.list'=[ordered]@{params=@();example='api evidence.list';description='List evidence captures'}
 'evidence.view'=[ordered]@{params=@('name=NAME optional latest');example='api evidence.view name=experiment-a';description='Read evidence manifest'}
 'evidence.load'=[ordered]@{params=@('name=NAME optional latest');example='api evidence.load name=experiment-a';description='Load captured evidence checkpoint paused'}
 'evidence.zip'=[ordered]@{params=@('name=NAME|latest optional');example='api evidence.zip name=latest';description='Create ZIP for one evidence capture'}
 'frame.capture'=[ordered]@{params=@('name=NAME optional');example='api frame.capture name=control';description='Capture current SDL frame PNG'}
 'image.compare'=[ordered]@{params=@('a=NAME required','b=NAME required','region=NAME optional default FULL','hud=normal|hidden|only optional default normal');example='api image.compare a=control b=test region=PLAYER_CAR hud=hidden';description='Compare two captures/snapshots and return pixel-difference metrics; HUD modes require layered snapshots'}
 'image.diff'=[ordered]@{params=@('a=NAME required','b=NAME required','region=NAME optional default FULL','output=NAME optional default image-diff','hud=normal|hidden|only optional default normal');example='api image.diff a=control b=test region=FULL hud=hidden output=diff';description='Compare two images and save grayscale diff PNG; HUD modes require layered snapshots'}
 'image.regions'=[ordered]@{params=@();example='api image.regions';description='Return raw named image-region document'}
 'gameplay.registry'=[ordered]@{params=@();example='api gameplay.registry';description='Return live evidence-backed gameplay registry'}
 'track.state'=[ordered]@{params=@();example='api track.state';description='Return current authoritative track/road state'}
 'track.record.start'=[ordered]@{params=@();example='api track.record.start';description='Enable the shared server-side Track View recorder; UI and scripts use the same recorder state'}
 'track.record.stop'=[ordered]@{params=@();example='api track.record.stop';description='Disable automatic/UI track recording while retaining collected samples'}
 'track.record.clear'=[ordered]@{params=@();example='api track.record.clear';description='Clear all shared track recorder samples'}
 'track.record.status'=[ordered]@{params=@();example='api track.record.status';description='Return recorder enabled state, sample/segment counts, frame range and spatial bounds'}
 'track.record.sample'=[ordered]@{params=@();example='api track.record.sample';description='Append one deterministic sample of authoritative track.state to the shared recorder'}
 'track.svg.export'=[ordered]@{params=@('name=NAME optional');example='api track.svg.export name=stage1-track-survey-3600';description='Export road edges, centreline and player trajectory as SVG under evidence/track-exports; refuses empty recordings'}
 'track.map.export'=[ordered]@{params=@('name=NAME optional');example='api track.map.export name=stage1-course-map';description='Export the shared v2 recorder as a reconstructable course-mapping dataset: JSON, track samples CSV, player trajectory CSV, dynamic-object/target evidence CSV, raw surface-signature CSV and SVG'}
 'course.follow.status'=[ordered]@{params=@();example='api course.follow.status';description='Read the live autonomous course follower configuration and current closed-loop telemetry'}
 'course.follow.configure'=[ordered]@{params=@('controller=legacy|predictive|hybrid|profile optional','steer=1..96 optional','deadzone=0..127 optional','lookahead=0..8 optional','lateralKp=0..0.05 optional','lateralKd=0..0.1 optional','lateralMax=1..96 optional','lateralDeadzone=0..4096 optional','bias=-8192..8192 optional','slew=1..96 optional','speedControl=true|false optional');example='api course.follow.configure controller=hybrid lookahead=8 bias=0 speedControl=true';description='Configure the existing native course follower at runtime without restarting the emulator'}
 'course.follow.start'=[ordered]@{params=@();example='api course.follow.start';description='Enable the native closed-loop course follower live'}
 'course.follow.stop'=[ordered]@{params=@();example='api course.follow.stop';description='Disable the native course follower and centre debugger steering'}
 'course.follow.reset'=[ordered]@{params=@();example='api course.follow.reset';description='Reset course-follower controller integrator/history/output while preserving configuration'}
 'course.survey.status'=[ordered]@{params=@();example='api course.survey.status';description='Read Workbench live course-survey orchestration and recorder state'}
 'course.survey.start'=[ordered]@{params=@('collisionResponse=true|false optional default false','timerHold=true|false optional default true');example='api course.survey.start collisionResponse=false timerHold=true';description='Start a clean live survey: follower on, optional timer hold, optional proven collision-response suppression, recorder clear/start; no emulator restart'}
 'course.survey.stop'=[ordered]@{params=@();example='api course.survey.stop';description='Stop the live survey/follower/recorder and restore the timer/collision response state captured at survey start'}
 'next-steps'=[ordered]@{params=@();example='api next-steps';description='Return packaged high-level research next steps'}
 'experiment.begin'=[ordered]@{params=@('name=NAME optional');example='api experiment.begin name=player-car-ab';description='Start a grouped research experiment/evidence manifest'}
 'experiment.capture'=[ordered]@{params=@('label=NAME optional','include=sprites,regions,gameplay,track optional');example='api experiment.capture label=control';description='Capture screenshot plus structured state into the active experiment'}
 'experiment.status'=[ordered]@{params=@();example='api experiment.status';description='Return the active experiment manifest or inactive state'}
 'experiment.end'=[ordered]@{params=@();example='api experiment.end';description='Finalize and close the active experiment manifest'}
 'platform.info'=[ordered]@{params=@();example='api platform.info';description='Return build, script engine, active profile and research-platform identity'}
 'features.list'=[ordered]@{params=@();example='api features.list';description='Return machine-readable feature/regression manifest'}
 'script.functions'=[ordered]@{params=@();example='api script.functions';description='List supported Script Console query/collection functions'}
 'registry.list'=[ordered]@{params=@();example='api registry.list';description='Return gameplay registry items as a flat structured list'}
 'track.records'=[ordered]@{params=@('before=0..64 optional default 4','after=0..64 optional default 12');example='api track.records before=2 after=8';description='Read track records around the current authoritative record'}
 'objects.list'=[ordered]@{params=@();example='api objects.list';description='Return semantic-object registry candidates and evidence confidence'}
 'audio.capabilities'=[ordered]@{params=@();example='api audio.capabilities';description='Describe planned/current audio research capabilities without mutating audio runtime'}
 'audio.channels'=[ordered]@{params=@();example='api audio.channels';description='Return audio channel/source descriptors from the audio research profile'}
 'audio.events'=[ordered]@{params=@();example='api audio.events';description='Return current audio-event foundation status; runtime capture is intentionally not enabled yet'}
 'audio.waveform.plan'=[ordered]@{params=@();example='api audio.waveform.plan';description='Return waveform-viewer/capture implementation contract for future validated audio tooling'}
 'audio.export.plan'=[ordered]@{params=@();example='api audio.export.plan';description='Return planned sample/music export contract and evidence formats'}
 'audio.inject.plan'=[ordered]@{params=@();example='api audio.inject.plan';description='Return guarded runtime audio-injection design; does not inject audio'}
 'evidence.names'=[ordered]@{params=@();example='api evidence.names';description='Return structured evidence-session directory list'}
 'docs.index'=[ordered]@{params=@();example='api docs.index';description='Return the machine-readable documentation/knowledge index'}
 'docs.search'=[ordered]@{params=@('query=TEXT optional','category=TEXT optional','status=TEXT optional');example='api docs.search query=HUD';description='Search documentation/knowledge entries'}
 'docs.get'=[ordered]@{params=@('id=ENTRY required');example='api docs.get id=hud-isolation';description='Return one documentation/knowledge entry with evidence and cross-links'}
 'docs.glossary'=[ordered]@{params=@();example='api docs.glossary';description='Return glossary entries from the authoritative machine-readable knowledge source'}
 'docs.handbook'=[ordered]@{params=@();example='api docs.handbook';description='Return the complete user-facing handbook navigation and authored guide content'}
 'docs.page'=[ordered]@{params=@('id=PAGE required');example='api docs.page id=graphics-lab-guide';description='Return one authored handbook page and its parent section'}
 'docs.handover'=[ordered]@{params=@();example='api docs.handover';description='Return the authoritative ChatGPT continuation handover prompt for this build'}
 'docs.export'=[ordered]@{params=@('format=html|pdf optional default html','id=ENTRY optional','query=TEXT optional','evidence=none|key|full optional default key','output=basename optional');example='api docs.export format=html evidence=key output=project-handbook';description='Export documentation from the same knowledge source; PDF uses Microsoft Edge headless printing when available'}
 'knowledge.list'=[ordered]@{params=@();example='api knowledge.list';description='Alias returning all knowledge entries'}
 'knowledge.get'=[ordered]@{params=@('id=ENTRY required');example='api knowledge.get id=layered-frame-forensics';description='Return one knowledge entry'}
 'knowledge.search'=[ordered]@{params=@('query=TEXT optional','category=TEXT optional','status=TEXT optional');example='api knowledge.search query=brake';description='Search machine-readable research knowledge'}
 'knowledge.related'=[ordered]@{params=@('id=ENTRY required');example='api knowledge.related id=hud-isolation';description='Return entries explicitly related to one knowledge object'}
 'timeline.record.start'=[ordered]@{params=@('name=NAME optional');example='api timeline.record.start name=turbo-complete-cycle';description='Begin a Forensic Timeline v1 recording in the current run artifacts; current frame is captured immediately and every subsequent emulated frame is checkpointed with full bus-write provenance'}
 'timeline.record.stop'=[ordered]@{params=@();example='api timeline.record.stop';description='Stop and finalize the active forensic timeline recording'}
 'timeline.status'=[ordered]@{params=@();example='api timeline.status';description='Read native timeline recording/import state'}
 'timeline.list'=[ordered]@{params=@();example='api timeline.list';description='List discoverable forensic timelines from active/completed runs and packaged research timelines'}
 'timeline.load'=[ordered]@{params=@('name=NAME or path=PATH required');example='api timeline.load name=turbo-complete-cycle';description='Import a forensic timeline and load its first recorded frame into the paused emulator inspection surface'}
 'timeline.seek'=[ordered]@{params=@('frame=N required');example='api timeline.seek frame=2064';description='Seek an imported timeline to an exact recorded frame; ordinary memory/register/sprite/palette/gameplay APIs then inspect that recorded state'}
 'timeline.next'=[ordered]@{params=@();example='api timeline.next';description='Seek to the next recorded timeline frame'}
 'timeline.prev'=[ordered]@{params=@();example='api timeline.prev';description='Seek to the previous recorded timeline frame'}
 'timeline.frames'=[ordered]@{params=@();example='api timeline.frames';description='Return recorded frame numbers for the imported timeline'}
 'timeline.play'=[ordered]@{params=@('name=NAME or path=PATH optional when a timeline is already loaded','from=N optional','to=N optional','fps=1..60 optional default 30');example='api timeline.play name=turbo-complete-cycle from=2064 to=2276 fps=30';description='Paced SDL playback of recorded historical frame checkpoints; this is visual state playback and does not re-execute CPUs'}
 'timeline.trim.event'=[ordered]@{params=@('name=NAME or path=PATH required','cpu=A|B required','address=HEX required','width=8|16|32 required','mask=HEX required','pre=N optional default 1','post=N optional default 1');example='api timeline.trim.event name=turbo-complete-cycle cpu=A address=0x10040F width=8 mask=02 pre=1 post=1';description='Find a masked rising/falling event window in one streaming pass and write a non-destructive analysis/event-window.json view; the authoritative recording is never trimmed or rewritten'}
 'timeline.scan.memory'=[ordered]@{params=@('name=NAME or path=PATH required','triggerFrame=N optional (trigger=N alias accepted)','stopFrame=N optional (stop=N alias accepted)','window=N optional default 1');example='api timeline.scan.memory name=turbo-complete-cycle triggerFrame=2065 stopFrame=2275 window=1';description='Streaming offline analysis of the recorded bus-write stream; writes portable ranked CSV/JSON/HTML into the current run artifacts while leaving the source timeline immutable'}
 'timeline.analyze.auto'=[ordered]@{params=@('name=NAME or path=PATH required','triggerFrame=N required (trigger=N alias accepted)','stopFrame=N required (stop=N alias accepted)','window=N optional default 1','top=N optional default 20');example='api timeline.analyze.auto name=turbo-complete-cycle triggerFrame=2065 stopFrame=2275 window=1 top=20';description='Automated portable forensic pipeline: temporal ranking, writer-PC disassembly, milestone state/layered snapshots and HUD_TURBO graphics correlation'}
 'timeline.fork-live'=[ordered]@{params=@();example='api timeline.fork-live';description='Leave timeline inspection mode while keeping the selected checkpoint loaded, creating a paused live counterfactual fork'}
 'timeline.unload'=[ordered]@{params=@();example='api timeline.unload';description='Leave timeline inspection mode and restore the live state that existed before timeline.load'}
 'timeline.reset'=[ordered]@{params=@();example='api timeline.reset';description='Idempotent cleanup for script-initiated timeline work: restore the live state captured before timeline.load even after timeline.fork-live; succeed unchanged when no timeline transaction is active'}
 'schema'=[ordered]@{params=@('action=ACTION optional');example='api schema action=memory.export';description='Describe scriptable API actions and parameter contracts'}
}
foreach($n in $scriptApiActions){if(-not $scriptActionSchemas.Contains($n)){throw "Missing schema contract for script API action: $n"}};foreach($n in $scriptActionSchemas.Keys){if($scriptApiActions -notcontains $n){throw "Schema action is not exposed through Script Console allow-list: $n"}}
function Get-ScriptActionHelp([string]$action){$k=([string]$action).ToLowerInvariant();if(-not $scriptActionSchemas.Contains($k)){throw "Unknown API action: $k"};return ([ordered]@{action=$k;contract=$scriptActionSchemas[$k]}|ConvertTo-Json -Depth 6)}
function Invoke-ScriptApiAction([string]$action,[hashtable]$a,[switch]$ValidateOnly){
    $action=$action.ToLowerInvariant();if($scriptApiActions -notcontains $action){throw "Unknown API script action: $action"};if($ValidateOnly){return "VALID api $action"}
    switch($action){
        'game.experiment.status'{return ((Get-GameLabState)|ConvertTo-Json -Depth 6 -Compress)}
        'game.target.one-hit'{$en=Bool-Arg $a 'enabled' $true;$r=Invoke-Chq @('target','one-hit',$(if($en){'on'}else{'off'}));return "$r state=$(((Get-GameLabState)|ConvertTo-Json -Compress))"}
        'game.turbo.stock'{$en=Bool-Arg $a 'enabled' $true;$was=Test-GameLabWasRunning;if($en){$v=if($a.ContainsKey('value')){[uint32]$a['value']}else{3};if($v -gt 65535){throw 'value must be 0..65535'};$r=Set-GameLabFreeze 'turbo-stock' '1003A2' 16 $v}else{$r=Remove-GameLabPatch 'turbo-stock'};Restore-GameLabRunState $was;return "$r preservedRun=$was state=$(((Get-GameLabState)|ConvertTo-Json -Compress))"}
        'game.turbo.active'{$en=Bool-Arg $a 'enabled' $true;$was=Test-GameLabWasRunning;if($en){$r=Set-GameLabFreeze 'turbo-active' '100414' 16 0}else{$r=Remove-GameLabPatch 'turbo-active'};Restore-GameLabRunState $was;return "$r preservedRun=$was state=$(((Get-GameLabState)|ConvertTo-Json -Compress))"}
        'game.turbo.fire'{$f=if($a.ContainsKey('frames')){[int]$a['frames']}else{3};if($f -lt 1 -or $f -gt 600){throw 'frames must be 1..600'};return Invoke-Chq @('input','pulse','3','01',[string]$f)}
        'game.turbo.timer.reset'{$was=Test-GameLabWasRunning;$r=Invoke-Chq @('write','A','100414','16','0000');Restore-GameLabRunState $was;return "$r preservedRun=$was state=$(((Get-GameLabState)|ConvertTo-Json -Compress))"}
        'game.turbo.remaining.set'{$v=[uint32]$a['value'];if($v -gt 65535){throw 'value must be 0..65535'};$was=Test-GameLabWasRunning;$r=Invoke-Chq @('write','A','1003A2','16',('{0:X}'-f$v));Restore-GameLabRunState $was;return "$r preservedRun=$was state=$(((Get-GameLabState)|ConvertTo-Json -Compress))"}
        'game.speed.set'{$v=[uint32]$a['value'];if($v -gt 65535){throw 'value must be 0..65535'};$was=Test-GameLabWasRunning;$r=Invoke-Chq @('write','A','10041C','16',('{0:X}'-f$v));Restore-GameLabRunState $was;return "$r preservedRun=$was state=$(((Get-GameLabState)|ConvertTo-Json -Compress))"}
        'game.speed.freeze'{$en=Bool-Arg $a 'enabled' $true;$was=Test-GameLabWasRunning;if($en){$v=if($a.ContainsKey('value')){[uint32]$a['value']}else{Read-ChqValue 'A' '10041C' 16};if($v -gt 65535){throw 'value must be 0..65535'};$r=Set-GameLabFreeze 'speed' '10041C' 16 $v}else{$r=Remove-GameLabPatch 'speed'};Restore-GameLabRunState $was;return "$r preservedRun=$was state=$(((Get-GameLabState)|ConvertTo-Json -Compress))"}
        'game.speed.clear'{$r=Remove-GameLabPatch 'speed';return "$r state=$(((Get-GameLabState)|ConvertTo-Json -Compress))"}
        'game.sprite.order.inspect'{return Invoke-Chq @('sprites','order')}
        'game.sprite.order.set'{$mode=[string]$a['mode'];if($mode-notin 'higher-slot','lower-slot'){throw 'mode must be higher-slot or lower-slot'};$r=Invoke-Chq @('sprites','tie-break',$mode);return "$r state=$(((Get-GameLabState)|ConvertTo-Json -Compress))"}
        'tile.inspect'{$pal=if($a.ContainsKey('palette')){[int]$a['palette']}else{0};$scale=if($a.ContainsKey('scale')){[int]$a['scale']}else{8};$m=Get-Tc0100TextCharacter ([string]$a['code']) $pal $scale;return ($m|ConvertTo-Json -Depth 10 -Compress)}
        'status'{return Invoke-Chq @('status')}
        'capabilities'{return Invoke-Chq @('capabilities')}
        'input.ports'{return Invoke-Chq @('input','ports')}
        'events.tail'{$n=if($a.ContainsKey('count')){[int]$a['count']}else{100};return Invoke-Chq @('events','tail',[string]$n)}
        'sprites.tail'{$n=if($a.ContainsKey('count')){[int]$a['count']}else{100};return Invoke-Chq @('sprites','tail',[string]$n,'semantic')}
        'memory.read'{$cpu=[string]$a['cpu'];$address=[string]$a['address'];$width=if($a.ContainsKey('width')){[string]$a['width']}else{'16'};return Invoke-Chq @('read',$cpu,$address,$width)}
        'memory.read-range'{$cpu=([string]$a['cpu']).ToUpperInvariant();if($cpu -notin 'A','B'){throw 'cpu must be A or B'};$aa=Hex-U32 ([string]$a['address']);$len=Count-U32 ([string]$a['length']);if($len -lt 1 -or $len -gt 65536){throw 'length must be 1..65536'};return Invoke-Chq @('readrange',$cpu,('{0:X}' -f $aa),[string]$len)}
        'memory.write'{$cpu=([string]$a['cpu']).ToUpperInvariant();$addr=[string]$a['address'];$width=if($a.ContainsKey('width')){[string]$a['width']}else{'16'};$value=[string]$a['value'];if($cpu -notin 'A','B' -or $width -notin '8','16','32'){throw 'invalid cpu/width'};$old=Read-ChqValue $cpu $addr ([int]$width);if($a.ContainsKey('expect')){$expect=Hex-U32 ([string]$a['expect']);if($old-ne$expect){throw ('expect mismatch at 0x{0}: expected 0x{1:X}, actual 0x{2:X}' -f $addr,$expect,$old)}};$r=Invoke-Chq @('write',$cpu,$addr,$width,$value);return "$r old=0x$('{0:X}' -f $old) new=0x$('{0:X}' -f (Read-ChqValue $cpu $addr ([int]$width)))"}
        'memory.trace.start'{$cpu=([string]$a['cpu']).ToUpperInvariant();if($cpu -notin 'A','B'){throw 'cpu must be A or B'};$addr=[string]$a['address'];$len=if($a.ContainsKey('length')){Count-U32 ([string]$a['length'])}else{1};if($len -lt 1 -or $len -gt 16777216){throw 'length must be 1..16777216'};$width=if($a.ContainsKey('width')){([string]$a['width']).ToLowerInvariant()}else{'any'};if($width -notin '8','16','32','any'){throw 'width must be 8, 16, 32 or any'};$limit=if($a.ContainsKey('limit')){[int]$a['limit']}else{4096};if($limit -lt 64 -or $limit -gt 1000000){throw 'limit must be 64..1000000'};return Invoke-Chq @('memory','trace','start',$cpu,$addr,[string]$len,$width,[string]$limit)}
        'memory.trace.status'{return Invoke-Chq @('memory','trace','status')}
        'memory.trace.tail'{$n=if($a.ContainsKey('count')){[int]$a['count']}else{20};return Invoke-Chq @('memory','trace','tail',[string]$n)}
        'memory.trace.stop'{return Invoke-Chq @('memory','trace','stop')}
        'memory.trace.clear'{return Invoke-Chq @('memory','trace','clear')}
        'register.read'{$cpu=([string]$a['cpu']).ToUpperInvariant();if($cpu -notin 'A','B'){throw 'cpu must be A or B'};if($a.ContainsKey('name')){return Invoke-Chq @('regs',$cpu,[string]$a['name'])};return Invoke-Chq @('regs',$cpu)}
        'register.write'{$cpu=([string]$a['cpu']).ToUpperInvariant();$name=[string]$a['name'];$value=[string]$a['value'];if($cpu -notin 'A','B'){throw 'cpu must be A or B'};return Invoke-Chq @('reg','write',$cpu,$name,$value)}
        'patch.freeze'{$cpu=([string]$a['cpu']).ToUpperInvariant();$addr=[string]$a['address'];$width=[string]$a['width'];$value=[string]$a['value'];$old=Read-ChqValue $cpu $addr ([int]$width);if($a.ContainsKey('expect')){$expect=Hex-U32 ([string]$a['expect']);if($old-ne$expect){throw ('expect mismatch: expected 0x{0:X}, actual 0x{1:X}' -f $expect,$old)}};$r=Invoke-Chq @('patch','freeze',$cpu,$addr,$width,$value);return "$r old=0x$('{0:X}' -f $old) new=0x$('{0:X}' -f (Read-ChqValue $cpu $addr ([int]$width)))"}
        'patch.replace'{return Invoke-Chq @('patch','replace',([string]$a['cpu']).ToUpperInvariant(),[string]$a['address'],[string]$a['width'],[string]$a['value'])}
        'patch.suppress'{return Invoke-Chq @('patch','suppress',([string]$a['cpu']).ToUpperInvariant(),[string]$a['address'],[string]$a['width'])}
        'patch.list'{return Invoke-Chq @('patch','list')}
        'patch.remove'{return Invoke-Chq @('patch','remove',[string]$a['id'])}
        'patch.clear'{return Invoke-Chq @('patch','clear')}
        'cpu.disassemble'{$cpu=if($a.ContainsKey('cpu')){[string]$a['cpu']}else{'A'};$address=[string]$a['address'];$count=if($a.ContainsKey('count')){[int]$a['count']}else{16};return Invoke-Chq @('disasm',$cpu,$address,[string]$count)}
        'control.pause'{return Invoke-Chq @('pause')}
        'control.resume'{return Invoke-Chq @('resume')}
        'control.step-frame'{$n=if($a.ContainsKey('frames')){[int]$a['frames']}else{1};return Invoke-Chq @('step','frame',[string]$n)}
        'control.run-frames'{$n=if($a.ContainsKey('frames')){[int]$a['frames']}else{1};if($n -lt 1 -or $n -gt 100000){throw 'frames out of range'};$timeout=if($a.ContainsKey('timeout')){[int]$a['timeout']}else{900000};if($timeout -lt 1000 -or $timeout -gt 3600000){throw 'timeout must be 1000..3600000 ms'};$null=Invoke-Chq @('pause');$st=Parse-Kv (Invoke-Chq @('status'));$start=[int]$st.frame;$null=Invoke-Chq @('run',[string]$n);$done=Wait-ChqPaused ($start+$n) $timeout 15000;return "OK start=$start frame=$($done.frame) frames=$n paused=$($done.paused) timeout_ms=$timeout"}
        'control.abort'{return Invoke-Chq @('pause')}
        'control.step-instruction'{$cpu=if($a.ContainsKey('cpu')){[string]$a['cpu']}else{'A'};return Invoke-Chq @('step','instr',$cpu)}
        'timer.hold'{$on=Bool-Arg $a 'enabled' $true;return Invoke-Chq @('timer',$(if($on){'freeze'}else{'resume'}))}
        'window.always-on-top'{$on=Bool-Arg $a 'enabled' $true;return Invoke-Chq @('window','top',$(if($on){'on'}else{'off'}))}
        'layer.offsets.get'{return Invoke-Chq @('layer-offset','get')}
        'layer.offsets.set'{return Invoke-Chq @('layer-offset','set',[string]$a['layer'],[string][int]$a['x'],[string][int]$a['y'])}
        'layer.offsets.reset'{$layer=if($a.ContainsKey('layer')){[string]$a['layer']}else{'all'};return Invoke-Chq @('layer-offset','reset',$layer)}
        'tc0100.mode'{if($a.ContainsKey('mode')){return Invoke-Chq @('tc0100-mode',[string]$a['mode'])};return Invoke-Chq @('tc0100-mode')}
        'frame.snapshot.assert-scene-no-sprites'{$dir=Resolve-StructuredFrameSnapshotDir ([string]$a['name']);$m=Get-Content (Join-Path $dir 'reconstruction.json') -Raw|ConvertFrom-Json;if(-not $m.reconstruction.exact -or -not $m.reconstruction.verifiedExact -or [int]$m.reconstruction.mismatchedPixels -ne 0){throw 'Snapshot reconstruction is not exact'};if([int]$m.sceneNoSprites.spriteOwnedPixels -le 0 -or [int]$m.sceneNoSprites.changedPixels -le 0){throw 'Scene-no-sprites regression: absent sprite ownership or identical final'};if((Get-FileHash (Join-Path $dir 'final.png')).Hash -eq (Get-FileHash (Join-Path $dir 'scene-no-sprites.png')).Hash){throw 'Scene-no-sprites image is identical to final'};return ($m.sceneNoSprites|ConvertTo-Json -Compress)}
        'layer.offsets.assert'{$st=Parse-Kv (Invoke-Chq @('layer-offset','get'));$layer=[string]$a['layer'];if([int](Prop $st ($layer+'_x') 999999) -ne [int]$a['x'] -or [int](Prop $st ($layer+'_y') 999999) -ne [int]$a['y']){throw 'Live layer offset assertion failed'};return 'PASS live layer offsets'}
        'release.assert-version'{$st=Parse-Kv (Invoke-Chq @('version'));$features=Get-FeatureManifest;if([string]$st.build -ne $workbenchVersion -or [string]$features.build -ne $workbenchVersion){throw 'Full release version mismatch'};return ('PASS build='+$workbenchVersion)}
        'frame.snapshot.assert-exact'{$dir=Resolve-StructuredFrameSnapshotDir ([string]$a['name']);$m=Get-Content (Join-Path $dir 'reconstruction.json') -Raw|ConvertFrom-Json;if(-not $m.reconstruction.exact -or -not $m.reconstruction.verifiedExact -or [int]$m.reconstruction.mismatchedPixels -ne 0){throw 'Snapshot reconstruction is not exact'};if([string]$m.build -ne $workbenchVersion){throw 'Snapshot release version mismatch'};if((Bool-Arg $a 'requireColumnZero' $false) -and [int]$m.columnScroll.nonzeroWords -ne 0){throw 'Expected zero TC0100SCN column-scroll table'};return 'PASS exact reconstruction'}
        'window.status'{return Invoke-Chq @('window','status')}
        'window.fullscreen'{$on=Bool-Arg $a 'enabled' $true;return Invoke-Chq @('window','fullscreen',$(if($on){'on'}else{'off'}))}
        'window.scale'{$n=[int]$a['scale'];if($n -lt 1 -or $n -gt 8){throw 'scale must be 1..8'};return Invoke-Chq @('window','scale',[string]$n)}
        'window.show'{return Invoke-Chq @('window','show')}
        'window.hide'{return Invoke-Chq @('window','hide')}
        'window.minimize'{return Invoke-Chq @('window','minimize')}
        'window.restore'{return Invoke-Chq @('window','restore')}
        'tuning.handling.get'{return Invoke-Chq @('handling','get')}
        'tuning.handling.set'{$c=if($a.ContainsKey('cornering')){[string]$a['cornering']}else{'1'};$sr=if($a.ContainsKey('speedRetain')){[string]$a['speedRetain']}else{'1'};return Invoke-Chq @('handling','set',$c,$sr)}
        'tuning.handling.reset'{return Invoke-Chq @('handling','reset')}
        'collision.state'{return Invoke-Chq @('collision','state')}
        'collision.response.get'{return Invoke-Chq @('collision','response','get')}
        'collision.response.set'{$en=if(Bool-Arg $a 'enabled' $true){'1'}else{'0'};return Invoke-Chq @('collision','response','set',$en)}
        'collision.response.reset'{return Invoke-Chq @('collision','response','reset')}
        'input.steering.get'{return Invoke-Chq @('input','steering','get')}
        'input.steering.set'{$v=[string]$a['value'];return Invoke-Chq @('input','steering','set',$v)}
        'palette.bank'{$bank=[string]$a['bank'];return Invoke-Chq @('palette','bank',$bank)}
        'palette.entry.set'{$idx=[string]$a['index'];$raw=[string]$a['raw'];return Invoke-Chq @('palette','freeze',$idx,$raw)}
        'palette.entry.restore'{$idx=[string]$a['index'];return Invoke-Chq @('palette','restore',$idx)}
        'palette.overrides.clear'{return Invoke-Chq @('palette','clear')}
        'palette.trace.start'{$indices=[string]$a['indices'];$limit=if($a.ContainsKey('limit')){[int]$a['limit']}else{4096};return Invoke-Chq @('palette','trace','start',$indices,[string]$limit)}
        'palette.trace.status'{return Invoke-Chq @('palette','trace','status')}
        'palette.trace.tail'{$n=if($a.ContainsKey('count')){[int]$a['count']}else{20};return Invoke-Chq @('palette','trace','tail',[string]$n)}
        'palette.trace.stop'{return Invoke-Chq @('palette','trace','stop')}
        'palette.trace.clear'{return Invoke-Chq @('palette','trace','clear')}
        'input.ioc.xor'{$p=[int]$a['port'];$m=[string]$a['mask'];return Invoke-Chq @('input','xor','set',('{0:X}' -f $p),$m)}
        'input.ioc.clear'{return Invoke-Chq @('input','xor','clear')}
        'input.ioc.pulse'{$p=[int]$a['port'];$m=[string]$a['mask'];$f=if($a.ContainsKey('frames')){[int]$a['frames']}else{3};return Invoke-Chq @('input','pulse',('{0:X}' -f $p),$m,[string]$f)}
        'input.ioc.sweep'{$file=if($a.ContainsKey('checkpoint')){[string]$a['checkpoint']}else{'stage1-gameplay-2064.chqstate'};$p=if($a.ContainsKey('port')){[int]$a['port']}else{3};$masks=if($a.ContainsKey('masks')){@([string]$a['masks'] -split ',')}else{@('01','02','04','08','10','20','40','80')};$pf=if($a.ContainsKey('pulseFrames')){[int]$a['pulseFrames']}else{8};$of=if($a.ContainsKey('observeFrames')){[int]$a['observeFrames']}else{90};$r=Invoke-IocSweepCore $file $p $masks $pf $of;return ($r|ConvertTo-Json -Depth 8 -Compress)}
        'checkpoint.list'{return ((Get-CheckpointCatalog)|ConvertTo-Json -Depth 6 -Compress)}
        'checkpoint.load'{$file=[string]$a['file'];$full=Resolve-CheckpointFile $file;$r=Invoke-Chq @('checkpoint','load',$full);$script:currentCheckpointFile=$file;if(Bool-Arg $a 'run' $false){$r += ' '+(Invoke-Chq @('resume'))};return $r}
        'checkpoint.save'{$file=[string]$a['file'];$full=Resolve-CheckpointSaveFile $file;$r=Invoke-Chq @('checkpoint','save',$full);$leaf=[IO.Path]::GetFileName($full);$meta=[ordered]@{schema='chq-checkpoint-v1';name=$(if($a.ContainsKey('name')){[string]$a['name']}else{[IO.Path]::GetFileNameWithoutExtension($leaf)});role=$(if($a.ContainsKey('role')){[string]$a['role']}else{'research'});description=$(if($a.ContainsKey('description')){[string]$a['description']}else{'Saved from v0.66.9.0 Research Workbench'});frame=(Prop (Parse-Kv (Invoke-Chq @('status'))) 'frame' $null);created=(Get-Date).ToString('o')};$meta|ConvertTo-Json -Depth 6|Set-Content ([IO.Path]::ChangeExtension($full,'.json')) -Encoding UTF8;return "$r metadata=$([IO.Path]::ChangeExtension($full,'.json'))"}
        'logs.list'{return ((Get-SessionLogFiles)|ConvertTo-Json -Compress)}
        'logs.tail'{$file=[string]$a['file'];$lines=if($a.ContainsKey('lines')){[int]$a['lines']}else{80};$safe=$file.Replace('\','/');if($safe -match '(^|/)\.\.(/|$)'){throw 'Invalid log path'};if($safe -eq '@runtime/web-stdout.log'){$full=Join-Path (Join-Path $root '.chq') 'web-stdout.log'}elseif($safe -eq '@runtime/web-stderr.log'){$full=Join-Path (Join-Path $root '.chq') 'web-stderr.log'}elseif($safe -eq '@runtime/web-listener.log'){$full=$runtimeListenerLog}elseif($safe -eq '@runtime/web-command-diagnostics.log'){$full=$runtimeCommandDiag}elseif($safe -eq '@runtime/web-frontend-errors.log'){$full=$runtimeFrontendDiag}else{if(-not $sessionPath){throw 'No active session path'};$full=[IO.Path]::GetFullPath((Join-Path $sessionPath $safe.Replace('/','\')));$base=[IO.Path]::GetFullPath($sessionPath)+[IO.Path]::DirectorySeparatorChar;if(-not $full.StartsWith($base,[StringComparison]::OrdinalIgnoreCase)){throw 'Log path escapes session'}};return (@(Get-Content $full -Tail $lines -ErrorAction Stop)-join "`n")}
        'evidence.capture'{$name=if($a.ContainsKey('name')){[string]$a['name']}else{'evidence-'+(Get-Date -Format 'yyyyMMdd-HHmmss')};return ((New-EvidenceCapture $name)|ConvertTo-Json -Depth 10 -Compress)}
        'evidence.list'{$items=@();foreach($d in Get-ChildItem $evidencePath -Directory -ErrorAction SilentlyContinue|Sort-Object LastWriteTime -Descending){$items += $d.Name};return ($items|ConvertTo-Json -Compress)}
        'evidence.view'{$name=if($a.ContainsKey('name')){[string]$a['name']}else{(Latest-EvidenceDir 'evidence').Name};$safe=SafeName $name;$mp=Join-Path (Join-Path $evidencePath $safe) 'manifest.json';if(-not(Test-Path $mp)){throw 'Evidence manifest not found'};return Get-Content $mp -Raw}
        'evidence.load'{$name=if($a.ContainsKey('name')){[string]$a['name']}else{(Latest-EvidenceDir 'evidence').Name};$safe=SafeName $name;$file=Join-Path (Join-Path $evidencePath $safe) 'state.chqstate';return Invoke-Chq @('checkpoint','load',$file)}
        'evidence.zip'{$name=if($a.ContainsKey('name')){[string]$a['name']}else{'latest'};return Zip-ResearchOutput 'evidence' $name}
        'frame.capture'{$base=if($a.ContainsKey('name')){SafeName ([string]$a['name'])}else{'frame-'+(Get-Date -Format 'yyyyMMdd-HHmmss-fff')};$name=$base;$i=1;$path=Join-Path $evidencePath ($name+'.png');while(Test-Path $path){$name=($base+'-'+$i);$i++;$path=Join-Path $evidencePath ($name+'.png')};return Invoke-Chq @('screenshot',$path)}
        'frame.snapshot'{$name=if($a.ContainsKey('name')){[string]$a['name']}else{''};return ((New-StructuredFrameSnapshot $name)|ConvertTo-Json -Depth 8 -Compress)}
        'frame.snapshot.assert-overlap'{return ((Assert-SpriteSnapshotOverlap -Snapshot (Resolve-StructuredFrameSnapshotDir ([string]$a['name'])) -Front ([int]$a['front']) -Back ([int]$a['back']) -FrontSolo (Resolve-StructuredFrameSnapshotDir ([string]$a['frontSolo'])) -BackSolo (Resolve-StructuredFrameSnapshotDir ([string]$a['backSolo'])))|ConvertTo-Json -Depth 5 -Compress)}
        'frame.snapshot.list'{$items=@(Get-StructuredFrameSnapshots);if($a.ContainsKey('require')){$required=SafeName ([string]$a['require']);if(@($items|Where-Object{[string]$_.name -eq $required}).Count -ne 1){throw "Required frame snapshot not listed: $required"}};return ($items|ConvertTo-Json -Depth 6 -Compress)}
        'image.compare'{$aa=[string]$a['a'];$bb=[string]$a['b'];$region=if($a.ContainsKey('region')){[string]$a['region']}else{'FULL'};$hud=if($a.ContainsKey('hud')){[string]$a['hud']}else{'normal'};return ((Compare-ResearchImages $aa $bb $region '' $hud)|ConvertTo-Json -Depth 6 -Compress)}
        'image.diff'{$aa=[string]$a['a'];$bb=[string]$a['b'];$region=if($a.ContainsKey('region')){[string]$a['region']}else{'FULL'};$out=if($a.ContainsKey('output')){[string]$a['output']}else{'image-diff'};$hud=if($a.ContainsKey('hud')){[string]$a['hud']}else{'normal'};return ((Compare-ResearchImages $aa $bb $region $out $hud)|ConvertTo-Json -Depth 6 -Compress)}
        'image.regions'{return (Get-Content $imageRegionsPath -Raw)}
        'regions.list'{return ((Get-ResearchRegions)|ConvertTo-Json -Depth 6 -Compress)}
        'profiles.list'{return ((Get-GameProfiles)|ConvertTo-Json -Depth 8 -Compress)}
        'profile.get'{$id=if($a.ContainsKey('id')){[string]$a['id']}else{'chasehq'};return ((Get-GameProfile $id)|ConvertTo-Json -Depth 10 -Compress)}
        'sprites.list'{return ((Get-LiveSprites)|ConvertTo-Json -Depth 6 -Compress)}
        'sprites.maps'{return (@(Get-LiveSprites|ForEach-Object{$_.map}|Sort-Object -Unique)|ConvertTo-Json -Compress)}
        'sprites.palettes'{return (@(Get-LiveSprites|ForEach-Object{$_.palette}|Sort-Object -Unique)|ConvertTo-Json -Compress)}
        'sprite.inspect'{$slot=[int]$a['slot'];$x=@(Get-LiveSprites|Where-Object{$_.slot -eq $slot}|Select-Object -First 1);if(-not$x){throw "Sprite slot not active: $slot"};return ($x[0]|ConvertTo-Json -Depth 5 -Compress)}
        'sprite.map.inspect'{
            if($a.ContainsKey('slot')){return Invoke-Chq @('sprites','map-inspect',[string]([int]$a['slot']))}
            if(-not $a.ContainsKey('map')){throw 'sprite.map.inspect requires slot=N or map=N'}
            $want=[int]$a['map'];$raw=Invoke-Chq @('sprites','list');$slots=@()
            foreach($line in ($raw -split "`r?`n")){$kv=Parse-Kv $line;if((Prop $kv 'map' $null)-ne$null -and [int](Prop $kv 'map' -1)-eq$want){$slots+=([int](Prop $kv 'slot' -1))}}
            if($slots.Count-eq0){throw "No active sprite uses map=$want"}
            return (($slots|ForEach-Object{Invoke-Chq @('sprites','map-inspect',[string]$_)}) -join "`n")
        }
        'sprite.order.inspect'{return Invoke-Chq @('sprites','order')}
        'sprite.order.set'{$mode=[string]$a['mode'];if($mode-notin 'higher-slot','lower-slot'){throw 'mode must be higher-slot or lower-slot'};return Invoke-Chq @('sprites','tie-break',$mode)}

        'timeline.record.start'{$name=if($a.ContainsKey('name')){SafeName ([string]$a['name'])}else{('timeline-'+(Get-Date -Format 'yyyyMMdd-HHmmss'))};$base=Join-Path $evidencePath 'timelines';New-Item -ItemType Directory -Force -Path $base|Out-Null;$dir=Join-Path $base ($name+'.chqtimeline');$i=1;while(Test-Path $dir){$dir=Join-Path $base ($name+'-'+$i+'.chqtimeline');$i++};return Invoke-Chq @('timeline','start',$dir)}
        'timeline.record.stop'{return Invoke-Chq @('timeline','stop')}
        'timeline.status'{return Invoke-Chq @('timeline','status')}
        'timeline.list'{return (@(Get-ForensicTimelines)|ConvertTo-Json -Depth 8 -Compress)}
        'timeline.load'{$src=if($a.ContainsKey('path')){[string]$a['path']}elseif($a.ContainsKey('name')){[string]$a['name']}else{throw 'name or path required'};$full=Resolve-ForensicTimeline $src;if(-not $script:TimelineResetCheckpoint){$resetPath=Join-Path $runtimeDiagDir ('timeline-reset-'+$Port+'.chqstate');$save=Invoke-Chq @('checkpoint','save',$resetPath);if(([string]$save)-match '^ERR(?:\s|$)'){throw $save};$script:TimelineResetCheckpoint=$resetPath};try{return Invoke-Chq @('timeline','load',$full)}catch{throw}}
        'timeline.seek'{return Invoke-Chq @('timeline','seek',[string]$a['frame'])}
        'timeline.next'{return Invoke-Chq @('timeline','next')}
        'timeline.prev'{return Invoke-Chq @('timeline','prev')}
        'timeline.frames'{return Invoke-Chq @('timeline','frames')}
        'timeline.play'{$src=if($a.ContainsKey('path')){[string]$a['path']}elseif($a.ContainsKey('name')){[string]$a['name']}else{''};$fps=if($a.ContainsKey('fps')){[int]$a['fps']}else{30};if($fps -lt 1 -or $fps -gt 60){throw 'fps must be 1..60'};if($src){$st=Parse-Kv(Invoke-Chq @('timeline','status'));if([int](Prop $st 'loaded' 0)-eq 1){throw 'a timeline is already loaded; unload it before timeline.play name=...'};$full=Resolve-ForensicTimeline $src;$null=Invoke-Chq @('timeline','load',$full)};$raw=[string](Invoke-Chq @('timeline','frames'));$frames=@($raw -split '\s+'|Select-Object -Skip 1|Where-Object{$_-match'^\d+$'}|ForEach-Object{[int]$_});if(-not$frames.Count){throw 'no timeline loaded'};$from=if($a.ContainsKey('from')){[int]$a['from']}else{[int]$frames[0]};$to=if($a.ContainsKey('to')){[int]$a['to']}else{[int]$frames[-1]};if($to -lt $from){throw 'to must be >= from'};$selected=@($frames|Where-Object{$_ -ge $from -and $_ -le $to});if(-not$selected.Count){throw 'requested playback range contains no recorded frames'};$delay=[Math]::Max(1,[int][Math]::Round(1000.0/$fps));foreach($f in $selected){$null=Invoke-Chq @('timeline','seek',[string]$f);Start-Sleep -Milliseconds $delay};return "OK timeline playback frames=$($selected.Count) from=$($selected[0]) to=$($selected[-1]) fps=$fps source=timeline"}
        'timeline.trim.event'{$src=if($a.ContainsKey('path')){[string]$a['path']}elseif($a.ContainsKey('name')){[string]$a['name']}else{throw 'name or path required'};$cpu=[string]$a['cpu'];$addr=[string]$a['address'];$width=[int]$a['width'];$mask=[string]$a['mask'];$pre=if($a.ContainsKey('pre')){[int]$a['pre']}else{1};$post=if($a.ContainsKey('post')){[int]$a['post']}else{1};if($width -notin 8,16,32){throw 'width must be 8, 16 or 32'};if($pre -lt 0 -or $pre -gt 120 -or $post -lt 0 -or $post -gt 120){throw 'pre/post must be 0..120'};return ((Trim-ForensicTimelineEvent $src $cpu $addr $width $mask $pre $post)|ConvertTo-Json -Depth 8 -Compress)}
        'timeline.scan.memory'{$src=if($a.ContainsKey('path')){[string]$a['path']}elseif($a.ContainsKey('name')){[string]$a['name']}else{throw 'name or path required'};$tf=if($a.ContainsKey('triggerFrame')){[int]$a['triggerFrame']}elseif($a.ContainsKey('trigger')){[int]$a['trigger']}else{-1};$sf=if($a.ContainsKey('stopFrame')){[int]$a['stopFrame']}elseif($a.ContainsKey('stop')){[int]$a['stop']}else{-1};$w=if($a.ContainsKey('window')){[int]$a['window']}else{1};if($w -lt 0 -or $w -gt 30){throw 'window must be 0..30'};return ((Analyze-ForensicTimelineWrites $src $tf $sf $w)|ConvertTo-Json -Depth 10 -Compress)}
        'timeline.analyze.auto'{$src=if($a.ContainsKey('path')){[string]$a['path']}elseif($a.ContainsKey('name')){[string]$a['name']}else{throw 'name or path required'};$tf=if($a.ContainsKey('triggerFrame')){[int]$a['triggerFrame']}elseif($a.ContainsKey('trigger')){[int]$a['trigger']}else{throw 'triggerFrame required'};$sf=if($a.ContainsKey('stopFrame')){[int]$a['stopFrame']}elseif($a.ContainsKey('stop')){[int]$a['stop']}else{throw 'stopFrame required'};$w=if($a.ContainsKey('window')){[int]$a['window']}else{1};$top=if($a.ContainsKey('top')){[int]$a['top']}else{20};return ((Invoke-TimelineAutomatedAnalysis $src $tf $sf $w $top)|ConvertTo-Json -Depth 14 -Compress)}
        'timeline.fork-live'{return Invoke-Chq @('timeline','fork')}
        'timeline.unload'{$r=Invoke-Chq @('timeline','unload');if($script:TimelineResetCheckpoint){Remove-Item $script:TimelineResetCheckpoint -Force -ErrorAction SilentlyContinue;$script:TimelineResetCheckpoint=$null};return $r}
        'timeline.reset'{$st=Parse-Kv (Invoke-Chq @('status'));$result='';if(([string](Prop $st 'timeline_loaded' '0')) -eq '1' -or ([string](Prop $st 'source' 'live')) -eq 'timeline'){$result=Invoke-Chq @('timeline','unload')}elseif($script:TimelineResetCheckpoint -and (Test-Path $script:TimelineResetCheckpoint -PathType Leaf)){$result=Invoke-Chq @('checkpoint','load',$script:TimelineResetCheckpoint)}else{$result='OK timeline_reset already_live=1'};if($script:TimelineResetCheckpoint){Remove-Item $script:TimelineResetCheckpoint -Force -ErrorAction SilentlyContinue;$script:TimelineResetCheckpoint=$null};return $result}
        'schema.all'{$all=[ordered]@{};foreach($k in $scriptApiActions){$all[$k]=$scriptActionSchemas[$k]};return ([ordered]@{script='chq-research-script-v2';build=$workbenchVersion;actions=$all}|ConvertTo-Json -Depth 12)}
        'script.schema'{return (Get-ScriptLanguageSchema|ConvertTo-Json -Depth 12)}
        'script.runs'{$limit=if($a.ContainsKey('limit')){[int]$a['limit']}else{25};$offset=if($a.ContainsKey('offset')){[int]$a['offset']}else{0};if($limit-lt 1-or$limit-gt 100){throw 'limit must be 1..100'};if($offset-lt 0){throw 'offset must be >= 0'};$all=@(Get-AuthoritativeScriptRuns);if($a.ContainsKey('status')){$sv=([string]$a['status']).ToUpperInvariant();$all=@($all|Where-Object{([string]$_.status).ToUpperInvariant()-eq$sv})};if($a.ContainsKey('version')){$vv=[string]$a['version'];$all=@($all|Where-Object{[string]$_.version-eq$vv})};if($a.ContainsKey('session')){$ss=[string]$a['session'];$all=@($all|Where-Object{[string]$_.session-eq$ss})};$total=$all.Count;$page=@($all|Select-Object -Skip $offset -First $limit);return ([ordered]@{schema='chq-script-run-list-v2';build=$workbenchVersion;currentSession=(Get-SessionId);total=$total;offset=$offset;limit=$limit;returned=$page.Count;hasMore=(($offset+$page.Count)-lt$total);runs=$page}|ConvertTo-Json -Depth 8 -Compress)}
        'ui.prompt'{if(-not $a.ContainsKey('message')){throw 'message is required'};$message=([string]$a['message']).Replace('_',' ');$pause=Bool-Arg $a 'pause' $true;$top=Bool-Arg $a 'top' $true;$timeout=if($a.ContainsKey('timeout')){[int]$a['timeout']}else{300000};if($timeout -lt 1000 -or $timeout -gt 3600000){throw 'timeout must be 1000..3600000 ms'};if($top){$null=Invoke-Chq @('window','top','on')};if($pause){$null=Invoke-Chq @('pause')};$null=Invoke-Chq @('prompt','show',$message);$sw=[Diagnostics.Stopwatch]::StartNew();do{$st=Parse-Kv (Invoke-Chq @('prompt','status'));if([int](Prop $st 'accepted' 0)-eq 1){return "OK prompt accepted elapsed_ms=$($sw.ElapsedMilliseconds)"};if([int](Prop $st 'cancelled' 0)-eq 1){throw 'SDL prompt cancelled by user'};Start-Sleep -Milliseconds 50}while($sw.ElapsedMilliseconds -lt $timeout);$null=Invoke-Chq @('prompt','clear');throw "Timed out waiting for SDL prompt after ${timeout}ms"}
        'sprite.override'{$slot=[int]$a['slot'];$map=if($a.ContainsKey('map')){[int]$a['map']}else{-1};$pal=if($a.ContainsKey('palette')){[int]$a['palette']}else{-1};$vis=if($a.ContainsKey('visible')){if(Bool-Arg $a 'visible') {1}else{0}}else{-1};return Invoke-Chq @('sprites','override',[string]$slot,[string]$map,[string]$pal,[string]$vis)}
        'sprite.override.clear'{return Invoke-Chq @('sprites','override-clear')}
        'sprite.visual'{$slot=[int]$a['slot'];$mode=([string]$a['mode']).ToLowerInvariant();return Invoke-Chq @('sprites','visual',[string]$slot,$mode)}
        'sprite.visual.clear'{return Invoke-Chq @('sprites','visual-clear')}
        'palette.entries'{$bank=[int]$a['bank'];return ((Get-PaletteEntryList $bank)|ConvertTo-Json -Depth 5 -Compress)}
        'state.snapshot'{$inc=if($a.ContainsKey('include')){[string]$a['include']}else{'sprites,regions,gameplay,track'};return ((Get-ResearchStateSnapshot $inc)|ConvertTo-Json -Depth 12 -Compress)}
        'gameplay.registry'{return ((Get-LiveGameplayRegistry)|ConvertTo-Json -Depth 8 -Compress)}
        'track.state'{return ((Get-LiveTrackState)|ConvertTo-Json -Depth 6 -Compress)}
        'track.record.start'{return ((Start-TrackRecorder)|ConvertTo-Json -Depth 8 -Compress)}
        'track.record.stop'{return ((Stop-TrackRecorder)|ConvertTo-Json -Depth 8 -Compress)}
        'track.record.clear'{return ((Clear-TrackRecorder)|ConvertTo-Json -Depth 8 -Compress)}
        'track.record.status'{return ((Get-TrackRecorderSnapshot $false)|ConvertTo-Json -Depth 8 -Compress)}
        'track.record.sample'{return ((Add-TrackRecorderSample $false)|ConvertTo-Json -Depth 12 -Compress)}
        'track.svg.export'{$name=if($a.ContainsKey('name')){[string]$a['name']}else{''};return ((Export-TrackRecorderSvg $name)|ConvertTo-Json -Depth 8 -Compress)}
        'track.map.export'{$name=if($a.ContainsKey('name')){[string]$a['name']}else{''};return ((Export-TrackMappingDataset $name)|ConvertTo-Json -Depth 10 -Compress)}
        'course.follow.status'{return ((Get-CourseFollowState)|ConvertTo-Json -Depth 8 -Compress)}
        'course.follow.configure'{return ((Set-CourseFollowConfig $a)|ConvertTo-Json -Depth 8 -Compress)}
        'course.follow.start'{return ((Start-CourseFollow)|ConvertTo-Json -Depth 8 -Compress)}
        'course.follow.stop'{return ((Stop-CourseFollow)|ConvertTo-Json -Depth 8 -Compress)}
        'course.follow.reset'{return ((Reset-CourseFollow)|ConvertTo-Json -Depth 8 -Compress)}
        'course.survey.status'{return (([ordered]@{schema='chq-course-survey-live-v1';active=[bool]$script:CourseSurveyActive;course=(Get-CourseFollowState);track=(Get-TrackRecorderStatus)})|ConvertTo-Json -Depth 9 -Compress)}
        'course.survey.start'{$collisionResponse=Bool-Arg $a 'collisionResponse' $false;$timerHold=Bool-Arg $a 'timerHold' $true;return ((Start-CourseSurvey (-not $collisionResponse) $timerHold)|ConvertTo-Json -Depth 10 -Compress)}
        'course.survey.stop'{return ((Stop-CourseSurvey)|ConvertTo-Json -Depth 10 -Compress)}
        'next-steps'{return (Get-Content $nextStepsPath -Raw)}
        'memory.export'{$cpu=([string]$a['cpu']).ToUpperInvariant();if($cpu -notin 'A','B'){throw 'cpu must be A or B'};if($a.ContainsKey('address')){$aa=Hex-U32 ([string]$a['address']);if(-not $a.ContainsKey('length')){throw 'length is required with address'};$len=[uint64](Count-U32 ([string]$a['length']));if($len -lt 1){throw 'length must be at least 1'};$zz=[uint64]$aa+$len-1}else{$start=[string]$a['start'];$end=[string]$a['end'];$aa=Hex-U32 $start;$zz=Hex-U32 $end;if($zz -lt $aa){throw 'end before start'};$len=[uint64]$zz-$aa+1};if($len -gt 1048576){throw 'memory export limited to 1 MiB'};$default=('memory-{0}-{1:X}-{2:X}.bin' -f $cpu,$aa,$zz);$requested=if($a.ContainsKey('output')){[string]$a['output']}elseif($a.ContainsKey('name')){(SafeName ([string]$a['name']))+'.bin'}else{$default};$file=Resolve-EvidenceOutput $requested $default;$raw=Invoke-Chq @('readrange',$cpu,('{0:X}' -f $aa),[string]$len);if($raw -notmatch '^OK '){throw "bulk read failed: $raw"};$hex=@();foreach($line in @($raw -split "`r?`n"|Select-Object -Skip 1)){if($line -match '^[0-9A-Fa-f]{8}:\s*(.*)$'){$hex += @($Matches[1] -split '\s+'|Where-Object{$_ -match '^[0-9A-Fa-f]{2}$'})}};if($hex.Count -ne [int]$len){throw "bulk read returned $($hex.Count) bytes; expected $len"};$bytes=New-Object byte[] ([int]$len);for($i=0;$i -lt $bytes.Length;$i++){$bytes[$i]=[Convert]::ToByte($hex[$i],16)};[IO.File]::WriteAllBytes($file,$bytes);return "OK memory export path=$file bytes=$len start=0x$('{0:X}' -f $aa) end=0x$('{0:X}' -f $zz)"}
        'experiment.begin'{$name=if($a.ContainsKey('name')){[string]$a['name']}else{''};return ((Begin-ResearchExperiment $name)|ConvertTo-Json -Depth 10 -Compress)}
        'experiment.capture'{$label=if($a.ContainsKey('label')){[string]$a['label']}else{'capture'};$inc=if($a.ContainsKey('include')){[string]$a['include']}else{'sprites,regions,gameplay,track'};return ((Capture-ResearchExperiment $label $inc)|ConvertTo-Json -Depth 10 -Compress)}
        'experiment.status'{if($script:activeExperiment){return ($script:activeExperiment|ConvertTo-Json -Depth 10 -Compress)};return (([ordered]@{active=$false})|ConvertTo-Json -Compress)}
        'experiment.end'{return ((End-ResearchExperiment)|ConvertTo-Json -Depth 12 -Compress)}
        'platform.info'{return ((Get-PlatformInfo)|ConvertTo-Json -Depth 8 -Compress)}
        'features.list'{return ((Get-FeatureManifest)|ConvertTo-Json -Depth 12 -Compress)}
        'script.functions'{return (([ordered]@{schema='chq-script-functions-v1';query=@('getregions','getprofiles','getsprites','getpaletteentries','getcheckpoints','getcapabilities','getmaps','getpalettes','getregistry','gettrackrecords','getevidence','getobjects','getaudiochannels');collections=@('count','first','last','contains','unique');notes=@('Query functions are API-backed.','Direct invocation prints the API result.','filter/sort/map are reserved for a later validated parser extension.')})|ConvertTo-Json -Depth 8 -Compress)}
        'registry.list'{return ((Get-LiveRegistryItems)|ConvertTo-Json -Depth 8 -Compress)}
        'track.records'{$before=if($a.ContainsKey('before')){[int]$a['before']}else{4};$after=if($a.ContainsKey('after')){[int]$a['after']}else{12};return ((Get-TrackRecords $before $after)|ConvertTo-Json -Depth 7 -Compress)}
        'objects.list'{return ((Get-SemanticObjects)|ConvertTo-Json -Depth 8 -Compress)}
        'audio.capabilities'{return ((Get-AudioCapabilities)|ConvertTo-Json -Depth 10 -Compress)}
        'audio.channels'{return ((Get-AudioChannels)|ConvertTo-Json -Depth 8 -Compress)}
        'audio.events'{return (([ordered]@{schema='chq-audio-events-v1';enabled=$false;reason='Native audio event capture requires Windows/SDL runtime validation';events=@();next='instrument sound CPU / mixer writes and timestamp against frame/audio sample clock'})|ConvertTo-Json -Depth 6 -Compress)}
        'audio.waveform.plan'{return ((Get-AudioCapabilities).waveform|ConvertTo-Json -Depth 8 -Compress)}
        'audio.export.plan'{return ((Get-AudioCapabilities).export|ConvertTo-Json -Depth 8 -Compress)}
        'audio.inject.plan'{return ((Get-AudioCapabilities).injection|ConvertTo-Json -Depth 8 -Compress)}
        'evidence.names'{return ((Get-EvidenceNames)|ConvertTo-Json -Depth 6 -Compress)}
        'docs.index'{return ((Get-DocumentationIndex)|ConvertTo-Json -Depth 12 -Compress)}
        'docs.search'{$q=if($a.ContainsKey('query')){[string]$a['query']}else{''};$cat=if($a.ContainsKey('category')){[string]$a['category']}else{''};$st=if($a.ContainsKey('status')){[string]$a['status']}else{''};return (@(Search-DocumentationEntries $q $cat $st)|ConvertTo-Json -Depth 12 -Compress)}
        'docs.get'{return ((Get-DocumentationEntry ([string]$a['id']))|ConvertTo-Json -Depth 12 -Compress)}
        'docs.glossary'{return (@(Get-DocumentationEntries|Where-Object{$_.kind -eq 'glossary'}|Sort-Object title)|ConvertTo-Json -Depth 12 -Compress)}
        'docs.handbook'{return ((Get-DocumentationHandbook)|ConvertTo-Json -Depth 20 -Compress)}
        'docs.page'{return ((Get-DocumentationHandbookPage ([string]$a['id']))|ConvertTo-Json -Depth 20 -Compress)}
        'docs.handover'{if(-not(Test-Path $chatgptHandoverPath)){throw 'CHATGPT_HANDOVER_PROMPT.md is missing'};return (Get-Content $chatgptHandoverPath -Raw -Encoding UTF8)}
        'docs.export'{$format=if($a.ContainsKey('format')){[string]$a['format']}else{'html'};$id=if($a.ContainsKey('id')){[string]$a['id']}else{''};$query=if($a.ContainsKey('query')){[string]$a['query']}else{''};$evidence=if($a.ContainsKey('evidence')){[string]$a['evidence']}else{'key'};$output=if($a.ContainsKey('output')){[string]$a['output']}else{''};return ((Export-Documentation $format $id $query $evidence $output)|ConvertTo-Json -Depth 8 -Compress)}
        'knowledge.list'{return (@(Get-DocumentationEntries)|ConvertTo-Json -Depth 12 -Compress)}
        'knowledge.get'{return ((Get-DocumentationEntry ([string]$a['id']))|ConvertTo-Json -Depth 12 -Compress)}
        'knowledge.search'{$q=if($a.ContainsKey('query')){[string]$a['query']}else{''};$cat=if($a.ContainsKey('category')){[string]$a['category']}else{''};$st=if($a.ContainsKey('status')){[string]$a['status']}else{''};return (@(Search-DocumentationEntries $q $cat $st)|ConvertTo-Json -Depth 12 -Compress)}
        'knowledge.related'{return (@(Get-DocumentationRelated ([string]$a['id']))|ConvertTo-Json -Depth 12 -Compress)}
        'schema'{if($a.ContainsKey('action')){$k=([string]$a['action']).ToLowerInvariant();if(-not $scriptActionSchemas.Contains($k)){throw "Unknown API action: $k"};return ([ordered]@{script='chq-research-script-v2';action=$k;contract=$scriptActionSchemas[$k]}|ConvertTo-Json -Depth 8 -Compress)};return ([ordered]@{script='chq-research-script-v2';apiActions=$scriptApiActions;contracts=$scriptActionSchemas}|ConvertTo-Json -Depth 8 -Compress)}
    }
}
function Get-ScriptLanguageSchema(){
    return [ordered]@{schema='chq-research-script-language-v1';engine='chq-research-script-v2';build=$workbenchVersion;statements=@(
      [ordered]@{syntax='# comment';description='Comment'},
      [ordered]@{syntax='set NAME = VALUE';description='Set a variable'},
      [ordered]@{syntax='${NAME}';description='Variable substitution'},
      [ordered]@{syntax='for ITEM in LIST ... end';description='Iterate a list/query result'},
      [ordered]@{syntax='repeat N ... end';description='Repeat a block; exposes ${repeat_index}'},
      [ordered]@{syntax='if status KEY=VALUE ... [else] ... end';description='Conditional status check'},
      [ordered]@{syntax='if memory cpu=A address=HEX width=8|16|32 value=HEX ... end';description='Conditional memory check'},
      [ordered]@{syntax='if capability NAME ... end';description='Conditional capability check'},
      [ordered]@{syntax='try ... finally ... end';description='Always run cleanup after the try body, including API/assert/runtime failure and user cancellation where practical; the original error remains primary'},
      [ordered]@{syntax='wait MS';description='Sleep 0..5000 ms'},
      [ordered]@{syntax='wait-until step-complete [TIMEOUT_MS]';description='Wait for native frame-step completion'},
      [ordered]@{syntax='require capability NAME';description='Require native capability'},
      [ordered]@{syntax='assert status KEY=VALUE';description='Assert a status field'},
      [ordered]@{syntax='echo TEXT';description='Write run output'},
      [ordered]@{syntax='api ACTION key=value ...';description='Invoke a documented Script API action'},
      [ordered]@{syntax='include "path.chqscript"';description='Include a saved library script'},
      [ordered]@{syntax='zip evidence latest|NAME';description='ZIP one evidence capture'},
      [ordered]@{syntax='zip evidence-match PREFIX [BUNDLE_NAME]';description='ZIP captures sharing a prefix'}
    );queryFunctions=@('getregions()','getprofiles()','getsprites()','getpaletteentries(bank=N)','getcheckpoints()','getcapabilities()','getmaps()','getpalettes()','getregistry()','gettrackrecords()','getevidence()','getobjects()','getaudiochannels()','count()','first()','last()','contains()','unique()');limits=[ordered]@{sourceBytes=65536;sourceLines=1000;expandedCommands=1000;loopIterations=256;nestingDepth=4;waitUntilMs=30000}}
}
function Get-ScriptHelpText(){
@'
ChaseHQ Research Script v2

Core syntax:
  set name = value
  ${name}
  for item in listVariable
      ...
  end
  repeat N
      ...
  end
  if status KEY OP VALUE
      ...
  else                         (optional)
      ...
  end
  if memory cpu=A address=ADDR width=8|16|32 value OP HEX
      ...
  end
  if capability NAME
      ...
  end
  try
      ...
  finally
      ...
  end
  OP is one of = != < <= > >=
  wait MS                     (0..5000)
  wait-until step-complete [timeout_ms]
  require capability NAME
  assert status key=value
  echo TEXT
  zip evidence latest|NAME
  zip ioc-sweep latest|NAME
  zip evidence-match PREFIX [BUNDLE_NAME]
  api ACTION key=value ...

API-backed query functions (assignment returns a comma-separated list; direct call prints JSON):
  getregions()
  getprofiles()
  getsprites()
  getpaletteentries(bank=N)
  getcheckpoints()
  getcapabilities()
  getmaps()
  getpalettes()

All existing safe native debugger commands remain valid too.
Use: api actions
for the current stable scriptable API action names.

Safety limits: 64 KiB source, 1000 source lines, 1000 expanded commands,
256 loop/repeat iterations, nesting depth 4, wait-until timeout <= 30000 ms.
'@
}
function Invoke-ChqScript([string]$script,[bool]$stopOnError,[bool]$validateOnly){
    $expanded=Expand-ChqScript $script;$results=@();$ok=$true
    foreach($item in $expanded.Items){
        $line=[string]$item.Command;$lineNo=[int]$item.Line;$context=[string]$item.Context
        try{
            $tokens=@(Split-ScriptTokens $line);if($tokens.Count -eq 0){continue};$first=([string]$tokens[0]).ToLowerInvariant();$out=''
            if($first -eq 'wait'){
                if($tokens.Count -ne 2){throw 'Usage: wait MS'};$ms=[int]$tokens[1];if($ms -lt 0 -or $ms -gt 5000){throw 'wait range is 0..5000 ms'}
                $out=if($validateOnly){"VALID wait $ms"}else{Start-Sleep -Milliseconds $ms;"OK waited ${ms}ms"}
            }
            elseif($first -eq 'wait-until'){
                if($tokens.Count -lt 2 -or $tokens[1].ToLowerInvariant() -ne 'step-complete'){throw 'Usage: wait-until step-complete [timeout_ms]'}
                $timeout=if($tokens.Count -ge 3){[int]$tokens[2]}else{10000};if($timeout -lt 100 -or $timeout -gt 30000){throw 'timeout range is 100..30000 ms'}
                if($validateOnly){$out="VALID wait-until step-complete $timeout"}else{$sw=[Diagnostics.Stopwatch]::StartNew();do{$st=Parse-Kv (Invoke-Chq @('status'));if([int]$st.step_remaining -eq 0){break};Start-Sleep -Milliseconds 20}while($sw.ElapsedMilliseconds -lt $timeout);if([int]$st.step_remaining -ne 0){throw 'Timed out waiting for step completion'};$out="OK step complete frame=$($st.frame)"}
            }
            elseif($first -eq 'echo'){$out=($tokens|Select-Object -Skip 1)-join ' '}
            elseif($first -in 'help','commands'){$out=if($first -eq 'commands'){"API actions:`n"+($scriptApiActions -join "`n")}elseif($tokens.Count -ge 2){Get-ScriptActionHelp ([string]$tokens[1])}else{Get-ScriptHelpText}}
            elseif($first -eq 'require'){
                if($tokens.Count -ne 3 -or $tokens[1].ToLowerInvariant() -ne 'capability'){throw 'Usage: require capability NAME'}
                if($validateOnly){$out='VALID'}else{$cap=Invoke-Chq @('capabilities');if($cap -notmatch ('(?i)(^|\s)'+[regex]::Escape($tokens[2])+'(\s|$)')){throw "Required capability missing: $($tokens[2])"};$out="OK capability $($tokens[2])"}
            }
            elseif($first -eq 'assert'){
                if($tokens.Count -ne 3 -or $tokens[1].ToLowerInvariant() -ne 'status' -or $tokens[2] -notmatch '^([^=]+)=(.*)$'){throw 'Usage: assert status key=value'}
                $key=$Matches[1];$expected=$Matches[2];if($validateOnly){$out='VALID'}else{$st=Parse-Kv (Invoke-Chq @('status'));$actual=Prop $st $key $null;if([string]$actual -ne $expected){throw "Assertion failed: status $key expected=$expected actual=$actual"};$out="OK assert $key=$expected"}
            }
            elseif($first -eq 'zip'){
                if($tokens.Count -lt 2){throw 'Usage: zip evidence|ioc-sweep [latest|NAME] | zip evidence-match PREFIX [BUNDLE_NAME]'}
                $kind=([string]$tokens[1]).ToLowerInvariant()
                if($kind -eq 'evidence-match'){
                    if($tokens.Count -lt 3 -or $tokens.Count -gt 4){throw 'Usage: zip evidence-match PREFIX [BUNDLE_NAME]'}
                    $prefix=[string]$tokens[2];$bundle=if($tokens.Count -eq 4){[string]$tokens[3]}else{''}
                    $out=if($validateOnly){"VALID zip evidence-match $prefix $bundle"}else{(Zip-EvidencePrefix $prefix $bundle|ConvertTo-Json -Compress)}
                }else{
                    if($tokens.Count -gt 3){throw 'Usage: zip evidence|ioc-sweep [latest|NAME]'};$name=if($tokens.Count -eq 3){[string]$tokens[2]}else{'latest'};$out=if($validateOnly){"VALID zip $kind $name"}else{Zip-ResearchOutput $kind $name}
                }
            }
            elseif($first -eq 'api'){
                if($tokens.Count -lt 2){throw 'Usage: api ACTION key=value ...'};$action=$tokens[1]
                if($action -eq 'actions'){$out=$scriptApiActions -join "`n"}else{$args=Script-Args $tokens 2;$out=Invoke-ScriptApiAction $action $args -ValidateOnly:$validateOnly}
            }
            else{
                if($validateOnly){if($line.Length -gt 512 -or $line -match '[;&|`$<>]'){throw 'Command failed validation'};if($allowedFirst -notcontains $first){throw 'Command not allowed by web bridge'};$out='VALID'}else{$out=Invoke-SafeCommand $line}
            }
            if(-not $validateOnly -and ([string]$out) -match '^ERR(?:\s|$)'){throw ([string]$out)}
            $results += [ordered]@{line=$lineNo;context=$context;command=$line;output=[string]$out}
        }catch{$ok=$false;$results += [ordered]@{line=$lineNo;context=$context;command=$line;error=$_.Exception.Message};if($stopOnError){break}}
    }
    return [ordered]@{ok=$ok;validateOnly=$validateOnly;count=$results.Count;expandedCount=$expanded.Count;results=$results}
}

function Get-NextRunId(){
    $runsRoot=Join-Path $sessionPath 'runs';New-Item -ItemType Directory -Force -Path $runsRoot|Out-Null
    $max=0;foreach($d in Get-ChildItem $runsRoot -Directory -ErrorAction SilentlyContinue){if($d.Name -match '^\d{3,}$'){$n=[int]$d.Name;if($n -gt $max){$max=$n}}}
    return ('{0:D3}' -f ($max+1))
}
function Get-SessionId(){if([string]::IsNullOrWhiteSpace($sessionPath)){return 'unknown'};return (Split-Path $sessionPath -Leaf)}
function Invoke-ChqScriptRun([string]$source,[bool]$stopOnError,[bool]$validateOnly,[string]$requestedRunId='',[string]$runSource='',[string]$runName='',[string]$runPurpose='',[string]$sourceType='',[string]$sourcePath=''){
    if($validateOnly){return Invoke-ChqScript $source $stopOnError $true}
    $started=Get-Date;$runId=if([string]::IsNullOrWhiteSpace($requestedRunId)){Get-NextRunId}else{SafeName $requestedRunId}
    $runRoot=Join-Path (Join-Path $sessionPath 'runs') $runId;$artRoot=Join-Path $runRoot 'artifacts';New-Item -ItemType Directory -Force -Path $artRoot|Out-Null
    $existingMeta=Join-Path $runRoot 'run-metadata.json';if(Test-Path $existingMeta){try{$priorMeta=Get-Content $existingMeta -Raw|ConvertFrom-Json;$priorStarted=[string](Prop $priorMeta 'started' '');if($priorStarted){$started=[datetimeoffset]::Parse($priorStarted).LocalDateTime}}catch{}}
    if(-not(Test-Path (Join-Path $runRoot 'script.chqscript'))){Set-Content (Join-Path $runRoot 'script.chqscript') $(if([string]::IsNullOrWhiteSpace($runSource)){$source}else{$runSource}) -Encoding UTF8}
    # Script-generated evidence is rooted directly in this run. Packaging is deferred until finalization.
    $priorEvidencePath=$evidencePath
    try{$evidencePath=$artRoot;$result=Invoke-ChqScript $source $stopOnError $false}finally{$evidencePath=$priorEvidencePath}
    $lines=@();foreach($r in @($result.results)){$lines += ('['+[string]$r.line+'] '+[string]$r.command);if($r.Contains('output')){$lines += [string]$r['output']};if($r.Contains('error')){$lines += ('ERROR: '+[string]$r['error'])};$lines += ''};Add-Content (Join-Path $runRoot 'console-output.txt') ($lines -join "`r`n") -Encoding UTF8
    $updated=Get-Date;$runSlug=if([string]::IsNullOrWhiteSpace($runName)){'script-run'}else{(([string]$runName -replace '[^A-Za-z0-9_.-]+','-').Trim('-'))};if([string]::IsNullOrWhiteSpace($runSlug)){$runSlug='script-run'};$downloadStem=((Get-SessionId)+'-r'+$runId+'-'+$runSlug).Trim('-');if($downloadStem.Length -gt 120){$downloadStem=$downloadStem.Substring(0,120).TrimEnd('-')};$downloadName=$downloadStem+'.zip';$meta=[ordered]@{schema='chq-script-run-v2';runId=$runId;build=$workbenchVersion;sessionId=(Get-SessionId);sessionPath=$sessionPath;started=$started.ToString('o');updated=$updated.ToString('o');ended=$null;durationMs=[int64](($updated-$started).TotalMilliseconds);ok=$null;status='RUNNING';validateOnly=$false;name=$runName;purpose=$runPurpose;sourceType=$sourceType;sourcePath=$sourcePath;artifactRoot=$artRoot;source='script.chqscript';console='console-output.txt';bundleDownloadName=$downloadName};$meta|ConvertTo-Json -Depth 8|Set-Content (Join-Path $runRoot 'run-metadata.json') -Encoding UTF8
    $script:AuthoritativeRunHistoryCache=$null;$script:AuthoritativeRunHistoryCacheAt=$null
    $result['runId']=$runId;$result['runPath']=$runRoot;$result['artifactPath']=$artRoot;$result['bundle']='';$result['bundleDownloadName']=$downloadName;$result['build']=$workbenchVersion;$result['sessionId']=Get-SessionId;return $result
}
function Finalize-ChqScriptRun([string]$requestedRunId,[string]$runStatus='PASS'){
    if([string]::IsNullOrWhiteSpace($requestedRunId)){throw 'runId is required for finalization'}
    $runId=SafeName $requestedRunId;$runRoot=Join-Path (Join-Path $sessionPath 'runs') $runId;$metaPath=Join-Path $runRoot 'run-metadata.json';if(-not(Test-Path $metaPath -PathType Leaf)){throw "Run metadata not found: $runId"}
    $meta=Get-Content $metaPath -Raw -Encoding UTF8|ConvertFrom-Json;$ended=Get-Date;$started=[datetimeoffset]::Parse([string](Prop $meta 'started' $ended.ToString('o')));$status=$runStatus.ToUpperInvariant();if($status -notin @('PASS','FAIL','CANCELLED')){throw 'runStatus must be PASS, FAIL or CANCELLED'}
    $final=[ordered]@{};foreach($prop in $meta.PSObject.Properties){$final[$prop.Name]=$prop.Value};$final['updated']=$ended.ToString('o');$final['ended']=$ended.ToString('o');$final['durationMs']=[int64](($ended-$started.LocalDateTime).TotalMilliseconds);$final['status']=$status;$final['ok']=($status -eq 'PASS');$final|ConvertTo-Json -Depth 8|Set-Content $metaPath -Encoding UTF8
    $manifestPath=Join-Path $runRoot 'bundle-manifest.json';$manifest=[ordered]@{schema='chq-run-bundle-manifest-v2';run=$final;files=@(Get-ChildItem $runRoot -Recurse -File|Where-Object{$_.Name -notin @('bundle.zip','bundle-manifest.json')}|ForEach-Object{$_.FullName.Substring($runRoot.Length).TrimStart([char[]]'\/')})};$manifest|ConvertTo-Json -Depth 8|Set-Content $manifestPath -Encoding UTF8
    $zip=Join-Path $runRoot 'bundle.zip';$items=Get-ChildItem $runRoot -Force|Where-Object{$_.FullName -ne $zip};if(Test-Path $zip){Remove-Item $zip -Force};Compress-Archive -Path $items.FullName -DestinationPath $zip -CompressionLevel Optimal
    $script:AuthoritativeRunHistoryCache=$null;$script:AuthoritativeRunHistoryCacheAt=$null
    return [ordered]@{ok=($status -eq 'PASS');status=$status;runId=$runId;runPath=$runRoot;artifactPath=(Join-Path $runRoot 'artifacts');bundle=$zip;bundleDownloadName=[string](Prop $final 'bundleDownloadName' 'bundle.zip');build=$workbenchVersion;sessionId=Get-SessionId}
}

try{while($listener.IsListening){$c=$listener.GetContext();$reqSw=[Diagnostics.Stopwatch]::StartNew();$path='?';$method='?';try{
$path=$c.Request.Url.AbsolutePath;$method=$c.Request.HttpMethod
if($path -eq '/') {Send $c $html 'text/html; charset=utf-8'}
elseif($path -eq '/api/status'){Send $c (Invoke-Chq @('status'))}
elseif($path -eq '/api/input/ports'){Send $c (Invoke-Chq @('input','ports'))}
elseif($path -eq '/api/events'){Send $c (Invoke-Chq @('events','tail','100'))}
elseif($path -eq '/api/sprites'){Send $c (Invoke-Chq @('sprites','tail','100','semantic'))}
elseif($path -eq '/api/cmd' -and $method -eq 'POST'){ $sr=[IO.StreamReader]::new($c.Request.InputStream,$c.Request.ContentEncoding);$q=$sr.ReadToEnd();$sr.Dispose();Send $c (Invoke-SafeCommand $q)}
elseif($path -eq '/api/health'){SendJson $c ([ordered]@{ok=$true;product='ChaseHQ-Native';bridge=$workbenchVersion;debugPort=$Port;httpPort=$HttpPort;session=$sessionPath;native=(Parse-Kv (Invoke-Chq @('status')))})}
elseif($path -eq '/api/v1/artifact' -and $method -eq 'GET'){
    $q=[System.Web.HttpUtility]::ParseQueryString($c.Request.Url.Query);$raw=[string]$q['path'];if([string]::IsNullOrWhiteSpace($raw)){throw 'Artifact path required'}
    $full=[IO.Path]::GetFullPath($raw);$allowed=$false
    foreach($base0 in @($evidencePath,$sessionPath,$evidenceSessionsRoot,$checkpointRoot)){
        if($base0){$base=[IO.Path]::GetFullPath($base0).TrimEnd([IO.Path]::DirectorySeparatorChar)+[IO.Path]::DirectorySeparatorChar;if($full.StartsWith($base,[StringComparison]::OrdinalIgnoreCase)){$allowed=$true;break}}
    }
    $norm=$full.Replace('/','\');$workspaceBase=[IO.Path]::GetFullPath($workspaceRoot).TrimEnd([IO.Path]::DirectorySeparatorChar)+[IO.Path]::DirectorySeparatorChar
    $isVersionEvidence=$norm.IndexOf('\evidence\sessions\',[StringComparison]::OrdinalIgnoreCase) -ge 0
    $isVersionCheckpoint=$norm.IndexOf('\checkpoints\',[StringComparison]::OrdinalIgnoreCase) -ge 0
    $isVersionTree=$norm.IndexOf('\ChaseHQ-Native-v',[StringComparison]::OrdinalIgnoreCase) -ge 0
    if(-not $allowed -and $full.StartsWith($workspaceBase,[StringComparison]::OrdinalIgnoreCase) -and $isVersionTree -and ($isVersionEvidence -or $isVersionCheckpoint)){$allowed=$true}
    # Old browser-history entries contain absolute paths from earlier extracted versions. If that exact version folder moved/was renamed,
    # resolve the stable evidence-session suffix against sibling ChaseHQ-Native-v* folders before declaring the preview missing.
    if(-not(Test-Path $full -PathType Leaf) -and $isVersionEvidence){
        $needle='\evidence\sessions\';$idx=$norm.IndexOf($needle,[StringComparison]::OrdinalIgnoreCase)
        if($idx -ge 0){$suffix=$norm.Substring($idx+$needle.Length);$candidate=Get-ChildItem $workspaceRoot -Directory -Filter 'ChaseHQ-Native-v*' -ErrorAction SilentlyContinue|ForEach-Object{Join-Path $_.FullName ('evidence\sessions\'+$suffix)}|Where-Object{Test-Path $_ -PathType Leaf}|Select-Object -First 1;if($candidate){$full=[IO.Path]::GetFullPath($candidate);$allowed=$true}}
    }
    if(-not $allowed -or -not(Test-Path $full -PathType Leaf)){throw 'Artifact is outside permitted ChaseHQ evidence/checkpoint roots or does not exist'}
    $ext=[IO.Path]::GetExtension($full).ToLowerInvariant();$type=switch($ext){'.png'{'image/png'}'.json'{'application/json; charset=utf-8'}'.txt'{'text/plain; charset=utf-8'}'.csv'{'text/csv; charset=utf-8'}default{'application/octet-stream'}}
    if($q['download'] -eq '1'){$downloadName=if($q['name']){[IO.Path]::GetFileName([string]$q['name'])}else{[IO.Path]::GetFileName($full)};if([string]::IsNullOrWhiteSpace($downloadName)){$downloadName=[IO.Path]::GetFileName($full)};SendBytes $c ([IO.File]::ReadAllBytes($full)) $downloadName}else{SendBinary $c ([IO.File]::ReadAllBytes($full)) $type}
}
elseif($path -eq '/api/v1/open-folder' -and $method -eq 'POST'){ $b=ReadJson $c.Request;$raw=[string](Prop $b 'path' '');if([string]::IsNullOrWhiteSpace($raw)){throw 'Folder path required'};$full=[IO.Path]::GetFullPath($raw);$allowed=$false;foreach($sessionsRoot in @(Get-KnownEvidenceSessionRoots)){$base=[IO.Path]::GetFullPath($sessionsRoot).TrimEnd([IO.Path]::DirectorySeparatorChar)+[IO.Path]::DirectorySeparatorChar;if(($full+[IO.Path]::DirectorySeparatorChar).StartsWith($base,[StringComparison]::OrdinalIgnoreCase)){$allowed=$true;break}};if(-not $allowed){throw 'Folder is outside known ChaseHQ evidence-session roots'};if(-not(Test-Path $full -PathType Container)){throw 'Folder not found'};Start-Process explorer.exe -ArgumentList @('/e,',('"'+$full+'"'));SendJson $c ([ordered]@{ok=$true;path=$full})}
elseif($path -eq '/api/v1/openapi.json'){SendJson $c ([ordered]@{openapi='3.0.3';info=[ordered]@{title='ChaseHQ Research API';version=$workbenchVersion;description='Localhost research/debug automation API'};servers=@([ordered]@{url="http://127.0.0.1:$HttpPort"});paths=[ordered]@{'/api/v1/status'=[ordered]@{get=[ordered]@{summary='Machine status'}};'/api/v1/profiles'=[ordered]@{get=[ordered]@{summary='List packaged game profiles without activating them'}};'/api/v1/profile'=[ordered]@{get=[ordered]@{summary='Read active or named game profile descriptor'}};'/api/v1/tuning/handling'=[ordered]@{get=[ordered]@{summary='Read live handling tuning'};put=[ordered]@{summary='Set live cornering/speed-retain override'};delete=[ordered]@{summary='Restore authentic handling'}};'/api/v1/collision/response'=[ordered]@{get=[ordered]@{summary='Read collision-response suppression state'};put=[ordered]@{summary='Enable/disable proven response writes while preserving detection'};delete=[ordered]@{summary='Restore authentic collision response'}};'/api/v1/palette/{bank}'=[ordered]@{get=[ordered]@{summary='Read one 16-pen palette bank'}};'/api/v1/palette/entry/{index}'=[ordered]@{put=[ordered]@{summary='Freeze a palette entry'};delete=[ordered]@{summary='Return palette entry to game control'}};'/api/v1/input/ioc/xor'=[ordered]@{put=[ordered]@{summary='Set held IOC XOR mask'};delete=[ordered]@{summary='Clear IOC overrides'}};'/api/v1/input/ioc/pulse'=[ordered]@{post=[ordered]@{summary='Pulse IOC mask for emulated frames'}};'/api/v1/cpu/disassemble'=[ordered]@{get=[ordered]@{summary='Read-only Musashi disassembly'}};'/api/v1/control/run-frames'=[ordered]@{post=[ordered]@{summary='Run exactly N emulated frames and stop'}};'/api/v1/gameplay/registry'=[ordered]@{get=[ordered]@{summary='Live evidence-backed Chase H.Q. gameplay registry'}};'/api/v1/track/state'=[ordered]@{get=[ordered]@{summary='Live confirmed road/course telemetry'}};'/api/v1/track/record'=[ordered]@{get=[ordered]@{summary='Shared Track View recorder state and samples'}};'/api/v1/track/record/sample'=[ordered]@{post=[ordered]@{summary='Append one deterministic track sample'}};'/api/v1/track/svg/export'=[ordered]@{post=[ordered]@{summary='Export shared recorded track as SVG'}};'/api/v1/track/map/export'=[ordered]@{post=[ordered]@{summary='Export reconstructable course mapping dataset'}};'/api/v1/course/follow'=[ordered]@{get=[ordered]@{summary='Read live course follower state'}};'/api/v1/course/follow/configure'=[ordered]@{post=[ordered]@{summary='Configure live course follower'}};'/api/v1/course/follow/start'=[ordered]@{post=[ordered]@{summary='Start live course follower'}};'/api/v1/course/follow/stop'=[ordered]@{post=[ordered]@{summary='Stop live course follower'}};'/api/v1/course/follow/reset'=[ordered]@{post=[ordered]@{summary='Reset live course follower controller'}};'/api/v1/course/survey'=[ordered]@{get=[ordered]@{summary='Read live course survey state'}};'/api/v1/course/survey/start'=[ordered]@{post=[ordered]@{summary='Start reversible live course survey'}};'/api/v1/course/survey/stop'=[ordered]@{post=[ordered]@{summary='Stop/restore live course survey'}};'/api/v1/next-steps'=[ordered]@{get=[ordered]@{summary='High-level outstanding diagnosis'}};'/api/v1/image/regions'=[ordered]@{get=[ordered]@{summary='Named frame-analysis regions'}};'/api/v1/image/compare'=[ordered]@{post=[ordered]@{summary='Local PNG comparison with optional source-aware HUD mode and diff output'}};'/api/v1/evidence/capture'=[ordered]@{post=[ordered]@{summary='Capture evidence bundle'}};'/api/v1/frame/snapshot'=[ordered]@{post=[ordered]@{summary='Capture reconstructable layered frame snapshot with HUD variants'}};'/api/v1/frame/snapshots'=[ordered]@{get=[ordered]@{summary='List structured frame snapshots'}};'/api/v1/frame/snapshot/{name}/manifest'=[ordered]@{get=[ordered]@{summary='Read v2 frame snapshot manifest and reconstruction metadata'}};'/api/v1/frame/snapshot/{name}/asset/{path}'=[ordered]@{get=[ordered]@{summary='Read a v2 frame snapshot PNG/JSON/CSV asset'}};'/api/v1/frame.png'=[ordered]@{get=[ordered]@{summary='Current SDL frame as PNG'}};'/api/v1/checkpoints'=[ordered]@{get=[ordered]@{summary='List packaged canonical checkpoints'}};'/api/v1/checkpoints/load'=[ordered]@{post=[ordered]@{summary='Load packaged checkpoint and optionally run'}};'/api/v1/input/ioc/sweep'=[ordered]@{post=[ordered]@{summary='Deterministic checkpoint-based IOC mask sweep'}};'/api/v1/logs'=[ordered]@{get=[ordered]@{summary='List current session logs'}};'/api/v1/logs/tail'=[ordered]@{get=[ordered]@{summary='Tail a current session log'}};'/api/v1/frontend-log'=[ordered]@{post=[ordered]@{summary='Record browser-side Workbench fault'}};'/api/v1/diagnostics/bundle'=[ordered]@{post=[ordered]@{summary='Create diagnostic ZIP for a runner/tool failure'}};'/api/v1/control/timer-freeze'=[ordered]@{put=[ordered]@{summary='Freeze game timer'};delete=[ordered]@{summary='Resume game timer'}};'/api/v1/window/always-on-top'=[ordered]@{put=[ordered]@{summary='Set SDL always-on-top'}};'/api/v1/window/status'=[ordered]@{get=[ordered]@{summary='Read SDL window state'}};'/api/v1/window/fullscreen'=[ordered]@{put=[ordered]@{summary='Set SDL fullscreen'}};'/api/v1/window/scale'=[ordered]@{put=[ordered]@{summary='Set SDL integer window scale'}};'/api/v1/window/show'=[ordered]@{post=[ordered]@{summary='Show SDL window'}};'/api/v1/window/hide'=[ordered]@{post=[ordered]@{summary='Hide SDL window'}};'/api/v1/window/minimize'=[ordered]@{post=[ordered]@{summary='Minimize SDL window'}};'/api/v1/window/restore'=[ordered]@{post=[ordered]@{summary='Restore SDL window'}};'/api/v1/docs/handbook'=[ordered]@{get=[ordered]@{summary='Complete authored User & Research Handbook model'}};'/api/v1/docs/page/{id}'=[ordered]@{get=[ordered]@{summary='One authored handbook page'}};'/api/v1/docs/export'=[ordered]@{post=[ordered]@{summary='Export full handbook or selected knowledge to HTML/PDF'}};'/api/v1/schema/actions'=[ordered]@{get=[ordered]@{summary='Complete Script API action schema'}};'/api/v1/schema/script'=[ordered]@{get=[ordered]@{summary='Complete Research Script language schema'}};'/api/v1/script'=[ordered]@{post=[ordered]@{summary='Validate or run safe research script'}};'/api/v1/script/help'=[ordered]@{get=[ordered]@{summary='Research Script v2 syntax and API actions'}};'/api/v1/scripts'=[ordered]@{get=[ordered]@{summary='List saved research scripts'}};'/api/v1/scripts/load'=[ordered]@{post=[ordered]@{summary='Load a saved research script'}};'/api/v1/scripts/save'=[ordered]@{post=[ordered]@{summary='Save a research script'}};'/api/v1/script/runs'=[ordered]@{get=[ordered]@{summary='List authoritative completed script runs from on-disk metadata'}};'/api/v1/game/action'=[ordered]@{post=[ordered]@{summary='Chase H.Q. Game Lab reversible target/turbo/current-speed experiment controls'}};'/api/v1/tile/inspect'=[ordered]@{post=[ordered]@{summary='Inspect/export one TC0100SCN RAM text character'}};'/api/v1/evidence'=[ordered]@{get=[ordered]@{summary='List evidence captures'}};'/api/v1/evidence/bundle'=[ordered]@{post=[ordered]@{summary='Create a ZIP from captures sharing a name prefix'}}}})}
elseif($path -eq '/api/docs/' -or $path -eq '/api/docs'){ $docs=@'
<!doctype html><meta charset="utf-8"><meta name="viewport" content="width=device-width"><title>ChaseHQ Research API</title><style>body{font:15px system-ui;max-width:1050px;margin:30px auto;padding:0 16px;background:#101216;color:#eee}code,pre{background:#1b2029;padding:3px 6px;border-radius:4px}section{border:1px solid #343944;border-radius:8px;padding:14px;margin:12px 0}button{padding:7px;background:#293140;color:#fff;border:1px solid #4b5563;border-radius:5px}a{color:#93c5fd}.m{color:#86efac}</style><h1>ChaseHQ Research API <small>v0.66.9.0</small></h1><p>Localhost-only live research/debug interface. <a href="/api/v1/openapi.json">OpenAPI JSON</a> | <a href="/api/health">Health</a> | <a href="/">Web Debugger</a></p><section><b class="m">GET</b> <code>/api/v1/status</code><p>Machine/frame/research status.</p></section><section><b class="m">GET</b> <code>/api/v1/tuning/handling</code><br><b>PUT</b> <code>/api/v1/tuning/handling</code> <code>{"cornering":1.15,"speedRetain":1.0}</code><br><b>DELETE</b> restores authentic values.</section><section><b class="m">GET</b> <code>/api/v1/palette/{bank}</code><p>Live 16-pen TC0110PCR bank.</p><b>PUT</b> <code>/api/v1/palette/entry/{index}</code> <code>{"raw":"7FFF"}</code><br><b>DELETE</b> returns entry to game control.</section><section><b>PUT</b> <code>/api/v1/input/ioc/xor</code> <code>{"port":3,"mask":"20"}</code><br><b>DELETE</b> clears held IOC overrides.<p>For momentary controls use <code>POST /api/v1/input/ioc/pulse</code>.</p></section><section><b>POST</b> <code>/api/v1/evidence/capture</code><p>Consistent checkpoint + screenshot + event/sprite exports + manifest. Preserves the machine's prior running/paused state.</p></section><section><b class="m">GET</b> <code>/api/v1/frame.png</code><p>Static/current SDL frame preview.</p><b class="m">GET</b> <code>/api/v1/checkpoints</code> | <b>POST</b> <code>/api/v1/checkpoints/load</code><p>Browse and load packaged canonical checkpoints.</p><b>POST</b> <code>/api/v1/input/ioc/sweep</code><p>Run repeatable IOC masks from the same checkpoint and collect screenshots/events automatically.</p><b class="m">GET</b> <code>/api/v1/logs</code> | <b class="m">GET</b> <code>/api/v1/logs/tail</code><p>Browse/tail logs from the active research session.</p></section><section><b class="m">GET</b> <code>/api/v1/gameplay/registry</code> | <b class="m">GET</b> <code>/api/v1/track/state</code><p>Live game-profile telemetry.</p><b class="m">GET</b> <code>/api/v1/cpu/disassemble</code><p>Read-only Musashi disassembly.</p><b>POST</b> <code>/api/v1/control/run-frames</code><p>Exact emulated-frame execution.</p><b>POST</b> <code>/api/v1/image/compare</code><p>Local captured-frame comparison / optional diff PNG.</p></section><section><b>POST</b> <code>/api/v1/script</code><p>Research Script v2 supports variables, for/repeat loops, deterministic waits, assertions, ZIP packaging and named <code>api ACTION</code> calls. <b>GET</b> <code>/api/v1/script/help</code> lists syntax/actions. The Script Library lives under <code>/api/v1/scripts</code>.</p></section><p>This page is deliberately dependency-free so it works offline. The OpenAPI document is the machine-readable contract.</p>
'@;Send $c $docs 'text/html; charset=utf-8'}
elseif($path -eq '/api/v1/tuning/handling' -and $method -eq 'GET'){SendJson $c (Parse-Kv (Invoke-Chq @('handling','get')))}
elseif($path -eq '/api/v1/tuning/handling' -and $method -eq 'PUT'){ $b=ReadJson $c.Request;$cs=[double](Prop $b 'cornering' 1.0);$sr=[double](Prop $b 'speedRetain' 1.0);SendJson $c (Parse-Kv (Invoke-Chq @('handling','set',([string]$cs),([string]$sr))))}
elseif($path -eq '/api/v1/tuning/handling' -and $method -eq 'DELETE'){SendJson $c (Parse-Kv (Invoke-Chq @('handling','reset')))}
elseif($path -match '^/api/v1/palette/([0-9]+)$' -and $method -eq 'GET'){SendJson $c (Parse-Kv (Invoke-Chq @('palette','bank',$Matches[1])))}
elseif($path -match '^/api/v1/palette/entry/([0-9]+)$' -and $method -eq 'PUT'){ $idx=$Matches[1];$b=ReadJson $c.Request;$raw=[string](Prop $b 'raw' '');SendJson $c (Parse-Kv (Invoke-Chq @('palette','freeze',$idx,$raw)))}
elseif($path -match '^/api/v1/palette/entry/([0-9]+)$' -and $method -eq 'DELETE'){SendJson $c (Parse-Kv (Invoke-Chq @('palette','restore',$Matches[1])))}
elseif($path -eq '/api/v1/palette/overrides' -and $method -eq 'DELETE'){SendJson $c (Parse-Kv (Invoke-Chq @('palette','clear')))}
elseif($path -eq '/api/v1/regions' -and $method -eq 'GET'){SendJson $c ([ordered]@{regions=(Get-ResearchRegions)})}
elseif($path -eq '/api/v1/sprites' -and $method -eq 'GET'){SendJson $c ([ordered]@{sprites=(Get-LiveSprites)})}
elseif($path -match '^/api/v1/sprites/([0-9]+)$' -and $method -eq 'GET'){ $slot=[int]$Matches[1];$x=@(Get-LiveSprites|Where-Object{$_.slot -eq $slot}|Select-Object -First 1);if(-not$x){throw 'Sprite slot not active'};SendJson $c $x[0]}
elseif($path -match '^/api/v1/sprites/([0-9]+)/override$' -and $method -eq 'PUT'){ $slot=[int]$Matches[1];$b=ReadJson $c.Request;$map=if($null-ne(Prop $b 'map' $null)){[int](Prop $b 'map' -1)}else{-1};$pal=if($null-ne(Prop $b 'palette' $null)){[int](Prop $b 'palette' -1)}else{-1};$v=Prop $b 'visible' $null;$vis=if($null-eq$v){-1}elseif([bool]$v){1}else{0};SendJson $c (Parse-Kv (Invoke-Chq @('sprites','override',[string]$slot,[string]$map,[string]$pal,[string]$vis)))}
elseif($path -eq '/api/v1/sprites/overrides' -and $method -eq 'DELETE'){SendJson $c (Parse-Kv (Invoke-Chq @('sprites','override-clear')))}
elseif($path -match '^/api/v1/sprites/([0-9]+)/visual$' -and $method -eq 'PUT'){ $slot=[int]$Matches[1];$b=ReadJson $c.Request;$mode=[string](Prop $b 'mode' '');SendJson $c (Parse-Kv (Invoke-Chq @('sprites','visual',[string]$slot,$mode)))}
elseif($path -eq '/api/v1/sprites/visual' -and $method -eq 'DELETE'){SendJson $c (Parse-Kv (Invoke-Chq @('sprites','visual-clear')))}
elseif($path -eq '/api/v1/state/snapshot' -and $method -eq 'GET'){ $q=[System.Web.HttpUtility]::ParseQueryString($c.Request.Url.Query);$inc=if($q['include']){[string]$q['include']}else{'sprites,regions,gameplay,track'};SendJson $c (Get-ResearchStateSnapshot $inc)}
elseif($path -eq '/api/v1/input/ioc/xor' -and $method -eq 'PUT'){ $b=ReadJson $c.Request;$iocPort=[int](Prop $b 'port' -1);$mask=[string](Prop $b 'mask' '');if($iocPort -lt 0 -or $iocPort -gt 15 -or $mask -notmatch '^[0-9A-Fa-f]{1,2}$'){throw 'Invalid IOC held-state arguments'};SendJson $c (Parse-Kv (Invoke-Chq @('input','xor','set',([string]$iocPort),$mask)))}
elseif($path -eq '/api/v1/input/ioc/xor' -and $method -eq 'DELETE'){SendJson $c (Parse-Kv (Invoke-Chq @('input','xor','clear')))}
elseif($path -eq '/api/v1/frame.png' -and $method -eq 'GET'){
    $null=Invoke-Chq @('screenshot',$previewPath)
    if(-not(Test-Path $previewPath)){throw 'Preview screenshot was not created'}
    SendBinary $c ([IO.File]::ReadAllBytes($previewPath)) 'image/png'
}
elseif($path -eq '/api/v1/checkpoints' -and $method -eq 'GET'){SendJson $c ([ordered]@{checkpoints=(Get-CheckpointCatalog)})}
elseif($path -eq '/api/v1/checkpoints/load' -and $method -eq 'POST'){
    $b=ReadJson $c.Request;$file=[string](Prop $b 'file' '');$run=[bool](Prop $b 'run' $true);$full=Resolve-CheckpointFile $file
    $r=Invoke-Chq @('checkpoint','load',$full);$script:currentCheckpointFile=$file;$resume=$null;if($run){$resume=Invoke-Chq @('resume')}
    SendJson $c ([ordered]@{ok=$true;file=$file;run=$run;load=$r;resume=$resume;status=(Parse-Kv (Invoke-Chq @('status')))})
}
elseif($path -eq '/api/v1/logs' -and $method -eq 'GET'){SendJson $c ([ordered]@{sessionRoot=$sessionPath;files=(Get-SessionLogFiles)})}
elseif($path -eq '/api/v1/logs/tail' -and $method -eq 'GET'){
    $q=[System.Web.HttpUtility]::ParseQueryString($c.Request.Url.Query);$rel=[string]$q['file'];$n=80;if($q['lines']){$n=[Math]::Max(10,[Math]::Min(2000,[int]$q['lines']))}
    if([string]::IsNullOrWhiteSpace($rel) -or $rel -match '\.\.' -or [IO.Path]::IsPathRooted($rel)){throw 'Invalid log path'}
    if($rel -eq '@runtime/web-stdout.log'){$resolved=Join-Path (Join-Path $root '.chq') 'web-stdout.log'}
    elseif($rel -eq '@runtime/web-stderr.log'){$resolved=Join-Path (Join-Path $root '.chq') 'web-stderr.log'}
    elseif($rel -eq '@runtime/web-listener.log'){$resolved=$runtimeListenerLog}
    elseif($rel -eq '@runtime/web-command-diagnostics.log'){$resolved=$runtimeCommandDiag}
    elseif($rel -eq '@runtime/web-frontend-errors.log'){$resolved=$runtimeFrontendDiag}
    else{if(-not $sessionPath){throw 'No session log root is associated with this Workbench'};$full=Join-Path $sessionPath ($rel -replace '/','\\');$resolved=(Resolve-Path $full -ErrorAction Stop).Path;if(-not $resolved.StartsWith((Resolve-Path $sessionPath).Path,[StringComparison]::OrdinalIgnoreCase)){throw 'Log path escapes session root'}}
    if(-not(Test-Path $resolved)){throw 'Log file not found'}
    Send $c ((Get-Content $resolved -Tail $n -ErrorAction Stop) -join "`n") 'text/plain; charset=utf-8'
}
elseif($path -eq '/api/v1/input/ioc/sweep' -and $method -eq 'POST'){ $b=ReadJson $c.Request;$file=[string](Prop $b 'checkpoint' 'stage1-gameplay-2064.chqstate');$iocPort=[int](Prop $b 'port' 3);$masks=@(Prop $b 'masks' @('01','02','04','08','10','20','40','80'));$pulseFrames=[int](Prop $b 'pulseFrames' 8);$observeFrames=[int](Prop $b 'observeFrames' 90);SendJson $c (Invoke-IocSweepCore $file $iocPort $masks $pulseFrames $observeFrames) }
elseif($path -eq '/api/v1/frontend-log' -and $method -eq 'POST'){ $b=ReadJson $c.Request;$entry=[ordered]@{received=(Get-Date).ToString('o');build=$workbenchVersion;phase=[string](Prop $b 'phase' '');context=[string](Prop $b 'context' '');script=[string](Prop $b 'script' '');message=[string](Prop $b 'message' '');stack=[string](Prop $b 'stack' '');source=[string](Prop $b 'source' '');sourceType=[string](Prop $b 'sourceType' '');lastApi=(Prop $b 'lastApi' $null)};Add-Content -Path $runtimeFrontendDiag -Value ($entry|ConvertTo-Json -Depth 8 -Compress) -Encoding UTF8;SendJson $c ([ordered]@{ok=$true;log='@runtime/web-frontend-errors.log'})}
elseif($path -eq '/api/v1/diagnostics/bundle' -and $method -eq 'POST'){ $b=ReadJson $c.Request;SendJson $c (New-DiagnosticBundle $b) }
elseif($path -eq '/api/v1/script/help' -and $method -eq 'GET'){SendJson $c ([ordered]@{text=(Get-ScriptHelpText);actions=$scriptApiActions})}
elseif($path -eq '/api/v1/scripts' -and $method -eq 'GET'){SendJson $c ([ordered]@{scripts=(Get-ScriptLibrary)})}
elseif($path -eq '/api/v1/scripts/load' -and $method -eq 'POST'){ $b=ReadJson $c.Request;$r=Resolve-ScriptLibraryPath ([string](Prop $b 'path' ''));SendJson $c ([ordered]@{path=$r.Relative;script=(Get-Content $r.Full -Raw)}) }
elseif($path -eq '/api/v1/scripts/save' -and $method -eq 'POST'){ $b=ReadJson $c.Request;$r=Resolve-ScriptLibraryPath ([string](Prop $b 'path' '')) -ForSave;$script=[string](Prop $b 'script' '');if($script.Length -gt 65536){throw 'Script too long'};New-Item -ItemType Directory -Path (Split-Path $r.Full) -Force|Out-Null;Set-Content $r.Full $script -Encoding UTF8;SendJson $c ([ordered]@{ok=$true;path=$r.Relative}) }
elseif($path -eq '/api/v1/script' -and $method -eq 'POST'){ $b=ReadJson $c.Request;$script=[string](Prop $b 'script' '');$stopOnError=[bool](Prop $b 'stopOnError' $true);$validateOnly=[bool](Prop $b 'validateOnly' $false);$rid=[string](Prop $b 'runId' '');$rsrc=[string](Prop $b 'runSource' '');$rn=[string](Prop $b 'runName' '');$rp=[string](Prop $b 'runPurpose' '');$st=[string](Prop $b 'sourceType' '');$sp=[string](Prop $b 'sourcePath' '');$transient=[bool](Prop $b 'transient' $false);$finalize=[bool](Prop $b 'finalizeRun' $false);$runStatus=[string](Prop $b 'runStatus' 'PASS');if($transient){SendJson $c (Invoke-ChqScript $script $stopOnError $validateOnly)}elseif($finalize){SendJson $c (Finalize-ChqScriptRun $rid $runStatus)}else{SendJson $c (Invoke-ChqScriptRun $script $stopOnError $validateOnly $rid $rsrc $rn $rp $st $sp)} }
elseif($path -eq '/api/v1/evidence' -and $method -eq 'GET'){
    $caps=@();foreach($d in Get-ChildItem $evidencePath -Directory -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending){$mp=Join-Path $d.FullName 'manifest.json';$state=Join-Path $d.FullName 'state.chqstate';$shot=Join-Path $d.FullName 'screenshot.png';if(-not(Test-Path $mp) -and -not((Test-Path $state) -and (Test-Path $shot))){continue};$m=$null;if(Test-Path $mp){try{$m=Get-Content $mp -Raw -Encoding UTF8|ConvertFrom-Json}catch{}};$schema=if($m){[string](Prop $m 'schema' '')}else{''};if($schema -and $schema -ne 'chq-evidence-v2'){continue};$frame=$null;if($m){$st=[string](Prop $m 'statusCaptured' '');$kv=Parse-Kv $st;$frame=Prop $kv 'frame' $null};$caps += [ordered]@{type='evidence';name=$d.Name;created=$(if($m){Prop $m 'created' $d.LastWriteTime.ToString('o')}else{$d.LastWriteTime.ToString('o')});frame=$frame;wasPaused=$(if($m){[bool](Prop $m 'wasPaused' $false)}else{$false});zipReady=(Test-Path (Join-Path $evidencePath ($d.Name+'.zip')))}};SendJson $c ([ordered]@{captures=$caps})
}
elseif($path -eq '/api/v1/evidence/bundle' -and $method -eq 'POST'){ $b=ReadJson $c.Request;$prefix=[string](Prop $b 'prefix' '');$name=[string](Prop $b 'name' '');SendJson $c (Zip-EvidencePrefix $prefix $name) }
elseif($path -match '^/api/v1/evidence-bundle/([A-Za-z0-9_.-]+)/download$' -and $method -eq 'GET'){ $name=SafeName $Matches[1];$zip=Join-Path $evidencePath ($name+'.zip');if(-not(Test-Path $zip)){throw 'Evidence bundle ZIP not found'};SendBytes $c ([IO.File]::ReadAllBytes($zip)) ($name+'.zip') }
elseif($path -match '^/api/v1/evidence/([A-Za-z0-9_.-]+)$' -and $method -eq 'GET'){
    $name=SafeName $Matches[1];$dir=Join-Path $evidencePath $name;if(-not(Test-Path $dir)){throw 'Evidence capture not found'};$mp=Join-Path $dir 'manifest.json';$m=if(Test-Path $mp){Get-Content $mp -Raw|ConvertFrom-Json}else{[pscustomobject]@{}};SendJson $c ([ordered]@{name=$name;path=$dir;manifest=$m;zipReady=(Test-Path (Join-Path $evidencePath ($name+'.zip')))})
}
elseif($path -match '^/api/v1/evidence/([A-Za-z0-9_.-]+)/screenshot\.png$' -and $method -eq 'GET'){
    $name=SafeName $Matches[1];$file=Join-Path (Join-Path $evidencePath $name) 'screenshot.png';if(-not(Test-Path $file)){throw 'Evidence screenshot not found'};SendBinary $c ([IO.File]::ReadAllBytes($file)) 'image/png'
}
elseif($path -match '^/api/v1/evidence/([A-Za-z0-9_.-]+)/load$' -and $method -eq 'POST'){
    $name=SafeName $Matches[1];$file=Join-Path (Join-Path $evidencePath $name) 'state.chqstate';if(-not(Test-Path $file)){throw 'Evidence state not found'};$r=Invoke-Chq @('checkpoint','load',$file);SendJson $c ([ordered]@{ok=$true;name=$name;result=$r;paused=$true})
}
elseif($path -match '^/api/v1/evidence/([A-Za-z0-9_.-]+)/zip$' -and $method -eq 'POST'){
    $name=SafeName $Matches[1];$dir=Join-Path $evidencePath $name;if(-not(Test-Path $dir)){throw 'Evidence capture not found'};$zip=Join-Path $evidencePath ($name+'.zip');if(Test-Path $zip){Remove-Item $zip -Force};Compress-Archive -Path (Join-Path $dir '*') -DestinationPath $zip -CompressionLevel Optimal;SendJson $c ([ordered]@{ok=$true;name=$name;path=$zip;download="/api/v1/evidence/$name/download"})
}
elseif($path -match '^/api/v1/evidence/([A-Za-z0-9_.-]+)/download$' -and $method -eq 'GET'){
    $name=SafeName $Matches[1];$zip=Join-Path $evidencePath ($name+'.zip');if(-not(Test-Path $zip)){throw 'Evidence ZIP not found; create it first'};SendBytes $c ([IO.File]::ReadAllBytes($zip)) ($name+'.zip')
}
elseif($path -eq '/api/v1/profiles' -and $method -eq 'GET'){SendJson $c ([ordered]@{schema='chq-game-profile-list-v1';active=$activeProfileId;profiles=@(Get-GameProfiles)})}
elseif($path -eq '/api/v1/profile' -and $method -eq 'GET'){ $q=[System.Web.HttpUtility]::ParseQueryString($c.Request.Url.Query);$id=if($q['game']){[string]$q['game']}else{$activeProfileId};SendJson $c (Get-GameProfile $id) }
elseif($path -eq '/api/v1/gameplay/registry' -and $method -eq 'GET'){SendJson $c (Get-LiveGameplayRegistry)}
elseif($path -eq '/api/v1/next-steps' -and $method -eq 'GET'){Send $c (Get-Content $nextStepsPath -Raw) 'application/json; charset=utf-8'}
elseif($path -eq '/api/v1/track/state' -and $method -eq 'GET'){SendJson $c (Get-LiveTrackState)}
elseif($path -eq '/api/v1/course/follow' -and $method -eq 'GET'){SendJson $c (Get-CourseFollowState)}
elseif($path -eq '/api/v1/course/follow/configure' -and $method -eq 'POST'){ $b=ReadJson $c.Request;$a=@{};foreach($pp in $b.PSObject.Properties){$a[$pp.Name]=$pp.Value};SendJson $c (Set-CourseFollowConfig $a)}
elseif($path -eq '/api/v1/course/follow/start' -and $method -eq 'POST'){SendJson $c (Start-CourseFollow)}
elseif($path -eq '/api/v1/course/follow/stop' -and $method -eq 'POST'){SendJson $c (Stop-CourseFollow)}
elseif($path -eq '/api/v1/course/follow/reset' -and $method -eq 'POST'){SendJson $c (Reset-CourseFollow)}
elseif($path -eq '/api/v1/course/survey' -and $method -eq 'GET'){SendJson $c ([ordered]@{schema='chq-course-survey-live-v1';active=[bool]$script:CourseSurveyActive;course=(Get-CourseFollowState);track=(Get-TrackRecorderStatus)})}
elseif($path -eq '/api/v1/course/survey/start' -and $method -eq 'POST'){ $b=ReadJson $c.Request;$collisionResponse=[bool](Prop $b 'collisionResponse' $false);$timerHold=[bool](Prop $b 'timerHold' $true);SendJson $c (Start-CourseSurvey (-not $collisionResponse) $timerHold)}
elseif($path -eq '/api/v1/course/survey/stop' -and $method -eq 'POST'){SendJson $c (Stop-CourseSurvey)}
elseif($path -eq '/api/v1/track/record' -and $method -eq 'GET'){SendJson $c (Get-TrackRecorderSnapshot $true)}
elseif($path -eq '/api/v1/track/record/start' -and $method -eq 'POST'){SendJson $c (Start-TrackRecorder)}
elseif($path -eq '/api/v1/track/record/stop' -and $method -eq 'POST'){SendJson $c (Stop-TrackRecorder)}
elseif($path -eq '/api/v1/track/record/clear' -and $method -eq 'POST'){SendJson $c (Clear-TrackRecorder)}
elseif($path -eq '/api/v1/track/record/sample' -and $method -eq 'POST'){SendJson $c (Add-TrackRecorderSample $true)}
elseif($path -eq '/api/v1/track/svg/export' -and $method -eq 'POST'){ $b=ReadJson $c.Request;$name=[string](Prop $b 'name' '');SendJson $c (Export-TrackRecorderSvg $name)}
elseif($path -eq '/api/v1/track/map/export' -and $method -eq 'POST'){ $b=ReadJson $c.Request;$name=[string](Prop $b 'name' '');SendJson $c (Export-TrackMappingDataset $name)}
elseif($path -match '^/api/v1/track/svg/([A-Za-z0-9_.-]+)/download$' -and $method -eq 'GET'){ $name=SafeName $Matches[1];$file=Join-Path (Join-Path $evidencePath 'track-exports') ($name+'.svg');if(-not(Test-Path $file)){throw 'Track SVG not found'};SendBytes $c ([IO.File]::ReadAllBytes($file)) ($name+'.svg')}

elseif($path -eq '/api/v1/image/regions' -and $method -eq 'GET'){Send $c (Get-Content $imageRegionsPath -Raw) 'application/json; charset=utf-8'}
elseif($path -eq '/api/v1/image/compare' -and $method -eq 'POST'){ $b=ReadJson $c.Request;$aa=[string](Prop $b 'a' '');$bb=[string](Prop $b 'b' '');$region=[string](Prop $b 'region' 'FULL');$out=[string](Prop $b 'output' '');$hud=[string](Prop $b 'hud' 'normal');SendJson $c (Compare-ResearchImages $aa $bb $region $out $hud) }
elseif($path -eq '/api/v1/cpu/disassemble' -and $method -eq 'GET'){ $q=[System.Web.HttpUtility]::ParseQueryString($c.Request.Url.Query);$cpu=([string]$q['cpu']).ToUpperInvariant();$addr=[string]$q['address'];$count=if($q['count']){[int]$q['count']}else{16};if($cpu -notin 'A','B' -or $addr -notmatch '^[0-9A-Fa-f]{1,8}$' -or $count -lt 1 -or $count -gt 256){throw 'Invalid disassembly arguments'};$text=Invoke-Chq @('disasm',$cpu,$addr,[string]$count);SendJson $c ([ordered]@{cpu=$cpu;address=$addr;count=$count;text=$text}) }
elseif($path -eq '/api/v1/control/run-frames' -and $method -eq 'POST'){ $b=ReadJson $c.Request;$n=[int](Prop $b 'frames' 1);if($n -lt 1 -or $n -gt 100000){throw 'frames out of range'};$timeout=[int](Prop $b 'timeout' 900000);if($timeout -lt 1000 -or $timeout -gt 3600000){throw 'timeout must be 1000..3600000 ms'};$null=Invoke-Chq @('pause');$st=Parse-Kv (Invoke-Chq @('status'));$start=[int]$st.frame;$null=Invoke-Chq @('run',[string]$n);$done=Wait-ChqPaused ($start+$n) $timeout 15000;SendJson $c ([ordered]@{ok=$true;start=$start;frame=[int]$done.frame;frames=$n;paused=([string]$done.paused -eq '1');timeoutMs=$timeout}) }
elseif($path -eq '/api/v1/frame/snapshot' -and $method -eq 'POST'){ $b=ReadJson $c.Request;$name=[string](Prop $b 'name' '');SendJson $c (New-StructuredFrameSnapshot $name) }
elseif($path -match '^/api/v1/frame/snapshot/([A-Za-z0-9_.-]+)/manifest$' -and $method -eq 'GET'){ $name=SafeName $Matches[1];$dir=Resolve-StructuredFrameSnapshotDir $name;$mp=Join-Path $dir 'manifest.json';$rp=Join-Path $dir 'reconstruction.json';if(-not(Test-Path $mp)){throw 'Frame snapshot not found'};$m=Get-Content $mp -Raw|ConvertFrom-Json;$r=if(Test-Path $rp){Get-Content $rp -Raw|ConvertFrom-Json}else{$null};SendJson $c ([ordered]@{name=$name;manifest=$m;reconstructionData=$r})}
elseif($path -match '^/api/v1/frame/snapshot/([A-Za-z0-9_.-]+)/asset/(.+)$' -and $method -eq 'GET'){ $name=SafeName $Matches[1];$rel=[Uri]::UnescapeDataString($Matches[2]);if($rel -match '\.\.' -or [IO.Path]::IsPathRooted($rel)){throw 'Invalid snapshot asset path'};$dir=Resolve-StructuredFrameSnapshotDir $name;$full=Join-Path $dir ($rel -replace '/','\');$resolved=(Resolve-Path $full -ErrorAction Stop).Path;if(-not $resolved.StartsWith((Resolve-Path $dir).Path,[StringComparison]::OrdinalIgnoreCase)){throw 'Snapshot asset escapes root'};$ext=[IO.Path]::GetExtension($resolved).ToLowerInvariant();$type=if($ext -eq '.png'){'image/png'}elseif($ext -eq '.json'){'application/json; charset=utf-8'}elseif($ext -eq '.csv'){'text/csv; charset=utf-8'}else{'text/plain; charset=utf-8'};SendBinary $c ([IO.File]::ReadAllBytes($resolved)) $type}
elseif($path -eq '/api/v1/frame/snapshots' -and $method -eq 'GET'){SendJson $c ([ordered]@{schema='chq-frame-snapshot-list-v2';snapshots=@(Get-StructuredFrameSnapshots)})}
elseif($path -eq '/api/v1/timeline/list' -and $method -eq 'GET'){SendJson $c ([ordered]@{schema='chq-forensic-timeline-list-v1';timelines=@(Get-ForensicTimelines)})}
elseif($path -eq '/api/v1/timeline/status' -and $method -eq 'GET'){SendJson $c ([ordered]@{schema='chq-forensic-timeline-status-v1';native=(Parse-Kv (Invoke-Chq @('timeline','status')))})}
elseif($path -eq '/api/v1/docs/index' -and $method -eq 'GET'){SendJson $c (Get-DocumentationIndex)}
elseif($path -eq '/api/v1/docs/search' -and $method -eq 'GET'){ $q=[System.Web.HttpUtility]::ParseQueryString($c.Request.Url.Query);SendJson $c ([ordered]@{schema='chq-docs-search-v1';build=$workbenchVersion;query=[string]$q['q'];entries=@(Search-DocumentationEntries ([string]$q['q']) ([string]$q['category']) ([string]$q['status']))}) }
elseif($path -eq '/api/v1/docs/glossary' -and $method -eq 'GET'){SendJson $c ([ordered]@{schema='chq-docs-glossary-v1';build=$workbenchVersion;entries=@(Get-DocumentationEntries|Where-Object{$_.kind -eq 'glossary'}|Sort-Object title)})}
elseif($path -eq '/api/v1/docs/handbook' -and $method -eq 'GET'){SendJson $c (Get-DocumentationHandbook)}
elseif($path -match '^/api/v1/docs/page/([A-Za-z0-9_.-]+)$' -and $method -eq 'GET'){SendJson $c (Get-DocumentationHandbookPage $Matches[1])}
elseif($path -eq '/api/v1/docs/handover' -and $method -eq 'GET'){if(-not(Test-Path $chatgptHandoverPath)){throw 'CHATGPT_HANDOVER_PROMPT.md is missing'};Send $c (Get-Content $chatgptHandoverPath -Raw -Encoding UTF8) 'text/markdown; charset=utf-8'}
elseif($path -eq '/api/v1/docs/export' -and $method -eq 'POST'){ $b=ReadJson $c.Request;SendJson $c (Export-Documentation ([string](Prop $b 'format' 'html')) ([string](Prop $b 'id' '')) ([string](Prop $b 'query' '')) ([string](Prop $b 'evidence' 'key')) ([string](Prop $b 'output' ''))) }
elseif($path -match '^/api/v1/docs/get/([A-Za-z0-9_.-]+)$' -and $method -eq 'GET'){SendJson $c (Get-DocumentationEntry $Matches[1])}
elseif($path -match '^/api/v1/docs/related/([A-Za-z0-9_.-]+)$' -and $method -eq 'GET'){SendJson $c ([ordered]@{schema='chq-docs-related-v1';id=$Matches[1];entries=@(Get-DocumentationRelated $Matches[1])})}
elseif($path -match '^/api/v1/docs/evidence/(.+)$' -and $method -eq 'GET'){ $rel=[Uri]::UnescapeDataString($Matches[1]);$file=Resolve-DocumentationEvidence $rel;$ext=[IO.Path]::GetExtension($file).ToLowerInvariant();$type=if($ext -eq '.png'){'image/png'}elseif($ext -eq '.jpg' -or $ext -eq '.jpeg'){'image/jpeg'}elseif($ext -eq '.webp'){'image/webp'}elseif($ext -eq '.json'){'application/json; charset=utf-8'}elseif($ext -eq '.csv'){'text/csv; charset=utf-8'}else{'text/plain; charset=utf-8'};SendBinary $c ([IO.File]::ReadAllBytes($file)) $type}
elseif($path -match '^/api/v1/docs/export/([A-Za-z0-9_.-]+\.(?:html|pdf))/download$' -and $method -eq 'GET'){ $name=$Matches[1];$file=Join-Path $docsExportRoot $name;if(-not(Test-Path $file -PathType Leaf)){throw 'Documentation export not found'};$bytes=[IO.File]::ReadAllBytes($file);$type=if($name.EndsWith('.pdf',[StringComparison]::OrdinalIgnoreCase)){'application/pdf'}else{'text/html; charset=utf-8'};$c.Response.StatusCode=200;$c.Response.ContentType=$type;$c.Response.AddHeader('Content-Disposition',('attachment; filename="'+$name+'"'));$c.Response.ContentLength64=$bytes.Length;$c.Response.OutputStream.Write($bytes,0,$bytes.Length);$c.Response.Close()}
elseif($path -eq '/api/v1/script/runs' -and $method -eq 'GET'){ $q=[System.Web.HttpUtility]::ParseQueryString($c.Request.Url.Query);$limit=if($q['limit']){[Math]::Max(1,[Math]::Min(100,[int]$q['limit']))}else{50};$offset=if($q['offset']){[Math]::Max(0,[int]$q['offset'])}else{0};$all=@(Get-AuthoritativeScriptRuns);if($q['status']){$sv=([string]$q['status']).ToUpperInvariant();$all=@($all|Where-Object{([string]$_.status).ToUpperInvariant()-eq$sv})};if($q['version']){$vv=[string]$q['version'];$all=@($all|Where-Object{[string]$_.version-eq$vv})};if($q['session']){$ss=[string]$q['session'];$all=@($all|Where-Object{[string]$_.session-eq$ss})};$total=$all.Count;$page=@($all|Select-Object -Skip $offset -First $limit);SendJson $c ([ordered]@{schema='chq-script-run-list-v2';build=$workbenchVersion;currentSession=(Get-SessionId);total=$total;offset=$offset;limit=$limit;returned=$page.Count;hasMore=(($offset+$page.Count)-lt$total);runs=$page})}
elseif($path -eq '/api/v1/schema/actions' -and $method -eq 'GET'){ $all=[ordered]@{};foreach($k in $scriptApiActions){$all[$k]=$scriptActionSchemas[$k]};SendJson $c ([ordered]@{schema='chq-script-api-actions-v1';build=$workbenchVersion;actions=$all})}
elseif($path -eq '/api/v1/schema/script' -and $method -eq 'GET'){SendJson $c (Get-ScriptLanguageSchema)}
elseif($path -eq '/api/v1/game/action' -and $method -eq 'POST'){
    $b=ReadJson $c.Request;$act=[string](Prop $b 'action' 'status');$map=@{'status'='game.experiment.status';'target.one-hit'='game.target.one-hit';'turbo.stock'='game.turbo.stock';'turbo.active'='game.turbo.active';'turbo.fire'='game.turbo.fire';'turbo.timer.reset'='game.turbo.timer.reset';'turbo.remaining.set'='game.turbo.remaining.set';'speed.set'='game.speed.set';'speed.freeze'='game.speed.freeze';'speed.clear'='game.speed.clear';'sprite.order.inspect'='game.sprite.order.inspect';'sprite.order.set'='game.sprite.order.set'};if(-not$map.ContainsKey($act)){throw 'Unknown Game Lab action'};$a=@{};foreach($pr in $b.PSObject.Properties){if($pr.Name-ne'action'){$a[$pr.Name]=[string]$pr.Value}};$out=Invoke-ScriptApiAction $map[$act] $a;if($act-eq'status'){SendJson $c ([ordered]@{ok=$true;state=($out|ConvertFrom-Json)})}else{SendJson $c ([ordered]@{ok=$true;result=$out;state=(Get-GameLabState)})}
}
elseif($path -eq '/api/v1/tile/inspect' -and $method -eq 'POST'){
    $b=ReadJson $c.Request;$a=@{code=[string](Prop $b 'code' '0C');palette=[string](Prop $b 'palette' '0');scale=[string](Prop $b 'scale' '8')};$out=Invoke-ScriptApiAction 'tile.inspect' $a;SendJson $c ([ordered]@{ok=$true;result=($out|ConvertFrom-Json)})
}
elseif($path -eq '/api/v1/schema'){SendJson $c ([ordered]@{schema='chq-http-api-v1';version='1.12';transport='localhost-http';endpoints=@('/api/v1/status','/api/v1/profiles','/api/v1/profile','/api/v1/capabilities','/api/v1/schema','/api/v1/openapi.json','/api/v1/tuning/handling','/api/v1/palette/{bank}','/api/v1/palette/entry/{index}','/api/v1/palette/overrides','/api/v1/input/ioc/xor','/api/v1/input/ports','/api/v1/events','/api/v1/sprites','/api/v1/memory/{cpu}/{address}?width=','/api/v1/memory/write','/api/v1/register/read','/api/v1/register/write','/api/v1/patch/freeze','/api/v1/patch/remove','/api/v1/patches','/api/v1/cpu/disassemble','/api/v1/gameplay/registry','/api/v1/track/state','/api/v1/track/record','/api/v1/track/record/start','/api/v1/track/record/stop','/api/v1/track/record/clear','/api/v1/track/record/sample','/api/v1/track/svg/export','/api/v1/track/map/export','/api/v1/course/follow','/api/v1/course/follow/configure','/api/v1/course/follow/start','/api/v1/course/follow/stop','/api/v1/course/follow/reset','/api/v1/course/survey','/api/v1/course/survey/start','/api/v1/course/survey/stop','/api/v1/next-steps','/api/v1/image/regions','/api/v1/image/compare','/api/v1/control/pause','/api/v1/control/resume','/api/v1/control/step-frame','/api/v1/control/run-frames','/api/v1/control/step-instruction','/api/v1/input/ioc/pulse','/api/v1/input/ioc/sweep','/api/v1/checkpoints','/api/v1/checkpoints/load','/api/v1/checkpoints/save','/api/v1/frame/snapshot','/api/v1/frame/snapshots','/api/v1/timeline/list','/api/v1/timeline/status','/api/v1/frame/snapshot/{name}/manifest','/api/v1/frame/snapshot/{name}/asset/{path}','/api/v1/docs/index','/api/v1/docs/search','/api/v1/docs/glossary','/api/v1/docs/handbook','/api/v1/docs/page/{id}','/api/v1/docs/handover','/api/v1/docs/get/{id}','/api/v1/docs/related/{id}','/api/v1/docs/evidence/{path}','/api/v1/docs/export','/api/v1/frame.png','/api/v1/logs','/api/v1/logs/tail','/api/v1/control/timer-freeze','/api/v1/window/always-on-top','/api/v1/window/status','/api/v1/window/fullscreen','/api/v1/window/scale','/api/v1/window/show','/api/v1/window/hide','/api/v1/window/minimize','/api/v1/window/restore','/api/v1/command','/api/v1/evidence/capture','/api/v1/controller/mapping','/api/v1/script','/api/v1/script/help','/api/v1/scripts','/api/v1/scripts/load','/api/v1/scripts/save','/api/v1/script/runs','/api/v1/game/action','/api/v1/tile/inspect','/api/v1/evidence','/api/v1/evidence/{name}','/api/v1/evidence/{name}/zip','/api/v1/evidence/{name}/download','/api/v1/evidence/bundle','/api/v1/evidence-bundle/{name}/download','/api/v1/export/memory');limits=[ordered]@{memoryExportBytes=1048576;commandLength=512};scriptActions=$scriptApiActions;notes=@('Native JSON/bulk export and SSE remain planned; bridge parsing is transitional.','Mutating operations execute through the debugger API and should be treated as research controls.')})}
elseif($path -eq '/api/v1/status'){SendJson $c (Parse-Kv (Invoke-Chq @('status')))}
elseif($path -eq '/api/v1/capabilities'){ $r=Invoke-Chq @('capabilities');SendJson $c ([ordered]@{raw=$r;capabilities=@(($r -replace '^OK\s*','') -split '\s+'|Where-Object{$_})}) }
elseif($path -eq '/api/v1/input/ports'){SendJson $c (Parse-Kv (Invoke-Chq @('input','ports')))}
elseif($path -eq '/api/v1/events'){ $r=Invoke-Chq @('events','tail','100');SendJson $c ([ordered]@{raw=$r;lines=@($r -split "`r?`n")}) }
elseif($path -eq '/api/v1/sprites'){ $r=Invoke-Chq @('sprites','tail','100','semantic');SendJson $c ([ordered]@{raw=$r;lines=@($r -split "`r?`n")}) }
elseif($path -match '^/api/v1/memory/([ABab])/([0-9A-Fa-f]+)$'){ $cpu=$Matches[1].ToUpperInvariant();$addr=$Matches[2];$width=16;try{$q=[System.Web.HttpUtility]::ParseQueryString($c.Request.Url.Query);if($q['width']){$width=[int]$q['width']}}catch{};if($width -notin 8,16,32){throw 'width must be 8, 16 or 32'};SendJson $c (Parse-Kv (Invoke-Chq @('read',$cpu,$addr,[string]$width))) }
elseif($path -eq '/api/v1/memory/write' -and $method -eq 'POST'){ $b=ReadJson $c.Request;$a=@{cpu=[string](Prop $b 'cpu' '');address=[string](Prop $b 'address' '');width=[string](Prop $b 'width' '16');value=[string](Prop $b 'value' '')};if(Prop $b 'expect' $null){$a['expect']=[string](Prop $b 'expect' '')};SendJson $c ([ordered]@{ok=$true;result=(Invoke-ScriptApiAction 'memory.write' $a)}) }
elseif($path -eq '/api/v1/patch/freeze' -and $method -eq 'POST'){ $b=ReadJson $c.Request;$a=@{cpu=[string](Prop $b 'cpu' '');address=[string](Prop $b 'address' '');width=[string](Prop $b 'width' '16');value=[string](Prop $b 'value' '')};if(Prop $b 'expect' $null){$a['expect']=[string](Prop $b 'expect' '')};SendJson $c ([ordered]@{ok=$true;result=(Invoke-ScriptApiAction 'patch.freeze' $a)}) }
elseif($path -eq '/api/v1/patch/remove' -and $method -eq 'POST'){ $b=ReadJson $c.Request;SendJson $c ([ordered]@{ok=$true;result=(Invoke-ScriptApiAction 'patch.remove' @{id=[string](Prop $b 'id' '')})}) }
elseif($path -eq '/api/v1/patches' -and $method -eq 'GET'){SendJson $c ([ordered]@{ok=$true;result=(Invoke-ScriptApiAction 'patch.list' @{})})}
elseif($path -eq '/api/v1/register/read' -and $method -eq 'GET'){ $q=[System.Web.HttpUtility]::ParseQueryString($c.Request.Url.Query);$a=@{cpu=[string]$q['cpu']};if($q['name']){$a['name']=[string]$q['name']};SendJson $c ([ordered]@{ok=$true;result=(Invoke-ScriptApiAction 'register.read' $a)}) }
elseif($path -eq '/api/v1/register/write' -and $method -eq 'POST'){ $b=ReadJson $c.Request;SendJson $c ([ordered]@{ok=$true;result=(Invoke-ScriptApiAction 'register.write' @{cpu=[string](Prop $b 'cpu' '');name=[string](Prop $b 'name' '');value=[string](Prop $b 'value' '')})}) }
elseif($path -eq '/api/v1/checkpoints/save' -and $method -eq 'POST'){ $b=ReadJson $c.Request;$a=@{file=[string](Prop $b 'file' '')};foreach($k in 'name','role','description'){if(Prop $b $k $null){$a[$k]=[string](Prop $b $k '')}};SendJson $c ([ordered]@{ok=$true;result=(Invoke-ScriptApiAction 'checkpoint.save' $a)}) }
elseif($path -eq '/api/v1/control/pause' -and $method -eq 'POST'){SendJson $c (Parse-Kv (Invoke-Chq @('pause')))}
elseif($path -eq '/api/v1/control/resume' -and $method -eq 'POST'){SendJson $c (Parse-Kv (Invoke-Chq @('resume')))}
elseif($path -eq '/api/v1/control/timer-freeze' -and $method -eq 'PUT'){SendJson $c (Parse-Kv (Invoke-Chq @('timer','freeze')))}
elseif($path -eq '/api/v1/control/timer-freeze' -and $method -eq 'DELETE'){SendJson $c (Parse-Kv (Invoke-Chq @('timer','resume')))}
elseif($path -eq '/api/v1/window/always-on-top' -and $method -eq 'PUT'){ $b=ReadJson $c.Request;$enabled=[bool](Prop $b 'enabled' $true);SendJson $c (Parse-Kv (Invoke-Chq @('window','top',$(if($enabled){'on'}else{'off'}))))}
elseif($path -eq '/api/v1/window/status' -and $method -eq 'GET'){SendJson $c (Parse-Kv (Invoke-Chq @('window','status')))}
elseif($path -eq '/api/v1/window/fullscreen' -and $method -eq 'PUT'){ $b=ReadJson $c.Request;$enabled=[bool](Prop $b 'enabled' $true);SendJson $c (Parse-Kv (Invoke-Chq @('window','fullscreen',$(if($enabled){'on'}else{'off'}))))}
elseif($path -eq '/api/v1/window/scale' -and $method -eq 'PUT'){ $b=ReadJson $c.Request;$n=[int](Prop $b 'scale' 3);if($n -lt 1 -or $n -gt 8){throw 'scale must be 1..8'};SendJson $c (Parse-Kv (Invoke-Chq @('window','scale',[string]$n)))}
elseif($path -eq '/api/v1/window/show' -and $method -eq 'POST'){SendJson $c (Parse-Kv (Invoke-Chq @('window','show')))}
elseif($path -eq '/api/v1/window/hide' -and $method -eq 'POST'){SendJson $c (Parse-Kv (Invoke-Chq @('window','hide')))}
elseif($path -eq '/api/v1/window/minimize' -and $method -eq 'POST'){SendJson $c (Parse-Kv (Invoke-Chq @('window','minimize')))}
elseif($path -eq '/api/v1/window/restore' -and $method -eq 'POST'){SendJson $c (Parse-Kv (Invoke-Chq @('window','restore')))}
elseif($path -eq '/api/v1/control/step-frame' -and $method -eq 'POST'){ $b=ReadJson $c.Request;$n=if((Prop $b 'frames' 0)){[int](Prop $b 'frames')}else{1};if($n -lt 1 -or $n -gt 100000){throw 'frames out of range'};SendJson $c (Parse-Kv (Invoke-Chq @('step','frame',[string]$n))) }
elseif($path -eq '/api/v1/control/step-instruction' -and $method -eq 'POST'){ $b=ReadJson $c.Request;$cpu=([string](Prop $b 'cpu' '')).ToUpperInvariant();if($cpu -notin 'A','B'){throw 'cpu must be A or B'};SendJson $c (Parse-Kv (Invoke-Chq @('step','instr',$cpu))) }
elseif($path -eq '/api/v1/input/ioc/pulse' -and $method -eq 'POST'){ $b=ReadJson $c.Request;$iocPort=[int](Prop $b 'port' -1);$mask=[string](Prop $b 'mask' '');$frames=if((Prop $b 'frames' 0)){[int](Prop $b 'frames')}else{3};if($iocPort -lt 0 -or $iocPort -gt 255 -or $mask -notmatch '^[0-9A-Fa-f]{1,2}$' -or $frames -lt 1 -or $frames -gt 100000){throw 'Invalid pulse arguments'};SendJson $c (Parse-Kv (Invoke-Chq @('input','pulse',('{0:X}' -f $iocPort),$mask,[string]$frames))) }
elseif($path -eq '/api/v1/command' -and $method -eq 'POST'){ $b=ReadJson $c.Request;$r=Invoke-SafeCommand ([string](Prop $b 'command' ''));SendJson $c ([ordered]@{ok=$true;raw=$r;lines=@($r -split "`r?`n")}) }
elseif($path -eq '/api/v1/controller/mapping' -and $method -eq 'POST'){ $b=ReadJson $c.Request;$mapDir=Join-Path $root 'controller-mappings';New-Item -ItemType Directory -Path $mapDir -Force|Out-Null;$id=[string](Prop $b 'controllerId' 'controller');$safe=($id -replace '[^A-Za-z0-9_.-]+','-').Trim('-');if([string]::IsNullOrWhiteSpace($safe)){$safe='controller'};if($safe.Length -gt 48){$safe=$safe.Substring(0,48)};$pathOut=Join-Path $mapDir ($safe+'-'+(Get-Date -Format 'yyyyMMdd-HHmmss')+'.json');$b|ConvertTo-Json -Depth 16|Set-Content $pathOut -Encoding UTF8;SendJson $c ([ordered]@{ok=$true;path=$pathOut}) }
elseif($path -eq '/api/v1/evidence/capture' -and $method -eq 'POST'){ $b=ReadJson $c.Request;$name=if((Prop $b 'name' '')){[string](Prop $b 'name')}else{'evidence-'+(Get-Date -Format 'yyyyMMdd-HHmmss')};SendJson $c (New-EvidenceCapture $name) }
elseif($path -eq '/api/v1/export/memory'){ $q=[System.Web.HttpUtility]::ParseQueryString($c.Request.Url.Query);$cpu=([string]$q['cpu']).ToUpperInvariant();if($cpu -notin 'A','B'){throw 'cpu must be A or B'};$a=Hex-U32 $q['start'];$z=Hex-U32 $q['end'];if($z -lt $a){throw 'end before start'};$len=[uint64]$z-$a+1;if($len -gt 1048576){throw 'bridge export is limited to 1 MiB; use smaller ranges until native bulk export lands'};$bytes=New-Object byte[] ([int]$len);for($i=0;$i -lt $bytes.Length;$i++){ $r=Invoke-Chq @('read',$cpu,('{0:X}' -f ($a+$i)),'8');if($r -notmatch 'value=0x([0-9A-Fa-f]+)'){throw "read failed at $('{0:X}' -f ($a+$i)): $r"};$bytes[$i]=[Convert]::ToByte($Matches[1],16)};SendBytes $c $bytes ("chq-$cpu-$('{0:X}' -f $a)-$('{0:X}' -f $z).bin") }
else {Send $c 'Not found' 'text/plain; charset=utf-8' 404}
}catch{
    $handlerError=$_.Exception.Message
    Web-Log "[WEB] request handler error for $method $path : $handlerError" DarkYellow
    # A client can disconnect after headers/body have already started. In that case a second
    # attempt to set ContentLength64 would itself throw and used to terminate the whole host.
    # Best-effort error response only; never allow one request to kill the listener loop.
    try { SendJson $c ([ordered]@{ok=$false;error=$handlerError}) 400 } catch {
        Web-Log "[WEB] could not return error response (connection already submitted/closed): $($_.Exception.Message)" DarkYellow
        try { $c.Response.Abort() } catch {}
    }
}finally{
    $reqSw.Stop();Web-RequestLog $method $path $reqSw.ElapsedMilliseconds
}}}finally{try{$frontProxy.Dispose()}catch{};$listener.Stop();$listener.Close()}
