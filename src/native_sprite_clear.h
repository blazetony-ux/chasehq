#pragma once
// RC5.3 EXPERIMENTAL: native substitution of the *trailing clear loop ONLY*.
// The original 68000 sprite bulk copier, caller setup and final RTS remain
// authoritative. The read-only guard must pass before any CPU/RAM mutation.
#include "native_candidates.h"
#include <cstdint>
#include <functional>
#include <string>
namespace chq::candidate {
struct ClearLoopContext {
    std::uint32_t a1=0,d0=0,d1=0,sr=0;
    std::uint16_t records=0;
    std::int16_t clear_dbra=-1;
    std::uint32_t stage_pointer=0;
};
struct ClearLoopPlan {
    bool permitted=false;
    std::string reason;
    std::uint32_t address=0, bytes=0, next_a1=0, next_d0=0;
    std::uint32_t return_pc=0x40174;
    std::uint32_t emulated_cycles=0;
};
using SpriteClearPeek=std::function<std::uint32_t(std::uint32_t,unsigned)>;
ClearLoopPlan plan_native_sprite_clear(const ClearLoopContext& context,
    const SpriteClearPeek& peek);
}
