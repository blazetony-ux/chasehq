#include "cpu_rom.h"
#include <fstream>
#include <iostream>
#include <stdexcept>
namespace chq {
Bytes interleave_main(const std::array<Bytes, 4>& lanes) {
    Bytes result(0x80000);
    for (std::size_t lane = 0; lane < 4; ++lane) {
        if (lanes[lane].size() != 0x20000)
            throw std::runtime_error("Each main CPU ROM must contain exactly 0x20000 bytes");
        const auto base = (lane / 2) * 0x40000 + lane % 2;
        for (std::size_t i = 0; i < 0x20000; ++i) result[base + i * 2] = lanes[lane][i];
    }
    return result;
}
static std::uint32_t crc32(const Bytes& bytes) {
    std::uint32_t crc = 0xffffffff;
    for (auto b : bytes) {
        crc ^= b;
        for (int i = 0; i < 8; ++i) crc = (crc >> 1) ^ ((crc & 1) ? 0xedb88320 : 0);
    }
    return ~crc;
}
Bytes load_main_rom(const std::filesystem::path& directory) {
    const char* names[] = {"b52-130.36", "b52-136.29", "b52-131.37", "b52-129.30"};
    const std::uint32_t crcs[] = {0x4e7beb46, 0x2f414df0, 0xaa945d83, 0x0eaebc08};
    std::array<Bytes, 4> lanes;
    for (std::size_t i = 0; i < 4; ++i) {
        const auto path = directory / names[i];
        std::ifstream f(path, std::ios::binary | std::ios::ate);
        if (!f) throw std::runtime_error("Missing main CPU ROM: " + path.string());
        if (f.tellg() != 0x20000) throw std::runtime_error("Wrong ROM size: " + path.string());
        f.seekg(0); lanes[i].resize(0x20000);
        if (!f.read(reinterpret_cast<char*>(lanes[i].data()), 0x20000))
            throw std::runtime_error("Cannot read ROM: " + path.string());
        if (crc32(lanes[i]) != crcs[i]) throw std::runtime_error("CRC mismatch (World chasehq set required): " + path.string());
        std::cout << "[CPU ROM] OK " << names[i] << '\n';
    }
    return interleave_main(lanes);
}
}
