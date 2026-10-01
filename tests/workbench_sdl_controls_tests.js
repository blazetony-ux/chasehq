const fs=require('fs'),path=require('path'),vm=require('vm'),assert=require('assert');
const source=fs.readFileSync(path.join(__dirname,'../Start-ChaseHQWeb.ps1'),'utf8');
const bindings=source.slice(source.indexOf('async function sdlControl('),source.indexOf("$('regsA').onclick="));
const elements={};for(const id of ['console','sdlShow','sdlHide','sdlMinimize','sdlRestore','sdlPause','sdlResume','sdlStep','sdlFullscreen','sdlTop','sdlScale','sdlSnapshot','sdlWindowState'])elements[id]={dataset:{},value:'3',textContent:''};
const commands=[],calls=[];
const state={always_on_top:'0',fullscreen:'0',visible:'1',minimized:'0',scale:'3'};
const context={console,Date,$:id=>elements[id],refresh:async()=>{},loadFrameSnapshots:async()=>{},parseStatus:s=>Object.fromEntries(s.matchAll(/(\w+)=(\w+)/g).map(m=>[m[1],m[2]])),cmd:async c=>{
  commands.push(c);
  if(c==='window fullscreen on')state.fullscreen='1';
  if(c==='window fullscreen off')state.fullscreen='0';
  if(c.startsWith('window scale '))state.scale=c.split(' ')[2];
  return 'OK '+Object.entries(state).map(([k,v])=>k+'='+v).join(' ');
},api:async(url,opts)=>{calls.push({url,opts});return '{}'}};
vm.createContext(context);vm.runInContext(bindings,context);
(async()=>{
  for(const [id,command] of Object.entries({sdlShow:'window show',sdlHide:'window hide',sdlMinimize:'window minimize',sdlRestore:'window restore',sdlPause:'pause',sdlResume:'resume',sdlStep:'step frame 1'})){
    commands.length=0;await elements[id].onclick();assert.equal(commands[0],command);assert.equal(commands[1],'window status');
  }
  await elements.sdlFullscreen.onclick();assert.equal(elements.sdlFullscreen.dataset.on,'1');
  await elements.sdlFullscreen.onclick();assert.equal(elements.sdlFullscreen.dataset.on,'0');
  elements.sdlScale.value='4';await elements.sdlScale.onchange();assert.equal(state.scale,'4');
  await elements.sdlSnapshot.onclick();assert.equal(calls[0].url,'/api/v1/frame/snapshot');assert.equal(calls[0].opts.method,'POST');
  console.log('Workbench SDL command bindings, toggle state and snapshot route: PASS');
})().catch(e=>{console.error(e);process.exitCode=1});
