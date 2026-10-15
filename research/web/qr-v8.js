/* ChaseHQ-Native minimal QR encoder for LAN pairing URLs.
 * QR Version 8, error correction L, byte mode, mask 0.
 * Self-contained implementation for ASCII/UTF-8 URLs; no network dependency.
 */
(function(root){
'use strict';
const SIZE=49, DATA_CW=194, BLOCK_DATA=97, EC_CW=24;
const EXP=new Array(512), LOG=new Array(256);
(function(){let x=1;for(let i=0;i<255;i++){EXP[i]=x;LOG[x]=i;x<<=1;if(x&0x100)x^=0x11d;}for(let i=255;i<512;i++)EXP[i]=EXP[i-255];LOG[0]=-1;})();
function mul(a,b){return !a||!b?0:EXP[LOG[a]+LOG[b]];}
function rsGenerator(n){let g=[1];for(let i=0;i<n;i++){const ng=new Array(g.length+1).fill(0);for(let j=0;j<g.length;j++){ng[j]^=g[j];ng[j+1]^=mul(g[j],EXP[i]);}g=ng;}return g;}
const GEN=rsGenerator(EC_CW);
function rsEncode(data){const msg=data.concat(new Array(EC_CW).fill(0));for(let i=0;i<data.length;i++){const f=msg[i];if(!f)continue;for(let j=0;j<GEN.length;j++)msg[i+j]^=mul(GEN[j],f);}return msg.slice(data.length);}
function bytes(s){if(typeof TextEncoder!=='undefined')return Array.from(new TextEncoder().encode(s));const out=[];for(let i=0;i<s.length;i++){let c=s.charCodeAt(i);if(c<0x80)out.push(c);else if(c<0x800)out.push(0xc0|(c>>6),0x80|(c&63));else out.push(0xe0|(c>>12),0x80|((c>>6)&63),0x80|(c&63));}return out;}
function pushBits(arr,val,n){for(let i=n-1;i>=0;i--)arr.push((val>>>i)&1);}
function makeData(text){const b=bytes(text);if(b.length>192)throw new Error('Pairing URL is too long for QR v8-L ('+b.length+' bytes)');const bits=[];pushBits(bits,4,4);pushBits(bits,b.length,8);for(const v of b)pushBits(bits,v,8);const cap=DATA_CW*8;for(let i=0;i<4&&bits.length<cap;i++)bits.push(0);while(bits.length%8)bits.push(0);const data=[];for(let i=0;i<bits.length;i+=8){let v=0;for(let j=0;j<8;j++)v=(v<<1)|bits[i+j];data.push(v);}let pad=0;while(data.length<DATA_CW)data.push((pad++&1)?0x11:0xec);if(data.length!==DATA_CW)throw new Error('QR data overflow');return data;}
function interleave(data){const b1=data.slice(0,BLOCK_DATA),b2=data.slice(BLOCK_DATA),e1=rsEncode(b1),e2=rsEncode(b2),out=[];for(let i=0;i<BLOCK_DATA;i++)out.push(b1[i],b2[i]);for(let i=0;i<EC_CW;i++)out.push(e1[i],e2[i]);return out;}
function bchDigit(x){let d=0;while(x){d++;x>>>=1;}return d;}
function typeInfo(data){let d=data<<10,g=0x537;while(bchDigit(d)-bchDigit(g)>=0)d^=g<<(bchDigit(d)-bchDigit(g));return ((data<<10)|d)^0x5412;}
function versionInfo(v){let d=v<<12,g=0x1f25;while(bchDigit(d)-bchDigit(g)>=0)d^=g<<(bchDigit(d)-bchDigit(g));return (v<<12)|d;}
function matrix(text){const m=Array.from({length:SIZE},()=>Array(SIZE).fill(null));
 function finder(row,col){for(let r=-1;r<=7;r++){for(let c=-1;c<=7;c++){const y=row+r,x=col+c;if(y<0||x<0||y>=SIZE||x>=SIZE)continue;const dark=(r>=0&&r<=6&&c>=0&&c<=6&&(r===0||r===6||c===0||c===6||(r>=2&&r<=4&&c>=2&&c<=4)));m[y][x]=dark;}}}
 finder(0,0);finder(SIZE-7,0);finder(0,SIZE-7);
 const pos=[6,24,42];for(const row of pos)for(const col of pos){if(m[row][col]!==null)continue;for(let r=-2;r<=2;r++)for(let c=-2;c<=2;c++)m[row+r][col+c]=(Math.max(Math.abs(r),Math.abs(c))!==1);}
 for(let i=8;i<SIZE-8;i++){if(m[i][6]===null)m[i][6]=(i%2===0);if(m[6][i]===null)m[6][i]=(i%2===0);}
 const vi=versionInfo(8);for(let i=0;i<18;i++){const bit=((vi>>>i)&1)!==0;m[Math.floor(i/3)][i%3+SIZE-11]=bit;m[i%3+SIZE-11][Math.floor(i/3)]=bit;}
 const fi=typeInfo((1<<3)|0);for(let i=0;i<15;i++){const bit=((fi>>>i)&1)!==0;if(i<6)m[i][8]=bit;else if(i<8)m[i+1][8]=bit;else m[SIZE-15+i][8]=bit;if(i<8)m[8][SIZE-i-1]=bit;else if(i<9)m[8][15-i]=bit;else m[8][15-i-1]=bit;}m[SIZE-8][8]=true;
 const code=interleave(makeData(text));let byteIndex=0,bitIndex=7,row=SIZE-1,inc=-1;for(let col=SIZE-1;col>0;col-=2){if(col===6)col--;while(true){for(let c=0;c<2;c++){const x=col-c;if(m[row][x]!==null)continue;let dark=false;if(byteIndex<code.length)dark=((code[byteIndex]>>>bitIndex)&1)!==0;if(((row+x)&1)===0)dark=!dark;m[row][x]=dark;if(--bitIndex<0){byteIndex++;bitIndex=7;}}row+=inc;if(row<0||row>=SIZE){row-=inc;inc=-inc;break;}}}
 return m;
}
function svg(text,cell=4,border=4){const m=matrix(text),n=SIZE+border*2,w=n*cell;let p='';for(let y=0;y<SIZE;y++)for(let x=0;x<SIZE;x++)if(m[y][x])p+=`M${(x+border)*cell},${(y+border)*cell}h${cell}v${cell}h-${cell}z`;return `<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 ${w} ${w}" width="${w}" height="${w}" role="img" aria-label="QR code"><rect width="100%" height="100%" fill="#fff"/><path d="${p}" fill="#000"/></svg>`;}
const api={matrix,svg};if(typeof module!=='undefined'&&module.exports)module.exports=api;root.ChaseHQQr=api;
})(typeof window!=='undefined'?window:globalThis);
