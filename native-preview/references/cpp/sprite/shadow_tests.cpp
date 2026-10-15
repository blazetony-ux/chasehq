#include "sprite_transfer_model.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cctype>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using chq_shadow::TransferInput;
using chq_shadow::Write;
using Ram = std::array<std::uint32_t, chq_shadow::kSpriteRamLongwords>;

struct Event {
    std::uint32_t pc, address, old_value, new_value;
    bool changed;
};

static std::uint32_t number(const std::string& x) {
    return static_cast<std::uint32_t>(std::stoul(x, nullptr, 0));
}

static void check(bool ok, const std::string& why) {
    if (!ok) throw std::runtime_error(why);
}

static void unit_tests() {
    {
        Ram ram{};
        auto writes = chq_shadow::transfer({1,0x1027F8,{0x12345678,0x9ABCDEF0},-1}, ram);
        check(writes.size()==2 && ram[0]==0x12345678 && ram[1]==0x9ABCDEF0, "one-record copy");
    }
    {
        Ram ram{}; ram[2]=0xFFFFFFFF; ram[3]=0xABCD1234;
        auto writes = chq_shadow::transfer({1,0x1027F8,{0x12345678,0x9ABCDEF0},0}, ram);
        check(writes.size()==4 && ram[2]==0 && ram[3]==0 && writes[2].destination==0xD00008, "one stale record clear");
    }
    {
        Ram ram{}; ram[0]=0xBAD;
        auto writes = chq_shadow::transfer({0,0x102800,{},0}, ram);
        check(writes.empty() && ram[0]==0xBAD, "zero-count early exit");
    }
    {
        Ram ram{};
        TransferInput in; in.latched_records=256; in.staged_start=0x102000;
        in.staged_words.resize(512, 0x10203040);
        auto writes = chq_shadow::transfer(in,ram);
        check(writes.size()==512 && writes.back().destination==0xD007FC, "full 2K boundary");
    }
    {
        Ram ram{};
        try {
            chq_shadow::transfer({257,0x102800,std::vector<std::uint32_t>(514,0),-1},ram);
            throw std::runtime_error("missing out-of-range rejection");
        } catch (const std::invalid_argument&) {}
    }
    std::cout << "PASS standalone unit tests (5)\n";
}

static void verify_fixture(const std::string& file) {
    std::ifstream stream(file);
    check(stream.good(), "cannot open fixture "+file);
    std::string header,line;
    std::getline(stream,header);
    check(header.rfind("# ",0)==0, "missing fixture metadata");
    std::string name;
    unsigned count=0, frame=0;
    int clear=-1;
    std::uint32_t source=0;
    {
        std::istringstream meta(header.substr(2));
        std::string tok;
        while (meta >> tok) {
            auto pos=tok.find('=');
            if (pos==std::string::npos) continue;
            auto key=tok.substr(0,pos),value=tok.substr(pos+1);
            if(key=="name") name=value;
            if(key=="frame") frame=static_cast<unsigned>(number(value));
            if(key=="count") count=static_cast<unsigned>(number(value));
            if(key=="source_start") source=number(value);
            if(key=="clear_dbra") clear=std::stoi(value);
        }
    }
    std::getline(stream,line);
    check(line=="pc,address,old,new,changed", "wrong fixture columns");
    std::vector<Event> observed;
    while (std::getline(stream,line)) {
        if (line.empty()) continue;
        std::istringstream row(line);
        std::vector<std::string> cols;
        std::string field;
        while (std::getline(row,field,',')) cols.push_back(field);
        check(cols.size()==5, "malformed trace fixture row");
        observed.push_back({number(cols[0]),number(cols[1]),number(cols[2]),number(cols[3]),number(cols[4])!=0});
    }
    check(observed.size() >= count*2, "missing copied trace rows");
    check(observed.size() <= 512, "too many sprite writes");

    // Inputs are extracted from observed hardware writes, NOT independently
    // captured whole-buffer staging snapshots. This is trace-replay validation
    // of control/addressing semantics, NOT live independent shadow verification.
    TransferInput in;
    in.latched_records=static_cast<std::uint16_t>(count);
    in.staged_start=source;
    in.clear_dbra_counter=static_cast<std::int16_t>(clear);
    for (std::size_t i=0; i<count*2; ++i) in.staged_words.push_back(observed[i].new_value);
    Ram ram{};
    for (const auto& e: observed) {
        auto idx=(e.address-chq_shadow::kSpriteRamStart)/4;
        check(idx<ram.size(), "trace address outside sprite RAM");
        ram[idx]=e.old_value;
    }
    auto expected=chq_shadow::transfer(in,ram);
    check(expected.size()==observed.size(), name+": event count mismatch");
    for (std::size_t i=0; i<expected.size(); ++i) {
        const auto& out=expected[i];
        const auto& obs=observed[i];
        check(out.destination==obs.address && out.value==obs.new_value, name+": bus address/value mismatch at event "+std::to_string(i+1));
        const auto pc = (i<count*2) ? (0x40576u-4u*count+2u*static_cast<unsigned>(i))
                                    : (0x4016Cu+2u*static_cast<unsigned>((i-count*2)%2));
        check(obs.pc==pc, name+": dispatcher/clear writer PC mismatch at event "+std::to_string(i+1));
        check(obs.changed==(obs.old_value!=obs.new_value), name+": changed-bit mismatch");
        check(ram[(obs.address-chq_shadow::kSpriteRamStart)/4]==obs.new_value, name+": final RAM mismatch");
    }
    std::cout << "PASS "<<name<<" frame="<<frame<<" copied_records="<<count
              <<" events="<<observed.size()<<" cleared_records="<<((observed.size()-count*2)/2)<<"\n";
}

int main(int argc,char** argv) {
    try {
        unit_tests();
        if(argc<2) {std::cerr<<"Usage: shadow_tests fixture.csv [fixture.csv ...]\n";return 2;}
        for(int i=1; i<argc; ++i) verify_fixture(argv[i]);
        std::cout << "PASS prototype fixture replay; NOT live shadow verification\n";
    } catch(const std::exception& e) {
        std::cerr << "FAIL: " << e.what() << '\n'; return 1;
    }
}
