#include "tc0100scn_geometry.h"
#include <cassert>
#include <iostream>
#include <array>
int main() {
    using namespace chq;
    assert(tc0100scn_source_x(0,0,0)==16);
    assert(tc0100scn_text_source_x(0,0,false)==16);
    assert(tc0100scn_text_source_x(7,0,false)==23);
    assert(tc0100scn_text_source_x(0,0,true)==22);
    assert(tc0100scn_text_source_x(319,0,true)==215);
    assert(tc0100scn_text_source_x(0,1,false)==17);
    assert(tc0100scn_text_source_x(0,1,true)==23);
    assert(tc0100scn_text_source_x(1,0,true)==21);
    assert(tc0100scn_source_x(0,7,0)==9);
    assert(tc0100scn_source_x(0,0,7)==9);
    assert(tc0100scn_source_x(0,0xffff,0xffff)==18);
    assert(tc0100scn_source_x(511,32,33)==462);
    for(int y=0;y<240;++y) assert(tc0100scn_rowscroll_index(y)==y+16);
    for(int x=-512;x<1024;++x) for(unsigned c: {0u,1u,511u,65535u}) {
        assert(tc0100scn_source_x(x,c,0)==tc0100scn_source_x(x+512,c,0));
        assert(tc0100scn_source_x(x,c,1)==((tc0100scn_source_x(x,c,0)-1)&511));
    }
    std::array<std::uint16_t,512> rows{};
    for(int sy=0;sy<240;++sy) {
        auto changed=rows; changed[tc0100scn_rowscroll_index(sy)]=23;
        for(int other=0;other<240;++other) {
            int before=tc0100scn_source_x(100,7,rows[tc0100scn_rowscroll_index(other)]);
            int after=tc0100scn_source_x(100,7,changed[tc0100scn_rowscroll_index(other)]);
            assert(other==sy ? after==((before-23)&511) : after==before);
        }
    }
    std::array<std::uint8_t,256> columns{};
    assert(tc0100scn_nonzero_column_words(columns.data())==0);
    columns[2]=1; columns[3]=2; columns[255]=1;
    assert(tc0100scn_nonzero_column_words(columns.data())==2);
    assert(tc0100scn_source_y(16,0)==8);
    assert(tc0100scn_source_y(16,1)==7);
    std::cout << "TC0100SCN geometry PASS\n";
}
