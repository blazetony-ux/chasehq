#include "tc0100scn_bg_native.h"

#include <algorithm>
#include <cstddef>

namespace chq {
namespace {

std::uint16_t be16(const std::vector<std::uint8_t>& bytes, std::size_t off) {
    if (off + 1 >= bytes.size()) return 0;
    return static_cast<std::uint16_t>((static_cast<unsigned>(bytes[off]) << 8) | bytes[off + 1]);
}

std::uint8_t bg_pen(const std::vector<std::uint8_t>& tiles, std::uint16_t code, int x, int y) {
    if (x < 0 || x > 7 || y < 0 || y > 7) return 0;
    const std::size_t off = static_cast<std::size_t>(code) * 32u
                          + static_cast<std::size_t>(y) * 4u
                          + static_cast<std::size_t>(x >> 1);
    if (off >= tiles.size()) return 0;
    const auto b = tiles[off];
    return (x & 1) ? static_cast<std::uint8_t>(b & 0x0fu)
                   : static_cast<std::uint8_t>(b >> 4);
}

std::uint32_t palette_color(const std::array<std::uint16_t,4096>& palette,
                            std::uint16_t bank, std::uint8_t pen) {
    // Match the authoritative runtime TC0100SCN contract exactly: pen 0
    // contributes priority/coverage on an opaque bottom pass, but its pixel
    // value remains transparent/zero.  Returning opaque black here caused
    // every physical BG1 bottom-layer comparison to fail in RC2 whenever a
    // pen-0 texel was encountered.
    if (pen == 0) return 0u;
    const auto index = static_cast<std::uint16_t>(((bank & 0xffu) << 4) | (pen & 0x0fu));
    const auto raw = palette[index & 0x0fffu];
    const auto expand5 = [](unsigned v) -> std::uint8_t {
        v &= 0x1fu;
        return static_cast<std::uint8_t>((v << 3) | (v >> 2));
    };
    const auto r = expand5(raw >> 0);
    const auto g = expand5(raw >> 5);
    const auto b = expand5(raw >> 10);
    return 0xff000000u | (static_cast<std::uint32_t>(r) << 16)
                         | (static_cast<std::uint32_t>(g) << 8)
                         | static_cast<std::uint32_t>(b);
}

// Deliberately independent from tc0100scn_geometry.h and Video::tile_pixel.
// This is a second implementation for Shadow/native promotion, not a wrapper
// around the authoritative TC0100SCN path.
int source_y(int hardware_y, std::uint16_t ctrl_y) {
    return (hardware_y - static_cast<std::int16_t>(ctrl_y) - 8) & 0x1ff;
}

int source_x(int screen_x, std::uint16_t ctrl_x, std::int16_t rowscroll, bool legacy_x) {
    if (legacy_x)
        return (screen_x + static_cast<std::int16_t>(ctrl_x) + rowscroll + 16) & 0x1ff;
    return (screen_x - static_cast<std::int16_t>(ctrl_x) - rowscroll + 16) & 0x1ff;
}

} // namespace

Tc0100BgNativeResult render_tc0100scn_bg_native(const Tc0100BgNativeInput& in) {
    Tc0100BgNativeResult out;
    if (in.width <= 0 || in.height <= 0 || (in.layer != 0 && in.layer != 1)) return out;
    out.pixels.assign(static_cast<std::size_t>(in.width) * static_cast<std::size_t>(in.height), 0u);
    out.coverage.assign(out.pixels.size(), 0u);
    if (!in.tilemap || !in.control || !in.tile_gfx || !in.palette) return out;
    const auto& tr = *in.tilemap;
    const auto& cr = *in.control;
    const auto& gfx = *in.tile_gfx;
    if (tr.size() < 0x10000u || cr.size() < 0x10u || gfx.empty()) return out;

    const auto ctrl_x = be16(cr, in.layer ? 2u : 0u);
    const auto ctrl_y = be16(cr, in.layer ? 8u : 6u);
    const auto ctrl6 = be16(cr, 12u);
    const auto ctrl7 = be16(cr, 14u);
    out.available = true;
    if (ctrl6 & (1u << in.layer)) return out;

    const std::size_t base = in.layer ? 0x8000u : 0x0000u;
    const std::size_t rowscroll_base = in.layer ? 0xc400u : 0xc000u;
    const bool flip = (ctrl7 & 1u) != 0;

    for (int sy = 0; sy < in.height; ++sy) {
        const int sample_y = sy - in.offset_y;
        const int hw_y = sample_y + in.visible_y_start;
        const std::size_t roff = rowscroll_base + static_cast<std::size_t>((hw_y & 0x1ff) * 2);
        const auto rowscroll = static_cast<std::int16_t>(be16(tr, roff));
        for (int sx = 0; sx < in.width; ++sx) {
            const int sample_x = sx - in.offset_x;
            int mx = source_x(sample_x, ctrl_x, rowscroll, in.legacy_x);
            int my = source_y(hw_y, ctrl_y);
            if (flip) { mx = 0x1ff - mx; my = 0x1ff - my; }

            const int tx = (mx >> 3) & 63;
            const int ty = (my >> 3) & 63;
            int px = mx & 7;
            int py = my & 7;
            const std::size_t off = base + static_cast<std::size_t>(ty * 64 + tx) * 4u;
            if (off + 3 >= tr.size()) continue;
            const auto attr = be16(tr, off);
            const auto code = be16(tr, off + 2u);
            if (attr & 0x4000u) px = 7 - px;
            if (attr & 0x8000u) py = 7 - py;
            const auto pen = bg_pen(gfx, code, px, py);
            if (!in.opaque && pen == 0) continue;

            const auto dst = static_cast<std::size_t>(sy) * static_cast<std::size_t>(in.width)
                           + static_cast<std::size_t>(sx);
            out.pixels[dst] = palette_color(*in.palette, static_cast<std::uint16_t>(attr & 0xffu), pen);
            out.coverage[dst] = 1u;
            ++out.visible_pixels;
        }
    }
    return out;
}

Tc0100BgNativeCompare compare_tc0100scn_bg_native(
    const std::vector<std::uint32_t>& original,
    const std::vector<std::uint8_t>& original_coverage,
    const Tc0100BgNativeResult& native,
    int width,
    int height) {
    Tc0100BgNativeCompare out;
    const auto n = std::min({original.size(), original_coverage.size(), native.pixels.size(), native.coverage.size()});
    out.pixels = n;
    for (std::size_t i = 0; i < n; ++i) {
        const bool pdiff = original[i] != native.pixels[i];
        const bool cdiff = (original_coverage[i] != 0) != (native.coverage[i] != 0);
        if (pdiff) ++out.pixel_mismatches;
        if (cdiff) ++out.coverage_mismatches;
        if ((pdiff || cdiff) && out.first_x < 0) {
            out.first_x = width > 0 ? static_cast<int>(i % static_cast<std::size_t>(width)) : -1;
            out.first_y = width > 0 ? static_cast<int>(i / static_cast<std::size_t>(width)) : -1;
            out.original_pixel = original[i];
            out.native_pixel = native.pixels[i];
            out.original_coverage = original_coverage[i] ? 1u : 0u;
            out.native_coverage = native.coverage[i] ? 1u : 0u;
        }
    }
    const auto max_pixels = std::max(original.size(), native.pixels.size());
    if (max_pixels > n) out.pixel_mismatches += max_pixels - n;
    const auto max_cov = std::max(original_coverage.size(), native.coverage.size());
    if (max_cov > n) out.coverage_mismatches += max_cov - n;
    (void)height;
    return out;
}

} // namespace chq
