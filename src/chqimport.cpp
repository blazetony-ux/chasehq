#include "content_pack.h"
#include "cpu_rom.h"
#include "rom_loader.h"
#include "version.h"
#include <filesystem>
#include <iostream>
#include <stdexcept>

static chq::PackBytes sprite_region(const chq::LoadedRom& a,const chq::LoadedRom& b,const chq::LoadedRom& c,const chq::LoadedRom& d){
    chq::PackBytes out(0x200000,0); const chq::LoadedRom* lanes[4]={&a,&b,&c,&d};
    for(int lane=0;lane<4;++lane) for(std::size_t i=0;i+1<lanes[lane]->data.size();i+=2){ auto dst=(i/2)*8+static_cast<std::size_t>(lane*2); out[dst]=lanes[lane]->data[i+1]; out[dst+1]=lanes[lane]->data[i]; }
    return out;
}
int main(int argc,char** argv){
    try{
        if(argc<3){ std::cout<<"chqimport "<<chq::kNativeVersion<<"\nUsage: chqimport <World ROM directory> <output.chqpack>\n"; return argc==1?0:2; }
        const std::filesystem::path roms=argv[1], output=argv[2]; chq::RomLoader loader(roms); if(!loader.load_required()) return 1;
        auto req=[&](const char* n)->const chq::LoadedRom&{auto* r=loader.find(n); if(!r) throw std::runtime_error(std::string("missing required graphics ROM ")+n); return *r;};
        chq::ContentPack pack;
        pack.set("cpu/main.bin",chq::load_main_rom(roms)); pack.set("cpu/sub.bin",chq::load_sub_rom(roms));
        pack.set("graphics/obj-a.bin",sprite_region(req("b52-34.5"),req("b52-35.7"),req("b52-36.9"),req("b52-37.11")));
        pack.set("graphics/obj-b.bin",sprite_region(req("b52-30.4"),req("b52-31.6"),req("b52-32.8"),req("b52-33.10")));
        pack.set("graphics/spritemap.bin",req("b52-38.34").data); pack.set("graphics/road.bin",req("b52-28.4").data);
        auto tiles=req("b52-29.27").data; for(std::size_t i=0;i+1<tiles.size();i+=2) std::swap(tiles[i],tiles[i+1]); pack.set("graphics/tc0100scn.bin",std::move(tiles));
        pack.set("prom/mixer-b52-01.bin",req("b52-01.7").data); pack.set("prom/road-b52-06.bin",req("b52-06.24").data);
        const std::string provenance="CHQPACK1\nsource_set=chasehq_world\nimporter_build="+std::string(chq::kNativeVersion)+"\ncontent=assembled-runtime-regions\n"; pack.set("meta/provenance.txt",chq::PackBytes(provenance.begin(),provenance.end()));
        std::string err; if(!pack.save(output,&err)) throw std::runtime_error(err);
        std::cout<<"[CHQPACK] OK "<<output.string()<<" entries="<<pack.entry_count()<<"\n"; return 0;
    }catch(const std::exception& e){std::cerr<<"chqimport: "<<e.what()<<"\n";return 1;}
}
