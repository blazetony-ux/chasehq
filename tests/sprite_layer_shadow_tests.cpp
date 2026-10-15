#include "sprite_layer_shadow.h"

#include <array>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>

static void require(bool v, const char* msg) { if (!v) throw std::runtime_error(msg); }

static void set_pen(std::vector<std::uint8_t>& gfx, std::size_t tile, int x, int y, std::uint8_t pen) {
    for (int physical_plane = 0; physical_plane < 4; ++physical_plane) {
        const auto logical_bit = 3 - physical_plane;
        if (!(pen & (1u << logical_bit))) continue;
        const std::size_t bit = tile * 1024u
            + static_cast<std::size_t>(y >> 2) * 256u
            + static_cast<std::size_t>(y & 3) * 64u
            + static_cast<std::size_t>(physical_plane) * 16u
            + static_cast<std::size_t>(x);
        gfx.at(bit / 8u) |= static_cast<std::uint8_t>(1u << (7u - (bit % 8u)));
    }
}
static void put_be16(std::vector<std::uint8_t>& v, std::size_t off, std::uint16_t w){v.at(off)=static_cast<std::uint8_t>(w>>8);v.at(off+1)=static_cast<std::uint8_t>(w);}
static void put_smap(std::vector<std::uint8_t>& v, std::size_t word, std::uint16_t w){const auto o=word*2u;v.at(o)=static_cast<std::uint8_t>(w);v.at(o+1)=static_cast<std::uint8_t>(w>>8);}

