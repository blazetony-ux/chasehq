#pragma once
// Independent, opt-in, read-only 68000 target projection selector oracle.
// No original CPU instruction, register, memory location or sprite is altered.
#include <cstdint>
#include <filesystem>
#include <string>

namespace chq {
struct ProjectionDecision {
    bool accepted = false;
    std::int32_t separation = 0;
    std::uint32_t table_record = 0;
};
ProjectionDecision native_projection_decide(std::uint32_t coordinate, std::uint32_t reference);

class ProjectionShadow {
public:
    void enable() { enabled_ = true; }
    bool enabled() const { return enabled_; }
    void reset();
    void entry(std::uint32_t coordinate, std::uint32_t reference, std::uint32_t frame);
    void lookup(std::uint32_t actual_record, std::uint32_t frame);
    void exit(std::uint32_t frame);
    std::string report_json() const;
    void export_json(const std::filesystem::path& path) const;
private:
    bool enabled_ = false;
    bool pending_ = false;
    ProjectionDecision expected_{};
    std::uint32_t entry_frame_ = 0;
    std::uint64_t entries_ = 0, accepted_ = 0, rejected_ = 0, matched_ = 0, mismatched_ = 0, unresolved_ = 0;
    std::string first_mismatch_;
    void finish(bool actual_accepted, std::uint32_t record, std::uint32_t frame);
};
}
