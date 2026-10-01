#include "runtime.h"
#include "version.h"
#include "m68k.h"
#include "m68kcpu.h"
#include <algorithm>
#include <cmath>
#include <array>
#include <cstring>
#include <cctype>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

static chq::Runtime* active = nullptr;
static chq::BusSpace active_space = chq::BusSpace::Main;

extern "C" unsigned int m68k_read_memory_8(unsigned int a) {
    return active->bus.read(a, 1, active_space);
}
extern "C" unsigned int m68k_read_memory_16(unsigned int a) {
    return active->bus.read(a, 2, active_space);
}
extern "C" unsigned int m68k_read_memory_32(unsigned int a) {
    return active->bus.read(a, 4, active_space);
}
extern "C" void m68k_write_memory_8(unsigned int a, unsigned int v) {
    active->bus.write(a, v, 1, active_space);
}
extern "C" void m68k_write_memory_16(unsigned int a, unsigned int v) {
    active->bus.write(a, v, 2, active_space);
}
extern "C" void m68k_write_memory_32(unsigned int a, unsigned int v) {
    active->bus.write(a, v, 4, active_space);
}

extern "C" unsigned int m68k_read_disassembler_16(unsigned int a) {
    return active ? active->bus.peek(a, 2, active_space) : 0xffff;
}
extern "C" unsigned int m68k_read_disassembler_32(unsigned int a) {
    return active ? active->bus.peek(a, 4, active_space) : 0xffffffffu;
}

void runtime_instruction_hook(unsigned int pc) {
    active->instruction_hook(pc);
}

int runtime_interrupt_ack(int level) {
    active->interrupt_ack(level);
    m68k_set_irq(0);
    return M68K_INT_ACK_AUTOVECTOR;
}