int main(){
    try {
        std::vector<std::uint8_t> ram(16,0), smap((0x30000u+32u)*2u,0xff), a(128,0), b(128,0);
        std::array<std::uint16_t,4096> pal{};
        // 32x128 map 1 uses spritemap words 0x30010..; put tile 0 in all chunks.
        for(std::size_t i=0;i<16;++i) put_smap(smap,0x30010u+i,0);
        // single red pen-1 pixel at source 0,0.
        set_pen(b,0,0,0,1); pal[(5u<<4)|1u]=0x001f;
        // W0 raw Y 25 -> native anchor 32 -> visible anchor 16; zoomY 128.
        // W1 zoomX 32 (raw31) + palette5. W2 x=10. W3 map1.
        put_be16(ram,0,static_cast<std::uint16_t>((127u<<9)|25u));
        put_be16(ram,2,static_cast<std::uint16_t>((5u<<7)|31u));
        put_be16(ram,4,10u); put_be16(ram,6,1u);
        chq::SpriteLayerShadowInput in{&ram,&smap,&a,&b,&pal,320,240,16,0,0,false};
        auto img=chq::render_sprite_layer_shadow(in);
        require(img.active_records==1,"active record parse failed");
        require(img.pixels.size()==320u*240u,"canvas size failed");
        // 32x128 object: first source chunk is 16x16 scaled to 16x16 at x=10,y=16.
        require(img.pixels[16u*320u+10u]==0xffff0000u,"raw record -> placed/palette pixel failed");
        require(img.owners[16u*320u+10u]==0,"owner failed");

        // Higher slot first in descending traversal must own an overlap.
        ram.resize(16,0); // slot1 clone at same geometry, palette6 green.
        for(std::size_t i=0;i<8;++i) ram[8+i]=ram[i];
        put_be16(ram,10,static_cast<std::uint16_t>((6u<<7)|31u)); pal[(6u<<4)|1u]=0x03e0;
        img=chq::render_sprite_layer_shadow(in);
        require(img.owners[16u*320u+10u]==1,"descending first-nontransparent ownership failed");
        require(img.pixels[16u*320u+10u]==0xff00ff00u,"owner palette failed");
        require(img.coverage[16u*320u+10u]==2u,"native overlap coverage must count occluded nonzero candidates");

        in.ascending_slots=true; img=chq::render_sprite_layer_shadow(in);
        require(img.owners[16u*320u+10u]==0,"ascending ownership failed");
        require(img.pixels[16u*320u+10u]==0xffff0000u,"ascending owner palette failed");

        // RC3 cross-format replacement: source descriptor remains a 32x128-class
        // placement box while target artwork is explicitly decoded as OBJ-A.
        // This proves format is no longer inferred solely from source zoom-X.
        ram.assign(8,0);
        put_be16(ram,0,static_cast<std::uint16_t>((127u<<9)|25u));
        put_be16(ram,2,static_cast<std::uint16_t>((5u<<7)|31u));
        put_be16(ram,4,10u); put_be16(ram,6,1u);
        std::vector<std::uint8_t> aa(128,0); set_pen(aa,0,0,0,1);
        for(std::size_t i=0;i<64;++i) put_smap(smap,128u+i,0);
        chq::SpriteLayerShadowOverride ov{}; ov.source_map=1; ov.target_map=2; ov.target_format=1; ov.palette=6; ov.flip_x=0; ov.dx=3;
        std::vector<chq::SpriteLayerShadowOverride> ovs{ov};
        in.gfx_a=&aa; in.overrides=&ovs; in.ascending_slots=false;
        img=chq::render_sprite_layer_shadow(in);
        require(img.pixels[16u*320u+13u]==0xff00ff00u,"cross-format OBJ-A replacement failed");
        in.overrides=nullptr; in.gfx_a=&a;


        // RC3.1.2 semantic player-car replacement must match orientation/body maps,
        // not only straight-ahead map516. Use map522/palette64 as a representative
        // alternate player body and prove the same target override remains active.
        ram.assign(8,0);
        put_be16(ram,0,static_cast<std::uint16_t>((127u<<9)|25u));
        put_be16(ram,2,static_cast<std::uint16_t>((64u<<7)|127u));
        put_be16(ram,4,10u); put_be16(ram,6,522u);
        for(std::size_t i=0;i<64;++i) put_smap(smap,128u+i,0);
        chq::SpriteLayerShadowOverride sem{}; sem.semantic_object=1; sem.target_map=2; sem.target_format=1; sem.palette=6; sem.flip_x=0;
        ovs.assign(1,sem); in.overrides=&ovs; in.gfx_a=&aa;
        img=chq::render_sprite_layer_shadow(in);
        require(img.pixels[16u*320u+10u]==0xff00ff00u,"semantic player body replacement failed on alternate body map");
        // v0.66.9.5-RC1.1: Run009 proved full-right steering uses map509/palette65.
        // Keep this exact family member locked into semantic player_car.body coverage.
        put_be16(ram,2,static_cast<std::uint16_t>((65u<<7)|127u)); put_be16(ram,6,509u);
        img=chq::render_sprite_layer_shadow(in);
        require(img.pixels[16u*320u+10u]==0xff00ff00u,"semantic player body replacement failed on proven map509/palette65");
        // A player shadow map/palette must not be consumed by the body replacement.
        put_be16(ram,2,static_cast<std::uint16_t>((70u<<7)|127u)); put_be16(ram,6,594u);
        img=chq::render_sprite_layer_shadow(in);
        require(img.pixels[16u*320u+10u]!=0xff00ff00u,"semantic player body replacement incorrectly matched player shadow");
        in.overrides=nullptr; in.gfx_a=&a;

        // Restore the original 32x128 source fixture for the independent wrap test.
        ram.assign(8,0);
        put_be16(ram,0,static_cast<std::uint16_t>((127u<<9)|25u));
        put_be16(ram,2,static_cast<std::uint16_t>((5u<<7)|31u));
        put_be16(ram,6,1u);

        // X wrap: raw 0x1ff means -1, so source x=0 lands clipped at -1 and x=1 at 0.
        put_be16(ram,4,0x01ffu); ram.resize(8); in.ascending_slots=false;
        set_pen(b,0,1,0,1);
        img=chq::render_sprite_layer_shadow(in);
        require(img.pixels[16u*320u+0u]==0xffff0000u,"signed X wrap/clipping failed");

        std::cout << "PASS independent whole placed sprite-layer shadow primitives\n";
        return 0;
    } catch(const std::exception& e){ std::cerr<<e.what()<<'\n'; return 1; }
}
