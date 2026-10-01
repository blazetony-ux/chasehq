#pragma once
#include <cstdint>

namespace chq {
// MAME prio_transpen/prio_zoom_transpen reserve a pixel on every nonzero pen,
// even when the background masks its colour. Keep occupancy separate from the
// existing BG/road/text mask and from the owner of an actually painted pixel.
inline bool accept_sprite_pixel(std::uint8_t pen, bool background_blocked,
                                std::uint8_t& occupied, std::uint16_t* coverage) {
    if (!pen) return false;
    if (coverage && *coverage != 0xffff) ++*coverage;
    const bool accepted = !occupied && !background_blocked;
    occupied = 1;
    return accepted;
}

inline int sprite_visible_y(std::uint16_t w0, int visible_start, int offset = 0) {
    int y = (w0 & 0x1ff) + 7;
    if (y > 0x140) y -= 0x200;
    return y - visible_start + offset;
}
}
