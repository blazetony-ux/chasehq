#include "native_candidates.h"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>
namespace {
std::vector<std::uint8_t> read(const char* path,std::size_t bytes){
 std::ifstream f(path,std::ios::binary);if(!f)throw std::runtime_error(std::string("open: ")+path);
 std::vector<std::uint8_t> b((std::istreambuf_iterator<char>(f)),{});
 if(b.size()!=bytes)throw std::runtime_error(std::string("size: ")+path);
 return b;
}
std::uint16_t word(const std::vector<std::uint8_t>& b,std::size_t i){return std::uint16_t((unsigned(b.at(i))<<8)|b.at(i+1));}
}
int main(int argc,char**argv){
 if(argc!=7){std::cerr<<"Usage: ChaseHQNativeEvidenceVerify boundary.bin lookup.bin same-frame-control-input.bin class06-decisions.bin class06-observed-input.bin class06-observed-output.bin\n";return 2;}
 try{
  const auto b=read(argv[1],208),lookup=read(argv[2],1024),source=read(argv[3],208),decisions=read(argv[4],64);
  const auto observed_src=read(argv[5],208),observed_out=read(argv[6],208);
  chq::candidate::RoadInput input;
  for(std::size_t i=0;i<104;++i){input.boundaries[i]=word(b,i*2);input.source[i]=word(source,i*2);}
  for(std::size_t i=0;i<1024;++i)input.lookup[i]=lookup[i];
  input.cf4=word(decisions,0x14);input.cf2=word(decisions,0x12);input.cf6=word(decisions,0x16);
  auto result=chq::candidate::road_mask(input);
  auto src=chq::candidate::compare_bytes(chq::candidate::be_bytes(result.mutated_source),observed_src);
  auto out=chq::candidate::compare_bytes(chq::candidate::be_bytes(result.output),observed_out);
  std::cout<<"selected="<<result.selected<<" mismatches_source="<<src.differences<<" mismatches_road="<<out.differences<<" bytes_compared="<<(src.bytes+out.bytes)<<"\n";
  return src.exact&&out.exact?0:1;
 }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 2;}
}
