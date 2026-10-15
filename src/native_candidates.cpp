#include "native_candidates.h"
#include <stdexcept>
namespace chq::candidate {
SpriteResult sprite_transfer(const SpriteInput& in,
    const std::array<std::uint32_t, sprite_capacity*2>& before) {
    const std::size_t n = in.latched;
    if(n > sprite_capacity || in.staging_longwords.size() != 2*n ||
       in.staging_pointer != 0x102800 - 8*n)
        throw std::invalid_argument("invalid sprite count/staging pointer/input size");
    const std::size_t clear = in.clear_dbra < 0 ? 0 : static_cast<std::size_t>(in.clear_dbra)+1;
    if (n + clear > sprite_capacity) throw std::invalid_argument("sprite clear exceeds capacity");
    SpriteResult result;result.ram=before;
    if(n==0)return result;
    result.writes.reserve((n+clear)*2);
    for(std::size_t i=0;i<n*2;++i){
        result.ram[i]=in.staging_longwords[i];
        result.writes.push_back({0xD00000u+static_cast<std::uint32_t>(i*4),result.ram[i]});
    }
    for(std::size_t i=n*2;i<(n+clear)*2;++i){
        result.ram[i]=0;
        result.writes.push_back({0xD00000u+static_cast<std::uint32_t>(i*4),0});
    }
    return result;
}
RoadResult road_mask(const RoadInput& in){
    RoadResult result;result.mutated_source=in.source;
    const std::uint16_t high=static_cast<std::uint16_t>(0xD200u+in.cf6)&0xFF00u;
    for(std::size_t i=0;i<road_entries;++i){
        const bool selected=in.cf4>=in.boundaries[i] && in.cf2<in.boundaries[i];
        result.output[i]=selected?0x9800:0x8800;
        if(selected){
            result.mutated_source[i]=static_cast<std::uint16_t>(high|in.lookup[in.source[i]&0x03ffu]);
            ++result.selected;
        }
    }
    return result;
}
Compare compare_bytes(const std::vector<std::uint8_t>& a,const std::vector<std::uint8_t>& b){
    Compare r;r.bytes=a.size();
    if(a.size()!=b.size()) throw std::invalid_argument("comparison dimensions differ");
    for(std::size_t i=0;i<a.size();++i) if(a[i]!=b[i]){
        r.exact=false;++r.differences;
        if(r.first_difference==static_cast<std::size_t>(-1))r.first_difference=i;
    }
    return r;
}
std::vector<std::uint8_t> be_bytes(const std::array<std::uint16_t,road_entries>& words){
    std::vector<std::uint8_t> b; b.reserve(words.size()*2);
    for(auto w:words){b.push_back(static_cast<std::uint8_t>(w>>8));b.push_back(static_cast<std::uint8_t>(w));}
    return b;
}
std::vector<std::uint8_t> be_bytes(const std::array<std::uint32_t,sprite_capacity*2>& words){
    std::vector<std::uint8_t> b;b.reserve(words.size()*4);
    for(auto w:words) for(int n=3;n>=0;--n)b.push_back(static_cast<std::uint8_t>(w>>(n*8)));
    return b;
}
}
