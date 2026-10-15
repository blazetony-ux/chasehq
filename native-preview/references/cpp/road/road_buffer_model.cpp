// ChaseHQ-Native independent CPU-B 104-entry road-buffer *candidate*.
// Extracted from observed M68000 behaviour around PC 0x3B84..0x3BE6.
// This operates on supplied data, never starts an emulator or runs 68000 code.
// Input to model is the same-frame unmodified 104-word source buffer.
// Exit status 0: both generated buffers byte-match the supplied observations.
// Public-facing package includes NO ROM tables or game asset bytes.
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
constexpr std::size_t kEntries = 104;
constexpr std::size_t kWordBytes = kEntries * 2;
uint16_t be16(const std::vector<uint8_t>& bytes, std::size_t byteOffset) {
    return uint16_t((uint16_t(bytes.at(byteOffset)) << 8) | bytes.at(byteOffset + 1));
}
void set_be16(std::vector<uint8_t>& bytes, std::size_t offset, uint16_t value) {
    bytes.at(offset) = static_cast<uint8_t>(value >> 8);
    bytes.at(offset + 1) = static_cast<uint8_t>(value);
}
std::vector<uint8_t> load(const std::string& path, std::size_t required) {
    std::ifstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("Cannot open: " + path);
    std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(f)), {});
    if (bytes.size() != required) throw std::runtime_error("Unexpected file length: " + path + " (" + std::to_string(bytes.size()) + ")");
    return bytes;
}
struct Result {
    std::vector<uint8_t> inputAfter, roadOutput;
    std::size_t active{};
};
// Corresponds to 0x3BC2..0x3BE2 with 104 iterations, but is a fresh,
// ordinary C++17 data transform, independent of the original 68000 runtime.
Result generate(const std::vector<uint8_t>& sourceBefore,
                const std::vector<uint8_t>& boundaryTable,
                const std::vector<uint8_t>& materialLookup,
                uint16_t firstBoundary, uint16_t secondBoundary,
                uint16_t roadPaletteWord) {
    Result result{sourceBefore, std::vector<uint8_t>(kWordBytes, 0), 0};
    const uint16_t highByte = static_cast<uint16_t>(0xD200u + roadPaletteWord) & 0xFF00u;
    for (std::size_t i = 0; i < kEntries; ++i) {
        const uint16_t boundary = be16(boundaryTable, 2 * i);
        // CMP.W / BCS / BCC use unsigned 16-bit status. Precisely equal
        // to both comparisons in the ROM for the captured CPU-B state.
        const bool selected = firstBoundary >= boundary && secondBoundary < boundary;
        set_be16(result.roadOutput, 2 * i, selected ? 0x9800 : 0x8800);
        if (selected) {
            const uint16_t sourceIndex = be16(sourceBefore, 2 * i) & 0x03FFu;
            const uint16_t newWord = static_cast<uint16_t>(highByte | materialLookup.at(sourceIndex));
            set_be16(result.inputAfter, 2 * i, newWord);
            ++result.active;
        }
    }
    return result;
}
void compare(const char* name, const std::vector<uint8_t>& actual, const std::vector<uint8_t>& expected) {
    std::size_t mismatches = 0;
    for (std::size_t i = 0; i < actual.size(); ++i) {
        if (actual[i] != expected[i]) {
            if (mismatches < 10) {
                std::cerr << "Mismatch " << name << " byte " << i << " modeled=0x"
                          << std::hex << unsigned(actual[i]) << " observed=0x"
                          << unsigned(expected[i]) << std::dec << '\n';
            }
            ++mismatches;
        }
    }
    std::cout << name << ": " << actual.size() << " bytes, " << mismatches << " mismatches\n";
    if (mismatches != 0) throw std::runtime_error(std::string("Comparison failed: ") + name);
}
}
int main(int argc, char** argv) {
    if (argc != 7) {
        std::cerr << "Usage: road_model boundary.bin lookup.bin same-frame-control-input.bin "
                     "class06-decisions.bin class06-observed-input.bin class06-observed-output.bin\n";
        return 2;
    }
    try {
        const auto table = load(argv[1], 208);
        const auto lookup = load(argv[2], 1024);
        const auto source = load(argv[3], 208);
        const auto decisions = load(argv[4], 64);
        const auto observedInput = load(argv[5], 208);
        const auto observedOutput = load(argv[6], 208);
        // Decision RAM snapshot starts at 0x102CE0.
        const uint16_t cf2 = be16(decisions, 0x12);
        const uint16_t cf4 = be16(decisions, 0x14);
        const uint16_t cf6 = be16(decisions, 0x16);
        auto result = generate(source, table, lookup, cf4, cf2, cf6);
        std::cout << "cf4=0x" << std::hex << cf4 << " cf2=0x" << cf2
                  << " cf6=0x" << cf6 << std::dec << " selected=" << result.active << "/104\n";
        compare("mutated input", result.inputAfter, observedInput);
        compare("road output", result.roadOutput, observedOutput);
        std::cout << "PASS: independent generator matches captured inputs and outputs\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
}
