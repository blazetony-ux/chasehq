#pragma once
// RC4.3: evidence-only execution census and deliberately fail-closed bridge.
// A 256-byte bin is a *region*, never automatically inferred as a routine.
// No original 68000 execution is skipped in this milestone.
#include <cstdint>
#include <filesystem>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace chq {
class RomExecutionCensus {
public:
    void enable();
    bool enabled() const { return enabled_; }
    void reset();
    void record(bool sub, std::uint32_t pc);
    std::uint64_t rom_instructions(bool sub) const;
    std::uint64_t other_instructions(bool sub) const;
    void export_json(const std::filesystem::path& summary,
                     const std::string& scenario, std::uint64_t cycles_main,
                     std::uint64_t cycles_sub) const;
    void export_pc_counts(const std::filesystem::path& path) const;
private:
    bool enabled_ = false;
    std::vector<std::uint64_t> a_, b_;
    std::uint64_t other_a_ = 0, other_b_ = 0;
};

class NativeExecutionBridge {
public:
    enum class Mode { Observe, Shadow, Replace };
    // All per-routine replacement paths remain ineligible until explicit,
    // independent implementation and state-equivalence machinery exist.
    bool request_mode(Mode mode) { return mode == Mode::Observe; }
    void observe_entry(bool sub, std::uint32_t pc, const std::string& label);
    void instruction(bool sub, std::uint32_t pc);
    void clear_counters();
    std::string report_json() const;
private:
    struct Entry { std::string name; std::uint64_t hits = 0; };
    std::map<std::pair<bool,std::uint32_t>,Entry> entries_;
};
}
