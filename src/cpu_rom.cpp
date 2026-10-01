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
        for (std::size_t i = 0; i < 0x20000; ++i)
            result[base + i * 2] = lanes[lane][i];
    }
    return result;
}

Bytes interleave_sub(const std::array<Bytes, 2>& lanes) {
    Bytes result(0x20000);
    for (std::size_t lane = 0; lane < 2; ++lane) {
        if (lanes[lane].size() != 0x10000)
            throw std::runtime_error("Each sub CPU ROM must contain exactly 0x10000 bytes");
        for (std::size_t i = 0; i < 0x10000; ++i)
            result[i * 2 + lane] = lanes[lane][i];
    }
    return result;
}

static std::uint32_t crc32(const Bytes& bytes) {
    std::uint32_t crc = 0xffffffff;
    for (auto b : bytes) {
        crc ^= b;
        for (int i = 0; i < 8; ++i)
            crc = (crc >> 1) ^ ((crc & 1) ? 0xedb88320 : 0);
    }
    return ~crc;
}

static Bytes load_exact(const std::filesystem::path& path, std::size_t size,
                        std::uint32_t expected_crc, const char* label) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f) throw std::runtime_error(std::string("Missing ") + label + " ROM: " + path.string());
    if (f.tellg() != static_cast<std::streamoff>(size))
        throw std::runtime_error("Wrong ROM size: " + path.string());
    f.seekg(0);
    Bytes data(size);
    if (!f.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(size)))
        throw std::runtime_error("Cannot read ROM: " + path.string());
    if (crc32(data) != expected_crc)
        throw std::runtime_error("CRC mismatch (World chasehq set required): " + path.string());
    std::cout << "[CPU ROM] OK " << path.filename().string() << '\n';
    return data;
}

Bytes load_main_rom(const std::filesystem::path& directory) {
    const char* names[] = {"b52-130.36", "b52-136.29", "b52-131.37", "b52-129.30"};
    const std::uint32_t crcs[] = {0x4e7beb46, 0x2f414df0, 0xaa945d83, 0x0eaebc08};
    std::array<Bytes, 4> lanes;
    for (std::size_t i = 0; i < 4; ++i)
        lanes[i] = load_exact(directory / names[i], 0x20000, crcs[i], "main CPU");
    return interleave_main(lanes);
}

Bytes load_sub_rom(const std::filesystem::path& directory) {
    const char* names[] = {"b52-132.39", "b52-133.55"};
    const std::uint32_t crcs[] = {0xa2f54789, 0x12232f95};
    std::array<Bytes, 2> lanes;
    for (std::size_t i = 0; i < 2; ++i)
        lanes[i] = load_exact(directory / names[i], 0x10000, crcs[i], "sub CPU");
    return interleave_sub(lanes);
}

}
