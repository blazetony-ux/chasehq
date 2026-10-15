// Standalone deterministic graphics decoder parity test for the Workbench CHQPACK viewer.
const fs=require('node:fs'),assert=require('node:assert/strict');
const source=fs.readFileSync('Start-ChaseHQWeb.ps1','utf8');
const decoder=source.match(/function packPixel\(data,name,tile,x,y\)\{[\s\S]*?\n\}/);
assert(decoder,'Workbench packPixel decoder missing');
const packPixel=Function(decoder[0]+';return packPixel')();
const data=Uint8Array.from({length:2048},(_,i)=>(i*37+91)&255);
for(const name of ['graphics/obj-a.bin','graphics/obj-b.bin','graphics/tc0100scn.bin']){
 const tileSide=name==='graphics/tc0100scn.bin'?8:16;
 for(let tile=0;tile<8;tile++)for(let y=0;y<tileSide;y++)for(let x=0;x<tileSide;x++){
  let expected=0;
  if(tileSide===8){const b=data[tile*32+y*4+(x>>1)];expected=x&1?b&15:b>>4}
  else {for(let p=0;p<4;p++){const bit=tile*1024+p*16+x+y*64;expected|=((data[bit>>3]>>(7-(bit&7)))&1)<<(3-p)}}
  assert.equal(packPixel(data,name,tile,x,y),expected,`${name} tile ${tile} x${x} y${y}`);
 }
}
console.log('PASS: pack sprite and tile pixel decoders match native layout on 8 tiles per region');
