#include "sprite_transfer_model.hpp"

#include <array>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <algorithm>

using namespace chq_shadow;

static void require(bool ok, const std::string& what) {
    if (!ok) throw std::runtime_error(what);
}

static std::vector<std::uint8_t> read_file(const std::string& path, std::size_t expect) {
    std::ifstream f(path, std::ios::binary);
    require(f.good(), "cannot open " + path);
    std::vector<std::uint8_t> out((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    require(out.size() == expect, path + " has unexpected size " + std::to_string(out.size()));
    return out;
}

static std::uint16_t u16(const std::vector<std::uint8_t>& v, std::size_t pos) {
    return (static_cast<std::uint16_t>(v.at(pos)) << 8) | v.at(pos+1);
}

static std::uint32_t u32(const std::vector<std::uint8_t>& v, std::size_t pos) {
    return (static_cast<std::uint32_t>(u16(v,pos)) << 16) | u16(v,pos+2);
}

static std::int16_t s16(const std::vector<std::uint8_t>& v, std::size_t pos) {
    const auto raw = u16(v,pos);
    const std::int32_t signedValue = raw < 0x8000u ? raw : static_cast<std::int32_t>(raw) - 65536;
    return static_cast<std::int16_t>(signedValue);
}

static std::array<std::uint32_t, kSpriteRamLongwords> parse_ram(const std::vector<std::uint8_t>& bytes) {
    std::array<std::uint32_t, kSpriteRamLongwords> result{};
    for (std::size_t i=0;i<result.size();++i) result[i] = u32(bytes,i*4);
    return result;
}

static void run_case(const std::string& base, int case_n,
                     std::array<std::uint32_t, kSpriteRamLongwords>& previous_ram) {
    const std::string prefix = base + "/rc47-r024-";
    const auto stage = read_file(prefix + "stage" + std::to_string(case_n) + ".bin",2048);
    const auto meta  = read_file(prefix + "metadata" + std::to_string(case_n) + ".bin",10);
    const auto clear = read_file(prefix + "clear" + std::to_string(case_n) + ".bin",2);
    const auto expected_bytes = read_file(prefix + "sprite-after" + std::to_string(case_n) + ".bin",2048);
    const auto expected = parse_ram(expected_bytes);

    const std::uint16_t current_n = u16(meta,2);
    const std::uint16_t count = u16(meta,4);
    const std::uint32_t source = u32(meta,6);
    const std::int16_t clear_dbra = s16(clear,0);
    require(count == case_n, "unexpected latched count in case " + std::to_string(case_n));
    require(source >= 0x102000 && source <= kStagingEnd, "invalid source pointer");
    require(source - 0x102000 + count*8 <= stage.size(), "source range outside captured memory");

    TransferInput input;
    input.latched_records=count;
    input.staged_start=source;
    input.clear_dbra_counter=clear_dbra;
    for (std::size_t offset = source - 0x102000; offset < source - 0x102000 + count*8; offset += 4)
        input.staged_words.push_back(u32(stage,offset));

    auto output = previous_ram;
    const auto writes = transfer(input,output); // actual standalone C++ implementation
    const auto expected_writes = (count + (clear_dbra >= 0 && count > 0 ? clear_dbra+1 : 0))*2;
    require(writes.size() == static_cast<std::size_t>(expected_writes), "unexpected write count");

    std::size_t mismatched=0;
    std::size_t changed_bytes=0;
    std::size_t first_mismatch=0;
    for (std::size_t i=0;i<output.size();++i) {
        const auto before=previous_ram[i];
        const auto actual=output[i];
        const auto expected_value=expected[i];
        if (before != expected_value) {
            for (int k=0;k<4;k++) changed_bytes += ((before >> (8*k))&255) != ((expected_value >> (8*k))&255);
        }
        if (actual != expected_value) {
            if (mismatched == 0) first_mismatch=i*4;
            for (int k=0;k<4;k++) mismatched += ((actual >> (8*k))&255) != ((expected_value >> (8*k))&255);
        }
    }
    if (mismatched) throw std::runtime_error("case " + std::to_string(case_n) + " differs at offset " + std::to_string(first_mismatch));
    previous_ram = expected; // actual emulated post-case RAM becomes the next input state
    std::cout << "PASS case=" << case_n << " current_n=" << current_n
              << " latched_n=" << count << " source=0x" << std::hex << source << std::dec
              << " clear_dbra=" << clear_dbra << " predicted_writes=" << writes.size()
              << " changed_output_bytes=" << changed_bytes
              << " mismatched_bytes=" << mismatched << " compared_bytes=2048\n";
}

int main(int argc, char** argv) {
    try {
        if (argc != 2) { std::cerr << "Usage: snapshot_shadow_tests fixtures_directory\n"; return 2; }
        const std::string base=argv[1];
        auto initial = parse_ram(read_file(base+"/rc47-r024-sprite-before45.bin",2048));
        for (int n: {45,44,50}) run_case(base,n,initial);
        std::cout << "PASS independent C++ model vs emulated full sprite RAM: 3 snapshots / 6144 bytes / 0 mismatches\n";
        std::cout << "NOTE offline snapshot agreement; not yet integrated live shadow validation\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "FAIL: " << e.what() << "\n";
        return 1;
    }
}
