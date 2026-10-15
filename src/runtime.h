#pragma once
#include "cpu_rom.h"
#include "native_execution.h"
#include "live_candidate_shadow.h"
#include "native_sprite_clear.h"
#include "projection_shadow.h"
#include "steering_conversion.h"
#include "native_health_abi.h"
#include "native_copy_abi.h"
#include "native_cpu_b_abi.h"
#include <algorithm>
#include <array>
#include <deque>
#include <fstream>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>

void runtime_instruction_hook(unsigned int pc);
int runtime_interrupt_ack(int level);

namespace chq {

enum class BusSpace { Main, Sub };

// v0.60.0 Research Workbench -------------------------------------------------
// Generic, low-overhead primitives for live observation/intervention.  These
// intentionally do not encode Chase H.Q.-specific semantics so future audio,
// input, device and rendering adapters can publish into the same model.
enum class ResearchEventKind { Read, Write, Intervention, SuppressedWrite };

// v0.66.4: semantic TC0110PCR palette-write provenance. Unlike generic memory
// watches this records the addressed palette entry and complete 16-bit colour.
struct PaletteWriteEvent {
    unsigned frame = 0;
    BusSpace space = BusSpace::Main;
    std::uint32_t pc = 0;
    std::uint16_t index = 0;
    std::uint16_t old_value = 0;
    std::uint16_t new_value = 0;
    std::uint32_t write_count = 0;
    bool changed = false;
};

// v0.66.5: generic bounded write provenance for arbitrary emulated memory.
// Captured at the Bus::write transaction boundary so PC/old/new correspond to
// the actual writer, not a later sampled machine state.
struct MemoryWriteTraceEvent {
    char access = 'W'; // W for writes; R when emitted by the RC4 bounded read-provenance trace
    unsigned frame = 0;
    BusSpace space = BusSpace::Main;
    std::uint32_t pc = 0;
    std::uint32_t address = 0;
    unsigned size = 1;
    std::uint32_t old_value = 0;
    std::uint32_t new_value = 0;
    std::uint64_t write_count = 0;
    bool changed = false;
};


// RC4 structured collision-response provenance. Detection-side interaction
// state is sampled at the exact response write so Workbench experiments no
// longer need to infer object/type from long trajectories or cumulative counts.
struct CollisionResponseEvent {
    unsigned frame = 0;
    BusSpace space = BusSpace::Main;
    std::uint32_t pc = 0;
    std::uint32_t address = 0;
    unsigned size = 1;
    std::uint32_t old_value = 0;
    std::uint32_t attempted_value = 0;
    std::uint32_t applied_value = 0;
    std::uint16_t interaction_type = 0;
    std::uint32_t object_reference = 0;
    bool lateral = false;
    bool speed = false;
    bool target_contact = false;
    bool suppressed = false;
    std::uint64_t sequence = 0;
};

// RC4 first runtime audio instrumentation: command traffic to the currently
// stubbed 0x820000..03 sound interface. This does not claim a sound CPU/mixer.
struct SoundCommandEvent {
    unsigned frame = 0;
    BusSpace space = BusSpace::Main;
    std::uint32_t pc = 0;
    std::uint32_t address = 0;
    unsigned size = 1;
    std::uint32_t value = 0;
    std::uint64_t sequence = 0;
};

enum class LivePatchMode { Freeze, Suppress, Replace };

struct ResearchEvent {
    unsigned frame = 0;
    ResearchEventKind kind = ResearchEventKind::Write;
    BusSpace space = BusSpace::Main;
    std::uint32_t pc = 0;
    std::uint32_t address = 0;
    unsigned size = 1;
    std::uint32_t old_value = 0;
    std::uint32_t new_value = 0;
    bool changed = false;
};

struct ResearchWatch {
    unsigned id = 0;
    BusSpace space = BusSpace::Main;
    std::uint32_t start = 0;
    std::uint32_t end = 0;
    bool reads = false;
    bool writes = true;
    bool change_only = false;
    bool break_on_match = false;
};

struct LivePatch {
    unsigned id = 0;
    BusSpace space = BusSpace::Main;
    std::uint32_t address = 0;
    unsigned size = 1;
    std::uint32_t value = 0;
    LivePatchMode mode = LivePatchMode::Freeze;
    std::uint64_t interceptions = 0;
    unsigned last_frame = 0;
    std::uint32_t last_pc = 0;
};

enum class TraceTriggerKind { Write, NonZeroWrite, Change };

struct TraceRange {
    std::uint32_t start = 0;
    std::uint32_t end = 0;
    bool reads = true;
    bool writes = true;
    bool nonzero_only = false;
    bool change_only = false;
};

struct TracePcRange {
    std::uint32_t start = 0;
    std::uint32_t end = 0;
};

struct TraceTrigger {
    std::uint32_t start = 0;
    std::uint32_t end = 0;
    TraceTriggerKind kind = TraceTriggerKind::Write;
};

struct TraceArmMemory {
    std::uint32_t address = 0;
    std::uint32_t value = 0;
};

struct TraceHistoryEntry {
    BusSpace space = BusSpace::Main;
    std::uint32_t pc = 0;
};

struct TraceConfig {
    std::vector<TraceRange> mem_ranges;
    std::vector<TracePcRange> pc_ranges;
    std::vector<TracePcRange> mem_pc_ranges; // optional PC gate for memory watches/triggers
    std::vector<TraceTrigger> triggers;
    std::vector<TracePcRange> arm_pc_ranges;
    std::vector<TraceArmMemory> arm_mem_writes;
    std::vector<std::uint32_t> task_slots;
    bool trace_all_tasks = false;
    unsigned from_frame = 0;
    unsigned to_frame = 0xffffffffu;
    std::optional<std::uint32_t> write_value_filter;
    unsigned cpu_mask = 0x3; // bit0=A, bit1=B
    bool allow_reads = true;
    bool allow_writes = true;
    std::uint64_t max_lines = 100000;
    unsigned before = 32;
    unsigned after = 64;

    bool enabled() const {
        return !mem_ranges.empty() || !pc_ranges.empty() || !triggers.empty() ||
               !arm_pc_ranges.empty() || !arm_mem_writes.empty() ||
               trace_all_tasks || !task_slots.empty();
    }
};



struct ProvenanceRange {
    std::uint32_t start = 0;
    std::uint32_t end = 0;
};

struct ProvenanceConfig {
    std::vector<ProvenanceRange> follow_ranges;
    bool reads = true;
    bool writes = true;
    bool access_filter_explicit = false; // v0.54: combine repeated --follow-read/--follow-write instead of last-option-wins
    // Shadow stacks are always maintained while provenance is enabled. These
    // switches control expensive global exports/logging rather than stack tracking.
    bool trace_callstack = false;
    bool trace_callgraph = false;
    bool profile_functions = false;
    bool verbose_calls = false;
    unsigned from_frame = 0;
    unsigned to_frame = 0xffffffffu;
    unsigned max_depth = 24;
    std::uint64_t max_events = 250000;
    bool enabled() const {
        return !follow_ranges.empty() || trace_callstack || trace_callgraph || profile_functions;
    }
};

struct Region {
    const char* name;
    std::uint32_t base;
    Bytes bytes;
    bool readonly = false;
    std::uint64_t reads = 0, writes = 0;
};

class Bus {
public:
    Bus(Bytes main_rom, Bytes sub_rom);
    void open_log(const std::filesystem::path&, std::uint64_t limit = 40000, bool all = false);
    void open_boot_debug_log(const std::filesystem::path&, std::uint64_t limit = 10000);
    void open_sub_debug_log(const std::filesystem::path&, std::uint64_t limit = 20000);
    void open_handshake_log(const std::filesystem::path&, std::uint64_t limit = 30000);
    void open_road_focus_log(const std::filesystem::path&, std::uint64_t limit = 40000);
    void open_road_data_flow_log(const std::filesystem::path&, std::uint64_t limit = 60000);
    void open_road_state_log(const std::filesystem::path&, std::uint64_t limit = 30000);
    void open_road_pending_log(const std::filesystem::path&, std::uint64_t limit = 30000);
    std::uint32_t read(std::uint32_t address, unsigned size, BusSpace space = BusSpace::Main);
    void set_native_steering_bus_observer(std::function<void(char,std::uint32_t,std::uint32_t,unsigned)> cb) {
        native_steering_bus_observer_=std::move(cb);
    }
    // RC6.3.11: interrupt acknowledge is an execution phase, not the previous
    // instruction. This flag is asserted by the real Musashi ACK callback and
    // cleared at the next instruction hook. Never infer IRQ ownership by PC.
    // RC6.3.27: separate original-ROM ACK-scoped bus observer. This is never
    // part of the native substitution transaction stream.
    // RC6.3.31: optional read-only witness of CPU-B data writes intersecting
    // the BTST source byte. No bus modifications or ROM replacement.
    void set_cpu_b_btst_writer_observer(std::function<void(BusSpace,std::uint32_t,unsigned,std::uint8_t,std::uint8_t)> cb) {
        cpu_b_btst_writer_observer_=std::move(cb);
    }
    void set_cpu_b_original_irq_phase(bool active) { cpu_b_original_irq_phase_=active; }
    void set_cpu_b_original_irq_observer(std::function<void(char,std::uint32_t,std::uint32_t,unsigned)> cb) {
        cpu_b_original_irq_observer_=std::move(cb);
    }
    void set_native_cpu_b_irq_phase(bool active) { native_cpu_b_irq_phase_=active; }
    void set_native_cpu_b_irq_bus_observer(std::function<void(char,std::uint32_t,std::uint32_t,unsigned)> cb) { native_cpu_b_irq_bus_observer_=std::move(cb); }
    void set_native_cpu_b_bus_observer(std::function<void(char,std::uint32_t,std::uint32_t,unsigned)> cb) {native_cpu_b_bus_observer_=std::move(cb);}
    void set_native_cpu_b_pair_bus_observer(std::function<void(char,std::uint32_t,std::uint32_t,unsigned)> cb) {native_cpu_b_pair_bus_observer_=std::move(cb);}
    void set_native_copy_bus_observer(std::function<void(char,std::uint32_t,std::uint32_t,unsigned)> cb) {native_copy_bus_observer_=std::move(cb);}
    void set_native_health_bus_observer(std::function<void(char,std::uint32_t,std::uint32_t,unsigned)> cb) {
        native_health_bus_observer_=std::move(cb);
    }

