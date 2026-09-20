#include "video.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <vector>

namespace chq {

bool Video::initialise() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return false;
    }

    window_ = SDL_CreateWindow(
        "Chase H.Q. Native v0.8 - Synthetic Scene Preview",
        WIDTH * 3,
        HEIGHT * 3,
        SDL_WINDOW_RESIZABLE);

    if (!window_) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        return false;
    }

    renderer_ = SDL_CreateRenderer(window_, nullptr);
    if (!renderer_) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << "\n";
        return false;
    }

    texture_ = SDL_CreateTexture(
        renderer_,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        WIDTH,
        HEIGHT);

    if (!texture_) {
        std::cerr << "SDL_CreateTexture failed: " << SDL_GetError() << "\n";
        return false;
    }

    SDL_SetTextureScaleMode(texture_, SDL_SCALEMODE_NEAREST);
    return true;
}

bool Video::process_events(
    bool& running,
    SceneState& scene,
    bool& changed) {

    changed = false;
    SDL_Event e{};

    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT)
            running = false;

        if (e.type != SDL_EVENT_KEY_DOWN)
            continue;

        const int step =
            (e.key.mod & SDL_KMOD_SHIFT) ? 8 : 2;

        switch (e.key.scancode) {
        case SDL_SCANCODE_ESCAPE:
            running = false;
            break;

        case SDL_SCANCODE_LEFT:
            scene.curve = std::max(-160, scene.curve - step);
            changed = true;
            break;

        case SDL_SCANCODE_RIGHT:
            scene.curve = std::min(160, scene.curve + step);
            changed = true;
            break;

        case SDL_SCANCODE_UP:
            scene.horizon = std::max(35, scene.horizon - step);
            changed = true;
            break;

        case SDL_SCANCODE_DOWN:
            scene.horizon = std::min(125, scene.horizon + step);
            changed = true;
            break;

        case SDL_SCANCODE_A:
            scene.road_offset = (scene.road_offset - step * 4) & 0x3ff;
            changed = true;
            break;

        case SDL_SCANCODE_D:
            scene.road_offset = (scene.road_offset + step * 4) & 0x3ff;
            changed = true;
            break;

        case SDL_SCANCODE_R:
            scene.show_road = !scene.show_road;
            changed = true;
            break;

        case SDL_SCANCODE_T:
            scene.show_sprites = !scene.show_sprites;
            changed = true;
            break;

        case SDL_SCANCODE_HOME:
            scene = SceneState{};
            changed = true;
            break;

        default:
            break;
        }
    }

    return running;
}

std::uint8_t Video::sprite_pixel(
    const std::vector<std::uint8_t>& region,
    std::size_t tile_index,
    int x,
    int y) {

    const std::size_t base_bit = tile_index * 1024;
    std::uint8_t value = 0;

    for (int plane = 0; plane < 4; ++plane) {
        const std::size_t bit_index =
            base_bit +
            static_cast<std::size_t>(
                plane * 16 + x + y * 64);

        const std::size_t byte_index = bit_index >> 3;
        if (byte_index >= region.size())
            continue;

        const int bit_in_byte =
            7 - static_cast<int>(bit_index & 7);

        const std::uint8_t bit =
            static_cast<std::uint8_t>(
                (region[byte_index] >> bit_in_byte) & 1);

        value |= static_cast<std::uint8_t>(bit << plane);
    }

    return value;
}

std::uint32_t Video::sprite_color(
    std::uint8_t palette_bank,
    std::uint8_t pen) {

    if (pen == 0)
        return 0;

    const std::uint8_t phase =
        static_cast<std::uint8_t>((palette_bank * 29) & 0xff);

    const std::uint8_t r =
        static_cast<std::uint8_t>(45 + ((pen * 51 + phase) % 200));

    const std::uint8_t g =
        static_cast<std::uint8_t>(45 + ((pen * 89 + phase / 2) % 200));

    const std::uint8_t b =
        static_cast<std::uint8_t>(45 + ((pen * 137 + phase / 3) % 200));

    return 0xff000000u |
           (static_cast<std::uint32_t>(r) << 16) |
           (static_cast<std::uint32_t>(g) << 8) |
           b;
}

// The TC0150ROD ROM stores 2bpp pixels in 16-bit words.
// A 0x200-byte tile is 256 words = two 1024-pixel lines:
// logical_x 0..1023   = edge line
// logical_x 1024..2047 = body line
std::uint8_t Video::road_pixel(
    const std::vector<std::uint8_t>& road,
    int tile,
    int logical_x) {

    tile &= 0x3ff;
    logical_x &= 0x7ff;

    const std::size_t word_index =
        static_cast<std::size_t>(tile) * 0x100 +
        static_cast<std::size_t>(logical_x >> 3);

    const std::size_t byte_index = word_index * 2;
    if (byte_index + 1 >= road.size())
        return 0;

    const std::uint16_t word =
        static_cast<std::uint16_t>(road[byte_index]) |
        (static_cast<std::uint16_t>(road[byte_index + 1]) << 8);

    const int bit = 7 - (logical_x & 7);

    return static_cast<std::uint8_t>(
        (((word >> (bit + 8)) & 1) << 1) |
         ((word >> bit) & 1));
}

