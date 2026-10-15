#!/usr/bin/env python3
"""Independent Python oracle creating synthetic examples, not original game captures."""
import json,base64,pathlib,struct
cases=[]
for frame in range(4):
    n=10+frame
    raw=[0xcafe0000 | i for i in range(512)]
    for i in range(n):
        raw[2*i]=0x91000000 | (((frame*6+i*11)&0xffff)<<8) | ((i*7+frame)&0xff)
        raw[2*i+1]=0x00600000 | (((i*41+frame*13)&0xffff)<<4) | ((i+3)&15)
    for j in range(2*n,2*n+4):raw[j]=0
    lookup=[(k*19+29)&255 for k in range(1024)]
    source=[(k*31+frame*9)&1023 for k in range(104)]
    masks=[]
    high=(0xd200+0x2400)&0xff00
    first,second=1470+frame*100,300+frame*40
    for i in range(104):
        bound=120+i*22
        if second<bound<=first:
            masks.append(0x9800)
            source[i]=high | lookup[source[i]&1023]
        else:masks.append(0x8800)
    def packed(fmt,vals):return base64.b64encode(b''.join(struct.pack(fmt,v) for v in vals)).decode()
    cases.append({'frame':frame,'sprite':packed('>I',raw),'source':packed('>H',source),'road':packed('>H',masks),'selected':masks.count(0x9800),'records':n})
out={'description':'SYNTHETIC generated fixtures only; no Chase H.Q. ROMs or gameplay pixels','cases':cases}
pathlib.Path(__file__).resolve().parent.parent.joinpath('golden.json').write_text(json.dumps(out,indent=2)+'\n')
print('generated',len(cases),'synthetic oracle cases')
