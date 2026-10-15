#include "content_pack.h"
#include <filesystem>
#include <iostream>
int main(){
    chq::ContentPack a; a.set("cpu/main.bin",{0,1,2,3,255}); a.set("meta/provenance.txt",{'o','k'});
    auto p=std::filesystem::temp_directory_path()/"chq-content-pack-test.chqpack"; std::string e;
    if(!a.save(p,&e)){std::cerr<<e;return 1;} chq::ContentPack b; if(!b.load(p,&e)){std::cerr<<e;return 2;}
    auto* x=b.find("cpu/main.bin"); if(!x||*x!=chq::PackBytes({0,1,2,3,255})||b.entry_count()!=2)return 3;
    chq::ContentPack overlay; overlay.set("graphics/road.bin", {1,2,3});
    chq::ContentPack base; base.set("graphics/road.bin", {0,0,0});
    if (!chq::apply_content_overlay(base, overlay, &e) || *base.find("graphics/road.bin") != chq::PackBytes({1,2,3})) return 4;
    chq::ContentPack invalid; invalid.set("cpu/main.bin", {1,2,3});
    if (chq::apply_content_overlay(base, invalid, &e)) return 5;
    chq::ContentPack wrong_size; wrong_size.set("graphics/road.bin", {1});
    if (chq::apply_content_overlay(base, wrong_size, &e)) return 6;
    std::filesystem::remove(p); return 0;
}
