#include "runtime.h"
#include "m68k.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <functional>
static void check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
static void rejects(const std::function<void()>& f, const char* message) {
    bool rejected = false;
    try { f(); } catch (const std::exception&) { rejected = true; }
    check(rejected, message);
}
static void word(chq::Bytes& rom, unsigned a, unsigned v) {
    rom.at(a) = static_cast<std::uint8_t>(v >> 8); rom.at(a + 1) = static_cast<std::uint8_t>(v);
}
static chq::Bytes program() {
    chq::Bytes rom(0x80000);
    word(rom, 0, 0x0011); word(rom, 2, 0x0000); // SP = one past work RAM
    word(rom, 4, 0); word(rom, 6, 0x0100);
    // MOVE.W #$1234,$100000; MOVE.B #$56,$100001;
    // MOVE.L #$89abcdef,$d00000; MOVEQ #3,D0; ADDQ.W #1,D0;
    // STOP #$2700 (does not consume additional instructions).
    unsigned pc = 0x100;
    for (auto v : {0x33fc,0x1234,0x0010,0x0000,0x13fc,0x0056,0x0010,0x0001,
                   0x23fc,0x89ab,0xcdef,0x00d0,0x0000,0x7003,0x5240,0x4e72,0x2700}) {
        word(rom, pc, v); pc += 2;
    }
    return rom;
}
int main() {
    try {
        std::cout << std::unitbuf << "[test] ROM reconstruction\n";
        std::array<chq::Bytes, 4> lanes;
        for (unsigned j = 0; j < 4; ++j) {
            lanes[j].resize(0x20000);
            for (unsigned i = 0; i < 0x20000; ++i) lanes[j][i] = static_cast<std::uint8_t>(i * 13 + j * 61);
        }
        const auto interleaved = chq::interleave_main(lanes);
        check(interleaved.size() == 0x80000, "region size");
        for (unsigned i = 0; i < 0x20000; ++i) {
            check(interleaved[i*2] == lanes[0][i] && interleaved[i*2+1] == lanes[1][i], "lower lanes");
            check(interleaved[0x40000+i*2] == lanes[2][i] && interleaved[0x40001+i*2] == lanes[3][i], "upper lanes");
        }
        lanes[0].pop_back();
        rejects([&] { chq::interleave_main(lanes); }, "short lane accepted");
        const auto fixtures = std::filesystem::current_path() / "runtime-test-output";
        std::filesystem::create_directories(fixtures / "missing");
        rejects([&] { chq::load_main_rom(fixtures / "missing"); }, "missing ROM accepted");
        {
            std::ofstream f(fixtures / "b52-130.36", std::ios::binary);
            f.put(0);
        }
        rejects([&] { chq::load_main_rom(fixtures); }, "short file accepted");
        {
            chq::Bytes zeros(0x20000);
            std::ofstream f(fixtures / "b52-130.36", std::ios::binary);
            f.write(reinterpret_cast<const char*>(zeros.data()), zeros.size());
        }
        rejects([&] { chq::load_main_rom(fixtures); }, "bad checksum accepted");
        {
            std::cout << "[test] Bus\n";
            chq::Bus bus(program());
            bus.write(0x100000, 0x12345678, 4);
            check(bus.read(0x100000, 2) == 0x1234 && bus.read(0x100003, 1) == 0x78, "big endian");
            bus.write(0x1100001, 0xab, 1);
            check(bus.peek(0x100000, 4) == 0x12ab5678, "24 bit wrap and byte lane");
            bus.write(0x100, 0xffff, 2);
            check(bus.peek(0x100, 2) == 0x33fc, "ROM changed");
            bus.write(0x107fff, 0xcafe, 2);
            check(bus.peek(0x107fff, 2) == 0xcafe, "region boundary");
            bus.write(0x108010, 0xface, 2);
            check(bus.peek(0x108010, 2, chq::BusSpace::Sub) == 0xface, "shared RAM alias");
            bus.write(0x800000, 0x1234, 2, chq::BusSpace::Sub);
            check(bus.peek(0x800000, 2) == 0, "road aliases main control");
            check(bus.peek(0x800000, 2, chq::BusSpace::Sub) == 0x1234, "sub road map");
            bus.write(0xa00000, 3, 2); bus.write(0xa00002, 0x7abc, 2);
            check(bus.palette[3] == 0x7abc && bus.read(0xa00002, 2) == 0x7abc, "palette indirect register");
            bus.write(0xa00003, 0xef, 1);
            check(bus.palette[3] == 0x7aef, "palette byte lane");
            check(bus.read(0xdead00, 2) == 0xffff, "open bus");
            bus.write(0xdead00, 1, 4);
            rejects([&] { bus.read(0, 3); }, "invalid bus size");
            rejects([&] { chq::Bus invalid(chq::Bytes(4)); }, "short region accepted");
        }
        {
            chq::Bus bus(program());
            bus.open_log(fixtures / "bus.log", 2);
            bus.read(0x100, 2); // ROM fetch is excluded from the default trace.
            bus.write(0x100000, 0xcafe, 2);
            bus.read(0x100000, 2);
            bus.write(0x100002, 0xface, 2); // Still counted after the log cap.
            bus.dump(fixtures / "ram");
            check(std::filesystem::file_size(fixtures / "ram/palette.bin") == 8192, "palette dump size");
            check(std::filesystem::file_size(fixtures / "ram/road.bin") == 8192, "road dump size");
        }
        {
            std::ifstream f(fixtures / "bus.log");
            const std::string log((std::istreambuf_iterator<char>(f)), {});
            check(log.find("cafe") != std::string::npos && log.find("trace limit reached") != std::string::npos,
                  "bus trace content/cap");
            check(log.find("face") == std::string::npos, "trace exceeds cap");
        }
        {
            std::cout << "[test] CPU construction\n";
            chq::Runtime runtime(program());
            std::cout << "[test] CPU reset/execute\n";
            runtime.reset(); runtime.run(1000);
            check(runtime.bus.peek(0x100000, 2) == 0x1256, "CPU MOVE word/byte");
            check(runtime.bus.peek(0xd00000, 4) == 0x89abcdef, "CPU MOVE long to sprite RAM");
            check(m68k_get_reg(nullptr, M68K_REG_D0) == 4, "CPU arithmetic");
            const auto count = runtime.instructions;
            runtime.run(1000);
            check(runtime.instructions == count, "STOP continues instructions");
            rejects([&] { chq::Runtime second(program()); }, "concurrent CPU accepted");
        }
        {
            std::cout << "[test] Interrupt\n";
            auto rom = program();
            // IRQ4 autovector -> handler at 0x200: MOVEQ #7,D1; RTE.
            word(rom, 0x70, 0); word(rom, 0x72, 0x200);
            word(rom, 0x100, 0x4e72); word(rom, 0x102, 0x2000);
            word(rom, 0x104, 0x60fe);
            word(rom, 0x200, 0x7207); word(rom, 0x202, 0x4e73);
            chq::Runtime runtime(std::move(rom));
            runtime.reset(); runtime.run(100); runtime.irq4(); runtime.run(300);
            check(m68k_get_reg(nullptr, M68K_REG_D1) == 7, "IRQ4 did not wake STOP");
            check(m68k_get_reg(nullptr, M68K_REG_SP) == 0x110000, "RTE stack restore");
        }
        {
            auto rom = program(); word(rom, 6, 0x101);
            chq::Runtime runtime(std::move(rom));
            rejects([&] { runtime.reset(); }, "odd reset PC accepted");
        }
        std::cout << "PASS: interleave, bus lanes/boundaries, ROM protection, shared/road maps, palette, CPU execution, STOP, IRQ/RTE, reset validation\n";
        return 0;
    } catch (const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}
