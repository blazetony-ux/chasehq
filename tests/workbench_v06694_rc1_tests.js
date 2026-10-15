const fs=require('fs'),path=require('path');
const root=path.resolve(__dirname,'..'),read=p=>fs.readFileSync(path.join(root,p),'utf8'),ok=(v,m)=>{if(!v)throw Error(m)};
const release='0.66.9.4-RC1';
const cfg=JSON.parse(read('chqdev.json')),version=read('src/version.h'),web=read('Start-ChaseHQWeb.ps1'),main=read('src/main.cpp'),video=read('src/video.cpp'),hud=read('src/hud_layout.h'),api=JSON.parse(read('research/schema/api-actions.json')),focused=read('research/scripts/regression/regression-v06694-rc1.chqscript'),full=read('research/scripts/regression/full-regression.chqscript'),audit=read('docs/DOCUMENTATION_AUDIT_0.66.9.4-RC1.md');
ok(cfg.release===release&&cfg.baseline==='0.66.9.3','release/baseline identity mismatch');
ok(version.includes(release)&&version.includes('Experimental Police Beacon Override')&&version.includes('Semantic HUD Layout'),'native RC1 identity missing');
for(const a of ['game.beacon.get','game.beacon.override','hud.layout.get','hud.layout.set','hud.layout.reset','hud.layout.preset']){
  ok(web.includes(`'${a}'`),`Workbench Script API missing ${a}`);ok(api.actions[a],`authoritative api-actions.json missing ${a}`);
}
ok(main.includes('beacon override authentic|on|off')&&main.includes('hud status|set|reset|preset'),'native command help missing beacon/HUD controls');
ok(main.includes('0x101854')&&main.includes('BeaconOverrideMode::ForceOn')&&main.includes('hud_custom'),'native beacon/HUD status integration missing');
ok(hud.includes('HudElement::HighScore')&&hud.includes('HudElement::Time')&&hud.includes('HudElement::Distance')&&hud.includes('HudElement::Turbo')&&hud.includes('hud_layout_apply_preset'),'semantic HUD mapping/presets missing');
ok(video.includes('hud_element_for_text_cell(tx, ty)')&&video.includes('presentation.dx')&&video.includes('presentation.dy'),'presentation-only text-layer HUD transform missing');
ok(video.includes('"hudPresentation"')||video.includes('hudPresentation'),'snapshot HUD provenance metadata missing');
ok(web.includes('id="gameBeaconAuthentic"')&&web.includes('id="gameBeaconOn"')&&web.includes('id="gameBeaconOff"'),'Game Lab beacon controls missing');
ok(web.includes('data-hud-row="high-score"')&&web.includes('data-hud-row="time"')&&web.includes('data-hud-row="distance"')&&web.includes('data-hud-row="turbo"'),'Game Lab semantic HUD rows missing');
ok(web.includes("gameAction('beacon.override'")&&web.includes("gameAction('hud.layout.set'")&&web.includes("gameAction('hud.layout.preset'"),'Game Lab JS wiring missing');
ok(focused.includes('game.beacon.override mode=on')&&focused.includes('hud.layout.set element=high-score')&&focused.includes('sprite.layer-shadow.compare failOnMismatch=true'),'focused RC1 regression missing new controls/carry-forward shadow gate');
ok(full.includes('regression-v06694-rc1.chqscript'),'Full Regression does not include RC1 focused capability gate');
ok(audit.includes('zero tolerated CTest failures')&&audit.includes('deterministic archive')&&audit.includes('Full Regression'),'RC1 audit does not retain release contract');
console.log('Workbench v0.66.9.4-RC1 beacon/HUD tests: PASS');