std::uint32_t Video::road_color(
    std::uint8_t pixel,
    int band,
    bool edge) {

    if (edge) {
        static constexpr std::uint32_t edge_cols[4] = {
            0xff355b2du, 0xffeeeeeeu, 0xffb73333u, 0xfff0d36cu
        };
        return edge_cols[pixel & 3];
    }

    const bool alt = (band & 1) != 0;
    static constexpr std::uint32_t body_a[4] = {
        0xff252525u, 0xff444444u, 0xff777777u, 0xffeeeeeeu
    };
    static constexpr std::uint32_t body_b[4] = {
        0xff282828u, 0xff4b4b4bu, 0xff707070u, 0xffd8d8d8u
    };

    return alt ? body_b[pixel & 3] : body_a[pixel & 3];
}

void Video::draw_sky(
    std::vector<std::uint32_t>& pixels,
    int horizon) {

    for (int y = 0; y < HEIGHT; ++y) {
        std::uint32_t c;

        if (y < horizon) {
            const int t = (y * 80) / std::max(1, horizon);
            const std::uint8_t r = static_cast<std::uint8_t>(22 + t / 3);
            const std::uint8_t g = static_cast<std::uint8_t>(48 + t / 2);
            const std::uint8_t b = static_cast<std::uint8_t>(92 + t);
            c = 0xff000000u |
                (static_cast<std::uint32_t>(r) << 16) |
                (static_cast<std::uint32_t>(g) << 8) |
                b;
        } else {
            c = 0xff31552bu;
        }

        for (int x = 0; x < WIDTH; ++x)
            pixels[y * WIDTH + x] = c;
    }
}

void Video::draw_road(
    std::vector<std::uint32_t>& pixels,
    const std::vector<std::uint8_t>& road,
    const SceneState& scene) {

    const int horizon = scene.horizon;

    for (int y = horizon; y < HEIGHT; ++y) {
        const int depth = y - horizon;
        const int denom = std::max(1, HEIGHT - horizon - 1);

        // Perspective: road widens quadratically toward the viewer.
        const double p =
            static_cast<double>(depth) /
            static_cast<double>(denom);

        const int half_width =
            10 + static_cast<int>(150.0 * p * p);

        const int curve_shift =
            static_cast<int>(
                scene.curve * (1.0 - p) * (1.0 - p));

        const int center =
            WIDTH / 2 + curve_shift;

        const int left = center - half_width;
        const int right = center + half_width;

        // Higher road tile numbers are generally wider in Chase H.Q.'s data.
        const int tile =
            std::clamp(
                1 + static_cast<int>(p * 0x1ff),
                1,
                0x1ff);

        const int band =
            ((depth + scene.road_offset / 4) / 12) & 1;

        // Fill verge.
        for (int x = 0; x < WIDTH; ++x) {
            if (x >= left && x <= right)
                continue;

            // Use edge ROM line around the road boundaries.
            const int d =
                x < left ? left - x : x - right;

            if (d < 22) {
                const int edge_src =
                    x < left
                        ? (511 - d * 10 + scene.road_offset)
                        : (512 + d * 10 + scene.road_offset);

                const auto pix =
                    road_pixel(road, tile, edge_src);

                pixels[y * WIDTH + x] =
                    road_color(pix, band, true);
            }
        }

        if (left > right)
            continue;

        // Road body samples the second 1024-pixel line.
        const int span = std::max(1, right - left + 1);

        for (int x = std::max(0, left);
             x <= std::min(WIDTH - 1, right);
             ++x) {

            const int rel = x - left;

            int src =
                1024 +
                (rel * 1023) / span;

            src =
                1024 +
                ((src - 1024 + scene.road_offset) & 0x3ff);

            const auto pix =
                road_pixel(road, tile, src);

            pixels[y * WIDTH + x] =
                road_color(pix, band, false);
        }
    }
}

void Video::draw_scaled_chunk(
    std::vector<std::uint32_t>& pixels,
    const std::vector<std::uint8_t>& region,
    std::uint16_t code,
    int x,
    int y,
    int width,
    int height,
    bool flip_x,
    bool flip_y,
    std::uint8_t palette_bank) {

    if (width <= 0 || height <= 0)
        return;

    const std::size_t tile =
        static_cast<std::size_t>(code & 0x3fff);

    for (int dy = 0; dy < height; ++dy) {
        int sy = std::clamp((dy * 16) / height, 0, 15);
        if (flip_y)
            sy = 15 - sy;

        for (int dx = 0; dx < width; ++dx) {
            int sx = std::clamp((dx * 16) / width, 0, 15);
            if (flip_x)
                sx = 15 - sx;

            const auto pen =
                sprite_pixel(region, tile, sx, sy);

            if (pen == 0)
                continue;

            const int px = x + dx;
            const int py = y + dy;

            if (px < 0 || px >= WIDTH ||
                py < 0 || py >= HEIGHT)
                continue;

            pixels[py * WIDTH + px] =
                sprite_color(palette_bank, pen);
        }
    }
}

