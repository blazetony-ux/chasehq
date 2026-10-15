#include "tc0100scn_bg_native.h"

#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

static void put_be16(std::vector<std::uint8_t>& v, std::size_t off, std::uint16_t x) {
    v[off] = static_cast<std::uint8_t>(x >> 8);
    v[off + 1] = static_cast<std::uint8_t>(x);
}

int main() {
    std::vector<std::uint8_t> ram(0x10000, 0);
    std::vector<std::uint8_t> control(0x10, 0);
    std::vector<std::uint8_t> gfx(32 * 2, 0);
    std::array<std::uint16_t,4096> palette{};

    // At screen (0,0), normal origin selects source (16,8) -> tile (2,1), pixel (0,0).
    const std::size_t tile_off = static_cast<std::size_t>(1 * 64 + 2) * 4;
    put_be16(ram, tile_off, 0x0003); // palette bank 3
    put_be16(ram, tile_off + 2, 0x0001);
    gfx[32] = 0xa0; // tile1 pixel(0,0) pen A
    palette[(3u << 4) | 0xau] = static_cast<std::uint16_t>((31u << 0) | (7u << 5) | (3u << 10));

    chq::Tc0100BgNativeInput in{};
    in.tilemap = &ram;
    in.control = &control;
    in.tile_gfx = &gfx;
    in.palette = &palette;
    in.layer = 0;
    in.opaque = false;
    in.width = 320;
    in.height = 240;
    in.visible_y_start = 16;

    const auto out = chq::render_tc0100scn_bg_native(in);
    assert(out.available);
    assert(out.coverage[0] == 1);
    assert(out.pixels[0] != 0);

    auto original = out.pixels;
    auto coverage = out.coverage;
    const auto cmp = chq::compare_tc0100scn_bg_native(original, coverage, out, 320, 240);
    assert(cmp.pixel_mismatches == 0);
    assert(cmp.coverage_mismatches == 0);

    // RC2.1 regression: opaque bottom-layer pen 0 must participate in
    // coverage/priority while retaining a zero pixel value. RC2 returned
    // 0xff000000 here, which made physical BG1 mismatch on every sampled
    // frame when CTRL6 selected BG1 as the bottom layer.
    std::fill(ram.begin(), ram.end(), 0);
    std::fill(control.begin(), control.end(), 0);
    in.layer = 1;
    in.opaque = true;
    const auto opaque_zero = chq::render_tc0100scn_bg_native(in);
    assert(opaque_zero.available);
    assert(opaque_zero.coverage[0] == 1);
    assert(opaque_zero.pixels[0] == 0);

    // Disable BG1: renderer remains available but contributes nothing.
    put_be16(control, 12, 0x0002);
    const auto disabled_bg1 = chq::render_tc0100scn_bg_native(in);
    assert(disabled_bg1.available);
    assert(disabled_bg1.visible_pixels == 0);

    // Restore BG0 parameters for the existing disable test.
    std::fill(control.begin(), control.end(), 0);
    in.layer = 0;
    in.opaque = false;

    // Disable BG0: renderer remains available but contributes nothing.
    put_be16(control, 12, 0x0001);
    const auto disabled = chq::render_tc0100scn_bg_native(in);
    assert(disabled.available);
    assert(disabled.visible_pixels == 0);

    std::cout << "tc0100scn_bg_native_tests: PASS\n";
    return 0;
}
