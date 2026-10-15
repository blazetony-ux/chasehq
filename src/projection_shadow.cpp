#include "projection_shadow.h"
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace chq {
ProjectionDecision native_projection_decide(std::uint32_t coordinate, std::uint32_t reference) {
    // 68000 32-bit wrap, signed threshold, arithmetic shift and 16-bit ADDA sign extension.
    const auto delta = static_cast<std::int32_t>(coordinate - reference);
    ProjectionDecision result;
    result.separation = delta;
    result.accepted = static_cast<std::int32_t>(coordinate) > 0 && delta >= -0x180 && delta < 0x8000;
    if (!result.accepted) return result;
    // Avoid implementation-defined signed >> and negative signed overflow.
    const std::int64_t half = delta >= 0 ? delta / 2 : -((-std::int64_t(delta) + 1) / 2);
    const auto word = static_cast<std::uint16_t>(static_cast<std::uint32_t>(half) & 0xfff8u);
    const auto signed_offset = word < 0x8000u ? static_cast<std::int32_t>(word) : static_cast<std::int32_t>(word) - 0x10000;
    result.table_record = static_cast<std::uint32_t>(0x54100 + signed_offset);
    return result;
}
void ProjectionShadow::reset() {
    pending_ = false;
    entries_=accepted_=rejected_=matched_=mismatched_=unresolved_=0;
    first_mismatch_.clear();
}
void ProjectionShadow::entry(std::uint32_t coordinate, std::uint32_t reference, std::uint32_t frame) {
    if (!enabled_) return;
    if (pending_) ++unresolved_; // Never silently count an unobserved exit as agreement.
    expected_=native_projection_decide(coordinate,reference);
    pending_=true;entry_frame_=frame;++entries_;
    if(expected_.accepted) ++accepted_;else ++rejected_;
}
void ProjectionShadow::finish(bool actual_accepted, std::uint32_t record, std::uint32_t frame) {
    if (!enabled_ || !pending_) return;
    bool equal=actual_accepted==expected_.accepted && (!actual_accepted || record==expected_.table_record);
    if(equal) ++matched_;
    else {
        ++mismatched_;
        if(first_mismatch_.empty()) {
            std::ostringstream s;
            s << "entryFrame="<<entry_frame_<<" exitFrame="<<frame<<" separation="<<expected_.separation
              <<" expectedAccepted="<<expected_.accepted<<" actualAccepted="<<actual_accepted
              <<" expectedRecord=0x"<<std::hex<<expected_.table_record<<" actualRecord=0x"<<record;
            first_mismatch_=s.str();
        }
    }
    pending_=false;
}
void ProjectionShadow::lookup(std::uint32_t actual_record, std::uint32_t frame) {finish(true,actual_record,frame);}
void ProjectionShadow::exit(std::uint32_t frame) {finish(false,0,frame);}
std::string ProjectionShadow::report_json() const {
    std::ostringstream out;
    out<<"{\"schema\":\"chq-native-projection-shadow-v1\",\"enabled\":"<<(enabled_?"true":"false")
       <<",\"replacementEnabled\":false,\"originalInstructionsSkipped\":0"
       <<",\"entryPC\":\"0x434EE\",\"lookupPC\":\"0x4351C\",\"exitPC\":\"0x40F5A\""
       <<",\"entries\":"<<entries_<<",\"predictedAccepted\":"<<accepted_
       <<",\"predictedRejected\":"<<rejected_<<",\"matched\":"<<matched_
       <<",\"mismatched\":"<<mismatched_<<",\"unresolved\":"<<unresolved_+(pending_?1:0);
    out<<",\"firstMismatch\":\"";
    for(char c:first_mismatch_) {if(c=='"'||c=='\\')out<<'\\';out<<c;}
    out<<"\",\"caveat\":\"Only this CPU-A entry/lookup/exit path; geometry, slot ownership and timing not shadow verified\"}\n";
    return out.str();
}
void ProjectionShadow::export_json(const std::filesystem::path& path) const {
    std::ofstream f(path,std::ios::binary|std::ios::trunc);
    if(!f)throw std::runtime_error("cannot open projection-shadow report");
    f<<report_json();
    if(!f)throw std::runtime_error("cannot write projection-shadow report");
}
}
