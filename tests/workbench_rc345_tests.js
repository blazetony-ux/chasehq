const fs=require('fs'),path=require('path'),cp=require('child_process');
const root=path.resolve(__dirname,'..'),read=p=>fs.readFileSync(path.join(root,p),'utf8'),ok=(v,m)=>{if(!v)throw Error(m)};
const cfg=JSON.parse(read('chqdev.json')),version=read('src/version.h'),pkg=read('tools/chqdev/package.go'),runtime=read('tools/chqdev/runtime.go'),regctl=read('tools/chqdev/regression.go'),runner=read('tools/chqdev/script_runner.go'),qual=read('tools/chqdev/qualify.go'),doc=read('docs/CHQDEV.md'),rp=read('docs/RELEASE_PROCESS.md'),full=read('research/scripts/regression/full-regression.chqscript'),reg=read('research/scripts/regression/regression-v06690-rc345.chqscript'),catalog=read('research/scripts/SCRIPT_CATALOG.md');
const coverage=read('docs/REGRESSION_COVERAGE_MAP.md'),historicalCatalog=read('research/scripts/SCRIPT_CATALOG.md');
ok(version.includes(cfg.release),'current native/config identity mismatch');
ok(pkg.includes('deterministicZipTime')&&pkg.includes('reproducibility')&&pkg.includes('SetModTime')&&pkg.includes('deterministicMode'),'RC3.4.5 deterministic package implementation missing');
ok(runtime.includes('/api/health')&&runtime.includes('/api/v1/mobile/pairing')&&runtime.includes('/api/v1/script/runs?limit=1'),'RC3.4.5 runtime readiness coverage missing');
ok((regctl+runner).includes('/api/v1/script')&&(regctl+runner).includes('finalizeRun')&&regctl.includes('focused')&&regctl.includes('full'),'RC3.4.5 regression orchestration missing');
ok(qual.includes('chqdev-qualification-v2')&&qual.includes('promotable')&&qual.includes('qualification.json'),'RC3.4.5 qualification state machine missing');
ok(/de facto/i.test(doc)&&/authoritative build\/release/i.test(doc)&&/compatibility\/fallback/i.test(doc),'chqdev authority policy missing');
ok(/de facto/i.test(rp)&&/deterministic/i.test(rp)&&/cross-host/i.test(rp),'release-process deterministic policy missing');
ok(!full.includes('regression-v06690-rc345.chqscript')&&historicalCatalog.includes('regression/regression-v06690-rc345.chqscript')&&historicalCatalog.includes('historical')&&coverage.includes('regression-v06690-rc345.chqscript')&&reg.includes('0.66.9.0-RC3.4.5'),'historical regression-v06690-rc345.chqscript must remain cataloged/mapped while duplicate Full Regression execution stays removed');
for(const f of cfg.buildProcessDocs){const t=read(f).toLowerCase();ok(t.includes('chqdev')&&(t.includes('de facto')||t.includes('authoritative build/release')),`chqdev authority wording missing: ${f}`)}
const arch=process.arch==='arm64'?'arm64':'amd64',bin=process.platform==='win32'?`tools/chqdev/bin/chqdev-windows-${arch}.exe`:`tools/chqdev/bin/chqdev-linux-${arch}`;
if(fs.existsSync(path.join(root,bin))){const out=cp.execFileSync(path.join(root,bin),['--root',root,'--json','preflight'],{encoding:'utf8'}),j=JSON.parse(out);ok(j.status==='PASS','bundled chqdev current preflight failed')}
console.log('Workbench RC3.4.5 deterministic/authoritative-controller carry-forward tests: PASS');
