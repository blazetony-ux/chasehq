#include "native_sprite_clear.h"
#include <cstddef>
namespace chq::candidate {
ClearLoopPlan plan_native_sprite_clear(const ClearLoopContext& c,
    const SpriteClearPeek& peek) {
    ClearLoopPlan plan;
    auto deny=[&](const char* why) {plan.reason=why;return plan;};
    if(c.records==0 || c.records>sprite_capacity || c.clear_dbra<0)
        return deny("unsupported-count-or-no-clear");
    const auto count=static_cast<std::uint32_t>(c.clear_dbra)+1u;
    if(count>sprite_capacity-c.records)return deny("clear-out-of-range");
    if(c.stage_pointer!=0x102800u-8u*c.records)return deny("staging-pointer");
    if(c.a1!=0xD00000u+8u*c.records)return deny("destination-pointer");
    if(static_cast<std::uint16_t>(c.d0)!=static_cast<std::uint16_t>(c.clear_dbra))
        return deny("dbra-count-register");
    if(c.d1!=0 || (c.sr&0x0fu)!=0x04u)return deny("clear-register-or-flags");
    // These exact 68000 instructions are required: two move.l D1,(A1)+;
    // dbra D0,-6 (0xFFFA); RTS. A different ROM/layout MUST fall back to emulation.
    if(peek(0x4016Cu,2)!=0x22C1u || peek(0x4016Eu,2)!=0x22C1u ||
       peek(0x40170u,2)!=0x51C8u || peek(0x40172u,2)!=0xFFFAu ||
       peek(0x40174u,2)!=0x4E75u)return deny("rom-fingerprint");
    // The copied bytes already in hardware must match the native staging input.
    // This is a cheap read-only precondition independent of a previous PASS.
    for(std::uint32_t i=0;i<2u*c.records;++i) {
        if(peek(0xD00000u+4*i,4)!=peek(c.stage_pointer+4*i,4))
            return deny("bulk-input-output-mismatch");
    }
    plan.permitted=true;plan.reason="guard-pass";
    plan.address=c.a1;plan.bytes=8u*count;
    plan.next_a1=c.a1+plan.bytes;
    plan.next_d0=(c.d0&0xffff0000u)|0xffffu;
    // 68000: MOVE.L Dn,(An)+ = 12 cycles, DBRA taken 10 / not taken 14.
    // Native block replaces count*(2*12) + (count-1)*10 + 14.
    plan.emulated_cycles=34u*count+4u;
    return plan;
}
}
