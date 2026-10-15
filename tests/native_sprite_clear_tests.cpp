#include "native_sprite_clear.h"
#include <cassert>
#include <cstdint>
#include <iostream>
#include <map>
using namespace chq::candidate;
int main(){
    std::map<std::uint32_t,std::uint8_t> mem;
    auto put=[&](std::uint32_t a,unsigned w,std::uint32_t v){for(unsigned i=0;i<w;++i)mem[a+i]=static_cast<std::uint8_t>(v>>(8*(w-i-1)));};
    auto peek=[&](std::uint32_t a,unsigned w)->std::uint32_t{std::uint32_t v=0;for(unsigned i=0;i<w;++i)v=(v<<8)|mem[a+i];return v;};
    put(0x4016c,2,0x22c1);put(0x4016e,2,0x22c1);put(0x40170,2,0x51c8);put(0x40172,2,0xfffa);put(0x40174,2,0x4e75);
    ClearLoopContext c;c.records=45;c.clear_dbra=3;c.d0=0x00000003;c.d1=0;c.sr=0x2404;
    c.a1=0xd00000u+8u*c.records;c.stage_pointer=0x102800u-8u*c.records;
    for(unsigned i=0;i<2u*c.records;++i){put(c.stage_pointer+4*i,4,i+0x1000);put(0xd00000u+4*i,4,i+0x1000);}
    auto check=[&](){return plan_native_sprite_clear(c,peek);};
    auto a=check();assert(a.permitted&&a.bytes==32&&a.emulated_cycles==140&&a.next_a1==c.a1+32&&a.next_d0==0xffff&&a.return_pc==0x40174);
    c.clear_dbra=-1;assert(!check().permitted);c.clear_dbra=3;
    c.d1=1;assert(!check().permitted);c.d1=0;
    c.sr=0x2400;assert(!check().permitted);c.sr=0x2404;
    c.a1+=4;assert(!check().permitted);c.a1-=4;
    c.stage_pointer+=4;assert(!check().permitted);c.stage_pointer-=4;
    c.d0=2;assert(!check().permitted);c.d0=3;
    put(c.stage_pointer+8,4,0xBABE);assert(!check().permitted);put(c.stage_pointer+8,4,0x1002);
    put(0x40172,2,0xfff8);assert(!check().permitted);put(0x40172,2,0xfffa);
    c.records=255;c.clear_dbra=1;assert(!check().permitted);
    c.records=45;c.clear_dbra=0;c.d0=0;a=check();assert(a.permitted&&a.bytes==8&&a.emulated_cycles==38);
    // Original CPU-A Run 005, frame 5587: A1=0xD00280 at 0x4016C,
    // latched 0x50 records, DBRA=3; disassembly 0x40170 -> 0x4016C.
    c.records=80;c.clear_dbra=3;c.d0=3;c.d1=0;c.sr=0x2404;
    c.a1=0xD00280;c.stage_pointer=0x102800-8u*80u;
    for(unsigned i=0;i<2u*c.records;++i){
        put(c.stage_pointer+4*i,4,i+0x3000);
        put(0xD00000+4*i,4,i+0x3000);
    }
    a=check();assert(a.permitted && a.address==0xD00280 && a.bytes==32);
    c.records=45;c.stage_pointer=0x102800u-8u*c.records;
    c.a1=0xd00000u+8u*c.records;
    for(unsigned i=0;i<2u*c.records;++i){put(c.stage_pointer+4*i,4,i+0x1000);put(0xd00000u+4*i,4,i+0x1000);}
    // Exhaustively compare the native tail plan to a simple 68000 clear/DBRA
    // reference model for every valid clear count at this sprite density.
    c.records=45;c.stage_pointer=0x102800u-8u*c.records;c.sr=0x2404;
    c.a1=0xd00000u+8u*c.records;c.d1=0;
    for(unsigned clears=1;clears<=211;++clears){
        c.clear_dbra=static_cast<std::int16_t>(clears-1);c.d0=clears-1;
        auto native=check();assert(native.permitted);
        unsigned refA1=c.a1,refD0=c.d0,refCycles=0;
        for(unsigned j=0;j<clears;++j){
            // MOVE.L D1,(A1)+ / MOVE.L D1,(A1)+ / DBRA D0,target
            refA1+=8;refCycles+=24;
            refD0=(refD0&0xffff0000u)|((refD0-1u)&0xffffu);
            refCycles+=(j+1<clears?10u:14u);
        }
        assert(native.bytes==clears*8u);
        assert(native.next_a1==refA1 && native.next_d0==refD0);
        assert(native.emulated_cycles==refCycles);
    }
    std::cout<<"PASS: guarded native clear-loop substitution; all failure cases fall back before mutation\n";
}
