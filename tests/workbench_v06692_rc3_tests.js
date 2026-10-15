const fs=require('fs'),path=require('path');
const root=path.resolve(__dirname,'..'),read=p=>fs.readFileSync(path.join(root,p),'utf8'),ok=(v,m)=>{if(!v)throw Error(m)};
const current='0.66.9.3-RC3',base='0.66.9.2',sha='f3c730e5bc41886189c11e23d4998967f205b085fa172a9c30fc812330227b26';
const cfg=JSON.parse(read('chqdev.json')),version=read('src/version.h'),web=read('Start-ChaseHQWeb.ps1'),launcher=read('Start-ChaseHQ.ps1'),full=read('research/scripts/regression/full-regression.chqscript'),reg=read('research/scripts/regression/regression-v06692-rc3.chqscript'),catalog=read('research/scripts/SCRIPT_CATALOG.md'),notes=read('docs/chasehq/discoveries/STAGE1_TRACK_RECORD_0.66.9.2-RC3.md'),goPreflight=read('tools/chqdev/preflight.go'),goPackage=read('tools/chqdev/package.go'),validation=read('VALIDATION.md'),quick=read('QUICK_START.txt'),changelog=read('CHANGELOG.md');
const coverage=read('docs/REGRESSION_COVERAGE_MAP.md'),historicalCatalog=read('research/scripts/SCRIPT_CATALOG.md');
ok(version.includes(current)&&version.includes('chqdev v0.5.5'),'RC3 version/platform identity missing');
const goMain=read('tools/chqdev/main.go');ok(goMain.includes('toolVersion')&&goMain.includes('version-specific qualification wrappers are not shipped')&&!goMain.includes('ChaseHQ-Native chqdev v0.4'),'chqdev help/version release policy is stale');
ok(cfg.release===current&&cfg.baseline===base&&cfg.baselineArchiveSha256===sha,'RC3 exact baseline identity wrong');
ok(!full.includes('regression-v06692-rc3.chqscript')&&historicalCatalog.includes('regression/regression-v06692-rc3.chqscript')&&historicalCatalog.includes('historical')&&coverage.includes('regression-v06692-rc3.chqscript')&&reg.includes('0.66.9.2-RC3'),'historical regression-v06692-rc3.chqscript must remain cataloged/mapped while duplicate Full Regression execution stays removed');
for(const x of ['featureType','roadProcessingSelector','paletteTransitionSelector','environmentTransitionSelector','featureParameter','featureParameterSemantic','courseRecordSignature','roadProcessingSignature','roadMaterialCandidateSignature'])ok(web.includes(x),`Track semantic field missing: ${x}`);
ok(web.includes('legacyFieldAliasesDeprecated=$true')&&web.includes('courseSurfaceSignatureDeprecated=$true'),'compatibility deprecation markers missing');
ok(web.includes('$surfaceSignature=$roadProcessingSignature')&&web.includes('surfaceStableSignature=$roadProcessingSignature'),'road/material signature still tied to unrelated course bytes');
ok(web.includes("if($lop -eq 'reset'){$script:LastLegacyTracePath=$null}")&&web.includes("Collect-LegacyTraceArtifact (Join-Path $runRoot 'artifacts') $started"),'legacytrace run-isolation guard missing');
ok(web.includes('LastWriteTimeUtc')&&web.includes('notBefore.UtcDateTime'),'legacytrace freshness check missing');
ok(launcher.includes('$projectBuildVersion')&&!launcher.includes("build = '0.66.9.0'"),'active-session build identity still hard-coded');
ok(web.includes('native-startup-phase.txt')&&web.includes('diagnostic-state.json')&&web.includes('native-status is authoritative live state'),'diagnostic live/startup-state distinction missing');
ok(web.includes("mode='ATTRACT MODE'")&&web.includes("mode='PLAYER GAMEPLAY'"),'checkpoint role distinction missing');
ok(!full.includes('regression-v06692-rc3.chqscript')&&historicalCatalog.includes('regression/regression-v06692-rc3.chqscript')&&historicalCatalog.includes('historical')&&coverage.includes('regression-v06692-rc3.chqscript')&&reg.includes('0.66.9.2-RC3'),'historical regression-v06692-rc3.chqscript must remain cataloged/mapped while duplicate Full Regression execution stays removed');
ok(notes.includes('+7')&&notes.includes('route-arrow')&&notes.includes('polymorphic'),'track-record evidence documentation missing');
for(const f of fs.readdirSync(root)){ok(!/^Qualify-.*\.bat$/i.test(f)&&!/^BUILD_NOTES_/i.test(f)&&!/^HOTFIX_NOTES_/i.test(f)&&!/^RELEASE_NOTES_/i.test(f)&&!/^VALIDATION_/i.test(f)&&!/^PACKAGE_MANIFEST_/i.test(f),'obsolete root release artifact present: '+f)}
ok(fs.existsSync(path.join(root,'CHANGELOG.md'))&&fs.existsSync(path.join(root,'VALIDATION.md')),'consolidated release docs missing');
ok(goPreflight.includes('checkReleaseRootHygiene')&&goPreflight.includes('Qualify-*.bat')&&goPreflight.includes('PACKAGE_MANIFEST_*'),'release-root hygiene preflight missing');
ok(goPackage.includes('const packageManifestJSON = "PACKAGE_MANIFEST.json"')&&goPackage.includes('const packageManifestTXT = "PACKAGE_MANIFEST.txt"'),'generic package manifest policy missing');
ok(!fs.readdirSync(path.join(root,'tools')).some(x=>/^Invoke-RC.*Qualification\.ps1$/i.test(x)),'version-specific qualification PowerShell wrapper returned');
ok(validation.includes('.\\chqdev.bat qualify --fresh')&&quick.includes('.\\chqdev.bat qualify --fresh'),'canonical Windows qualification command drifted');
ok(changelog.includes('single canonical human-readable release history')&&changelog.includes('v0.66.9.2-RC3'),'consolidated changelog contract missing');

const readyIndex=web.indexOf("fetch('/api/v1/frontend-ready'");
const libraryIndex=web.indexOf('await loadScriptLibrary()',readyIndex);
const historyIndex=web.indexOf('await refreshAuthoritativeRunHistory()',readyIndex);
const finalRefreshIndex=web.indexOf('await refresh();restart();refreshPreview();restartPreview();',readyIndex);
ok(readyIndex>0&&libraryIndex>readyIndex&&historyIndex>readyIndex&&finalRefreshIndex>readyIndex,'frontend readiness is still blocked behind heavyweight hydration');
ok(web.includes("const bootstrapHealth=JSON.parse(await api('/api/health'))")&&web.includes("phase:'core-ready'")&&web.includes('startup-hydration'),'frontend core-readiness / background-hydration split missing');
ok(launcher.includes('$frontDeadline=(Get-Date).AddSeconds(60)'),'frontend bootstrap safety deadline must remain 60 seconds');

console.log('Workbench v0.66.9.2-RC3 track semantics/evidence/bootstrap readiness tests: PASS');
