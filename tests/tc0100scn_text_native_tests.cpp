#include "tc0100scn_text_native.h"

#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <vector>

static void require(bool ok, const char* msg) {
    if (!ok) { std::cerr << "FAIL: " << msg << '\n'; std::exit(1); }
}

int main() {
    std::vector<std::uint8_t> tilemap(0x10000, 0);
    std::vector<std::uint8_t> control(0x10, 0);
    std::array<std::uint16_t,4096> palette{};
    chq::HudLayoutState hud{};

    // Screen (0,0) at neutral controls maps to source text cell (2,1), pixel (0,0).
    const std::size_t cell = 0x4000u + static_cast<std::size_t>(1 * 64 + 2) * 2u;
    tilemap[cell] = 0x01;     // palette bank 1 in bits 8..13
    tilemap[cell + 1] = 0x01; // RAM character 1
    tilemap[0x6000u + 16u] = 0x80; // code1 row0 plane0, x0 = pen1
    palette[0x11] = 0x001f; // full red in xBBBBBGGGGGRRRRR

    chq::Tc0100TextNativeInput in{};
    in.tilemap = &tilemap;
    in.control = &control;
    in.palette = &palette;
    in.hud_layout = &hud;
    in.width = 320;
    in.height = 240;
    in.visible_y_start = 16;

    const auto out = chq::render_tc0100scn_text_native(in);
    require(out.available, "renderer should be available");
    require(out.pixels.size() == 320u * 240u, "pixel size");
    require(out.pixels[0] == 0xffff0000u, "neutral-origin glyph pixel");
    require(out.coverage[0] == 1u, "coverage pixel");

    const auto cmp = chq::compare_tc0100scn_text_native(out.pixels, out, 320, 240);
    require(cmp.pixel_mismatches == 0, "self compare should be exact");

    control[12] = 0x00; control[13] = 0x04; // ctrl6 text disable
    const auto disabled = chq::render_tc0100scn_text_native(in);
    require(disabled.available, "disabled renderer still available");
    require(disabled.visible_pixels == 0, "disabled text plane must be empty");

    std::cout << "tc0100scn_text_native_tests: PASS\n";
    return 0;
}
