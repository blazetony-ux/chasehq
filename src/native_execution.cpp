#include "native_execution.h"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <tuple>

namespace chq {
namespace {
std::string hex_pc(std::uint32_t pc) {
    std::ostringstream s;s<<"0x"<<std::hex<<std::uppercase<<std::setfill('0')<<std::setw(6)<<pc;return s.str();
}
std::string escape_json(const std::string& in){
    std::ostringstream out;
    for(unsigned char c:in){
        if(c=='"'||c=='\\'){out<<'\\'<<char(c);}
        else if(c<0x20){out<<"\\u"<<std::hex<<std::setw(4)<<std::setfill('0')<<unsigned(c)<<std::dec;}
        else out<<char(c);
    }
    return out.str();
}
struct Hit { bool sub; std::uint32_t pc; std::uint64_t count; };
}
void RomExecutionCensus::enable(){
    enabled_=true;a_.assign(0x80000/2,0);b_.assign(0x20000/2,0);
    other_a_=other_b_=0;
}
void RomExecutionCensus::reset(){
    if(!enabled_)return;
    std::fill(a_.begin(),a_.end(),0);std::fill(b_.begin(),b_.end(),0);
    other_a_=other_b_=0;
}
void RomExecutionCensus::record(bool sub,std::uint32_t pc){
    if(!enabled_)return;
    auto& counts=sub?b_:a_;
    if((pc&1u)==0&&pc/2<counts.size())++counts[pc/2];
    else ++(sub?other_b_:other_a_);
}
std::uint64_t RomExecutionCensus::rom_instructions(bool sub) const {
    std::uint64_t total=0;for(auto n:(sub?b_:a_))total+=n;return total;
}
std::uint64_t RomExecutionCensus::other_instructions(bool sub) const {return sub?other_b_:other_a_;}
void RomExecutionCensus::export_pc_counts(const std::filesystem::path& path) const {
    if(!enabled_)throw std::runtime_error("ROM census not enabled");
    std::ofstream out(path,std::ios::binary|std::ios::trunc);if(!out)throw std::runtime_error("cannot write census PC counts");
    out<<"cpu\tpc\tinstructionHits\n";
    for(int cpu=0;cpu<2;++cpu){const auto& counts=cpu?b_:a_;for(std::size_t i=0;i<counts.size();++i)
        if(counts[i])out<<(cpu?"CPU-B":"CPU-A")<<'\t'<<hex_pc(static_cast<std::uint32_t>(i*2))<<'\t'<<counts[i]<<'\n';}
    if(!out)throw std::runtime_error("cannot finish census PC counts");
}
void RomExecutionCensus::export_json(const std::filesystem::path& path,const std::string& scenario,
                                      std::uint64_t cycles_main,std::uint64_t cycles_sub) const {
    if(!enabled_)throw std::runtime_error("ROM census not enabled");
    std::ofstream out(path,std::ios::binary|std::ios::trunc);if(!out)throw std::runtime_error("cannot write ROM census");
    std::vector<Hit> hot;
    std::uint64_t distinct[2]{0,0};
    for(int cpu=0;cpu<2;++cpu){const auto& v=cpu?b_:a_;for(std::size_t i=0;i<v.size();++i)
        if(v[i]){++distinct[cpu];hot.push_back({cpu==1,static_cast<std::uint32_t>(i*2),v[i]});}}
    std::sort(hot.begin(),hot.end(),[](const Hit& a,const Hit& b){if(a.count!=b.count)return a.count>b.count;return std::tie(a.sub,a.pc)<std::tie(b.sub,b.pc);});
    out<<"{\n  \"schema\": \"chq-rom-execution-census-v1\",\n"
       <<"  \"scenario\": \""<<escape_json(scenario)<<"\",\n"
       <<"  \"measurement\": \"instruction-hook hits; original ROM address windows only\",\n"
       <<"  \"caveats\": [\"Observed scenarios only; absence of hits is not proof of unused code\","
       <<" \"PCs and 256-byte regions are not proven routine boundaries\","
       <<" \"CPU cycles do not isolate host wall-time by original routine\"],\n"
       <<"  \"nativeInstructionReplacements\": 0,\n"
       <<"  \"cpuA\": {\"romInstructionHits\": "<<rom_instructions(false)<<", \"distinctRomPCs\": "<<distinct[0]<<", \"otherInstructionHits\": "<<other_a_<<", \"emulatedCycles\": "<<cycles_main<<"},\n"
       <<"  \"cpuB\": {\"romInstructionHits\": "<<rom_instructions(true)<<", \"distinctRomPCs\": "<<distinct[1]<<", \"otherInstructionHits\": "<<other_b_<<", \"emulatedCycles\": "<<cycles_sub<<"},\n"
       <<"  \"hotPCs\": [\n";
    for(std::size_t i=0;i<std::min(std::size_t(32),hot.size());++i){auto h=hot[i];
        out<<"    {\"cpu\": \""<<(h.sub?"B":"A")<<"\", \"pc\": \""<<hex_pc(h.pc)<<"\", \"instructionHits\": "<<h.count<<"}";
        out<<(i+1<std::min(std::size_t(32),hot.size())?",":"")<<"\n";
    }
    out<<"  ],\n  \"regionGranularityBytes\": 256,\n  \"hotRegions\": [\n";
    std::vector<Hit> regions;
    for(int cpu=0;cpu<2;++cpu){const auto& v=cpu?b_:a_;for(std::size_t i=0;i<v.size();i+=128){
        std::uint64_t sum=0;for(std::size_t j=i;j<std::min(i+128,v.size());++j)sum+=v[j];
        if(sum)regions.push_back({cpu==1,static_cast<std::uint32_t>(i*2),sum});}}
    std::sort(regions.begin(),regions.end(),[](const Hit& a,const Hit& b){if(a.count!=b.count)return a.count>b.count;return std::tie(a.sub,a.pc)<std::tie(b.sub,b.pc);});
    for(std::size_t i=0;i<std::min(std::size_t(32),regions.size());++i){auto h=regions[i];
        out<<"    {\"cpu\": \""<<(h.sub?"B":"A")<<"\", \"start\": \""<<hex_pc(h.pc)<<"\", \"instructionHits\": "<<h.count<<"}";
        out<<(i+1<std::min(std::size_t(32),regions.size())?",":"")<<"\n";
    }
    out<<"  ]\n}\n";
    if(!out)throw std::runtime_error("cannot finish ROM census");
}
void NativeExecutionBridge::observe_entry(bool sub,std::uint32_t pc,const std::string& name){
    if(name.empty()||pc&1u)throw std::invalid_argument("bridge entry requires a name and even PC");
    const auto k=std::make_pair(sub,pc);
    if(entries_.count(k))throw std::invalid_argument("duplicate native bridge entry");
    entries_.emplace(k,Entry{name,0});
}
void NativeExecutionBridge::instruction(bool sub,std::uint32_t pc){
    const auto it=entries_.find(std::make_pair(sub,pc));if(it!=entries_.end())++it->second.hits;
}
void NativeExecutionBridge::clear_counters(){for(auto& kv:entries_)kv.second.hits=0;}
std::string NativeExecutionBridge::report_json() const {
    std::ostringstream out;out<<"{\"schema\":\"chq-native-execution-bridge-v1\",\"activeMode\":\"observe\","
       <<"\"shadowEnabled\":false,\"replaceEnabled\":false,\"nativeRoutines\":0,\"observedEntries\":[";
    bool first=true;for(const auto& [k,e]:entries_){if(!first)out<<',';first=false;
        out<<"{\"cpu\":\""<<(k.first?"B":"A")<<"\",\"pc\":\""<<hex_pc(k.second)
           <<"\",\"label\":\""<<escape_json(e.name)<<"\",\"hits\":"<<e.hits<<"}";}
    out<<"]}\n";return out.str();
}
}
