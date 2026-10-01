const fs=require('fs'),vm=require('vm'),assert=require('assert'),path=require('path');
const source=fs.readFileSync(path.join(__dirname,'../Start-ChaseHQWeb.ps1'),'utf8');
const pure=source.slice(source.indexOf('function deriveReleaseVersion('),source.indexOf('function getRunHistory('));
const refresh=source.slice(source.indexOf('async function refreshAuthoritativeRunHistory('),source.indexOf('function saveRunHistory('));
const elements={scriptHistorySession:{value:'',innerHTML:''},scriptProgress:{textContent:''}};
const queries=[];
const context={URLSearchParams,console,$:id=>elements[id],escapeHtml:x=>x,renderRunHistory:()=>{},currentHistoryFilters:()=>({session:elements.scriptHistorySession.value}),api:async url=>{
  queries.push(url);
  if(url.endsWith('limit=1'))return JSON.stringify({currentSession:'active',runs:[{session:'old'}]});
  assert(url.includes('session=active'),'Current session must be queried server-side');
  return JSON.stringify({runs:[{serverKey:'active/001',version:'0.66.9.0-RC2.8',session:'active'}]});
}};
vm.createContext(context);
vm.runInContext('let authoritativeRunHistory=[],defaultHistorySession="",historyDefaultApplied=false;'+pure+refresh,context);
(async()=>{
  const result=await vm.runInContext('refreshAuthoritativeRunHistory()',context);
  assert.equal(result[0].version,'0.66.9.0-RC2.8');
  assert.equal(result[0].id,'active/001');
  assert.equal(elements.scriptHistorySession.value,'active');
  assert.equal(queries.length,2);
  const legacy=vm.runInContext("normalizeLegacyRun({version:'0.66.9.0',runPath:'ChaseHQ-Native-v0.66.9.0-RC2.6.1'})",context);
  assert.equal(legacy.releaseVersion,'0.66.9.0','Version must not be guessed from directory names');
  console.log('Workbench current-session query, run identity and full version: PASS');
})().catch(e=>{console.error(e);process.exitCode=1});
