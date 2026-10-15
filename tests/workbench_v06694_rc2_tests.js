const fs=require('fs'),path=require('path');
const root=path.resolve(__dirname,'..'),read=p=>fs.readFileSync(path.join(root,p),'utf8'),ok=(v,m)=>{if(!v)throw Error(m)};
const release='0.66.9.4-RC2';
const cfg=JSON.parse(read('chqdev.json')),version=read('src/version.h'),web=read('Start-ChaseHQWeb.ps1'),main=read('src/main.cpp'),video=read('src/video.cpp'),header=read('src/video.h'),api=JSON.parse(read('research/schema/api-actions.json')),focused=read('research/scripts/regression/regression-v06694-rc2.chqscript'),full=read('research/scripts/regression/full-regression.chqscript'),audit=read('docs/DOCUMENTATION_AUDIT_0.66.9.4-RC2.md');
ok(cfg.release===release&&cfg.baseline==='0.66.9.3','release/baseline identity mismatch');
ok(version.includes(release)&&version.includes('Native Reconstruction Lab')&&version.includes('Switchable Sprite Backend'),'native RC2 identity missing');
for(const a of ['backend.sprite.get','backend.sprite.set','backend.sprite.reset-verification','sprite.map-override','sprite.map-override.clear','text.charset.export']){
  ok(web.includes(`'${a}'`),`Workbench Script API missing ${a}`);ok(api.actions[a],`authoritative api-actions.json missing ${a}`);
}
ok(header.includes('enum class SpriteBackend { Original, Shadow, Native }'),'sprite backend enum missing');
ok(video.includes('draw_runtime_sprites_native')&&video.includes('draw_runtime_sprites_selected'),'independent native live sprite path missing');
ok(video.includes('sprite_backend_compared_frames_')&&video.includes('sprite_backend_first_mismatch_frame_'),'live shadow first-divergence counters missing');
ok(main.includes('backend sprites status|set original|shadow|native|reset-verify')&&main.includes('map-override SOURCE_MAP TARGET_MAP'),'native backend/map substitution command help missing');
ok(web.includes('Native Reconstruction Lab')&&web.includes('id="nativeSpriteOriginal"')&&web.includes('id="nativeSpriteShadow"')&&web.includes('id="nativeSpriteNative"'),'Native Lab UI missing backend controls');
ok(web.includes('.hud-layout-card{grid-column:span 2;min-width:560px}')&&web.includes('class="game-lab-card hud-layout-card"'),'HUD Layout responsive width fix missing');
ok(web.includes('Export-Tc0100TextCharset')&&web.includes('characterCount=256')&&web.includes("'atlas.png'")&&web.includes("'character-ram.bin'"),'complete charset export missing');
ok(fs.existsSync(path.join(root,'checkpoints/stage2-gameplay-7500.chqstate'))&&fs.existsSync(path.join(root,'checkpoints/stage2-enemy-10514.chqstate')),'Stage 2 checkpoints missing');
ok(focused.includes('stage2-gameplay-7500.chqstate')&&focused.includes('stage2-enemy-10514.chqstate')&&focused.includes('backend.sprite.set mode=native')&&focused.includes('text.charset.export'),'focused RC2 regression missing cross-stage/native/font gates');
ok(full.includes('regression-v06694-rc2.chqscript'),'Full Regression does not include RC2 focused gate');
ok(audit.includes('zero tolerated CTest failures')&&audit.includes('deterministic')&&audit.includes('Full Regression')&&audit.includes('SHADOW VERIFIED'),'RC2 audit does not retain release/evidence contract');
console.log('Workbench v0.66.9.4-RC2 native/asset tests: PASS');