    void write(std::uint32_t address, std::uint32_t value, unsigned size, BusSpace space = BusSpace::Main);
    std::uint32_t peek(std::uint32_t address, unsigned size, BusSpace space = BusSpace::Main);

    // v0.60.0 live Research Workbench.  debug_write() deliberately bypasses
    // active interventions so the investigator can alter the paused machine.
    void research_enable(bool enabled) { research_enabled_ = enabled; }
    bool research_enabled() const { return research_enabled_; }
    void research_clear_events() { research_events_.clear(); }
    void research_set_event_limit(std::size_t n) { research_event_limit_ = std::max<std::size_t>(64, std::min<std::size_t>(n, 1000000)); while(research_events_.size()>research_event_limit_) research_events_.pop_front(); }
    std::size_t research_event_limit() const { return research_event_limit_; }
    const std::deque<ResearchEvent>& research_events() const { return research_events_; }
    unsigned research_add_watch(const ResearchWatch& watch);
    bool research_remove_watch(unsigned id);
    void research_clear_watches() { research_watches_.clear(); research_break_pending_ = false; research_break_reason_.clear(); }
    const std::vector<ResearchWatch>& research_watches() const { return research_watches_; }
    void palette_trace_start(const std::vector<std::uint16_t>& indices, std::size_t limit = 4096);
    void palette_trace_stop() { palette_trace_enabled_ = false; }
    void palette_trace_clear() { palette_trace_events_.clear(); }
    bool palette_trace_enabled() const { return palette_trace_enabled_; }
    std::size_t palette_trace_limit() const { return palette_trace_limit_; }
    const std::array<bool,4096>& palette_trace_filter() const { return palette_trace_filter_; }
    const std::deque<PaletteWriteEvent>& palette_trace_events() const { return palette_trace_events_; }
    void memory_trace_start(BusSpace space, std::uint32_t address, std::uint32_t length, unsigned width = 0, std::size_t limit = 4096, bool reads = false, bool writes = true);
    void memory_trace_stop() { memory_trace_enabled_ = false; }
    void memory_trace_clear() { memory_trace_events_.clear(); memory_trace_write_count_ = 0; }
    bool memory_trace_enabled() const { return memory_trace_enabled_; }
    BusSpace memory_trace_space() const { return memory_trace_space_; }
    std::uint32_t memory_trace_address() const { return memory_trace_address_; }
    std::uint32_t memory_trace_length() const { return memory_trace_length_; }
    unsigned memory_trace_width() const { return memory_trace_width_; }
    std::size_t memory_trace_limit() const { return memory_trace_limit_; }
    bool memory_trace_reads() const { return memory_trace_reads_; }
    bool memory_trace_writes() const { return memory_trace_writes_; }
    const std::deque<MemoryWriteTraceEvent>& memory_trace_events() const { return memory_trace_events_; }
    // v0.66.8.0 forensic timeline: optional unbounded-by-address write sink.
    // The timeline owner is responsible for bounding capture duration/storage.
    void set_timeline_write_sink(std::function<void(const MemoryWriteTraceEvent&)> sink) { timeline_write_count_ = 0; timeline_write_sink_ = std::move(sink); }
    void clear_timeline_write_sink() { timeline_write_sink_ = {}; }
    bool timeline_write_sink_enabled() const { return static_cast<bool>(timeline_write_sink_); }
    bool research_break_pending() const { return research_break_pending_; }
    std::string research_take_break_reason();
    void debug_write(std::uint32_t address, std::uint32_t value, unsigned size, BusSpace space = BusSpace::Main);
    unsigned research_add_patch(BusSpace space, std::uint32_t address, unsigned size, std::uint32_t value, LivePatchMode mode);
    void research_clear_patches() { live_patches_.clear(); }
    bool research_remove_patch(unsigned id);
    const std::vector<LivePatch>& research_patches() const { return live_patches_; }
    void summary(std::ostream&) const;
    void dump(const std::filesystem::path&) const;
    bool executable(std::uint32_t pc, BusSpace space) const;
    void set_trace_pc(BusSpace space, std::uint32_t pc);
    void set_quiet_fast_forward(bool enabled) { quiet_fast_forward_ = enabled; }
    // v0.59.1 mapping/research assist: suppress proven physical collision responses
    // while preserving collision detection, event/scoring logic and object simulation.
    void set_suppress_collision_shove(bool enabled) { suppress_collision_shove_ = enabled; }
    bool suppress_collision_shove() const { return suppress_collision_shove_; }
    std::uint64_t suppressed_collision_shoves() const { return suppressed_collision_shoves_; }
    std::uint64_t suppressed_collision_lateral() const { return suppressed_collision_lateral_; }
    std::uint64_t suppressed_collision_speed() const { return suppressed_collision_speed_; }
    const std::deque<CollisionResponseEvent>& collision_response_events() const { return collision_response_events_; }
    void clear_collision_response_events() { collision_response_events_.clear(); collision_response_sequence_ = 0; }
    const std::deque<SoundCommandEvent>& sound_command_events() const { return sound_command_events_; }
    void clear_sound_command_events() { sound_command_events_.clear(); sound_command_sequence_ = 0; }
    using GenericTraceCallback = std::function<void(char, BusSpace, std::uint32_t, std::uint32_t, unsigned, std::uint32_t, bool, const char*)>;
    using GenericTraceInterestCallback = std::function<bool(char, BusSpace, std::uint32_t, unsigned)>;
    using GenericTraceOldValueCallback = std::function<bool(BusSpace, std::uint32_t, unsigned)>;
    void set_generic_trace_callback(GenericTraceCallback cb) { generic_trace_cb_ = std::move(cb); }
    void set_generic_trace_interest_callback(GenericTraceInterestCallback cb) { generic_trace_interest_cb_ = std::move(cb); }
    void set_generic_trace_old_value_callback(GenericTraceOldValueCallback cb) { generic_trace_old_value_cb_ = std::move(cb); }

    std::uint8_t ioc_selected_port() const { return ioc_port_; }
    std::uint8_t ioc_port_value(std::uint8_t port) const;
    void set_ioc_steering(std::uint16_t value) { ioc_steering_ = value; }
    std::uint16_t ioc_steering() const { return ioc_steering_; }
    // Persistent debugger/research intervention layer. This must survive running frames.
    void set_ioc_input_xor_mask(std::uint8_t port, std::uint8_t mask) { ioc_input_xor_[port & 0x0f] = mask; }
    void clear_ioc_input_overrides() { ioc_input_xor_.fill(0); }
    std::uint8_t ioc_input_xor_mask(std::uint8_t port) const { return ioc_input_xor_[port & 0x0f]; }
    // Scenario/CLI pulse layer. Updated by deterministic frame runners without clobbering debugger state.
    void set_ioc_scenario_xor_mask(std::uint8_t port, std::uint8_t mask) { ioc_scenario_xor_[port & 0x0f] = mask; }
    void clear_ioc_scenario_overrides() { ioc_scenario_xor_.fill(0); }
    std::uint8_t ioc_scenario_xor_mask(std::uint8_t port) const { return ioc_scenario_xor_[port & 0x0f]; }
    std::uint8_t ioc_effective_xor_mask(std::uint8_t port) const { const auto p=port & 0x0f; return static_cast<std::uint8_t>(ioc_input_xor_[p] ^ ioc_scenario_xor_[p]); }
    std::uint64_t ioc_read_count(std::uint8_t port) const { return ioc_reads_[port & 0x0f]; }
    void set_ioc_debug_frame(unsigned frame) { ioc_debug_frame_ = frame; }
    void open_ioc_pulse_log(const std::filesystem::path& path);
    void save_checkpoint_state(std::ostream& out) const;
    void load_checkpoint_state(std::istream& in);

    bool sub_enabled() const { return (cpu_control_ & 1) != 0; }
    std::uint8_t cpu_control() const { return cpu_control_; }
    const std::array<std::uint64_t, 0x1000>& road_word_writes() const { return road_word_writes_; }
    const std::array<std::uint64_t, 0x1000>& road_word_value_changes() const { return road_word_value_changes_; }
    const std::array<std::uint64_t, 0x1000>& road_word_nonzero_writes() const { return road_word_nonzero_writes_; }
    const std::array<std::uint16_t, 0x1000>& road_word_last_values() const { return road_word_last_values_; }
    const std::array<std::uint16_t, 0x1000>& road_word_min_values() const { return road_word_min_values_; }
    const std::array<std::uint16_t, 0x1000>& road_word_max_values() const { return road_word_max_values_; }
    void reset_cpu_control() { cpu_control_ = 0xff; }

