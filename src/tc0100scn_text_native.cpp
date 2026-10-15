#include "tc0100scn_text_native.h"

#include <algorithm>
#include <cstddef>

namespace chq {
namespace {

std::uint16_t be16(const std::vector<std::uint8_t>& bytes, std::size_t off) {
    if (off + 1 >= bytes.size()) return 0;
    return static_cast<std::uint16_t>((static_cast<unsigned>(bytes[off]) << 8) | bytes[off + 1]);
}

std::uint8_t text_pen(const std::vector<std::uint8_t>& tilemap, std::uint8_t code, int x, int y) {
    const std::size_t off = 0x6000u + static_cast<std::size_t>(code) * 16u + static_cast<std::size_t>(y) * 2u;
    if (off + 1 >= tilemap.size()) return 0;
    const int bit = 7 - x;
    const auto p0 = static_cast<std::uint8_t>((tilemap[off] >> bit) & 1u);
    const auto p1 = static_cast<std::uint8_t>((tilemap[off + 1] >> bit) & 1u);
    return static_cast<std::uint8_t>(p0 | (p1 << 1));
}

std::uint32_t palette_color(const std::array<std::uint16_t,4096>& palette,
                            std::uint16_t bank, std::uint8_t pen) {
    if (pen == 0) return 0;
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

// Intentionally independent from tc0100scn_geometry.h.  The shadow/native
// renderer carries its own coordinate implementation so comparison can detect
// regressions in the authoritative path rather than sharing the same helper.
int source_y(int hardware_y, std::uint16_t ctrl_y) {
    return (hardware_y - static_cast<std::int16_t>(ctrl_y) - 8) & 0x1ff;
}

int text_source_x(int screen_x, std::uint16_t ctrl_x, bool flip) {
    const int scroll = static_cast<std::int16_t>(ctrl_x);
    if (flip) return (0x1ff - screen_x + scroll + 23) & 0x1ff;
    return (screen_x + scroll + 16) & 0x1ff;
}

} // namespace

Tc0100TextNativeResult render_tc0100scn_text_native(const Tc0100TextNativeInput& in) {
    Tc0100TextNativeResult out;
    if (in.width <= 0 || in.height <= 0) return out;
    out.pixels.assign(static_cast<std::size_t>(in.width) * static_cast<std::size_t>(in.height), 0u);
    out.coverage.assign(out.pixels.size(), 0u);
    if (!in.tilemap || !in.control || !in.palette) return out;
    const auto& tr = *in.tilemap;
    const auto& cr = *in.control;
    if (tr.size() < 0x7000u || cr.size() < 0x10u) return out;

    const auto ctrl2 = be16(cr, 4);
    const auto ctrl5 = be16(cr, 10);
    const auto ctrl6 = be16(cr, 12);
    const auto ctrl7 = be16(cr, 14);
    out.available = true;
    if (ctrl6 & 0x04u) return out;
    const bool flip = (ctrl7 & 1u) != 0;

    for (int sy = 0; sy < in.height; ++sy) {
        const int sample_y = sy - in.offset_y;
        const int hw_y = sample_y + in.visible_y_start;
        for (int sx = 0; sx < in.width; ++sx) {
            const int sample_x = sx - in.offset_x;
            const int mx = text_source_x(sample_x, ctrl2, flip);
            int my = source_y(hw_y, ctrl5);
            if (flip) my = 0x1ff - my;
            const int tx = (mx >> 3) & 63;
            const int ty = (my >> 3) & 63;
            const std::size_t off = 0x4000u + static_cast<std::size_t>(ty * 64 + tx) * 2u;
            if (off + 1 >= tr.size()) continue;
            const auto attr = be16(tr, off);
            const auto code = static_cast<std::uint8_t>(attr & 0xffu);
            int px = mx & 7;
            int py = my & 7;
            if (attr & 0x4000u) px = 7 - px;
            if (attr & 0x8000u) py = 7 - py;
            const auto pen = text_pen(tr, code, px, py);
            if (!pen) continue;

            int out_x = sx;
            int out_y = sy;
            if (in.hud_layout) {
                const auto element = hud_element_for_text_cell(tx, ty);
                if (element != HudElement::None) {
                    const auto& presentation = in.hud_layout->elements[static_cast<std::size_t>(element)];
                    if (!presentation.visible) continue;
                    out_x += presentation.dx;
                    out_y += presentation.dy;
                }
            }
            if (out_x < 0 || out_x >= in.width || out_y < 0 || out_y >= in.height) continue;
            const auto bank = static_cast<std::uint16_t>((attr >> 8) & 0x3fu);
            const auto dst = static_cast<std::size_t>(out_y) * static_cast<std::size_t>(in.width) + static_cast<std::size_t>(out_x);
            out.pixels[dst] = palette_color(*in.palette, bank, pen);
            out.coverage[dst] = 1u;
            ++out.visible_pixels;
        }
    }
    return out;
}

Tc0100TextNativeCompare compare_tc0100scn_text_native(
    const std::vector<std::uint32_t>& original,
    const Tc0100TextNativeResult& native,
    int width,
    int height) {
    Tc0100TextNativeCompare out;
    const auto n = std::min(original.size(), native.pixels.size());
    out.pixels = n;
    for (std::size_t i = 0; i < n; ++i) {
        if (original[i] == native.pixels[i]) continue;
        ++out.pixel_mismatches;
        if (out.first_x < 0) {
            out.first_x = width > 0 ? static_cast<int>(i % static_cast<std::size_t>(width)) : -1;
            out.first_y = width > 0 ? static_cast<int>(i / static_cast<std::size_t>(width)) : -1;
            out.original_pixel = original[i];
            out.native_pixel = native.pixels[i];
        }
    }
    if (original.size() != native.pixels.size()) {
        out.pixel_mismatches += original.size() > n ? original.size() - n : native.pixels.size() - n;
    }
    (void)height;
    return out;
}

} // namespace chq
