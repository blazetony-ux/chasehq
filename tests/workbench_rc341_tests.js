const fs=require('fs'),path=require('path');
const root=path.resolve(__dirname,'..'),read=p=>fs.readFileSync(path.join(root,p),'utf8'),ok=(v,m)=>{if(!v)throw Error(m)};
const version=read('src/version.h'),preflight=read('Validate-WindowsPreflight.ps1'),goPreflight=read('tools/chqdev/preflight.go'),full=read('research/scripts/regression/full-regression.chqscript'),reg=read('research/scripts/regression/regression-v06690-rc341.chqscript'),vr=read('Validate-Release.ps1'),rp=read('docs/RELEASE_PROCESS.md'),handover=read('docs/CHATGPT_HANDOVER_PROMPT.md');
const coverage=read('docs/REGRESSION_COVERAGE_MAP.md'),historicalCatalog=read('research/scripts/SCRIPT_CATALOG.md');
ok(version.includes('0.66.9.3-RC3'),'current release identity missing while testing RC3.4.1 release-guard carry-forward');
ok(preflight.includes('Windows script ASCII policy self-test')&&preflight.includes('Windows-owned script ASCII safety')&&preflight.includes('[char]0x00B7'),'Windows ASCII fail-closed/self-test wiring missing');
ok(goPreflight.includes('scanWindowsASCII')&&goPreflight.includes('windows-script-ascii'),'authoritative chqdev ASCII preflight missing');
ok(!fs.readdirSync(root).some(x=>/^Qualify-.*\.bat$/i.test(x)),'version-specific qualification wrapper returned');
ok(!full.includes('regression-v06690-rc341.chqscript')&&historicalCatalog.includes('regression/regression-v06690-rc341.chqscript')&&historicalCatalog.includes('historical')&&coverage.includes('regression-v06690-rc341.chqscript')&&reg.includes('0.66.9.0-RC3.4.1'),'historical regression-v06690-rc341.chqscript must remain cataloged/mapped while duplicate Full Regression execution stays removed');
ok(vr.includes('RC3.4.1 Windows-script ASCII fail-closed policy'),'release validator does not enforce new ASCII policy');
ok(rp.includes('non-ASCII')&&rp.includes('U+00B7')&&rp.includes('MUST NOT hand off a candidate archive unless the final frozen source bytes pass'),'release-process ASCII incident rule missing');
ok(handover.includes('v0.66.9.3-RC3')&&/authoritative continuation/i.test(handover)&&/Immediate next task/i.test(handover),'ChatGPT handover/validator contract is stale');
const excluded=new Set(['SDL','roms','out','evidence']); const exts=new Set(['.ps1','.psm1','.psd1','.bat','.cmd']); const failures=[];
function walk(dir){for(const de of fs.readdirSync(dir,{withFileTypes:true})){if(de.isDirectory()&&dir===root&&excluded.has(de.name))continue;const p=path.join(dir,de.name);if(de.isDirectory())walk(p);else if(exts.has(path.extname(de.name).toLowerCase())){let b=fs.readFileSync(p);if(b.length>=3&&b[0]===0xef&&b[1]===0xbb&&b[2]===0xbf)b=b.subarray(3);for(let i=0;i<b.length;i++){if(b[i]>0x7f){failures.push(path.relative(root,p)+': byte '+i+' = 0x'+b[i].toString(16));break;}}}}}
walk(root);ok(failures.length===0,'non-ASCII Windows-owned script source: '+failures.join('; '));
const ascii=Buffer.from("Write-Host 'ok'\r\n",'utf8'),bad=Buffer.from("Write-Host 'bad·fixture'\r\n",'utf8'); ok(![...ascii].some(x=>x>127)&&[...bad].some(x=>x>127),'portable ASCII detector self-test failed');
console.log('Workbench RC3.4.1 release-guard / Windows ASCII policy tests: PASS');
