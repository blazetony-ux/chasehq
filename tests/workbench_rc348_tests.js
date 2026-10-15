const fs=require('fs'),path=require('path'),cp=require('child_process');
const root=path.resolve(__dirname,'..'),read=p=>fs.readFileSync(path.join(root,p),'utf8'),ok=(v,m)=>{if(!v)throw Error(m)};
const current='0.66.9.3-RC3',cfg=JSON.parse(read('chqdev.json')),version=read('src/version.h'),main=read('tools/chqdev/main.go'),runner=read('tools/chqdev/script_runner.go'),regctl=read('tools/chqdev/regression.go'),qual=read('tools/chqdev/qualify.go'),doc=read('docs/CHQDEV.md'),rp=read('docs/RELEASE_PROCESS.md'),full=read('research/scripts/regression/full-regression.chqscript'),reg=read('research/scripts/regression/regression-v06690-rc348.chqscript'),catalog=read('research/scripts/SCRIPT_CATALOG.md'),gotest=read('tools/chqdev/runtime_test.go');
const coverage=read('docs/REGRESSION_COVERAGE_MAP.md'),historicalCatalog=read('research/scripts/SCRIPT_CATALOG.md');
ok(version.includes(current)&&version.includes('chqdev v0.5.5'),'RC3.4.8/chqdev v0.4.1 identity missing');
ok(cfg.release===current,'current config identity regressed while carrying RC3.4.8 forward');
ok(main.includes('const toolVersion = "0.5.5"')&&main.includes('case "--fresh"'),'chqdev v0.4.x/fresh command surface missing');
ok(runner.includes('defaults := scriptDefaultVars(src)')&&runner.includes('g.resolveQueries(src, defaults)')&&runner.includes('compileScriptLines(strings.Split(src, "\\n"), defaults)'),'Go runner does not seed query/compile stages from script parameter defaults');
ok(gotest.includes('TestGoScriptRunnerParameterDefaults')&&gotest.includes('echo frames=4')&&gotest.includes('api control.run-frames frames=4'),'real parameter-default runner regression missing');
ok(regctl.includes('go-script-console-v2')&&regctl.includes('runPath')&&regctl.includes('bundlePath'),'regression failure diagnostics/run-path surface missing');
ok(qual.includes('reportFailureDetail')&&qual.includes('reportFailureDetail(fr')&&qual.includes('reportFailureDetail(full'),'fresh qualifier still collapses nested regression errors');
ok(/parameter default/i.test(doc)&&/failure diagnostic/i.test(doc),'CHQDEV RC3.4.8 parameter-default/failure-diagnostic documentation missing');
ok(/parameter default/i.test(rp)&&/failure diagnostic/i.test(rp),'release-process RC3.4.8 contract missing');
ok(!full.includes('regression-v06690-rc348.chqscript')&&historicalCatalog.includes('regression/regression-v06690-rc348.chqscript')&&historicalCatalog.includes('historical')&&coverage.includes('regression-v06690-rc348.chqscript')&&reg.includes('0.66.9.0-RC3.4.8'),'historical regression-v06690-rc348.chqscript must remain cataloged/mapped while duplicate Full Regression execution stays removed');
for(const f of cfg.currentDocs)ok(read(f).includes(current),`current documentation stale: ${f}`);
for(const f of cfg.buildProcessDocs){const t=read(f).toLowerCase();ok(t.includes('chqdev')&&(t.includes('de facto')||t.includes('authoritative build/release')),`chqdev authority wording missing: ${f}`);ok(t.includes('qualify --fresh'),`fresh qualification wording missing: ${f}`)}
const arch=process.arch==='arm64'?'arm64':'amd64',bin=process.platform==='win32'?`tools/chqdev/bin/chqdev-windows-${arch}.exe`:`tools/chqdev/bin/chqdev-linux-${arch}`;
if(fs.existsSync(path.join(root,bin))){const out=cp.execFileSync(path.join(root,bin),['--root',root,'--json','preflight'],{encoding:'utf8'}),j=JSON.parse(out);ok(j.status==='PASS'&&j.toolVersion==='0.5.5','bundled current chqdev preflight failed')}
console.log('Workbench RC3.4.8 chqdev v0.4.1 parameter-default / failure-diagnostic tests: PASS');
