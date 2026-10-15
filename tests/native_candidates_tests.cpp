#include "native_candidates.h"
#include <cassert>
#include <iostream>
#include <stdexcept>
int main(){
 using namespace chq::candidate;
 std::array<std::uint32_t, sprite_capacity*2> before{};
 before.fill(0xABCD1234);
 SpriteInput s; s.latched=2; s.staging_pointer=0x1027F0; s.staging_longwords={1,2,3,4};s.clear_dbra=0;
 auto a=sprite_transfer(s,before);
 assert(a.writes.size()==6 && a.ram[0]==1 && a.ram[3]==4 && a.ram[4]==0 && a.ram[6]==0xABCD1234);
 s.latched=0;s.staging_pointer=0x102800;s.staging_longwords.clear();
 assert(sprite_transfer(s,before).writes.empty());
 s.latched=257;bool blocked=false;try{sprite_transfer(s,before);}catch(const std::invalid_argument&){blocked=true;}assert(blocked);
 RoadInput r;for(std::size_t i=0;i<road_entries;++i){r.boundaries[i]=static_cast<std::uint16_t>(100+i*10);r.source[i]=static_cast<std::uint16_t>(i);}
 for(std::size_t i=0;i<1024;++i)r.lookup[i]=static_cast<std::uint8_t>(i);
 r.cf4=500;r.cf2=150;r.cf6=0x2400;
 const auto v=road_mask(r);assert(v.selected==35 && v.output[6]==0x9800 && v.output[5]==0x8800);
 assert(v.mutated_source[6]==(0xF600|6));
 auto b=be_bytes(v.output);auto z=b;assert(compare_bytes(b,z).exact);
 z[10]^=1;const auto mismatch=compare_bytes(b,z);assert(!mismatch.exact && mismatch.differences==1 && mismatch.first_difference==10);
 std::cout<<"PASS: native candidates, bounds, reset, byte order, deliberate mismatch\n";
}