    void debug_note(const std::string& text);
    void debug_watchdog_read(std::uint8_t value);
    void sub_debug_note(const std::string& text);
    void debug_sub_shared_read(std::uint32_t address, std::uint32_t value, unsigned size);
    void debug_sub_road_write(std::uint32_t address, std::uint32_t value, unsigned size);
    void debug_handshake_access(char op, BusSpace space, std::uint32_t address, std::uint32_t value, unsigned size);
    void note_dispatch(std::uint8_t command, std::uint32_t target, std::uint32_t pc);
    void debug_road_focus_access(char op, BusSpace space, std::uint32_t address, std::uint32_t value, unsigned size);
    void road_focus_note(const std::string& text);
    void debug_road_data_flow_access(char op, BusSpace space, std::uint32_t address, std::uint32_t value, unsigned size);
    void road_data_flow_note(const std::string& text);
    void debug_road_state_access(char op, BusSpace space, std::uint32_t address, std::uint32_t value, unsigned size);
    void road_state_note(const std::string& text);
    void debug_road_pending_access(char op, BusSpace space, std::uint32_t address, std::uint32_t value, unsigned size);
    void road_pending_note(const std::string& text);
    std::uint64_t watchdog_reads() const { return ioc_watchdog_reads_; }
    std::uint32_t trace_pc(BusSpace space) const { return space == BusSpace::Main ? pc_a_ : pc_b_; }

    // Research Workbench state.  Event capture is dormant unless explicitly enabled.
    bool research_enabled_ = false;
    bool research_internal_write_ = false;
    bool research_break_pending_ = false;
    std::string research_break_reason_;
    std::deque<ResearchEvent> research_events_;
    std::vector<ResearchWatch> research_watches_;
    std::vector<LivePatch> live_patches_;
    unsigned next_live_patch_id_ = 1;
    std::size_t research_event_limit_ = 4096;
    unsigned next_research_watch_id_ = 1;

    bool palette_trace_enabled_ = false;
    std::array<bool,4096> palette_trace_filter_{};
    std::deque<PaletteWriteEvent> palette_trace_events_;
    std::size_t palette_trace_limit_ = 4096;
    std::array<std::uint16_t,4096> palette_trace_pending_old_{};
    std::array<bool,4096> palette_trace_pending_valid_{};

    bool memory_trace_enabled_ = false;
    BusSpace memory_trace_space_ = BusSpace::Main;
    std::uint32_t memory_trace_address_ = 0;
    std::uint32_t memory_trace_length_ = 1;
    unsigned memory_trace_width_ = 0; // bytes; 0 = any
    bool memory_trace_reads_ = false;
    bool memory_trace_writes_ = true;
    std::deque<MemoryWriteTraceEvent> memory_trace_events_;
    std::size_t memory_trace_limit_ = 4096;
    std::uint64_t memory_trace_write_count_ = 0;
    std::function<void(const MemoryWriteTraceEvent&)> timeline_write_sink_;
    std::uint64_t timeline_write_count_ = 0;

    std::array<std::uint16_t, 4096> palette{};
    // v0.39: provenance for every TC0110PCR entry.  This is deliberately
    // maintained by the bus rather than inferred from the final palette so
    // diagnostics can distinguish reset/unwritten $0000 from a deliberate
    // game write of $0000.
    std::array<std::uint32_t, 4096> palette_write_count{};
    std::array<std::uint32_t, 4096> palette_first_write_frame{};
    std::array<std::uint32_t, 4096> palette_last_write_frame{};
    std::array<std::uint32_t, 4096> palette_last_write_pc{};
    std::array<std::uint16_t, 4096> palette_first_written_value{};
    std::array<std::uint16_t, 4096> palette_last_written_value{};
    std::vector<Region> regions;

private:
    std::function<void(char,std::uint32_t,std::uint32_t,unsigned)> native_steering_bus_observer_;
    std::function<void(char,std::uint32_t,std::uint32_t,unsigned)> native_health_bus_observer_;
    std::function<void(char,std::uint32_t,std::uint32_t,unsigned)> native_copy_bus_observer_;
    std::function<void(char,std::uint32_t,std::uint32_t,unsigned)> native_cpu_b_bus_observer_;
    std::function<void(char,std::uint32_t,std::uint32_t,unsigned)> native_cpu_b_irq_bus_observer_;
    std::function<void(char,std::uint32_t,std::uint32_t,unsigned)> native_cpu_b_pair_bus_observer_;
    std::function<void(char,std::uint32_t,std::uint32_t,unsigned)> cpu_b_original_irq_observer_;
    std::function<void(BusSpace,std::uint32_t,unsigned,std::uint8_t,std::uint8_t)> cpu_b_btst_writer_observer_;
    bool cpu_b_original_irq_phase_=false;
    bool native_cpu_b_irq_phase_=false;
    bool quiet_fast_forward_ = false;
    bool suppress_collision_shove_ = false;
    std::uint64_t suppressed_collision_shoves_ = 0;
    std::uint64_t suppressed_collision_lateral_ = 0;
    std::uint64_t suppressed_collision_speed_ = 0;
    std::deque<CollisionResponseEvent> collision_response_events_;
    std::uint64_t collision_response_sequence_ = 0;
    std::size_t collision_response_event_limit_ = 4096;
    std::deque<SoundCommandEvent> sound_command_events_;
    std::uint64_t sound_command_sequence_ = 0;
    std::size_t sound_command_event_limit_ = 4096;
    Region* region(std::uint32_t, BusSpace);
    const Region* region(std::uint32_t, BusSpace) const;
    std::uint8_t read_byte(std::uint32_t, BusSpace);
    void write_byte(std::uint32_t, std::uint8_t, BusSpace);
    void trace(char, std::uint32_t, std::uint32_t, unsigned, BusSpace);

    std::uint8_t ioc_portreg_r();
    void ioc_portreg_w(std::uint8_t value);
    void ioc_port_w(std::uint8_t value);
    std::uint8_t ioc_watchdog_r();

    std::uint16_t palette_address_ = 0, palette_latch_ = 0;
    std::uint8_t cpu_control_ = 0xff;

    std::uint8_t ioc_port_ = 0;
    std::uint8_t ioc_coin_output_ = 0;
    std::uint16_t ioc_steering_ = 0x0000;
    std::array<std::uint8_t, 16> ioc_last_write_{};
    std::array<std::uint8_t, 16> ioc_input_xor_{}; // persistent debugger/research input inversion
    std::array<std::uint8_t, 16> ioc_scenario_xor_{}; // per-frame CLI/scenario pulse layer (not checkpointed)
    unsigned ioc_debug_frame_ = 0;
    std::ofstream ioc_pulse_log_;
    std::array<std::uint64_t, 16> ioc_reads_{};
    std::array<std::uint64_t, 16> ioc_writes_{};
    std::uint64_t ioc_selector_writes_ = 0;
    std::uint64_t ioc_watchdog_reads_ = 0;

    std::uint32_t pc_a_ = 0, pc_b_ = 0;
    std::ofstream log_;
    std::uint64_t log_limit_ = 40000, logged_ = 0;
    std::ofstream boot_log_;
    std::uint64_t boot_log_limit_ = 10000, boot_logged_ = 0;
    std::ofstream sub_log_;
    std::ofstream handshake_log_;
    std::ofstream road_focus_log_;
    std::ofstream road_data_flow_log_;
    std::ofstream road_state_log_;
    std::ofstream road_pending_log_;
    std::uint64_t sub_log_limit_ = 20000, sub_logged_ = 0;
    std::uint64_t sub_shared_reads_ = 0, sub_road_writes_debug_ = 0;
    std::uint64_t handshake_log_limit_ = 30000, handshake_logged_ = 0;
    std::uint64_t road_focus_log_limit_ = 40000, road_focus_logged_ = 0;
    std::uint64_t road_data_flow_log_limit_ = 60000, road_data_flow_logged_ = 0;
    std::uint64_t road_state_log_limit_ = 30000, road_state_logged_ = 0;
    std::uint64_t road_pending_log_limit_ = 30000, road_pending_logged_ = 0;
    std::uint64_t pending_value_reads_ = 0, pending_value_writes_ = 0, pending_value_nonzero_writes_ = 0;
    std::uint64_t pending_flag_reads_ = 0, pending_flag_writes_ = 0, pending_flag_nonzero_writes_ = 0;
    std::uint64_t road_ctrl_reads_ = 0, road_ctrl_writes_ = 0, road_ctrl_nonzero_writes_ = 0;
    std::map<std::uint32_t, std::uint64_t> pending_value_read_pc_counts_, pending_value_write_pc_counts_;
    std::map<std::uint32_t, std::uint64_t> pending_flag_read_pc_counts_, pending_flag_write_pc_counts_;
    std::map<std::uint32_t, std::uint64_t> road_ctrl_read_pc_counts_, road_ctrl_write_pc_counts_;
    std::uint64_t road_flag_reads_ = 0, road_flag_writes_ = 0, road_flag_nonzero_writes_ = 0;
    std::map<std::uint32_t, std::uint64_t> road_flag_read_pc_counts_;
    std::map<std::uint32_t, std::uint64_t> road_flag_write_pc_counts_;
    std::uint64_t flow_source_reads_ = 0, flow_source_writes_ = 0, flow_road_writes_ = 0, flow_nonzero_road_writes_ = 0;
    std::map<std::uint32_t, std::uint64_t> flow_source_read_pc_counts_;
    std::map<std::uint32_t, std::uint64_t> flow_source_write_pc_counts_;
    std::map<std::uint32_t, std::uint64_t> flow_nonzero_road_pc_counts_;
    bool flow_first_nonzero_seen_ = false;
    std::uint64_t road_focus_reads_ = 0, road_focus_writes_ = 0, road_focus_road_writes_ = 0;
    std::map<std::uint32_t, std::uint64_t> road_focus_pc_hits_;
    std::map<std::uint32_t, std::uint64_t> road_focus_read_pc_counts_;
    std::uint64_t main_cmd_reads_ = 0, main_cmd_writes_ = 0, sub_cmd_reads_ = 0, sub_cmd_writes_ = 0;
    std::uint64_t main_10801a_reads_ = 0, main_10801a_writes_ = 0, sub_10801a_reads_ = 0, sub_10801a_writes_ = 0;
    std::array<std::uint64_t, 32> dispatch_counts_{};
    std::array<std::uint32_t, 32> dispatch_targets_{};
    std::map<std::uint32_t, std::uint64_t> road_write_pc_counts_;
    std::map<std::uint32_t, std::uint64_t> road_nonzero_pc_counts_;
    std::uint64_t unmapped_reads_ = 0, unmapped_writes_ = 0;
    std::array<std::uint64_t, 0x1000> road_word_writes_{};
    std::array<std::uint64_t, 0x1000> road_word_value_changes_{};
    std::array<std::uint64_t, 0x1000> road_word_nonzero_writes_{};
    std::array<std::uint16_t, 0x1000> road_word_last_values_{};
    std::array<std::uint16_t, 0x1000> road_word_min_values_{};
    std::array<std::uint16_t, 0x1000> road_word_max_values_{};
    std::array<bool, 0x1000> road_word_seen_{};
    bool all_ = false;
    GenericTraceCallback generic_trace_cb_;
    GenericTraceInterestCallback generic_trace_interest_cb_;
    GenericTraceOldValueCallback generic_trace_old_value_cb_;
};

class Runtime {
public:
    Runtime(Bytes main_rom, Bytes sub_rom);
    ~Runtime();
    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;

