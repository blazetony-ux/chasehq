#include "live_candidate_shadow.h"
#include <cassert>
#include <cstdint>
#include <map>
#include <string>
#include <iostream>

int main(){
    using namespace chq;
    std::map<std::uint32_t,std::uint8_t> mem;
    auto put=[&](std::uint32_t a,unsigned sz,std::uint32_t v){
        for(unsigned i=0;i<sz;++i)mem[a+i]=static_cast<std::uint8_t>(v>>(8*(sz-i-1)));
    };
    auto peek=[&](std::uint32_t a,unsigned sz)->std::uint32_t{
        std::uint32_t v=0;for(unsigned i=0;i<sz;++i)v=v*256+mem[a+i];return v;
    };
    candidate::RoadInput in;
    in.cf4=400;in.cf2=200;in.cf6=0x2400;
    for(std::size_t i=0;i<candidate::road_entries;++i){
        in.source[i]=static_cast<std::uint16_t>(i);
        in.boundaries[i]=static_cast<std::uint16_t>(i*10);
        put(0x101384+static_cast<std::uint32_t>(i*2),2,in.source[i]);
        put(0x1FF24+static_cast<std::uint32_t>(i*2),2,in.boundaries[i]);
    }
    for(std::size_t i=0;i<in.lookup.size();++i){
        in.lookup[i]=static_cast<std::uint8_t>(i);
        put(0x91A0+static_cast<std::uint32_t>(i),1,in.lookup[i]);
    }
    put(0x102CF4,2,in.cf4);put(0x102CF2,2,in.cf2);put(0x102CF6,2,in.cf6);
    LiveCandidateShadow shadow;shadow.configure(true,false);
    shadow.instruction(true,0x3B84,peek);
    const auto actual=candidate::road_mask(in);
    for(std::size_t i=0;i<candidate::road_entries;++i){
       put(0x101384+static_cast<std::uint32_t>(i*2),2,actual.mutated_source[i]);
       put(0x101454+static_cast<std::uint32_t>(i*2),2,actual.output[i]);
    }
    shadow.instruction(true,0x3BDC,peek);
    assert(shadow.report_json().find("\"pass\":1")!=std::string::npos);
    shadow.instruction(true,0x3B84,peek);
    put(0x101454,2,0x7777);
    shadow.instruction(true,0x3BE6,peek);
    assert(shadow.report_json().find("\"fail\":1")!=std::string::npos);

    auto setup_sprite=[&](int n,int clear){
        put(0x101844,2,static_cast<std::uint32_t>(n));
        const std::uint32_t stage=0x102800u-static_cast<std::uint32_t>(8*n);
        put(0x101846,4,stage);
        put(0x101812,2,static_cast<std::uint16_t>(clear));
        for(int i=0;i<n*2;++i) put(stage+static_cast<std::uint32_t>(i*4),4,0x12340000u+static_cast<std::uint32_t>(i));
        for(std::size_t i=0;i<candidate::sprite_capacity*2;++i)put(0xD00000+static_cast<std::uint32_t>(i*4),4,0x98765432);
        return stage;
    };
    auto bulk_copy=[&](int n){
        for(int i=0;i<n*2;++i)put(0xD00000+static_cast<std::uint32_t>(i*4),4,0x12340000u+static_cast<std::uint32_t>(i));
    };
    auto trailing_clear=[&](int n,int cleared){
        for(int i=n*2;i<(n+cleared)*2;++i)put(0xD00000+static_cast<std::uint32_t>(i*4),4,0);
    };
    shadow.configure(false,true);
    const auto stage1=setup_sprite(2,0); // one cleared record
    shadow.instruction(false,0x4013A,peek,5588);
    bulk_copy(2);
    shadow.instruction(false,0x40576,peek,5588); // copy is complete, clear is NOT
    auto report=shadow.report_json();
    assert(report.find("\"pending\":true")!=std::string::npos);
    assert(report.find("\"bulkPass\":1")!=std::string::npos);
    assert(report.find("\"pass\":0")!=std::string::npos);
    trailing_clear(2,1);
    shadow.instruction(false,0x40174,peek,5588);
    report=shadow.report_json();
    assert(report.find("\"pass\":1")!=std::string::npos);
    assert(report.find("\"pending\":false")!=std::string::npos);
    assert(report.find("\"frame\":5588")!=std::string::npos);
    assert(report.find("\"clearDbra\":0")!=std::string::npos);
    assert(report.find("\"stagePointer\":"+std::to_string(stage1))!=std::string::npos);

    setup_sprite(3,-1); // no clear
    shadow.instruction(false,0x4013A,peek,6334);bulk_copy(3);
    shadow.instruction(false,0x40576,peek,6334);
    shadow.instruction(false,0x40174,peek,6334);
    report=shadow.report_json();
    assert(report.find("\"pass\":2")!=std::string::npos);
    assert(report.find("\"bulkPass\":2")!=std::string::npos);

    // A tail mismatch must fail, even though the copied region passed.
    setup_sprite(4,1); // two cleared records expected
    shadow.instruction(false,0x4013A,peek,9549);bulk_copy(4);
    shadow.instruction(false,0x40576,peek,9549);
    trailing_clear(4,2);
    put(0xD00000+8*4,4,0xDEADBEEF);
    shadow.instruction(false,0x40174,peek,9549);
    report=shadow.report_json();
    assert(report.find("\"fail\":1")!=std::string::npos);
    assert(report.find("\"firstLastMismatchByte\":32")!=std::string::npos);
    assert(report.find("\"expectedWord\":0,\"actualWord\":3735928559")!=std::string::npos);
    assert(report.find("\"bulkPass\":3")!=std::string::npos);
    assert(report.find("\"replacementEnabled\":false")!=std::string::npos);

    // A newly entered transfer must not silently discard an unfinished one.
    setup_sprite(2,0);
    shadow.instruction(false,0x4013A,peek,100);
    shadow.instruction(false,0x4013A,peek,101);
    assert(shadow.report_json().find("\"incomplete\":1")!=std::string::npos);
    bulk_copy(2);shadow.instruction(false,0x40576,peek,101);
    trailing_clear(2,1);shadow.instruction(false,0x40174,peek,101);

    // A missing copy boundary is explicitly incomplete, never a PASS.
    setup_sprite(2,-1);shadow.instruction(false,0x4013A,peek,102);
    bulk_copy(2);shadow.instruction(false,0x40174,peek,102);
    report=shadow.report_json();
    assert(report.find("\"incomplete\":2")!=std::string::npos);

    // Invalid input must be rejected and reported, not substituted or ignored.
    setup_sprite(2,-1);put(0x101846,4,0xBADBAD);
    shadow.instruction(false,0x4013A,peek,103);
    assert(shadow.report_json().find("\"rejected\":1")!=std::string::npos);
    shadow.reset();
    report=shadow.report_json();
    assert(report.find("\"entries\":0")!=std::string::npos);
    assert(report.find("\"recent\":[]")!=std::string::npos);
    std::cout<<"PASS: sprite compare deferred until 0x40174; bulk-only check at 0x40576; clear/no-clear, failure, evidence, incomplete, invalid inputs, reset, read-only\n";
}
