#include "native_execution.h"
#include <cassert>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <filesystem>
int main(){
    chq::RomExecutionCensus census;
    census.record(false,0x200);assert(census.rom_instructions(false)==0);
    census.enable();census.record(false,0x200);census.record(false,0x200);census.record(true,0x200);
    census.record(true,0x20000);census.record(false,0x100000);
    assert(census.rom_instructions(false)==2&&census.rom_instructions(true)==1);
    assert(census.other_instructions(false)==1&&census.other_instructions(true)==1);
    auto out=std::filesystem::temp_directory_path()/"chq-native-census-test.json";
    auto tsv=std::filesystem::temp_directory_path()/"chq-native-census-test.tsv";
    census.export_json(out,"unit-test",200,100);census.export_pc_counts(tsv);
    std::ifstream in(out);const std::string content((std::istreambuf_iterator<char>(in)),std::istreambuf_iterator<char>());
    assert(content.find("\"romInstructionHits\": 2")!=std::string::npos);
    assert(content.find("\"nativeInstructionReplacements\": 0")!=std::string::npos);
    assert(content.find("unit-test")!=std::string::npos);
    in.close();std::filesystem::remove(out);std::filesystem::remove(tsv);
    census.reset();assert(census.rom_instructions(false)==0);
    chq::NativeExecutionBridge bridge;
    assert(bridge.request_mode(chq::NativeExecutionBridge::Mode::Observe));
    assert(!bridge.request_mode(chq::NativeExecutionBridge::Mode::Shadow));
    assert(!bridge.request_mode(chq::NativeExecutionBridge::Mode::Replace));
    bridge.observe_entry(false,0xa112,"target_damage_candidate");bridge.instruction(false,0xa112);bridge.instruction(true,0xa112);
    assert(bridge.report_json().find("\"hits\":1")!=std::string::npos);
    assert(bridge.report_json().find("\"replaceEnabled\":false")!=std::string::npos);
    bool duplicate_rejected=false;try{bridge.observe_entry(false,0xa112,"duplicate");}catch(const std::invalid_argument&){duplicate_rejected=true;}assert(duplicate_rejected);
}
