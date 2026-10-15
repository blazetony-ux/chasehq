#pragma once

#include "hud_layout.h"

#include <array>
#include <cstdint>
#include <vector>

namespace chq {

struct Tc0100TextNativeInput {
    const std::vector<std::uint8_t>* tilemap = nullptr;
    const std::vector<std::uint8_t>* control = nullptr;
    const std::array<std::uint16_t,4096>* palette = nullptr;
    const HudLayoutState* hud_layout = nullptr;
    int width = 320;
    int height = 240;
    int visible_y_start = 16;
    int offset_x = 0;
    int offset_y = 0;
};

struct Tc0100TextNativeResult {
    std::vector<std::uint32_t> pixels;   // ARGB; zero means transparent/no text pen.
    std::vector<std::uint8_t> coverage;  // 1 where a non-zero text pen is present.
    std::uint64_t visible_pixels = 0;
    bool available = false;
};

struct Tc0100TextNativeCompare {
    std::uint64_t pixels = 0;
    std::uint64_t pixel_mismatches = 0;
    int first_x = -1;
    int first_y = -1;
    std::uint32_t original_pixel = 0;
    std::uint32_t native_pixel = 0;
};

Tc0100TextNativeResult render_tc0100scn_text_native(const Tc0100TextNativeInput& in);
Tc0100TextNativeCompare compare_tc0100scn_text_native(
    const std::vector<std::uint32_t>& original,
    const Tc0100TextNativeResult& native,
    int width,
    int height);

} // namespace chq
