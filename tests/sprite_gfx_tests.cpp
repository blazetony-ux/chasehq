#include "sprite_gfx.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>

static void require(bool value, const char* message) {
    if (!value)
        throw std::runtime_error(message);
}

static void set_source_plane_bit(
    std::vector<std::uint8_t>& tile,
    int plane,
    int x,
    int y) {

    const std::size_t bit_index =
        static_cast<std::size_t>(plane * 16 + x + y * 64);
    const std::size_t byte_index = bit_index >> 3;
    const int bit_in_byte = 7 - static_cast<int>(bit_index & 7);
    tile.at(byte_index) |= static_cast<std::uint8_t>(1u << bit_in_byte);
}

static std::uint8_t reverse_nibble(std::uint8_t value) {
    return static_cast<std::uint8_t>(
        ((value & 0x01u) << 3) |
        ((value & 0x02u) << 1) |
        ((value & 0x04u) >> 1) |
        ((value & 0x08u) >> 3));
}

int main() {
    try {
        constexpr int x = 7;
        constexpr int y = 9;

        // Exercise every possible physical-plane combination. The old RC2.5
        // decoder returned sourceMask directly; the corrected decoder must
        // return its 4-bit reversal because physical plane 0 is the MSB.
        for (std::uint8_t source_mask = 0; source_mask < 16; ++source_mask) {
            std::vector<std::uint8_t> tile(128, 0); // 16x16x4bpp = 1024 bits.
            for (int plane = 0; plane < 4; ++plane) {
                if (source_mask & (1u << plane))
                    set_source_plane_bit(tile, plane, x, y);
            }

            const auto actual = chq::decode_sprite_pixel_4bpp(tile, 0, x, y);
            const auto expected = reverse_nibble(source_mask);
            if (actual != expected) {
                std::cerr << "source mask " << static_cast<unsigned>(source_mask)
                          << " decoded " << static_cast<unsigned>(actual)
                          << " expected " << static_cast<unsigned>(expected) << '\n';
                return 1;
            }
        }

        // Geometry and transparency invariants must not move with the pen fix.
        std::vector<std::uint8_t> tile(128, 0);
        set_source_plane_bit(tile, 0, x, y);
        require(chq::decode_sprite_pixel_4bpp(tile, 0, x, y) == 8,
                "physical plane 0 must contribute pen bit 3");
        require(chq::decode_sprite_pixel_4bpp(tile, 0, x + 1, y) == 0,
                "sprite pixel geometry changed");
        require(chq::decode_sprite_pixel_4bpp(tile, 0, -1, y) == 0,
                "out-of-range x must be transparent");
        require(chq::decode_sprite_pixel_4bpp(tile, 0, x, 16) == 0,
                "out-of-range y must be transparent");

        // Bit reversal preserves transparent-vs-opaque occupancy semantics:
        // zero stays zero and every nonzero nibble stays nonzero.
        require(reverse_nibble(0) == 0, "transparent pen must remain zero");
        for (std::uint8_t pen = 1; pen < 16; ++pen)
            require(reverse_nibble(pen) != 0, "nonzero pen became transparent");

        std::cout << "PASS Chase H.Q. sprite 4bpp plane significance (all 16 pens)\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
