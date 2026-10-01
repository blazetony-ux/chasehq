#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace chq {

// Decode one Chase H.Q. 16x16 4bpp sprite pixel.
//
// The four physical plane blocks are laid out at bit offsets 0,16,32,48 for
// each four-row group. MAME's Chase H.Q. tile16x16 layout lists those planes
// in high-to-low significance order, so physical plane 0 contributes pen bit
// 3, plane 1 -> bit 2, plane 2 -> bit 1, and plane 3 -> bit 0.
//
// RC2.5's Map-405 causal palette probe independently confirmed this ordering:
// the old decoder produced the exact 4-bit-reversed pen numbers, while the
// MAME-consistent significance restored coherent cloud shading without changing
// geometry, priority, or sprite visibility.
inline std::uint8_t decode_sprite_pixel_4bpp(
    const std::vector<std::uint8_t>& region,
    std::size_t tile_index,
    int x,
    int y) {

    if (x < 0 || x >= 16 || y < 0 || y >= 16)
        return 0;

    const std::size_t base_bit = tile_index * 1024;
    std::uint8_t value = 0;

    for (int plane = 0; plane < 4; ++plane) {
        const std::size_t bit_index =
            base_bit +
            static_cast<std::size_t>(plane * 16 + x + y * 64);

        const std::size_t byte_index = bit_index >> 3;
        if (byte_index >= region.size())
            continue;

        const int bit_in_byte =
            7 - static_cast<int>(bit_index & 7);

        const std::uint8_t bit =
            static_cast<std::uint8_t>(
                (region[byte_index] >> bit_in_byte) & 1);

        value |= static_cast<std::uint8_t>(bit << (3 - plane));
    }

    return value;
}

} // namespace chq
