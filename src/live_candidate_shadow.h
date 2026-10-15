#pragma once
// Read-only instruction-boundary verifier. Original CPUs always execute.
// Sprite full comparison MUST occur at the caller's final RTS (0x40174),
// after the optional trailing clear; 0x40576 is bulk-copy-only.
#include "native_candidates.h"
#include <cstdint>
#include <deque>
#include <functional>
#include <string>

namespace chq {
class LiveCandidateShadow {
public:
    using Peek = std::function<std::uint32_t(std::uint32_t, unsigned)>;
    void configure(bool road, bool sprite) {road_enabled_=road; sprite_enabled_=sprite; reset();}
    void reset();
    bool enabled() const {return road_enabled_ || sprite_enabled_;}
    bool sprite_clear_eligible() const {return sprite_enabled_ && sprite_pending_ && bulk_seen_ && bulk_verified_;}
    bool last_completed_sprite_pass() const {return last_sprite_completion_pass_;}
    // Transfer-entry state; authoritative for native clear guard even if the
    // game updates the public latch before the routine completes.
    std::uint16_t active_sprite_records() const {return sprite_records_;}
    std::uint32_t active_sprite_stage_pointer() const {return sprite_stage_;}
    std::int16_t active_sprite_clear_dbra() const {return sprite_clear_dbra_;}
    void instruction(bool cpu_b, std::uint32_t pc, const Peek& peek, std::uint32_t frame=0);
    std::string report_json() const;
private:
    struct SpriteEvent {
        std::uint32_t frame=0;
        std::uint16_t records=0;
        std::int16_t clear_dbra=-1;
        std::uint32_t stage=0;
        std::uint32_t comparison_pc=0;
        long long first_difference=-1;
        std::uint32_t expected=0, actual=0;
        bool success=false;
    };
    static constexpr std::size_t max_events=32;
    bool road_enabled_=false,sprite_enabled_=false,road_pending_=false,sprite_pending_=false;
    bool bulk_seen_=false,bulk_verified_=false,last_sprite_completion_pass_=false;
    std::uint64_t road_entries_=0,road_pass_=0,road_fail_=0;
    std::uint64_t sprite_entries_=0,sprite_pass_=0,sprite_fail_=0;
    std::uint64_t bulk_pass_=0,bulk_fail_=0,sprite_incomplete_=0,sprite_rejected_=0;
    long long last_road_diff_=-1,last_sprite_diff_=-1;
    std::uint32_t sprite_frame_=0,sprite_stage_=0;
    std::uint16_t sprite_records_=0;
    std::int16_t sprite_clear_dbra_=-1;
    std::deque<SpriteEvent> sprite_events_;
    candidate::RoadResult expected_road_;
    candidate::SpriteResult expected_sprite_;
    void road_begin(const Peek& peek);
    void road_end(const Peek& peek);
    void sprite_begin(const Peek& peek,std::uint32_t frame);
    void sprite_bulk_end(const Peek& peek);
    void sprite_end(const Peek& peek);
    void push_event(const SpriteEvent& event);
};
}
