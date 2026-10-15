#pragma once

#include "sprite_gfx_shadow.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

namespace chq {

// RC4 independent whole placed-sprite source renderer.
// This deliberately does not call Video::draw_sprite(), Video::draw_scaled_chunk(),
// Video::sprite_visible_y(), Machine::format_for_zoomx(), or Machine::spritemap_word().
// Its purpose is SHADOW/COMPARE validation of the raw 8-byte sprite records,
// spritemap lookup, chunk decode, zoom/flip placement, palette decode and
// first-nontransparent slot ownership as one independent implementation.
struct SpriteLayerShadowOverride {
    std::uint16_t source_map = 0;
    std::uint16_t target_map = 0;
    // 0=exact source_map match, 1=semantic player_car.body match.
    int semantic_object = 0;
    // 0=preserve descriptor format, 1=OBJ-A 128x128, 2=OBJ-B 64x128, 3=OBJ-B 32x128.
    int target_format = 0;
    int palette = -1;
    int flip_x = -1;
    int flip_y = -1;
    int dx = 0;
    int dy = 0;
};


inline bool shadow_player_car_body_match(std::uint16_t map, std::uint8_t palette) noexcept {
    // Proven player-body family used by semantic object evidence. Deliberately
    // excludes shadow/beacon maps so appearance replacement cannot consume them.
    switch (map) {
        case 472: case 475: case 476: case 508: case 509: case 514: case 516: case 522: case 530: case 536:
            return palette == 64u || palette == 65u;
        default:
            return false;
    }
}

inline bool shadow_override_matches(const SpriteLayerShadowOverride& ov,
                                    std::uint16_t source_map,
                                    std::uint8_t source_palette) noexcept {
    if (ov.semantic_object == 1)
        return shadow_player_car_body_match(source_map, source_palette);
    return ov.source_map == source_map;
}

struct SpriteLayerShadowInput {
    const std::vector<std::uint8_t>* sprite_ram = nullptr;
    const std::vector<std::uint8_t>* spritemap = nullptr;
    const std::vector<std::uint8_t>* gfx_a = nullptr;
    const std::vector<std::uint8_t>* gfx_b = nullptr;
    const std::array<std::uint16_t,4096>* palette = nullptr;
    int width = 320;
    int height = 240;
    int visible_y_start = 16;
    int offset_x = 0;
    int offset_y = 0;
    bool ascending_slots = false;
    const std::vector<SpriteLayerShadowOverride>* overrides = nullptr;
};

struct SpriteLayerShadowImage {
    std::vector<std::uint32_t> pixels;
    std::vector<std::uint16_t> owners;
    // Count every in-bounds non-zero sprite candidate at each pixel, including
    // candidates later hidden by sprite occupancy or compositor masking. This
    // mirrors the authoritative forensic overlap-density contract.
    std::vector<std::uint16_t> coverage;
    std::size_t active_records = 0;
    std::size_t nonzero_source_pixels = 0;
};

struct SpriteLayerShadowCompare {
    std::size_t pixels = 0;
    int width = 320;
    std::size_t pixel_mismatches = 0;
    std::size_t owner_mismatches = 0;
    int first_x = -1;
    int first_y = -1;
    std::uint32_t authoritative_pixel = 0;
    std::uint32_t shadow_pixel = 0;
    std::uint16_t authoritative_owner = 0xffff;
    std::uint16_t shadow_owner = 0xffff;
};

inline std::uint16_t shadow_be16(const std::vector<std::uint8_t>& v, std::size_t off) {
    if (off + 1 >= v.size()) return 0;
    return static_cast<std::uint16_t>((static_cast<std::uint16_t>(v[off]) << 8) | v[off + 1]);
}

inline std::uint16_t shadow_spritemap_word(const std::vector<std::uint8_t>& v, std::size_t word) {
    const auto off = word * 2u;
    if (off + 1 >= v.size()) return 0xffff;
    // The STY ROM is consumed little-endian by the Taito Z object hardware.
    return static_cast<std::uint16_t>(v[off] | (static_cast<std::uint16_t>(v[off + 1]) << 8));
}

inline std::uint32_t shadow_palette_argb(const std::array<std::uint16_t,4096>& palette,
                                         std::uint8_t bank, std::uint8_t pen) {
    if (!pen) return 0;
    const auto raw = palette[((static_cast<unsigned>(bank) << 4) | (pen & 15u)) & 0x0fffu];
    const auto expand5 = [](unsigned v) -> std::uint8_t {
        v &= 31u;
        return static_cast<std::uint8_t>((v << 3) | (v >> 2));
    };
    const auto r = expand5(raw);
    const auto g = expand5(raw >> 5);
    const auto b = expand5(raw >> 10);
    return 0xff000000u | (static_cast<std::uint32_t>(r) << 16)
        | (static_cast<std::uint32_t>(g) << 8) | b;
}

inline SpriteLayerShadowImage render_sprite_layer_shadow(const SpriteLayerShadowInput& in) {
    SpriteLayerShadowImage out{};
    if (!in.sprite_ram || !in.spritemap || !in.gfx_a || !in.gfx_b || !in.palette
        || in.width <= 0 || in.height <= 0) return out;
    const auto& ram = *in.sprite_ram;
    const auto& smap = *in.spritemap;
    const auto& gfx_a = *in.gfx_a;
    const auto& gfx_b = *in.gfx_b;
    const auto& palette = *in.palette;
    const auto count = ram.size() / 8u;
    const auto canvas = static_cast<std::size_t>(in.width) * static_cast<std::size_t>(in.height);
    out.pixels.assign(canvas, 0u);
    out.owners.assign(canvas, 0xffffu);
    out.coverage.assign(canvas, 0u);
    std::vector<std::uint8_t> occupied(canvas, 0u);

    auto draw_slot = [&](std::size_t pos) {
        const std::size_t off = pos * 8u;
        const auto w0 = shadow_be16(ram, off + 0);
        const auto w1 = shadow_be16(ram, off + 2);
        const auto w2 = shadow_be16(ram, off + 4);
        const auto w3 = shadow_be16(ram, off + 6);
        const auto source_map = static_cast<std::uint16_t>(w3 & 0x07ffu);
        if (!source_map) return;
        const auto source_palette = static_cast<std::uint8_t>((w1 & 0x7f80u) >> 7);
        std::uint16_t map = source_map;
        int forced_format = 0, forced_palette = -1, forced_flip_x = -1, forced_flip_y = -1;
        int override_dx = 0, override_dy = 0;
        if (in.overrides) {
            for (const auto& ov : *in.overrides) if (shadow_override_matches(ov, source_map, source_palette)) {
                map = ov.target_map; forced_format = ov.target_format; forced_palette = ov.palette;
                forced_flip_x = ov.flip_x; forced_flip_y = ov.flip_y;
                override_dx = ov.dx; override_dy = ov.dy;
                break;
            }
        }
        if (!map) return;

        const int source_zoom_y = static_cast<int>((w0 >> 9) & 0x7f) + 1;
        const int source_zoom_x = static_cast<int>(w1 & 0x7f) + 1;
        const int raw_zoom_x = (source_zoom_x - 1) & 0x7f;
        const int source_fmt = (raw_zoom_x & 0x40) ? 1 : ((raw_zoom_x & 0x20) ? 2 : (((raw_zoom_x & 0x60) == 0) ? 3 : 0));

        auto layout_for = [&](int which_fmt, std::uint16_t which_map, int& out_cols,
                              std::size_t& out_base, const std::vector<std::uint8_t>*& out_gfx) -> bool {
            if (which_fmt == 1) {
                out_cols = 8; out_base = static_cast<std::size_t>(which_map) << 6; out_gfx = &gfx_a; return true;
            }
            if (which_fmt == 2) {
                out_cols = 4; out_base = (static_cast<std::size_t>(which_map) << 5) + 0x20000u; out_gfx = &gfx_b; return true;
            }
            if (which_fmt == 3) {
                out_cols = 2; out_base = (static_cast<std::size_t>(which_map) << 4) + 0x30000u; out_gfx = &gfx_b; return true;
            }
            return false;
        };

        int cols = 0;
        std::size_t map_base = 0;
        const std::vector<std::uint8_t>* gfx = nullptr;
        const int fmt = forced_format ? forced_format : source_fmt;
        if (!layout_for(fmt, map, cols, map_base, gfx)) return;
        ++out.active_records;

        int source_x = static_cast<int>(w2 & 0x01ffu);
        if (source_x > 0x140) source_x -= 0x200;
        source_x += in.offset_x;
        int y_anchor = static_cast<int>(w0 & 0x01ffu) + 7;
        if (y_anchor > 0x140) y_anchor -= 0x200;
        y_anchor = y_anchor - in.visible_y_start + in.offset_y;
        const int source_base_y = y_anchor + (128 - source_zoom_y);
        const int source_base_x = source_x;
        const bool source_flip_x = (w2 & 0x4000u) != 0;
        const bool source_flip_y = (w2 & 0x8000u) != 0;
        const bool flip_x = forced_flip_x >= 0 ? (forced_flip_x != 0) : source_flip_x;
        const bool flip_y = forced_flip_y >= 0 ? (forced_flip_y != 0) : source_flip_y;
        const auto bank = static_cast<std::uint8_t>(forced_palette >= 0 ? forced_palette : ((w1 & 0x7f80u) >> 7));
        constexpr int rows = 8;

        int zoom_x = source_zoom_x, zoom_y = source_zoom_y;
        int base_x = source_base_x + override_dx;
        int base_y = source_base_y + override_dy;

        // RC3.1 semantic cross-format fit. A map ID does not describe the same
        // logical canvas in OBJ-A and OBJ-B. Determine the non-transparent
        // content bounds of both source and replacement, then fit the replacement
        // content to the authentic source content box with bottom/centre-equivalent
        // placement. This preserves the gameplay object's screen footprint while
        // decoding the replacement with its own hardware format.
        if (forced_format && source_fmt && source_fmt != fmt) {
            struct Bounds { int l=0,t=0,r=-1,b=-1; bool valid=false; };
            auto content_bounds = [&](int bf, std::uint16_t bm) -> Bounds {
                int bc=0; std::size_t bb=0; const std::vector<std::uint8_t>* bg=nullptr;
                Bounds z{}; if(!layout_for(bf,bm,bc,bb,bg)) return z;
                const int full_w=bc*16;
                for(int yy=0;yy<128;++yy) for(int xx=0;xx<full_w;++xx){
                    const int cx=xx>>4, cy=yy>>4;
                    const auto raw_code=shadow_spritemap_word(smap,bb+static_cast<std::size_t>(cx+cy*bc));
                    const auto code=static_cast<std::uint16_t>(raw_code&0x3fffu);
                    if(!decode_sprite_pixel_4bpp_shadow(*bg,code,xx&15,yy&15)) continue;
                    if(!z.valid){z={xx,yy,xx,yy,true};}else{if(xx<z.l)z.l=xx;if(xx>z.r)z.r=xx;if(yy<z.t)z.t=yy;if(yy>z.b)z.b=yy;}
                }
                return z;
            };
            auto transform_bounds=[](Bounds b,int full_w,bool fx,bool fy){
                if(!b.valid) return b;
                if(fx){const int nl=full_w-1-b.r,nr=full_w-1-b.l;b.l=nl;b.r=nr;}
                if(fy){const int nt=127-b.b,nb=127-b.t;b.t=nt;b.b=nb;}
                return b;
            };
            int sc=0; std::size_t sb=0; const std::vector<std::uint8_t>* sg=nullptr;
            if(layout_for(source_fmt,source_map,sc,sb,sg)){
                auto srcb=transform_bounds(content_bounds(source_fmt,source_map),sc*16,source_flip_x,source_flip_y);
                auto dstb=transform_bounds(content_bounds(fmt,map),cols*16,flip_x,flip_y);
                if(srcb.valid&&dstb.valid){
                    const int sw=sc*16, tw=cols*16;
                    const int sl=source_base_x+(srcb.l*source_zoom_x)/sw;
                    const int sr=source_base_x+(((srcb.r+1)*source_zoom_x+sw-1)/sw);
                    const int st=source_base_y+(srcb.t*source_zoom_y)/128;
                    const int sbot=source_base_y+(((srcb.b+1)*source_zoom_y+127)/128);
                    const int want_w=std::max(1,sr-sl), want_h=std::max(1,sbot-st);
                    const int bw=std::max(1,dstb.r-dstb.l+1), bh=std::max(1,dstb.b-dstb.t+1);
                    zoom_x=std::max(1,(want_w*tw+bw/2)/bw);
                    zoom_y=std::max(1,(want_h*128+bh/2)/bh);
                    base_x=sl-(dstb.l*zoom_x)/tw+override_dx;
                    base_y=st-(dstb.t*zoom_y)/128+override_dy;
                }
            }
        }

        for (int j = 0; j < rows; ++j) {
            for (int k = 0; k < cols; ++k) {
                const int map_x = flip_x ? (cols - 1 - k) : k;
                const int map_y = flip_y ? (rows - 1 - j) : j;
                const auto raw_code = shadow_spritemap_word(
                    smap, map_base + static_cast<std::size_t>(map_x + map_y * cols));
                const auto code = static_cast<std::uint16_t>(raw_code & 0x3fffu);
                const int x0 = base_x + (k * zoom_x) / cols;
                const int y0 = base_y + (j * zoom_y) / rows;
                const int x1 = base_x + ((k + 1) * zoom_x) / cols;
                const int y1 = base_y + ((j + 1) * zoom_y) / rows;
                const int cw = x1 - x0, ch = y1 - y0;
                if (cw <= 0 || ch <= 0) continue;
                for (int dy = 0; dy < ch; ++dy) {
                    int sy = (dy * 16) / ch;
                    if (sy < 0) sy = 0; else if (sy > 15) sy = 15;
                    if (flip_y) sy = 15 - sy;
                    for (int dx = 0; dx < cw; ++dx) {
                        int sx = (dx * 16) / cw;
                        if (sx < 0) sx = 0; else if (sx > 15) sx = 15;
                        if (flip_x) sx = 15 - sx;
                        const auto pen = decode_sprite_pixel_4bpp_shadow(*gfx, code, sx, sy);
                        if (!pen) continue;
                        ++out.nonzero_source_pixels;
                        const int px = x0 + dx, py = y0 + dy;
                        if (px < 0 || px >= in.width || py < 0 || py >= in.height) continue;
                        const auto dst = static_cast<std::size_t>(py) * static_cast<std::size_t>(in.width)
                            + static_cast<std::size_t>(px);
                        if (out.coverage[dst] != 0xffffu) ++out.coverage[dst];
                        // First nontransparent sprite owns the location regardless of
                        // later sprites, matching the proven pdrawgfx occupancy rule.
                        if (occupied[dst]) continue;
                        occupied[dst] = 1;
                        out.pixels[dst] = shadow_palette_argb(palette, bank, pen);
                        out.owners[dst] = static_cast<std::uint16_t>(pos);
                    }
                }
            }
        }
    };

    if (in.ascending_slots) {
        for (std::size_t pos = 0; pos < count; ++pos) draw_slot(pos);
    } else {
        for (std::size_t pos = count; pos-- > 0;) draw_slot(pos);
    }
    return out;
}

inline SpriteLayerShadowCompare compare_sprite_layer_shadow(
    const std::vector<std::uint32_t>& authoritative_pixels,
    const std::vector<std::uint16_t>& authoritative_owners,
    const SpriteLayerShadowImage& shadow) {
    SpriteLayerShadowCompare r{};
    const auto n = std::min(authoritative_pixels.size(), shadow.pixels.size());
    r.pixels = n;
    if (!shadow.pixels.empty() && shadow.owners.size() == shadow.pixels.size()) {
        // Current Taito Z raster is 320 wide; keep geometry explicit in the result.
        r.width = 320;
    }
    for (std::size_t i = 0; i < n; ++i) {
        const bool pixel_bad = authoritative_pixels[i] != shadow.pixels[i];
        const bool owner_bad = i < authoritative_owners.size() && i < shadow.owners.size()
            && authoritative_owners[i] != shadow.owners[i];
        if (pixel_bad) ++r.pixel_mismatches;
        if (owner_bad) ++r.owner_mismatches;
        if ((pixel_bad || owner_bad) && r.first_x < 0) {
            r.first_x = static_cast<int>(i % static_cast<std::size_t>(r.width));
            r.first_y = static_cast<int>(i / static_cast<std::size_t>(r.width));
            r.authoritative_pixel = authoritative_pixels[i];
            r.shadow_pixel = shadow.pixels[i];
            if (i < authoritative_owners.size()) r.authoritative_owner = authoritative_owners[i];
            if (i < shadow.owners.size()) r.shadow_owner = shadow.owners[i];
        }
    }
    return r;
}

} // namespace chq
