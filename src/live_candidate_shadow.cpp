#include "live_candidate_shadow.h"
#include <sstream>
#include <stdexcept>
namespace chq {
void LiveCandidateShadow::reset() {
    road_pending_=sprite_pending_=bulk_seen_=bulk_verified_=last_sprite_completion_pass_=false;
    road_entries_=road_pass_=road_fail_=0;
    sprite_entries_=sprite_pass_=sprite_fail_=0;
    bulk_pass_=bulk_fail_=sprite_incomplete_=sprite_rejected_=0;
    last_road_diff_=last_sprite_diff_=-1;
    sprite_frame_=sprite_stage_=0;sprite_records_=0;sprite_clear_dbra_=-1;
    sprite_events_.clear();
}
void LiveCandidateShadow::road_begin(const Peek& p) {
    candidate::RoadInput in;
    for(std::size_t i=0;i<candidate::road_entries;++i){
        in.source[i]=static_cast<std::uint16_t>(p(0x101384u+static_cast<std::uint32_t>(2*i),2));
        in.boundaries[i]=static_cast<std::uint16_t>(p(0x01FF24u+static_cast<std::uint32_t>(2*i),2));
    }
    for(std::size_t i=0;i<in.lookup.size();++i) in.lookup[i]=static_cast<std::uint8_t>(p(0x91A0u+static_cast<std::uint32_t>(i),1));
    in.cf4=static_cast<std::uint16_t>(p(0x102CF4,2));
    in.cf2=static_cast<std::uint16_t>(p(0x102CF2,2));
    in.cf6=static_cast<std::uint16_t>(p(0x102CF6,2));
    expected_road_=candidate::road_mask(in);
    ++road_entries_;road_pending_=true;
}
void LiveCandidateShadow::road_end(const Peek& p) {
    bool equal=true;
    for(std::size_t i=0;i<candidate::road_entries;++i){
        const auto actual_output=p(0x101454u+static_cast<std::uint32_t>(i*2),2);
        const auto actual_source=p(0x101384u+static_cast<std::uint32_t>(i*2),2);
        if(actual_output!=expected_road_.output[i]||actual_source!=expected_road_.mutated_source[i]){
            if(equal)last_road_diff_=static_cast<long long>(i*2);
            equal=false;
        }
    }
    (equal?road_pass_:road_fail_)++;
    road_pending_=false;
}
void LiveCandidateShadow::push_event(const SpriteEvent& event) {
    if(sprite_events_.size()==max_events)sprite_events_.pop_front();
    sprite_events_.push_back(event);
}
void LiveCandidateShadow::sprite_begin(const Peek& p,std::uint32_t frame) {
    candidate::SpriteInput input;
    input.latched=static_cast<std::uint16_t>(p(0x101844,2));
    input.staging_pointer=p(0x101846,4);
    input.clear_dbra=static_cast<std::int16_t>(p(0x101812,2));
    ++sprite_entries_;
    sprite_frame_=frame;sprite_records_=input.latched;
    sprite_clear_dbra_=input.clear_dbra;sprite_stage_=input.staging_pointer;
    bulk_seen_=bulk_verified_=last_sprite_completion_pass_=false;
    if(input.latched>candidate::sprite_capacity){
        ++sprite_fail_;++sprite_rejected_;last_sprite_diff_=-2;
        push_event({frame,input.latched,input.clear_dbra,input.staging_pointer,0x4013Au,-2,0,0,false});
        return;
    }
    for(std::size_t i=0;i<input.latched*2;++i)
        input.staging_longwords.push_back(p(input.staging_pointer+static_cast<std::uint32_t>(i*4),4));
    std::array<std::uint32_t,candidate::sprite_capacity*2> before{};
    for(std::size_t i=0;i<before.size();++i)before[i]=p(0xD00000u+static_cast<std::uint32_t>(i*4),4);
    try {expected_sprite_=candidate::sprite_transfer(input,before);sprite_pending_=true;}
    catch(const std::invalid_argument&){
        sprite_pending_=false;++sprite_fail_;++sprite_rejected_;last_sprite_diff_=-2;
        push_event({frame,input.latched,input.clear_dbra,input.staging_pointer,0x4013Au,-2,0,0,false});
    }
}
void LiveCandidateShadow::sprite_bulk_end(const Peek& p) {
    // 0x40576 is the unrolled copier's RTS, before the caller's clear loop.
    // Only compare the copied records here; NOT the whole 2048-byte sprite RAM.
    bool equal=true;
    for(std::size_t i=0;i<static_cast<std::size_t>(sprite_records_)*2;++i)
        if(p(0xD00000u+static_cast<std::uint32_t>(i*4),4)!=expected_sprite_.ram[i]){equal=false;break;}
    (equal?bulk_pass_:bulk_fail_)++;
    bulk_seen_=true;bulk_verified_=equal;
}
void LiveCandidateShadow::sprite_end(const Peek& p) {
    SpriteEvent event;
    event.frame=sprite_frame_;event.records=sprite_records_;event.clear_dbra=sprite_clear_dbra_;
    event.stage=sprite_stage_;event.comparison_pc=0x40174;
    event.success=true;
    for(std::size_t i=0;i<expected_sprite_.ram.size();++i){
        const std::uint32_t actual=p(0xD00000u+static_cast<std::uint32_t>(i*4),4);
        if(actual!=expected_sprite_.ram[i]){
            if(event.success){
                event.first_difference=static_cast<long long>(i*4);
                event.expected=expected_sprite_.ram[i];event.actual=actual;
            }
            event.success=false;
        }
    }
    if(!bulk_seen_){++sprite_incomplete_;event.success=false;event.first_difference=-3;}
    last_sprite_completion_pass_=event.success;
    (event.success?sprite_pass_:sprite_fail_)++;
    if(!event.success){last_sprite_diff_=event.first_difference;push_event(event);}
    else if(sprite_events_.size()<max_events)push_event(event);
    sprite_pending_=false;bulk_seen_=bulk_verified_=false;
}
void LiveCandidateShadow::instruction(bool cpu_b,std::uint32_t pc,const Peek& peek,std::uint32_t frame){
    if(cpu_b){
        if(road_enabled_){
            if(pc==0x3B84 && !road_pending_)road_begin(peek);
            else if(road_pending_ && (pc==0x3BDC || pc==0x3BE6))road_end(peek);
        }
    }else if(sprite_enabled_){
        if(pc==0x4013A){
            if(sprite_pending_){
                ++sprite_incomplete_;
                push_event({sprite_frame_,sprite_records_,sprite_clear_dbra_,sprite_stage_,pc,-3,0,0,false});
                sprite_pending_=false;
            }
            sprite_begin(peek,frame);
        }else if(sprite_pending_ && pc==0x40576){
            sprite_bulk_end(peek);
        }else if(sprite_pending_ && pc==0x40174){
            sprite_end(peek);
        }
    }
}
std::string LiveCandidateShadow::report_json() const {
    std::ostringstream os;
    // Additive extensions preserve the v1 fields parsed by existing headless tooling.
    os<<"{\"schema\":\"chq-live-candidate-shadow-v1\",\"mode\":\"shadow-observe-only\",\"replacementEnabled\":false,"
      <<"\"road\":{\"enabled\":"<<(road_enabled_?"true":"false")<<",\"entries\":"<<road_entries_<<",\"pass\":"<<road_pass_<<",\"fail\":"<<road_fail_<<",\"pending\":"<<(road_pending_?"true":"false")<<",\"firstLastMismatchByte\":"<<last_road_diff_<<"},"
      <<"\"spriteTransfer\":{\"enabled\":"<<(sprite_enabled_?"true":"false")<<",\"entries\":"<<sprite_entries_<<",\"pass\":"<<sprite_pass_<<",\"fail\":"<<sprite_fail_<<",\"pending\":"<<(sprite_pending_?"true":"false")<<",\"firstLastMismatchByte\":"<<last_sprite_diff_
      <<",\"fullComparisonPC\":262516,\"bulkOnlyPC\":263542,\"bulkPass\":"<<bulk_pass_<<",\"bulkFail\":"<<bulk_fail_
      <<",\"incomplete\":"<<sprite_incomplete_<<",\"rejected\":"<<sprite_rejected_<<",\"recent\":[";
    bool first=true;
    for(const auto& event:sprite_events_){
        if(!first)os<<',';first=false;
        os<<"{\"frame\":"<<event.frame<<",\"records\":"<<event.records<<",\"clearDbra\":"<<event.clear_dbra
          <<",\"stagePointer\":"<<event.stage<<",\"comparisonPC\":"<<event.comparison_pc<<",\"pass\":"<<(event.success?"true":"false")
          <<",\"firstMismatchByte\":"<<event.first_difference<<",\"expectedWord\":"<<event.expected<<",\"actualWord\":"<<event.actual<<'}';
    }
    os<<"]}}\n";
    return os.str();
}
}
