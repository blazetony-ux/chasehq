#pragma once
#include "rom_loader.h"

#include <cstdint>
#include <filesystem>
#include <memory>
#include <vector>

namespace chq {

enum class SpriteFormat {
    Obj128x128,
    Obj64x128,
    Obj32x128,
    Invalid
};

struct SpriteInstance {
    int x = 0;
    int y = 0;
    int zoom_x = 128;
    int zoom_y = 128;
    std::uint16_t tile_number = 1;
    std::uint8_t color = 0x20;
    bool flip_x = false;
    bool flip_y = false;
    int priority = 0;
};

struct SceneState {
    int curve = 0;          // signed road curvature
    int horizon = 78;       // screen y
    int road_offset = 0;    // texture phase
    bool show_road = true;
    bool show_sprites = true;
    bool show_tiles = true;
    bool show_text = true;
    bool show_priority = false;
    bool show_prom_trace = false;
    bool use_reference_mixer = false;
    bool use_prom_mixer = true;
    bool show_prom_bits = false;
    bool show_road_probe = false;
    bool show_road_ram_map = false;
    int road_ram_scanline = 0;
    std::uint8_t prom_input_mask = 0xff;
};

class Machine {
public:
    bool load_roms(const std::filesystem::path& directory);

    const std::vector<std::uint8_t>& sprites_a() const { return sprites_a_; }
    const std::vector<std::uint8_t>& sprites_b() const { return sprites_b_; }
    const std::vector<std::uint8_t>& road_gfx() const { return road_gfx_; }
    const std::vector<std::uint8_t>& tile_gfx() const { return tile_gfx_; }
    const std::vector<std::uint8_t>& priority_prom() const { return priority_prom_; }
    const std::vector<std::uint8_t>& road_priority_prom() const { return road_priority_prom_; }

    std::uint16_t spritemap_word(std::size_t word_index) const;

    static SpriteFormat format_for_zoomx(int zoom_x);
    static const char* format_name(SpriteFormat format);

private:
    static std::vector<std::uint8_t> build_sprite_region(
        const LoadedRom& r0,
        const LoadedRom& r1,
        const LoadedRom& r2,
        const LoadedRom& r3);

    std::unique_ptr<RomLoader> loader_;

    std::vector<std::uint8_t> sprites_a_;
    std::vector<std::uint8_t> sprites_b_;
    std::vector<std::uint8_t> spritemap_;
    std::vector<std::uint8_t> road_gfx_;
    std::vector<std::uint8_t> tile_gfx_;
    std::vector<std::uint8_t> priority_prom_;
    std::vector<std::uint8_t> road_priority_prom_;
};

}
