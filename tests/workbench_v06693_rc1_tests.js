const fs=require('fs'),path=require('path');
const root=path.resolve(__dirname,'..'),read=p=>fs.readFileSync(path.join(root,p),'utf8'),ok=(v,m)=>{if(!v)throw Error(m)};
const current='0.66.9.3-RC3',baseline='0.66.9.2',sha='f3c730e5bc41886189c11e23d4998967f205b085fa172a9c30fc812330227b26';
const cfg=JSON.parse(read('chqdev.json')),version=read('src/version.h'),web=read('Start-ChaseHQWeb.ps1'),main=read('src/main.cpp'),runner=read('tools/chqdev/script_runner.go'),gotest=read('tools/chqdev/runtime_test.go'),full=read('research/scripts/regression/full-regression.chqscript'),focused=read('research/scripts/regression/regression-v06693-rc1.chqscript'),platform=read('research/scripts/regression/regression-platform-current-contracts.chqscript'),coverage=read('docs/REGRESSION_COVERAGE_MAP.md'),validation=read('VALIDATION.md');
ok(cfg.release===current&&cfg.baseline===baseline&&cfg.baselineArchiveSha256===sha,'RC1 carry-forward baseline/current identity wrong');
ok(focused.includes('# regression-scope: historical'),'RC1 script must remain packaged as historical after RC2 supersedes it');
ok(version.includes(current)&&version.includes('chqdev v0.5.5'),'RC1/chqdev v0.5.5 version identity missing');
// Sprite independent decoder compare: native + Script API + HTTP + Workbench UI parity.
ok(main.includes('shadow-compare')&&main.includes('compare_sprite_decoder_4bpp')&&main.includes('sprites.shadow_compare'),'native sprite shadow compare/capability missing');
ok(web.includes("'sprite.shadow.compare'")&&web.includes('/api/v1/sprite/shadow-compare')&&web.includes('spriteShadowCompare')&&web.includes('failOnMismatch')&&web.includes("scope must be visible|slot")&&!web.includes("scope must be visible|all|slot")&&!/\$args\s*\+=/.test(web),'sprite shadow Script API/HTTP/Workbench parity or PowerShell-safe argument handling missing');
// Explicit runtime queries and bounded corpus execution.
ok(web.includes("'query.runtime'")&&web.includes('Evaluate an API-backed query at execution time')&&web.includes("'corpus.run'"),'runtime query/corpus API missing');
ok(web.includes("'getregistry'{return ((Get-LiveRegistryItems)") ,'query.runtime getregistry must use the live flat registry items');
ok(web.includes('corpus-return-')&&web.includes("$nested -eq 'sprite.shadow.compare'")&&web.includes('corpus-')&&web.includes('sprite-shadow.json'),'corpus state restore/structured sprite evidence missing');
// Tuple loops and command headroom must exist in server, browser and Go runners.
ok(web.includes('function Parse-ChqTupleList')&&web.includes('function parseTupleListClient')&&web.includes('$scriptExpandedCommandLimit=2048')&&web.includes('CHQ_SCRIPT_MAX_COMMANDS'),'PowerShell/browser tuple or command-limit support missing');
ok(runner.includes('func parseTupleListGo')&&runner.includes('func scriptCommandLimit() int')&&runner.includes('CHQ_SCRIPT_MAX_COMMANDS'),'Go tuple/command-limit parity missing');
ok(gotest.includes('TestTupleLoopCompilation')&&gotest.includes('TestScriptCommandLimitEnv')&&gotest.includes('TestFullRegressionConsolidatedCommandBudget'),'Go tuple/limit/regression-budget tests missing');
// Evidence bundles must be self-describing and selected runs must actually open in Evidence.
ok(web.includes("schema='chq-run-bundle-manifest-v3'")&&web.includes('Get-FileHash')&&web.includes("schema='chq-run-bundle-index-v2'")&&web.includes("schema='chq-run-bundle-verify-v1'"),'hashed/indexed evidence bundle contract missing');
ok(web.includes('async function openRunInEvidence')&&web.includes("addButton('Open in Evidence',()=>openRunInEvidence(r))")&&web.includes("bundle-index.json"),'selected-run Evidence viewer wiring missing');
ok(web.includes("'script.run.verify'")&&web.includes('Test-RunBundleIntegrity'),'run evidence verification action missing');
// Historical release-tail duplication must be removed without removing scripts or coverage mapping.
for(const x of ['regression-v06690-rc32.chqscript','regression-v06690-rc33.chqscript','regression-v06690-rc34.chqscript','regression-v06690-rc345.chqscript','regression-v06691.chqscript','regression-v06692-rc3.chqscript','regression-v06692-rc4.chqscript','regression-v06692.chqscript']) ok(!full.includes(`include "regression/${x}"`),`historical duplicate still auto-included: ${x}`);
ok(full.includes('regression-platform-current-contracts.chqscript')&&full.includes('regression-v06693-rc3.chqscript')&&!full.includes('include "regression/regression-v06693-rc1.chqscript"')&&!full.includes('include "regression/regression-v06693-rc2.chqscript"'),'consolidated/current tooling regression supersession missing from Full Regression');
ok(platform.includes('legacytrace start')&&platform.includes('checkpoint.save')&&platform.includes('gameplay.registry')&&platform.includes('track.records')&&platform.includes('script.runs'),'current platform contract does not retain release-tail runtime coverage');
ok(coverage.includes('Coverage deliberately unchanged')&&coverage.includes('regression-v06690-rc32.chqscript')&&coverage.includes('regression-v06692.chqscript'),'coverage equivalence map incomplete');
ok(focused.includes('sprite.shadow.compare')&&focused.includes('query.runtime')&&focused.includes('for address,width in')&&focused.includes('corpus.run'),'focused tooling regression missing new contracts');
ok(validation.includes('intentionally')&&validation.includes('qualification')&&validation.includes('deferred')&&validation.includes(current),'deferred qualification state not documented');
console.log('Workbench v0.66.9.3-RC2 research-tooling hardening tests: PASS');
