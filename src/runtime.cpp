#include "runtime.h"
#include "m68k.h"
#include <iomanip>
#include <iostream>
#include <stdexcept>

// Musashi is process-global. This prototype deliberately permits one runtime.
static chq::Runtime* active = nullptr;
extern "C" unsigned int m68k_read_memory_8(unsigned int a) { return active->bus.read(a, 1); }
extern "C" unsigned int m68k_read_memory_16(unsigned int a) { return active->bus.read(a, 2); }
extern "C" unsigned int m68k_read_memory_32(unsigned int a) { return active->bus.read(a, 4); }
extern "C" void m68k_write_memory_8(unsigned int a, unsigned int v) { active->bus.write(a, v, 1); }
extern "C" void m68k_write_memory_16(unsigned int a, unsigned int v) { active->bus.write(a, v, 2); }
extern "C" void m68k_write_memory_32(unsigned int a, unsigned int v) { active->bus.write(a, v, 4); }
static void instruction(unsigned int pc) {
    active->bus.pc = pc;
    ++active->instructions;
    if (!active->bus.executable(pc)) {
        active->fault = "PC outside executable ROM/work RAM (see summary)";
        m68k_end_timeslice();
    }
}
static int interrupt_ack(int) { m68k_set_irq(0); return M68K_INT_ACK_AUTOVECTOR; }

