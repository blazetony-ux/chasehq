#pragma once
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace chq {

struct RomSpec {
    std::string name;
    std::size_t expected_size;
};

struct LoadedRom {
    RomSpec info;
    std::vector<std::uint8_t> data;
};

class RomLoader {
public:
    explicit RomLoader(std::filesystem::path directory);
    bool load_required();
    const LoadedRom* find(const std::string& name) const;

private:
    bool load_one(const RomSpec& spec);

    std::filesystem::path directory_;
    std::vector<LoadedRom> roms_;
};

}
