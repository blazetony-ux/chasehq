#include "machine.h"
#include <iostream>

namespace chq {

std::vector<std::uint8_t> Machine::build_sprite_region(
    const LoadedRom& r0,
    const LoadedRom& r1,
    const LoadedRom& r2,
    const LoadedRom& r3) {

    // MAME ROM_LOAD64_WORD_SWAP at offsets 0,2,4,6.
    std::vector<std::uint8_t> out(0x200000, 0);
    const LoadedRom* lanes[4] = { &r0, &r1, &r2, &r3 };

    for (int lane = 0; lane < 4; ++lane) {
        const auto& src = lanes[lane]->data;

        for (std::size_t i = 0; i + 1 < src.size(); i += 2) {
            const std::size_t word = i / 2;
            const std::size_t dst =
                word * 8 + static_cast<std::size_t>(lane * 2);

            out[dst]     = src[i + 1];
            out[dst + 1] = src[i];
        }
    }

    return out;
}

bool Machine::load_roms(const std::filesystem::path& directory) {
    loader_ = std::make_unique<RomLoader>(directory);

    if (!loader_->load_required())
        return false;

    const auto* a0 = loader_->find("b52-34.5");
    const auto* a1 = loader_->find("b52-35.7");
    const auto* a2 = loader_->find("b52-36.9");
    const auto* a3 = loader_->find("b52-37.11");

    const auto* b0 = loader_->find("b52-30.4");
    const auto* b1 = loader_->find("b52-31.6");
    const auto* b2 = loader_->find("b52-32.8");
    const auto* b3 = loader_->find("b52-33.10");

    const auto* map = loader_->find("b52-38.34");
    const auto* road = loader_->find("b52-28.4");

    if (!a0 || !a1 || !a2 || !a3 ||
        !b0 || !b1 || !b2 || !b3 ||
        !map || !road)
        return false;

    sprites_a_ = build_sprite_region(*a0, *a1, *a2, *a3);
    sprites_b_ = build_sprite_region(*b0, *b1, *b2, *b3);
    spritemap_ = map->data;
    road_gfx_ = road->data;

    std::cout << "\nAssembled graphics hardware data:\n"
              << "  OBJ A graphics : " << sprites_a_.size() << " bytes\n"
              << "  OBJ B graphics : " << sprites_b_.size() << " bytes\n"
              << "  Spritemap      : " << spritemap_.size() << " bytes\n"
              << "  TC0150ROD ROM  : " << road_gfx_.size() << " bytes\n\n";

    return true;
}

std::uint16_t Machine::spritemap_word(std::size_t word_index) const {
    const std::size_t b = word_index * 2;
    if (b + 1 >= spritemap_.size())
        return 0xffff;

    return static_cast<std::uint16_t>(spritemap_[b]) |
           (static_cast<std::uint16_t>(spritemap_[b + 1]) << 8);
}

SpriteFormat Machine::format_for_zoomx(int zoom_x) {
    const int raw = (zoom_x - 1) & 0x7f;

    if (raw & 0x40)
        return SpriteFormat::Obj128x128;

    if (raw & 0x20)
        return SpriteFormat::Obj64x128;

    if ((raw & 0x60) == 0)
        return SpriteFormat::Obj32x128;

    return SpriteFormat::Invalid;
}

const char* Machine::format_name(SpriteFormat format) {
    switch (format) {
    case SpriteFormat::Obj128x128: return "128x128 / OBJ A";
    case SpriteFormat::Obj64x128:  return "64x128 / OBJ B";
    case SpriteFormat::Obj32x128:  return "32x128 / OBJ B";
    default:                       return "invalid";
    }
}

}
