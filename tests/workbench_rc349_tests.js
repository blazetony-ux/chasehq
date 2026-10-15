const fs=require('fs'),path=require('path'),cp=require('child_process');
const root=path.resolve(__dirname,'..'),read=p=>fs.readFileSync(path.join(root,p),'utf8'),ok=(v,m)=>{if(!v)throw Error(m)};
const current='0.66.9.3-RC3',cfg=JSON.parse(read('chqdev.json')),version=read('src/version.h'),runtime=read('tests/runtime_tests.cpp'),build=read('tools/chqdev/build.go'),configGo=read('tools/chqdev/config.go'),qual=read('tools/chqdev/qualify.go'),bd=read('Build-Debug.bat'),br=read('Build-Release.bat'),doc=read('docs/CHQDEV.md'),rp=read('docs/RELEASE_PROCESS.md'),full=read('research/scripts/regression/full-regression.chqscript'),reg=read('research/scripts/regression/regression-v06690-rc349.chqscript'),catalog=read('research/scripts/SCRIPT_CATALOG.md');
const coverage=read('docs/REGRESSION_COVERAGE_MAP.md'),historicalCatalog=read('research/scripts/SCRIPT_CATALOG.md');
ok(version.includes(current)&&version.includes('chqdev v0.5.5'),'RC3.4.9/chqdev v0.5.1 identity missing');
ok(cfg.release===current&&cfg.baseline==='0.66.9.2'&&cfg.baselineArchiveSha256==='f3c730e5bc41886189c11e23d4998967f205b085fa172a9c30fc812330227b26'&&cfg.focusedRegression.endsWith('regression-v06693-rc3.chqscript'),'RC3.4.9 config/baseline/regression contract wrong');
ok(!Object.prototype.hasOwnProperty.call(cfg,'knownCTestFailure'),'historical knownCTestFailure config exception still present');
ok(!/KnownCTestFailure|known historical failure/.test(build+configGo),'chqdev still tolerates historical CTest failure');
ok(qual.includes('all CTest cases PASS')&&!qual.includes('known-single-failure policy'),'qualification still documents a tolerated CTest failure');
ok((runtime.match(/std::make_unique<chq::Runtime>\(/g)||[]).length===16,'expected 16 heap-allocated Runtime objects');
ok(!/^\s*chq::Runtime\s+[A-Za-z_]/m.test(runtime),'stack-local Runtime object reintroduced');
ok((runtime.match(/tf\.close\(\);/g)||[]).length===6&&runtime.includes('pf.close();')&&runtime.includes('cf.close();'),'Windows input-stream cleanup closes missing');
ok(!/KNOWN_SEGFAULT|KNOWN TEST ISSUE/.test(bd+br)&&bd.includes('if not "%CTEST_RC%"=="0"')&&br.includes('if not "%CTEST_RC%"=="0"'),'legacy build wrappers are not fail-closed for every CTest failure');
ok(/zero tolerated|zero-tolerated|any CTest failure/i.test(doc+rp),'clean CTest policy missing from authoritative docs');
ok(!full.includes('regression-v06690-rc349.chqscript')&&historicalCatalog.includes('regression/regression-v06690-rc349.chqscript')&&historicalCatalog.includes('historical')&&coverage.includes('regression-v06690-rc349.chqscript')&&reg.includes('0.66.9.0-RC3.4.9'),'historical regression-v06690-rc349.chqscript must remain cataloged/mapped while duplicate Full Regression execution stays removed');
for(const f of cfg.currentDocs)ok(read(f).includes(current),`current documentation stale: ${f}`);
for(const f of cfg.buildProcessDocs){const t=read(f).toLowerCase();ok(t.includes('chqdev')&&(t.includes('de facto')||t.includes('authoritative build/release')),`chqdev authority wording missing: ${f}`);ok(t.includes('qualify --fresh'),`fresh qualification wording missing: ${f}`)}
const arch=process.arch==='arm64'?'arm64':'amd64',bin=process.platform==='win32'?`tools/chqdev/bin/chqdev-windows-${arch}.exe`:`tools/chqdev/bin/chqdev-linux-${arch}`;
if(fs.existsSync(path.join(root,bin))){const out=cp.execFileSync(path.join(root,bin),['--root',root,'--json','preflight'],{encoding:'utf8'}),j=JSON.parse(out);ok(j.status==='PASS'&&j.toolVersion==='0.5.5','bundled chqdev v0.5.1 preflight failed')}
console.log('Workbench RC3.4.9 clean CTest / Windows portability tests: PASS');
