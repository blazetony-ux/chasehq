#pragma once

// Standalone, side-effect-free C++17 prototype of the CPU-A sprite list transfer.
// Behaviour derived from RC4.7 disassembly; not integrated into ChaseHQ-Native.

#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace chq_shadow {
constexpr std::uint32_t kSpriteRamStart = 0xD00000;
constexpr std::uint32_t kStagingEnd = 0x102800;
constexpr std::size_t kSpriteCapacityRecords = 256;
constexpr std::size_t kSpriteRamLongwords = kSpriteCapacityRecords * 2;

struct TransferInput {
    std::uint16_t latched_records = 0;  // CPU-A [0x101844]
    std::uint32_t staged_start = 0;     // CPU-A [0x101846]
    std::vector<std::uint32_t> staged_words; // CPU-A big-endian longword values in ascending address order
    std::int16_t clear_dbra_counter = -1; // CPU-A [0x101812]; any negative value skips clearing
};

struct Write {
    std::uint32_t destination = 0;
    std::uint32_t value = 0;
};

// Model output is an ordered series of bus-addressed longword writes.
// It deliberately does not execute ROM instructions or call emulator code.
inline std::vector<Write> transfer(const TransferInput& in,
                                   std::array<std::uint32_t, kSpriteRamLongwords>& sprite_ram) {
    const auto n = static_cast<std::size_t>(in.latched_records);
    if (n > kSpriteCapacityRecords) throw std::invalid_argument("latched sprite count exceeds 256");
    if (in.staged_words.size() != n * 2) throw std::invalid_argument("expected exactly two longwords per staged sprite");
    if (in.staged_start != kStagingEnd - 8 * n) throw std::invalid_argument("staging pointer does not match latched record count");
    // At PC 0x40148, a zero count exits before the clear loop.
    if (n == 0) return {};
    const auto clear_records = (in.clear_dbra_counter < 0)
        ? std::size_t{0} : static_cast<std::size_t>(in.clear_dbra_counter) + 1;
    if (n + clear_records > kSpriteCapacityRecords)
        throw std::invalid_argument("copy and trailing clear exceed sprite RAM capacity");

    std::vector<Write> writes;
    writes.reserve((n + clear_records) * 2);
    for (std::size_t i = 0; i < n * 2; ++i) {
        sprite_ram[i] = in.staged_words[i];
        writes.push_back({kSpriteRamStart + static_cast<std::uint32_t>(4 * i), in.staged_words[i]});
    }
    for (std::size_t i = n * 2; i < (n + clear_records) * 2; ++i) {
        sprite_ram[i] = 0;
        writes.push_back({kSpriteRamStart + static_cast<std::uint32_t>(4 * i), 0});
    }
    return writes;
}

} // namespace chq_shadow