namespace chq {

Bus::Bus(Bytes main_rom, Bytes sub_rom) {
    if (main_rom.size() != 0x80000)
        throw std::runtime_error("Main ROM region must be 0x80000 bytes");
    if (sub_rom.size() != 0x20000)
        throw std::runtime_error("Sub ROM region must be 0x20000 bytes");

    regions = {
        {"rom_a", 0x000000, std::move(main_rom), true},       // 0
        {"work_low", 0x100000, Bytes(0x8000)},                // 1
        {"shared", 0x108000, Bytes(0x4000)},                  // 2
        {"work_high", 0x10c000, Bytes(0x4000)},               // 3
        {"tc0040ioc", 0x400000, Bytes(4, 0xff)},              // 4
        {"cpu_control", 0x800000, Bytes(2, 0xff)},            // 5
        {"sound_stub", 0x820000, Bytes(4)},                   // 6
        {"palette_registers", 0xa00000, Bytes(8)},            // 7
        {"tilemap", 0xc00000, Bytes(0x10000)},                // 8
        {"tile_control", 0xc20000, Bytes(0x10)},              // 9
        {"sprites", 0xd00000, Bytes(0x800)},                  // 10
        {"motor_stub", 0xe00000, Bytes(0x400)},               // 11
        {"rom_b", 0x000000, std::move(sub_rom), true},        // 12
        {"sub_work", 0x100000, Bytes(0x4000)},                // 13
        {"road", 0x800000, Bytes(0x2000)}                     // 14
    };
}

Region* Bus::region(std::uint32_t a, BusSpace space) {
    if (space == BusSpace::Sub) {
        for (auto index : {12u, 13u, 2u, 14u}) {
            auto& r = regions[index];
            if (a >= r.base && a - r.base < r.bytes.size()) return &r;
        }
        return nullptr;
    }

    for (std::size_t i = 0; i < 12; ++i) {
        auto& r = regions[i];
        if (a >= r.base && a - r.base < r.bytes.size()) return &r;
    }
    return nullptr;
}

const Region* Bus::region(std::uint32_t a, BusSpace space) const {
    return const_cast<Bus*>(this)->region(a, space);
}

std::uint8_t Bus::ioc_port_value(std::uint8_t port) const {
    const auto p = static_cast<std::uint8_t>(port & 0x0f);
    std::uint8_t value = 0xff;
    switch (p) {
    case 0x00: value = 0xff; break; // DSWA: factory World upright/steering-lock, game mode, demo sound on
    case 0x01: value = 0xff; break; // DSWB: factory difficulty/timer/nitro/continue settings
    case 0x02: value = 0x33; break; // IN0 observed neutral/default state
    case 0x03: value = 0x3f; break; // IN1 observed neutral/default state
    case 0x07: value = 0xff; break; // IN2 unused, active-low

    // Chase H.Q. bypasses the IOC for these selected ports.
    case 0x08:
    case 0x09:
    case 0x0a:
    case 0x0b:
        value = 0xff; break;
    case 0x0c:
        value = static_cast<std::uint8_t>(ioc_steering_ & 0xff); break;
    case 0x0d:
        value = static_cast<std::uint8_t>(ioc_steering_ >> 8); break;
    default:
        value = ioc_last_write_[p]; break;
    }
    // Debugger-only pulse injection inverts selected input bits temporarily.
    // XOR is intentional: it lets us exercise either side of a branch without
    // prematurely assuming active-high/active-low semantics for an unknown bit.
    return static_cast<std::uint8_t>(value ^ ioc_effective_xor_mask(p));
}

void Bus::open_ioc_pulse_log(const std::filesystem::path& path) {
    ioc_pulse_log_.open(path);
    if (!ioc_pulse_log_)
        throw std::runtime_error("Cannot create IOC pulse log: " + path.string());
    ioc_pulse_log_ << "# Chase H.Q. Native v0.27 IOC read-path pulse log.\n";
}

std::uint8_t Bus::ioc_portreg_r() {
    const auto port = static_cast<std::uint8_t>(ioc_port_ & 0x0f);
    ++ioc_reads_[port];
    const auto mask = ioc_effective_xor_mask(port);
    const auto value = ioc_port_value(port);
    if (mask != 0 && ioc_pulse_log_) {
        const auto base = static_cast<std::uint8_t>(value ^ mask);
        ioc_pulse_log_ << "IOC_PULSE_READ frame=" << std::dec << ioc_debug_frame_
                       << " port=" << std::hex << std::setfill('0') << std::setw(2) << static_cast<unsigned>(port)
                       << " base=" << std::setw(2) << static_cast<unsigned>(base)
                       << " mask=" << std::setw(2) << static_cast<unsigned>(mask)
                       << " result=" << std::setw(2) << static_cast<unsigned>(value)
                       << std::dec << " read_count=" << ioc_reads_[port] << '\n';
    }
    return value;
}

void Bus::ioc_portreg_w(std::uint8_t value) {
    const auto port = static_cast<std::uint8_t>(ioc_port_ & 0x0f);
    ++ioc_writes_[port];
    ioc_last_write_[port] = value;
    if (port == 0x04)
        ioc_coin_output_ = value; // coin lockout/counters; bookkeeping only for now
}

void Bus::ioc_port_w(std::uint8_t value) {
    ++ioc_selector_writes_;
    ioc_port_ = value;
}

std::uint8_t Bus::ioc_watchdog_r() {
    ++ioc_watchdog_reads_;
    // MAME maps this read to TC0040IOC::watchdog_r; the value is not game data.
    // 0xff is the normal open/high idle value and also services the watchdog.
    constexpr std::uint8_t value = 0xff;
    debug_watchdog_read(value);
    return value;
}

std::uint8_t Bus::read_byte(std::uint32_t a, BusSpace space) {
    a &= 0xffffff;

    if (space == BusSpace::Main) {
        // Chase H.Q. only connects the low byte lane:
        // 400001 = selected-port data, 400003 = watchdog read / selector write.
        if (a == 0x400001) return ioc_portreg_r();
        if (a == 0x400003) return ioc_watchdog_r();
        if (a == 0x400000 || a == 0x400002) return 0xff;

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

    if (space == BusSpace::Main) {
        if (a == 0x400001) {
            ioc_portreg_w(value);
            return;
        }
        if (a == 0x400003) {
            ioc_port_w(value);
            return;
        }
        // Even byte lane is not connected to the 8-bit IOC.
        if (a == 0x400000 || a == 0x400002)
            return;
    }

    if (space == BusSpace::Main && a >= 0x800000 && a <= 0x800001) {
        // MAME's cpua_ctrl_w accepts either byte lane and keeps the low 8 bits.
        // Bit 0 controls CPU B reset: 1 = enabled/released, 0 = held reset.
        cpu_control_ = value;
    }

    if (space == BusSpace::Main && a >= 0xa00000 && a <= 0xa00003) {
        if (a < 0xa00002) {
            palette_latch_ = static_cast<std::uint16_t>((r->bytes[0] << 8) | r->bytes[1]);
            palette_address_ = palette_latch_ & 0xfff;
        } else {
            const auto pi = static_cast<std::size_t>(palette_address_ & 0x0fff);
            auto& color = palette[palette_address_];
            const auto byte_old = color;
            if (!(a & 1)) {
                palette_trace_pending_old_[pi] = color;
                palette_trace_pending_valid_[pi] = true;
            }
            color = static_cast<std::uint16_t>((a & 1)
                ? ((color & 0xff00) | value)
                : ((color & 0x00ff) | (value << 8)));
            // A normal 16-bit write reaches this handler twice. Count the low-byte
            // commit as one complete palette transaction; still record high-byte-only
            // traffic if it is all a title happens to perform.
            if (!quiet_fast_forward_ && ((a & 1) || palette_write_count[pi] == 0)) {
                const auto transaction_old = ((a & 1) && palette_trace_pending_valid_[pi])
                    ? palette_trace_pending_old_[pi] : byte_old;
                if (palette_write_count[pi] == 0) {
                    palette_first_write_frame[pi] = ioc_debug_frame_;
                    palette_first_written_value[pi] = color;
                }
                ++palette_write_count[pi];
                palette_last_write_frame[pi] = ioc_debug_frame_;
                palette_last_write_pc[pi] = trace_pc(space);
                palette_last_written_value[pi] = color;
                if ((a & 1) && palette_trace_enabled_ && pi < palette_trace_filter_.size() && palette_trace_filter_[pi]) {
                    PaletteWriteEvent e{};
                    e.frame = ioc_debug_frame_; e.space = space; e.pc = trace_pc(space);
                    e.index = static_cast<std::uint16_t>(pi); e.old_value = transaction_old; e.new_value = color;
                    e.write_count = palette_write_count[pi]; e.changed = e.old_value != e.new_value;
                    palette_trace_events_.push_back(e);
                    while (palette_trace_events_.size() > palette_trace_limit_) palette_trace_events_.pop_front();
                }
            }
            if (a & 1) palette_trace_pending_valid_[pi] = false;
        }
    }
}

std::uint32_t Bus::peek(std::uint32_t a, unsigned size, BusSpace space) {
    if (size != 1 && size != 2 && size != 4)
        throw std::invalid_argument("Bus size must be 1, 2 or 4");
    std::uint32_t v = 0;
    for (unsigned i = 0; i < size; ++i)
        v = (v << 8) | read_byte(a + i, space);
    return v;
}

unsigned Bus::research_add_watch(const ResearchWatch& watch) {
    ResearchWatch w = watch;
    w.id = next_research_watch_id_++;
    w.start &= 0xffffffu; w.end &= 0xffffffu;
    if (w.end < w.start) std::swap(w.start, w.end);
    research_watches_.push_back(w);
    return w.id;
}

bool Bus::research_remove_watch(unsigned id) {
    const auto old = research_watches_.size();
    research_watches_.erase(std::remove_if(research_watches_.begin(), research_watches_.end(), [=](const ResearchWatch& w){ return w.id==id; }), research_watches_.end());
    return research_watches_.size()!=old;
}

void Bus::palette_trace_start(const std::vector<std::uint16_t>& indices, std::size_t limit) {
    palette_trace_filter_.fill(false);
    for (const auto index : indices)
        if (index < palette_trace_filter_.size()) palette_trace_filter_[index] = true;
    palette_trace_limit_ = std::max<std::size_t>(64, std::min<std::size_t>(limit, 1000000));
    while (palette_trace_events_.size() > palette_trace_limit_) palette_trace_events_.pop_front();
    palette_trace_pending_valid_.fill(false);
    palette_trace_enabled_ = true;
}

void Bus::memory_trace_start(BusSpace space, std::uint32_t address, std::uint32_t length, unsigned width, std::size_t limit) {
    memory_trace_space_ = space;
    memory_trace_address_ = address & 0xffffffu;
    const auto remaining = 0x1000000u - memory_trace_address_;
    memory_trace_length_ = std::max<std::uint32_t>(1, std::min<std::uint32_t>(length, remaining));
    memory_trace_width_ = (width == 1 || width == 2 || width == 4) ? width : 0;
    memory_trace_limit_ = std::max<std::size_t>(64, std::min<std::size_t>(limit, 1000000));
    while (memory_trace_events_.size() > memory_trace_limit_) memory_trace_events_.pop_front();
    memory_trace_write_count_ = 0;
    memory_trace_enabled_ = true;
}

std::string Bus::research_take_break_reason() {
    research_break_pending_ = false;
    auto reason = research_break_reason_;
    research_break_reason_.clear();
    return reason;
}

void Bus::debug_write(std::uint32_t address, std::uint32_t value, unsigned size, BusSpace space) {
    const auto oldv = peek(address, size, space);
    const bool old_bypass = research_internal_write_;
    research_internal_write_ = true;
    write(address, value, size, space);
    research_internal_write_ = old_bypass;
    if (research_enabled_) {
        ResearchEvent e{}; e.frame=ioc_debug_frame_; e.kind=ResearchEventKind::Intervention; e.space=space;
        e.pc=trace_pc(space); e.address=address&0xffffffu; e.size=size; e.old_value=oldv; e.new_value=peek(address,size,space); e.changed=e.old_value!=e.new_value;
        research_events_.push_back(e); while(research_events_.size()>research_event_limit_) research_events_.pop_front();
    }
}

bool Bus::research_remove_patch(unsigned id) {
    const auto old=live_patches_.size();
    live_patches_.erase(std::remove_if(live_patches_.begin(),live_patches_.end(),[=](const LivePatch& p){return p.id==id;}),live_patches_.end());
    return live_patches_.size()!=old;
}

unsigned Bus::research_add_patch(BusSpace space, std::uint32_t address, unsigned size, std::uint32_t value, LivePatchMode mode) {
    if (size!=1 && size!=2 && size!=4) throw std::invalid_argument("Live patch size must be 1, 2 or 4");
    LivePatch p{}; p.id=next_live_patch_id_++; p.space=space; p.address=address&0xffffffu; p.size=size; p.value=value; p.mode=mode;
    live_patches_.push_back(p); return p.id;
}

std::uint32_t Bus::read(std::uint32_t a, unsigned size, BusSpace space) {
    a &= 0xffffff;
    const auto value = peek(a, size, space);
    if (research_enabled_ && !research_internal_write_) {
        const auto end = (a + size - 1) & 0xffffffu;
        for (const auto& w : research_watches_) {
            if (!w.reads || w.space != space || a > w.end || end < w.start) continue;
            ResearchEvent e{}; e.frame=ioc_debug_frame_; e.kind=ResearchEventKind::Read; e.space=space; e.pc=trace_pc(space); e.address=a; e.size=size; e.old_value=value; e.new_value=value; e.changed=false;
            research_events_.push_back(e); while(research_events_.size()>research_event_limit_) research_events_.pop_front();
            if (w.break_on_match) { research_break_pending_=true; std::ostringstream q; q<<"READ "<<(space==BusSpace::Main?'A':'B')<<":"<<std::hex<<a<<" PC="<<trace_pc(space); research_break_reason_=q.str(); }
            break;
        }
    }
    if (quiet_fast_forward_) return value;
    if (space == BusSpace::Sub && a >= 0x108000 && a <= 0x10bfff)
        debug_sub_shared_read(a, value, size);
    if (a >= 0x108000 && a <= 0x1080ff)
        debug_handshake_access('R', space, a, value, size);
    if (space == BusSpace::Sub && pc_b_ >= 0x980 && pc_b_ <= 0xa10)
        debug_road_focus_access('R', space, a, value, size);
    debug_road_data_flow_access('R', space, a, value, size);
    debug_road_state_access('R', space, a, value, size);
    debug_road_pending_access('R', space, a, value, size);
    if (generic_trace_cb_ && (!generic_trace_interest_cb_ || generic_trace_interest_cb_('R', space, a, size))) {
        const auto* gr = region(a, space);
        generic_trace_cb_('R', space, a, value, size, 0, false, gr ? gr->name : "UNMAPPED");
    }
    for (unsigned i = 0; i < size; ++i) {
        if (auto* r = region((a + i) & 0xffffff, space)) ++r->reads;
        else ++unmapped_reads_;
    }
    trace('R', a, value, size, space);
    return value;
}

void Bus::write(std::uint32_t a, std::uint32_t value, unsigned size, BusSpace space) {
    if (size != 1 && size != 2 && size != 4)
        throw std::invalid_argument("Bus size must be 1, 2 or 4");
    a &= 0xffffff;
    // Live intervention patches are enforcement controls, not merely trace features.
    // They must remain active even when research event recording itself is disabled.
    LivePatch* live_patch = nullptr;
    if (!research_internal_write_) {
        for (auto& p : live_patches_) if (p.space==space && p.address==a && p.size==size) { live_patch=&p; break; }
    }
    const bool timeline_write_interested = static_cast<bool>(timeline_write_sink_) && !quiet_fast_forward_ && !research_internal_write_;
    const auto research_old_value = ((research_enabled_ || live_patch || timeline_write_interested) && !research_internal_write_) ? peek(a,size,space) : 0u;
    const auto memory_trace_end = (memory_trace_address_ + memory_trace_length_ - 1u) & 0xffffffu;
    const auto write_end = (a + size - 1u) & 0xffffffu;
    const bool memory_trace_interested = memory_trace_enabled_ && !quiet_fast_forward_ && !research_internal_write_ &&
        space == memory_trace_space_ && (memory_trace_width_ == 0 || memory_trace_width_ == size) &&
        memory_trace_end >= memory_trace_address_ && a <= memory_trace_end && write_end >= memory_trace_address_;
    const std::uint32_t memory_trace_old_value = memory_trace_interested ? peek(a,size,space) : 0u;
    if (live_patch && !research_internal_write_) {
            live_patch->interceptions++; live_patch->last_frame=ioc_debug_frame_; live_patch->last_pc=trace_pc(space);
            if (live_patch->mode==LivePatchMode::Suppress) {
                ResearchEvent e{}; e.frame=ioc_debug_frame_; e.kind=ResearchEventKind::SuppressedWrite; e.space=space; e.pc=trace_pc(space); e.address=a; e.size=size; e.old_value=research_old_value; e.new_value=research_old_value; e.changed=false;
                research_events_.push_back(e); while(research_events_.size()>research_event_limit_) research_events_.pop_front();
                return;
            }
            value = live_patch->value;
        }
    // v0.59.1: collision-free mapping suppresses only response writes whose
    // semantics have been proven from execution traces. Detection, object state,
    // event/scoring logic and timers remain live.
    if (suppress_collision_shove_ && space == BusSpace::Main) {
        const bool lateral_response = a == 0x10a044 && size == 2 &&
            (pc_a_ == 0x00a142 || pc_a_ == 0x00a156 || pc_a_ == 0x00a1be || pc_a_ == 0x00a1c4);
        const bool speed_response = a == 0x10041c && size == 4 && pc_a_ == 0x00a200;
        if (lateral_response || speed_response) {
            ++suppressed_collision_shoves_;
            if (lateral_response) ++suppressed_collision_lateral_;
            if (speed_response) ++suppressed_collision_speed_;
            return;
        }
    }
    const bool generic_trace_interested = !quiet_fast_forward_ && generic_trace_cb_ &&
        (!generic_trace_interest_cb_ || generic_trace_interest_cb_('W', space, a, size));
    const bool generic_need_old = generic_trace_interested && generic_trace_old_value_cb_ &&
        generic_trace_old_value_cb_(space, a, size);
    const std::uint32_t generic_old_value = generic_need_old ? peek(a, size, space) : 0;

    std::array<std::size_t, 2> touched_road_words{road_word_writes_.size(), road_word_writes_.size()};
    unsigned touched_road_count = 0;
    if (!quiet_fast_forward_ && space == BusSpace::Sub) {
        for (unsigned i = 0; i < size; ++i) {
            const auto address = (a + i) & 0xffffff;
            if (address < 0x800000 || address > 0x801fff) continue;
            const std::size_t word = static_cast<std::size_t>((address - 0x800000) >> 1);
            bool already = false;
            for (unsigned j = 0; j < touched_road_count; ++j)
                already |= touched_road_words[j] == word;
            if (!already && touched_road_count < touched_road_words.size())
                touched_road_words[touched_road_count++] = word;
        }
    }

    for (unsigned i = 0; i < size; ++i) {
        const auto address = (a + i) & 0xffffff;
        if (!quiet_fast_forward_) { if (auto* r = region(address, space)) ++r->writes; else ++unmapped_writes_; }
        write_byte(address, static_cast<std::uint8_t>(value >> ((size - i - 1) * 8)), space);
    }

    if (memory_trace_interested) {
        const auto now = peek(a,size,space);
        MemoryWriteTraceEvent e{};
        e.frame = ioc_debug_frame_; e.space = space; e.pc = trace_pc(space); e.address = a; e.size = size;
        e.old_value = memory_trace_old_value; e.new_value = now; e.changed = e.old_value != e.new_value;
        e.write_count = ++memory_trace_write_count_;
        memory_trace_events_.push_back(e);
        while (memory_trace_events_.size() > memory_trace_limit_) memory_trace_events_.pop_front();
    }

    if (timeline_write_interested) {
        const auto now = peek(a,size,space);
        MemoryWriteTraceEvent e{};
        e.frame = ioc_debug_frame_; e.space = space; e.pc = trace_pc(space); e.address = a; e.size = size;
        e.old_value = research_old_value; e.new_value = now; e.changed = e.old_value != e.new_value;
        e.write_count = ++timeline_write_count_;
        timeline_write_sink_(e);
    }

    if (!quiet_fast_forward_ && space == BusSpace::Sub) {
        for (unsigned j = 0; j < touched_road_count; ++j) {
            const std::size_t word = touched_road_words[j];
            if (word >= road_word_writes_.size()) continue;
            ++road_word_writes_[word];
            const std::uint32_t base = 0x800000u + static_cast<std::uint32_t>(word * 2);
            const auto* r = region(base, BusSpace::Sub);
            if (!r) continue;
            const std::size_t off = static_cast<std::size_t>(base - r->base);
            const std::uint16_t now = static_cast<std::uint16_t>((static_cast<std::uint16_t>(r->bytes[off]) << 8) | r->bytes[off + 1]);
            if (!road_word_seen_[word]) {
                road_word_seen_[word] = true;
                road_word_min_values_[word] = now;
                road_word_max_values_[word] = now;
            } else {
                if (road_word_last_values_[word] != now) ++road_word_value_changes_[word];
                road_word_min_values_[word] = std::min(road_word_min_values_[word], now);
                road_word_max_values_[word] = std::max(road_word_max_values_[word], now);
            }
            road_word_last_values_[word] = now;
            if (now != 0) ++road_word_nonzero_writes_[word];
        }
    }
    if (research_enabled_ && !research_internal_write_) {
        const auto now = peek(a,size,space);
        const auto end = (a + size - 1) & 0xffffffu;
        bool watched = live_patch != nullptr;
        bool should_break = false;
        for (const auto& w : research_watches_) {
            if (!w.writes || w.space != space || a > w.end || end < w.start) continue;
            if (!w.change_only || research_old_value != now) { watched = true; should_break |= w.break_on_match; }
        }
        if (watched) {
            ResearchEvent e{}; e.frame=ioc_debug_frame_; e.kind=ResearchEventKind::Write; e.space=space; e.pc=trace_pc(space); e.address=a; e.size=size; e.old_value=research_old_value; e.new_value=now; e.changed=e.old_value!=e.new_value;
            research_events_.push_back(e); while(research_events_.size()>research_event_limit_) research_events_.pop_front();
        }
        if (should_break) { research_break_pending_=true; std::ostringstream q; q<<"WRITE "<<(space==BusSpace::Main?'A':'B')<<":"<<std::hex<<a<<" "<<research_old_value<<"->"<<now<<" PC="<<trace_pc(space); research_break_reason_=q.str(); }
    }
    if (quiet_fast_forward_) return;
    if (a >= 0x108000 && a <= 0x1080ff)
        debug_handshake_access('W', space, a, value, size);
    if (space == BusSpace::Sub && a >= 0x800000 && a <= 0x801fff) {
        debug_sub_road_write(a, value, size);
        ++road_write_pc_counts_[pc_b_];
        if (value != 0) ++road_nonzero_pc_counts_[pc_b_];
    }
    if (space == BusSpace::Sub && pc_b_ >= 0x980 && pc_b_ <= 0xa10)
        debug_road_focus_access('W', space, a, value, size);
    debug_road_data_flow_access('W', space, a, value, size);
    debug_road_state_access('W', space, a, value, size);
    debug_road_pending_access('W', space, a, value, size);
    if (generic_trace_interested) {
        const auto* gr = region(a, space);
        generic_trace_cb_('W', space, a, value, size, generic_old_value, generic_need_old, gr ? gr->name : "UNMAPPED");
    }
    trace('W', a, value, size, space);
}

void Bus::open_log(const std::filesystem::path& path, std::uint64_t limit, bool all) {
    log_.open(path);
    if (!log_) throw std::runtime_error("Cannot create bus log: " + path.string());
    log_limit_ = limit;
    all_ = all;
    log_ << "# cpu PC R/W bits address value region; hex values. ROM reads omitted unless --trace-all\n";
}

void Bus::open_boot_debug_log(const std::filesystem::path& path, std::uint64_t limit) {
    boot_log_.open(path);
    if (!boot_log_) throw std::runtime_error("Cannot create boot debug log: " + path.string());
    boot_log_limit_ = limit;
    boot_log_ << "# Chase H.Q. boot-loop debugger. Hex unless noted.\n";
}

void Bus::open_sub_debug_log(const std::filesystem::path& path, std::uint64_t limit) {
    sub_log_.open(path);
    if (!sub_log_) throw std::runtime_error("Cannot create CPU B debug log: " + path.string());
    sub_log_limit_ = limit;
    sub_log_ << "# Chase H.Q. Native v0.26 CPU B debugger. Hex unless noted.\n";
    sub_log_ << "# Focus range 000440..000520 plus handler sampling; shared reads and road writes include originating PC.\n";
}

void Bus::open_handshake_log(const std::filesystem::path& path, std::uint64_t limit) {
    handshake_log_.open(path);
    if (!handshake_log_) throw std::runtime_error("Cannot create handshake log: " + path.string());
    handshake_log_limit_ = limit;
    handshake_log_ << "# Chase H.Q. Native v0.26 CPU A <-> CPU B handshake debugger. Hex unless noted.\n";
    handshake_log_ << "# Tracks shared 108000..1080ff, command dispatches, jump table and road-write origins.\n";
}

void Bus::open_road_focus_log(const std::filesystem::path& path, std::uint64_t limit) {
    road_focus_log_.open(path);
    if (!road_focus_log_) throw std::runtime_error("Cannot create road focus log: " + path.string());
    road_focus_log_limit_ = limit;
    road_focus_log_ << "# Chase H.Q. Native v0.26 CPU B road-write focus debugger. Hex unless noted.\n";
    road_focus_log_ << "# Focus PC range 000980..000a10. DATA lines are bus reads/writes while executing that range.\n";
    road_focus_log_ << "# ROAD writes include exact origin PC, width, address and value.\n";
}

void Bus::open_road_data_flow_log(const std::filesystem::path& path, std::uint64_t limit) {
    road_data_flow_log_.open(path);
    if (!road_data_flow_log_) throw std::runtime_error("Cannot create road data-flow log: " + path.string());
    road_data_flow_log_limit_ = limit;
    road_data_flow_log_ << "# Chase H.Q. Native v0.26 road-data flow tracer. Hex unless noted.\n";
    road_data_flow_log_ << "# Tracks CPU-B accesses to prepared work tables 101600..102200 and all non-zero road writes.\n";
    road_data_flow_log_ << "# Goal: identify the instruction path that transfers prepared table data into TC0150ROD RAM.\n";
}

void Bus::debug_note(const std::string& text) {
    if (!boot_log_.is_open() || boot_logged_ >= boot_log_limit_) return;
    boot_log_ << text << '\n';
    if (++boot_logged_ == boot_log_limit_)
        boot_log_ << "# boot debug log limit reached; counters continue\n";
}

void Bus::debug_watchdog_read(std::uint8_t value) {
    if (!boot_log_.is_open() || boot_logged_ >= boot_log_limit_) return;
    if (pc_a_ < 0x580 || pc_a_ > 0x5c0) return;
    std::ostringstream ss;
    ss << "WATCHDOG pc=" << std::hex << std::setfill('0') << std::setw(6) << pc_a_
       << " value=" << std::setw(2) << static_cast<unsigned>(value)
       << std::dec << " count=" << ioc_watchdog_reads_;
    debug_note(ss.str());
}

void Bus::sub_debug_note(const std::string& text) {
    if (!sub_log_.is_open() || sub_logged_ >= sub_log_limit_) return;
    sub_log_ << text << '\n';
    if (++sub_logged_ == sub_log_limit_)
        sub_log_ << "# CPU B debug log limit reached; counters continue\n";
}

void Bus::debug_sub_shared_read(std::uint32_t address, std::uint32_t value, unsigned size) {
    if (pc_b_ < 0x440 || pc_b_ > 0x520) return;
    ++sub_shared_reads_;
    if (!sub_log_.is_open() || sub_logged_ >= sub_log_limit_) return;
    std::ostringstream ss;
    ss << "SHARED_R pc=" << std::hex << std::setfill('0') << std::setw(6) << pc_b_
       << " bits=" << std::dec << size * 8
       << " addr=" << std::hex << std::setw(6) << address
       << " value=" << std::setw(size * 2) << value
       << std::dec << " count=" << sub_shared_reads_;
    sub_debug_note(ss.str());
}

void Bus::debug_sub_road_write(std::uint32_t address, std::uint32_t value, unsigned size) {
    if (pc_b_ < 0x440 || pc_b_ > 0x520) return;
    ++sub_road_writes_debug_;
    if (!sub_log_.is_open() || sub_logged_ >= sub_log_limit_) return;
    std::ostringstream ss;
    ss << "ROAD_W pc=" << std::hex << std::setfill('0') << std::setw(6) << pc_b_
       << " bits=" << std::dec << size * 8
       << " addr=" << std::hex << std::setw(6) << address
       << " value=" << std::setw(size * 2) << value
       << std::dec << " count=" << sub_road_writes_debug_;
    sub_debug_note(ss.str());
}

void Bus::debug_handshake_access(char op, BusSpace space, std::uint32_t address, std::uint32_t value, unsigned size) {
    if (address < 0x108000 || address > 0x1080ff) return;
    const bool main = space == BusSpace::Main;
    if (main) {
        if (op == 'R') ++main_cmd_reads_; else ++main_cmd_writes_;
    } else {
        if (op == 'R') ++sub_cmd_reads_; else ++sub_cmd_writes_;
    }
    if (address <= 0x10801b && address + size > 0x10801a) {
        if (main) { if (op == 'R') ++main_10801a_reads_; else ++main_10801a_writes_; }
        else { if (op == 'R') ++sub_10801a_reads_; else ++sub_10801a_writes_; }
    }
    if (!handshake_log_.is_open() || handshake_logged_ >= handshake_log_limit_) return;
    std::ostringstream ss;
    ss << "SHARED_" << op << " cpu=" << (main ? 'A' : 'B')
       << " pc=" << std::hex << std::setfill('0') << std::setw(6) << (main ? pc_a_ : pc_b_)
       << " bits=" << std::dec << size * 8
       << " addr=" << std::hex << std::setw(6) << address
       << " value=" << std::setw(size * 2) << value;
    handshake_log_ << ss.str() << '\n';
    if (++handshake_logged_ == handshake_log_limit_)
        handshake_log_ << "# handshake log limit reached; counters continue\n";
}

void Bus::note_dispatch(std::uint8_t command, std::uint32_t target, std::uint32_t pc) {
    const auto slot = static_cast<std::size_t>(command & 0x1f);
    ++dispatch_counts_[slot];
    dispatch_targets_[slot] = target;
    if (!handshake_log_.is_open() || handshake_logged_ >= handshake_log_limit_) return;
    handshake_log_ << "DISPATCH pc=" << std::hex << std::setfill('0') << std::setw(6) << pc
                   << " cmd=" << std::setw(2) << static_cast<unsigned>(command & 0x1f)
                   << " target=" << std::setw(6) << target
                   << std::dec << " count=" << dispatch_counts_[slot] << '\n';
    if (++handshake_logged_ == handshake_log_limit_)
        handshake_log_ << "# handshake log limit reached; counters continue\n";
}

void Bus::open_road_state_log(const std::filesystem::path& path, std::uint64_t limit) {
    road_state_log_.open(path);
    if (!road_state_log_) throw std::runtime_error("Cannot create road state log: " + path.string());
    road_state_log_limit_ = limit;
    road_state_log_ << "# Chase H.Q. Native v0.26 road-state / command-2 debugger. Hex unless noted.\n";
    road_state_log_ << "# Tracks CPU-B accesses overlapping 101a5a, control flow 0004b0..0004ca, and command-2 handler 0006c2..0007ff.\n";
    road_state_log_ << "# --force-road-flag is diagnostic-only and writes 0001 to 101a5a at the first CPU-B visit to 0004b0.\n";
}


void Bus::open_road_pending_log(const std::filesystem::path& path, std::uint64_t limit) {
    road_pending_log_.open(path);
    if (!road_pending_log_) throw std::runtime_error("Cannot create road pending-pair log: " + path.string());
    road_pending_log_limit_ = limit;
    road_pending_log_ << "# Chase H.Q. Native v0.26 pending road-control pair tracer. Hex unless noted.\n";
    road_pending_log_ << "# Tracks CPU-B accesses to 101a58..101a5b and the special TC0150ROD word 801ffe..801fff.\n";
    road_pending_log_ << "# 101a58 = pending value candidate; 101a5a = one-shot strobe/valid flag observed in v0.24.\n";
}

void Bus::road_pending_note(const std::string& text) {
    if (!road_pending_log_.is_open() || road_pending_logged_ >= road_pending_log_limit_) return;
    road_pending_log_ << text << '\n';
    if (++road_pending_logged_ == road_pending_log_limit_)
        road_pending_log_ << "# road-pending log limit reached; counters continue\n";
}

void Bus::debug_road_pending_access(char op, BusSpace space, std::uint32_t address, std::uint32_t value, unsigned size) {
    if (space != BusSpace::Sub) return;
    const auto end = address + size - 1;
    const bool value_hit = address <= 0x101a59 && end >= 0x101a58;
    const bool flag_hit  = address <= 0x101a5b && end >= 0x101a5a;
    const bool ctrl_hit  = address <= 0x801fff && end >= 0x801ffe;
    if (!value_hit && !flag_hit && !ctrl_hit) return;
    const auto pc = pc_b_;
    auto bump = [&](bool hit, std::uint64_t& reads, std::uint64_t& writes, std::uint64_t& nonzero,
                    auto& read_map, auto& write_map) {
        if (!hit) return;
        if (op == 'R') { ++reads; ++read_map[pc]; }
        else { ++writes; ++write_map[pc]; if (value != 0) ++nonzero; }
    };
    bump(value_hit, pending_value_reads_, pending_value_writes_, pending_value_nonzero_writes_, pending_value_read_pc_counts_, pending_value_write_pc_counts_);
    bump(flag_hit, pending_flag_reads_, pending_flag_writes_, pending_flag_nonzero_writes_, pending_flag_read_pc_counts_, pending_flag_write_pc_counts_);
    bump(ctrl_hit, road_ctrl_reads_, road_ctrl_writes_, road_ctrl_nonzero_writes_, road_ctrl_read_pc_counts_, road_ctrl_write_pc_counts_);

    std::ostringstream ss;
    ss << (ctrl_hit ? "ROAD_CTRL_" : "PENDING_") << op
       << " pc=" << std::hex << std::setfill('0') << std::setw(6) << pc
       << " bits=" << std::dec << size * 8
       << " addr=" << std::hex << std::setw(6) << address
       << " value=" << std::setw(size * 2) << value
       << " pending=" << std::setw(4) << peek(0x101a58, 2, BusSpace::Sub)
       << " flag=" << std::setw(4) << peek(0x101a5a, 2, BusSpace::Sub)
       << " ctrl=" << std::setw(4) << peek(0x801ffe, 2, BusSpace::Sub);
    road_pending_note(ss.str());
}

void Bus::road_focus_note(const std::string& text) {
    if (!road_focus_log_.is_open() || road_focus_logged_ >= road_focus_log_limit_) return;
    road_focus_log_ << text << '\n';
    if (++road_focus_logged_ == road_focus_log_limit_)
        road_focus_log_ << "# road-focus log limit reached; counters continue\n";
}

void Bus::debug_road_focus_access(char op, BusSpace space, std::uint32_t address, std::uint32_t value, unsigned size) {
    if (space != BusSpace::Sub || pc_b_ < 0x980 || pc_b_ > 0xa10) return;
    if (op == 'R') { ++road_focus_reads_; ++road_focus_read_pc_counts_[pc_b_]; }
    else ++road_focus_writes_;
    const bool road = address >= 0x800000 && address <= 0x801fff;
    if (op == 'W' && road) ++road_focus_road_writes_;
    if (!road_focus_log_.is_open() || road_focus_logged_ >= road_focus_log_limit_) return;

    const auto* r = region(address & 0xffffff, BusSpace::Sub);
    road_focus_log_ << (road && op == 'W' ? "ROAD" : "DATA")
                    << '_' << op
                    << " pc=" << std::hex << std::setfill('0') << std::setw(6) << pc_b_
                    << " bits=" << std::dec << size * 8
                    << " addr=" << std::hex << std::setw(6) << (address & 0xffffff)
                    << " value=" << std::setw(size * 2) << value
                    << " region=" << (r ? r->name : "UNMAPPED") << '\n';
    if (++road_focus_logged_ == road_focus_log_limit_)
        road_focus_log_ << "# road-focus log limit reached; counters continue\n";
}

void Bus::road_data_flow_note(const std::string& text) {
    if (!road_data_flow_log_.is_open() || road_data_flow_logged_ >= road_data_flow_log_limit_) return;
    road_data_flow_log_ << text << '\n';
    if (++road_data_flow_logged_ == road_data_flow_log_limit_)
        road_data_flow_log_ << "# road-data-flow log limit reached; counters continue\n";
}

void Bus::road_state_note(const std::string& text) {
    if (!road_state_log_.is_open() || road_state_logged_ >= road_state_log_limit_) return;
    road_state_log_ << text << '\n';
    if (++road_state_logged_ == road_state_log_limit_) road_state_log_ << "# road-state log limit reached; counters continue\n";
}

void Bus::debug_road_state_access(char op, BusSpace space, std::uint32_t address, std::uint32_t value, unsigned size) {
    if (space != BusSpace::Sub) return;
    address &= 0xffffff;
    const auto end = address + size - 1;
    if (end < 0x101a5a || address > 0x101a5b) return;
    if (op == 'R') { ++road_flag_reads_; ++road_flag_read_pc_counts_[pc_b_]; }
    else { ++road_flag_writes_; if (value != 0) ++road_flag_nonzero_writes_; ++road_flag_write_pc_counts_[pc_b_]; }
    std::ostringstream ss;
    ss << "FLAG_" << op << " pc=" << std::hex << std::setfill('0') << std::setw(6) << pc_b_
       << " bits=" << std::dec << size * 8 << " addr=" << std::hex << std::setw(6) << address
       << " value=" << std::setw(size * 2) << value;
    road_state_note(ss.str());
}

void Bus::debug_road_data_flow_access(char op, BusSpace space, std::uint32_t address, std::uint32_t value, unsigned size) {
    if (space != BusSpace::Sub) return;
    address &= 0xffffff;
    const bool source = address >= 0x101600 && address <= 0x102200;
    const bool road = address >= 0x800000 && address <= 0x801fff;
    if (!source && !(road && op == 'W')) return;

    if (source) {
        if (op == 'R') { ++flow_source_reads_; ++flow_source_read_pc_counts_[pc_b_]; }
        else { ++flow_source_writes_; ++flow_source_write_pc_counts_[pc_b_]; }
        if (road_data_flow_log_.is_open() && road_data_flow_logged_ < road_data_flow_log_limit_) {
            const auto* r = region(address, BusSpace::Sub);
            std::ostringstream ss;
            ss << "SOURCE_" << op
               << " pc=" << std::hex << std::setfill('0') << std::setw(6) << pc_b_
               << " bits=" << std::dec << size * 8
               << " addr=" << std::hex << std::setw(6) << address
               << " value=" << std::setw(size * 2) << value
               << " region=" << (r ? r->name : "UNMAPPED");
            road_data_flow_note(ss.str());
        }
    }

    if (road && op == 'W') {
        ++flow_road_writes_;
        if (value != 0) {
            ++flow_nonzero_road_writes_;
            ++flow_nonzero_road_pc_counts_[pc_b_];
            std::ostringstream ss;
            ss << (flow_first_nonzero_seen_ ? "ROAD_NONZERO" : "FIRST_NONZERO_ROAD")
               << " pc=" << std::hex << std::setfill('0') << std::setw(6) << pc_b_
               << " bits=" << std::dec << size * 8
               << " addr=" << std::hex << std::setw(6) << address
               << " value=" << std::setw(size * 2) << value
               << " count=" << std::dec << flow_nonzero_road_writes_;
            road_data_flow_note(ss.str());
            flow_first_nonzero_seen_ = true;
        }
    }
}

void Bus::set_trace_pc(BusSpace space, std::uint32_t pc) {
    if (space == BusSpace::Main) pc_a_ = pc;
    else pc_b_ = pc;
}

void Bus::trace(char op, std::uint32_t a, std::uint32_t v, unsigned size, BusSpace space) {
    if (!log_.is_open()) return;
    if (!all_ && op == 'R' && a < (space == BusSpace::Main ? 0x80000u : 0x20000u)) return;
    if (logged_ >= log_limit_) return;

    auto* r = region(a, space);
    const auto pc = space == BusSpace::Main ? pc_a_ : pc_b_;
    log_ << (space == BusSpace::Main ? 'A' : 'B') << ' '
         << std::hex << std::setfill('0') << std::setw(6) << pc << ' '
         << op << std::dec << size * 8 << ' '
         << std::hex << std::setw(6) << a << ' '
         << std::setw(size * 2) << v << ' '
         << (r ? r->name : "UNMAPPED") << '\n';
    if (++logged_ == log_limit_)
        log_ << "# trace limit reached; counters continue\n";
}

bool Bus::executable(std::uint32_t pc, BusSpace space) const {
    if (pc & 1) return false;
    if (space == BusSpace::Main)
        return pc < 0x80000 || (pc >= 0x100000 && pc < 0x110000);
    return pc < 0x20000 || (pc >= 0x100000 && pc < 0x104000) ||
           (pc >= 0x108000 && pc < 0x10c000);
}

void Bus::summary(std::ostream& out) const {
    out << std::dec << "Bus byte-access counters (ROM write counts are rejected attempts):\n";
    for (const auto& r : regions)
        out << "  " << r.name << " R=" << r.reads << " W=" << r.writes << '\n';
    out << "  cpu_control=0x" << std::hex << static_cast<unsigned>(cpu_control_)
        << " (CPU B " << (sub_enabled() ? "enabled" : "reset") << ")\n" << std::dec;
    out << "TC0040IOC: selected=0x" << std::hex << static_cast<unsigned>(ioc_port_)
        << " coin_out=0x" << static_cast<unsigned>(ioc_coin_output_)
        << " steer=0x" << ioc_steering_ << std::dec
        << " selector_writes=" << ioc_selector_writes_
        << " watchdog_reads=" << ioc_watchdog_reads_ << '\n';
    out << "  port access R/W:";
    for (unsigned i = 0; i < 16; ++i) {
        if (ioc_reads_[i] || ioc_writes_[i])
            out << " [" << std::hex << i << std::dec << ':' << ioc_reads_[i] << '/' << ioc_writes_[i] << ']';
    }
    out << '\n';
    out << "CPU B focused I/O: shared_reads_0x440_0x520=" << sub_shared_reads_
        << " road_writes_0x440_0x520=" << sub_road_writes_debug_ << '\n';
    out << "Handshake shared[0x108000..0x1080ff]: A R/W=" << main_cmd_reads_ << '/' << main_cmd_writes_
        << " B R/W=" << sub_cmd_reads_ << '/' << sub_cmd_writes_ << '\n';
    out << "  shared 0x10801a overlap: A R/W=" << main_10801a_reads_ << '/' << main_10801a_writes_
        << " B R/W=" << sub_10801a_reads_ << '/' << sub_10801a_writes_ << '\n';
    out << "CPU B command dispatches:";
    bool any_dispatch = false;
    for (std::size_t i = 0; i < dispatch_counts_.size(); ++i) {
        if (!dispatch_counts_[i]) continue;
        any_dispatch = true;
        out << " [" << i << "->0x" << std::hex << dispatch_targets_[i] << std::dec << ':' << dispatch_counts_[i] << ']';
    }
    if (!any_dispatch) out << " none";
    out << '\n';
    out << "Road write origin PCs:";
    std::vector<std::pair<std::uint32_t, std::uint64_t>> origins(road_write_pc_counts_.begin(), road_write_pc_counts_.end());
    std::sort(origins.begin(), origins.end(), [](const auto& a, const auto& b){ return a.second > b.second; });
    for (std::size_t i = 0; i < std::min<std::size_t>(12, origins.size()); ++i)
        out << " [0x" << std::hex << origins[i].first << std::dec << ':' << origins[i].second << ']';
    if (origins.empty()) out << " none";
    out << '\n';
    if (!road_nonzero_pc_counts_.empty()) {
        out << "Road NONZERO write origin PCs:";
        std::vector<std::pair<std::uint32_t, std::uint64_t>> nz(road_nonzero_pc_counts_.begin(), road_nonzero_pc_counts_.end());
        std::sort(nz.begin(), nz.end(), [](const auto& a, const auto& b){ return a.second > b.second; });
        for (std::size_t i = 0; i < std::min<std::size_t>(12, nz.size()); ++i)
            out << " [0x" << std::hex << nz[i].first << std::dec << ':' << nz[i].second << ']';
        out << '\n';
    }
    out << "CPU B road-focus bus activity 0x980..0xa10: R=" << road_focus_reads_
        << " W=" << road_focus_writes_ << " roadW=" << road_focus_road_writes_ << '\n';
    out << "  hottest read-origin PCs:";
    std::vector<std::pair<std::uint32_t, std::uint64_t>> rf_reads(road_focus_read_pc_counts_.begin(), road_focus_read_pc_counts_.end());
    std::sort(rf_reads.begin(), rf_reads.end(), [](const auto& a, const auto& b){ return a.second > b.second; });
    for (std::size_t i = 0; i < std::min<std::size_t>(8, rf_reads.size()); ++i)
        out << " [0x" << std::hex << rf_reads[i].first << std::dec << ':' << rf_reads[i].second << ']';
    if (rf_reads.empty()) out << " none";
    out << '\n';
    out << "CPU B road-data flow source[0x101600..0x102200]: R=" << flow_source_reads_
        << " W=" << flow_source_writes_ << " roadW=" << flow_road_writes_
        << " nonzeroRoadW=" << flow_nonzero_road_writes_ << '\n';
    auto print_hot = [&out](const char* label, const auto& m) {
        out << "  " << label << ':';
        std::vector<std::pair<std::uint32_t, std::uint64_t>> v(m.begin(), m.end());
        std::sort(v.begin(), v.end(), [](const auto& a, const auto& b){ return a.second > b.second; });
        for (std::size_t i = 0; i < std::min<std::size_t>(10, v.size()); ++i)
            out << " [0x" << std::hex << v[i].first << std::dec << ':' << v[i].second << ']';
        if (v.empty()) out << " none";
        out << '\n';
    };
    print_hot("source read PCs", flow_source_read_pc_counts_);
    print_hot("source write PCs", flow_source_write_pc_counts_);
    print_hot("NONZERO road write PCs", flow_nonzero_road_pc_counts_);
    out << "CPU B road-state flag 0x101a5a: R=" << road_flag_reads_ << " W=" << road_flag_writes_ << " nonzeroW=" << road_flag_nonzero_writes_ << '\n';
    print_hot("flag read PCs", road_flag_read_pc_counts_);
    print_hot("flag write PCs", road_flag_write_pc_counts_);
    out << "CPU B pending road-control pair 0x101a58/0x101a5a:\n";
    out << "  value 101a58 R=" << pending_value_reads_ << " W=" << pending_value_writes_ << " nonzeroW=" << pending_value_nonzero_writes_ << '\n';
    print_hot("value read PCs", pending_value_read_pc_counts_);
    print_hot("value write PCs", pending_value_write_pc_counts_);
    out << "  strobe 101a5a R=" << pending_flag_reads_ << " W=" << pending_flag_writes_ << " nonzeroW=" << pending_flag_nonzero_writes_ << '\n';
    print_hot("strobe read PCs", pending_flag_read_pc_counts_);
    print_hot("strobe write PCs", pending_flag_write_pc_counts_);
    out << "  road control 801ffe R=" << road_ctrl_reads_ << " W=" << road_ctrl_writes_ << " nonzeroW=" << road_ctrl_nonzero_writes_ << '\n';
    print_hot("road-control read PCs", road_ctrl_read_pc_counts_);
    print_hot("road-control write PCs", road_ctrl_write_pc_counts_);
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
    for (auto v : palette) {
        f.put(static_cast<char>(v >> 8));
        f.put(static_cast<char>(v));
    }
    if (!f) throw std::runtime_error("Palette dump write failed");
}


static void checkpoint_write_u32(std::ostream& o, std::uint32_t v) {
    const unsigned char b[4]={(unsigned char)v,(unsigned char)(v>>8),(unsigned char)(v>>16),(unsigned char)(v>>24)};
    o.write(reinterpret_cast<const char*>(b),4);
}
static void checkpoint_write_u64(std::ostream& o, std::uint64_t v) {
    checkpoint_write_u32(o,(std::uint32_t)v); checkpoint_write_u32(o,(std::uint32_t)(v>>32));
}
static std::uint32_t checkpoint_read_u32(std::istream& in) {
    unsigned char b[4]{}; in.read(reinterpret_cast<char*>(b),4); if(!in) throw std::runtime_error("Truncated checkpoint");
    return (std::uint32_t)b[0]|((std::uint32_t)b[1]<<8)|((std::uint32_t)b[2]<<16)|((std::uint32_t)b[3]<<24);
}
static std::uint64_t checkpoint_read_u64(std::istream& in) { const auto lo=checkpoint_read_u32(in),hi=checkpoint_read_u32(in); return lo|((std::uint64_t)hi<<32); }
static void checkpoint_write_blob(std::ostream& o,const void* p,std::size_t n){ checkpoint_write_u32(o,(std::uint32_t)n); if(n)o.write(reinterpret_cast<const char*>(p),(std::streamsize)n); }
static void checkpoint_read_blob(std::istream& in,void* p,std::size_t expected){ const auto n=checkpoint_read_u32(in); if(n!=expected)throw std::runtime_error("Checkpoint state-size mismatch"); if(n)in.read(reinterpret_cast<char*>(p),(std::streamsize)n); if(!in)throw std::runtime_error("Truncated checkpoint state"); }

void Bus::save_checkpoint_state(std::ostream& out) const {
    std::vector<const Region*> mutable_regions; for(const auto& r:regions) if(!r.readonly) mutable_regions.push_back(&r);
    checkpoint_write_u32(out,(std::uint32_t)mutable_regions.size());
    for(const auto* r:mutable_regions){ const std::string name=r->name; checkpoint_write_blob(out,name.data(),name.size()); checkpoint_write_u32(out,r->base); checkpoint_write_blob(out,r->bytes.data(),r->bytes.size()); }
    checkpoint_write_blob(out,palette.data(),palette.size()*sizeof(palette[0]));
    checkpoint_write_blob(out,palette_write_count.data(),palette_write_count.size()*sizeof(palette_write_count[0]));
    checkpoint_write_blob(out,palette_first_write_frame.data(),palette_first_write_frame.size()*sizeof(palette_first_write_frame[0]));
    checkpoint_write_blob(out,palette_last_write_frame.data(),palette_last_write_frame.size()*sizeof(palette_last_write_frame[0]));
    checkpoint_write_blob(out,palette_last_write_pc.data(),palette_last_write_pc.size()*sizeof(palette_last_write_pc[0]));
    checkpoint_write_blob(out,palette_first_written_value.data(),palette_first_written_value.size()*sizeof(palette_first_written_value[0]));
    checkpoint_write_blob(out,palette_last_written_value.data(),palette_last_written_value.size()*sizeof(palette_last_written_value[0]));
    checkpoint_write_u32(out,palette_address_); checkpoint_write_u32(out,palette_latch_); checkpoint_write_u32(out,cpu_control_);
    checkpoint_write_u32(out,ioc_port_); checkpoint_write_u32(out,ioc_coin_output_); checkpoint_write_u32(out,ioc_steering_);
    // Preserve the historical checkpoint field for binary compatibility, but never bake
    // research/scenario interventions into machine state.
    const std::array<std::uint8_t,16> checkpoint_ioc_xor{};
    checkpoint_write_blob(out,ioc_last_write_.data(),ioc_last_write_.size()); checkpoint_write_blob(out,checkpoint_ioc_xor.data(),checkpoint_ioc_xor.size());
    checkpoint_write_u32(out,pc_a_); checkpoint_write_u32(out,pc_b_);
}

void Bus::load_checkpoint_state(std::istream& in) {
    const auto count=checkpoint_read_u32(in);
    for(std::uint32_t i=0;i<count;++i){
        const auto name_len=checkpoint_read_u32(in); std::string name(name_len,'\0'); if(name_len)in.read(name.data(),name_len); if(!in)throw std::runtime_error("Truncated checkpoint region name");
        const auto base=checkpoint_read_u32(in); const auto data_len=checkpoint_read_u32(in);
        Region* target=nullptr; for(auto& r:regions) if(!r.readonly && name==r.name && r.base==base){target=&r;break;}
        if(!target || target->bytes.size()!=data_len) throw std::runtime_error("Checkpoint RAM layout mismatch at "+name);
        if(data_len)in.read(reinterpret_cast<char*>(target->bytes.data()),data_len); if(!in)throw std::runtime_error("Truncated checkpoint RAM");
    }
    checkpoint_read_blob(in,palette.data(),palette.size()*sizeof(palette[0]));
    checkpoint_read_blob(in,palette_write_count.data(),palette_write_count.size()*sizeof(palette_write_count[0]));
    checkpoint_read_blob(in,palette_first_write_frame.data(),palette_first_write_frame.size()*sizeof(palette_first_write_frame[0]));
    checkpoint_read_blob(in,palette_last_write_frame.data(),palette_last_write_frame.size()*sizeof(palette_last_write_frame[0]));
    checkpoint_read_blob(in,palette_last_write_pc.data(),palette_last_write_pc.size()*sizeof(palette_last_write_pc[0]));
    checkpoint_read_blob(in,palette_first_written_value.data(),palette_first_written_value.size()*sizeof(palette_first_written_value[0]));
    checkpoint_read_blob(in,palette_last_written_value.data(),palette_last_written_value.size()*sizeof(palette_last_written_value[0]));
    palette_address_=(std::uint16_t)checkpoint_read_u32(in); palette_latch_=(std::uint16_t)checkpoint_read_u32(in); cpu_control_=(std::uint8_t)checkpoint_read_u32(in);
    ioc_port_=(std::uint8_t)checkpoint_read_u32(in); ioc_coin_output_=(std::uint8_t)checkpoint_read_u32(in); ioc_steering_=(std::uint16_t)checkpoint_read_u32(in);
    std::array<std::uint8_t,16> checkpoint_ioc_xor_legacy{};
    checkpoint_read_blob(in,ioc_last_write_.data(),ioc_last_write_.size()); checkpoint_read_blob(in,checkpoint_ioc_xor_legacy.data(),checkpoint_ioc_xor_legacy.size());
    // Input interventions are execution/research context, not authentic machine state.
    // Read and discard the legacy XOR field so all existing checkpoints remain compatible.
    ioc_input_xor_.fill(0);
    ioc_scenario_xor_.fill(0);
    pc_a_=checkpoint_read_u32(in); pc_b_=checkpoint_read_u32(in);
}

Runtime::Runtime(Bytes main_rom, Bytes sub_rom)
    : bus(std::move(main_rom), std::move(sub_rom)) {
    if (active) throw std::runtime_error("Only one dual-Musashi runtime may exist");
    active = this;

    m68k_init();
    m68k_set_cpu_type(M68K_CPU_TYPE_68000);
    m68k_set_instr_hook_callback(runtime_instruction_hook);
    m68k_set_int_ack_callback(runtime_interrupt_ack);

    context_a_.resize(m68k_context_size());
    context_b_.resize(m68k_context_size());
    bus.set_generic_trace_callback([this](char op, BusSpace space, std::uint32_t address,
                                          std::uint32_t value, unsigned size,
                                          std::uint32_t old_value, bool has_old,
                                          const char* region) {
        generic_trace_memory(op, space, address, value, size, old_value, has_old, region);
        provenance_memory(op, space, address, value, size, region);
    });
    bus.set_generic_trace_interest_callback([this](char op, BusSpace space, std::uint32_t address, unsigned size) {
        if (provenance_interested(op, space, address, size)) return true;
        if (!generic_trace_config_.enabled() || !generic_trace_frame_active()) return false;
        const unsigned cpu_bit = space == BusSpace::Main ? 1u : 2u;
        if ((generic_trace_config_.cpu_mask & cpu_bit) == 0) return false;
        const auto current_pc = bus.trace_pc(space);
        const auto end = static_cast<std::uint32_t>((address + size - 1) & 0xffffff);
        auto overlaps = [&](std::uint32_t start, std::uint32_t finish) {
            return address <= finish && end >= start;
        };

        // Arming watches remain visible before the normal trace is armed.
        if (op == 'W') {
            for (const auto& a : generic_trace_config_.arm_mem_writes)
                if (overlaps(a.address, a.address)) return true;
        }

        if (!generic_trace_armed_) return false;
        if (op == 'W' && space == BusSpace::Main && current_pc == 0x1096 && size == 2 &&
            generic_task_slot_selected(address)) return true;
        if (!generic_trace_config_.mem_pc_ranges.empty()) {
            bool pc_ok = false;
            for (const auto& r : generic_trace_config_.mem_pc_ranges) {
                if (current_pc >= r.start && current_pc <= r.end) { pc_ok = true; break; }
            }
            if (!pc_ok) return false;
        }
        if ((op == 'R' && generic_trace_config_.allow_reads) ||
            (op == 'W' && generic_trace_config_.allow_writes)) {
            for (const auto& r : generic_trace_config_.mem_ranges) {
                if (!overlaps(r.start, r.end)) continue;
                if (op == 'R' && r.reads) return true;
                if (op == 'W' && r.writes) return true;
            }
        }
        if (op == 'W') {
            for (const auto& t : generic_trace_config_.triggers)
                if (overlaps(t.start, t.end)) return true;
        }
        return false;
    });
    bus.set_generic_trace_old_value_callback([this](BusSpace space, std::uint32_t address, unsigned size) {
        if (!generic_trace_config_.enabled() || !generic_trace_frame_active()) return false;
        const unsigned cpu_bit = space == BusSpace::Main ? 1u : 2u;
        if ((generic_trace_config_.cpu_mask & cpu_bit) == 0) return false;
        const auto current_pc = bus.trace_pc(space);
        const auto end = static_cast<std::uint32_t>((address + size - 1) & 0xffffff);
        if (!generic_trace_armed_) return false;
        if (space == BusSpace::Main && current_pc == 0x1096 && size == 2 &&
            generic_task_slot_selected(address)) return true;
        if (!generic_trace_config_.mem_pc_ranges.empty()) {
            bool pc_ok = false;
            for (const auto& r : generic_trace_config_.mem_pc_ranges) {
                if (current_pc >= r.start && current_pc <= r.end) { pc_ok = true; break; }
            }
            if (!pc_ok) return false;
        }
        for (const auto& r : generic_trace_config_.mem_ranges) {
            if (r.change_only && address <= r.end && end >= r.start) return true;
        }
        for (const auto& t : generic_trace_config_.triggers) {
            if (t.kind == TraceTriggerKind::Change && address <= t.end && end >= t.start) return true;
        }
        return false;
    });
}


void Runtime::save_checkpoint(const std::filesystem::path& path, unsigned frame) const {
    if(path.has_parent_path()) std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path,std::ios::binary); if(!out) throw std::runtime_error("Cannot create checkpoint: "+path.string());
    const char magic[8]={'C','H','Q','S','T','A','T','E'}; out.write(magic,8); checkpoint_write_u32(out,1); checkpoint_write_u32(out,frame);
    checkpoint_write_blob(out,context_a_.data(),context_a_.size()); checkpoint_write_blob(out,context_b_.data(),context_b_.size());
    checkpoint_write_u64(out,cycles_a); checkpoint_write_u64(out,cycles_b); checkpoint_write_u64(out,instructions_a); checkpoint_write_u64(out,instructions_b);
    checkpoint_write_u32(out,sub_was_enabled_?1u:0u); bus.save_checkpoint_state(out); if(!out)throw std::runtime_error("Checkpoint write failed: "+path.string());
}

static void repair_loaded_musashi_context(std::vector<std::uint8_t>& context) {
    // CHQSTATE v1 stores Musashi's complete m68ki_cpu_core.  That structure contains
    // host-process pointers (cycle tables and callback function addresses).  They are
    // valid for an F9 save/load inside one process, but ASLR makes them invalid when a
    // checkpoint is loaded by a newly-started process.  Rebind every host-dependent
    // field while preserving the architectural CPU state restored from the checkpoint.
    if (context.size() != sizeof(m68ki_cpu_core))
        throw std::runtime_error("Checkpoint Musashi context-size mismatch");

    m68k_set_context(context.data());
    m68k_set_cpu_type(M68K_CPU_TYPE_68000); // repairs cycle-table pointers/configuration

    // Do not retain any callback address serialized by a different process.
    m68ki_cpu.int_ack_callback = runtime_interrupt_ack;
    m68ki_cpu.bkpt_ack_callback = nullptr;
    m68ki_cpu.reset_instr_callback = nullptr;
    m68ki_cpu.cmpild_instr_callback = nullptr;
    m68ki_cpu.rte_instr_callback = nullptr;
    m68ki_cpu.tas_instr_callback = nullptr;
    m68ki_cpu.illg_instr_callback = nullptr;
    m68ki_cpu.trap_instr_callback = nullptr;
    m68ki_cpu.pc_changed_callback = nullptr;
    m68ki_cpu.set_fc_callback = nullptr;
    m68ki_cpu.instr_hook_callback = runtime_instruction_hook;

    m68k_get_context(context.data());
}

unsigned Runtime::load_checkpoint(const std::filesystem::path& path) {
    std::ifstream in(path,std::ios::binary); if(!in)throw std::runtime_error("Cannot open checkpoint: "+path.string());
    char magic[8]{};in.read(magic,8);if(std::memcmp(magic,"CHQSTATE",8)!=0)throw std::runtime_error("Invalid checkpoint magic");
    const auto version=checkpoint_read_u32(in); if(version!=1)throw std::runtime_error("Unsupported checkpoint version"); const auto frame=checkpoint_read_u32(in);
    checkpoint_read_blob(in,context_a_.data(),context_a_.size()); checkpoint_read_blob(in,context_b_.data(),context_b_.size());
    cycles_a=checkpoint_read_u64(in);cycles_b=checkpoint_read_u64(in);instructions_a=checkpoint_read_u64(in);instructions_b=checkpoint_read_u64(in);sub_was_enabled_=checkpoint_read_u32(in)!=0;
    bus.load_checkpoint_state(in);

    repair_loaded_musashi_context(context_a_);
    repair_loaded_musashi_context(context_b_);

    fault.clear(); generic_trace_history_.clear(); forensic_history_.clear(); provenance_cpu_a_={}; provenance_cpu_b_={}; return frame;
}

Runtime::~Runtime() {
    if (active == this) active = nullptr;
}

void Runtime::enable_boot_debug(bool enabled, unsigned sample_limit) {
    boot_debug_ = enabled;
    boot_sample_limit_ = std::max(1u, sample_limit);
}

void Runtime::open_boot_debug_log(const std::filesystem::path& path, std::uint64_t limit) {
    bus.open_boot_debug_log(path, limit);
}

void Runtime::enable_sub_debug(bool enabled, unsigned sample_limit) {
    sub_debug_ = enabled;
    sub_sample_limit_ = std::max(1u, sample_limit);
}

void Runtime::open_sub_debug_log(const std::filesystem::path& path, std::uint64_t limit) {
    bus.open_sub_debug_log(path, limit);
}

void Runtime::open_handshake_log(const std::filesystem::path& path, std::uint64_t limit) {
    bus.open_handshake_log(path, limit);
}

void Runtime::enable_road_focus_debug(bool enabled, unsigned sample_limit) {
    road_focus_debug_ = enabled;
    road_focus_sample_limit_ = sample_limit;
}

void Runtime::open_road_focus_log(const std::filesystem::path& path, std::uint64_t limit) {
    bus.open_road_focus_log(path, limit);
}

void Runtime::open_road_data_flow_log(const std::filesystem::path& path, std::uint64_t limit) {
    bus.open_road_data_flow_log(path, limit);
}

void Runtime::open_road_state_log(const std::filesystem::path& path, std::uint64_t limit) {
    road_state_sample_limit_ = static_cast<unsigned>(std::min<std::uint64_t>(limit, 200000));
    bus.open_road_state_log(path, limit);
}


bool Runtime::generic_task_slot_selected(std::uint32_t slot) const {
    if (generic_trace_config_.trace_all_tasks)
        return slot >= 0x100000 && slot <= 0x1000f0 && ((slot - 0x100000) & 0x0f) == 0;
    return std::find(generic_trace_config_.task_slots.begin(), generic_trace_config_.task_slots.end(), slot)
        != generic_trace_config_.task_slots.end();
}

void Runtime::configure_generic_trace(const TraceConfig& config, const std::filesystem::path& path) {
    if (generic_trace_log_.is_open()) { generic_trace_log_.flush(); generic_trace_log_.close(); }
    generic_trace_config_ = config;
    generic_trigger_fired_.assign(config.triggers.size(), false);
    generic_trace_lines_ = 0;
    generic_trace_mem_events_ = 0;
    generic_trace_pc_events_ = 0;
    generic_trace_triggers_ = 0;
    generic_trace_history_.clear();
    generic_after_remaining_ = 0;
    generic_trace_limit_noted_ = false;
    generic_trace_armed_ = config.arm_pc_ranges.empty() && config.arm_mem_writes.empty();

    if (!config.enabled())
        return;

    generic_trace_log_.open(path);
    if (!generic_trace_log_)
        throw std::runtime_error("Cannot create generic trace log: " + path.string());

    generic_trace_log_ << "# Chase H.Q. Native v0.32 graphics/sprite diagnostic runtime tracer. Hex values unless noted.\n";
    generic_trace_log_ << "# CPU mask="
                       << (config.cpu_mask == 1 ? "A" : config.cpu_mask == 2 ? "B" : "both")
                       << " max_lines=" << std::dec << config.max_lines
                       << " before=" << config.before << " after=" << config.after
                       << " frame_window=" << config.from_frame << ':' << config.to_frame
                       << " armed=" << (generic_trace_armed_ ? "yes" : "no") << '\n';
    for (const auto& r : config.mem_ranges) {
        generic_trace_log_ << "# MEM " << std::hex << std::setfill('0') << std::setw(6) << r.start
                           << '-' << std::setw(6) << r.end
                           << " mode=" << (r.reads && r.writes ? "rw" : r.reads ? "r" : "w")
                           << (r.nonzero_only ? " nonzero" : "")
                           << (r.change_only ? " change-only" : "") << '\n';
    }
    for (const auto& r : config.pc_ranges) {
        generic_trace_log_ << "# PC  " << std::hex << std::setfill('0') << std::setw(6) << r.start
                           << '-' << std::setw(6) << r.end << '\n';
    }
    for (const auto& r : config.mem_pc_ranges) {
        generic_trace_log_ << "# MEM_PC " << std::hex << std::setfill('0') << std::setw(6) << r.start
                           << '-' << std::setw(6) << r.end << '\n';
    }
    for (const auto& r : config.arm_pc_ranges) {
        generic_trace_log_ << "# ARM_PC " << std::hex << std::setfill('0') << std::setw(6) << r.start
                           << '-' << std::setw(6) << r.end << '\n';
    }
    for (const auto& a : config.arm_mem_writes) {
        generic_trace_log_ << "# ARM_MEM " << std::hex << std::setfill('0') << std::setw(6) << a.address
                           << " value=" << std::setw(8) << a.value << '\n';
    }
    if (config.write_value_filter)
        generic_trace_log_ << "# WRITE_VALUE " << std::hex << std::setfill('0') << std::setw(8) << *config.write_value_filter << '\n';
    if (config.trace_all_tasks) generic_trace_log_ << "# TASKS all scheduler slots 100000-1000f0\n";
    for (auto slot : config.task_slots)
        generic_trace_log_ << "# TASK " << std::hex << std::setfill('0') << std::setw(6) << slot << '\n';
    for (const auto& t : config.triggers) {
        const char* kind = t.kind == TraceTriggerKind::Write ? "write" :
                           t.kind == TraceTriggerKind::NonZeroWrite ? "nonzero-write" : "change";
        generic_trace_log_ << "# TRIGGER " << kind << ' '
                           << std::hex << std::setfill('0') << std::setw(6) << t.start
                           << '-' << std::setw(6) << t.end << '\n';
    }
}


void Runtime::configure_provenance(const ProvenanceConfig& config, const std::filesystem::path& directory) {
    if (provenance_flow_csv_.is_open()) { provenance_flow_csv_.flush(); provenance_flow_csv_.close(); }
    if (provenance_stack_log_.is_open()) { provenance_stack_log_.flush(); provenance_stack_log_.close(); }
    provenance_config_ = config;
    provenance_dir_ = directory;
    provenance_events_ = 0;
    provenance_limit_noted_ = false;
    provenance_cpu_a_ = {};
    provenance_cpu_b_ = {};
    provenance_edges_.clear();
    provenance_functions_.clear();
    if (!config.enabled()) return;
    std::filesystem::create_directories(directory);
    provenance_flow_csv_.open(directory / "provenance_address_flow.csv");
    if (!provenance_flow_csv_) throw std::runtime_error("Cannot create provenance_address_flow.csv");
    provenance_flow_csv_ << "frame,cpu,op,pc,function,bits,address,value,region,call_depth,call_stack\n";
    provenance_stack_log_.open(directory / "provenance_callstack.log");
    if (!provenance_stack_log_) throw std::runtime_error("Cannot create provenance_callstack.log");
    provenance_stack_log_ << "# Chase H.Q. Native v" << kNativeVersion << " dynamic execution provenance\n";
    provenance_stack_log_ << "# frame_window=" << config.from_frame << ':' << config.to_frame
                          << " max_depth=" << config.max_depth << " max_events=" << config.max_events << "\n";
    for (const auto& r : config.follow_ranges)
        provenance_stack_log_ << "# FOLLOW " << std::hex << std::setfill('0') << std::setw(6) << r.start
                              << '-' << std::setw(6) << r.end << std::dec << "\n";
}

bool Runtime::provenance_interested(char op, BusSpace, std::uint32_t address, unsigned size) const {
    if (!provenance_config_.enabled()) return false;
    if (provenance_frame_ < provenance_config_.from_frame || provenance_frame_ > provenance_config_.to_frame) return false;
    if (op == 'R' && !provenance_config_.reads) return false;
    if (op == 'W' && !provenance_config_.writes) return false;
    if (provenance_config_.follow_ranges.empty()) return false;
    const auto end = static_cast<std::uint32_t>((address + size - 1) & 0xffffff);
    for (const auto& r : provenance_config_.follow_ranges)
        if (address <= r.end && end >= r.start) return true;
    return false;
}

std::string Runtime::provenance_stack_string(BusSpace space) const {
    const auto& st = space == BusSpace::Main ? provenance_cpu_a_ : provenance_cpu_b_;
    std::ostringstream ss;
    const std::size_t begin = st.stack.size() > provenance_config_.max_depth ? st.stack.size() - provenance_config_.max_depth : 0;
    bool first = true;
    for (std::size_t i = begin; i < st.stack.size(); ++i) {
        if (!first) ss << '>';
        first = false;
        ss << std::hex << std::setfill('0') << std::setw(6) << st.stack[i].function_pc;
    }
    if (st.current_function != 0 && (st.stack.empty() || st.stack.back().function_pc != st.current_function)) {
        if (!first) ss << '>';
        ss << std::hex << std::setfill('0') << std::setw(6) << st.current_function;
    }
    return ss.str();
}

void Runtime::provenance_instruction(std::uint32_t pc) {
    if (!provenance_config_.enabled()) return;
    auto& st = active_space == BusSpace::Main ? provenance_cpu_a_ : provenance_cpu_b_;
    const char cpu = active_space == BusSpace::Main ? 'A' : 'B';

    // Resolve the previous instruction using the actual next PC. This makes indirect JSR/BSR
    // targets accurate without having to decode effective-address modes ourselves.
    if (st.prev_pc != 0xffffffffu) {
        if (st.prev_return) {
            if (!st.stack.empty()) st.stack.pop_back();
            st.current_function = st.stack.empty() ? 0 : st.stack.back().function_pc;
        }
        if (st.prev_call) {
            const std::uint32_t ret = (st.prev_pc + st.prev_size) & 0xffffff;
            st.stack.push_back({st.prev_pc, pc, ret});
            st.current_function = pc;
            if (provenance_config_.trace_callgraph)
                ++provenance_edges_[{cpu,{st.prev_pc,pc}}];
            if (provenance_config_.profile_functions)
                ++provenance_functions_[{cpu,pc}].calls;
            if (provenance_stack_log_ && provenance_frame_ >= provenance_config_.from_frame && provenance_frame_ <= provenance_config_.to_frame && provenance_config_.verbose_calls) {
                provenance_stack_log_ << "CALL frame=" << std::dec << provenance_frame_ << " cpu=" << cpu
                    << " from=" << std::hex << std::setfill('0') << std::setw(6) << st.prev_pc
                    << " to=" << std::setw(6) << pc << " return=" << std::setw(6) << ret
                    << " depth=" << std::dec << st.stack.size() << " stack=" << provenance_stack_string(active_space) << "\n";
            }
        }
        // Exception/interrupt returns and unusual control flow can desynchronise a pure shadow
        // stack. Re-synchronise opportunistically when execution reaches a known return address.
        while (!st.stack.empty() && pc == st.stack.back().return_pc && !st.prev_call) {
            st.stack.pop_back();
            st.current_function = st.stack.empty() ? 0 : st.stack.back().function_pc;
        }
    }

    if (provenance_config_.profile_functions && provenance_frame_ >= provenance_config_.from_frame && provenance_frame_ <= provenance_config_.to_frame) {
        auto key = std::make_pair(cpu, st.current_function ? st.current_function : pc);
        ++provenance_functions_[key].instructions;
    }

    unsigned size = 0;
    auto dis = disassemble_space(active_space, pc, &size);
    std::string low = dis;
    std::transform(low.begin(), low.end(), low.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
    auto starts = [&](const char* x){ return low.rfind(x, 0) == 0; };
    st.prev_call = starts("bsr") || starts("jsr");
    st.prev_return = starts("rts") || starts("rte") || starts("rtr");
    st.prev_pc = pc;
    st.prev_size = size ? size : 2;
}

void Runtime::provenance_memory(char op, BusSpace space, std::uint32_t address, std::uint32_t value, unsigned size, const char* region) {
    if (!provenance_interested(op, space, address, size)) return;
    if (provenance_events_ >= provenance_config_.max_events) {
        if (!provenance_limit_noted_ && provenance_stack_log_) {
            provenance_stack_log_ << "# provenance event limit reached; summary counters continue\n";
            provenance_limit_noted_ = true;
        }
        return;
    }
    ++provenance_events_;
    const char cpu = space == BusSpace::Main ? 'A' : 'B';
    const auto pc = bus.trace_pc(space);
    auto& st = space == BusSpace::Main ? provenance_cpu_a_ : provenance_cpu_b_;
    const auto fn = st.current_function ? st.current_function : pc;
    if (provenance_config_.profile_functions) {
        auto& prof = provenance_functions_[{cpu,fn}];
        if (op == 'R') ++prof.reads; else ++prof.writes;
    }
    auto stack = provenance_stack_string(space);
    if (provenance_flow_csv_) {
        provenance_flow_csv_ << std::dec << provenance_frame_ << ',' << cpu << ',' << op << ','
            << std::hex << std::setfill('0') << std::setw(6) << pc << ',' << std::setw(6) << fn << ','
            << std::dec << size*8 << ',' << std::hex << std::setw(6) << address << ',' << std::setw(size*2) << value << ','
            << (region ? region : "UNMAPPED") << ',' << std::dec << st.stack.size() << ",\"" << stack << "\"\n";
    }
    if (provenance_stack_log_) {
        provenance_stack_log_ << "MEM_" << op << " frame=" << std::dec << provenance_frame_ << " cpu=" << cpu
            << " pc=" << std::hex << std::setfill('0') << std::setw(6) << pc
            << " fn=" << std::setw(6) << fn << " bits=" << std::dec << size*8
            << " addr=" << std::hex << std::setw(6) << address << " value=" << std::setw(size*2) << value
            << " depth=" << std::dec << st.stack.size() << " stack=" << stack << "\n";
    }
}

void Runtime::write_provenance_reports() {
    if (!provenance_config_.enabled() || provenance_dir_.empty()) return;
    if (provenance_flow_csv_) provenance_flow_csv_.flush();
    if (provenance_stack_log_) provenance_stack_log_.flush();
    if (provenance_config_.trace_callgraph) {
        std::ofstream f(provenance_dir_ / "provenance_callgraph.csv");
        f << "cpu,caller_pc,callee_pc,count\n";
        for (const auto& [k,count] : provenance_edges_)
            f << k.first << ',' << std::hex << std::setfill('0') << std::setw(6) << k.second.first << ','
              << std::setw(6) << k.second.second << ',' << std::dec << count << "\n";
    }
    if (provenance_config_.profile_functions) {
        std::ofstream f(provenance_dir_ / "provenance_functions.csv");
        f << "cpu,function_pc,instructions,reads,writes,calls\n";
        for (const auto& [k,p] : provenance_functions_)
            f << k.first << ',' << std::hex << std::setfill('0') << std::setw(6) << k.second << ',' << std::dec
              << p.instructions << ',' << p.reads << ',' << p.writes << ',' << p.calls << "\n";
    }
    if (provenance_config_.trace_callgraph) {
        std::ofstream f(provenance_dir_ / "provenance_callgraph.dot");
        f << "digraph chasehq_callgraph {\n  rankdir=LR;\n";
        for (const auto& [k,count] : provenance_edges_) {
            std::ostringstream a,b; a << k.first << "_" << std::hex << std::setfill('0') << std::setw(6) << k.second.first;
            b << k.first << "_" << std::hex << std::setfill('0') << std::setw(6) << k.second.second;
            f << "  \"" << a.str() << "\" -> \"" << b.str() << "\" [label=\"" << std::dec << count << "\"];\n";
        }
        f << "}\n";
    }
    {
        std::ofstream f(provenance_dir_ / "provenance_summary.txt");
        f << "Chase H.Q. Native v" << kNativeVersion << " Execution Provenance summary\n"
          << "events=" << provenance_events_ << "\n"
          << "call_edges=" << provenance_edges_.size() << "\n"
          << "functions=" << provenance_functions_.size() << "\n"
          << "frame_window=" << provenance_config_.from_frame << ':' << provenance_config_.to_frame << "\n"
          << "NOTE: shadow call stacks are dynamically reconstructed from observed BSR/JSR and return flow.\n"
          << "      Interrupt/exception paths are opportunistically re-synchronised at known return addresses.\n";
    }
}

void Runtime::generic_trace_line(const std::string& line) {
    if (!generic_trace_log_.is_open())
        return;
    if (generic_trace_lines_ >= generic_trace_config_.max_lines) {
        if (!generic_trace_limit_noted_) {
            generic_trace_log_ << "# generic trace line limit reached; counters continue\n";
            generic_trace_limit_noted_ = true;
        }
        return;
    }
    generic_trace_log_ << line << '\n';
    ++generic_trace_lines_;
}

void Runtime::generic_trace_instruction(std::uint32_t pc) {
    if (!generic_trace_config_.enabled() || !generic_trace_frame_active())
        return;
    const unsigned cpu_bit = active_space == BusSpace::Main ? 1u : 2u;
    if ((generic_trace_config_.cpu_mask & cpu_bit) == 0)
        return;

    if (!generic_trace_armed_) {
        bool arm = false;
        for (const auto& r : generic_trace_config_.arm_pc_ranges) {
            if (pc >= r.start && pc <= r.end) { arm = true; break; }
        }
        if (!arm) return;
        generic_trace_armed_ = true;
        std::ostringstream a;
        a << "TRACE_ARM frame=" << std::dec << generic_trace_frame_
          << " cpu=" << (active_space == BusSpace::Main ? 'A' : 'B')
          << " pc=" << std::hex << std::setfill('0') << std::setw(6) << pc << " kind=pc";
        generic_trace_line(a.str());
    }

    bool pc_match = false;
    for (const auto& r : generic_trace_config_.pc_ranges) {
        if (pc >= r.start && pc <= r.end) {
            pc_match = true;
            break;
        }
    }

    // v0.26.1 performance hotfix: the rolling BEFORE buffer stores only CPU+PC.
    // Register snapshots and disassembly are generated only for lines that will
    // actually be emitted (explicit PC watches or active post-trigger context).
    if (generic_trace_config_.before != 0) {
        generic_trace_history_.push_back({active_space, pc});
        while (generic_trace_history_.size() > generic_trace_config_.before)
            generic_trace_history_.pop_front();
    }

    const bool can_emit = generic_trace_lines_ < generic_trace_config_.max_lines;
    const bool need_full_snapshot = generic_after_remaining_ != 0 || (pc_match && can_emit);
    if (!need_full_snapshot) {
        if (pc_match)
            ++generic_trace_pc_events_;
        return;
    }

    TraceSnapshot snap;
    snap.space = active_space;
    snap.pc = pc;
    snap.sr = m68k_get_reg(nullptr, M68K_REG_SR);
    for (int i = 0; i < 8; ++i)
        snap.regs[static_cast<std::size_t>(i)] =
            m68k_get_reg(nullptr, static_cast<m68k_register_t>(M68K_REG_D0 + i));
    for (int i = 0; i < 8; ++i)
        snap.regs[static_cast<std::size_t>(8 + i)] =
            m68k_get_reg(nullptr, static_cast<m68k_register_t>(M68K_REG_A0 + i));

    auto format_snapshot = [&](const char* tag, const TraceSnapshot& x) {
        std::ostringstream ss;
        ss << tag << " frame=" << std::dec << generic_trace_frame_
           << " cpu=" << (x.space == BusSpace::Main ? 'A' : 'B')
           << " pc=" << std::hex << std::setfill('0') << std::setw(6) << x.pc
           << " sr=" << std::setw(4) << x.sr;
        for (int i = 0; i < 8; ++i)
            ss << " d" << i << '=' << std::setw(8) << x.regs[static_cast<std::size_t>(i)];
        for (int i = 0; i < 8; ++i)
            ss << " a" << i << '=' << std::setw(8) << x.regs[static_cast<std::size_t>(8 + i)];
        ss << " | " << disassemble_space(x.space, x.pc, nullptr);
        return ss.str();
    };

    if (pc_match) {
        ++generic_trace_pc_events_;
        if (can_emit)
            generic_trace_line(format_snapshot("EXEC", snap));
    }

    if (generic_after_remaining_ != 0) {
        generic_trace_line(format_snapshot("CTX_AFTER", snap));
        --generic_after_remaining_;
        if (generic_after_remaining_ == 0)
            generic_trace_line("CTX_END");
    }
}

void Runtime::generic_trace_memory(char op, BusSpace space, std::uint32_t address, std::uint32_t value,
                                   unsigned size, std::uint32_t old_value, bool has_old, const char* region_name) {
    if (!generic_trace_config_.enabled() || !generic_trace_frame_active())
        return;
    const unsigned cpu_bit = space == BusSpace::Main ? 1u : 2u;
    if ((generic_trace_config_.cpu_mask & cpu_bit) == 0)
        return;

    const auto pc = bus.trace_pc(space);
    const auto end = static_cast<std::uint32_t>((address + size - 1) & 0xffffff);

    if (!generic_trace_armed_ && op == 'W') {
        for (const auto& a : generic_trace_config_.arm_mem_writes) {
            if (address == a.address && value == a.value) {
                generic_trace_armed_ = true;
                std::ostringstream ar;
                ar << "TRACE_ARM frame=" << std::dec << generic_trace_frame_
                   << " cpu=" << (space == BusSpace::Main ? 'A' : 'B')
                   << " pc=" << std::hex << std::setfill('0') << std::setw(6) << pc
                   << " addr=" << std::setw(6) << address << " value=" << std::setw(size * 2) << value
                   << " kind=mem-write-value";
                generic_trace_line(ar.str());
                break;
            }
        }
    }
    if (!generic_trace_armed_) return;

    // Scheduler task-state decoder: $1096 writes D1 to the first word of the current task record.
    if (op == 'W' && space == BusSpace::Main && pc == 0x1096 && size == 2 &&
        generic_task_slot_selected(address) && has_old && old_value != value) {
        std::ostringstream ts;
        ts << "TASK_STATE frame=" << std::dec << generic_trace_frame_
           << " slot=" << std::hex << std::setfill('0') << std::setw(6) << address
           << " old=" << std::setw(4) << old_value << " new=" << std::setw(4) << value
           << " pc=" << std::setw(6) << pc;
        generic_trace_line(ts.str());
    }

    if (!generic_trace_config_.mem_pc_ranges.empty()) {
        bool pc_ok = false;
        for (const auto& r : generic_trace_config_.mem_pc_ranges) {
            if (pc >= r.start && pc <= r.end) { pc_ok = true; break; }
        }
        if (!pc_ok) return;
    }
    auto overlaps = [&](std::uint32_t start, std::uint32_t finish) {
        return address <= finish && end >= start;
    };

    bool log_mem = false;
    if ((op == 'R' && generic_trace_config_.allow_reads) ||
        (op == 'W' && generic_trace_config_.allow_writes)) {
        for (const auto& r : generic_trace_config_.mem_ranges) {
            if (!overlaps(r.start, r.end))
                continue;
            if (op == 'R' && !r.reads)
                continue;
            if (op == 'W' && !r.writes)
                continue;
            if (r.nonzero_only && (op != 'W' || value == 0))
                continue;
            if (r.change_only && (op != 'W' || !has_old || old_value == value))
                continue;
            if (op == 'W' && generic_trace_config_.write_value_filter &&
                value != *generic_trace_config_.write_value_filter)
                continue;
            log_mem = true;
            break;
        }
    }

    if (log_mem) {
        ++generic_trace_mem_events_;
        std::ostringstream ss;
        ss << "MEM_" << op
           << " frame=" << std::dec << generic_trace_frame_
           << " cpu=" << (space == BusSpace::Main ? 'A' : 'B')
           << " pc=" << std::hex << std::setfill('0') << std::setw(6) << pc
           << " bits=" << std::dec << size * 8
           << " addr=" << std::hex << std::setw(6) << address
           << " value=" << std::setw(size * 2) << value;
        if (op == 'W' && has_old)
            ss << " old=" << std::setw(size * 2) << old_value;
        ss << " region=" << (region_name ? region_name : "UNMAPPED");
        generic_trace_line(ss.str());
    }

    if (op != 'W')
        return;

    for (std::size_t i = 0; i < generic_trace_config_.triggers.size(); ++i) {
        if (i < generic_trigger_fired_.size() && generic_trigger_fired_[i])
            continue;
        const auto& t = generic_trace_config_.triggers[i];
        if (!overlaps(t.start, t.end))
            continue;

        bool fire = false;
        switch (t.kind) {
        case TraceTriggerKind::Write:
            fire = true;
            break;
        case TraceTriggerKind::NonZeroWrite:
            fire = value != 0;
            break;
        case TraceTriggerKind::Change:
            fire = has_old && old_value != value;
            break;
        }
        if (!fire)
            continue;

        if (i < generic_trigger_fired_.size())
            generic_trigger_fired_[i] = true;
        ++generic_trace_triggers_;

        std::ostringstream trigger;
        trigger << "TRIGGER frame=" << std::dec << generic_trace_frame_
                << " cpu=" << (space == BusSpace::Main ? 'A' : 'B')
                << " pc=" << std::hex << std::setfill('0') << std::setw(6) << pc
                << " addr=" << std::setw(6) << address
                << " bits=" << std::dec << size * 8
                << " value=" << std::hex << std::setw(size * 2) << value;
        if (has_old)
            trigger << " old=" << std::setw(size * 2) << old_value;
        trigger << " kind="
                << (t.kind == TraceTriggerKind::Write ? "write" :
                    t.kind == TraceTriggerKind::NonZeroWrite ? "nonzero-write" : "change");
        generic_trace_line(trigger.str());

        for (const auto& x : generic_trace_history_) {
            std::ostringstream ss;
            ss << "CTX_BEFORE frame=" << std::dec << generic_trace_frame_
               << " cpu=" << (x.space == BusSpace::Main ? 'A' : 'B')
               << " pc=" << std::hex << std::setfill('0') << std::setw(6) << x.pc
               << " | " << disassemble_space(x.space, x.pc, nullptr);
            generic_trace_line(ss.str());
        }
        generic_after_remaining_ = generic_trace_config_.after;
    }
}

std::string Runtime::disassemble_space(BusSpace space, std::uint32_t pc, unsigned* size_out) const {
    char buffer[256]{};
    std::array<unsigned char, 32> raw{};
    const auto& rom = space == BusSpace::Main ? bus.regions[0].bytes : bus.regions[12].bytes;
    for (std::size_t i = 0; i < raw.size(); ++i) {
        const auto address = pc + static_cast<std::uint32_t>(i);
        if (address < rom.size()) raw[i] = rom[address];
    }
    const unsigned size = m68k_disassemble_raw(buffer, pc, raw.data(), raw.data(), M68K_CPU_TYPE_68000);
    if (size_out) *size_out = size;
    return buffer;
}

void Runtime::log_loop_sample(std::uint32_t pc) {
    if (!boot_debug_ || boot_samples_ >= boot_sample_limit_) return;

    unsigned size = 0;
    const auto disasm = disassemble_space(BusSpace::Main, pc, &size);
    const auto d0 = m68k_get_reg(nullptr, M68K_REG_D0);
    const auto d1 = m68k_get_reg(nullptr, M68K_REG_D1);
    const auto d2 = m68k_get_reg(nullptr, M68K_REG_D2);
    const auto d3 = m68k_get_reg(nullptr, M68K_REG_D3);
    const auto a0 = m68k_get_reg(nullptr, M68K_REG_A0);
    const auto a1 = m68k_get_reg(nullptr, M68K_REG_A1);
    const auto a2 = m68k_get_reg(nullptr, M68K_REG_A2);
    const auto a3 = m68k_get_reg(nullptr, M68K_REG_A3);
    const auto sp = m68k_get_reg(nullptr, M68K_REG_SP);
    const auto sr = m68k_get_reg(nullptr, M68K_REG_SR);

    std::ostringstream ss;
    ss << "STEP n=" << std::dec << boot_samples_
       << " pc=" << std::hex << std::setfill('0') << std::setw(6) << pc
       << " sr=" << std::setw(4) << sr
       << " d0=" << std::setw(8) << d0 << " d1=" << std::setw(8) << d1
       << " d2=" << std::setw(8) << d2 << " d3=" << std::setw(8) << d3
       << " a0=" << std::setw(8) << a0 << " a1=" << std::setw(8) << a1
       << " a2=" << std::setw(8) << a2 << " a3=" << std::setw(8) << a3
       << " sp=" << std::setw(8) << sp
       << " | " << disasm;
    bus.debug_note(ss.str());

    if (previous_debug_pc_ != 0xffffffffu &&
        pc != previous_debug_pc_ + previous_debug_size_) {
        std::ostringstream branch;
        branch << "FLOW from=" << std::hex << std::setw(6) << std::setfill('0') << previous_debug_pc_
               << " to=" << std::setw(6) << pc << " (non-sequential/taken branch, exception, or return)";
        bus.debug_note(branch.str());
    }

    previous_debug_pc_ = pc;
    previous_debug_size_ = size;
    ++boot_samples_;
}

void Runtime::log_sub_sample(std::uint32_t pc) {
    if (!sub_debug_ || sub_samples_ >= sub_sample_limit_) return;

    unsigned size = 0;
    const auto disasm = disassemble_space(BusSpace::Sub, pc, &size);
    std::ostringstream ss;
    ss << "STEP n=" << std::dec << sub_samples_
       << " pc=" << std::hex << std::setfill('0') << std::setw(6) << pc
       << " sr=" << std::setw(4) << m68k_get_reg(nullptr, M68K_REG_SR);
    for (int i = 0; i < 8; ++i)
        ss << " d" << i << '=' << std::setw(8) << m68k_get_reg(nullptr, static_cast<m68k_register_t>(M68K_REG_D0 + i));
    for (int i = 0; i < 8; ++i)
        ss << " a" << i << '=' << std::setw(8) << m68k_get_reg(nullptr, static_cast<m68k_register_t>(M68K_REG_A0 + i));
    ss << " | " << disasm;
    bus.sub_debug_note(ss.str());

    if (previous_sub_debug_pc_ != 0xffffffffu && pc != previous_sub_debug_pc_ + previous_sub_debug_size_) {
        std::ostringstream branch;
        branch << "FLOW from=" << std::hex << std::setfill('0') << std::setw(6) << previous_sub_debug_pc_
               << " to=" << std::setw(6) << pc << " (non-sequential/taken branch, exception, or return)";
        bus.sub_debug_note(branch.str());
    }
    previous_sub_debug_pc_ = pc;
    previous_sub_debug_size_ = size;
    ++sub_samples_;
}


void Runtime::log_sub_handler_sample(std::uint32_t pc) {
    if (!sub_debug_ || sub_handler_samples_ >= sub_sample_limit_) return;
    unsigned size = 0;
    const auto disasm = disassemble_space(BusSpace::Sub, pc, &size);
    std::ostringstream ss;
    ss << "HANDLER n=" << std::dec << sub_handler_samples_
       << " pc=" << std::hex << std::setfill('0') << std::setw(6) << pc
       << " sr=" << std::setw(4) << m68k_get_reg(nullptr, M68K_REG_SR);
    for (int i = 0; i < 4; ++i)
        ss << " d" << i << '=' << std::setw(8) << m68k_get_reg(nullptr, static_cast<m68k_register_t>(M68K_REG_D0 + i));
    for (int i = 0; i < 6; ++i)
        ss << " a" << i << '=' << std::setw(8) << m68k_get_reg(nullptr, static_cast<m68k_register_t>(M68K_REG_A0 + i));
    ss << " | " << disasm;
    bus.sub_debug_note(ss.str());
    ++sub_handler_samples_;
}

void Runtime::log_road_focus_sample(std::uint32_t pc) {
    if (!road_focus_debug_ || road_focus_samples_ >= road_focus_sample_limit_) return;
    unsigned size = 0;
    const auto disasm = disassemble_space(BusSpace::Sub, pc, &size);
    std::ostringstream ss;
    ss << "EXEC n=" << std::dec << road_focus_samples_
       << " pc=" << std::hex << std::setfill('0') << std::setw(6) << pc
       << " sr=" << std::setw(4) << m68k_get_reg(nullptr, M68K_REG_SR);
    for (int i = 0; i < 8; ++i)
        ss << " d" << i << '=' << std::setw(8) << m68k_get_reg(nullptr, static_cast<m68k_register_t>(M68K_REG_D0 + i));
    for (int i = 0; i < 8; ++i)
        ss << " a" << i << '=' << std::setw(8) << m68k_get_reg(nullptr, static_cast<m68k_register_t>(M68K_REG_A0 + i));
    ss << " | " << disasm;
    bus.road_focus_note(ss.str());
    ++road_focus_samples_;
}

void Runtime::write_forensic_history(const std::filesystem::path& path) const {
    std::ofstream f(path);
    if (!f) throw std::runtime_error("Cannot create forensic history: " + path.string());
    f << "# Chase H.Q. Native v" << kNativeVersion << " rolling pre-break execution history\n";
    f << "# entries=" << forensic_history_.size() << " (oldest to newest)\n";
    for (const auto& e : forensic_history_)
        f << (e.space == BusSpace::Main ? 'A' : 'B') << ",0x" << std::hex << std::setfill('0') << std::setw(6) << e.pc << std::dec << "\n";
}

void Runtime::dump_sub_dispatch_table() {
    bus.sub_debug_note("# CPU B dispatch table @00052e (32 longword targets)");
    for (unsigned i = 0; i < 32; ++i) {
        const auto target = bus.peek(0x52e + i * 4, 4, BusSpace::Sub);
        std::ostringstream ss;
        ss << "JUMPTABLE cmd=" << std::dec << i
           << " slot=" << std::hex << std::setfill('0') << std::setw(6) << (0x52e + i * 4)
           << " target=" << std::setw(6) << target;
        bus.sub_debug_note(ss.str());
    }
}

void Runtime::log_road_state_sample(std::uint32_t pc) {
    if (!road_state_debug_ || road_state_samples_ >= road_state_sample_limit_) return;
    const auto disasm = disassemble_space(BusSpace::Sub, pc, nullptr);
    std::ostringstream ss;
    ss << "STATE n=" << std::dec << road_state_samples_ << " pc=" << std::hex << std::setfill('0') << std::setw(6) << pc
       << " sr=" << std::setw(4) << m68k_get_reg(nullptr, M68K_REG_SR)
       << " flag=" << std::setw(4) << bus.peek(0x101a5a, 2, BusSpace::Sub);
    for (int i=0;i<8;++i) ss << " d" << i << '=' << std::setw(8) << m68k_get_reg(nullptr, static_cast<m68k_register_t>(M68K_REG_D0+i));
    for (int i=0;i<8;++i) ss << " a" << i << '=' << std::setw(8) << m68k_get_reg(nullptr, static_cast<m68k_register_t>(M68K_REG_A0+i));
    ss << " | " << disasm;
    bus.road_state_note(ss.str());
    ++road_state_samples_;
}

void Runtime::instruction_hook(unsigned int pc) {
    bus.set_trace_pc(active_space, pc);
    if (quiet_fast_forward_) {
        if (!bus.executable(pc, active_space)) {
            fault = std::string(active_space == BusSpace::Main ? "CPU A" : "CPU B") + " PC outside executable ROM/work RAM";
            m68k_end_timeslice();
        }
        return;
    }
    if (forensic_history_limit_) {
        forensic_history_.push_back({active_space, pc});
        while (forensic_history_.size() > forensic_history_limit_) forensic_history_.pop_front();
    }
    provenance_instruction(pc);
    generic_trace_instruction(pc);
    if (active_space == BusSpace::Sub && road_state_debug_) {
        const bool state_branch = pc >= 0x4b0 && pc <= 0x4ca && (pc & 1) == 0;
        const bool cmd2_handler = pc >= 0x6c2 && pc <= 0x7ff && (pc & 1) == 0;
        if (state_branch || cmd2_handler) {
            ++road_state_hot_[pc];
            if (cmd2_handler) ++cmd2_entries_;
            log_road_state_sample(pc);
        }
        if (force_road_flag_ && !force_road_flag_applied_ && pc == 0x4b0) {
            const auto before = bus.peek(0x101a5a, 2, BusSpace::Sub);
            std::ostringstream ss;
            ss << "FORCE_FLAG pc=0004b0 before=" << std::hex << std::setfill('0') << std::setw(4) << before << " after=0001";
            bus.road_state_note(ss.str());
            bus.write(0x101a5a, 1, 2, BusSpace::Sub);
            force_road_flag_applied_ = true;
        }
    }
    // RC2.9: one-hit special-target research cheat. This runs before the authentic
    // SUBQ.W at PC 0xA112. Do not freeze the counter: arm it to zero once at the real
    // damage instruction so Musashi performs 0000->FFFF and sets authentic flags.
    if (active_space == BusSpace::Main && pc == 0x00a112 && target_one_hit_) {
        const auto remaining = bus.peek(0x1002ae, 2, BusSpace::Main);
        if (remaining != 0xffffu && remaining != 0u) {
            bus.debug_write(0x1002ae, 0u, 2, BusSpace::Main);
            ++target_one_hit_arms_;
        }
    }

    // v0.59.3 handling/cornering telemetry + controlled coefficient override.
    // Hooks run BEFORE the current instruction.  The lifetime points were proven
    // from v0.59.2 traces: D4 is forward coefficient at $008AAA and the forward
    // product at $008AAC; D3 is lateral coefficient at $008AB2 and product at $008AB4.
    if (active_space == BusSpace::Main) {
        if (pc == 0x008a76) {
            handling_snapshot_.turn_state = static_cast<std::int16_t>(m68k_get_reg(nullptr, M68K_REG_D3) & 0xffffu);
        } else if (pc == 0x008aaa) {
            handling_snapshot_.table_index = static_cast<std::int16_t>(m68k_get_reg(nullptr, M68K_REG_D1) & 0xffffu);
            handling_snapshot_.speed_internal = static_cast<std::uint16_t>(m68k_get_reg(nullptr, M68K_REG_D2) & 0xffffu);
            const auto raw = static_cast<std::int16_t>(m68k_get_reg(nullptr, M68K_REG_D4) & 0xffffu);
            handling_snapshot_.forward_coeff = static_cast<std::uint16_t>(raw);
            auto applied = static_cast<int>(std::lround(double(raw) * handling_speed_retain_));
            applied = std::clamp(applied, -32768, 32767);
            handling_snapshot_.applied_forward_coeff = static_cast<std::uint16_t>(static_cast<std::int16_t>(applied));
            handling_snapshot_.override_active = std::abs(handling_cornering_scale_-1.0)>1e-12 || std::abs(handling_speed_retain_-1.0)>1e-12;
            if (handling_snapshot_.override_active) {
                const auto reg = m68k_get_reg(nullptr, M68K_REG_D4);
                m68k_set_reg(M68K_REG_D4, (reg & 0xffff0000u) | handling_snapshot_.applied_forward_coeff);
            }
            handling_snapshot_.valid = true; ++handling_snapshot_.samples;
        } else if (pc == 0x008aac && handling_snapshot_.valid) {
            handling_snapshot_.forward_component = static_cast<std::int32_t>(m68k_get_reg(nullptr, M68K_REG_D4)) >> 8;
        } else if (pc == 0x008ab2 && handling_snapshot_.valid) {
            const auto raw = static_cast<std::int16_t>(m68k_get_reg(nullptr, M68K_REG_D3) & 0xffffu);
            handling_snapshot_.lateral_coeff = static_cast<std::uint16_t>(raw);
            auto applied = static_cast<int>(std::lround(double(raw) * handling_cornering_scale_));
            applied = std::clamp(applied, -32768, 32767);
            handling_snapshot_.applied_lateral_coeff = static_cast<std::uint16_t>(static_cast<std::int16_t>(applied));
            if (handling_snapshot_.override_active) {
                const auto reg = m68k_get_reg(nullptr, M68K_REG_D3);
                m68k_set_reg(M68K_REG_D3, (reg & 0xffff0000u) | handling_snapshot_.applied_lateral_coeff);
            }
        } else if (pc == 0x008ab4 && handling_snapshot_.valid) {
            handling_snapshot_.lateral_component = static_cast<std::int32_t>(m68k_get_reg(nullptr, M68K_REG_D3)) >> 8;
        }
    }

    if (active_space == BusSpace::Main) ++instructions_a;
    else ++instructions_b;

    if (active_space == BusSpace::Main && pc >= 0x580 && pc <= 0x5c0 && (pc & 1) == 0) {
        ++loop_entries_;
        const std::size_t slot = static_cast<std::size_t>((pc - 0x580) / 2);
        if (slot < loop_pc_hits_.size()) ++loop_pc_hits_[slot];
        log_loop_sample(pc);
    }

    if (active_space == BusSpace::Sub && pc >= 0x440 && pc <= 0x520 && (pc & 1) == 0) {
        ++sub_loop_entries_;
        const std::size_t slot = static_cast<std::size_t>((pc - 0x440) / 2);
        if (slot < sub_pc_hits_.size()) ++sub_pc_hits_[slot];
        log_sub_sample(pc);
    }

    if (active_space == BusSpace::Sub && pc >= 0x5b0 && pc <= 0x0a40 && (pc & 1) == 0) {
        ++sub_handler_entries_;
        ++sub_handler_hot_[pc];
        log_sub_handler_sample(pc);
    }

    if (active_space == BusSpace::Sub && pc >= 0x980 && pc <= 0xa10 && (pc & 1) == 0) {
        ++road_focus_entries_;
        ++road_focus_hot_[pc];
        if (pc == 0x9c8) ++pc_9c8_hits_;
        if (pc == 0x9ca) ++pc_9ca_hits_;
        log_road_focus_sample(pc);
    }

    if (active_space == BusSpace::Sub && pc == 0x50c) {
        const auto command = static_cast<std::uint8_t>(bus.peek(0x100806, 2, BusSpace::Sub) & 0x1f);
        const auto target = static_cast<std::uint32_t>(m68k_get_reg(nullptr, M68K_REG_A1)) & 0xffffff;
        bus.note_dispatch(command, target, pc);
    }

    if (!bus.executable(pc, active_space)) {
        fault = std::string(active_space == BusSpace::Main ? "CPU A" : "CPU B") +
            " PC outside executable ROM/work RAM";
        m68k_end_timeslice();
    }
}

void Runtime::interrupt_ack(int level) {
    ++irq_acks_;
    if (sub_debug_ && active_space == BusSpace::Sub) {
        std::ostringstream ss;
        ss << "IRQ_ACK cpu=B level=" << std::dec << level
           << " pc=" << std::hex << std::setfill('0') << std::setw(6)
           << m68k_get_reg(nullptr, M68K_REG_PC)
           << " sr=" << std::setw(4) << m68k_get_reg(nullptr, M68K_REG_SR);
        bus.sub_debug_note(ss.str());
    }
    if (boot_debug_ && active_space == BusSpace::Main) {
        std::ostringstream ss;
        ss << "IRQ_ACK cpu=A level=" << std::dec << level
           << " pc=" << std::hex << std::setfill('0') << std::setw(6)
           << m68k_get_reg(nullptr, M68K_REG_PC)
           << " sr=" << std::setw(4) << m68k_get_reg(nullptr, M68K_REG_SR);
        bus.debug_note(ss.str());
    }
}

void Runtime::initialise_context(BusSpace space, std::vector<std::uint8_t>& context) {
    active_space = space;
    m68k_set_cpu_type(M68K_CPU_TYPE_68000);
    m68k_pulse_reset();
    m68k_get_context(context.data());
}

void Runtime::reset_sub_context() {
    initialise_context(BusSpace::Sub, context_b_);
}

void Runtime::reset() {
    const auto sp_a = bus.peek(0, 4, BusSpace::Main);
    const auto pc_a = bus.peek(4, 4, BusSpace::Main);
    const auto sp_b = bus.peek(0, 4, BusSpace::Sub);
    const auto pc_b = bus.peek(4, 4, BusSpace::Sub);

    if (sp_a & 1 || sp_a < 0x100000 || sp_a > 0x110000 || pc_a < 8 || pc_a >= 0x80000 || pc_a & 1)
        throw std::runtime_error("Invalid CPU A reset vectors");
    if (sp_b & 1 || sp_b < 0x100000 || sp_b > 0x10c000 || pc_b < 8 || pc_b >= 0x20000 || pc_b & 1)
        throw std::runtime_error("Invalid CPU B reset vectors");

    cycles_a = cycles_b = instructions_a = instructions_b = 0;
    irq4_requests_a_ = irq4_requests_b_ = irq_acks_ = loop_entries_ = 0;
    loop_pc_hits_.fill(0);
    sub_pc_hits_.fill(0);
    boot_samples_ = 0;
    sub_samples_ = 0;
    sub_loop_entries_ = 0;
    sub_handler_entries_ = 0;
    sub_handler_samples_ = 0;
    sub_handler_hot_.clear();
    road_focus_samples_ = 0;
    road_focus_entries_ = 0;
    road_focus_hot_.clear();
    pc_9c8_hits_ = pc_9ca_hits_ = 0;
    road_state_samples_ = 0; road_state_hot_.clear(); cmd2_entries_ = 0; force_road_flag_applied_ = false;
    generic_trace_mem_events_ = generic_trace_pc_events_ = generic_trace_triggers_ = 0;
    generic_trace_history_.clear();
    generic_after_remaining_ = 0;
    generic_trigger_fired_.assign(generic_trace_config_.triggers.size(), false);
    previous_debug_pc_ = 0xffffffffu;
    previous_debug_size_ = 0;
    previous_sub_debug_pc_ = 0xffffffffu;
    previous_sub_debug_size_ = 0;
    fault.clear();
    bus.reset_cpu_control();

    initialise_context(BusSpace::Main, context_a_);
    initialise_context(BusSpace::Sub, context_b_);
    sub_was_enabled_ = bus.sub_enabled();
    if (sub_debug_) dump_sub_dispatch_table();

    std::cout << "[RESET A] SP=0x" << std::hex << sp_a << " PC=0x" << pc_a << '\n'
              << "[RESET B] SP=0x" << sp_b << " PC=0x" << pc_b
              << " enabled=" << (sub_was_enabled_ ? "yes" : "no") << std::dec << '\n';
}

void Runtime::sync_sub_reset() {
    const bool enabled = bus.sub_enabled();
    if (enabled && !sub_was_enabled_) {
        reset_sub_context();
        std::cout << "[CPU B] reset released by CPU A control latch\n";
    } else if (!enabled && sub_was_enabled_) {
        std::cout << "[CPU B] reset asserted by CPU A control latch\n";
    }
    sub_was_enabled_ = enabled;
}

int Runtime::execute_slice(BusSpace space, int budget) {
    if (budget <= 0 || !fault.empty()) return 0;
    if (space == BusSpace::Sub && !bus.sub_enabled()) return 0;

    auto& context = space == BusSpace::Main ? context_a_ : context_b_;
    active_space = space;
    m68k_set_context(context.data());
    const auto used = m68k_execute(budget);
    m68k_get_context(context.data());

    if (used > 0) {
        if (space == BusSpace::Main) cycles_a += static_cast<unsigned>(used);
        else cycles_b += static_cast<unsigned>(used);
    }
    return used;
}

int Runtime::run(int cycles_per_cpu) {
    if (cycles_per_cpu <= 0 || !fault.empty()) return 0;

    int total = 0;
    int remaining_a = cycles_per_cpu;
    int remaining_b = cycles_per_cpu;

    for (int slice = 0; slice < scheduler_slices_ && !fault.empty() == false; ++slice) {
        const int slices_left = scheduler_slices_ - slice;
        const int budget_a = std::max(1, remaining_a / slices_left);
        const int used_a = execute_slice(BusSpace::Main, budget_a);
        remaining_a = std::max(0, remaining_a - used_a);
        total += used_a;

        sync_sub_reset();
        if (!fault.empty()) break;

        if (bus.sub_enabled()) {
            const int budget_b = std::max(1, remaining_b / slices_left);
            const int used_b = execute_slice(BusSpace::Sub, budget_b);
            remaining_b = std::max(0, remaining_b - used_b);
            total += used_b;
        }
    }

    return total;
}

int Runtime::step_instruction(BusSpace space) {
    // Musashi executes at least one complete instruction even with a one-cycle
    // budget.  This is intentionally a CPU-local research operation: it does
    // not advance frame timing, IRQ cadence, or the peer CPU.
    return execute_slice(space, 1);
}


static std::optional<m68k_register_t> debug_reg_from_name(std::string name) {
    for(char& c:name) c=static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    if(name.size()==2 && name[0]=='D' && name[1]>='0' && name[1]<='7')
        return static_cast<m68k_register_t>(M68K_REG_D0 + (name[1]-'0'));
    if(name.size()==2 && name[0]=='A' && name[1]>='0' && name[1]<='7')
        return static_cast<m68k_register_t>(M68K_REG_A0 + (name[1]-'0'));
    if(name=="PC") return M68K_REG_PC;
    if(name=="SR") return M68K_REG_SR;
    if(name=="SP") return M68K_REG_SP;
    if(name=="USP") return M68K_REG_USP;
    if(name=="ISP") return M68K_REG_ISP;
    return std::nullopt;
}

std::uint32_t Runtime::debug_get_register(BusSpace space, const std::string& name) const {
    const auto reg=debug_reg_from_name(name);
    if(!reg) throw std::invalid_argument("unknown 68000 register: "+name);
    const auto& context = space == BusSpace::Main ? context_a_ : context_b_;
    return static_cast<std::uint32_t>(m68k_get_reg(const_cast<std::uint8_t*>(context.data()), *reg));
}

bool Runtime::debug_set_register(BusSpace space, const std::string& name, std::uint32_t value) {
    const auto reg=debug_reg_from_name(name); if(!reg) return false;
    auto& context = space == BusSpace::Main ? context_a_ : context_b_;
    active_space=space;
    m68k_set_context(context.data());
    m68k_set_reg(*reg, value);
    m68k_get_context(context.data());
    bus.set_trace_pc(space, static_cast<std::uint32_t>(m68k_get_reg(context.data(), M68K_REG_PC)));
    return true;
}

void Runtime::set_irq(BusSpace space, unsigned level) {
    if (space == BusSpace::Main) ++irq4_requests_a_;
    else ++irq4_requests_b_;
    if (space == BusSpace::Sub && !bus.sub_enabled()) return;
    auto& context = space == BusSpace::Main ? context_a_ : context_b_;
    active_space = space;
    m68k_set_context(context.data());
    m68k_set_irq(level);
    m68k_get_context(context.data());
}

void Runtime::irq4() {
    set_irq(BusSpace::Main, 4);
    set_irq(BusSpace::Sub, 4);
}

void Runtime::summary(std::ostream& out) const {
    out << std::dec;
    out << "CPU A: cycles=" << cycles_a
        << " instructions=" << instructions_a
        << " PC=0x" << std::hex << m68k_get_reg(const_cast<std::uint8_t*>(context_a_.data()), M68K_REG_PC)
        << " SR=0x" << m68k_get_reg(const_cast<std::uint8_t*>(context_a_.data()), M68K_REG_SR)
        << " SP=0x" << m68k_get_reg(const_cast<std::uint8_t*>(context_a_.data()), M68K_REG_SP) << std::dec << '\n';

    out << "CPU B: cycles=" << cycles_b
        << " instructions=" << instructions_b
        << " enabled=" << (bus.sub_enabled() ? "yes" : "no")
        << " PC=0x" << std::hex << m68k_get_reg(const_cast<std::uint8_t*>(context_b_.data()), M68K_REG_PC)
        << " SR=0x" << m68k_get_reg(const_cast<std::uint8_t*>(context_b_.data()), M68K_REG_SR)
        << " SP=0x" << m68k_get_reg(const_cast<std::uint8_t*>(context_b_.data()), M68K_REG_SP) << std::dec << '\n';

    out << "Status: " << (fault.empty()
        ? "budget reached / diagnostic run ended; dual CPU execution is not proof of full game boot"
        : fault) << '\n';
    out << "Boot-loop diagnostics: range=0x580..0x5c0 hits=" << loop_entries_
        << " samples_logged=" << boot_samples_ << "/" << boot_sample_limit_
        << " irq4_requests_A=" << irq4_requests_a_
        << " irq4_requests_B=" << irq4_requests_b_
        << " irq_acks=" << irq_acks_ << '\n';
    out << "  hot PCs:";
    bool any_hot = false;
    for (std::size_t i = 0; i < loop_pc_hits_.size(); ++i) {
        if (!loop_pc_hits_[i]) continue;
        any_hot = true;
        out << " [0x" << std::hex << (0x580 + i * 2) << std::dec << ':' << loop_pc_hits_[i] << ']';
    }
    if (!any_hot) out << " none";
    out << '\n';
    out << "CPU B diagnostics: range=0x440..0x520 hits=" << sub_loop_entries_
        << " samples_logged=" << sub_samples_ << "/" << sub_sample_limit_ << '\n';
    out << "  hot PCs:";
    bool any_sub_hot = false;
    for (std::size_t i = 0; i < sub_pc_hits_.size(); ++i) {
        if (!sub_pc_hits_[i]) continue;
        any_sub_hot = true;
        out << " [0x" << std::hex << (0x440 + i * 2) << std::dec << ':' << sub_pc_hits_[i] << ']';
    }
    if (!any_sub_hot) out << " none";
    out << '\n';
    out << "CPU B handler diagnostics: range=0x5b0..0xa40 hits=" << sub_handler_entries_
        << " samples_logged=" << sub_handler_samples_ << "/" << sub_sample_limit_ << '\n';
    out << "  handler hot PCs:";
    std::vector<std::pair<std::uint32_t, std::uint64_t>> handler_hot(sub_handler_hot_.begin(), sub_handler_hot_.end());
    std::sort(handler_hot.begin(), handler_hot.end(), [](const auto& a, const auto& b){ return a.second > b.second; });
    for (std::size_t i = 0; i < std::min<std::size_t>(12, handler_hot.size()); ++i)
        out << " [0x" << std::hex << handler_hot[i].first << std::dec << ':' << handler_hot[i].second << ']';
    if (handler_hot.empty()) out << " none";
    out << '\n';
    out << "CPU B road-write focus: range=0x980..0xa10 hits=" << road_focus_entries_
        << " samples_logged=" << road_focus_samples_ << "/" << road_focus_sample_limit_
        << " pc9c8=" << pc_9c8_hits_ << " pc9ca=" << pc_9ca_hits_ << '\n';
    out << "  focus hot PCs:";
    std::vector<std::pair<std::uint32_t, std::uint64_t>> rf_hot(road_focus_hot_.begin(), road_focus_hot_.end());
    std::sort(rf_hot.begin(), rf_hot.end(), [](const auto& a, const auto& b){ return a.second > b.second; });
    for (std::size_t i = 0; i < std::min<std::size_t>(12, rf_hot.size()); ++i)
        out << " [0x" << std::hex << rf_hot[i].first << std::dec << ':' << rf_hot[i].second << ']';
    if (rf_hot.empty()) out << " none";
    out << '\n';
    out << "CPU B road-state debugger: samples_logged=" << road_state_samples_ << "/" << road_state_sample_limit_
        << " cmd2_handler_entries=" << cmd2_entries_ << " force_flag=" << (force_road_flag_ ? "on" : "off")
        << " applied=" << (force_road_flag_applied_ ? "yes" : "no") << '\n';
    out << "  state hot PCs:";
    std::vector<std::pair<std::uint32_t, std::uint64_t>> state_hot(road_state_hot_.begin(), road_state_hot_.end());
    std::sort(state_hot.begin(), state_hot.end(), [](const auto& a, const auto& b){ return a.second > b.second; });
    for (std::size_t i=0;i<std::min<std::size_t>(16,state_hot.size());++i) out << " [0x" << std::hex << state_hot[i].first << std::dec << ':' << state_hot[i].second << ']';
    if (state_hot.empty()) out << " none";
    out << '\n';
    if (generic_trace_config_.enabled()) {
        out << "Generic trace: mem_events=" << generic_trace_mem_events_
            << " pc_events=" << generic_trace_pc_events_
            << " triggers=" << generic_trace_triggers_
            << " lines=" << generic_trace_lines_ << "/" << generic_trace_config_.max_lines
            << " before=" << generic_trace_config_.before
            << " after=" << generic_trace_config_.after
            << " cpu=" << (generic_trace_config_.cpu_mask == 1 ? "A" : generic_trace_config_.cpu_mask == 2 ? "B" : "both")
            << '\n';
    }
    bus.summary(out);
}


static std::string trim_copy(std::string text) {
    const auto first = text.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

static std::uint32_t parse_trace_hex(const std::string& text) {
    auto t = trim_copy(text);
    if (t.starts_with("0x") || t.starts_with("0X"))
        t = t.substr(2);
    if (t.empty()) throw std::runtime_error("Empty trace address");
    std::size_t end = 0;
    const auto v = std::stoul(t, &end, 16);
    if (end != t.size() || v > 0xffffff)
        throw std::runtime_error("Invalid 24-bit trace address: " + text);
    return static_cast<std::uint32_t>(v);
}

static std::pair<std::uint32_t, std::uint32_t> parse_trace_range(const std::string& text) {
    auto t = trim_copy(text);
    std::size_t split = t.find(':');
    if (split == std::string::npos)
        split = t.find('-');
    if (split == std::string::npos) {
        const auto a = parse_trace_hex(t);
        return {a, a};
    }
    auto a = parse_trace_hex(t.substr(0, split));
    auto b = parse_trace_hex(t.substr(split + 1));
    if (b < a) std::swap(a, b);
    return {a, b};
}

static void add_trace_mem(TraceConfig& c, const std::string& range, bool reads, bool writes, bool nonzero = false, bool change_only = false) {
    const auto [a, b] = parse_trace_range(range);
    c.mem_ranges.push_back({a, b, reads, writes, nonzero, change_only});
}

static void add_trace_pc(TraceConfig& c, const std::string& range) {
    const auto [a, b] = parse_trace_range(range);
    c.pc_ranges.push_back({a, b});
}

static void add_trace_mem_pc(TraceConfig& c, const std::string& range) {
    const auto [a, b] = parse_trace_range(range);
    c.mem_pc_ranges.push_back({a, b});
}

static void add_trace_trigger(TraceConfig& c, const std::string& range, TraceTriggerKind kind) {
    const auto [a, b] = parse_trace_range(range);
    c.triggers.push_back({a, b, kind});
}

static void add_trace_arm_pc(TraceConfig& c, const std::string& range) {
    const auto [a, b] = parse_trace_range(range);
    c.arm_pc_ranges.push_back({a, b});
}

static void add_trace_arm_mem(TraceConfig& c, const std::string& spec) {
    const auto split = spec.find(':');
    if (split == std::string::npos || spec.find(':', split + 1) != std::string::npos)
        throw std::runtime_error("trace arm mem must be ADDRESS:VALUE");
    c.arm_mem_writes.push_back({parse_trace_hex(spec.substr(0, split)), parse_trace_hex(spec.substr(split + 1))});
}

static void add_trace_task(TraceConfig& c, const std::string& slot_text) {
    const auto slot = parse_trace_hex(slot_text);
    if (slot < 0x100000 || slot > 0x1000f0 || ((slot - 0x100000) & 0x0f) != 0)
        throw std::runtime_error("trace task slot must be 100000..1000f0 on a 0x10 boundary");
    c.task_slots.push_back(slot);
}

static void apply_trace_preset(TraceConfig& c, const std::string& name) {
    if (name == "road") {
        add_trace_mem(c, "100802-100807", true, true);
        add_trace_mem(c, "101600-102200", true, true);
        add_trace_mem(c, "101a58-101a5b", true, true);
        add_trace_mem(c, "800000-801fff", false, true, true);
        add_trace_pc(c, "0480-04d6");
        add_trace_pc(c, "04e0-0540");
        add_trace_trigger(c, "800000-801fff", TraceTriggerKind::NonZeroWrite);
    } else if (name == "handshake") {
        add_trace_mem(c, "108000-1080ff", true, true);
        add_trace_pc(c, "0440-04d6");
        add_trace_pc(c, "04e0-0540");
    } else if (name == "sprite") {
        add_trace_mem(c, "d00000-d007ff", false, true);
        add_trace_trigger(c, "d00000-d007ff", TraceTriggerKind::Change);
    } else if (name == "command") {
        add_trace_mem(c, "100802-100807", true, true);
        add_trace_mem(c, "108018-10801b", true, true);
        add_trace_pc(c, "0474-04d6");
        add_trace_pc(c, "04e0-0540");
        add_trace_trigger(c, "100802-100807", TraceTriggerKind::Change);
    } else {
        throw std::runtime_error("Unknown trace preset: " + name + " (expected road, handshake, command, or sprite)");
    }
}

static void set_trace_cpu(TraceConfig& c, const std::string& cpu) {
    if (cpu == "A" || cpu == "a") c.cpu_mask = 1;
    else if (cpu == "B" || cpu == "b") c.cpu_mask = 2;
    else if (cpu == "both" || cpu == "BOTH" || cpu == "Both") c.cpu_mask = 3;
    else throw std::runtime_error("Trace CPU must be A, B, or both");
}

static unsigned parse_trace_count(const std::string& text, const char* label, unsigned max_value) {
    std::size_t end = 0;
    const auto v = std::stoul(text, &end);
    if (end != text.size() || v > max_value)
        throw std::runtime_error(std::string(label) + " must be 0.." + std::to_string(max_value));
    return static_cast<unsigned>(v);
}

static void load_trace_config_file(TraceConfig& c, const std::filesystem::path& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("Cannot open trace config: " + path.string());
    std::string line;
    unsigned line_no = 0;
    while (std::getline(in, line)) {
        ++line_no;
        const auto comment = line.find('#');
        if (comment != std::string::npos) line.resize(comment);
        line = trim_copy(line);
        if (line.empty()) continue;

        const auto eq = line.find('=');
        if (eq == std::string::npos)
            throw std::runtime_error("Trace config line " + std::to_string(line_no) + ": expected key=value");
        const auto key = trim_copy(line.substr(0, eq));
        const auto val = trim_copy(line.substr(eq + 1));

        try {
            if (key == "cpu") set_trace_cpu(c, val);
            else if (key == "pc") add_trace_pc(c, val);
            else if (key == "mem_pc" || key == "mem-pc") add_trace_mem_pc(c, val);
            else if (key == "arm_pc" || key == "arm-pc") add_trace_arm_pc(c, val);
            else if (key == "arm_mem" || key == "arm-mem") add_trace_arm_mem(c, val);
            else if (key == "task") add_trace_task(c, val);
            else if (key == "tasks") { if (val == "all") c.trace_all_tasks = true; else add_trace_task(c, val); }
            else if (key == "from_frame" || key == "from-frame") c.from_frame = parse_trace_count(val, "trace from frame", 36000);
            else if (key == "to_frame" || key == "to-frame") c.to_frame = parse_trace_count(val, "trace to frame", 36000);
            else if (key == "write_value" || key == "write-value") c.write_value_filter = parse_trace_hex(val);
            else if (key == "preset") apply_trace_preset(c, val);
            else if (key == "before") c.before = parse_trace_count(val, "trace before", 10000);
            else if (key == "after") c.after = parse_trace_count(val, "trace after", 10000);
            else if (key == "max") c.max_lines = parse_trace_count(val, "trace max", 2000000);
            else if (key == "access") {
                if (val == "r") { c.allow_reads = true; c.allow_writes = false; }
                else if (val == "w") { c.allow_reads = false; c.allow_writes = true; }
                else if (val == "rw" || val == "wr") { c.allow_reads = c.allow_writes = true; }
                else throw std::runtime_error("access must be r, w, or rw");
            } else if (key == "mem") {
                auto comma = val.find(',');
                const auto range = trim_copy(val.substr(0, comma));
                const auto mode = comma == std::string::npos ? std::string("rw") : trim_copy(val.substr(comma + 1));
                if (mode == "r") add_trace_mem(c, range, true, false);
                else if (mode == "w") add_trace_mem(c, range, false, true);
                else if (mode == "rw" || mode == "wr") add_trace_mem(c, range, true, true);
                else if (mode == "nonzero" || mode == "nonzero-w") add_trace_mem(c, range, false, true, true);
                else if (mode == "change" || mode == "change-w") add_trace_mem(c, range, false, true, false, true);
                else throw std::runtime_error("mem mode must be r, w, rw, nonzero, or change");
            } else if (key == "trigger") {
                const auto comma = val.find(',');
                if (comma == std::string::npos) throw std::runtime_error("trigger needs kind,range");
                const auto kind = trim_copy(val.substr(0, comma));
                const auto range = trim_copy(val.substr(comma + 1));
                if (kind == "write") add_trace_trigger(c, range, TraceTriggerKind::Write);
                else if (kind == "nonzero" || kind == "nonzero-write") add_trace_trigger(c, range, TraceTriggerKind::NonZeroWrite);
                else if (kind == "change") add_trace_trigger(c, range, TraceTriggerKind::Change);
                else throw std::runtime_error("trigger kind must be write, nonzero, or change");
            } else {
                throw std::runtime_error("unknown key " + key);
            }
        } catch (const std::exception& e) {
            throw std::runtime_error("Trace config line " + std::to_string(line_no) + ": " + e.what());
        }
    }
}


static unsigned parse_dec_limit(const std::string& text, const char* label, unsigned maxv) {
    std::size_t end=0; auto v=std::stoul(trim_copy(text),&end,0);
    if(end!=trim_copy(text).size() || v>maxv) throw std::runtime_error(std::string(label)+" must be 0.."+std::to_string(maxv));
    return (unsigned)v;
}

static std::pair<unsigned,unsigned> parse_dec_pair(const std::string& text, const char* label, unsigned maxv) {
    auto t=trim_copy(text); auto c=t.find(':'); if(c==std::string::npos) throw std::runtime_error(std::string(label)+" must be A:B");
    return {parse_dec_limit(t.substr(0,c),label,maxv),parse_dec_limit(t.substr(c+1),label,maxv)};
}

static std::pair<int,int> parse_xy(const std::string& text) {
    auto t=trim_copy(text); auto c=t.find(':'); if(c==std::string::npos)c=t.find(','); if(c==std::string::npos)throw std::runtime_error("pixel must be X:Y");
    int x=std::stoi(trim_copy(t.substr(0,c))), y=std::stoi(trim_copy(t.substr(c+1)));
    if(x<0||x>=320||y<0||y>=240) throw std::runtime_error("pixel must be within 0..319 x 0..239");
    return {x,y};
}

// v0.45: parse multiple screen coordinates in one argument.  Accepted forms include
//   99:165,150:224,122:239,197:239
//   [(99,165),(150,224),(122,239),(197,239)]
// The parser intentionally extracts integer pairs so batch/config generators do not
// have to care about punctuation style.
static std::vector<std::pair<int,int>> parse_xy_list(const std::string& text) {
    std::vector<int> values;
    for (std::size_t i=0; i<text.size();) {
        if (std::isdigit(static_cast<unsigned char>(text[i]))) {
            std::size_t j=i;
            while (j<text.size() && std::isdigit(static_cast<unsigned char>(text[j]))) ++j;
            values.push_back(std::stoi(text.substr(i,j-i)));
            i=j;
        } else {
            ++i;
        }
    }
    if (values.empty() || (values.size() & 1u))
        throw std::runtime_error("pixel list must contain X:Y coordinate pairs");
    std::vector<std::pair<int,int>> out;
    out.reserve(values.size()/2);
    for (std::size_t i=0; i<values.size(); i+=2) {
        const int x=values[i], y=values[i+1];
        if (x<0||x>=320||y<0||y>=240)
            throw std::runtime_error("pixel list coordinates must be within 0..319 x 0..239");
        out.emplace_back(x,y);
    }
    return out;
}

static void append_xy_list(GraphicsTraceConfig& g, const std::string& text, bool provenance) {
    const auto pts=parse_xy_list(text);
    g.explicit_location=true;
    if (provenance) g.pixel_provenance=true;
    g.pixels.insert(g.pixels.end(), pts.begin(), pts.end());
}

static void apply_graphics_regression_set(GraphicsTraceConfig& g, const std::string& name) {
    if (name=="known-bad" || name=="known-bad-v045") {
        g.explicit_location=true;
        g.pixel_provenance=true;
        // Known discriminating pixels established during v0.44 investigation:
        // continue-screen text/sprite conflict, coloured car overlap, and the two
        // slot-44-only edge pixels that separate ordering from zero/effect semantics.
        const std::pair<int,int> pts[]={{99,165},{150,224},{122,239},{197,239}};
        g.pixels.insert(g.pixels.end(), std::begin(pts), std::end(pts));
    } else {
        throw std::runtime_error("unknown graphics regression set: "+name+" (expected known-bad)");
    }
}

static void apply_layer_offset(Options& o, const std::string& text) {
    std::stringstream ss(text); std::string layer,xs,ys,extra;
    if(!std::getline(ss,layer,':') || !std::getline(ss,xs,':') || !std::getline(ss,ys,':') || std::getline(ss,extra,':'))
        throw std::runtime_error("layer offset must be LAYER:X:Y");
    layer=trim_copy(layer); xs=trim_copy(xs); ys=trim_copy(ys);
    int x=0,y=0;
    try { x=std::stoi(xs,nullptr,0); y=std::stoi(ys,nullptr,0); } catch(...) { throw std::runtime_error("layer offset X/Y must be signed integers"); }
    auto set=[&](Options::LayerOffset& d){d.x=x;d.y=y;};
    if(layer=="bg0"||layer=="bottom") set(o.layer_offset_bg0);
    else if(layer=="bg1"||layer=="upper") set(o.layer_offset_bg1);
    else if(layer=="text") set(o.layer_offset_text);
    else if(layer=="sprite"||layer=="sprites") set(o.layer_offset_sprites);
    else if(layer=="road") set(o.layer_offset_road);
    else if(layer=="all") {set(o.layer_offset_bg0);set(o.layer_offset_bg1);set(o.layer_offset_text);set(o.layer_offset_sprites);set(o.layer_offset_road);}
    else throw std::runtime_error("unknown layer offset target: "+layer+" (expected bg0,bg1,text,sprites,road,all)");
}

static GraphicsTraceRegion parse_region(const std::string& text) {
    std::vector<int> v; std::string cur; for(char ch:text){if(ch==','||ch==':'){if(cur.empty())throw std::runtime_error("region must be X1:Y1:X2:Y2");v.push_back(std::stoi(trim_copy(cur)));cur.clear();}else cur+=ch;} if(!cur.empty())v.push_back(std::stoi(trim_copy(cur)));
    if(v.size()!=4) throw std::runtime_error("region must be X1:Y1:X2:Y2");
    GraphicsTraceRegion r{v[0],v[1],v[2],v[3]}; if(r.x2<r.x1)std::swap(r.x1,r.x2);if(r.y2<r.y1)std::swap(r.y1,r.y2);
    r.x1=std::clamp(r.x1,0,319);r.x2=std::clamp(r.x2,0,319);r.y1=std::clamp(r.y1,0,239);r.y2=std::clamp(r.y2,0,239);return r;
}

static void apply_graphics_trace_preset(GraphicsTraceConfig& g, const std::string& name) {
    if(name=="car") { g.sprite_pairs.push_back({44,45}); g.sprite_slots.push_back(44); g.sprite_slots.push_back(45); g.palette_banks.push_back(64); g.palette_banks.push_back(70); g.palette_entries.push_back(1124); g.regions.push_back({64,150,255,239}); g.zero_palette_use=true; g.pixel_provenance=true; g.pixel_provenance_pair=std::make_pair(44u,45u); }
    else if(name=="palette") { g.palette_banks.push_back(64); g.palette_banks.push_back(70); g.zero_palette_use=true; }
    else if(name=="priority") { g.layers={"road","bg0","bg1","sprite","text"}; g.anomalies_only=true; }
    else if(name=="prom") { g.prom_addr_ranges.push_back({0,255}); }
    else if(name=="road-priority") { g.layers.push_back("road"); g.anomalies_only=true; }
    else if(name=="compositor") { g.layers={"road","bg0","bg1","sprite","text"}; g.regions.push_back({0,120,319,239}); }
    else throw std::runtime_error("Unknown graphics trace preset: "+name+" (expected car, palette, priority, prom, road-priority, compositor)");
}

static void load_graphics_trace_config_file(GraphicsTraceConfig& g, const std::filesystem::path& path) {
    std::ifstream in(path); if(!in) throw std::runtime_error("Cannot open graphics trace config: "+path.string());
    std::string line; unsigned line_no=0;
    while(std::getline(in,line)) { ++line_no; auto c=line.find('#'); if(c!=std::string::npos)line.resize(c); line=trim_copy(line); if(line.empty())continue; auto eq=line.find('='); if(eq==std::string::npos)throw std::runtime_error("Graphics trace config line "+std::to_string(line_no)+": expected key=value"); auto key=trim_copy(line.substr(0,eq)), val=trim_copy(line.substr(eq+1));
        try {
            if(key=="slot") g.sprite_slots.push_back(parse_dec_limit(val,"sprite slot",255));
            else if(key=="pair") g.sprite_pairs.push_back(parse_dec_pair(val,"sprite pair",255));
            else if(key=="palette_bank"||key=="palette-bank") g.palette_banks.push_back(parse_dec_limit(val,"palette bank",255));
            else if(key=="palette_entry"||key=="palette-entry") g.palette_entries.push_back(parse_dec_limit(val,"palette entry",4095));
            else if(key=="pen") g.pens.push_back(parse_dec_limit(val,"pen",15));
            else if(key=="prom_addr"||key=="prom-addr") {auto [a,b]=parse_trace_range(val);if(b>255)throw std::runtime_error("PROM address must be 00..ff");g.prom_addr_ranges.push_back({a,b});}
            else if(key=="prom_value"||key=="prom-value") {auto v=parse_trace_hex(val);if(v>255)throw std::runtime_error("PROM value must be 00..ff");g.prom_value=v;}
            else if(key=="priority") g.priority_classes.push_back(parse_dec_limit(val,"priority class",255));
            else if(key=="pixel") { g.explicit_location=true; g.pixels.push_back(parse_xy(val)); }
            else if(key=="pixels") append_xy_list(g,val,false);
            else if(key=="regression_set"||key=="regression-set") apply_graphics_regression_set(g,val);
            else if(key=="region") { g.explicit_location=true; g.regions.push_back(parse_region(val)); }
            else if(key=="layer") g.layers.push_back(val);
            else if(key=="road_priority"||key=="road-priority") g.road_priority=parse_dec_limit(val,"road priority",255);
            else if(key=="zero_palette"||key=="zero-palette") g.zero_palette_use=(val!="0"&&val!="false"&&val!="off");
            else if(key=="anomalies") g.anomalies_only=(val!="0"&&val!="false"&&val!="off");
            else if(key=="pixel_provenance"||key=="pixel-provenance") { g.pixel_provenance=true; g.explicit_location=true; g.pixels.push_back(parse_xy(val)); }
            else if(key=="pixel_provenance_pixels"||key=="pixel-provenance-pixels") append_xy_list(g,val,true);
            else if(key=="pixel_provenance_pair"||key=="pixel-provenance-pair") { g.pixel_provenance=true; g.pixel_provenance_pair=parse_dec_pair(val,"pixel provenance pair",255); }
            else if(key=="pixel_provenance_max"||key=="pixel-provenance-max") g.pixel_provenance_max=parse_trace_count(val,"pixel provenance max",512);
            else if(key=="experiment_zero_slot"||key=="experiment-zero-slot") g.experiment_zero_slot=parse_dec_limit(val,"experiment zero slot",255);
            else if(key=="experiment_zero_palette_bank"||key=="experiment-zero-palette-bank") g.experiment_zero_palette_bank=parse_dec_limit(val,"experiment zero palette bank",255);
            else if(key=="experiment_sprite_pair_order"||key=="experiment-sprite-pair-order") g.experiment_sprite_pair_order=parse_dec_pair(val,"experiment sprite pair order",255);
            else if(key=="experiment_sprite_order"||key=="experiment-sprite-order") { if(val!="ascending"&&val!="descending") throw std::runtime_error("sprite order must be ascending or descending"); g.experiment_sprite_order=val; }
            else if(key=="experiment_layer_order"||key=="experiment-layer-order") { g.experiment_layer_order.clear(); std::stringstream ss(val); std::string x; while(std::getline(ss,x,',')){x=trim_copy(x); if(!x.empty())g.experiment_layer_order.push_back(x);} }
            else if(key=="experiment_layer_matrix"||key=="experiment-layer-matrix") g.experiment_layer_matrix=(val!="0"&&val!="false"&&val!="off");
            else if(key=="from_frame"||key=="from-frame") g.from_frame=parse_trace_count(val,"graphics trace from frame",36000);
            else if(key=="to_frame"||key=="to-frame") g.to_frame=parse_trace_count(val,"graphics trace to frame",36000);
            else if(key=="max") g.max_lines=parse_trace_count(val,"graphics trace max",2000000);
            else if(key=="preset") apply_graphics_trace_preset(g,val);
            else throw std::runtime_error("unknown key "+key);
        } catch(const std::exception& e) { throw std::runtime_error("Graphics trace config line "+std::to_string(line_no)+": "+e.what()); }
    }
}


static void apply_stage1_gameplay_scenario(Options& o) {
    const InputPulse seq[] = {
        {0x2,0x10,120,5},{0x3,0x08,500,5},{0x3,0x20,650,20},{0x3,0x20,720,20},
        {0xc,0x60,810,70},{0x3,0x20,820,20},{0x3,0x01,900,20},{0xc,0xa0,960,70},
        {0x3,0x20,970,20},{0x3,0x20,1080,20},{0x3,0x01,1180,20},{0xc,0x60,1270,70},
        {0x3,0x20,1280,20}
    };
    o.input_pulses.insert(o.input_pulses.end(), std::begin(seq), std::end(seq));
    o.frames = std::max(o.frames, 1800u);
}

static void apply_scenario(Options& o, const std::string& name) {
    if(name=="boot") { o.frames=std::max(o.frames,600u); }
    else if(name=="stage1-gameplay" || name=="stage1-driving") apply_stage1_gameplay_scenario(o);
    else if(name=="service-mode") { o.frames=std::max(o.frames,900u); /* placeholder state: no undocumented IOC assumptions */ }
    else if(name=="crash-test") { apply_stage1_gameplay_scenario(o); o.frames=std::max(o.frames,2100u); }
    else throw std::runtime_error("Unknown scenario: "+name+" (expected boot, stage1-gameplay, stage1-driving, service-mode, crash-test)");
    o.scenario=name;
}

static void apply_event_trace(Options& o, const std::string& name) {
    auto add_mem=[&](std::uint32_t a,std::uint32_t b,bool rd=true,bool wr=true){ TraceRange r; r.start=a;r.end=b;r.reads=rd;r.writes=wr;o.trace.mem_ranges.push_back(r); };
    if(name=="ioc") add_mem(0x400000,0x400003,true,true);
    else if(name=="palette") add_mem(0xa00000,0xa00007,true,true);
    else if(name=="sprite") add_mem(0xd00000,0xd007ff,true,true);
    else if(name=="road") add_mem(0x800000,0x801fff,true,true);
    else if(name=="sound" || name=="sound-command") add_mem(0x820000,0x820003,true,true);
    else if(name=="cpu-control") add_mem(0x800000,0x800001,true,true);
    else if(name=="irq") { /* IRQ events are summarized separately; keep category in manifest. */ }
    else throw std::runtime_error("Unknown --trace-event category: "+name);
    o.event_trace.events.push_back(name);
}

static std::uint16_t parse_steering_value(const std::string& text);
static SteeringEvent parse_steering_event(const std::string& text, bool range);
static void load_master_debug_config(Options& o, const std::filesystem::path& path) {
    std::ifstream in(path); if(!in) throw std::runtime_error("Cannot open debug config: "+path.string());
    std::string line; unsigned ln=0;
    while(std::getline(in,line)){ ++ln; auto c=line.find('#'); if(c!=std::string::npos)line.resize(c); line=trim_copy(line); if(line.empty())continue; auto eq=line.find('='); if(eq==std::string::npos)throw std::runtime_error("Debug config line "+std::to_string(ln)+": expected key=value"); auto k=trim_copy(line.substr(0,eq)),v=trim_copy(line.substr(eq+1));
        try {
            if(k=="scenario") apply_scenario(o,v);
            else if(k=="layer_offset"||k=="layer-offset") apply_layer_offset(o,v);
            else if(k=="steering") o.steering_fixed=parse_steering_value(v);
            else if(k=="steering_at"||k=="steering-at") o.steering_events.push_back(parse_steering_event(v,false));
            else if(k=="steering_range"||k=="steering-range") o.steering_events.push_back(parse_steering_event(v,true));
            else if(k=="gameplay_state_log"||k=="gameplay-state-log") o.gameplay_state_log=(v!="0"&&v!="false"&&v!="off");
            else if(k=="target_state_log"||k=="target-state-log") o.target_state_log=(v!="0"&&v!="false"&&v!="off");
            else if(k=="target_one_hit"||k=="target-one-hit"||k=="one_hit_target"||k=="one-hit-target") o.target_one_hit=(v!="0"&&v!="false"&&v!="off");
            else if(k=="no_collisions"||k=="no-collisions") { o.no_collisions=(v!="0"&&v!="false"&&v!="off"); if(o.no_collisions) o.target_state_log=true; }
            else if(k=="course_follow_controller"||k=="course-follow-controller") { o.course_follow_controller=v; }
            else if(k=="course_profile"||k=="course-profile") { o.course_profile_file=v; o.course_follow=true; o.course_follow_controller="profile"; }
            else if(k=="cornering_scale"||k=="cornering-scale") { o.cornering_scale=std::stod(v); }
            else if(k=="cornering_speed_retain"||k=="cornering-speed-retain") { o.cornering_speed_retain=std::stod(v); }
            else if(k=="course_follow_lateral_kd"||k=="course-follow-lateral-kd") { o.course_follow_lateral_kd=std::stod(v); }
            else if(k=="course_follow_slew"||k=="course-follow-slew") { o.course_follow_slew=std::stoi(v); }
            else if(k=="course_follow_speed_control"||k=="course-follow-speed-control") { o.course_follow_speed_control=(v!="0"&&v!="false"&&v!="off"); }
            else if(k=="tc0100scn_trace"||k=="tc0100scn-trace") o.tc0100scn_trace=(v!="0"&&v!="false"&&v!="off");
            else if(k=="sprite_tie_break"||k=="sprite-tie-break") { if(v=="lower-slot") o.sprite_tie_break=Options::SpriteTieBreak::LowerSlot; else if(v=="higher-slot") o.sprite_tie_break=Options::SpriteTieBreak::HigherSlot; else throw std::runtime_error("sprite_tie_break must be lower-slot or higher-slot"); }
            else if(k=="capture_frame"||k=="capture-frame") o.capture_frames.push_back(parse_trace_count(v,"capture frame",36000));
            else if(k=="event") apply_event_trace(o,v);
            else if(k=="graphics_preset"||k=="graphics-preset") { o.graphics_debug_all=true; apply_graphics_trace_preset(o.graphics_trace,v); }
            else if(k=="graphics_frame"||k=="graphics-frame") { o.graphics_debug_all=true; o.graphics_debug_frames.push_back(parse_trace_count(v,"graphics frame",36000)); }
            else if(k=="sprite_frame"||k=="sprite-frame") { o.sprite_debug=true; o.sprite_debug_frames.push_back(parse_trace_count(v,"sprite frame",36000)); }
            else if(k=="input_trace"||k=="input-trace") o.input_trace=(v!="0"&&v!="false"&&v!="off");
            else if(k=="debug_everything"||k=="debug-everything") o.debug_everything=(v!="0"&&v!="false"&&v!="off");
            else if(k=="frames") { o.frames=parse_trace_count(v,"frames",36000); o.frames_explicit=true; }
            else if(k=="follow_address"||k=="follow-address") { auto [a,b]=parse_trace_range(v); o.provenance.follow_ranges.push_back({a,b}); }
            else if(k=="provenance_full_graph"||k=="provenance-full-graph") { bool on=(v!="0"&&v!="false"&&v!="off"); o.provenance.trace_callgraph=on; o.provenance.profile_functions=on; }
            else if(k=="provenance_verbose_calls"||k=="provenance-verbose-calls") o.provenance.verbose_calls=(v!="0"&&v!="false"&&v!="off");
            else if(k=="provenance_from_frame"||k=="provenance-from-frame") o.provenance.from_frame=parse_trace_count(v,"provenance from frame",36000);
            else if(k=="provenance_to_frame"||k=="provenance-to-frame") o.provenance.to_frame=parse_trace_count(v,"provenance to frame",36000);
            else if(k=="provenance_depth"||k=="provenance-depth") o.provenance.max_depth=parse_trace_count(v,"provenance depth",256);
            else if(k=="provenance_max"||k=="provenance-max") o.provenance.max_events=parse_trace_count(v,"provenance max",2000000);
            else if(k=="fast_forward_to"||k=="fast-forward-to") o.fast_forward_to=parse_trace_count(v,"fast-forward frame",36000);
            else if(k=="break_frame"||k=="break-frame") o.break_frames.push_back(parse_trace_count(v,"break frame",36000));
            else if(k=="auto_zip_logs"||k=="auto-zip-logs") o.auto_zip_logs=(v!="0"&&v!="false"&&v!="off");
            else throw std::runtime_error("unknown key "+k);
        } catch(const std::exception& e){ throw std::runtime_error("Debug config line "+std::to_string(ln)+": "+e.what()); }
    }
}

static InputPulse parse_input_pulse(const std::string& text) {
    std::vector<std::string> part;
    std::size_t start = 0;
    while (true) {
        const auto pos = text.find(':', start);
        part.push_back(text.substr(start, pos == std::string::npos ? std::string::npos : pos - start));
        if (pos == std::string::npos) break;
        start = pos + 1;
    }
    if (part.size() < 3 || part.size() > 4)
        throw std::runtime_error("--pulse-ioc expects PORT:MASK:FRAME[:DURATION]");

    auto parse_hex = [](const std::string& v, const char* what, unsigned max) -> unsigned {
        if (v.empty()) throw std::runtime_error(std::string("Empty ") + what + " in --pulse-ioc");
        std::size_t end = 0;
        const auto n = std::stoul(v, &end, 16);
        if (end != v.size() || n > max) throw std::runtime_error(std::string("Invalid ") + what + " in --pulse-ioc");
        return static_cast<unsigned>(n);
    };
    auto parse_dec = [](const std::string& v, const char* what, unsigned max) -> unsigned {
        if (v.empty()) throw std::runtime_error(std::string("Empty ") + what + " in --pulse-ioc");
        std::size_t end = 0;
        const auto n = std::stoul(v, &end, 10);
        if (end != v.size() || n > max) throw std::runtime_error(std::string("Invalid ") + what + " in --pulse-ioc");
        return static_cast<unsigned>(n);
    };

    InputPulse pulse;
    pulse.port = static_cast<std::uint8_t>(parse_hex(part[0], "port", 0x0f));
    pulse.mask = static_cast<std::uint8_t>(parse_hex(part[1], "mask", 0xff));
    pulse.start_frame = parse_dec(part[2], "frame", 36000);
    pulse.duration_frames = part.size() == 4 ? parse_dec(part[3], "duration", 36000) : 3;
    if (pulse.mask == 0) throw std::runtime_error("--pulse-ioc mask must be non-zero");
    if (pulse.duration_frames == 0) throw std::runtime_error("--pulse-ioc duration must be at least 1 frame");
    return pulse;
}

static std::uint16_t parse_steering_value(const std::string& text) {
    std::size_t used=0; unsigned long v=std::stoul(text,&used,0);
    if(used!=text.size() || v>0xffff) throw std::runtime_error("steering value must be 0..65535 / 0xffff");
    return static_cast<std::uint16_t>(v);
}
static SteeringEvent parse_steering_event(const std::string& text, bool range) {
    std::vector<std::string> p; std::stringstream ss(text); std::string x; while(std::getline(ss,x,':')) p.push_back(trim_copy(x));
    if((!range && p.size()!=2) || (range && p.size()!=3)) throw std::runtime_error(range?"--steering-range expects START:END:VALUE":"--steering-at expects FRAME:VALUE");
    SteeringEvent e; e.start_frame=parse_trace_count(p[0],"steering frame",36000); e.end_frame=range?parse_trace_count(p[1],"steering end frame",36000):e.start_frame; e.value=parse_steering_value(p[range?2:1]);
    if(e.end_frame<e.start_frame) throw std::runtime_error("steering range END must be >= START"); return e;
}
static std::uint16_t parse_steering_signed_value(const std::string& text) {
    std::size_t used=0; long v=std::stol(text,&used,0);
    if(used!=text.size() || v < -2048 || v > 2047) throw std::runtime_error("signed steering value must be -2048..2047");
    return static_cast<std::uint16_t>(v) & 0x0fff;
}
static SteeringEvent parse_steering_signed_event(const std::string& text, bool range) {
    std::vector<std::string> p; std::stringstream ss(text); std::string x; while(std::getline(ss,x,':')) p.push_back(trim_copy(x));
    if((!range && p.size()!=2) || (range && p.size()!=3)) throw std::runtime_error(range?"--steering-signed-range expects START:END:VALUE":"--steering-signed-at expects FRAME:VALUE");
    SteeringEvent e; e.start_frame=parse_trace_count(p[0],"steering frame",36000); e.end_frame=range?parse_trace_count(p[1],"steering end frame",36000):e.start_frame; e.value=parse_steering_signed_value(p[range?2:1]);
    if(e.end_frame<e.start_frame) throw std::runtime_error("steering range END must be >= START"); return e;
}

static PeriodicInputPulse parse_periodic_input_pulse(const std::string& text) {
    std::vector<std::string> part; std::size_t start=0;
    while(true){auto pos=text.find(':',start);part.push_back(text.substr(start,pos==std::string::npos?std::string::npos:pos-start));if(pos==std::string::npos)break;start=pos+1;}
    if(part.size()<4||part.size()>6) throw std::runtime_error("--pulse-ioc-every expects PORT:MASK:START:EVERY[:DURATION[:COUNT]]");
    auto hx=[](const std::string&v,const char*w,unsigned m){if(v.empty())throw std::runtime_error(std::string("Empty ")+w+" in --pulse-ioc-every");std::size_t e=0;auto n=std::stoul(v,&e,16);if(e!=v.size()||n>m)throw std::runtime_error(std::string("Invalid ")+w+" in --pulse-ioc-every");return (unsigned)n;};
    auto dc=[](const std::string&v,const char*w,unsigned m){if(v.empty())throw std::runtime_error(std::string("Empty ")+w+" in --pulse-ioc-every");std::size_t e=0;auto n=std::stoul(v,&e,10);if(e!=v.size()||n>m)throw std::runtime_error(std::string("Invalid ")+w+" in --pulse-ioc-every");return (unsigned)n;};
    PeriodicInputPulse q; q.port=(std::uint8_t)hx(part[0],"port",0xf);q.mask=(std::uint8_t)hx(part[1],"mask",0xff);q.start_frame=dc(part[2],"start",36000);q.every_frames=dc(part[3],"interval",36000);q.duration_frames=part.size()>=5?dc(part[4],"duration",36000):3;q.count=part.size()>=6?dc(part[5],"count",100000):0;
    if(!q.mask||!q.every_frames||!q.duration_frames) throw std::runtime_error("--pulse-ioc-every mask/interval/duration must be non-zero"); return q;
}

static BusSpace parse_patch_cpu(const std::string&s){if(s=="A"||s=="a"||s=="main")return BusSpace::Main;if(s=="B"||s=="b"||s=="sub")return BusSpace::Sub;throw std::runtime_error("patch CPU must be A or B");}
static PatchWidth parse_patch_width(const std::string&s){if(s=="8"||s=="byte")return PatchWidth::Byte;if(s=="16"||s=="word")return PatchWidth::Word;if(s=="32"||s=="long")return PatchWidth::Long;throw std::runtime_error("patch width must be byte, word, long, 8, 16, or 32");}
static MemoryPatch parse_memory_patch(const std::string& text, bool conditional) {
    std::vector<std::string> p;std::size_t st=0;while(true){auto q=text.find(':',st);p.push_back(text.substr(st,q==std::string::npos?std::string::npos:q-st));if(q==std::string::npos)break;st=q+1;}
    const std::size_t want=conditional?6:5;if(p.size()!=want)throw std::runtime_error(conditional?"--patch-when expects CPU:WIDTH:ADDR:EXPECTED:VALUE:FRAME":"--patch-at-frame expects FRAME:CPU:WIDTH:ADDR:VALUE");
    auto hx=[](const std::string&v,const char*w){if(v.empty())throw std::runtime_error(std::string("Empty ")+w+" in memory patch");std::size_t e=0;auto n=std::stoul(v,&e,16);if(e!=v.size())throw std::runtime_error(std::string("Invalid ")+w+" in memory patch");return (std::uint32_t)n;};
    auto dc=[](const std::string&v,const char*w){if(v.empty())throw std::runtime_error(std::string("Empty ")+w+" in memory patch");std::size_t e=0;auto n=std::stoul(v,&e,10);if(e!=v.size()||n>36000)throw std::runtime_error(std::string("Invalid ")+w+" in memory patch");return (unsigned)n;};
    MemoryPatch m;m.conditional=conditional;
    if(!conditional){m.frame=dc(p[0],"frame");m.space=parse_patch_cpu(p[1]);m.width=parse_patch_width(p[2]);m.address=hx(p[3],"address");m.value=hx(p[4],"value");}
    else {m.space=parse_patch_cpu(p[0]);m.width=parse_patch_width(p[1]);m.address=hx(p[2],"address");m.expected=hx(p[3],"expected");m.value=hx(p[4],"value");m.frame=dc(p[5],"start frame");}
    return m;
}

static unsigned parse_hex_byte_option(const std::string& text, const char* what) {
    if(text.empty()) throw std::runtime_error(std::string("Empty ")+what);
    std::size_t end=0; auto n=std::stoul(text,&end,16);
    if(end!=text.size() || n>0xff) throw std::runtime_error(std::string("Invalid ")+what+" (expected hex 00..ff)");
    return static_cast<unsigned>(n);
}

static std::vector<unsigned> parse_mask_list(const std::string& text) {
    if(text=="default") return {0xf0,0xfc,0x0f,0x3f,0xcf,0xff};
    std::vector<unsigned> out; std::stringstream ss(text); std::string tok;
    while(std::getline(ss,tok,',')) { tok=trim_copy(tok); if(!tok.empty()) out.push_back(parse_hex_byte_option(tok,"sprite mask matrix value")); }
    if(out.empty()) throw std::runtime_error("--experiment-sprite-mask-matrix expects comma-separated hex masks or 'default'");
    return out;
}

static MemoryWatch parse_memory_watch(const std::string& text) {
    std::vector<std::string> p; std::size_t st=0;
    while(true){auto q=text.find(':',st);p.push_back(text.substr(st,q==std::string::npos?std::string::npos:q-st));if(q==std::string::npos)break;st=q+1;}
    if(p.size()<3||p.size()>4) throw std::runtime_error("--watch-address expects CPU:WIDTH:ADDR[:LABEL]");
    auto hx=[](const std::string&v){if(v.empty())throw std::runtime_error("Empty watch address");std::size_t e=0;auto n=std::stoul(v,&e,16);if(e!=v.size())throw std::runtime_error("Invalid watch address");return (std::uint32_t)n;};
    MemoryWatch w; w.space=parse_patch_cpu(p[0]); w.width=parse_patch_width(p[1]); w.address=hx(p[2]); w.label=p.size()==4?p[3]:"watch"; return w;
}


static std::vector<int> parse_signed_frame_offsets(const std::string& text,const char* option) {
    std::vector<int> out; std::stringstream ss(text); std::string tok;
    while(std::getline(ss,tok,',')){ tok=trim_copy(tok); if(tok.empty())continue; std::size_t e=0; long n=std::stol(tok,&e,10); if(e!=tok.size()||n<-36000||n>36000)throw std::runtime_error(std::string(option)+" expects comma-separated frame offsets"); out.push_back((int)n); }
    if(out.empty())throw std::runtime_error(std::string(option)+" expects comma-separated frame offsets"); return out;
}
static std::vector<unsigned> parse_frame_list(const std::string& text,const char* option) {
    std::vector<unsigned> out;std::stringstream ss(text);std::string tok;while(std::getline(ss,tok,',')){tok=trim_copy(tok);if(tok.empty())continue;out.push_back(parse_trace_count(tok,"capture frame",36000));}if(out.empty())throw std::runtime_error(std::string(option)+" expects comma-separated frames");return out;
}
static TransitionTrigger parse_transition_trigger(const std::string& text,bool on_change) {
    std::vector<std::string> p;std::size_t st=0;while(true){auto q=text.find(':',st);p.push_back(text.substr(st,q==std::string::npos?std::string::npos:q-st));if(q==std::string::npos)break;st=q+1;}
    const std::size_t minparts=on_change?3:4;if(p.size()<minparts||p.size()>minparts+1)throw std::runtime_error(on_change?"--capture-on-change expects CPU:WIDTH:ADDR[:LABEL]":"--capture-on-value expects CPU:WIDTH:ADDR:VALUE[:LABEL]");
    auto hx=[](const std::string&v){if(v.empty())throw std::runtime_error("Empty transition value");std::size_t e=0;auto n=std::stoul(v,&e,16);if(e!=v.size())throw std::runtime_error("Invalid transition hex value");return(std::uint32_t)n;};
    TransitionTrigger t;t.space=parse_patch_cpu(p[0]);t.width=parse_patch_width(p[1]);t.address=hx(p[2]);t.on_change=on_change;if(!on_change)t.value=hx(p[3]);t.label=p.size()>minparts?p.back():(on_change?"change":"value");return t;
}
static SaveCheckpointRequest parse_save_checkpoint(const std::string& text){auto colon=text.find(':');if(colon==std::string::npos||colon==0||colon+1>=text.size())throw std::runtime_error("--save-checkpoint expects FRAME:PATH");SaveCheckpointRequest q;q.frame=parse_trace_count(text.substr(0,colon),"checkpoint frame",36000);q.path=text.substr(colon+1);return q;}
static void apply_diagnostics_profile(Options& o,const std::string& name){
    o.diagnostics_profile=name;
    if(name=="none"||name=="graphics"||name=="memory"||name=="auto"){o.boot_debug=o.sub_debug=o.handshake_debug=o.road_focus_debug=o.road_flow_debug=o.road_state_debug=o.road_pending_debug=false;}
    else if(name=="road"){o.road_focus_debug=o.road_flow_debug=o.road_state_debug=o.road_pending_debug=true;o.boot_debug=o.sub_debug=o.handshake_debug=false;}
    else if(name=="cpu"){o.boot_debug=o.sub_debug=o.handshake_debug=true;o.road_focus_debug=o.road_flow_debug=o.road_state_debug=o.road_pending_debug=false;}
    else if(name=="all"){o.boot_debug=o.sub_debug=o.handshake_debug=o.road_focus_debug=o.road_flow_debug=o.road_state_debug=o.road_pending_debug=true;}
    else throw std::runtime_error("--diagnostics expects auto|none|graphics|memory|road|cpu|all");
}

static void apply_investigate_profile(Options& o, const std::string& name) {
    o.investigate_profile=name;
    if(name=="compositor-known-bad") {
        o.graphics_debug_all=true;
        o.graphics_trace.pixel_provenance=true;
        o.graphics_trace.explicit_location=true;
        for(auto q: {std::pair<int,int>{99,165},{150,224},{122,239},{197,239}}) o.graphics_trace.pixels.push_back(q);
        o.graphics_trace.experiment_sprite_mask_matrix={0xf0,0xfc,0x0f,0x3f,0xcf,0xff};
        o.graphics_trace.experiment_zero_slot=44;
        o.graphics_trace.experiment_zero_palette_bank=70;
        o.graphics_trace.experiment_sprite_pair_order=std::make_pair(44u,45u);
        o.auto_zip_logs=true;
        return;
    }
    if(name=="timeout-watch") {
        o.memory_watches.push_back({BusSpace::Main,PatchWidth::Byte,0x100200,"race_timer"});
        o.auto_zip_logs=true;
        return;
    }
    throw std::runtime_error("Unknown --investigate profile: "+name+" (known: compositor-known-bad, timeout-watch)");
}

static std::string option_value(int& i, int argc, char** argv, const std::string& arg) {
    if (++i >= argc) throw std::runtime_error("Missing value after " + arg);
    return argv[i];
}


static std::vector<int> parse_selector_values(const std::string& text, const char* option) {
    std::vector<int> out; std::stringstream ss(text); std::string part;
    while(std::getline(ss,part,'|')) {
        part=trim_copy(part); if(part.empty()) throw std::runtime_error(std::string(option)+" empty value in set: "+text);
        unsigned long n=0; std::size_t end=0;
        try { n=std::stoul(part,&end,0); } catch(...) { throw std::runtime_error(std::string(option)+" invalid value: "+part); }
        if(end!=part.size()) throw std::runtime_error(std::string(option)+" invalid value: "+part);
        out.push_back((int)n);
    }
    return out;
}

static Options::SpriteSelector parse_sprite_selector(const std::string& text, const char* option) {
    Options::SpriteSelector sel;
    std::stringstream ss(text); std::string tok; bool any=false;
    while (std::getline(ss,tok,',')) {
        tok=trim_copy(tok); if(tok.empty()) continue;
        auto eq=tok.find('='); if(eq==std::string::npos) throw std::runtime_error(std::string(option)+" expects key=value pairs, e.g. map=778|779|780|781,palette=134");
        auto key=trim_copy(tok.substr(0,eq)), val=trim_copy(tok.substr(eq+1));
        auto values=parse_selector_values(val,option);
        if(key=="slot") sel.slots=std::move(values);
        else if(key=="map") sel.maps=std::move(values);
        else if(key=="palette") sel.palettes=std::move(values);
        else if(key=="priority") sel.priorities=std::move(values);
        else throw std::runtime_error(std::string(option)+" unknown selector key: "+key+" (supported: slot,map,palette,priority)");
        any=true;
    }
    if(!any) throw std::runtime_error(std::string(option)+" requires at least one selector");
    return sel;
}


static Options::SpriteSelector semantic_object_selector(const std::string& name, const char* option) {
    Options::SpriteSelector sel;
    if(name=="player_car") {
        sel.maps={472,475,476,508,514,516,572,575,576,590,592,594};
        sel.palettes={64,65,70};
        return sel;
    }
    throw std::runtime_error(std::string(option)+" unknown semantic object: "+name+" (supported: player_car)");
}

static Options::SpriteQuadCandidate parse_sprite_quad_candidate(const std::string& text, const char* option) {
    Options::SpriteQuadCandidate q; std::stringstream ss(text); std::string tok; bool any=false;
    while(std::getline(ss,tok,',')) {
        tok=trim_copy(tok); if(tok.empty()) continue;
        auto eq=tok.find('='); if(eq==std::string::npos) throw std::runtime_error(std::string(option)+" expects key=value pairs");
        auto key=trim_copy(tok.substr(0,eq)), val=trim_copy(tok.substr(eq+1));
        if(key=="name") { q.name=val; any=true; continue; }
        unsigned long n=0; std::size_t end=0; try { n=std::stoul(val,&end,0); } catch(...) { throw std::runtime_error(std::string(option)+" invalid value: "+val); }
        if(end!=val.size()) throw std::runtime_error(std::string(option)+" invalid value: "+val);
        if(key=="tl") q.tl=(int)n; else if(key=="tr") q.tr=(int)n; else if(key=="bl") q.bl=(int)n; else if(key=="br") q.br=(int)n;
        else if(key=="palette") q.palette=(int)n; else if(key=="priority") q.priority=(int)n;
        else throw std::runtime_error(std::string(option)+" unknown key: "+key+" (supported: name,tl,tr,bl,br,palette,priority)");
        any=true;
    }
    if(!any || q.name.empty() || q.tl<0 || q.tr<0 || q.bl<0 || q.br<0)
        throw std::runtime_error(std::string(option)+" requires name,tl,tr,bl,br; optional palette,priority");
    return q;
}

static bool parse_core_option(Options& o, const std::string& arg, int& i, int argc, char** argv) {
    auto value = [&]() { return option_value(i, argc, argv, arg); };
    if (arg == "--roms") { o.roms = value(); return true; }
    else if (arg == "--logs") { o.logs = value(); o.logs_explicit=true; return true; }
    else if (arg == "--frames" || arg == "--exit-at-frame") {
        const auto text = value(); std::size_t end = 0;
        const auto n = std::stoul(text, &end);
        if (end != text.size() || n < 1 || n > 36000) throw std::runtime_error("Frame bound must be 1..36000");
        o.frames = static_cast<unsigned>(n); o.frames_explicit = true; return true;
    }
    else if (arg == "--irq4") { o.irq = true; return true; }
    else if (arg == "--no-boot-debug") { o.boot_debug = false; return true; }
    else if (arg == "--no-sub-debug") { o.sub_debug = false; return true; }
    else if (arg == "--no-handshake-debug") { o.handshake_debug = false; return true; }
    else if (arg == "--no-road-focus") { o.road_focus_debug = false; return true; }
    else if (arg == "--no-road-flow") { o.road_flow_debug = false; return true; }
    else if (arg == "--no-road-state") { o.road_state_debug = false; return true; }
    else if (arg == "--no-road-pending") { o.road_pending_debug = false; return true; }
    else if (arg == "--legacy-debug") {
        o.boot_debug = true; o.sub_debug = true; o.handshake_debug = true;
        o.road_focus_debug = true; o.road_flow_debug = true; o.road_state_debug = true; o.road_pending_debug = true;
        return true;
    }
    else if (arg == "--no-legacy-debug") {
        // v0.46 compatibility alias: legacy diagnostics are now off by default.
        o.boot_debug = false; o.sub_debug = false; o.handshake_debug = false;
        o.road_focus_debug = false; o.road_flow_debug = false; o.road_state_debug = false; o.road_pending_debug = false;
        return true;
    }
    else if (arg == "--force-road-flag") { o.force_road_flag = true; return true; }
    else if (arg == "--sub-samples") {
        const auto text=value(); std::size_t end=0; const auto n=std::stoul(text,&end);
        if(end!=text.size()||n<1||n>100000) throw std::runtime_error("Sub samples must be 1..100000");
        o.sub_samples=static_cast<unsigned>(n); return true;
    }
    else if (arg == "--road-samples") {
        const auto text=value(); std::size_t end=0; const auto n=std::stoul(text,&end);
        if(end!=text.size()||n<1||n>200000) throw std::runtime_error("Road samples must be 1..200000");
        o.road_samples=static_cast<unsigned>(n); return true;
    }
    else if (arg == "--state-samples") {
        const auto text=value(); std::size_t end=0; const auto n=std::stoul(text,&end);
        if(end!=text.size()||n<1||n>200000) throw std::runtime_error("State samples must be 1..200000");
        o.state_samples=static_cast<unsigned>(n); return true;
    }
    else if (arg == "--pending-samples") {
        const auto text=value(); std::size_t end=0; const auto n=std::stoul(text,&end);
        if(end!=text.size()||n<1||n>200000) throw std::runtime_error("Pending samples must be 1..200000");
        o.pending_samples=static_cast<unsigned>(n); return true;
    }
    else if (arg == "--flow-samples") {
        const auto text=value(); std::size_t end=0; const auto n=std::stoul(text,&end);
        if(end!=text.size()||n<1||n>500000) throw std::runtime_error("Flow samples must be 1..500000");
        o.flow_samples=static_cast<unsigned>(n); return true;
    }
    else if (arg == "--loop-samples") {
        const auto text=value(); std::size_t end=0; const auto n=std::stoul(text,&end);
        if(end!=text.size()||n<1||n>100000) throw std::runtime_error("Loop samples must be 1..100000");
        o.loop_samples=static_cast<unsigned>(n); return true;
    }
    else if (arg == "--pulse-ioc") { o.input_pulses.push_back(parse_input_pulse(value())); return true; }
    else if (arg == "--pulse-ioc-every") { o.periodic_input_pulses.push_back(parse_periodic_input_pulse(value())); return true; }
    else if (arg == "--one-hit-target") { o.target_one_hit=true; return true; }
    else if (arg == "--sprite-debug") { o.sprite_debug=true; return true; }
    else if (arg == "--sprite-debug-large") { o.sprite_debug=true; o.sprite_debug_large_only=true; return true; }
    else if (arg == "--sprite-dump-ram") { o.sprite_debug=true; o.sprite_dump_ram=true; return true; }
    else if (arg == "--sprite-export") { o.sprite_debug=true; o.sprite_export=true; return true; }
    else if (arg == "--sprite-export-tiles") { o.sprite_debug=true; o.sprite_export=true; o.sprite_export_tiles=true; return true; }
    else if (arg == "--sprite-export-analysis") { o.sprite_debug=true; o.sprite_export=true; o.sprite_export_analysis=true; return true; }
    else if (arg == "--sprite-export-atlas") { o.sprite_debug=true; o.sprite_export=true; o.sprite_export_atlas=true; return true; }
    else if (arg == "--sprite-export-alternates") { o.sprite_debug=true; o.sprite_export=true; o.sprite_export_alternates=true; return true; }
    else if (arg == "--sprite-debug-frame") { o.sprite_debug=true; o.sprite_debug_frames.push_back(parse_trace_count(value(),"sprite debug frame",36000)); return true; }
    else if (arg == "--sprite-debug-index") { o.sprite_debug=true; o.sprite_debug_index=static_cast<int>(parse_trace_count(value(),"sprite debug index",255)); return true; }
    return false;
}

static bool parse_graphics_option(Options& o, const std::string& arg, int& i, int argc, char** argv) {
    auto value = [&]() { return option_value(i, argc, argv, arg); };
    if (arg == "--graphics-debug-frame") { o.graphics_debug_all=true; o.graphics_debug_frames.push_back(parse_trace_count(value(),"graphics debug frame",36000)); return true; }
    else if (arg == "--sprite-evidence-every") { o.sprite_evidence_every=parse_trace_count(value(),"sprite evidence every",36000); if(!o.sprite_evidence_every) throw std::runtime_error("--sprite-evidence-every must be at least 1"); return true; }
    else if (arg == "--sprite-evidence-from") { o.sprite_evidence_from=parse_trace_count(value(),"sprite evidence from",36000); return true; }
    else if (arg == "--sprite-evidence-to") { o.sprite_evidence_to=parse_trace_count(value(),"sprite evidence to",36000); return true; }
    else if (arg == "--sprite-evidence-dir") { o.sprite_evidence_dir=value(); return true; }
    else if (arg == "--sprite-solo") { o.sprite_solo.push_back(parse_sprite_selector(value(),"--sprite-solo")); return true; }
    else if (arg == "--sprite-hide") { o.sprite_hide.push_back(parse_sprite_selector(value(),"--sprite-hide")); return true; }
    else if (arg == "--sprite-quad-candidate") { o.sprite_quad_candidates.push_back(parse_sprite_quad_candidate(value(),"--sprite-quad-candidate")); return true; }
    else if (arg == "--object-solo") { auto n=value(); o.sprite_solo.push_back(semantic_object_selector(n,"--object-solo")); return true; }
    else if (arg == "--object-evidence") { auto n=value(); semantic_object_selector(n,"--object-evidence"); o.object_evidence.push_back(n); return true; }
    else if (arg == "--object-track") { auto n=value(); semantic_object_selector(n,"--object-track"); o.object_track.push_back(n); return true; }
    else if (arg == "--diagnostic-background") { auto v=value(); if(v=="none")o.diagnostic_background=Options::DiagnosticBackground::None; else if(v=="black")o.diagnostic_background=Options::DiagnosticBackground::Black; else if(v=="white")o.diagnostic_background=Options::DiagnosticBackground::White; else if(v=="checkerboard")o.diagnostic_background=Options::DiagnosticBackground::Checkerboard; else throw std::runtime_error("--diagnostic-background must be none, black, white or checkerboard"); return true; }
    else if (arg == "--screenshot-every") { o.screenshot_every=parse_trace_count(value(),"screenshot every",36000); if(!o.screenshot_every) throw std::runtime_error("--screenshot-every must be at least 1"); return true; }
    else if (arg == "--screenshot-from") { o.screenshot_from=parse_trace_count(value(),"screenshot from",36000); return true; }
    else if (arg == "--screenshot-to") { o.screenshot_to=parse_trace_count(value(),"screenshot to",36000); return true; }
    else if (arg == "--screenshot-limit") { o.screenshot_limit=parse_trace_count(value(),"screenshot limit",100000); return true; }
    else if (arg == "--screenshot-dir") { o.screenshot_dir=value(); return true; }
    else if (arg == "--screenshot-at") { auto v=parse_frame_list(value(),"--screenshot-at"); o.screenshot_at.insert(o.screenshot_at.end(),v.begin(),v.end()); return true; }
    else if (arg == "--graphics-debug-all") { o.graphics_debug_all=true; o.graphics_export_raw_maps=true; return true; }
    else if (arg == "--graphics-export-raw-maps") { o.graphics_debug_all=true; o.graphics_export_raw_maps=true; return true; }
    else if (arg == "--gfx-trace-slot") { o.graphics_debug_all=true; o.graphics_trace.sprite_slots.push_back(parse_dec_limit(value(),"sprite slot",255)); return true; }
    else if (arg == "--gfx-trace-pair") { o.graphics_debug_all=true; o.graphics_trace.sprite_pairs.push_back(parse_dec_pair(value(),"sprite pair",255)); return true; }
    else if (arg == "--gfx-trace-palette-bank") { o.graphics_debug_all=true; o.graphics_trace.palette_banks.push_back(parse_dec_limit(value(),"palette bank",255)); return true; }
    else if (arg == "--gfx-trace-palette-entry") { o.graphics_debug_all=true; o.graphics_trace.palette_entries.push_back(parse_dec_limit(value(),"palette entry",4095)); return true; }
    else if (arg == "--gfx-trace-pen") { o.graphics_debug_all=true; o.graphics_trace.pens.push_back(parse_dec_limit(value(),"pen",15)); return true; }
    else if (arg == "--gfx-trace-prom-addr") { o.graphics_debug_all=true; auto [a,b]=parse_trace_range(value()); if(b>255) throw std::runtime_error("PROM address must be 00..ff"); o.graphics_trace.prom_addr_ranges.push_back({a,b}); return true; }
    else if (arg == "--gfx-trace-prom-value") { o.graphics_debug_all=true; auto v=parse_trace_hex(value()); if(v>255) throw std::runtime_error("PROM value must be 00..ff"); o.graphics_trace.prom_value=v; return true; }
    else if (arg == "--gfx-trace-priority") { o.graphics_debug_all=true; o.graphics_trace.priority_classes.push_back(parse_dec_limit(value(),"priority class",255)); return true; }
    else if (arg == "--gfx-trace-pixel") { o.graphics_debug_all=true; o.graphics_trace.explicit_location=true; o.graphics_trace.pixels.push_back(parse_xy(value())); return true; }
    else if (arg == "--gfx-trace-pixels") { o.graphics_debug_all=true; append_xy_list(o.graphics_trace,value(),false); return true; }
    else if (arg == "--gfx-trace-regression-set") { o.graphics_debug_all=true; apply_graphics_regression_set(o.graphics_trace,value()); return true; }
    else if (arg == "--gfx-trace-region") { o.graphics_debug_all=true; o.graphics_trace.explicit_location=true; o.graphics_trace.regions.push_back(parse_region(value())); return true; }
    else if (arg == "--gfx-trace-layer") { o.graphics_debug_all=true; o.graphics_trace.layers.push_back(value()); return true; }
    else if (arg == "--gfx-trace-road-priority") { o.graphics_debug_all=true; o.graphics_trace.road_priority=parse_dec_limit(value(),"road priority",255); return true; }
    else if (arg == "--gfx-trace-zero-palette") { o.graphics_debug_all=true; o.graphics_trace.zero_palette_use=true; return true; }
    else if (arg == "--gfx-trace-anomalies") { o.graphics_debug_all=true; o.graphics_trace.anomalies_only=true; return true; }
    else if (arg == "--pixel-provenance") { o.graphics_debug_all=true; o.graphics_trace.pixel_provenance=true; o.graphics_trace.explicit_location=true; o.graphics_trace.pixels.push_back(parse_xy(value())); return true; }
    else if (arg == "--pixel-provenance-pixels") { o.graphics_debug_all=true; append_xy_list(o.graphics_trace,value(),true); return true; }
    else if (arg == "--pixel-provenance-pair") { o.graphics_debug_all=true; o.graphics_trace.pixel_provenance=true; auto pr=parse_dec_pair(value(),"pixel provenance pair",255); o.graphics_trace.pixel_provenance_pair=pr; o.graphics_trace.sprite_pairs.push_back(pr); return true; }
    else if (arg == "--pixel-provenance-max") { o.graphics_debug_all=true; o.graphics_trace.pixel_provenance=true; o.graphics_trace.pixel_provenance_max=parse_trace_count(value(),"pixel provenance max",512); return true; }
    else if (arg == "--experiment-slot-zero-transparent") { o.graphics_debug_all=true; o.graphics_trace.experiment_zero_slot=parse_dec_limit(value(),"experiment zero slot",255); return true; }
    else if (arg == "--experiment-palette-zero-transparent") { o.graphics_debug_all=true; o.graphics_trace.experiment_zero_palette_bank=parse_dec_limit(value(),"experiment zero palette bank",255); return true; }
    else if (arg == "--experiment-sprite-pair-order") { o.graphics_debug_all=true; o.graphics_trace.experiment_sprite_pair_order=parse_dec_pair(value(),"experiment sprite pair order",255); return true; }
    else if (arg == "--experiment-sprite-order") { o.graphics_debug_all=true; auto v=value(); if(v!="ascending"&&v!="descending") throw std::runtime_error("--experiment-sprite-order expects ascending or descending"); o.graphics_trace.experiment_sprite_order=v; return true; }
    else if (arg == "--experiment-layer-order") {
        o.graphics_debug_all=true; o.graphics_trace.experiment_layer_order.clear(); std::stringstream ss(value()); std::string x;
        while(std::getline(ss,x,',')){ x=trim_copy(x); if(!x.empty()) o.graphics_trace.experiment_layer_order.push_back(x); }
        return true;
    }
    else if (arg == "--experiment-layer-matrix") { o.graphics_debug_all=true; o.graphics_trace.experiment_layer_matrix=true; return true; }
    else if (arg == "--experiment-sprite-mask-prio0") { o.graphics_debug_all=true; o.graphics_trace.experiment_sprite_mask_prio0=parse_hex_byte_option(value(),"priority-0 sprite mask"); return true; }
    else if (arg == "--experiment-sprite-mask-prio1") { o.graphics_debug_all=true; o.graphics_trace.experiment_sprite_mask_prio1=parse_hex_byte_option(value(),"priority-1 sprite mask"); return true; }
    else if (arg == "--experiment-sprite-mask-matrix") { o.graphics_debug_all=true; o.graphics_trace.experiment_sprite_mask_matrix=parse_mask_list(value()); return true; }
    else if (arg == "--gfx-trace-from-frame") { o.graphics_debug_all=true; o.graphics_trace.from_frame=parse_trace_count(value(),"graphics trace from frame",36000); return true; }
    else if (arg == "--gfx-trace-to-frame") { o.graphics_debug_all=true; o.graphics_trace.to_frame=parse_trace_count(value(),"graphics trace to frame",36000); return true; }
    else if (arg == "--gfx-trace-max") { o.graphics_debug_all=true; o.graphics_trace.max_lines=parse_trace_count(value(),"graphics trace max",2000000); return true; }
    else if (arg == "--gfx-trace-preset") { o.graphics_debug_all=true; apply_graphics_trace_preset(o.graphics_trace,value()); return true; }
    else if (arg == "--gfx-trace-config") { o.graphics_debug_all=true; o.graphics_trace_config_file=value(); load_graphics_trace_config_file(o.graphics_trace,o.graphics_trace_config_file); return true; }
    return false;
}

static bool parse_diagnostic_option(Options& o, const std::string& arg, int& i, int argc, char** argv) {
    auto value = [&]() { return option_value(i, argc, argv, arg); };
    if (arg == "--scenario") { apply_scenario(o,value()); return true; }
    else if (arg == "--scenario-list") { o.scenario_list=true; return true; }
    else if (arg == "--scenario-describe") { o.scenario_describe=value(); return true; }
    else if (arg == "--capture-frame") { o.capture_frames.push_back(parse_trace_count(value(),"capture frame",36000)); return true; }
    else if (arg == "--capture-frames") { auto v=parse_frame_list(value(),"--capture-frames"); o.capture_frames.insert(o.capture_frames.end(),v.begin(),v.end()); return true; }
    else if (arg == "--capture-relative") { o.capture_relative_offsets=parse_signed_frame_offsets(value(),"--capture-relative"); return true; }
    else if (arg == "--capture-after-patch") { o.capture_after_patch_offsets=parse_signed_frame_offsets(value(),"--capture-after-patch"); return true; }
    else if (arg == "--capture-on-change") { o.transition_triggers.push_back(parse_transition_trigger(value(),true)); return true; }
    else if (arg == "--capture-on-value") { o.transition_triggers.push_back(parse_transition_trigger(value(),false)); return true; }
    else if (arg == "--save-checkpoint") { o.save_checkpoints.push_back(parse_save_checkpoint(value())); return true; }
    else if (arg == "--load-checkpoint") { o.load_checkpoint=value(); return true; }
    else if (arg == "--checkpoint-dir") { o.checkpoint_dir=value(); return true; }
    else if (arg == "--checkpoint-slot") { o.checkpoint_slot=parse_trace_count(value(),"checkpoint slot",9); return true; }
    else if (arg == "--diagnostics") { apply_diagnostics_profile(o,value()); return true; }
    else if (arg == "--input-trace") { o.input_trace=true; return true; }
    else if (arg == "--trace-event") { apply_event_trace(o,value()); return true; }
    else if (arg == "--event-trace-from-frame") { o.event_trace.from_frame=parse_trace_count(value(),"event trace from frame",36000); o.trace.from_frame=o.event_trace.from_frame; return true; }
    else if (arg == "--event-trace-to-frame") { o.event_trace.to_frame=parse_trace_count(value(),"event trace to frame",36000); o.trace.to_frame=o.event_trace.to_frame; return true; }
    else if (arg == "--event-trace-max") { o.event_trace.max_lines=parse_trace_count(value(),"event trace max",2000000); o.trace.max_lines=o.event_trace.max_lines; return true; }
    else if (arg == "--debug-config") { o.debug_config_file=value(); load_master_debug_config(o,o.debug_config_file); return true; }
    else if (arg == "--debug-everything") { o.debug_everything=true; return true; }
    else if (arg == "--debug-api-port") { o.debug_api_port=parse_trace_count(value(),"debug API port",65535); if(o.debug_api_port<1) throw std::runtime_error("debug API port must be 1..65535"); return true; }
    else if (arg == "--no-debug-api") { o.debug_api=false; return true; }
    else if (arg == "--follow-address") { auto [a,b]=parse_trace_range(value()); o.provenance.follow_ranges.push_back({a,b}); o.provenance.reads=true; o.provenance.writes=true; o.provenance.access_filter_explicit=true; return true; }
    else if (arg == "--follow-read") { auto [a,b]=parse_trace_range(value()); o.provenance.follow_ranges.push_back({a,b}); if(!o.provenance.access_filter_explicit){o.provenance.reads=false;o.provenance.writes=false;o.provenance.access_filter_explicit=true;} o.provenance.reads=true; return true; }
    else if (arg == "--follow-write") { auto [a,b]=parse_trace_range(value()); o.provenance.follow_ranges.push_back({a,b}); if(!o.provenance.access_filter_explicit){o.provenance.reads=false;o.provenance.writes=false;o.provenance.access_filter_explicit=true;} o.provenance.writes=true; return true; }
    else if (arg == "--trace-callstack") { o.provenance.trace_callstack=true; o.provenance.verbose_calls=true; return true; }
    else if (arg == "--trace-callgraph") { o.provenance.trace_callgraph=true; return true; }
    else if (arg == "--profile-functions") { o.provenance.profile_functions=true; return true; }
    else if (arg == "--provenance-full-graph") { o.provenance.trace_callgraph=true; o.provenance.profile_functions=true; return true; }
    else if (arg == "--provenance-verbose-calls") { o.provenance.verbose_calls=true; return true; }
    else if (arg == "--provenance-from-frame") { o.provenance.from_frame=parse_trace_count(value(),"provenance from frame",36000); return true; }
    else if (arg == "--provenance-to-frame") { o.provenance.to_frame=parse_trace_count(value(),"provenance to frame",36000); return true; }
    else if (arg == "--provenance-depth") { o.provenance.max_depth=parse_trace_count(value(),"provenance depth",256); return true; }
    else if (arg == "--provenance-max") { o.provenance.max_events=parse_trace_count(value(),"provenance max",2000000); return true; }
    else if (arg == "--fast-forward-to") { o.fast_forward_to=parse_trace_count(value(),"fast-forward frame",36000); return true; }
    else if (arg == "--fast-forward-status") { o.fast_forward_status_every=parse_trace_count(value(),"fast-forward status interval",36000); return true; }
    else if (arg == "--progress-status") { o.progress_status_every=parse_trace_count(value(),"progress status interval",36000); return true; }
    else if (arg == "--fast-forward-events") { o.fast_forward_event_every=parse_trace_count(value(),"fast-forward event interval",36000); return true; }
    else if (arg == "--fast-forward-render-every") { o.fast_forward_render_every=parse_trace_count(value(),"fast-forward render interval",36000); return true; }
    else if (arg == "--investigate") { apply_investigate_profile(o,value()); return true; }
    else if (arg == "--watch-address") { o.memory_watches.push_back(parse_memory_watch(value())); return true; }
    else if (arg == "--patch-at-frame") { o.memory_patches.push_back(parse_memory_patch(value(),false)); return true; }
    else if (arg == "--patch-when") { o.memory_patches.push_back(parse_memory_patch(value(),true)); return true; }
    else if (arg == "--break-frame") { o.break_frames.push_back(parse_trace_count(value(),"break frame",36000)); return true; }
    else if (arg == "--break-frame-continue") { o.break_frame_continue=true; return true; }
    else if (arg == "--break-frame-wait") { o.break_frame_wait=true; o.break_frame_continue=true; return true; }
    else if (arg == "--break-history") { o.break_history=parse_trace_count(value(),"break history",100000); return true; }
    else if (arg == "--auto-zip-logs" || arg == "--evidence-bundle") { o.auto_zip_logs=true; return true; }
    else if (arg == "--shared-logs") { o.isolated_log_run=false; return true; }
    else if (arg == "--zip-name") { o.auto_zip_logs=true; o.zip_name=value(); return true; }
    else if (arg == "--evidence-name") { o.auto_zip_logs=true; o.evidence_name=value(); return true; }
    else if (arg == "--zip-delete-source") { o.auto_zip_logs=true; o.zip_delete_source=true; return true; }
    else if (arg == "--batch") { o.batch_file=value(); return true; }
    else if (arg == "--find-first-change") { auto v=value(); add_trace_mem(o.trace,v,false,true,false,true); auto [a,b]=parse_trace_range(v); o.provenance.follow_ranges.push_back({a,b}); o.provenance.reads=false; o.provenance.writes=true; return true; }
    else if (arg == "--no-run-manifest") { o.write_run_manifest=false; return true; }
    else if (arg == "--steering") { o.steering_fixed=parse_steering_value(value()); return true; }
    else if (arg == "--steering-at") { o.steering_events.push_back(parse_steering_event(value(),false)); return true; }
    else if (arg == "--steering-range") { o.steering_events.push_back(parse_steering_event(value(),true)); return true; }
    else if (arg == "--steering-signed") { o.steering_fixed=parse_steering_signed_value(value()); return true; }
    else if (arg == "--steering-signed-at") { o.steering_events.push_back(parse_steering_signed_event(value(),false)); return true; }
    else if (arg == "--steering-signed-range") { o.steering_events.push_back(parse_steering_signed_event(value(),true)); return true; }
    else if (arg == "--gameplay-state-log") { o.gameplay_state_log=true; return true; }
    else if (arg == "--target-state-log") { o.target_state_log=true; return true; }
    else if (arg == "--no-collisions") { o.no_collisions=true; o.target_state_log=true; return true; }
    else if (arg == "--course-data-log") { o.course_data_log=true; return true; }
    else if (arg == "--course-survey") { o.course_survey=true; o.course_follow=true; o.infinite_time=true; o.auto_turbo=true; o.unlimited_turbo=true; o.course_data_log=true; o.gameplay_state_log=true; o.target_state_log=true; o.auto_zip_logs=true; if(o.evidence_name.empty()) o.evidence_name="course_survey"; return true; }
    else if (arg == "--course-follow") { o.course_follow=true; return true; }
    else if (arg == "--course-profile") { o.course_profile_file=value(); o.course_follow=true; o.course_follow_controller="profile"; return true; }
    else if (arg == "--cornering-scale") { auto v=std::stod(value()); if(v<0.0||v>4.0) throw std::runtime_error("--cornering-scale expects 0.0..4.0"); o.cornering_scale=v; return true; }
    else if (arg == "--cornering-speed-retain") { auto v=std::stod(value()); if(v<0.0||v>2.0) throw std::runtime_error("--cornering-speed-retain expects 0.0..2.0"); o.cornering_speed_retain=v; return true; }
    else if (arg == "--course-follow-steer") { auto v=std::stoi(value()); if(v<1||v>96) throw std::runtime_error("--course-follow-steer expects 1..96"); o.course_follow_steer=v; return true; }
    else if (arg == "--course-follow-deadzone") { auto v=std::stoi(value()); if(v<0||v>127) throw std::runtime_error("--course-follow-deadzone expects 0..127"); o.course_follow_deadzone=v; return true; }
    else if (arg == "--course-follow-lookahead") { auto v=std::stoi(value()); if(v<0||v>8) throw std::runtime_error("--course-follow-lookahead expects 0..8"); o.course_follow_lookahead=v; return true; }
    else if (arg == "--course-follow-recovery-steer") { auto v=std::stoi(value()); if(v<1||v>96) throw std::runtime_error("--course-follow-recovery-steer expects 1..96"); o.course_follow_recovery_steer=v; return true; }
    else if (arg == "--course-follow-lateral-kp") { auto v=std::stod(value()); if(v<0.0||v>0.05) throw std::runtime_error("--course-follow-lateral-kp expects 0..0.05"); o.course_follow_lateral_kp=v; return true; }
    else if (arg == "--course-follow-lateral-max") { auto v=std::stoi(value()); if(v<1||v>96) throw std::runtime_error("--course-follow-lateral-max expects 1..96"); o.course_follow_lateral_max=v; return true; }
    else if (arg == "--course-follow-lateral-deadzone") { auto v=std::stoi(value()); if(v<0||v>4096) throw std::runtime_error("--course-follow-lateral-deadzone expects 0..4096"); o.course_follow_lateral_deadzone=v; return true; }
    else if (arg == "--course-follow-controller") { auto v=value(); if(v!="legacy"&&v!="predictive"&&v!="hybrid"&&v!="profile") throw std::runtime_error("--course-follow-controller expects legacy|predictive|hybrid|profile"); o.course_follow_controller=v; return true; }
    else if (arg == "--course-follow-lateral-kd") { auto v=std::stod(value()); if(v<0.0||v>0.1) throw std::runtime_error("--course-follow-lateral-kd expects 0..0.1"); o.course_follow_lateral_kd=v; return true; }
    else if (arg == "--course-follow-slew") { auto v=std::stoi(value()); if(v<1||v>96) throw std::runtime_error("--course-follow-slew expects 1..96"); o.course_follow_slew=v; return true; }
    else if (arg == "--course-follow-speed-control") { auto v=value(); if(v!="on"&&v!="off"&&v!="1"&&v!="0") throw std::runtime_error("--course-follow-speed-control expects on|off"); o.course_follow_speed_control=(v=="on"||v=="1"); return true; }
    else if (arg == "--infinite-time") { o.infinite_time=true; return true; }
    else if (arg == "--unlimited-turbo") { o.unlimited_turbo=true; return true; }
    else if (arg == "--auto-turbo") { o.auto_turbo=true; o.unlimited_turbo=true; return true; }
    else if (arg == "--tc0100scn-trace") { o.tc0100scn_trace=true; return true; }
    else if (arg == "--tc0100scn-trace-from") { o.tc0100scn_trace=true; o.tc0100scn_trace_from=parse_trace_count(value(),"TC0100SCN trace from",36000); return true; }
    else if (arg == "--tc0100scn-trace-to") { o.tc0100scn_trace=true; o.tc0100scn_trace_to=parse_trace_count(value(),"TC0100SCN trace to",36000); return true; }
    else if (arg == "--tc0100scn-scanlines") { o.tc0100scn_trace=true; auto v=value(); auto c=v.find(':'); if(c==std::string::npos) throw std::runtime_error("--tc0100scn-scanlines expects FROM:TO[:STEP]"); auto c2=v.find(':',c+1); o.tc0100scn_trace_scanline_from=parse_trace_count(v.substr(0,c),"scanline from",239); o.tc0100scn_trace_scanline_to=parse_trace_count(v.substr(c+1,c2==std::string::npos?std::string::npos:c2-c-1),"scanline to",239); if(c2!=std::string::npos) o.tc0100scn_trace_scanline_step=std::max(1u,parse_trace_count(v.substr(c2+1),"scanline step",240)); return true; }
    else if (arg == "--layer-offset") { apply_layer_offset(o,value()); return true; }
    else if (arg == "--sprite-tie-break") { auto v=value(); if(v=="lower-slot") o.sprite_tie_break=Options::SpriteTieBreak::LowerSlot; else if(v=="higher-slot") o.sprite_tie_break=Options::SpriteTieBreak::HigherSlot; else throw std::runtime_error("--sprite-tie-break expects lower-slot or higher-slot"); return true; }
    else if (arg == "--mixer") {
        const auto m=value();
        if(m=="reference") o.mixer=Options::MixerChoice::Reference;
        else if(m=="prom") o.mixer=Options::MixerChoice::Prom;
        else if(m=="legacy") o.mixer=Options::MixerChoice::Legacy;
        else throw std::runtime_error("--mixer must be reference, prom or legacy");
        return true;
    }
    return false;
}

static bool parse_trace_option(Options& o, const std::string& arg, int& i, int argc, char** argv) {
    auto value = [&]() { return option_value(i, argc, argv, arg); };
    if (arg == "--trace-mem") { add_trace_mem(o.trace,value(),true,true); return true; }
    else if (arg == "--trace-mem-r") { add_trace_mem(o.trace,value(),true,false); return true; }
    else if (arg == "--trace-mem-w") { add_trace_mem(o.trace,value(),false,true); return true; }
    else if (arg == "--trace-mem-nonzero-w") { add_trace_mem(o.trace,value(),false,true,true); return true; }
    else if (arg == "--trace-mem-change") { add_trace_mem(o.trace,value(),false,true,false,true); return true; }
    else if (arg == "--trace-pc") { add_trace_pc(o.trace,value()); return true; }
    else if (arg == "--trace-mem-pc") { add_trace_mem_pc(o.trace,value()); return true; }
    else if (arg == "--trace-arm-pc") { add_trace_arm_pc(o.trace,value()); return true; }
    else if (arg == "--trace-arm-mem") { add_trace_arm_mem(o.trace,value()); return true; }
    else if (arg == "--trace-task") { add_trace_task(o.trace,value()); return true; }
    else if (arg == "--trace-tasks") { o.trace.trace_all_tasks=true; return true; }
    else if (arg == "--trace-from-frame") { o.trace.from_frame=parse_trace_count(value(),"trace from frame",36000); return true; }
    else if (arg == "--trace-to-frame") { o.trace.to_frame=parse_trace_count(value(),"trace to frame",36000); return true; }
    else if (arg == "--trace-write-value") { o.trace.write_value_filter=parse_trace_hex(value()); return true; }
    else if (arg == "--trace-trigger-write" || arg == "--break-on-write") { add_trace_trigger(o.trace,value(),TraceTriggerKind::Write); return true; }
    else if (arg == "--trace-trigger-nonzero-write" || arg == "--break-on-nonzero-write") { add_trace_trigger(o.trace,value(),TraceTriggerKind::NonZeroWrite); return true; }
    else if (arg == "--trace-trigger-change" || arg == "--break-on-change") { add_trace_trigger(o.trace,value(),TraceTriggerKind::Change); return true; }
    else if (arg == "--trace-cpu") { set_trace_cpu(o.trace,value()); return true; }
    else if (arg == "--trace-preset") { apply_trace_preset(o.trace,value()); return true; }
    else if (arg == "--trace-config") { o.trace_config_file=value(); load_trace_config_file(o.trace,o.trace_config_file); return true; }
    else if (arg == "--trace-max") { const auto text=value(); o.trace.max_lines=parse_trace_count(text,"trace max",2000000); if(o.trace.max_lines<1) throw std::runtime_error("trace max must be at least 1"); return true; }
    else if (arg == "--trace-before") { o.trace.before=parse_trace_count(value(),"trace before",10000); return true; }
    else if (arg == "--trace-after") { o.trace.after=parse_trace_count(value(),"trace after",10000); return true; }
    else if (arg == "--trace-context") { const auto n=parse_trace_count(value(),"trace context",10000); o.trace.before=o.trace.after=n; return true; }
    else if (arg == "--trace-read") { o.trace.allow_reads=true; o.trace.allow_writes=false; return true; }
    else if (arg == "--trace-write") { o.trace.allow_reads=false; o.trace.allow_writes=true; return true; }
    else if (arg == "--trace-rw") { o.trace.allow_reads=o.trace.allow_writes=true; return true; }
    else if (arg == "--trace-all") { o.all=true; return true; }
    else if (arg == "--scene-only") { o.scene_only=true; return true; }
    else if (arg == "--help" || arg == "-h") { o.help=true; return true; }
    return false;
}

Options parse_options(int argc, char** argv) {
    Options o;
    for (int i=1; i<argc; ++i) {
        const std::string arg=argv[i];
        if (parse_core_option(o,arg,i,argc,argv)) continue;
        if (parse_graphics_option(o,arg,i,argc,argv)) continue;
        if (parse_diagnostic_option(o,arg,i,argc,argv)) continue;
        if (parse_trace_option(o,arg,i,argc,argv)) continue;
        if (i==1 && !arg.starts_with("--")) { o.roms=arg; continue; }
        throw std::runtime_error("Unknown option: " + arg);
    }
    if (o.debug_everything) {
        o.graphics_debug_all=true; o.graphics_export_raw_maps=true;
        o.sprite_debug=true; o.sprite_dump_ram=true; o.sprite_export=true;
        o.sprite_export_tiles=true; o.sprite_export_analysis=true;
        o.sprite_export_atlas=true; o.sprite_export_alternates=true;
        o.input_trace=true; o.graphics_trace.anomalies_only=true;
        if(o.graphics_debug_frames.empty()) o.graphics_debug_frames.push_back(o.frames);
        if(o.sprite_debug_frames.empty()) o.sprite_debug_frames.push_back(o.frames);
    }
    if (o.trace.to_frame < o.trace.from_frame) throw std::runtime_error("trace to frame must be >= trace from frame");
    if (o.graphics_trace.to_frame < o.graphics_trace.from_frame) throw std::runtime_error("graphics trace to frame must be >= graphics trace from frame");
    if (o.provenance.to_frame < o.provenance.from_frame) throw std::runtime_error("provenance to frame must be >= provenance from frame");
    return o;
}

void print_help() {
    std::cout << "ChaseHQ-Native " << kNativeVersion << " " << kNativePlatform << "\n"
        "[rom-directory] [--roms path] [--frames N] [--logs logs] [--irq4]\n"
        "Scenarios / reusable diagnostics:\n"
        "  --frames N                     explicit absolute frame bound (SDL frontend is continuous by default)\n"
        "  --exit-at-frame N              friendly alias for --frames N; works with loaded checkpoints\n"
        "  --scenario NAME                apply a reproducible setup (boot, stage1-gameplay, stage1-driving, service-mode, crash-test)\n"
        "  --scenario-list                list built-in scenarios and exit\n"
        "  --scenario-describe NAME       describe a built-in scenario and exit\n"
        "  --capture-frame N              capture mutable hardware state at frame N (repeatable)\n"
        "  --capture-frames LIST          comma-separated capture frames\n"
        "  --capture-on-change SPEC       CPU:WIDTH:ADDR[:LABEL], fire transition capture once\n"
        "  --capture-on-value SPEC        CPU:WIDTH:ADDR:VALUE[:LABEL], fire transition capture once\n"
        "  --capture-relative LIST        signed frame offsets around transition trigger\n"
        "  --capture-after-patch LIST     frame offsets after first applied memory patch\n"
        "  --save-checkpoint FRAME:PATH   save lightweight deterministic machine checkpoint\n"
        "  --load-checkpoint PATH         restore checkpoint and resume at its stored frame\n"
        "  --checkpoint-dir PATH          persistent UI state-slot directory (default checkpoints)\n"
        "  --checkpoint-slot N            initial UI save/load slot, 0-9 (default 0)\n"
        "  --diagnostics PROFILE          auto|none|graphics|memory|road|cpu|all\n"
        "  --debug-everything             maximal sprite/video/input diagnostic bundle at selected final frame\n"
        "  --debug-api-port N             localhost Research Workbench API port (default 37600)\n"
        "  --no-debug-api                 disable localhost Research Workbench API\n"
        "  --debug-config FILE            master key=value scenario/trace/capture config\n"
        "  --trace-event NAME             generic event category: ioc,palette,sprite,road,sound,cpu-control,irq\n"
        "  --event-trace-from-frame N / --event-trace-to-frame N / --event-trace-max N\n"
        "  --input-trace                  write per-frame IOC input/read-count CSV\n"
        "  --no-run-manifest              disable logs/run_manifest.json\n"
        "  --evidence-bundle              isolate logs and zip the complete run on exit\n"
        "  --evidence-name NAME           friendly bundle/run name; implies --evidence-bundle\n"
        "Execution provenance / timeline acceleration (v0.43):\n"
        "  --follow-address A[:B]         follow reads+writes with dynamic caller chain\n"
        "  --follow-read A[:B]            follow reads only with dynamic caller chain\n"
        "  --follow-write A[:B]           follow writes only with dynamic caller chain\n"
        "  --trace-callstack              reconstruct BSR/JSR/RTS/RTE call stacks\n"
        "  --trace-callgraph              export observed dynamic caller->callee graph\n"
        "  --profile-functions            export per-function instruction/read/write counts\n"
        "  --find-first-change A[:B]      change-only watch plus provenance for the writer\n"
        "  --provenance-from-frame N / --provenance-to-frame N\n"
        "  --provenance-depth N           maximum stack depth printed (default 24)\n"
        "  --provenance-max N             cap followed memory events (default 250000)\n"
        "  --provenance-full-graph        also build global callgraph + function profile (higher CPU cost)\n"
        "  --provenance-verbose-calls     log every observed CALL in the provenance window\n"
        "  --fast-forward-to N            run frames before N without normal pacing/rendering\n"
        "  --fast-forward-status N        print progress every N frames (default 120)\n"
        "  --progress-status N            bounded-run percent/rate/ETA every N frames (default 300; 0 disables)\n"
        "  --fast-forward-events N        pump SDL events every N frames (default 8)\n"
        "  --fast-forward-render-every N  render a progress frame every N fast-forward frames\n"
        "  --investigate NAME             one-command investigation profile: compositor-known-bad|timeout-watch\n"
        "  --watch-address SPEC          compact change watch CPU:WIDTH:ADDR[:LABEL]\n"
        "  --patch-at-frame SPEC          FRAME:CPU:WIDTH:ADDR:VALUE (CPU A/B; hex addr/value)\n"
        "  --patch-when SPEC              CPU:WIDTH:ADDR:EXPECTED:VALUE:STARTFRAME\n"
        "  --break-frame N                forensic frame breakpoint (repeatable)\n"
        "  --break-frame-continue         continue after forensic breakpoint (default stops)\n"
        "  --break-frame-wait             wait for Enter at breakpoint, then continue\n"
        "  --break-history N              rolling pre-break PC history entries (default 2048)\n"
        "  --auto-zip-logs                package this run into a self-describing ZIP\n"
        "  --shared-logs                  disable per-run isolated log directory\n"
        "  --zip-name NAME                override auto-generated ZIP filename\n"
        "  --zip-delete-source            delete raw logs after successful ZIP creation\n"
        "  --batch FILE                   run unattended jobs: one name|arguments line per job\n"
        "Input injection (debugger-only):\n"
        "  --pulse-ioc PORT:MASK:FRAME[:DURATION]  Repeatable; XOR selected IOC bits for N frames (default 3)\n"
        "  --pulse-ioc-every PORT:MASK:START:EVERY[:DURATION[:COUNT]] periodic deterministic IOC stimulus\n"
        "      PORT and MASK are hex; FRAME/DURATION are decimal. Repeatable.\n"
        "Deterministic steering / gameplay state + course diagnostics (v0.55.0):\n"
        "  --steering VALUE              fixed 16-bit IOC steering value (decimal or 0xHEX)\n"
        "  --steering-at FRAME:VALUE     set steering for one frame; repeatable\n"
        "  --steering-range A:B:VALUE   set raw steering for inclusive frame range; repeatable\n"
        "  --steering-signed VALUE       signed 12-bit steering (-2048..2047; known gameplay range approx -96..+96)\n"
        "  --steering-signed-at F:VALUE signed steering for one frame; repeatable\n"
        "  --steering-signed-range A:B:VALUE signed steering for inclusive frame range; repeatable\n"
        "  --gameplay-state-log          write authoritative known gameplay_state.csv (signed steering + road state)\n"
        "  --target-state-log            write target_state.csv + collision_events.csv for Stage-1 pursuit research\n"
        "  --one-hit-target             RC2.9 research cheat: next genuine special-target damage takes authentic terminal path\n"
        "  --no-collisions               suppress proven physical collision responses at $A142/$A156/$A1BE/$A1C4/$A200\n"
        "  --course-data-log             export 0x109000 course banks and live course position/channel state\n"
        "  --course-survey              accel + course-follow + infinite time + auto/unlimited turbo + state logs\n"
        "  --course-follow              predictive curvature + PD live-lateral course follower\n"
        "  --course-follow-steer N      steering magnitude for course follower, 1..96 (default 32)\n"
        "  --course-follow-deadzone N   abs(curvature) treated as straight (default 7)\n"
        "  --course-follow-lookahead N  inspect up to N upcoming records (default 2)\n"
        "  --course-follow-recovery-steer N legacy v0.56 compatibility option (closed-loop ignores blind recovery)\n"
        "  --course-follow-lateral-kp X proportional centring gain, 0..0.05 (default 0.006)\n"
        "  --course-follow-lateral-max N cap centring correction, 1..96 (default 48)\n"
        "  --course-follow-lateral-deadzone N ignore abs lateral error <= N (default 96)\n"
        "  --course-follow-controller MODE legacy|predictive|hybrid|profile (default hybrid)\n  --course-profile FILE          replay driver_profile.csv as feed-forward + live centre correction\n  --cornering-scale X            scale proven lateral handling coefficient (1.0 = native)\n  --cornering-speed-retain X     scale proven forward handling coefficient (1.0 = native)\n"
        "  --course-follow-lateral-kd X derivative damping gain, 0..0.1 (default 0.012)\n"
        "  --course-follow-slew N       max steering-output change/frame, 1..96 (default 8)\n"
        "  --course-follow-speed-control on|off predictive curve-aware throttle/brake (default off)\n"
        "  --infinite-time              continuously hold the race timer at its starting value\n"
        "  --unlimited-turbo            continuously restore turbos remaining to 3\n"
        "  --auto-turbo                 unlimited turbo plus periodic genuine turbo-button pulses\n"
        "TC0100SCN geometry tracing (v0.53.0):\n"
        "  --tc0100scn-trace             write tc0100scn_trace.csv while frames render\n"
        "  --tc0100scn-trace-from N      lower frame bound\n"
        "  --tc0100scn-trace-to N        upper frame bound (0 = unbounded)\n"
        "  --tc0100scn-scanlines A:B[:S] output visible scanlines A..B, optional step (default 8)\n"
        "Sprite diagnostics (frontend):\n"
        "  --sprite-debug                 enable decoded sprite snapshots\n"
        "  --sprite-debug-frame N         dump decoded sprites at frame N (repeatable)\n"
        "  --sprite-debug-index N         restrict decoded dump to sprite slot N\n"
        "  --sprite-debug-large           restrict dump to 64/128-wide multi-chunk objects\n"
        "  --sprite-dump-ram              also save raw 0x800-byte sprite RAM per snapshot\n"
        "  --sprite-export                export assembled/rendered sprites + contact sheet as PNG\n"
        "  --sprite-export-tiles          also export every referenced raw 16x16 chunk PNG\n"
        "  --sprite-export-analysis       write map/tile/zoom validation CSVs and fault summaries\n"
        "  --sprite-export-atlas          export complete OBJ-A/OBJ-B tile atlas PNGs\n"
        "  --sprite-export-alternates     export current vs byte-swapped/alternate-gfx comparison PNGs\n"
        "  --trace-preset sprite          trace all sprite-RAM writes and first change context\n"
        "Layer geometry experiments (v0.51.2; presentation only):\n"
        "  --layer-offset LAYER:X:Y      move bg0,bg1,text,sprites,road or all by signed pixels; repeatable\n"
        "                                negative Y moves upward; BG0/BG1 default to 0:0 in RC2.4 after the TC0100SCN Y-scroll sign fix\n"
        "                                config-file equivalent: layer_offset=bg0:0:0\n"
        "  --sprite-tie-break MODE       sprite traversal: lower-slot=descending (default), higher-slot=ascending; first nonzero pen owns\n"
        "                                config: sprite_tie_break=lower-slot\n"
        "Final-video/compositor diagnostics (frontend):\n"
        "  --sprite-evidence-every N     export authoritative composed sprite CSV every N frames\n"
        "  --sprite-evidence-from N      lower frame bound for sprite evidence\n"
        "  --sprite-evidence-to N        upper frame bound for sprite evidence (0 = unbounded)\n"
        "  --sprite-evidence-dir PATH    output directory (default <logs>/sprite_evidence)\n"
        "  --sprite-solo SELECTOR        render only matching sprites; fields support pipe sets, e.g. map=778|779|780|781,palette=134\n"
        "  --sprite-hide SELECTOR        suppress matching sprites; same selector syntax; repeatable OR selectors\n"
        "  --sprite-quad-candidate DEF   report exact 2x2 candidate assemblies: name=N,tl=778,tr=779,bl=780,br=781,palette=134\n"
        "  --object-solo NAME            presentation-only semantic object isolation (currently: player_car)\n"
        "  --object-evidence NAME        export one semantic-object summary row per sprite-evidence frame\n"
        "  --object-track NAME           keep semantic-object evidence enabled without forcing solo rendering\n"
        "  --diagnostic-background MODE  none|black|white|checkerboard; presentation-only background\n"
        "  --screenshot-every N          save final composed framebuffer every N emulated frames\n"
        "  --screenshot-at LIST          save final framebuffer at specific comma-separated frames\n"
        "  --screenshot-from N           lower frame bound for periodic capture (default 0)\n"
        "  --screenshot-to N             upper frame bound for periodic capture (0 = unbounded)\n"
        "  --screenshot-limit N          maximum automatic screenshots (default 1000)\n"
        "  --screenshot-dir PATH         output directory (default <logs>/screenshots)\n"
        "  --mixer reference|prom|legacy  choose live compositor (default prom)\n"
        "  --graphics-debug-all           export full layer/priority/sprite-owner diagnostic bundle\n"
        "  --graphics-debug-frame N       capture compositor diagnostics at frame N (repeatable)\n"
        "  --graphics-export-raw-maps     also export per-pixel raw/binary maps and CSV\n"
        "Parameterized graphics tracer (repeat filters as needed):\n"
        "  --gfx-trace-slot N             focus logical sprite slot N\n"
        "  --gfx-trace-pair A:B           focus interaction/overlap of two sprite slots\n"
        "  --gfx-trace-palette-bank N     focus TC0110PCR palette bank 0..255\n"
        "  --gfx-trace-palette-entry N    focus exact TC0110PCR entry 0..4095\n"
        "  --gfx-trace-pen N              focus source pen 0..15\n"
        "  --gfx-trace-prom-addr A[:B]    filter PROM addresses (hex 00..ff)\n"
        "  --gfx-trace-prom-value HEX     filter exact PROM output\n"
        "  --gfx-trace-priority N         filter decoded priority class\n"
        "  --gfx-trace-pixel X:Y          full pipeline for one screen pixel (repeatable)\n"
        "  --gfx-trace-pixels LIST         multiple pixels in one arg, e.g. 99:165,150:224\n"
        "  --gfx-trace-regression-set NAME named pixel set; includes known-bad\n"
        "  --gfx-trace-region X1:Y1:X2:Y2 pipeline rows for a rectangle\n"
        "  --gfx-trace-layer NAME         focus road|bg0|bg1|sprite|text (repeatable)\n"
        "  --gfx-trace-road-priority N    filter raw road-priority/probe value\n"
        "  --gfx-trace-zero-palette       focus uses of hardware palette value $0000\n"
        "  --gfx-trace-anomalies          emit suspicious/invariant cases only\n"
        "  --pixel-provenance X:Y         explain every sprite/palette/priority decision at pixel\n"
        "  --pixel-provenance-pixels LIST explain multiple pixels in one run (tuple/list syntax accepted)\n"
        "  --pixel-provenance-pair A:B    auto-pick overlapping pixels for a sprite pair\n"
        "  --pixel-provenance-max N       cap auto-selected provenance pixels (default 32)\n"
        "Controlled compositor experiments (diagnostic only; never change default renderer):\n"
        "  --experiment-slot-zero-transparent N    zero-valued palette pixels transparent for one slot\n"
        "  --experiment-palette-zero-transparent N zero-valued pixels transparent for one palette bank\n"
        "  --experiment-sprite-pair-order A:B       render A-before-B and B-before-A comparison\n"
        "  --experiment-sprite-order ascending|descending alternate sprite RAM traversal\n"
        "  --experiment-layer-order LIST            comma order: bottom,upper,road,sprites,text\n"
        "  --experiment-layer-matrix                render controlled layer-order comparison matrix\n"
        "  --experiment-sprite-mask-prio0 HEX     override priority-0 primask in experiment renders\n"
        "  --experiment-sprite-mask-prio1 HEX     override priority-1 primask in experiment renders\n"
        "  --experiment-sprite-mask-matrix LIST   test hex masks (comma list or 'default') and score regressions\n"
        "  --gfx-trace-from-frame N --gfx-trace-to-frame N   graphics trace window\n"
        "  --gfx-trace-max N              targeted CSV line cap\n"
        "  --gfx-trace-preset car|palette|priority|prom|road-priority|compositor\n"
        "  --gfx-trace-config FILE        load repeatable graphics filters from key=value file\n"
        "Generic tracer (repeat --trace-mem / --trace-pc as needed):\n"
        "  --trace-mem START[:END]          trace reads+writes\n"
        "  --trace-mem-r START[:END]        trace reads only\n"
        "  --trace-mem-w START[:END]        trace writes only\n"
        "  --trace-mem-nonzero-w RANGE      trace only non-zero writes\n"
        "  --trace-mem-change RANGE         trace writes only when memory value changes\n"
        "  --trace-pc START[:END]           log execution + registers + disassembly\n"
        "  --trace-mem-pc START[:END]       gate memory watches/triggers by executing PC\n"
        "  --trace-from-frame N --trace-to-frame N   frame window; every record includes frame=N\n"
        "  --trace-write-value HEX          log watched writes only when value exactly matches\n"
        "  --trace-arm-pc RANGE             keep normal trace silent until this PC executes\n"
        "  --trace-arm-mem ADDRESS:VALUE    keep normal trace silent until exact write occurs\n"
        "  --trace-task SLOT                log scheduler state changes for one task slot\n"
        "  --trace-tasks                    log state changes for all 0x10-byte scheduler slots\n"
        "  --trace-cpu A|B|both             select CPU(s), default both\n"
        "  --trace-read | --trace-write | --trace-rw   global memory-access filter\n"
        "  --trace-trigger-write RANGE      capture context on first write\n"
        "  --trace-trigger-nonzero-write RANGE   capture context on first non-zero write\n"
        "  --trace-trigger-change RANGE     capture context on first value change\n"
        "  --break-on-write RANGE | --break-on-nonzero-write RANGE | --break-on-change RANGE\n"
        "      aliases for the trigger options; tracing continues after capture\n"
        "  --trace-before N --trace-after N   rolling instruction context (default 32/64)\n"
        "  --trace-context N                set before and after to the same value\n"
        "  --trace-max N                    maximum lines in logs/trace.log (default 100000)\n"
        "  --trace-preset road|handshake|command|sprite\n"
        "  --trace-config FILE              load key=value watchpoints from a text file\n"
        "  --legacy-debug                   enable older hard-coded diagnostic logs (OFF by default)\n"
        "  --no-legacy-debug                deprecated compatibility alias\n"
        "Legacy options remain available: --trace-all --loop-samples --sub-samples --road-samples\n"
        "--flow-samples --state-samples --pending-samples --force-road-flag and --no-* debug flags.\n"
        "Addresses supplied to the generic tracer are hexadecimal with or without 0x.\n"
        "Generic output is written to logs/trace.log and summarized in summary.txt.\n";
}

}
