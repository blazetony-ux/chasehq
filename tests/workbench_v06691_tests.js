const fs=require('fs'),path=require('path'),cp=require('child_process');
const root=path.resolve(__dirname,'..'),read=p=>fs.readFileSync(path.join(root,p),'utf8'),ok=(v,m)=>{if(!v)throw Error(m)};
const current='0.66.9.3-RC3',cfg=JSON.parse(read('chqdev.json')),version=read('src/version.h'),main=read('tools/chqdev/main.go'),pre=read('tools/chqdev/preflight.go'),doc=read('docs/RELEASE_GOVERNANCE.md'),rp=read('docs/RELEASE_PROCESS.md'),cq=read('QUICK_START.txt'),local=read('BUILD_LOCAL_SETUP.md'),chq=read('docs/CHQDEV.md'),handbook=read('research/knowledge/handbook.json'),knowledge=read('research/knowledge/documentation.json'),full=read('research/scripts/regression/full-regression.chqscript'),reg=read('research/scripts/regression/regression-v06691.chqscript'),catalog=read('research/scripts/SCRIPT_CATALOG.md');
const coverage=read('docs/REGRESSION_COVERAGE_MAP.md'),historicalCatalog=read('research/scripts/SCRIPT_CATALOG.md');
ok(version.includes(current)&&version.includes('chqdev v0.5.5'),'v0.66.9.1/chqdev v0.5.1 identity missing');
ok(cfg.release===current&&cfg.baseline==='0.66.9.2'&&cfg.baselineArchiveSha256==='f3c730e5bc41886189c11e23d4998967f205b085fa172a9c30fc812330227b26','stable release baseline identity wrong');
ok(cfg.focusedRegression.endsWith('regression-v06693-rc3.chqscript'),'current focused regression not configured');
ok(main.includes('const toolVersion = "0.5.5"'),'chqdev v0.5.1 missing');
ok(pre.includes('checkReleaseProcessContract')&&pre.includes('release-process-contract')&&pre.includes('forbidden legacy canonical claim'),'release-process documentation guard missing');
for(const [name,t] of Object.entries({governance:doc,releaseProcess:rp,quickStart:cq,localSetup:local,chqdev:chq,handbook})){
 const x=t.toLowerCase();
 ok(x.includes('chqdev')&&x.includes('qualify --fresh'),`${name}: canonical chqdev fresh qualification missing`);
 ok(x.includes('zero tolerated ctest')||x.includes('any ctest failure is fatal'),`${name}: strict CTest policy missing`);
 ok(x.includes('deterministic')&&x.includes('archive'),`${name}: deterministic archive contract missing`);
 ok(x.includes('full regression'),`${name}: Full Regression contract missing`);
 ok((x.includes('powershell/bat')||x.includes('shell'))&&x.includes('compatibility'),`${name}: shell compatibility-only rule missing`);
}
for(const stale of ['canonical windows build path is build-debug.bat','normal windows development entry point is build-debug.bat','build only through build-debug.bat','known windows test exception']){
 for(const [name,t] of Object.entries({governance:doc,releaseProcess:rp,quickStart:cq,localSetup:local,chqdev:chq,handbook})) ok(!t.toLowerCase().includes(stale),`${name}: stale canonical claim ${stale}`);
}
ok(handbook.includes(current)&&knowledge.includes(current),'machine-readable documentation identity stale');
ok(!full.includes('regression-v06691.chqscript')&&historicalCatalog.includes('regression/regression-v06691.chqscript')&&historicalCatalog.includes('historical')&&coverage.includes('regression-v06691.chqscript')&&reg.includes('0.66.9.1'),'historical regression-v06691.chqscript must remain cataloged/mapped while duplicate Full Regression execution stays removed');
for(const f of cfg.currentDocs)ok(read(f).includes(current),`current documentation stale: ${f}`);
const arch=process.arch==='arm64'?'arm64':'amd64',bin=process.platform==='win32'?`tools/chqdev/bin/chqdev-windows-${arch}.exe`:`tools/chqdev/bin/chqdev-linux-${arch}`;
if(fs.existsSync(path.join(root,bin))){const out=cp.execFileSync(path.join(root,bin),['--root',root,'--json','preflight'],{encoding:'utf8'}),j=JSON.parse(out);ok(j.status==='PASS'&&j.toolVersion==='0.5.5','bundled chqdev v0.5.1 preflight failed')}
console.log('Workbench v0.66.9.1 release-governance/documentation tests: PASS');
