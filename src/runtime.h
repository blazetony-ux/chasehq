#pragma once
#include "cpu_rom.h"
#include <array>
#include <fstream>
#include <string>
namespace chq {
enum class BusSpace { Main, Sub };
struct Region {
    const char* name;
    std::uint32_t base;
    Bytes bytes;
    bool readonly = false;
    std::uint64_t reads = 0, writes = 0;
};
class Bus {
public:
    explicit Bus(Bytes rom);
    void open_log(const std::filesystem::path&, std::uint64_t limit = 20000, bool all = false);
    std::uint32_t read(std::uint32_t address, unsigned size, BusSpace space = BusSpace::Main);
    void write(std::uint32_t address, std::uint32_t value, unsigned size, BusSpace space = BusSpace::Main);
    std::uint32_t peek(std::uint32_t address, unsigned size, BusSpace space = BusSpace::Main);
    void summary(std::ostream&) const;
    void dump(const std::filesystem::path&) const;
    bool executable(std::uint32_t pc) const;
    std::uint32_t pc = 0;
    std::array<std::uint16_t, 4096> palette{};
    std::vector<Region> regions;
private:
    Region* region(std::uint32_t, BusSpace);
    std::uint8_t read_byte(std::uint32_t, BusSpace);
    void write_byte(std::uint32_t, std::uint8_t, BusSpace);
    void trace(char, std::uint32_t, std::uint32_t, unsigned, BusSpace);
    std::uint16_t palette_address_ = 0, palette_latch_ = 0;
    std::ofstream log_;
    std::uint64_t log_limit_ = 20000, logged_ = 0, unmapped_reads_ = 0, unmapped_writes_ = 0;
    bool all_ = false;
};
class Runtime {
public:
    explicit Runtime(Bytes rom);
    ~Runtime();
    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;
    void reset();
    int run(int cycles);
    void irq4();
    void summary(std::ostream&) const;
    Bus bus;
    std::uint64_t cycles = 0, instructions = 0;
    std::string fault;
};
struct Options {
    std::filesystem::path roms = std::filesystem::path(CHASEHQ_PROJECT_DIR) / "roms/chasehq";
    std::filesystem::path logs = "logs";
    unsigned frames = 120;
    bool irq = false, all = false, scene_only = false, help = false;
};
Options parse_options(int argc, char** argv);
void print_help();
}
