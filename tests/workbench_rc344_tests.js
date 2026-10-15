const fs=require('fs'),path=require('path'),cp=require('child_process');
const root=path.resolve(__dirname,'..'),read=p=>fs.readFileSync(path.join(root,p),'utf8'),ok=(v,m)=>{if(!v)throw Error(m)};
const current='0.66.9.3-RC3';
const version=read('src/version.h'),cfg=JSON.parse(read('chqdev.json')),doc=read('docs/CHQDEV.md'),full=read('research/scripts/regression/full-regression.chqscript'),reg=read('research/scripts/regression/regression-v06690-rc344.chqscript'),catalog=read('research/scripts/SCRIPT_CATALOG.md'),rp=read('docs/RELEASE_PROCESS.md');
const coverage=read('docs/REGRESSION_COVERAGE_MAP.md'),historicalCatalog=read('research/scripts/SCRIPT_CATALOG.md');
ok(version.includes(current)&&read('tools/chqdev/main.go').includes('const toolVersion'),'RC3.4.4 chqdev transition carry-forward identity missing');
ok(cfg.schema==='chqdev-config-v1'&&cfg.release===current,'chqdev current config identity incorrect');
for(const f of ['tools/chqdev/go.mod','tools/chqdev/main.go','tools/chqdev/preflight.go','tools/chqdev/build.go','tools/chqdev/package.go','chqdev.bat','chqdev.sh','tools/chqdev/bin/chqdev-windows-amd64.exe','tools/chqdev/bin/chqdev-linux-amd64'])ok(fs.existsSync(path.join(root,f)),`missing chqdev surface: ${f}`);
ok(doc.includes('doctor')&&doc.includes('preflight')&&doc.includes('package')&&doc.includes('verify')&&doc.includes('--json')&&/CRLF/.test(doc)&&/LF/.test(doc),'chqdev documentation contract missing');
ok(!full.includes('regression-v06690-rc344.chqscript')&&historicalCatalog.includes('regression/regression-v06690-rc344.chqscript')&&historicalCatalog.includes('historical')&&coverage.includes('regression-v06690-rc344.chqscript')&&reg.includes('0.66.9.0-RC3.4.4'),'historical regression-v06690-rc344.chqscript must remain cataloged/mapped while duplicate Full Regression execution stays removed');
ok(rp.includes('chqdev')&&rp.includes('compatibility')&&rp.includes('authoritative'),'RC3.4.4 transition policy was not carried forward');
const arch=process.arch==='arm64'?'arm64':'amd64';const bin=process.platform==='win32'?`tools/chqdev/bin/chqdev-windows-${arch}.exe`:`tools/chqdev/bin/chqdev-linux-${arch}`;
if(fs.existsSync(path.join(root,bin))){const out=cp.execFileSync(path.join(root,bin),['--root',root,'--json','preflight'],{encoding:'utf8'});const j=JSON.parse(out);ok(j.status==='PASS'&&j.command==='preflight','bundled chqdev native preflight failed')}
console.log('Workbench RC3.4.4 chqdev v0.1 transition tests: PASS');
