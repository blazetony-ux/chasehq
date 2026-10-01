#pragma once

#include <cstdint>

namespace chq {

// Convert a TC0100SCN destination/hardware Y coordinate into the 512-line
// source tilemap coordinate. The chip stores BG/FG/TEXT Y scroll in control
// words 3/4/5; MAME models those as negative scroll values. Equivalently,
// source Y is hardware Y minus the signed control word, with the established
// 8-pixel TC0100SCN visible-area delta.
constexpr int tc0100scn_source_y(int hardware_y, std::uint16_t ctrl_y) noexcept {
    return (hardware_y - static_cast<std::int16_t>(ctrl_y) - 8) & 0x1ff;
}

// Standard-width background source X; controls and rowscroll are signed words.
constexpr int tc0100scn_source_x(int screen_x, std::uint16_t ctrl_x,
                                std::uint16_t rowscroll) noexcept {
    return (screen_x - static_cast<std::int16_t>(ctrl_x)
            - static_cast<std::int16_t>(rowscroll) + 16) & 0x1ff;
}
constexpr int tc0100scn_rowscroll_index(int visible_y) noexcept {
    return (visible_y + 16) & 0x1ff;
}
// Standard-width BG1 column table contains 128 big-endian words.
constexpr unsigned tc0100scn_nonzero_column_words(const std::uint8_t* bytes) noexcept {
    unsigned count=0;
    for(unsigned i=0;i<256;i+=2) if(bytes[i] || bytes[i+1]) ++count;
    return count;
}
} // namespace chq