    void reset();
    int run(int cycles_per_cpu);
    int step_instruction(BusSpace space);
    std::uint32_t debug_get_register(BusSpace space, const std::string& name) const;
    bool debug_set_register(BusSpace space, const std::string& name, std::uint32_t value);

    void irq4();
    void summary(std::ostream&) const;
    void enable_boot_debug(bool enabled, unsigned sample_limit = 500);
    void open_boot_debug_log(const std::filesystem::path& path, std::uint64_t limit = 10000);
    void enable_sub_debug(bool enabled, unsigned sample_limit = 2000);
    void open_sub_debug_log(const std::filesystem::path& path, std::uint64_t limit = 20000);
    void open_handshake_log(const std::filesystem::path& path, std::uint64_t limit = 30000);
    void enable_road_focus_debug(bool enabled, unsigned sample_limit = 12000);
    void open_road_focus_log(const std::filesystem::path& path, std::uint64_t limit = 40000);
    void open_road_data_flow_log(const std::filesystem::path& path, std::uint64_t limit = 60000);
    void enable_road_state_debug(bool enabled, unsigned sample_limit = 12000) { road_state_debug_ = enabled; road_state_sample_limit_ = sample_limit; }
    void open_road_state_log(const std::filesystem::path& path, std::uint64_t limit = 30000);
    void open_road_pending_log(const std::filesystem::path& path, std::uint64_t limit = 30000);
    void set_force_road_flag(bool enabled) { force_road_flag_ = enabled; }
    void configure_generic_trace(const TraceConfig& config, const std::filesystem::path& path);
    void set_trace_frame(unsigned frame) { generic_trace_frame_ = frame; provenance_frame_ = frame; }
    // Opt-in census/bridge observation, never a replacement dispatch path.
    void enable_rom_execution_census() { rom_execution_census_.enable(); }
    // RC6.3.7: read-only, independent observed CPU-B instruction graph.
    void enable_cpu_b_edge_probe() { cpu_b_edge_enabled_ = true; }
    // RC6.3.10: guarded opt-in Ghidra BEQ.B CPU-B instruction only.
    void load_native_cpu_b_library(const std::filesystem::path& path);
    void enable_native_cpu_b_replacement(bool forced_fallback=false) {
        native_cpu_b_enabled_=true;native_cpu_b_fault_probe_=forced_fallback;
    }
    void enable_native_cpu_b_capture(const std::filesystem::path& path) {
        native_cpu_b_state_stream_.open(path,std::ios::binary|std::ios::trunc);
        if(!native_cpu_b_state_stream_) throw std::runtime_error("Cannot open native CPU-B state capture");
        native_cpu_b_bus_stream_.open(path.parent_path()/"native-cpu-b-bus.tsv",std::ios::binary|std::ios::trunc);
        if(!native_cpu_b_bus_stream_) throw std::runtime_error("Cannot open native CPU-B bus capture");
        native_cpu_b_irq_stream_.open(path.parent_path()/"native-cpu-b-irq.tsv",std::ios::binary|std::ios::trunc);
        if(!native_cpu_b_irq_stream_) throw std::runtime_error("Cannot open native CPU-B IRQ capture");
        bus.set_native_cpu_b_irq_bus_observer([this](char op,std::uint32_t address,std::uint32_t value,unsigned width){
            native_cpu_b_irq_stream_<<generic_trace_frame_<<'\t'<<op<<'\t'<<std::hex<<address
                                     <<'\t'<<value<<std::dec<<'\t'<<width<<'\n';
        });
        bus.set_native_cpu_b_bus_observer([this](char op,std::uint32_t address,std::uint32_t value,unsigned width){
            native_cpu_b_bus_stream_<<generic_trace_frame_<<'\t'<<op<<'\t'<<std::hex<<address
                                    <<'\t'<<value<<std::dec<<'\t'<<width<<'\n';
        });
    }
    // Separate from legacy single-BEQ capture, with independently audited TST R16.
    void load_native_cpu_b_pair_library(const std::filesystem::path& path);
    void enable_native_cpu_b_pair_replacement(bool forced_fallback=false) {
        native_cpu_b_pair_enabled_=true; native_cpu_b_pair_fault_probe_=forced_fallback;
    }
    void enable_native_cpu_b_pair_capture(const std::filesystem::path& path) {
        native_cpu_b_pair_state_stream_.open(path,std::ios::binary|std::ios::trunc);
        native_cpu_b_pair_bus_stream_.open(path.parent_path()/"native-cpu-b-pair-bus.tsv",std::ios::binary|std::ios::trunc);
        native_cpu_b_pair_irq_stream_.open(path.parent_path()/"native-cpu-b-pair-irq.tsv",std::ios::binary|std::ios::trunc);
        if(!native_cpu_b_pair_state_stream_||!native_cpu_b_pair_bus_stream_||!native_cpu_b_pair_irq_stream_)
            throw std::runtime_error("Cannot open CPU-B pair proof streams");
        bus.set_native_cpu_b_pair_bus_observer([this](char op,std::uint32_t a,std::uint32_t v,unsigned w){
            native_cpu_b_pair_bus_stream_<<generic_trace_frame_<<'\t'<<op<<'\t'<<std::hex<<a<<'\t'<<v<<std::dec<<'\t'<<w<<'\n';
        });
        bus.set_native_cpu_b_irq_bus_observer([this](char op,std::uint32_t a,std::uint32_t v,unsigned w){
            native_cpu_b_pair_irq_stream_<<generic_trace_frame_<<'\t'<<op<<'\t'<<std::hex<<a<<'\t'<<v<<std::dec<<'\t'<<w<<'\n';
        });
    }
    void enable_native_dispatch_audit(const std::filesystem::path& path) {
        native_dispatch_stream_.open(path, std::ios::binary | std::ios::trunc);
        if (!native_dispatch_stream_) throw std::runtime_error("Cannot open native dispatcher audit");
    }
    void enable_native_projection_shadow() { projection_shadow_.enable(); }
    void enable_native_steering_shadow() { native_steering_shadow_enabled_ = true; }
    // Experimental CPU-A MOVE.W instruction substitution: never enabled by default.
    // Load exactly the independently generated/compiled candidate DLL/SO.
    // Unavailable/invalid libraries fail closed before runtime starts.
    void load_native_steering_library(const std::filesystem::path& path);
    void enable_native_steering_replacement(bool fault_probe = false) {
        native_steering_shadow_enabled_ = true;
        native_steering_replace_enabled_ = true;
        native_steering_fault_probe_ = fault_probe;
    }
    void enable_native_steering_bus_capture(const std::filesystem::path& path) {
        native_steering_bus_stream_.open(path, std::ios::binary | std::ios::trunc);
        if (!native_steering_bus_stream_) throw std::runtime_error("Cannot open native steering bus capture");
        bus.set_native_steering_bus_observer([this](char op,std::uint32_t a,std::uint32_t v,unsigned size) {
            native_steering_bus_stream_ << generic_trace_frame_ << '\t' << op << '\t'
                << std::hex << a << '\t' << v << '\t' << std::dec << size << '\n';
        });
    }
    void enable_native_steering_state_capture(const std::filesystem::path& path) {
        native_steering_state_stream_.open(path, std::ios::binary | std::ios::trunc);
        if (!native_steering_state_stream_) throw std::runtime_error("Cannot open native steering state capture");
    }
    // RC6.3.6: independent Ghidra-derived, opt-in CPU-A 16-longword copy block.
    void load_native_copy_library(const std::filesystem::path& path);
    void enable_native_copy_replacement(bool fault=false) {native_copy_enabled_=true;native_copy_fault_=fault;}
    void enable_native_copy_capture(const std::filesystem::path& path) {
        native_copy_state_stream_.open(path,std::ios::binary|std::ios::trunc);
        if(!native_copy_state_stream_)throw std::runtime_error("Cannot open native copy return-state trace");
        native_copy_bus_stream_.open(path.parent_path()/"native-copy-bus.tsv",std::ios::binary|std::ios::trunc);
        if(!native_copy_bus_stream_)throw std::runtime_error("Cannot open native copy bus trace");
        bus.set_native_copy_bus_observer([this](char op,std::uint32_t a,std::uint32_t v,unsigned width){
            native_copy_bus_stream_<<generic_trace_frame_<<'\t'<<op<<'\t'<<std::hex<<a<<'\t'<<v<<'\t'<<std::dec<<width<<'\n';
        });
    }
    void load_native_health_library(const std::filesystem::path& path);
    void set_native_health_case(const std::string& name) {
        if (name!="natural" && name!="bypass" && name!="nonterminal" && name!="terminal" && name!="overflow")
            throw std::invalid_argument("unsupported --native-health-case (natural|bypass|nonterminal|terminal|overflow)");
        native_health_case_=name;
    }

