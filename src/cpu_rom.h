#pragma once
#include <array>
#include <cstdint>
#include <filesystem>
#include <vector>
namespace chq {
using Bytes = std::vector<std::uint8_t>;
Bytes interleave_main(const std::array<Bytes, 4>& lanes);
Bytes load_main_rom(const std::filesystem::path& directory);
}
