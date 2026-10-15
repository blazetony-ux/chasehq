#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace chq {

struct Tc0100BgNativeInput {
    const std::vector<std::uint8_t>* tilemap = nullptr;
    const std::vector<std::uint8_t>* control = nullptr;
    const std::vector<std::uint8_t>* tile_gfx = nullptr;
    const std::array<std::uint16_t,4096>* palette = nullptr;
    int layer = 0;              // physical TC0100SCN BG layer: 0 or 1
    bool opaque = false;        // bottom compositor pass treats pen 0 as opaque
    bool legacy_x = false;      // compatibility with historical diagnostic mode
    int width = 320;
    int height = 240;
    int visible_y_start = 16;
    int offset_x = 0;
    int offset_y = 0;
};

struct Tc0100BgNativeResult {
    std::vector<std::uint32_t> pixels;   // ARGB; zero means no contribution
    std::vector<std::uint8_t> coverage;  // 1 where this layer contributes
    std::uint64_t visible_pixels = 0;
    bool available = false;
};

struct Tc0100BgNativeCompare {
    std::uint64_t pixels = 0;
    std::uint64_t pixel_mismatches = 0;
    std::uint64_t coverage_mismatches = 0;
    int first_x = -1;
    int first_y = -1;
    std::uint32_t original_pixel = 0;
    std::uint32_t native_pixel = 0;
    std::uint8_t original_coverage = 0;
    std::uint8_t native_coverage = 0;
};

Tc0100BgNativeResult render_tc0100scn_bg_native(const Tc0100BgNativeInput& in);
Tc0100BgNativeCompare compare_tc0100scn_bg_native(
    const std::vector<std::uint32_t>& original,
    const std::vector<std::uint8_t>& original_coverage,
    const Tc0100BgNativeResult& native,
    int width,
    int height);

} // namespace chq