void Video::draw_sprite(
    std::vector<std::uint32_t>& pixels,
    const Machine& machine,
    const SpriteInstance& sprite) {

    const SpriteFormat format =
        Machine::format_for_zoomx(sprite.zoom_x);

    int cols = 0;
    int rows = 8;
    std::size_t map_offset = 0;
    const std::vector<std::uint8_t>* gfx = nullptr;

    switch (format) {
    case SpriteFormat::Obj128x128:
        cols = 8;
        map_offset =
            static_cast<std::size_t>(sprite.tile_number) << 6;
        gfx = &machine.sprites_a();
        break;

    case SpriteFormat::Obj64x128:
        cols = 4;
        map_offset =
            (static_cast<std::size_t>(sprite.tile_number) << 5) +
            0x20000;
        gfx = &machine.sprites_b();
        break;

    case SpriteFormat::Obj32x128:
        cols = 2;
        map_offset =
            (static_cast<std::size_t>(sprite.tile_number) << 4) +
            0x30000;
        gfx = &machine.sprites_b();
        break;

    default:
        return;
    }

    int base_x = sprite.x;
    int base_y = sprite.y + (128 - sprite.zoom_y);

    for (int j = 0; j < rows; ++j) {
        for (int k = 0; k < cols; ++k) {
            const int map_x =
                sprite.flip_x ? (cols - 1 - k) : k;

            const int map_y =
                sprite.flip_y ? (rows - 1 - j) : j;

            const auto code =
                machine.spritemap_word(
                    map_offset +
                    static_cast<std::size_t>(
                        map_x + map_y * cols));

            if (code == 0xffff)
                continue;

            const int cur_x =
                base_x + (k * sprite.zoom_x) / cols;

            const int cur_y =
                base_y + (j * sprite.zoom_y) / rows;

            const int next_x =
                base_x + ((k + 1) * sprite.zoom_x) / cols;

            const int next_y =
                base_y + ((j + 1) * sprite.zoom_y) / rows;

            draw_scaled_chunk(
                pixels,
                *gfx,
                code,
                cur_x,
                cur_y,
                next_x - cur_x,
                next_y - cur_y,
                sprite.flip_x,
                sprite.flip_y,
                sprite.color);
        }
    }
}

void Video::update_title(const SceneState& scene) {
    std::ostringstream ss;

    ss << "Chase H.Q. Native v0.8 SYNTHETIC PREVIEW | "
       << "curve " << scene.curve
       << " | horizon " << scene.horizon
       << " | road phase " << scene.road_offset
       << " | road " << (scene.show_road ? "ON" : "OFF")
       << " | sprites " << (scene.show_sprites ? "ON" : "OFF");

    SDL_SetWindowTitle(window_, ss.str().c_str());
}

void Video::draw_scene(
    const Machine& machine,
    const SceneState& scene) {

    std::vector<std::uint32_t> pixels(
        WIDTH * HEIGHT,
        0xff000000u);

    draw_sky(pixels, scene.horizon);

    if (scene.show_road)
        draw_road(pixels, machine.road_gfx(), scene);

    if (scene.show_sprites) {
        // Far vehicle.
        draw_sprite(
            pixels,
            machine,
            SpriteInstance{
                144, 105,
                64, 52,
                0x0a2,
                0x31,
                false, false,
                0
            });

        // Road-side sign.
        draw_sprite(
            pixels,
            machine,
            SpriteInstance{
                228, 95,
                72, 72,
                0x11c,
                0x44,
                false, false,
                1
            });

        // Medium-distance car using the confirmed rear-quarter view.
        draw_sprite(
            pixels,
            machine,
            SpriteInstance{
                80, 118,
                96, 82,
                0x003,
                0x28,
                false, false,
                1
            });

        // Player/near car.
        draw_sprite(
            pixels,
            machine,
            SpriteInstance{
                96, 135,
                128, 118,
                0x001,
                0x20,
                false, false,
                2
            });
    }

    update_title(scene);
    present(pixels);
}

void Video::present(
    const std::vector<std::uint32_t>& pixels) {

    SDL_UpdateTexture(
        texture_,
        nullptr,
        pixels.data(),
        WIDTH * static_cast<int>(sizeof(std::uint32_t)));

    SDL_SetRenderDrawColor(
        renderer_,
        0, 0, 0, 255);

    SDL_RenderClear(renderer_);
    SDL_RenderTexture(
        renderer_,
        texture_,
        nullptr,
        nullptr);

    SDL_RenderPresent(renderer_);
}

void Video::shutdown() {
    if (texture_)
        SDL_DestroyTexture(texture_);

    if (renderer_)
        SDL_DestroyRenderer(renderer_);

    if (window_)
        SDL_DestroyWindow(window_);

    texture_ = nullptr;
    renderer_ = nullptr;
    window_ = nullptr;

    SDL_Quit();
}

}