    void enable_native_health_replacement(bool fault_probe=false) {
        native_health_enabled_=true; native_health_fault_probe_=fault_probe;
    }
    void enable_native_health_state_capture(const std::filesystem::path& path) {
        native_health_stream_.open(path,std::ios::binary|std::ios::trunc);
        if(!native_health_stream_)throw std::runtime_error("Cannot open native health state trace");
        native_health_bus_stream_.open(path.parent_path()/"native-health-bus.tsv",std::ios::binary|std::ios::trunc);
        if(!native_health_bus_stream_)throw std::runtime_error("Cannot open native health bus trace");
        native_health_path_stream_.open(path.parent_path()/"native-health-path.tsv",std::ios::binary|std::ios::trunc);
        if(!native_health_path_stream_)throw std::runtime_error("Cannot open native health path trace");
        native_health_caller_stream_.open(path.parent_path()/"native-health-caller-path.tsv",std::ios::binary|std::ios::trunc);
        if(!native_health_caller_stream_)throw std::runtime_error("Cannot open native health dynamic caller path trace");
        native_health_entry_stream_.open(path.parent_path()/"native-health-entry.tsv",std::ios::binary|std::ios::trunc);
        if(!native_health_entry_stream_)throw std::runtime_error("Cannot open native health entry provenance trace");
        native_health_seed_stream_.open(path.parent_path()/"native-health-seed.tsv",std::ios::binary|std::ios::trunc);
        if(!native_health_seed_stream_)throw std::runtime_error("Cannot open native health controlled-entry trace");
        bus.set_native_health_bus_observer([this](char op,std::uint32_t a,std::uint32_t v,unsigned size){
            native_health_bus_stream_<<generic_trace_frame_<<'\t'<<op<<'\t'
                <<std::hex<<a<<'\t'<<v<<'\t'<<std::dec<<size<<'\n';
        });
    }
    void export_native_endstate(const std::filesystem::path& path) const;
    void observe_native_entry(BusSpace space, std::uint32_t pc, const std::string& label) {
        native_execution_bridge_.observe_entry(space == BusSpace::Sub, pc, label);
    }
    void write_native_execution_evidence(const std::filesystem::path& directory,
                                         const std::string& scenario) const;
    void set_quiet_fast_forward(bool enabled) { quiet_fast_forward_ = enabled; bus.set_quiet_fast_forward(enabled); }
    void configure_provenance(const ProvenanceConfig& config, const std::filesystem::path& directory);
    void write_provenance_reports();
    void configure_forensic_history(unsigned max_entries) { forensic_history_limit_ = max_entries; forensic_history_.clear(); }
    void write_forensic_history(const std::filesystem::path& path) const;
    void save_checkpoint(const std::filesystem::path& path, unsigned frame) const;
    unsigned load_checkpoint(const std::filesystem::path& path);

    // v0.59.3: authoritative instruction-time handling/cornering snapshot + optional coefficient override.
    // Captured from the proven $008Axx speed/turn coefficient path.
    struct HandlingSnapshot {
        bool valid = false;
        std::int16_t turn_state = 0;
        std::int16_t table_index = 0;
        std::uint16_t speed_internal = 0;
        std::uint16_t forward_coeff = 0x0100;          // original table value
        std::uint16_t lateral_coeff = 0;               // original table value
        std::uint16_t applied_forward_coeff = 0x0100;  // after optional override
        std::uint16_t applied_lateral_coeff = 0;       // after optional override
        std::int32_t forward_component = 0;
        std::int32_t lateral_component = 0;
        bool override_active = false;
        std::uint64_t samples = 0;
    };
    const HandlingSnapshot& handling_snapshot() const { return handling_snapshot_; }
    void configure_handling_override(double cornering_scale, double speed_retain) {
        handling_cornering_scale_ = cornering_scale; handling_speed_retain_ = speed_retain;
    }
    double handling_cornering_scale() const { return handling_cornering_scale_; }
    double handling_speed_retain() const { return handling_speed_retain_; }
    // RC2.9: causally-proven Stage-1 special-target one-hit research control.
    // When enabled, the instruction hook arms 0x1002AE to zero immediately before
    // authentic damage PC 0xA112; the original decrement/flags/terminal path executes.
    void set_target_one_hit(bool enabled) { target_one_hit_ = enabled; }
    bool target_one_hit() const { return target_one_hit_; }
    std::uint64_t target_one_hit_arms() const { return target_one_hit_arms_; }

    enum class BeaconOverrideMode { Authentic, ForceOn, ForceOff };
    void set_beacon_override(BeaconOverrideMode mode);
    BeaconOverrideMode beacon_override() const { return beacon_override_mode_; }
    const char* beacon_override_name() const;
    // v0.64.0: safe read-only disassembly surface for the Research API.
    std::string disassemble(BusSpace space, std::uint32_t pc, unsigned* size_out = nullptr) const { return disassemble_space(space, pc, size_out); }

    Bus bus;
    std::uint64_t cycles_a = 0, cycles_b = 0;
    std::uint64_t instructions_a = 0, instructions_b = 0;
    std::string fault;

private:
    RomExecutionCensus rom_execution_census_;
    bool cpu_b_edge_enabled_ = false, cpu_b_edge_have_previous_ = false;
    std::uint32_t cpu_b_edge_previous_ = 0;
    // Key is (previous authentic instruction PC, next authentic instruction PC).
    std::map<std::pair<std::uint32_t,std::uint32_t>, std::uint64_t> cpu_b_edges_;
    std::map<std::uint32_t,std::uint16_t> cpu_b_instruction_words_;
    std::uint64_t cpu_b_edge_transitions_ = 0;
    // RC6.3.22: original-ROM instruction-hook chronology. Bounded, opt-in,
    // read-only. This is a *selected-PC* projection, never a full trace.
    struct CpuBOrderedHook { std::uint64_t ordinal, cycles; std::uint32_t frame, pc, sp, sr; std::uint16_t opcode; };
    std::vector<CpuBOrderedHook> cpu_b_ordered_hooks_;
    std::uint64_t cpu_b_ordered_dropped_ = 0;
    // RC6.3.28: independent instruction-entry samples of both sides of
    // RC6.3.30: read-only original-ROM CPU-B BTST.B #7,($100802).L
    // source byte on entry, immediately followed by the genuine 0x0516 SR.
    // This is a non-mutating memory peek, not a bus access trace or a patch.
    struct CpuB050eSource {
        std::uint64_t ordinal=0, frame=0, cycles=0, nextOrdinal=0, nextFrame=0, nextCycles=0;
        std::uint32_t sourceByte=0, nextPC=0, nextSR=0;
    };
    // Stores *actual* Bus::write events in a bounded window; zero recorded
    // writers is a valid, explicitly unresolved writer-provenance result.
    struct CpuB100802Writer { std::uint64_t ordinal, frame, cycles;
        std::uint32_t pc, address, width, before, after; char cpu; };
    std::vector<CpuB100802Writer> cpu_b_100802_writers_;
    std::uint64_t cpu_b_100802_hits_=0, cpu_b_100802_dropped_=0;
    std::vector<CpuB050eSource> cpu_b_050e_sources_;
    CpuB050eSource cpu_b_050e_pending_context_{};
    std::uint64_t cpu_b_050e_hits_=0, cpu_b_050e_dropped_=0;
    bool cpu_b_050e_pending_=false;
    // CPU-B BNE.B at 0x0516. Original-ROM only, no flags or PC modification.
    struct CpuB0516Branch {
        std::uint64_t ordinal=0, cycles=0, nextCycles=0;
        std::uint32_t frame=0, nextFrame=0, sr=0, nextPc=0;
    };
    std::vector<CpuB0516Branch> cpu_b_0516_branches_;
    CpuB0516Branch cpu_b_0516_pending_context_{};
    std::uint64_t cpu_b_0516_hits_=0, cpu_b_0516_dropped_=0;
    bool cpu_b_0516_pending_=false;

    // RC6.3.26: read-only six-byte exception-frame RAM snapshots at selected
    // original-ROM instruction hooks, plus the immediately succeeding CPU-B
    // hook after RTE. This is NOT bus order, vector ACK or all IRQ delivery.
    struct CpuBExceptionFrameHook {
        std::uint64_t ordinal;
        std::uint32_t frame, pc, sp, usp, isp, sr, savedSr, savedPc;
        const char* kind;
    };
    std::vector<CpuBExceptionFrameHook> cpu_b_exception_frame_hooks_;
    std::uint64_t cpu_b_exception_frame_dropped_ = 0;
    bool cpu_b_exception_rte_pending_ = false;
    // RC6.3.27: ACK callback -> original Bus::read/write -> first handler hook.
    // Read-only evidence; no inferred events, never a hardware timing proof.
    struct CpuBIRQBusEvent {
        std::uint64_t sample;
        std::uint32_t frame, address, value, width;
        char kind;
    };
    std::vector<CpuBIRQBusEvent> cpu_b_irq_bus_events_;
    std::uint64_t cpu_b_irq_bus_dropped_ = 0, cpu_b_irq_bus_samples_ = 0;
    bool cpu_b_irq_bus_pending_ = false;
    void record_cpu_b_irq_bus(char kind,std::uint32_t addr,std::uint32_t value,unsigned width);


