#pragma once
// RC5 independent, CPU-free, fail-closed native candidates. No ROM, no replacements.
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
namespace chq::candidate {
constexpr std::size_t sprite_capacity = 256;
constexpr std::size_t road_entries = 104;
struct SpriteInput {
    std::uint16_t latched = 0;
    std::uint32_t staging_pointer = 0;
    std::vector<std::uint32_t> staging_longwords;
    std::int16_t clear_dbra = -1;
};
struct SpriteWrite { std::uint32_t address; std::uint32_t value; };
struct SpriteResult {
    std::array<std::uint32_t, sprite_capacity*2> ram{};
    std::vector<SpriteWrite> writes;
};
SpriteResult sprite_transfer(const SpriteInput& input,
    const std::array<std::uint32_t, sprite_capacity*2>& before);
struct RoadInput {
    std::array<std::uint16_t, road_entries> source{};
    std::array<std::uint16_t, road_entries> boundaries{};
    std::array<std::uint8_t, 1024> lookup{};
    std::uint16_t cf4 = 0, cf2 = 0, cf6 = 0;
};
struct RoadResult {
    std::array<std::uint16_t, road_entries> mutated_source{};
    std::array<std::uint16_t, road_entries> output{};
    std::size_t selected = 0;
};
RoadResult road_mask(const RoadInput& input);
struct Compare {
    bool exact = true;
    std::size_t bytes = 0, differences = 0;
    std::size_t first_difference = static_cast<std::size_t>(-1);
};
Compare compare_bytes(const std::vector<std::uint8_t>& actual,
                      const std::vector<std::uint8_t>& expected);
std::vector<std::uint8_t> be_bytes(const std::array<std::uint16_t, road_entries>& words);
std::vector<std::uint8_t> be_bytes(const std::array<std::uint32_t, sprite_capacity*2>& words);
}
