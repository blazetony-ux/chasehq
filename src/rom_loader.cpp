#include "rom_loader.h"
#include <fstream>
#include <iostream>

namespace chq {

RomLoader::RomLoader(std::filesystem::path directory)
    : directory_(std::move(directory)) {}

const LoadedRom* RomLoader::find(const std::string& name) const {
    for (const auto& rom : roms_) {
        if (rom.info.name == name)
            return &rom;
    }
    return nullptr;
}

bool RomLoader::load_one(const RomSpec& spec) {
    const auto path = directory_ / spec.name;
    std::ifstream f(path, std::ios::binary | std::ios::ate);

    if (!f) {
        std::cerr << "[ROM] MISSING  " << spec.name << "\n";
        return false;
    }

    const auto size = f.tellg();
    if (size < 0 || static_cast<std::size_t>(size) != spec.expected_size) {
        std::cerr << "[ROM] BAD SIZE " << spec.name
                  << " expected " << spec.expected_size
                  << ", got " << size << "\n";
        return false;
    }

    f.seekg(0);

    LoadedRom loaded{
        spec,
        std::vector<std::uint8_t>(spec.expected_size)
    };

    if (!f.read(reinterpret_cast<char*>(loaded.data.data()),
                static_cast<std::streamsize>(loaded.data.size()))) {
        std::cerr << "[ROM] READ ERR " << spec.name << "\n";
        return false;
    }

    std::cout << "[ROM] OK      " << spec.name << "\n";
    roms_.push_back(std::move(loaded));
    return true;
}

bool RomLoader::load_required() {
    static const RomSpec required[] = {
        {"b52-34.5",  524288},
        {"b52-35.7",  524288},
        {"b52-36.9",  524288},
        {"b52-37.11", 524288},

        {"b52-30.4",  524288},
        {"b52-31.6",  524288},
        {"b52-32.8",  524288},
        {"b52-33.10", 524288},

        {"b52-38.34", 524288},
        {"b52-28.4",  524288}
    };

    roms_.clear();

    std::cout << "ROM directory:\n  " << directory_.string() << "\n\n";

    bool ok = true;
    for (const auto& spec : required) {
        if (!load_one(spec))
            ok = false;
    }
    return ok;
}

}
