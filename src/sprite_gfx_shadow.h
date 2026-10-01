#pragma once

#include "sprite_gfx.h"
#include <cstddef>
#include <cstdint>
#include <vector>

namespace chq {

// Independent, deliberately straightforward reference decoder for SHADOW /
// COMPARE validation. Keep its addressing expression separate from the live
// optimized decoder so plane-order or bit-offset regressions are observable.
inline std::uint8_t decode_sprite_pixel_4bpp_shadow(
    const std::vector<std::uint8_t>& region, std::size_t tile, int x, int y) {
    if (x < 0 || x >= 16 || y < 0 || y >= 16) return 0;
    std::uint8_t pen = 0;
    for (int plane = 0; plane != 4; ++plane) {
        const std::size_t row_group = static_cast<std::size_t>(y >> 2) * 256;
        const std::size_t row_in_group = static_cast<std::size_t>(y & 3) * 64;
        const std::size_t plane_offset = static_cast<std::size_t>(plane) * 16;
        const std::size_t bit = tile * 1024 + row_group + row_in_group + plane_offset + static_cast<std::size_t>(x);
        const std::size_t byte = bit / 8;
        if (byte < region.size() && ((region[byte] >> (7 - (bit % 8))) & 1))
            pen |= static_cast<std::uint8_t>(1u << (3 - plane));
    }
    return pen;
}

struct SpriteDecoderCompare {
    std::size_t pixels = 0;
    std::size_t mismatches = 0;
    std::size_t first_tile = 0;
    int first_x = -1;
    int first_y = -1;
    std::uint8_t live_pen = 0;
    std::uint8_t shadow_pen = 0;
};

inline SpriteDecoderCompare compare_sprite_decoder_4bpp(
    const std::vector<std::uint8_t>& region, std::size_t first_tile,
    std::size_t tile_count) {
    SpriteDecoderCompare result{};
    for (std::size_t tile = first_tile; tile < first_tile + tile_count; ++tile)
        for (int y = 0; y < 16; ++y)
            for (int x = 0; x < 16; ++x) {
                const auto live = decode_sprite_pixel_4bpp(region, tile, x, y);
                const auto shadow = decode_sprite_pixel_4bpp_shadow(region, tile, x, y);
                ++result.pixels;
                if (live != shadow) {
                    if (result.mismatches++ == 0) {
                        result.first_tile = tile; result.first_x = x; result.first_y = y;
                        result.live_pen = live; result.shadow_pen = shadow;
                    }
                }
            }
    return result;
}

} // namespace chq