    std::ofstream native_dispatch_stream_;
    bool native_cpu_b_enabled_=false, native_cpu_b_fault_probe_=false;
    void* native_cpu_b_handle_=nullptr;
    NativeCpuBBranchFunction native_cpu_b_function_=nullptr;
    std::ofstream native_cpu_b_state_stream_,native_cpu_b_bus_stream_,native_cpu_b_irq_stream_;
    bool native_cpu_b_pending_=false;
    std::uint32_t native_cpu_b_entry_frame_=0;
    std::uint64_t native_cpu_b_attempts_=0,native_cpu_b_commits_=0,native_cpu_b_rejected_=0,native_cpu_b_rollbacks_=0;
    std::uint64_t native_cpu_b_taken_=0,native_cpu_b_not_taken_=0,native_cpu_b_irq_edges_=0,native_cpu_b_timeslice_guards_=0;
    std::string native_cpu_b_reason_;
    bool native_cpu_b_pair_enabled_=false,native_cpu_b_pair_fault_probe_=false;
    bool native_cpu_b_pair_pending_=false,native_cpu_b_pair_saw_beq_=false;
    std::uint32_t native_cpu_b_pair_entry_frame_=0;
    std::uint64_t native_cpu_b_pair_attempts_=0,native_cpu_b_pair_commits_=0,native_cpu_b_pair_rejected_=0,
                  native_cpu_b_pair_rollbacks_=0,native_cpu_b_pair_timeslice_guards_=0,
                  native_cpu_b_pair_taken_=0,native_cpu_b_pair_not_taken_=0;
    std::string native_cpu_b_pair_reason_;
    void* native_cpu_b_pair_handle_=nullptr;
    NativeCpuBPairFunction native_cpu_b_pair_function_=nullptr;
    std::ofstream native_cpu_b_pair_state_stream_,native_cpu_b_pair_bus_stream_,native_cpu_b_pair_irq_stream_;
    void process_native_cpu_b_pair(unsigned pc);
    void record_native_cpu_b_pair_exit(unsigned nextPC);

    void process_native_cpu_b_branch(std::uint32_t pc);
    void record_native_cpu_b_exit(std::uint32_t nextPC);
    struct NativeDispatchResult {bool health_committed=false, copy_committed=false, cpu_b_committed=false;};
    NativeDispatchResult dispatch_generated_candidates(std::uint32_t pc);
    NativeExecutionBridge native_execution_bridge_;
    LiveCandidateShadow live_candidate_shadow_;
    // This is intentionally narrower than full sprite transfer: the original
    // CPU executes the bulk copier and final RTS, native can skip only the
    // established trailing clear DBRA loop. Disabled unless explicitly armed.
    enum class NativeClearMode { Off, Compare, Native };
    NativeClearMode native_clear_mode_ = NativeClearMode::Off;
    bool native_clear_disabled_ = false;
    std::uint64_t native_clear_attempts_ = 0, native_clear_guards_passed_ = 0,
                  native_clear_rejected_ = 0, native_clear_committed_ = 0,
                  native_clear_verified_ = 0, native_clear_postcheck_fail_ = 0;
    std::string native_clear_last_rejection_;
    // RC5.3: real instruction-time guard diagnostics for rejected substitutions.
    std::uint32_t native_clear_last_observed_a1_=0,native_clear_last_expected_a1_=0;
    std::uint32_t native_clear_last_entry_stage_=0,native_clear_last_current_stage_=0;
    std::uint16_t native_clear_last_entry_records_=0,native_clear_last_current_records_=0;
    std::int16_t native_clear_last_clear_dbra_=-1;
    std::uint16_t native_clear_last_rom_displacement_=0;
    std::uint64_t native_clear_rollback_=0;
    void process_native_clear_instruction(unsigned int pc);
    std::string native_clear_report_json() const;
    ProjectionShadow projection_shadow_;
    bool native_copy_enabled_=false, native_copy_disabled_=false, native_copy_fault_=false, native_copy_pending_=false;
    void* native_copy_handle_=nullptr;
    NativeCopyFunction native_copy_function_=nullptr;
    std::uint64_t native_copy_attempts_=0,native_copy_commits_=0,native_copy_rejected_=0,native_copy_rollbacks_=0;
    std::string native_copy_reason_;
    std::ofstream native_copy_state_stream_,native_copy_bus_stream_;
    void process_native_copy_block(unsigned pc);
    void record_native_copy_state(const char* mode);
    bool native_health_enabled_=false;
    bool native_health_disabled_=false;
    bool native_health_fault_probe_=false;
    void* native_health_handle_=nullptr;
    NativeHealthFunction native_health_function_=nullptr;
    std::uint64_t native_health_attempts_=0, native_health_commits_=0,
                  native_health_rejections_=0, native_health_rollbacks_=0;
    std::string native_health_reason_;
    std::ofstream native_health_stream_;
    std::ofstream native_health_bus_stream_;
    std::ofstream native_health_path_stream_;
    std::ofstream native_health_seed_stream_;
    std::ofstream native_health_entry_stream_;
    std::ofstream native_health_caller_stream_;
    std::deque<std::uint32_t> native_health_cpu_a_history_;
    void record_native_health_caller_path(std::uint32_t ret, std::uint32_t entry);
    std::string native_health_case_="natural";
    bool native_health_case_applied_=false;
    std::vector<std::uint32_t> native_health_path_;
    void apply_native_health_entry_case();
    void record_native_health_path(const char* mode);
    bool native_health_pending_=false;
    std::uint32_t native_health_return_pc_=0;
    void process_native_health_function(unsigned pc);
    void record_native_health_return(const char* mode);
    bool native_steering_shadow_enabled_ = false;
    bool native_steering_replace_enabled_ = false;
    void* native_steering_library_handle_=nullptr;
    using NativeSteeringFn=std::int16_t (*)(std::int16_t);
    NativeSteeringFn native_steering_generated_fn_=nullptr;

    bool native_steering_replace_disabled_ = false;
    bool native_steering_fault_probe_ = false;
    std::uint64_t native_steering_attempts_ = 0, native_steering_commits_ = 0,
                  native_steering_guards_rejected_ = 0, native_steering_rollbacks_ = 0;
    std::string native_steering_reject_reason_;
    std::ofstream native_steering_state_stream_;
    std::ofstream native_steering_bus_stream_;
    void process_native_steering_instruction(unsigned pc);
    void complete_native_steering_observation();
    void record_native_steering_state(unsigned pc);
    bool steering_shadow_pending_ = false;
    std::int16_t steering_shadow_expected_ = 0;
    std::int16_t steering_shadow_input_ = 0;
    unsigned steering_shadow_entry_frame_ = 0;
    std::uint64_t steering_shadow_entries_ = 0, steering_shadow_compared_ = 0,
                  steering_shadow_matches_ = 0, steering_shadow_mismatches_ = 0,
                  steering_shadow_incomplete_ = 0, steering_shadow_bypass_ = 0;
    std::string steering_shadow_first_mismatch_;
    std::string native_steering_shadow_report_json() const;
    bool quiet_fast_forward_ = false;
    HandlingSnapshot handling_snapshot_{};
    double handling_cornering_scale_ = 1.0;
    double handling_speed_retain_ = 1.0;
    bool target_one_hit_ = false;
    std::uint64_t target_one_hit_arms_ = 0;
    BeaconOverrideMode beacon_override_mode_ = BeaconOverrideMode::Authentic;
    unsigned beacon_override_patch_id_ = 0;
    unsigned beacon_request_patch_id_ = 0;
    void initialise_context(BusSpace space, std::vector<std::uint8_t>& context);
    int execute_slice(BusSpace space, int budget);
    void sync_sub_reset();
    void reset_sub_context();
    void set_irq(BusSpace space, unsigned level);
    void instruction_hook(unsigned int pc);
    void interrupt_ack(int level);
    std::string disassemble_space(BusSpace space, std::uint32_t pc, unsigned* size_out = nullptr) const;
    void log_loop_sample(std::uint32_t pc);
    void log_sub_sample(std::uint32_t pc);
    void log_sub_handler_sample(std::uint32_t pc);
    void log_road_focus_sample(std::uint32_t pc);
    void dump_sub_dispatch_table();
    void log_road_state_sample(std::uint32_t pc);
    void generic_trace_instruction(std::uint32_t pc);
    void generic_trace_memory(char op, BusSpace space, std::uint32_t address, std::uint32_t value,
                              unsigned size, std::uint32_t old_value, bool has_old, const char* region);
    void generic_trace_line(const std::string& line);
    struct ProvenanceCallFrame {
        std::uint32_t caller_pc = 0;
        std::uint32_t function_pc = 0;
        std::uint32_t return_pc = 0;
    };
    struct ProvenanceCpuState {
        std::vector<ProvenanceCallFrame> stack;
        std::uint32_t prev_pc = 0xffffffffu;
        unsigned prev_size = 0;
        bool prev_call = false;
        bool prev_return = false;
        std::uint32_t current_function = 0;
    };
    struct FunctionProfile {
        std::uint64_t instructions = 0;
        std::uint64_t reads = 0;
        std::uint64_t writes = 0;
        std::uint64_t calls = 0;
    };
    void provenance_instruction(std::uint32_t pc);
    void provenance_memory(char op, BusSpace space, std::uint32_t address, std::uint32_t value, unsigned size, const char* region);
    bool provenance_interested(char op, BusSpace space, std::uint32_t address, unsigned size) const;
    std::string provenance_stack_string(BusSpace space) const;


    std::vector<std::uint8_t> context_a_;
    std::vector<std::uint8_t> context_b_;
    bool sub_was_enabled_ = true;
    static constexpr int scheduler_slices_ = 16;