namespace chq {
Bus::Bus(Bytes rom) {
    if (rom.size() != 0x80000) throw std::runtime_error("Main ROM region must be 0x80000 bytes");
    regions = {
        {"rom", 0x000000, std::move(rom), true},
        {"work_low", 0x100000, Bytes(0x8000)},
        {"shared", 0x108000, Bytes(0x4000)},
        {"work_high", 0x10c000, Bytes(0x4000)},
        {"io_stub", 0x400000, Bytes(4, 0xff)},
        {"cpu_control_stub", 0x800000, Bytes(2)},
        {"sound_stub", 0x820000, Bytes(4)},
        {"palette_registers", 0xa00000, Bytes(8)},
        {"tilemap", 0xc00000, Bytes(0x10000)},
        {"tile_control", 0xc20000, Bytes(0x10)},
        {"sprites", 0xd00000, Bytes(0x800)},
        {"motor_stub", 0xe00000, Bytes(0x400)},
        {"sub_work", 0x100000, Bytes(0x4000)},
        {"road", 0x800000, Bytes(0x2000)}
    };
}
Region* Bus::region(std::uint32_t a, BusSpace space) {
    if (space == BusSpace::Sub) {
        for (auto index : {2u, 12u, 13u}) {
            auto& r = regions[index];
            if (a >= r.base && a - r.base < r.bytes.size()) return &r;
        }
        return nullptr; // Sub CPU ROM and execution are deliberately absent.
    }
    for (std::size_t i = 0; i < 12; ++i) {
        auto& r = regions[i];
        if (a >= r.base && a - r.base < r.bytes.size()) return &r;
    }
    return nullptr;
}
std::uint8_t Bus::read_byte(std::uint32_t a, BusSpace space) {
    a &= 0xffffff;
    if (space == BusSpace::Main) {
        // Neutral inputs and idle sound status; these are NOT device emulation.
        if (a >= 0x400000 && a <= 0x400003) return 0xff;
        if (a >= 0x820000 && a <= 0x820003) return (a & 1) ? 0 : 0xff;
        if (a >= 0xa00000 && a <= 0xa00007) {
            const std::uint16_t value = ((a & 6) == 2) ? palette[palette_address_] : 0x00ff;
            return static_cast<std::uint8_t>(value >> ((a & 1) ? 0 : 8));
        }
    }
    auto* r = region(a, space);
    return r ? r->bytes[a - r->base] : 0xff;
}
void Bus::write_byte(std::uint32_t a, std::uint8_t value, BusSpace space) {
    a &= 0xffffff;
    auto* r = region(a, space);
    if (!r || r->readonly) return;
    r->bytes[a - r->base] = value;
    if (space == BusSpace::Main && a >= 0xa00000 && a <= 0xa00003) {
        if (a < 0xa00002) {
            palette_latch_ = static_cast<std::uint16_t>((r->bytes[0] << 8) | r->bytes[1]);
            palette_address_ = palette_latch_ & 0xfff; // Chase H.Q. configures TC0110PCR shift=0.
        } else {
            auto& color = palette[palette_address_];
            color = static_cast<std::uint16_t>((a & 1) ? ((color & 0xff00) | value) : ((color & 0xff) | (value << 8)));
        }
    }
}
std::uint32_t Bus::peek(std::uint32_t a, unsigned size, BusSpace space) {
    if (size != 1 && size != 2 && size != 4) throw std::invalid_argument("Bus size must be 1, 2 or 4");
    std::uint32_t v = 0;
    for (unsigned i = 0; i < size; ++i) v = (v << 8) | read_byte(a + i, space);
    return v;
}
std::uint32_t Bus::read(std::uint32_t a, unsigned size, BusSpace space) {
    a &= 0xffffff;
    const auto value = peek(a, size, space);
    for (unsigned i = 0; i < size; ++i) {
        if (auto* r = region((a + i) & 0xffffff, space)) ++r->reads;
        else ++unmapped_reads_;
    }
    trace('R', a, value, size, space);
    return value;
}
void Bus::write(std::uint32_t a, std::uint32_t value, unsigned size, BusSpace space) {
    if (size != 1 && size != 2 && size != 4) throw std::invalid_argument("Bus size must be 1, 2 or 4");
    a &= 0xffffff;
    for (unsigned i = 0; i < size; ++i) {
        const auto address = (a + i) & 0xffffff;
        if (auto* r = region(address, space)) ++r->writes;
        else ++unmapped_writes_;
        write_byte(address, static_cast<std::uint8_t>(value >> ((size - i - 1) * 8)), space);
    }
    trace('W', a, value, size, space);
}
void Bus::open_log(const std::filesystem::path& path, std::uint64_t limit, bool all) {
    log_.open(path);
    if (!log_) throw std::runtime_error("Cannot create bus log: " + path.string());
    log_limit_ = limit; all_ = all;
    log_ << "# space PC R/W bits address value region; hex values. ROM reads omitted unless --trace-all\n";
}
void Bus::trace(char op, std::uint32_t a, std::uint32_t v, unsigned size, BusSpace space) {
    if (!log_.is_open() || (!all_ && op == 'R' && space == BusSpace::Main && a < 0x80000)) return;
    if (logged_ >= log_limit_) return;
    auto* r = region(a, space);
    log_ << (space == BusSpace::Main ? 'A' : 'B') << ' ' << std::hex << std::setfill('0')
         << std::setw(6) << pc << ' ' << op << std::dec << size * 8 << ' ' << std::hex
         << std::setw(6) << a << ' ' << std::setw(size * 2) << v << ' '
         << (r ? r->name : "UNMAPPED") << '\n';
    if (++logged_ == log_limit_) log_ << "# trace limit reached; counters continue\n";
}
bool Bus::executable(std::uint32_t pc_value) const {
    return !(pc_value & 1) && (pc_value < 0x80000 || (pc_value >= 0x100000 && pc_value < 0x110000));
}
void Bus::summary(std::ostream& out) const {
    out << std::dec << "Bus byte-access counters (ROM write counts are rejected attempts):\n";
    for (const auto& r : regions) out << "  " << r.name << " R=" << r.reads << " W=" << r.writes << '\n';
    out << "  unmapped R=" << unmapped_reads_ << " W=" << unmapped_writes_ << '\n';
}
void Bus::dump(const std::filesystem::path& directory) const {
    std::filesystem::create_directories(directory);
    for (const auto& r : regions) {
        if (r.readonly) continue;
        std::ofstream f(directory / (std::string(r.name) + ".bin"), std::ios::binary);
        f.write(reinterpret_cast<const char*>(r.bytes.data()), static_cast<std::streamsize>(r.bytes.size()));
        if (!f) throw std::runtime_error("RAM dump write failed");
    }
    std::ofstream f(directory / "palette.bin", std::ios::binary);
    for (auto v : palette) { f.put(static_cast<char>(v >> 8)); f.put(static_cast<char>(v)); }
    if (!f) throw std::runtime_error("Palette dump write failed");
}
Runtime::Runtime(Bytes rom) : bus(std::move(rom)) {
    if (active) throw std::runtime_error("Only one Musashi runtime may exist");
    active = this;
    m68k_init(); m68k_set_cpu_type(M68K_CPU_TYPE_68000);
    m68k_set_instr_hook_callback(instruction);
    m68k_set_int_ack_callback(interrupt_ack);
}
Runtime::~Runtime() { if (active == this) active = nullptr; }
void Runtime::reset() {
    const auto sp = bus.peek(0, 4), pc = bus.peek(4, 4);
    if (sp & 1 || sp < 0x100000 || sp > 0x110000 || pc < 8 || pc >= 0x80000 || pc & 1)
        throw std::runtime_error("Invalid reset vectors: expected even ROM PC and work-RAM stack");
    cycles = instructions = 0; fault.clear(); bus.pc = pc;
    m68k_pulse_reset();
    std::cout << "[RESET] SP=0x" << std::hex << sp << " PC=0x" << pc << std::dec << '\n';
}
int Runtime::run(int budget) {
    if (budget <= 0 || !fault.empty()) return 0;
    const auto used = m68k_execute(budget);
    if (used > 0) cycles += static_cast<unsigned>(used);
    return used;
}
void Runtime::irq4() { m68k_set_irq(4); }
void Runtime::summary(std::ostream& out) const {
    out << "CPU A: cycles=" << std::dec << cycles << " instructions=" << instructions
        << " PC=0x" << std::hex << m68k_get_reg(nullptr, M68K_REG_PC)
        << " SR=0x" << m68k_get_reg(nullptr, M68K_REG_SR)
        << " SP=0x" << m68k_get_reg(nullptr, M68K_REG_SP) << std::dec << '\n';
    out << "Status: " << (fault.empty() ? "budget reached / diagnostic run ended; not proof of game boot" : fault) << '\n';
    bus.summary(out);
}
Options parse_options(int argc, char** argv) {
    Options o;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        auto value = [&]() -> std::string {
            if (++i >= argc) throw std::runtime_error("Missing value after " + arg);
            return argv[i];
        };
        if (arg == "--roms") o.roms = value();
        else if (arg == "--logs") o.logs = value();
        else if (arg == "--frames") {
            const auto text = value(); std::size_t end = 0;
            const auto n = std::stoul(text, &end);
            if (end != text.size() || n < 1 || n > 36000) throw std::runtime_error("Frames must be 1..36000");
            o.frames = static_cast<unsigned>(n);
        } else if (arg == "--irq4") o.irq = true;
        else if (arg == "--trace-all") o.all = true;
        else if (arg == "--scene-only") o.scene_only = true;
        else if (arg == "--help" || arg == "-h") o.help = true;
        else if (i == 1 && !arg.starts_with("--")) o.roms = arg;
        else throw std::runtime_error("Unknown option: " + arg);
    }
    return o;
}
void print_help() {
    std::cout << "ChaseHQ-Native 0.8 CPU runtime\n"
        "[rom-directory] [--roms path] [--frames 120] [--logs logs]\n"
        "[--trace-all] [--irq4] [--scene-only (SDL frontend only)]\n"
        "200000 CPU cycles/frame; bounded at 120 frames by default.\n"
        "Main CPU only; scene remains the v0.7 synthetic diagnostic.\n";
}
}
