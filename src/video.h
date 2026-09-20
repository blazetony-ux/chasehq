#pragma once
#include "machine.h"

#include <cstdint>
#include <vector>

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

namespace chq {

class Video {
public:
    static constexpr int WIDTH = 320;
    static constexpr int HEIGHT = 240;

    bool initialise();
    void shutdown();

    bool process_events(
        bool& running,
        SceneState& scene,
        bool& changed);

    void draw_scene(
        const Machine& machine,
        const SceneState& scene);

private:
    static std::uint8_t sprite_pixel(
        const std::vector<std::uint8_t>& region,
        std::size_t tile_index,
        int x,
        int y);

    static std::uint32_t sprite_color(
        std::uint8_t palette_bank,
        std::uint8_t pen);

    static std::uint8_t road_pixel(
        const std::vector<std::uint8_t>& road,
        int tile,
        int logical_x);

    static std::uint32_t road_color(
        std::uint8_t pixel,
        int band,
        bool edge);

    static void draw_sky(
        std::vector<std::uint32_t>& pixels,
        int horizon);

    static void draw_road(
        std::vector<std::uint32_t>& pixels,
        const std::vector<std::uint8_t>& road,
        const SceneState& scene);

    static void draw_scaled_chunk(
        std::vector<std::uint32_t>& pixels,
        const std::vector<std::uint8_t>& region,
        std::uint16_t code,
        int x,
        int y,
        int width,
        int height,
        bool flip_x,
        bool flip_y,
        std::uint8_t palette_bank);

    static void draw_sprite(
        std::vector<std::uint32_t>& pixels,
        const Machine& machine,
        const SpriteInstance& sprite);

    void update_title(const SceneState& scene);
    void present(const std::vector<std::uint32_t>& pixels);

    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    SDL_Texture* texture_ = nullptr;
};

}