    bool boot_debug_ = false;
    unsigned boot_sample_limit_ = 500;
    unsigned boot_samples_ = 0;
    std::uint32_t previous_debug_pc_ = 0xffffffffu;
    unsigned previous_debug_size_ = 0;
    std::uint64_t irq4_requests_a_ = 0, irq4_requests_b_ = 0, irq_acks_ = 0;
    std::uint64_t loop_entries_ = 0;
    std::array<std::uint64_t, 0x21> loop_pc_hits_{}; // 0x580..0x5c0, even PCs

    bool sub_debug_ = true;
    unsigned sub_sample_limit_ = 2000;
    unsigned sub_samples_ = 0;
    std::uint32_t previous_sub_debug_pc_ = 0xffffffffu;
    unsigned previous_sub_debug_size_ = 0;
    std::uint64_t sub_loop_entries_ = 0;
    std::array<std::uint64_t, 0x71> sub_pc_hits_{}; // 0x440..0x520, even PCs
    unsigned sub_handler_samples_ = 0;
    std::uint64_t sub_handler_entries_ = 0;
    std::map<std::uint32_t, std::uint64_t> sub_handler_hot_;
    bool handshake_debug_ = true;
    bool road_focus_debug_ = true;
    bool road_flow_debug_ = true;
    bool road_state_debug_ = true;
    bool force_road_flag_ = false;
    bool force_road_flag_applied_ = false;
    unsigned road_state_samples_ = 0;
    unsigned road_state_sample_limit_ = 12000;
    std::map<std::uint32_t, std::uint64_t> road_state_hot_;
    std::uint64_t cmd2_entries_ = 0;
    unsigned road_focus_sample_limit_ = 12000;
    unsigned road_focus_samples_ = 0;
    std::uint64_t road_focus_entries_ = 0;
    std::map<std::uint32_t, std::uint64_t> road_focus_hot_;
    std::uint64_t pc_9c8_hits_ = 0, pc_9ca_hits_ = 0;

    struct TraceSnapshot {
        BusSpace space = BusSpace::Main;
        std::uint32_t pc = 0;
        std::uint32_t sr = 0;
        std::array<std::uint32_t, 16> regs{};
    };
    TraceConfig generic_trace_config_{};
    std::ofstream generic_trace_log_;
    std::uint64_t generic_trace_lines_ = 0;
    std::uint64_t generic_trace_mem_events_ = 0;
    std::uint64_t generic_trace_pc_events_ = 0;
    std::uint64_t generic_trace_triggers_ = 0;
    std::deque<TraceHistoryEntry> generic_trace_history_;
    std::deque<TraceHistoryEntry> forensic_history_;
    unsigned forensic_history_limit_ = 0;
    std::vector<bool> generic_trigger_fired_;
    unsigned generic_after_remaining_ = 0;
    bool generic_trace_limit_noted_ = false;
    unsigned generic_trace_frame_ = 0;
    bool generic_trace_armed_ = true;
    ProvenanceConfig provenance_config_{};
    unsigned provenance_frame_ = 0;
    std::filesystem::path provenance_dir_;
    std::ofstream provenance_flow_csv_;
    std::ofstream provenance_stack_log_;
    std::uint64_t provenance_events_ = 0;
    bool provenance_limit_noted_ = false;
    ProvenanceCpuState provenance_cpu_a_{};
    ProvenanceCpuState provenance_cpu_b_{};
    std::map<std::pair<char,std::pair<std::uint32_t,std::uint32_t>>, std::uint64_t> provenance_edges_;
    std::map<std::pair<char,std::uint32_t>, FunctionProfile> provenance_functions_;


    bool generic_trace_frame_active() const {
        return generic_trace_frame_ >= generic_trace_config_.from_frame &&
               generic_trace_frame_ <= generic_trace_config_.to_frame;
    }
    bool generic_task_slot_selected(std::uint32_t slot) const;


    friend void ::runtime_instruction_hook(unsigned int);
    friend int ::runtime_interrupt_ack(int);
};


struct SteeringEvent {
    unsigned start_frame = 0;
    unsigned end_frame = 0; // inclusive
    std::uint16_t value = 0;
};

struct InputPulse {
    std::uint8_t port = 0;
    std::uint8_t mask = 0;
    unsigned start_frame = 0;
    unsigned duration_frames = 3;
};

// v0.45: deterministic repeated stimulus for accelerator/brake/shift/steering experiments.
struct PeriodicInputPulse {
    std::uint8_t port = 0;
    std::uint8_t mask = 0;
    unsigned start_frame = 0;
    unsigned every_frames = 100;
    unsigned duration_frames = 3;
    unsigned count = 0; // 0 = repeat until run ends
};

enum class PatchWidth : unsigned { Byte = 1, Word = 2, Long = 4 };
struct MemoryPatch {
    BusSpace space = BusSpace::Main;
    PatchWidth width = PatchWidth::Byte;
    std::uint32_t address = 0;
    std::uint32_t value = 0;
    unsigned frame = 0;
    bool conditional = false;
    std::uint32_t expected = 0;
    bool applied = false;
};


struct GraphicsTraceRange {
    unsigned start = 0;
    unsigned end = 0;
};

struct GraphicsTraceRegion {
    int x1 = 0, y1 = 0, x2 = 0, y2 = 0;
};

struct GraphicsTraceConfig {
    std::vector<unsigned> sprite_slots;
    std::vector<std::pair<unsigned,unsigned>> sprite_pairs;
    std::vector<unsigned> palette_banks;
    std::vector<unsigned> palette_entries;
    std::vector<unsigned> pens;
    std::vector<GraphicsTraceRange> prom_addr_ranges;
    std::optional<unsigned> prom_value;
    std::vector<unsigned> priority_classes;
    std::vector<std::pair<int,int>> pixels;
    std::vector<GraphicsTraceRegion> regions;
    std::vector<std::string> layers;
    std::optional<unsigned> road_priority;
    bool zero_palette_use = false;
    bool anomalies_only = false;
    // v0.43: explain the complete sprite/palette/priority decision for selected
    // pixels. If a pair is supplied, overlapping pixels are auto-selected.
    bool pixel_provenance = false;
    std::optional<std::pair<unsigned,unsigned>> pixel_provenance_pair;
    unsigned pixel_provenance_max = 32;
    // v0.44: distinguish user-selected pixels/regions from preset defaults.
    bool explicit_location = false;
    // Controlled renderer experiments. These never change the default renderer.
    std::optional<unsigned> experiment_zero_slot;
    std::optional<unsigned> experiment_zero_palette_bank;
    std::optional<std::pair<unsigned,unsigned>> experiment_sprite_pair_order;
    std::vector<std::string> experiment_layer_order;
    std::string experiment_sprite_order = "descending";
    bool experiment_layer_matrix = false;
    // v0.46: parameterised reference sprite primask experiments.
    std::optional<unsigned> experiment_sprite_mask_prio0;
    std::optional<unsigned> experiment_sprite_mask_prio1;
    std::vector<unsigned> experiment_sprite_mask_matrix;
    unsigned from_frame = 0;
    unsigned to_frame = 0xffffffffu;
    std::uint64_t max_lines = 250000;

