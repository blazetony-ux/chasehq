#include "sprite_priority.h"
#include <array>
#include <iostream>
#include <stdexcept>

static void require(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}
int main() {
    try {
        struct Pixel { unsigned rgb=0xff123456; unsigned owner=0xffff; std::uint8_t occupied=0; std::uint16_t coverage=0; };
        auto draw=[](Pixel& p, unsigned slot, std::uint8_t pen, unsigned rgb, bool blocked=false) {
            if(chq::accept_sprite_pixel(pen, blocked, p.occupied, &p.coverage)) {p.rgb=rgb;p.owner=slot;}
        };
        std::array<Pixel,4> row{};
        // Synthetic front body and rear black shadow; no ROM/frame dependency.
        draw(row[0],82,3,0xffabcdef); draw(row[0],81,4,0xff000000);
        draw(row[1],82,0,0xffabcdef); draw(row[1],81,4,0xff000000);
        draw(row[2],82,3,0xffabcdef,true); draw(row[2],81,4,0xff000000);
        draw(row[3],82,0,0xffabcdef,true); draw(row[3],81,8,0xff000000);
        require(row[0].owner==82 && row[0].rgb==0xffabcdef && row[0].coverage==2,"front ownership/overlap coverage");
        require(row[1].owner==81 && row[1].rgb==0xff000000 && row[1].coverage==1,"transparent front / opaque black shadow");
        require(row[2].owner==0xffff && row[2].rgb==0xff123456 && row[2].coverage==2,"masked front must reserve pixel without becoming visible owner");
        require(row[3].owner==81,"transparent masked front must not reserve pixel");
        Pixel reverse; draw(reverse,81,4,0xff000000); draw(reverse,82,3,0xffabcdef);
        require(reverse.owner==81 && reverse.coverage==2,"reversed diagnostic traversal must reverse overlap ownership");
        for(bool blocked : {false,true}) {Pixel p;draw(p,82,1,0xff999999,blocked);require(p.owner==(blocked?0xffffu:82u),"retain background acceptance decision");}
        Pixel saturated; saturated.coverage=0xffff;draw(saturated,81,1,0xff000000);require(saturated.coverage==0xffff,"coverage saturation");
        require(chq::sprite_visible_y(196,16)==187,"visible raster subtraction");
        require(chq::sprite_visible_y(196,16,-3)==184,"presentation offset applied once");
        require(chq::sprite_visible_y(511,16)==-10,"signed sprite Y wrapping");
        std::cout<<"PASS sprite ownership, masking, coverage, black pens and visible coordinates\n";
        return 0;
    } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}
