const fs=require('fs'),path=require('path');
const root=path.resolve(__dirname,'..');
const read=p=>fs.readFileSync(path.join(root,p),'utf8');
const web=read('Start-ChaseHQWeb.ps1'),video=read('src/video.cpp'),version=read('src/version.h'),full=read('research/scripts/regression/full-regression.chqscript'),validator=read('Validate-Workbench.ps1'),preflight=read('Validate-WindowsPreflight.ps1'),research=read('Start-ChaseHQResearch.ps1'),probe=read('tools/Run-SpriteProbeBatch.ps1');
function ok(v,msg){if(!v)throw new Error(msg)}
ok(version.includes('0.66.9.3-RC3'),'RC3.1 version identity missing');
ok(web.includes('function Set-CourseFollowConfig($config)')&&web.includes('$config.ContainsKey($key)'),'course-follow config bridge fix missing');
ok(!web.includes('function Set-CourseFollowConfig($args)')&&!web.includes('$args.ContainsKey($key)'),'reserved $args regression present');
ok(web.includes("track:['Course Mapping / Live Survey']")&&!web.includes("track:['Live SVG Track View']"),'Track View route regression');
ok(video.includes('SDL_SCANCODE_PAUSE')&&video.includes('SDL_SCANCODE_P')&&video.includes('SDL_KMOD_CTRL'),'Pause/Break + Ctrl+P fallback missing');
ok(web.includes('$surfaceSignature=$roadProcessingSignature')&&web.includes('surfaceStableSignature=$roadProcessingSignature')&&web.includes('courseRecordSignature=$courseRecordSignature'),'road/material vs course-record identity separation missing');
ok(web.includes('surfaceStableSignature')&&web.includes('rawSurfaceRenderVariantCount')&&web.includes('track-surface-segments.csv'),'surface quality model incomplete');
ok(web.includes('New-TrackSpriteCandidates')&&!web.includes('function Group-TrackSprites'),'unsafe proximity grouping reintroduced');
ok(web.includes('UNCLASSIFIED_OBJECT_CANDIDATE')&&web.includes('persistentObjectCandidateTrackCount')&&web.includes('track-object-candidates.csv'),'candidate tracking model incomplete');
ok(!/\$pid\s*=/i.test(web),'PowerShell automatic read-only $PID assignment regression present');
ok(web.includes('$delta=Signed16 (($roadRight-$roadLeft)-band 0xFFFF)')&&web.includes('$latErr=Signed16 (($lateral-$roadCentre)-band 0xFFFF)'),'track signed-16 arithmetic fix missing');
ok(!/\[int16\]\s*\([^\r\n]*-band\s+0xFFFF/i.test(web),'unsafe direct [int16] reinterpret cast remains in masked 16-bit track arithmetic');
ok(web.includes('if($o -is [System.Collections.IDictionary])')&&web.includes('[System.StringComparison]::OrdinalIgnoreCase'),'production OrderedDictionary Prop() support missing');
ok(web.includes("surfaceSegmentsCsv=$surfaceSegmentCsv")&&!web.includes("surfaceSegments=$surfaceSegmentCsv"),'surfaceSegments CSV/count key collision regression');
ok(web.includes("if(trackRecording&&!scriptRunning)sampleTrack()"),'Track View can still inject samples during scripted surveys');
ok(web.includes("duplicateFrameSuppressed")&&web.includes("Prop $prev 'frame' -1")&&web.includes("$point['frame']=$frame"),'same-frame recorder suppression missing');

ok(web.includes('function Get-TrackObjectCandidates([object[]]$sprites)')&&web.includes('$candidateGroups=@(New-TrackSpriteCandidates -sprites $sprites)')&&web.includes('$objectCandidates=@(Get-TrackObjectCandidates -sprites $sprites)'),'object-candidate array preservation fix missing');
ok(web.includes("$point['objectCandidates']=@($objectCandidates)")&&web.includes("sourceMap=[int](Prop $g 'sourceMap' -1)")&&web.includes("centreX=[double](Prop $g 'centreX' 0)"),'object-candidate evidence enrichment is not explicitly preserved');
ok(!web.includes('Assign-TrackObjectIds (New-TrackSpriteCandidates $sprites)'),'fragile nested candidate pipeline binding reintroduced');
ok(preflight.includes('Test-ProtectedVariableName')&&preflight.includes('ScopedItemOptions]::ReadOnly')&&validator.includes('workbench_object_candidate_tests.ps1'),'PowerShell protected-variable/object-candidate runtime gates missing');
for(const [name,text] of [['Start-ChaseHQWeb.ps1',web],['Start-ChaseHQResearch.ps1',research],['Run-SpriteProbeBatch.ps1',probe]]){
  ok(!/\$(?:args|pid|matches|profile)\s*=/i.test(text),`${name} reintroduces unsafe PowerShell automatic-variable assignment`);
}
ok(web.includes("classification=$targetClass")&&web.includes("'UNCLASSIFIED_TARGET_RECORD'"),'Stage-1 target semantic scope guard missing');
ok(web.includes("schema='chq-course-map-v2'")&&web.includes('track-map-summary.json'),'mapping export v2 incomplete');
ok(full.includes('regression-v06690-rc30-course-mapping-live-survey.chqscript')&&full.includes('regression-v06690-rc31-course-mapping-quality.chqscript'),'RC3.0/RC3.1 permanent regression wiring incomplete');
ok(research.includes('$nativeLaunchArgs = @(')&&research.includes('$nativeLaunchArgs += @($EmulatorArgs)')&&research.includes("$argLine = (($nativeLaunchArgs | ForEach-Object { Quote-Arg $_ }) -join ' ')")&&!research.includes("$argLine = (($EmulatorArgs | ForEach-Object { Quote-Arg $_ }) -join ' ')")&&!research.includes('$emulatorArgs += $EmulatorArgs'),'research launcher argument-vector contract missing');
console.log('Workbench RC3.1 course mapping quality + carried-forward fixes: PASS');