    bool enabled() const {
        return !sprite_slots.empty() || !sprite_pairs.empty() || !palette_banks.empty() ||
               !palette_entries.empty() || !pens.empty() || !prom_addr_ranges.empty() ||
               prom_value.has_value() || !priority_classes.empty() || !pixels.empty() ||
               !regions.empty() || !layers.empty() || road_priority.has_value() ||
               zero_palette_use || anomalies_only || pixel_provenance || pixel_provenance_pair.has_value() ||
               experiment_zero_slot.has_value() || experiment_zero_palette_bank.has_value() ||
               experiment_sprite_pair_order.has_value() || !experiment_layer_order.empty() ||
               experiment_layer_matrix || experiment_sprite_mask_prio0.has_value() ||
               experiment_sprite_mask_prio1.has_value() || !experiment_sprite_mask_matrix.empty() ||
               experiment_sprite_order != "descending" ||
               from_frame != 0 || to_frame != 0xffffffffu;
    }
};

struct DebugEventTrace {
    std::vector<std::string> events;
    unsigned from_frame = 0;
    unsigned to_frame = 0xffffffffu;
    std::uint64_t max_lines = 250000;
    bool enabled() const { return !events.empty(); }
};

struct TransitionTrigger {
    BusSpace space = BusSpace::Main;
    PatchWidth width = PatchWidth::Byte;
    std::uint32_t address = 0;
    bool on_change = false;
    std::optional<std::uint32_t> value;
    std::uint32_t last_value = 0;
    bool initialised = false;
    bool fired = false;
    std::string label;
};

struct SaveCheckpointRequest {
    unsigned frame = 0;
    std::filesystem::path path;
    bool saved = false;
};

struct MemoryWatch {
    BusSpace space = BusSpace::Main;
    PatchWidth width = PatchWidth::Byte;
    std::uint32_t address = 0;
    std::string label;
    std::uint32_t last_value = 0;
    bool initialised = false;
};

struct Options {
    std::filesystem::path roms = std::filesystem::path(CHASEHQ_PROJECT_DIR) / "roms/chasehq";
    std::filesystem::path content_pack;
    std::vector<std::filesystem::path> content_overlays;
    bool rom_census = false;
    bool cpu_b_edge_probe = false;
    bool native_dispatch_audit = false;
    bool native_cpu_b_replace = false, native_cpu_b_fault_probe = false, native_cpu_b_state_capture = false;
    bool native_cpu_b_pair_replace = false, native_cpu_b_pair_fault_probe = false, native_cpu_b_pair_state_capture = false;
    std::filesystem::path native_cpu_b_pair_library;
    std::filesystem::path native_cpu_b_library;
    bool native_projection_shadow = false;
    bool native_steering_shadow = false;
    bool native_steering_replace = false;
    std::filesystem::path native_steering_library;
    bool native_steering_rollback_probe = false;
    bool native_steering_state_capture = false;
    bool native_copy_replace = false;
    bool native_copy_fault_probe = false;
    std::filesystem::path native_copy_library;
    bool native_copy_state_capture = false;
    bool native_health_replace = false;
    bool native_health_fault_probe = false;
    std::filesystem::path native_health_library;
    bool native_health_state_capture = false;
    std::string native_health_case = "natural";
    std::vector<std::string> native_observe_entries;
    std::filesystem::path logs = "logs";
    bool logs_explicit = false;
    unsigned frames = 120;
    bool frames_explicit = false; // SDL frontend runs continuously by default unless a bound/scenario is requested
    unsigned loop_samples = 500;
    unsigned sub_samples = 2000;
    unsigned road_samples = 12000;
    unsigned flow_samples = 30000;
    unsigned state_samples = 12000;
    unsigned pending_samples = 30000;
    bool irq = false, all = false, scene_only = false, help = false, boot_debug = false, sub_debug = false, handshake_debug = false, road_focus_debug = false, road_flow_debug = false, road_state_debug = false, road_pending_debug = false, force_road_flag = false;
    TraceConfig trace;
    std::filesystem::path trace_config_file;
    GraphicsTraceConfig graphics_trace;
    std::filesystem::path graphics_trace_config_file;
    std::vector<InputPulse> input_pulses;
    std::vector<PeriodicInputPulse> periodic_input_pulses;
    std::optional<std::uint16_t> steering_fixed;
    std::vector<SteeringEvent> steering_events;
    bool gameplay_state_log = false;
    bool target_state_log = false; // v0.59 target/pursuit + collision telemetry
    bool target_one_hit = false; // RC2.9 special-target research cheat; default OFF
    bool no_collisions = false;    // v0.59.1 proven lateral + speed collision-response suppression
    bool course_data_log = false; // export banked course source + live course position/channel state
    // v0.55 course-survey assists. These are explicit debugger interventions and
    // must never be confused with native game behaviour in evidence reports.
    bool course_survey = false; // convenience mode: accel + follow + infinite time + auto turbo
    bool course_follow = false;
    int course_follow_steer = 32; // signed IOC steering magnitude used for non-zero curvature
    int course_follow_deadzone = 7; // abs(ch0) <= this is treated as straight
    int course_follow_lookahead = 8; // records ahead included in steering decision
    int course_follow_recovery_steer = 48; // legacy v0.56 option retained for CLI compatibility; closed-loop follower does not use blind recovery
    double course_follow_lateral_kp = 0.0060; // signed steering units per lateral-error unit
    int course_follow_lateral_max = 48; // maximum absolute centring correction
    int course_follow_lateral_deadzone = 96; // ignore small centre error to avoid hunting
    int course_follow_lateral_bias = 0; // live mapping target offset from geometric road centre, signed 16-bit course-lateral units
    std::string course_follow_controller = "hybrid"; // hybrid|predictive|legacy|profile
    double course_follow_lateral_kd = 0.012; // derivative damping gain
    int course_follow_slew = 8; // max steering-output change per frame
    bool course_follow_speed_control = false; // predictive mapping driver: lift/brake for severe upcoming curvature
    std::filesystem::path course_profile_file; // v0.59.3 replay driver_profile.csv as course-position feed-forward
    double cornering_scale = 1.0;              // scale lateral handling-table coefficient after ROM load
    double cornering_speed_retain = 1.0;       // scale forward handling-table coefficient after ROM load
    bool infinite_time = false;
    bool unlimited_turbo = false;
    bool auto_turbo = false;
    std::vector<MemoryPatch> memory_patches;
    std::vector<MemoryWatch> memory_watches;
    std::vector<TransitionTrigger> transition_triggers;
    std::vector<int> capture_relative_offsets;
    std::vector<int> capture_after_patch_offsets;
    std::vector<SaveCheckpointRequest> save_checkpoints;
    std::filesystem::path load_checkpoint;
    std::filesystem::path checkpoint_dir = "checkpoints"; // v0.49.1 persistent UI state slots
    unsigned checkpoint_slot = 0; // selected UI state slot 0-9
    std::string diagnostics_profile = "auto";
    std::string investigate_profile;
    std::string scenario;
    bool scenario_list = false;
    std::string scenario_describe;
    std::vector<unsigned> capture_frames;
    bool debug_everything = false;
    bool debug_api = true; // v0.60.1 localhost-only live control plane
    unsigned debug_api_port = 37600;
    bool input_trace = false;
    bool write_run_manifest = true;
    DebugEventTrace event_trace;
    ProvenanceConfig provenance;
    unsigned fast_forward_to = 0;
    // v0.45 fast-forward responsiveness/progress.
    unsigned fast_forward_status_every = 120;
    unsigned progress_status_every = 300; // v0.59.1 bounded-run progress/ETA; 0 disables
    unsigned fast_forward_event_every = 8;
    unsigned fast_forward_render_every = 0;
    // v0.43 forensic frame breakpoints. Each break emits a hardware capture,
    // graphics diagnostics and recent execution history; it may stop, wait, or continue.
    std::vector<unsigned> break_frames;
    bool break_frame_continue = false;
    bool break_frame_wait = false;
    unsigned break_history = 2048;
    // v0.43 self-contained diagnostic bundle packaging.
    bool auto_zip_logs = false;
    bool isolated_log_run = true; // v0.45: avoid stale artifacts entering bundles
    bool zip_delete_source = false;
    std::string zip_name;
    std::string evidence_name; // v0.49.1 friendly per-run evidence bundle name
    // Sequential unattended experiment queue. Format: name|command-line args.
    std::filesystem::path batch_file;
    std::filesystem::path debug_config_file;
    // v0.32 sprite diagnostics. Empty frame list means no automatic snapshots.
    std::vector<unsigned> sprite_debug_frames;
    int sprite_debug_index = -1; // -1 = all logical sprites
    bool sprite_debug = false;
    bool sprite_debug_large_only = false;
    bool sprite_dump_ram = false;
    bool sprite_export = false;
    bool sprite_export_tiles = false;
    bool sprite_export_analysis = false;
    bool sprite_export_atlas = false;
    bool sprite_export_alternates = false;
    // v0.39 compositor / priority / palette forensic diagnostics.
    std::vector<unsigned> graphics_debug_frames;
    bool graphics_debug_all = false;
    bool graphics_export_raw_maps = false;
    // v0.48.4 deterministic final-frame screenshot capture.
    unsigned screenshot_every = 0;
    unsigned screenshot_from = 0;
    unsigned screenshot_to = 0; // 0 = no upper bound
    unsigned screenshot_limit = 1000;
    std::filesystem::path screenshot_dir; // empty = <logs>/screenshots
    std::vector<unsigned> screenshot_at;
    // v0.49.0 authoritative per-frame sprite evidence exported after composition.
    unsigned sprite_evidence_every = 0;
    unsigned sprite_evidence_from = 0;
    unsigned sprite_evidence_to = 0;
    std::filesystem::path sprite_evidence_dir;
    // v0.50.0 presentation-only sprite forensic filtering. Each field is a set: empty = wildcard.
    // CLI values may be pipe-separated, e.g. map=778|779|780|781,palette=134.
    struct SpriteSelector {
        std::vector<int> slots, maps, palettes, priorities;
        static bool contains(const std::vector<int>& values, int value) {
            return values.empty() || std::find(values.begin(), values.end(), value) != values.end();
        }
        bool matches(unsigned s, unsigned m, unsigned p, int pri) const {
            return contains(slots,(int)s) && contains(maps,(int)m) &&
                   contains(palettes,(int)p) && contains(priorities,pri);
        }
    };
    struct SpriteQuadCandidate {
        std::string name;
        int tl=-1, tr=-1, bl=-1, br=-1, palette=-1, priority=-1;
    };
    std::vector<SpriteSelector> sprite_solo;
    std::vector<SpriteSelector> sprite_hide;
    std::vector<SpriteQuadCandidate> sprite_quad_candidates;
    // v0.50.3 semantic-object aliases discovered from structured evidence.
    std::vector<std::string> object_evidence;
    std::vector<std::string> object_track;
    enum class DiagnosticBackground { None, Black, White, Checkerboard };
    DiagnosticBackground diagnostic_background = DiagnosticBackground::None;
    struct LayerOffset { int x = 0; int y = 0; };
    // Presentation-only per-layer diagnostic offsets. Negative Y moves a layer upward.
    // RC2.4 restores neutral BG defaults: the historical -4/-20 compensation was
    // masking a TC0100SCN Y-scroll sign error now corrected in the renderer.
    LayerOffset layer_offset_bg0{};
    LayerOffset layer_offset_bg1{};
    LayerOffset layer_offset_text{};
    LayerOffset layer_offset_sprites{};
    LayerOffset layer_offset_road{};
    // v0.66.9.0-RC2.2: Chase H.Q. native default follows MAME's documented reverse sprite-RAM traversal; first nontransparent entry reserves overlap (RC2.2 inferred the winner incorrectly). Diagnostic override remains available.
    enum class SpriteTieBreak { LowerSlot, HigherSlot };
    SpriteTieBreak sprite_tie_break = SpriteTieBreak::LowerSlot;
    bool tc0100scn_trace = false;
    unsigned tc0100scn_trace_from = 0;
    unsigned tc0100scn_trace_to = 0;
    unsigned tc0100scn_trace_scanline_from = 0;
    unsigned tc0100scn_trace_scanline_to = 239;
    unsigned tc0100scn_trace_scanline_step = 8;
    enum class MixerChoice { Reference, Prom, Legacy };
    MixerChoice mixer = MixerChoice::Prom;
};

Options parse_options(int argc, char** argv);
void print_help();

}
