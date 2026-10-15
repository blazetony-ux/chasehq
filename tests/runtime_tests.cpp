#include "runtime.h"
#include "tc0100scn_geometry.h"
#include "m68k.h"
#include "m68kcpu.h"
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>

static void check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

static void rejects(const std::function<void()>& f, const char* message) {
    bool rejected = false;
    try { f(); } catch (const std::exception&) { rejected = true; }
    check(rejected, message);
}

static void word(chq::Bytes& rom, unsigned a, unsigned v) {
    rom.at(a) = static_cast<std::uint8_t>(v >> 8);
    rom.at(a + 1) = static_cast<std::uint8_t>(v);
}

static chq::Bytes main_program() {
    chq::Bytes rom(0x80000);
    word(rom, 0, 0x0011); word(rom, 2, 0x0000); // SP 0x110000
    word(rom, 4, 0); word(rom, 6, 0x0100);
    unsigned pc = 0x100;
    // shared=$1234; release CPU B; then STOP
    for (auto v : {0x33fc,0x1234,0x0010,0x8000,
                   0x33fc,0x0001,0x0080,0x0000,
                   0x4e72,0x2700}) {
        word(rom, pc, v); pc += 2;
    }
    return rom;
}

static chq::Bytes call_program() {
    chq::Bytes rom(0x80000);
    word(rom, 0, 0x0011); word(rom, 2, 0x0000);
    word(rom, 4, 0); word(rom, 6, 0x0100);
    word(rom, 0x100, 0x6100); word(rom, 0x102, 0x001e); // BSR.W $120
    word(rom, 0x104, 0x4e72); word(rom, 0x106, 0x2700); // STOP
    unsigned pc=0x120;
    for (auto v : {0x33fc,0x1234,0x0010,0x8020,0x4e75}) { word(rom,pc,v); pc+=2; }
    return rom;
}

// RC4.4: synthetic original-PC instruction-hook fixture; deliberately not
// a fake ROM-level parity claim. It proves that shadow sees Musashi A2/PC.
static chq::Bytes projection_hook_program(bool lookup) {
    chq::Bytes rom(0x80000);
    word(rom,0,0x0011);word(rom,2,0x0000);
    word(rom,4,0x0004);word(rom,6,0x34ee);
    // LEA $54040.L,A2 followed by JMP to original lookup/exit PC.
    for(unsigned i=0;i<6;++i) word(rom,0x434ee + i*2,
        std::array<unsigned,6>{0x45f9,0x0005,0x4040,0x4ef9,0x0004,lookup ? 0x351cu : 0x0f5au}[i]);
    word(rom,lookup?0x4351c:0x40f5a,0x4e72);word(rom,(lookup?0x4351c:0x40f5a)+2,0x2700);
    return rom;
}

// Synthetic micro-op fixture. It only validates execution dispatch semantics;
// it is NOT evidence that the authentic game ROM or complete steering function
// has been reconstructed.
static chq::Bytes steering_move_fixture() {
    auto rom=main_program();
    word(rom,0x820c,0x3b50);word(rom,0x820e,0x8300); // MOVE.W (A0),(-0x7D00,A5)
    word(rom,0x8210,0x4e72);word(rom,0x8212,0x2700); // STOP after the candidate
    word(rom,0x3132c,0); // verified normal-mode input 0 => output 0
    return rom;
}

// Synthetic RC6.3.2 health function + caller. The caller writes W32 at
// 0x102DE0 immediately after the authentic RTS. This reproduces the exact
// bus-trace ownership hazard seen in Windows RC6.3.1 evidence; it is NOT a
// substitute for the independent actual-game-ROM differential acceptance.
static chq::Bytes health_return_boundary_fixture() {
    auto rom=main_program();
    const std::array<unsigned,20> opcodes={
        0x4a6d,0x812e, // A10C TST.W (-0x7ED2,A5)
        0x660e,        // A110 BNE.B A120
        0x536d,0x82ae, // A112 SUBQ.W #1,(-0x7D52,A5)
        0x6a08,        // A116 BPL.B A120
        0x3b7c,0x1400,0x82d8, // A118 MOVE.W #$1400,(-0x7D28,A5)
        0x6008,        // A11E BRA.B A128
        0x302d,0x82ba, // A120 MOVE.W (-0x7D46,A5),D0
        0xd16d,0x82d8, // A124 ADD.W D0,(-0x7D28,A5)
        0x4e75,        // A128 RTS
        0x4e71,0x4e71,0x4e71,0x4e71,0x4e71 // padding
    };
    for (std::size_t i=0;i<opcodes.size();++i)word(rom,0xa10c+static_cast<unsigned>(i*2),opcodes[i]);
    word(rom,0xa09c,0x2f3c);word(rom,0xa09e,0x0000);word(rom,0xa0a0,0xa0a0); // MOVE.L #$A0A0,-(SP)
    word(rom,0xa0a2,0x4e72);word(rom,0xa0a4,0x2700); // STOP
    return rom;
}

// RC6.3.5 synthetic *enclosing routine* call: BSR.B A080 from A09A,
// followed by a JMP into the native candidate tail A10C. Both original and
// generated substitution must preserve real call provenance; this fixture
// must never be mistaken for evidence about the authentic game-ROM caller.
// RC6.3.6 synthetic original CPU-A copy block. This proves Musashi hook
// semantics only; authentic ROM parity remains a separate Windows gate.
static chq::Bytes native_copy_fixture(){
    auto rom=main_program();
    for(unsigned i=0;i<16;++i)word(rom,0x40540u+2u*i,0x22d8u); // MOVE.L (A0)+,(A1)+
    word(rom,0x40560,0x4e72);word(rom,0x40562,0x2700); // STOP
    return rom;
}

static chq::Bytes health_enclosing_caller_fixture() {
    auto rom=health_return_boundary_fixture();
    word(rom,0xa09a,0x61e4); // BSR.B A080; return A09C
    word(rom,0xa080,0x4ef9); word(rom,0xa082,0x0000);
    word(rom,0xa084,0xa10c); // JMP absolute-long into function tail
    return rom;
}

static chq::Bytes sub_program() {
    chq::Bytes rom(0x20000);
    word(rom, 0, 0x0010); word(rom, 2, 0x4000); // SP 0x104000
    word(rom, 4, 0); word(rom, 6, 0x0100);
    unsigned pc = 0x100;
    // Read shared word, increment, write back; write road; STOP
    for (auto v : {0x3039,0x0010,0x8000,
                   0x5240,
                   0x33c0,0x0010,0x8002,
                   0x33fc,0xbeef,0x0080,0x0000,
                   0x4e72,0x2700}) {
        word(rom, pc, v); pc += 2;
    }
    return rom;
}

// RC6.3.10 synthetic CPU-B ROM fixture: BEQ.B at 4DE, both outcomes.
// This checks hook/clock/state semantics only, not original Chase H.Q. ROM.
static chq::Bytes cpu_b_branch_fixture(bool taken) {
    chq::Bytes rom(0x20000);
    word(rom,0,0x0010);word(rom,2,0x4000);
    word(rom,4,0);word(rom,6,0x04d8);
    word(rom,0x4d8,taken?0x4a00:0x7001); // TST.B D0 (Z=1) or MOVEQ #1,D0 (Z=0)
    word(rom,0x4da,0x4e71); // NOP
    word(rom,0x4dc,0x4e71); // NOP
    word(rom,0x4de,0x67f8); // BEQ.B 4D8
    word(rom,0x4e0,0x4e72);word(rom,0x4e2,0x2700); // STOP on not-taken
    return rom;
}

int main() {
    try {
        std::cout << std::unitbuf << "[test] TC0100SCN Y-scroll geometry\n";
        check(chq::tc0100scn_source_y(16, 0x01e0) == 40, "TC0100SCN 0x01e0 Y-scroll must wrap to source line 40 at visible top");
        check(chq::tc0100scn_source_y(16, 0x0000) == 8, "TC0100SCN zero Y-scroll visible-area delta");
        check(chq::tc0100scn_source_y(80, 0x01e0) == 104, "TC0100SCN Y-scroll arithmetic remains signed/wrapped");

        std::cout << std::unitbuf << "[test] ROM reconstruction\n";

        std::array<chq::Bytes, 4> main_lanes;
        for (unsigned j = 0; j < 4; ++j) {
            main_lanes[j].resize(0x20000);
            for (unsigned i = 0; i < 0x20000; ++i)
                main_lanes[j][i] = static_cast<std::uint8_t>(i * 13 + j * 61);
        }
        const auto main = chq::interleave_main(main_lanes);
        check(main.size() == 0x80000, "main region size");
        check(main[0] == main_lanes[0][0] && main[1] == main_lanes[1][0], "main lanes");

        std::array<chq::Bytes, 2> sub_lanes;
        sub_lanes[0].resize(0x10000, 0x12);
        sub_lanes[1].resize(0x10000, 0x34);
        const auto sub = chq::interleave_sub(sub_lanes);
        check(sub.size() == 0x20000, "sub region size");
        check(sub[0] == 0x12 && sub[1] == 0x34, "sub lanes");
        sub_lanes[0].pop_back();
        rejects([&] { chq::interleave_sub(sub_lanes); }, "short sub lane accepted");

#if defined(CHQ_CPU_B_TEST_LIBRARY_PATH)
        // Real Musashi CPU-B substitution; the stand-in library is test-only.
        // Original/native/fallback must agree on both branch outcomes and
        // mapped data-bus activity (zero for BEQ), CPU state and cycles.
        for(bool taken : {false,true}) {
            std::array<std::string,3> traces{},busTraces{},audits{},reports{};
            std::array<std::uint32_t,3> pc{},sr{};
            std::array<std::uint64_t,3> ticks{};
            for(int mode=0;mode<3;++mode) {
                auto emulator=std::make_unique<chq::Runtime>(main_program(),cpu_b_branch_fixture(taken));
                auto path=std::filesystem::temp_directory_path()/("chq-rc6310-cpu-b-"+std::to_string(taken)+"-"+std::to_string(mode));
                std::filesystem::create_directories(path);
                if(mode)emulator->load_native_cpu_b_library(CHQ_CPU_B_TEST_LIBRARY_PATH);
                if(mode)emulator->enable_native_cpu_b_replacement(mode==2);
                emulator->enable_native_cpu_b_capture(path/"native-cpu-b-state.tsv");
                emulator->enable_native_dispatch_audit(path/"native-dispatch.tsv");
                emulator->reset();
                emulator->run(10000);
                emulator->write_native_execution_evidence(path,"synthetic-CPU-B-BEQ");
                pc[mode]=emulator->debug_get_register(chq::BusSpace::Sub,"PC");
                sr[mode]=emulator->debug_get_register(chq::BusSpace::Sub,"SR");
                ticks[mode]=emulator->cycles_b;
                emulator.reset();
                auto read=[&](const char* filename) {std::ifstream in(path/filename,std::ios::binary);
                  return std::string(std::istreambuf_iterator<char>(in),{});};
                traces[mode]=read("native-cpu-b-state.tsv");
                busTraces[mode]=read("native-cpu-b-bus.tsv");
                reports[mode]=read("native-cpu-b-report.json");
                audits[mode]=read("native-dispatch.tsv");
                std::filesystem::remove_all(path);
            }
            check(!traces[0].empty(),"synthetic CPU-B branch never executed");
            check(busTraces[0].empty(),"synthetic original BEQ data-bus contract incorrect");
            check(reports[1].find("\"commits\":0")==std::string::npos,"synthetic generated CPU-B not committed");
            check(reports[2].find("\"rollbacks\":1")!=std::string::npos,"synthetic CPU-B generated rejection absent");
            check(audits[1].find("B:000004DE-000004E0")!=std::string::npos,"shared dispatcher missed CPU-B route");
            for(int mode=1;mode<3;++mode) {
                if(traces[mode]!=traces[0]) {
                   std::cerr<<"CPU-B PARITY DIAG taken="<<taken<<" mode="<<mode
                     <<" original="<<traces[0].substr(0,400)<<" native="<<traces[mode].substr(0,400)
                     <<" sizes="<<traces[0].size()<<"/"<<traces[mode].size()<<"\n";
                }
                check(traces[mode]==traces[0],"CPU-B original/native/rollback state or cycles diverged");
                check(busTraces[mode]==busTraces[0],"CPU-B original/native/rollback data bus diverged");
                check(pc[mode]==pc[0]&&sr[mode]==sr[0]&&ticks[mode]==ticks[0],"CPU-B emulator final register/cycle parity failed");
            }
        }
        std::cout << "[test] RC6.3.10 CPU-B synthetic two-branch ROM/native/fallback equivalence PASS\n";
        // RC6.3.11: the previous trace falsely labelled Musashi interrupt
        // vector/stack effects as BEQ's data bus accesses because trace_pc
        // remained at 0x4DE until the next instruction callback. Exercise a
        // real interrupt ACK + vector/stack sequence in the synthetic CPU.
        {
            std::array<std::string,3> irq{},data{};
            for (int mode=0;mode<3;++mode) {
                auto sub=cpu_b_branch_fixture(true);
                word(sub,0x70,0); word(sub,0x72,0x474); // IRQ4 autovector -> RTE
                word(sub,0x474,0x4e73);
                auto emulator=std::make_unique<chq::Runtime>(main_program(),std::move(sub));
                auto dir=std::filesystem::temp_directory_path()/("chq-rc6311-irq-phase-"+std::to_string(mode));
                std::filesystem::create_directories(dir);
                if(mode)emulator->load_native_cpu_b_library(CHQ_CPU_B_TEST_LIBRARY_PATH);
                if(mode)emulator->enable_native_cpu_b_replacement(mode==2);
                emulator->enable_native_cpu_b_capture(dir/"native-cpu-b-state.tsv");
                emulator->reset();
                emulator->run(1000);
                check(emulator->debug_set_register(chq::BusSpace::Sub,"SR",0x2004u),"cannot unmask CPU-B synthetic IRQ");
                emulator->irq4();
                emulator->run(2000);
                emulator.reset();
                auto read=[&](const char* name){std::ifstream in(dir/name,std::ios::binary);
                    return std::string(std::istreambuf_iterator<char>(in),{});};
                irq[mode]=read("native-cpu-b-irq.tsv");
                data[mode]=read("native-cpu-b-bus.tsv");
                std::filesystem::remove_all(dir);
            }
            check(irq[0].find("\tA\t")!=std::string::npos,"no real CPU-B interrupt ACK captured");
            check(irq[0].find("\tR\t70\t474\t4")!=std::string::npos,"IRQ vector read not separately captured");
            check(irq[0].find("\tW\t")!=std::string::npos,"IRQ stack writes not separately captured");
            check(data[0].empty(),"interrupt effects incorrectly assigned to BEQ branch data bus");
            for(int mode=1;mode<3;++mode) {
                check(data[mode]==data[0],"synthetic interrupt BEQ data bus not preserved");
                check(irq[mode]==irq[0],"synthetic interrupt ACK/vector/stack trace not preserved");
            }
        }
        std::cout << "[test] RC6.3.11 Musashi IRQ transaction ownership/rollback PASS\n";
#endif
#if defined(CHQ_CPU_B_PAIR_TEST_LIBRARY_PATH)
        // RC6.3.13: genuine Musashi CPU-B two-opcode TST.W abs.long + BEQ,
        // with one mapped data read, flags, cycles and forced ROM fallback.
        for(bool zero : {false,true}) {
            std::array<std::string,3> state{},data{},report{},irq{};
            std::array<std::uint64_t,3> ticks{};
            for(int mode=0;mode<3;++mode) {
                chq::Bytes rom(0x20000);
                word(rom,0,0x0010);word(rom,2,0x4000);
                word(rom,4,0);word(rom,6,0x04d8);
                word(rom,0x4d8,0x4a79);word(rom,0x4da,0x0010);word(rom,0x4dc,0x0804);
                word(rom,0x4de,0x67f8);word(rom,0x4e0,0x4e72);word(rom,0x4e2,0x2700);
                auto em=std::make_unique<chq::Runtime>(main_program(),std::move(rom));
                const auto dir=std::filesystem::temp_directory_path()/("chq-rc6313-pair-"+std::to_string(zero)+"-"+std::to_string(mode));
                std::filesystem::create_directories(dir);
                if(mode)em->load_native_cpu_b_pair_library(CHQ_CPU_B_PAIR_TEST_LIBRARY_PATH);
                if(mode)em->enable_native_cpu_b_pair_replacement(mode==2);
                em->enable_native_cpu_b_pair_capture(dir/"native-cpu-b-pair-state.tsv");
                em->enable_native_dispatch_audit(dir/"native-dispatch.tsv");
                em->reset();
                if(!zero)em->bus.debug_write(0x100804,1,2,chq::BusSpace::Sub);
                em->run(10000);em->write_native_execution_evidence(dir,"synthetic-CPU-B-TST-BEQ-pair");
                ticks[mode]=em->cycles_b;em.reset();
                auto read=[&](const char* name){std::ifstream f(dir/name,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
                state[mode]=read("native-cpu-b-pair-state.tsv");data[mode]=read("native-cpu-b-pair-bus.tsv");
                report[mode]=read("native-cpu-b-pair-report.json");irq[mode]=read("native-cpu-b-pair-irq.tsv");
                std::filesystem::remove_all(dir);
            }
            check(!state[0].empty()&&!data[0].empty(),"CPU-B pair missing original state or mapped R16");
            check(report[1].find("\"commits\":0")==std::string::npos,"generated CPU-B pair never committed");
            check(report[2].find("\"rollbacks\":1")!=std::string::npos,"generated CPU-B pair deliberate rollback absent");
            for(int m=1;m<3;++m) {
                if(state[m]!=state[0])std::cerr<<"LENS="<<state[0].size()<<"/"<<state[m].size()<<" FIRST="<<state[0].find_first_not_of(state[m])<<" "<<"RC6313 PAIR STATE DIAG zero="<<zero<<" mode="<<m<<" ROM="<<state[0].substr(0,250)<<" NATIVE="<<state[m].substr(0,250)<<"\n";
                check(state[m]==state[0],"CPU-B pair return register/flags/cycles not equal");
                check(data[m]==data[0],"CPU-B pair original/native R16 stream differs");
                check(irq[m]==irq[0],"CPU-B pair IRQ stream differs");
                check(ticks[m]==ticks[0],"CPU-B pair total CPU cycles differ");
            }
        }
        std::cout << "[test] RC6.3.13 real Musashi CPU-B TST.W+BEQ original/native/fallback PASS\n";
#endif
        // RC6.3.7: independent ORIGINAL CPU-B ROM-PC graph. Observe opcode
        // fingerprints and actual next-hook PCs, not a guessed function CFG.
        {
            auto cpu=std::make_unique<chq::Runtime>(main_program(),sub_program());
            auto folder=std::filesystem::temp_directory_path()/"chq-rc637-cpu-b-edges";
            std::filesystem::create_directories(folder);
            cpu->enable_rom_execution_census();
            cpu->enable_cpu_b_edge_probe();
            cpu->reset();
            cpu->run(6000);
            cpu->write_native_execution_evidence(folder,"synthetic-original-cpu-b-edges");
            auto read=[&](const char* f){std::ifstream in(folder/f,std::ios::binary);
                return std::string(std::istreambuf_iterator<char>(in),{});};
            const auto edges=read("cpu-b-observed-edges.tsv");
            const auto counts=read("rom-execution-pc-counts.tsv");
            check(!edges.empty(),"CPU-B edge observation absent");
            check(edges.find("100\t106\t3039\t")!=std::string::npos,
                  "CPU-B ROM opcode/next-PC fingerprint does not match actual 68000 instruction");
            check(counts.find("CPU-B")!=std::string::npos,"original CPU-B census absent");
            std::filesystem::remove_all(folder);
        }
        // RC6.2.4: prove the hook actually skips CPU-A ROM MOVE.W and updates
        // SR/RAM/PC/cycles, rather than merely overlaying the final value.
        // A forced mismatch must preserve bus state and execute original ROM without speculative writes.
        {
            std::array<std::string,3> reports, stateRecords, busRecords;
            std::array<std::uint32_t,3> ram{},sr{},pc{};
            std::array<std::uint64_t,3> cycles{};
            for (int mode=0;mode<3;++mode) {
                auto runtime=std::make_unique<chq::Runtime>(steering_move_fixture(),sub_program());
                if(mode==0)runtime->enable_native_steering_shadow();
                else runtime->enable_native_steering_replacement(mode==2);
                const auto d=std::filesystem::temp_directory_path()/("chq-rc626-steering-"+std::to_string(mode));
                std::filesystem::create_directories(d);
                runtime->enable_native_steering_state_capture(d/"native-steering-state.tsv");
                runtime->enable_native_steering_bus_capture(d/"native-steering-bus.tsv");
                runtime->reset();
                check(runtime->debug_set_register(chq::BusSpace::Main,"PC",0x820c),"cannot set synthetic steering PC");
                check(runtime->debug_set_register(chq::BusSpace::Main,"A0",0x3132c),"cannot set synthetic steering A0");
                check(runtime->debug_set_register(chq::BusSpace::Main,"A5",0x108000),"cannot set synthetic steering A5");
                check(runtime->debug_set_register(chq::BusSpace::Main,"D0",0),"cannot initialise synthetic steering D0");
                runtime->bus.write(0x100300,0,2,chq::BusSpace::Main);
                runtime->bus.write(0x10012e,0,2,chq::BusSpace::Main);
                runtime->run(500);
                runtime->write_native_execution_evidence(d,"synthetic-move-step");
                std::ifstream f(d/"native-steering-shadow.json");
                reports[mode]=std::string(std::istreambuf_iterator<char>(f),{});
                f.close();
                ram[mode]=runtime->bus.peek(0x100300,2,chq::BusSpace::Main);
                sr[mode]=runtime->debug_get_register(chq::BusSpace::Main,"SR");
                pc[mode]=runtime->debug_get_register(chq::BusSpace::Main,"PC");
                cycles[mode]=runtime->cycles_a;
                runtime.reset(); // flush actual instruction/state and ordered bus records
                std::ifstream stateFile(d/"native-steering-state.tsv",std::ios::binary);
                stateRecords[mode]=std::string(std::istreambuf_iterator<char>(stateFile),{});
                std::ifstream busFile(d/"native-steering-bus.tsv",std::ios::binary);
                busRecords[mode]=std::string(std::istreambuf_iterator<char>(busFile),{});
                stateFile.close(); busFile.close();std::filesystem::remove_all(d);
            }
            check(reports[0].find("\"commits\":0")!=std::string::npos,"original ROM falsely reported native commits");
            check(reports[1].find("\"commits\":1")!=std::string::npos,"native MOVE.W was not actually executed");
            check(reports[2].find("\"rollbacks\":1")!=std::string::npos,"mutated native MOVE.W did not roll back");
            check(!stateRecords[0].empty(),"original ROM did not emit CPU return boundary");
            check(stateRecords[0].find("008210")!=std::string::npos,"wrong native CPU boundary");
            check(reports[1].find("\"compared\":1")!=std::string::npos,"native shadow still pending after skipped CPU hook");
            check(reports[1].find("\"incomplete\":0")!=std::string::npos,"native shadow incorrectly marked incomplete");
            for(int mode=1;mode<3;++mode) {
                check(stateRecords[mode]==stateRecords[0],"actual return-state snapshots differ from original ROM");
                check(busRecords[mode]==busRecords[0],"ordered bus effects differ from original ROM");
                check(ram[mode]==ram[0],"native / rollback RAM diverged from original");
                check(sr[mode]==sr[0],"native / rollback CPU SR diverged from original");
                check(pc[mode]==pc[0],"native / rollback CPU PC diverged from original");
                check(cycles[mode]==cycles[0],"native / rollback CPU cycles diverged from original");
            }
        }
#if defined(CHQ_COPY_TEST_LIBRARY_PATH)
        // Different instruction family from the branched health tail. Verify
        // every 68000 longword bus operation, final CPU flags, original PC,
        // and forced rollback with NO speculative RAM side effects.
        {
            std::array<std::string,3> busRecord,stateRecord,report,dispatch;
            std::array<std::uint32_t,3> pc{},a0{},a1{},sr{},cycles{};
            std::array<std::array<std::uint32_t,16>,3> output{};
            for(int mode=0;mode<3;++mode){
                auto emulator=std::make_unique<chq::Runtime>(native_copy_fixture(),sub_program());
                auto dir=std::filesystem::temp_directory_path()/("chq-rc636-copy-"+std::to_string(mode));
                std::filesystem::create_directories(dir);
                if(mode){
                    emulator->load_native_copy_library(CHQ_COPY_TEST_LIBRARY_PATH);
                    emulator->enable_native_copy_replacement(mode==2);
                }
                emulator->reset();
                check(emulator->debug_set_register(chq::BusSpace::Main,"PC",0x40540),"copy fixture PC");
                check(emulator->debug_set_register(chq::BusSpace::Main,"A0",0x100500),"copy fixture A0");
                check(emulator->debug_set_register(chq::BusSpace::Main,"A1",0x100700),"copy fixture A1");
                check(emulator->debug_set_register(chq::BusSpace::Main,"D0",0),"copy fixture D0");
                for(unsigned i=0;i<16;++i)emulator->bus.write(0x100500+4*i,0x82340000u+i*119u,4,chq::BusSpace::Main);
                emulator->enable_native_copy_capture(dir/"native-copy-state.tsv");
                emulator->enable_native_dispatch_audit(dir/"native-dispatch.tsv");
                emulator->run(6000);
                pc[mode]=emulator->debug_get_register(chq::BusSpace::Main,"PC");
                a0[mode]=emulator->debug_get_register(chq::BusSpace::Main,"A0");
                a1[mode]=emulator->debug_get_register(chq::BusSpace::Main,"A1");
                sr[mode]=emulator->debug_get_register(chq::BusSpace::Main,"SR");
                cycles[mode]=emulator->cycles_a;
                for(unsigned i=0;i<16;++i)output[mode][i]=emulator->bus.peek(0x100700+4*i,4,chq::BusSpace::Main);
                emulator->write_native_execution_evidence(dir,"synthetic-rc636-copy");
                emulator.reset();
                auto load=[&](const char* n){std::ifstream f(dir/n,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
                busRecord[mode]=load("native-copy-bus.tsv");
                stateRecord[mode]=load("native-copy-state.tsv");
                dispatch[mode]=load("native-dispatch.tsv");
                if(mode)report[mode]=load("native-copy-report.json");
                std::filesystem::remove_all(dir);
            }
            std::cerr<<"COPY_TRACE: size="<<busRecord[0].size()<<" count="<<std::count(busRecord[0].begin(),busRecord[0].end(),'\n')<<" first="<<busRecord[0].substr(0,600)<<" state="<<stateRecord[0].substr(0,400)<<"\n";
            check(std::count(busRecord[0].begin(),busRecord[0].end(),'\n')==32,"original ROM copy bus transaction count not 32");
            check(!stateRecord[0].empty(),"original ROM copy block exit state missing");
            check(report[1].find("\"commits\":1")!=std::string::npos,"native copy not committed");
            check(dispatch[0].find("A:00040540-00040560\t40540\toriginal-rom")!=std::string::npos,"original copy not audited by shared dispatcher");
            check(dispatch[1].find("A:00040540-00040560\t40540\tnative-commit")!=std::string::npos,"generated copy not committed via shared dispatcher");
            check(dispatch[2].find("A:00040540-00040560\t40540\toriginal-fallback")!=std::string::npos,"copy fault not rejected via shared dispatcher");
            check(report[2].find("\"rollbacks\":1")!=std::string::npos,"copy mismatch did not roll back");
            check(report[2].find("\"commits\":0")!=std::string::npos,"copy fallback wrote speculative data");
            for(int mode=1;mode<3;++mode){
                check(busRecord[mode]==busRecord[0],"copy ordered original/native/rollback bus differs");
                auto clean=[](std::string x){auto pos=x.find("\trom\t");if(pos!=std::string::npos)x.replace(pos,5,"\tlabel\t");
                    pos=x.find("\tnative\t");if(pos!=std::string::npos)x.replace(pos,8,"\tlabel\t");return x;};
                check(clean(stateRecord[mode])==clean(stateRecord[0]),"copy block original/native CPU return state differs");
                check(output[mode]==output[0],"copy contents diverged");
                check(pc[mode]==pc[0]&&a0[mode]==a0[0]&&a1[mode]==a1[0]&&sr[mode]==sr[0]&&cycles[mode]==cycles[0],"copy CPU state/cycles diverged");
            }
            std::cout<<"[test] RC6.3.6 genuine 68000 copy block synthetic original/native/rollback PASS\n";
        }
#endif
        // RC6.3.2: real emulator execution with a deliberately synthetic ROM
        // and test-only DLL. Compare original/native/rollback traces without
        // trimming post-return transactions. The caller stack write must stay
        // OUT of all function-scoped bus traces, even when the native hook
        // redirects PC before Musashi invokes another instruction hook.
#if defined(CHQ_HEALTH_TEST_LIBRARY_PATH)
        {
            std::array<std::string,3> healthStates,healthBus,healthReports;
            std::array<std::uint32_t,3> cpuSR{},cpuPC{},cpuD0{},memScore{},stackValue{};
            for(int mode=0;mode<3;++mode){
                auto r=std::make_unique<chq::Runtime>(health_return_boundary_fixture(),sub_program());
                const auto folder=std::filesystem::temp_directory_path()/("chq-rc632-health-"+std::to_string(mode));
                std::filesystem::create_directories(folder);
                if(mode){
                    r->load_native_health_library(CHQ_HEALTH_TEST_LIBRARY_PATH);
                    r->enable_native_health_replacement(mode==2);
                }
                r->reset();
                check(r->debug_set_register(chq::BusSpace::Main,"PC",0xa10c),"health fixture PC");
                check(r->debug_set_register(chq::BusSpace::Main,"A5",0x108000),"health fixture A5");
                check(r->debug_set_register(chq::BusSpace::Main,"SP",0x102de0),"health fixture SP");
                check(r->debug_set_register(chq::BusSpace::Main,"D0",0),"health fixture D0");
                r->bus.write(0x102de0,0xa09c,4,chq::BusSpace::Main);
                r->bus.write(0x10012e,1,2,chq::BusSpace::Main);
                r->bus.write(0x1002ae,14,2,chq::BusSpace::Main);
                r->bus.write(0x1002ba,0x155,2,chq::BusSpace::Main);
                r->bus.write(0x1002d8,0,2,chq::BusSpace::Main);
                r->enable_native_health_state_capture(folder/"native-health-state.tsv");
                r->enable_native_dispatch_audit(folder/"native-dispatch.tsv");
                r->run(800);
                cpuSR[mode]=r->debug_get_register(chq::BusSpace::Main,"SR");
                cpuPC[mode]=r->debug_get_register(chq::BusSpace::Main,"PC");
                cpuD0[mode]=r->debug_get_register(chq::BusSpace::Main,"D0");
                memScore[mode]=r->bus.peek(0x1002d8,2,chq::BusSpace::Main);
                stackValue[mode]=r->bus.peek(0x102de0,4,chq::BusSpace::Main);
                r->write_native_execution_evidence(folder,"synthetic-health-return-boundary");
                r.reset();
                auto load=[&](const char* name){std::ifstream f(folder/name,std::ios::binary);
                    return std::string(std::istreambuf_iterator<char>(f),{});};
                healthStates[mode]=load("native-health-state.tsv");
                const auto dispatcher=load("native-dispatch.tsv");
                const auto want=mode==0?"original-rom":(mode==1?"native-commit":"original-fallback");
                check(dispatcher.find(std::string("A:0000A10C-0000A12A\ta10c\t")+want)!=std::string::npos,
                      "health result not audited by common dispatcher");
                healthBus[mode]=load("native-health-bus.tsv");
                if(mode) healthReports[mode]=load("native-health-report.json");
                std::filesystem::remove_all(folder);
            }
            check(!healthBus[0].empty(),"synthetic original health bus trace absent");
            check(healthBus[0].find("\tW\t102de0\t")==std::string::npos,"authentic caller write was attributed to health function");
            check(healthBus[0]==healthBus[1],"native health trace falsely includes caller stack write (or lost real bus access)");
            check(healthBus[0]==healthBus[2],"rollback health bus transaction parity failed");
            check(healthReports[1].find("\"commits\":1")!=std::string::npos,"synthetic generated native function did not commit");
            check(healthReports[2].find("\"rollbacks\":1")!=std::string::npos,"forced native mismatch did not roll back");
            check(healthReports[2].find("\"commits\":0")!=std::string::npos,"rollback committed speculative candidate");
            for(int mode=1;mode<3;++mode){
                check(cpuSR[mode]==cpuSR[0],"health synthetic CPU SR diverged");
                check(cpuPC[mode]==cpuPC[0],"health synthetic CPU PC diverged");
                check(cpuD0[mode]==cpuD0[0],"health synthetic CPU D0 diverged");
                check(memScore[mode]==memScore[0],"health synthetic score diverged");
                check(stackValue[mode]==stackValue[0],"caller stack write result diverged");
            }
            std::cout<<"[test] RC6.3.2 live hook caller-stack ownership, native and rollback PASS\n";
        }
        // RC6.3.5: capture actual CPU-A BSR target and the executed path
        // into the candidate tail, with strict original/native/rollback parity.
        {
            std::array<std::string,3> paths;
            for(int mode=0;mode<3;++mode) {
                auto r=std::make_unique<chq::Runtime>(health_enclosing_caller_fixture(),sub_program());
                const auto dir=std::filesystem::temp_directory_path()/("chq-rc635-real-caller-"+std::to_string(mode));
                std::filesystem::create_directories(dir);
                if(mode) {
                    r->load_native_health_library(CHQ_HEALTH_TEST_LIBRARY_PATH);
                    r->enable_native_health_replacement(mode==2);
                }
                r->reset();
                check(r->debug_set_register(chq::BusSpace::Main,"PC",0xa09a),"RC635 BSR PC");
                check(r->debug_set_register(chq::BusSpace::Main,"SP",0x102de4),"RC635 caller SP");
                check(r->debug_set_register(chq::BusSpace::Main,"A5",0x108000),"RC635 A5");
                check(r->debug_set_register(chq::BusSpace::Main,"D0",0),"RC635 D0");
                r->bus.write(0x10012e,1,2,chq::BusSpace::Main);
                r->bus.write(0x1002ae,14,2,chq::BusSpace::Main);
                r->bus.write(0x1002ba,0x155,2,chq::BusSpace::Main);
                r->bus.write(0x1002d8,0,2,chq::BusSpace::Main);
                r->enable_native_health_state_capture(dir/"native-health-state.tsv");
                r->run(900);
                r.reset();
                std::ifstream in(dir/"native-health-caller-path.tsv",std::ios::binary);
                paths[mode]=std::string(std::istreambuf_iterator<char>(in),{});
                in.close();
                std::filesystem::remove_all(dir);
            }
            check(paths[0].find("\ta09c\ta09a\ta080\ta09a,a080,a10c\n")!=std::string::npos,
                "true CPU-A BSR-to-earlier-entry path not captured");
            check(paths[1]==paths[0],"native altered pre-entry CPU-A call path");
            check(paths[2]==paths[0],"rollback altered pre-entry CPU-A call path");
            std::cout<<"[test] RC6.3.5 actual BSR-to-larger-function tail tracing PASS\n";
        }
        // RC6.3.3: real Musashi instruction-hook paths must reflect the
        // debugger-controlled entry state, NOT a predicted fixture label.
        // Synthetic ROM only: ROM-backed Windows equivalence is a distinct gate.
        for(const auto& testcase:std::array<std::pair<const char*,const char*>,4>{{
            {"bypass","a10c,a110,a120,a124,a128"},
            {"nonterminal","a10c,a110,a112,a116,a120,a124,a128"},
            {"terminal","a10c,a110,a112,a116,a118,a11e,a128"},
            {"overflow","a10c,a110,a112,a116,a120,a124,a128"}
        }}){
            auto r=std::make_unique<chq::Runtime>(health_return_boundary_fixture(),sub_program());
            const auto folder=std::filesystem::temp_directory_path()/(std::string("chq-rc633-")+testcase.first);
            std::filesystem::create_directories(folder);
            r->reset();
            check(r->debug_set_register(chq::BusSpace::Main,"PC",0xa10c),"synthetic health entry");
            check(r->debug_set_register(chq::BusSpace::Main,"A5",0x108000),"synthetic A5");
            check(r->debug_set_register(chq::BusSpace::Main,"SP",0x102de0),"synthetic SP");
            check(r->debug_set_register(chq::BusSpace::Main,"D0",0),"synthetic D0");
            r->bus.write(0x102de0,0xa09c,4,chq::BusSpace::Main);
            r->set_native_health_case(testcase.first);
            r->enable_native_health_state_capture(folder/"native-health-state.tsv");
            r->run(800);
            r.reset();
            auto load=[&](const char* name){std::ifstream f(folder/name,std::ios::binary);
                return std::string(std::istreambuf_iterator<char>(f),{});};
            const auto path=load("native-health-path.tsv");
            const auto seed=load("native-health-seed.tsv");
            check(path.find(std::string("\trom\t")+testcase.first+"\t"+testcase.second)!=std::string::npos,
                "actual Musashi health branch trace inconsistent with entry case");
            check(seed.find(std::string("\t")+testcase.first+"\t10012e\t")!=std::string::npos,
                "controlled RAM intervention missing evidence");
            std::filesystem::remove_all(folder);
        }
        std::cout<<"[test] RC6.3.3 controlled entry cases/actual ROM paths PASS\n";
#endif
        // RC4.3 integration: instruction census and entry observation must be
        // attached to the real Musashi dispatch, not only tested standalone.
        {
            auto emulator = std::make_unique<chq::Runtime>(call_program(), sub_program());
            emulator->enable_rom_execution_census();
            emulator->observe_native_entry(chq::BusSpace::Main, 0x100, "test-bsr");
            emulator->reset();
            emulator->run(10000);
            const auto d = std::filesystem::temp_directory_path() / "chq-rc43-runtime-census-test";
            std::filesystem::create_directories(d);
            emulator->write_native_execution_evidence(d, "unit-test-M68K");
            std::ifstream in(d / "rom-execution-census.json");
            const std::string json((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
            check(json.find("\"romInstructionHits\": 0") == std::string::npos,
                  "census did not count real 68000 instructions");
            in.close();
            std::ifstream bridge(d / "native-execution-bridge.json");
            const std::string observed((std::istreambuf_iterator<char>(bridge)), std::istreambuf_iterator<char>());
            check(observed.find("\"hits\":1") != std::string::npos,
                  "bridge did not observe actual Musashi entry");
            bridge.close();
            std::filesystem::remove_all(d);
        }
        // Instrument the actual Musashi PC hook at original selector/lookup
        // addresses using small synthetic programs and controlled RAM.
        for (bool accepted : {true,false}) {
            auto emulator=std::make_unique<chq::Runtime>(projection_hook_program(accepted),sub_program());
            emulator->enable_native_projection_shadow();
            emulator->reset();
            check(emulator->debug_set_register(chq::BusSpace::Main,"A1",0x10a080),"cannot seed A1");
            check(emulator->debug_set_register(chq::BusSpace::Main,"D2",0xffff7a00),"cannot seed D2");
            check(emulator->debug_set_register(chq::BusSpace::Main,"A5",0x108000),"cannot seed A5");
            emulator->bus.write(0x101a00,0x00158b87,4,chq::BusSpace::Main);
            emulator->bus.write(0x101a80,accepted?0x00158a07:0x00158a06,4,chq::BusSpace::Main);
            emulator->run(10000);
            const auto d=std::filesystem::temp_directory_path()/"chq-rc44-projection-hook-test";
            std::filesystem::create_directories(d);
            emulator->write_native_execution_evidence(d,"synthetic-instruction-path");
            std::ifstream in(d/"native-projection-shadow.json");
            check(bool(in),"live native projection shadow evidence missing");
            const std::string text((std::istreambuf_iterator<char>(in)),std::istreambuf_iterator<char>());
            check(text.find("\"matched\":1")!=std::string::npos,"projection shadow Musashi hook did not match");
            check(text.find("\"mismatched\":0")!=std::string::npos,"projection shadow Musashi hook mismatch");
            in.close();std::filesystem::remove_all(d);
        }
        std::cout << "[test] dual bus maps\n";
        auto bus = std::make_unique<chq::Bus>(main_program(), sub_program());
        bus->write(0x108010, 0xface, 2, chq::BusSpace::Main);
        check(bus->peek(0x108010, 2, chq::BusSpace::Sub) == 0xface, "shared RAM alias");
        bus->write(0x800000, 0x1234, 2, chq::BusSpace::Sub);
        check(bus->peek(0x800000, 2, chq::BusSpace::Sub) == 0x1234, "CPU B road RAM");
        check(bus->road_word_writes()[0] == 1, "road word write counter");
        check(bus->road_word_last_values()[0] == 0x1234, "road write-time last value");
        check(bus->road_word_nonzero_writes()[0] == 1, "road nonzero write counter");
        bus->write(0x800000, 0x0000, 2, chq::BusSpace::Sub);
        check(bus->road_word_value_changes()[0] == 1, "road write-time value change counter");
        check(bus->road_word_min_values()[0] == 0x0000 && bus->road_word_max_values()[0] == 0x1234, "road write-time min/max");
        bus->write(0x800002, 0x89abcdef, 4, chq::BusSpace::Sub);
        check(bus->road_word_writes()[1] == 1 && bus->road_word_writes()[2] == 1, "road long write counters");
        check(bus->peek(0x800000, 2, chq::BusSpace::Main) == 0xffff, "CPU B road aliases CPU A latch");
        check(bus->peek(0, 2, chq::BusSpace::Main) != bus->peek(0, 2, chq::BusSpace::Sub), "separate ROMs");

        std::cout << "[test] v0.66.4 palette-write trace\n";
        bus->set_trace_pc(chq::BusSpace::Main,0x123456);
        bus->set_ioc_debug_frame(77);
        bus->palette_trace_start({1037,1053},64);
        bus->write(0xa00000,1037,2,chq::BusSpace::Main);
        bus->write(0xa00002,0x18de,2,chq::BusSpace::Main);
        check(bus->palette_trace_events().size()==1,"palette trace first transaction");
        { const auto& e=bus->palette_trace_events().back(); check(e.frame==77 && e.pc==0x123456 && e.index==1037,"palette trace identity"); check(e.old_value==0x0000 && e.new_value==0x18de && e.changed,"palette trace old/new"); }
        bus->write(0xa00002,0x18de,2,chq::BusSpace::Main);
        check(bus->palette_trace_events().size()==2 && !bus->palette_trace_events().back().changed,"palette trace records unchanged write");
        bus->write(0xa00000,1038,2,chq::BusSpace::Main);
        bus->write(0xa00002,0x1111,2,chq::BusSpace::Main);
        check(bus->palette_trace_events().size()==2,"palette trace filter");
        bus->palette_trace_stop();
        bus->write(0xa00000,1037,2,chq::BusSpace::Main);
        bus->write(0xa00002,0x000e,2,chq::BusSpace::Main);
        check(bus->palette_trace_events().size()==2,"palette trace stop");
        bus->palette_trace_clear(); check(bus->palette_trace_events().empty(),"palette trace clear");

        std::cout << "[test] v0.66.5 generic memory-write trace\n";
        bus->set_trace_pc(chq::BusSpace::Main,0x004321);
        bus->set_ioc_debug_frame(88);
        bus->memory_trace_start(chq::BusSpace::Main,0x100303,1,1,64);
        bus->write(0x100303,0x14,1,chq::BusSpace::Main);
        check(bus->memory_trace_events().size()==1,"memory trace first write");
        { const auto& e=bus->memory_trace_events().back(); check(e.frame==88 && e.pc==0x004321 && e.address==0x100303 && e.size==1,"memory trace identity"); check(e.old_value==0 && e.new_value==0x14 && e.changed && e.write_count==1,"memory trace old/new/count"); }
        bus->write(0x100303,0x14,1,chq::BusSpace::Main);
        check(bus->memory_trace_events().size()==2 && !bus->memory_trace_events().back().changed,"memory trace records unchanged write");
        bus->write(0x100304,0x55,1,chq::BusSpace::Main);
        check(bus->memory_trace_events().size()==2,"memory trace range filter");
        bus->memory_trace_stop();
        bus->write(0x100303,0x00,1,chq::BusSpace::Main);
        check(bus->memory_trace_events().size()==2,"memory trace stop");
        bus->memory_trace_clear(); check(bus->memory_trace_events().empty(),"memory trace clear");

        std::cout << "[test] RC4 bounded memory read provenance\n";
        bus->set_trace_pc(chq::BusSpace::Main,0x00cafe);
        bus->set_ioc_debug_frame(89);
        bus->memory_trace_start(chq::BusSpace::Main,0x108010,2,2,64,true,false);
        const auto traced_read=bus->read(0x108010,2,chq::BusSpace::Main);
        check(traced_read==0xface,"memory read provenance returned value");
        check(bus->memory_trace_events().size()==1,"memory read provenance event");
        { const auto& e=bus->memory_trace_events().back(); check(e.access=='R' && e.frame==89 && e.pc==0x00cafe,"memory read provenance identity"); check(e.address==0x108010 && e.size==2 && e.old_value==0xface && e.new_value==0xface,"memory read provenance value"); }
        bus->write(0x108010,0xbeef,2,chq::BusSpace::Main);
        check(bus->memory_trace_events().size()==1,"read-only provenance excludes writes");
        bus->memory_trace_clear();
        bus->memory_trace_start(chq::BusSpace::Main,0x108010,2,2,64,true,true);
        (void)bus->read(0x108010,2,chq::BusSpace::Main);
        bus->write(0x108010,0xabcd,2,chq::BusSpace::Main);
        check(bus->memory_trace_events().size()==2 && bus->memory_trace_events()[0].access=='R' && bus->memory_trace_events()[1].access=='W',"rw provenance ordering");
        bus->memory_trace_stop(); bus->memory_trace_clear();

        std::cout << "[test] RC4 structured collision response events\n";
        bus->write(0x10042e,0x0001,2,chq::BusSpace::Main);
        bus->write(0x100420,0x0010a4c0,4,chq::BusSpace::Main);
        bus->write(0x10a044,0x9000,2,chq::BusSpace::Main);
        bus->set_trace_pc(chq::BusSpace::Main,0x00a1be);
        bus->set_ioc_debug_frame(2125);
        bus->write(0x10a044,0x8f00,2,chq::BusSpace::Main);
        check(bus->collision_response_events().size()==1,"collision lateral event captured");
        { const auto& e=bus->collision_response_events().back(); check(e.frame==2125 && e.pc==0x00a1be && e.lateral && !e.speed,"collision lateral identity"); check(e.interaction_type==1 && e.object_reference==0x0010a4c0,"collision interaction context"); check(e.old_value==0x9000 && e.attempted_value==0x8f00 && e.applied_value==0x8f00 && !e.suppressed,"collision authentic values"); }
        bus->set_trace_pc(chq::BusSpace::Main,0x008700);
        bus->write(0x10041c,0x00100000,4,chq::BusSpace::Main);
        const auto before_speed=bus->peek(0x10041c,4,chq::BusSpace::Main);
        bus->set_suppress_collision_shove(true);
        bus->set_trace_pc(chq::BusSpace::Main,0x00a200);
        bus->write(0x10041c,0x000c0000,4,chq::BusSpace::Main);
        check(bus->collision_response_events().size()==2,"collision speed event captured");
        { const auto& e=bus->collision_response_events().back(); check(e.speed && e.suppressed,"collision suppressed speed class"); check(e.old_value==before_speed && e.attempted_value==0x000c0000 && e.applied_value==before_speed,"collision suppressed values"); }
        check(bus->peek(0x10041c,4,chq::BusSpace::Main)==before_speed,"collision suppression preserves speed");
        bus->set_suppress_collision_shove(false); bus->clear_collision_response_events();
        check(bus->collision_response_events().empty(),"collision event clear");

        // v0.66.9.5-RC1.1: target-local physical overlap response is a distinct
        // structured class and is intentionally observational (not suppressed by
        // collision.response). Run010 proved PC A13C -> 0x10A084.
        bus->write(0x10a084,0x808a,2,chq::BusSpace::Main);
        bus->set_trace_pc(chq::BusSpace::Main,0x00a13c);
        bus->set_ioc_debug_frame(5588);
        bus->set_suppress_collision_shove(true);
        bus->write(0x10a084,0x80d5,2,chq::BusSpace::Main);
        check(bus->collision_response_events().size()==1,"target contact event captured");
        { const auto& e=bus->collision_response_events().back(); check(e.target_contact && !e.lateral && !e.speed,"target contact event class"); check(!e.suppressed && e.old_value==0x808a && e.applied_value==0x80d5,"target contact remains authentic under player-response suppression"); }
        check(bus->peek(0x10a084,2,chq::BusSpace::Main)==0x80d5,"target contact write applied");
        bus->set_suppress_collision_shove(false); bus->clear_collision_response_events();

        std::cout << "[test] RC4 sound command-interface events\n";
        bus->set_trace_pc(chq::BusSpace::Main,0x001234);
        bus->set_ioc_debug_frame(3000);
        bus->write(0x820000,0x5a,1,chq::BusSpace::Main);
        check(bus->sound_command_events().size()==1,"sound command event captured");
        { const auto& e=bus->sound_command_events().back(); check(e.frame==3000 && e.pc==0x001234 && e.address==0x820000 && e.size==1 && e.value==0x5a,"sound command event identity"); }
        bus->write(0x810000,0x11,1,chq::BusSpace::Main);
        check(bus->sound_command_events().size()==1,"sound command range filter");
        bus->clear_sound_command_events(); check(bus->sound_command_events().empty(),"sound command clear");

        std::cout << "[test] v0.60 Research Workbench memory interventions\n";
        bus->research_enable(true);
        bus->debug_write(0x108020, 0x1111, 2, chq::BusSpace::Main);
        check(bus->peek(0x108020,2,chq::BusSpace::Main)==0x1111, "debug write");
        const auto freeze_id=bus->research_add_patch(chq::BusSpace::Main,0x108020,2,0x2222,chq::LivePatchMode::Freeze);
        check(freeze_id!=0,"freeze patch id");
        bus->write(0x108020,0x3333,2,chq::BusSpace::Main);
        check(bus->peek(0x108020,2,chq::BusSpace::Main)==0x2222,"freeze write interception");
        bus->research_clear_patches();
        bus->debug_write(0x108020,0x4444,2,chq::BusSpace::Main);
        bus->research_add_patch(chq::BusSpace::Main,0x108020,2,0,chq::LivePatchMode::Suppress);
        bus->write(0x108020,0x5555,2,chq::BusSpace::Main);
        check(bus->peek(0x108020,2,chq::BusSpace::Main)==0x4444,"suppress write interception");
        bus->research_clear_patches();
        // v0.64.8.4: intervention patches are controls and must enforce even when
        // research event collection is disabled. This mirrors normal Workbench use.
        bus->research_enable(false);
        bus->debug_write(0x108020, 0x6666, 2, chq::BusSpace::Main);
        const auto offline_freeze_id=bus->research_add_patch(chq::BusSpace::Main,0x108020,2,0x7777,chq::LivePatchMode::Freeze);
        bus->write(0x108020,0x8888,2,chq::BusSpace::Main);
        check(bus->peek(0x108020,2,chq::BusSpace::Main)==0x7777,"freeze enforcement independent of research tracing");
        check(!bus->research_patches().empty() && bus->research_patches().front().interceptions==1,"freeze hit count independent of research tracing");
        check(offline_freeze_id!=0,"offline freeze patch id");
        bus->research_clear_patches();
        bus->research_enable(true);
        chq::ResearchWatch rw{}; rw.space=chq::BusSpace::Main; rw.start=rw.end=0x108024; rw.writes=true; rw.change_only=true; rw.break_on_match=true; bus->research_add_watch(rw);
        bus->write(0x108024,0x7777,2,chq::BusSpace::Main);
        check(bus->research_break_pending(),"watch break pending");
        check(!bus->research_take_break_reason().empty(),"watch break reason");
        check(!bus->research_events().empty(),"research event history");
        bus->research_clear_watches(); bus->research_clear_events();

        std::cout << "[test] TC0040IOC selected-port model\n";
        bus->write(0x400003, 0x00, 1); // select DSWA
        check(bus->read(0x400001, 1) == 0xff, "DSWA factory default");
        bus->write(0x400003, 0x01, 1); // select DSWB
        check(bus->read(0x400001, 1) == 0xff, "DSWB factory default");
        bus->write(0x400003, 0x02, 1); // IN0
        check(bus->read(0x400001, 1) == 0x33, "IN0 inactive defaults");
        bus->write(0x400003, 0x03, 1); // IN1
        check(bus->read(0x400001, 1) == 0x3f, "IN1 inactive defaults");
        bus->write(0x400003, 0x0c, 1); // steering low
        check(bus->read(0x400001, 1) == 0x00, "steering low centered");
        bus->write(0x400003, 0x0d, 1); // steering high
        check(bus->read(0x400001, 1) == 0x00, "steering high centered");
        bus->set_ioc_steering(0x0060);
        bus->write(0x400003, 0x0c, 1); check(bus->read(0x400001,1)==0x60, "steering +96 low byte");
        bus->write(0x400003, 0x0d, 1); check(bus->read(0x400001,1)==0x00, "steering +96 high byte");
        bus->set_ioc_steering(0x0fa0);
        bus->write(0x400003, 0x0c, 1); check(bus->read(0x400001,1)==0xa0, "steering -96 low byte");
        bus->write(0x400003, 0x0d, 1); check(bus->read(0x400001,1)==0x0f, "steering -96 high byte");
        bus->set_ioc_steering(0x0000);
        check(bus->read(0x400003, 1) == 0xff, "IOC watchdog read");
        bus->set_ioc_input_xor_mask(0x02, 0x10);
        bus->write(0x400003, 0x02, 1);
        check(bus->read(0x400001, 1) == 0x23, "IOC debugger XOR override");
        check(bus->ioc_input_xor_mask(0x02) == 0x10, "IOC debugger layer retained");
        bus->set_ioc_scenario_xor_mask(0x02, 0x20);
        check(bus->ioc_scenario_xor_mask(0x02) == 0x20, "IOC scenario layer set");
        check(bus->ioc_effective_xor_mask(0x02) == 0x30, "IOC XOR layers compose");
        check(bus->read(0x400001, 1) == 0x03, "IOC debugger + scenario XOR combined");
        bus->clear_ioc_scenario_overrides();
        check(bus->read(0x400001, 1) == 0x23, "clearing scenario preserves debugger XOR");
        bus->clear_ioc_input_overrides();
        check(bus->read(0x400001, 1) == 0x33, "IOC debugger override clear");

        {
            std::cout << "[test] input pulse option parsing\n";
            char a0[] = "runtime_tests";
            char a1[] = "--pulse-ioc";
            char a2[] = "2:10:120:5";
            char* argv[] = {a0, a1, a2};
            const auto opts = chq::parse_options(3, argv);
            check(opts.input_pulses.size() == 1, "input pulse option missing");
            check(opts.input_pulses[0].port == 0x02, "input pulse port parse");
            check(opts.input_pulses[0].mask == 0x10, "input pulse mask parse");
            check(opts.input_pulses[0].start_frame == 120, "input pulse frame parse");
            check(opts.input_pulses[0].duration_frames == 5, "input pulse duration parse");
        }

        {
            std::cout << "[test] repeated input pulse option parsing\n";
            char a0[] = "runtime_tests";
            char a1[] = "--pulse-ioc";
            char a2[] = "2:10:120:5";
            char a3[] = "--pulse-ioc";
            char a4[] = "2:10:180:5";
            char a5[] = "--pulse-ioc";
            char a6[] = "2:10:240:5";
            char* argv[] = {a0, a1, a2, a3, a4, a5, a6};
            const auto opts = chq::parse_options(7, argv);
            check(opts.input_pulses.size() == 3, "repeated input pulse options were not accumulated");
            check(opts.input_pulses[0].start_frame == 120, "first repeated pulse frame parse");
            check(opts.input_pulses[1].start_frame == 180, "second repeated pulse frame parse");
            check(opts.input_pulses[2].start_frame == 240, "third repeated pulse frame parse");
            check(opts.input_pulses[0].port == 0x02 && opts.input_pulses[1].port == 0x02 && opts.input_pulses[2].port == 0x02,
                  "repeated pulse ports parse");
            check(opts.input_pulses[0].mask == 0x10 && opts.input_pulses[1].mask == 0x10 && opts.input_pulses[2].mask == 0x10,
                  "repeated pulse masks parse");
        }

        {
            std::cout << "[test] RC2.9 one-hit target option parsing\n";
            const char* argv[] = {"runtime", "--one-hit-target"};
            const auto opts = chq::parse_options(static_cast<int>(std::size(argv)), const_cast<char**>(argv));
            check(opts.target_one_hit, "one-hit target option parse");
        }

        {
            std::cout << "[test] v0.27 conditional trace option parsing\n";
            const char* argv[] = {"runtime", "--trace-mem-change", "100080", "--trace-from-frame", "1200", "--trace-to-frame", "1600", "--trace-arm-mem", "100172:3800", "--trace-task", "100080", "--trace-write-value", "0"};
            const auto opts = chq::parse_options(static_cast<int>(std::size(argv)), const_cast<char**>(argv));
            check(opts.trace.mem_ranges.size() == 1 && opts.trace.mem_ranges[0].change_only, "change-only trace parse");
            check(opts.trace.from_frame == 1200 && opts.trace.to_frame == 1600, "trace frame window parse");
            check(opts.trace.arm_mem_writes.size() == 1 && opts.trace.arm_mem_writes[0].address == 0x100172 && opts.trace.arm_mem_writes[0].value == 0x3800, "trace arm mem parse");
            check(opts.trace.task_slots.size() == 1 && opts.trace.task_slots[0] == 0x100080, "trace task parse");
            check(opts.trace.write_value_filter && *opts.trace.write_value_filter == 0, "trace write value parse");
        }

        {
            std::cout << "[test] v0.51 PROM mixer default and explicit overrides\n";
            const char* argv0[] = {"runtime"};
            const auto def = chq::parse_options(static_cast<int>(std::size(argv0)), const_cast<char**>(argv0));
            check(def.mixer == chq::Options::MixerChoice::Prom, "PROM is not the default mixer");
            const char* argv1[] = {"runtime", "--mixer", "reference"};
            const auto ref = chq::parse_options(static_cast<int>(std::size(argv1)), const_cast<char**>(argv1));
            check(ref.mixer == chq::Options::MixerChoice::Reference, "reference mixer override parse");
            const char* argv2[] = {"runtime", "--mixer", "legacy"};
            const auto legacy = chq::parse_options(static_cast<int>(std::size(argv2)), const_cast<char**>(argv2));
            check(legacy.mixer == chq::Options::MixerChoice::Legacy, "legacy mixer override parse");
        }

        {
            std::cout << "[test] v0.50 multi-value sprite selector and quad candidate parsing\n";
            const char* argv[] = {"runtime", "--sprite-solo", "map=778|779|780|781,palette=134", "--sprite-quad-candidate", "name=quad778,tl=778,tr=779,bl=780,br=781,palette=134"};
            const auto opts = chq::parse_options(static_cast<int>(std::size(argv)), const_cast<char**>(argv));
            check(opts.sprite_solo.size() == 1, "sprite solo selector missing");
            check(opts.sprite_solo[0].maps.size() == 4 && opts.sprite_solo[0].matches(0,778,134,0) && opts.sprite_solo[0].matches(0,781,134,0), "multi-map selector parse/match");
            check(!opts.sprite_solo[0].matches(0,817,134,0) && !opts.sprite_solo[0].matches(0,778,197,0), "multi-map selector rejection");
            check(opts.sprite_quad_candidates.size() == 1 && opts.sprite_quad_candidates[0].tl == 778 && opts.sprite_quad_candidates[0].br == 781 && opts.sprite_quad_candidates[0].palette == 134, "quad candidate parse");
        }

        {
            std::cout << "[test] v0.50.3 semantic player-car CLI parsing\n";
            const char* argv[] = {"runtime", "--object-solo", "player_car", "--object-evidence", "player_car", "--object-track", "player_car"};
            const auto opts = chq::parse_options(static_cast<int>(std::size(argv)), const_cast<char**>(argv));
            check(opts.sprite_solo.size() == 1, "player_car solo selector missing");
            check(opts.sprite_solo[0].matches(0,472,64,0) && opts.sprite_solo[0].matches(0,594,70,0), "player_car semantic selector membership");
            check(!opts.sprite_solo[0].matches(0,778,134,0), "player_car semantic selector rejection");
            check(opts.object_evidence.size() == 1 && opts.object_evidence[0] == "player_car", "player_car evidence parse");
            check(opts.object_track.size() == 1 && opts.object_track[0] == "player_car", "player_car track parse");
        }

        {
            std::cout << "[test] v0.59.1 target/collision options and generalized response suppression\n";
            const char* argv[] = {"runtime", "--target-state-log", "--no-collisions", "--course-follow-controller", "predictive", "--course-follow-lateral-kd", "0.02", "--course-follow-slew", "6", "--course-follow-speed-control", "on", "--course-follow-lateral-bias", "384", "--progress-status", "240"};
            const auto opts = chq::parse_options(static_cast<int>(std::size(argv)), const_cast<char**>(argv));
            check(opts.target_state_log, "target-state log option parse");
            check(opts.no_collisions, "no-collisions option parse");
            check(opts.course_follow_controller == "predictive" && opts.course_follow_lateral_kd == 0.02 && opts.course_follow_slew == 6 && opts.course_follow_speed_control && opts.course_follow_lateral_bias == 384, "predictive course-follow parse");
            check(opts.progress_status_every == 240, "progress status parse");
            auto collision_bus = std::make_unique<chq::Bus>(main_program(), sub_program());
            collision_bus->write(0x10a044, 0x7b6b, 2, chq::BusSpace::Main);
            collision_bus->write(0x10041c, 0x00123456, 4, chq::BusSpace::Main);
            collision_bus->set_suppress_collision_shove(true);
            for (auto pc : {0x00a142u,0x00a156u,0x00a1beu,0x00a1c4u}) {
                collision_bus->set_trace_pc(chq::BusSpace::Main, pc);
                collision_bus->write(0x10a044, 0x7dd9, 2, chq::BusSpace::Main);
                check(collision_bus->peek(0x10a044, 2, chq::BusSpace::Main) == 0x7b6b, "collision lateral response suppression failed");
            }
            collision_bus->set_trace_pc(chq::BusSpace::Main, 0x00a200);
            collision_bus->write(0x10041c, 0x000d0000, 4, chq::BusSpace::Main);
            check(collision_bus->peek(0x10041c, 4, chq::BusSpace::Main) == 0x00123456, "collision speed penalty suppression failed");
            check(collision_bus->suppressed_collision_shoves() == 5, "collision response suppression counter");
            check(collision_bus->suppressed_collision_lateral() == 4 && collision_bus->suppressed_collision_speed() == 1, "collision suppression category counters");
            collision_bus->set_trace_pc(chq::BusSpace::Main, 0x008000);
            collision_bus->write(0x10a044, 0x7000, 2, chq::BusSpace::Main);
            check(collision_bus->peek(0x10a044, 2, chq::BusSpace::Main) == 0x7000, "non-collision lateral write was incorrectly suppressed");
        }

        {
            std::cout << "[test] v0.59.3 handling override and course-profile options\n";
            const char* argv[] = {"runtime", "--cornering-scale", "1.25", "--cornering-speed-retain", "1.05", "--course-profile", "driver_profile.csv"};
            const auto opts = chq::parse_options(static_cast<int>(std::size(argv)), const_cast<char**>(argv));
            check(std::abs(opts.cornering_scale - 1.25) < 1e-9, "cornering-scale parse");
            check(std::abs(opts.cornering_speed_retain - 1.05) < 1e-9, "cornering-speed-retain parse");
            check(opts.course_follow && opts.course_follow_controller == "profile" && opts.course_profile_file == std::filesystem::path("driver_profile.csv"), "course profile parse");
        }

        {
            std::cout << "[test] v0.66.9.4 police beacon override lifecycle\n";
            const auto state_path = std::filesystem::temp_directory_path() / "chq_beacon_override_test.chqstate";
            std::filesystem::remove(state_path);
            auto runtime = std::make_unique<chq::Runtime>(main_program(), sub_program());
            runtime->reset();
            runtime->bus.debug_write(0x101854, 0x0002, 2, chq::BusSpace::Main);
            runtime->bus.debug_write(0x108082, 0x1234, 2, chq::BusSpace::Main);
            runtime->save_checkpoint(state_path, 77);

            runtime->set_beacon_override(chq::Runtime::BeaconOverrideMode::ForceOn);
            check(runtime->beacon_override()==chq::Runtime::BeaconOverrideMode::ForceOn, "beacon force-on mode");
            check(runtime->bus.peek(0x101854,2,chq::BusSpace::Main)==0x0006, "beacon force-on immediate phase");
            check(runtime->bus.peek(0x108082,2,chq::BusSpace::Main)==0xffff, "beacon force-on deployment request");
            runtime->bus.write(0x101854,0x0003,2,chq::BusSpace::Main);
            runtime->bus.write(0x108082,0x0000,2,chq::BusSpace::Main);
            check(runtime->bus.peek(0x101854,2,chq::BusSpace::Main)==0x0006, "beacon force-on write interception");
            check(runtime->bus.peek(0x108082,2,chq::BusSpace::Main)==0xffff, "beacon force-on request interception");

            runtime->set_beacon_override(chq::Runtime::BeaconOverrideMode::ForceOff);
            check(runtime->bus.peek(0x101854,2,chq::BusSpace::Main)==0x0000, "beacon force-off immediate phase");
            check(runtime->bus.peek(0x108082,2,chq::BusSpace::Main)==0x0000, "beacon force-off deployment request");
            runtime->bus.write(0x101854,0x0006,2,chq::BusSpace::Main);
            check(runtime->bus.peek(0x101854,2,chq::BusSpace::Main)==0x0000, "beacon force-off write interception");

            runtime->set_beacon_override(chq::Runtime::BeaconOverrideMode::Authentic);
            runtime->bus.write(0x101854,0x0004,2,chq::BusSpace::Main);
            check(runtime->bus.peek(0x101854,2,chq::BusSpace::Main)==0x0004, "beacon authentic mode must not intercept");

            runtime->set_beacon_override(chq::Runtime::BeaconOverrideMode::ForceOn);
            const auto restored = runtime->load_checkpoint(state_path);
            check(restored==77, "beacon checkpoint restore frame");
            check(runtime->beacon_override()==chq::Runtime::BeaconOverrideMode::Authentic, "checkpoint load must clear beacon override");
            check(runtime->bus.peek(0x101854,2,chq::BusSpace::Main)==0x0002, "checkpoint load restored authentic beacon phase");
            check(runtime->bus.peek(0x108082,2,chq::BusSpace::Main)==0x1234, "checkpoint load restored authentic beacon request");
            std::filesystem::remove(state_path);
        }

        {
            std::cout << "[test] dual CPU scheduler/shared RAM\n";
            auto runtime = std::make_unique<chq::Runtime>(main_program(), sub_program());
            runtime->reset();
            runtime->run(5000);
            check(runtime->bus.peek(0x108000, 2) == 0x1234, "CPU A shared write");
            check(runtime->bus.peek(0x108002, 2) == 0x1235, "CPU B shared communication");
            check(runtime->bus.peek(0x800000, 2, chq::BusSpace::Sub) == 0xbeef, "CPU B road write");
            check(runtime->instructions_a > 0 && runtime->instructions_b > 0, "both CPUs executed");
        }

        {
            std::cout << "[test] checkpoint resume after runtime reconstruction\n";
            const auto state_path = std::filesystem::temp_directory_path() / "chq_checkpoint_resume_test.chqstate";
            std::filesystem::remove(state_path);
            {
                auto first = std::make_unique<chq::Runtime>(main_program(), sub_program());
                first->reset();
                first->run(5000);
                first->save_checkpoint(state_path, 123);
            }
            // Simulate loading the checkpoint in another ASLR process by deliberately
            // poisoning the host-process pointers serialized inside both Musashi v1 contexts.
            // v0.49.4 must repair these before the first resumed instruction executes.
            {
                std::fstream f(state_path, std::ios::in | std::ios::out | std::ios::binary);
                check(bool(f), "checkpoint poison open");
                const std::size_t context_size = sizeof(m68ki_cpu_core);
                const std::size_t context_a = 20; // magic + version + frame + first blob length
                const std::size_t context_b = context_a + context_size + 4; // second blob length
                const std::size_t pointer_offsets[] = {
                    offsetof(m68ki_cpu_core, cyc_instruction), offsetof(m68ki_cpu_core, cyc_exception),
                    offsetof(m68ki_cpu_core, int_ack_callback), offsetof(m68ki_cpu_core, bkpt_ack_callback),
                    offsetof(m68ki_cpu_core, reset_instr_callback), offsetof(m68ki_cpu_core, cmpild_instr_callback),
                    offsetof(m68ki_cpu_core, rte_instr_callback), offsetof(m68ki_cpu_core, tas_instr_callback),
                    offsetof(m68ki_cpu_core, illg_instr_callback), offsetof(m68ki_cpu_core, trap_instr_callback),
                    offsetof(m68ki_cpu_core, pc_changed_callback), offsetof(m68ki_cpu_core, set_fc_callback),
                    offsetof(m68ki_cpu_core, instr_hook_callback)
                };
                for (const auto base : {context_a, context_b}) {
                    for (const auto off : pointer_offsets) {
                        f.seekp(static_cast<std::streamoff>(base + off));
                        std::uintptr_t poison = static_cast<std::uintptr_t>(0x1);
                        f.write(reinterpret_cast<const char*>(&poison), sizeof(poison));
                    }
                }
                check(bool(f), "checkpoint poison write");
            }
            {
                auto resumed = std::make_unique<chq::Runtime>(main_program(), sub_program());
                resumed->reset();
                const auto restored_frame = resumed->load_checkpoint(state_path);
                check(restored_frame == 123, "checkpoint frame restore");
                resumed->run(5000);
                check(resumed->fault.empty(), "checkpoint resume fault");
                check(resumed->instructions_a > 0, "checkpoint CPU A did not resume");
            }
            std::filesystem::remove(state_path);
        }

        {
            std::cout << "[test] generic runtime tracer\n";
            const auto trace_path = std::filesystem::temp_directory_path() / "chq_v026_trace_test.log";
            std::filesystem::remove(trace_path);
            {
                auto traced = std::make_unique<chq::Runtime>(main_program(), sub_program());
                chq::TraceConfig cfg;
                cfg.cpu_mask = 2;
                cfg.max_lines = 1000;
                cfg.before = 8;
                cfg.after = 8;
                cfg.mem_ranges.push_back({0x108002, 0x108003, false, true, false});
                cfg.pc_ranges.push_back({0x100, 0x120});
                cfg.triggers.push_back({0x108002, 0x108003, chq::TraceTriggerKind::Write});
                traced->configure_generic_trace(cfg, trace_path);
                traced->reset();
                traced->run(5000);
            }
            std::ifstream tf(trace_path);
            const std::string text((std::istreambuf_iterator<char>(tf)), std::istreambuf_iterator<char>());
            tf.close();
            check(text.find("MEM_W frame=0 cpu=B") != std::string::npos, "generic memory trace missing");
            check(text.find("TRIGGER frame=0 cpu=B") != std::string::npos, "generic trigger missing");
            check(text.find("EXEC frame=0 cpu=B") != std::string::npos, "generic PC trace missing");
            std::filesystem::remove(trace_path);
        }

        {
            std::cout << "[test] generic memory PC gate\n";
            const auto trace_path = std::filesystem::temp_directory_path() / "chq_v0262_mem_pc_trace_test.log";
            std::filesystem::remove(trace_path);
            {
                auto traced = std::make_unique<chq::Runtime>(main_program(), sub_program());
                chq::TraceConfig cfg;
                cfg.cpu_mask = 2;
                cfg.max_lines = 1000;
                cfg.mem_ranges.push_back({0x108002, 0x108003, false, true, false});
                cfg.mem_pc_ranges.push_back({0x100, 0x107}); // excludes CPU-B write at 0x108
                traced->configure_generic_trace(cfg, trace_path);
                traced->reset();
                traced->run(5000);
            }
            std::ifstream tf(trace_path);
            const std::string text((std::istreambuf_iterator<char>(tf)), std::istreambuf_iterator<char>());
            tf.close();
            check(text.find("MEM_W frame=0 cpu=B") == std::string::npos, "memory PC gate failed to suppress out-of-range write");
            check(text.find("# MEM_PC 000100-000107") != std::string::npos, "memory PC gate header missing");
            std::filesystem::remove(trace_path);
        }

        {
            std::cout << "[test] generic memory PC gate matching write\n";
            const auto trace_path = std::filesystem::temp_directory_path() / "chq_v0262_mem_pc_trace_match_test.log";
            std::filesystem::remove(trace_path);
            {
                auto traced = std::make_unique<chq::Runtime>(main_program(), sub_program());
                chq::TraceConfig cfg;
                cfg.cpu_mask = 2;
                cfg.max_lines = 1000;
                cfg.mem_ranges.push_back({0x108002, 0x108003, false, true, false});
                cfg.mem_pc_ranges.push_back({0x108, 0x10d});
                traced->configure_generic_trace(cfg, trace_path);
                traced->reset();
                traced->run(5000);
            }
            std::ifstream tf(trace_path);
            const std::string text((std::istreambuf_iterator<char>(tf)), std::istreambuf_iterator<char>());
            tf.close();
            check(text.find("MEM_W frame=0 cpu=B") != std::string::npos, "memory PC gate suppressed matching write");
            std::filesystem::remove(trace_path);
        }

        {
            std::cout << "[test] v0.27 memory arming + frame tag\n";
            const auto trace_path = std::filesystem::temp_directory_path() / "chq_v027_arm_trace_test.log";
            std::filesystem::remove(trace_path);
            {
                auto traced = std::make_unique<chq::Runtime>(main_program(), sub_program());
                chq::TraceConfig cfg;
                cfg.cpu_mask = 1;
                cfg.max_lines = 100;
                cfg.mem_ranges.push_back({0x100080, 0x100081, false, true, false, false});
                cfg.arm_mem_writes.push_back({0x100172, 0x3800});
                traced->configure_generic_trace(cfg, trace_path);
                traced->set_trace_frame(42);
                traced->bus.set_trace_pc(chq::BusSpace::Main, 0x2000);
                traced->bus.write(0x100080, 0x1111, 2, chq::BusSpace::Main); // must be silent
                traced->bus.write(0x100172, 0x3800, 4, chq::BusSpace::Main); // arms
                traced->bus.write(0x100080, 0x2222, 2, chq::BusSpace::Main); // logged
            }
            std::ifstream tf(trace_path);
            const std::string text((std::istreambuf_iterator<char>(tf)), std::istreambuf_iterator<char>());
            tf.close();
            check(text.find("TRACE_ARM frame=42") != std::string::npos, "memory arm event missing");
            check(text.find("value=2222") != std::string::npos, "post-arm memory write missing");
            check(text.find("value=1111") == std::string::npos, "pre-arm memory write leaked");
            std::filesystem::remove(trace_path);
        }

        {
            std::cout << "[test] v0.27 change-only memory trace\n";
            const auto trace_path = std::filesystem::temp_directory_path() / "chq_v027_change_trace_test.log";
            std::filesystem::remove(trace_path);
            {
                auto traced = std::make_unique<chq::Runtime>(main_program(), sub_program());
                chq::TraceConfig cfg;
                cfg.cpu_mask = 1;
                cfg.max_lines = 100;
                cfg.mem_ranges.push_back({0x100080, 0x100081, false, true, false, true});
                traced->configure_generic_trace(cfg, trace_path);
                traced->bus.set_trace_pc(chq::BusSpace::Main, 0x2000);
                traced->bus.write(0x100080, 1, 2, chq::BusSpace::Main);
                traced->bus.write(0x100080, 1, 2, chq::BusSpace::Main);
            }
            std::ifstream tf(trace_path);
            const std::string text((std::istreambuf_iterator<char>(tf)), std::istreambuf_iterator<char>());
            tf.close();
            const auto first = text.find("MEM_W frame=0");
            check(first != std::string::npos, "change-only first change missing");
            check(text.find("MEM_W frame=0", first + 1) == std::string::npos, "change-only logged unchanged rewrite");
            std::filesystem::remove(trace_path);
        }

        {
            std::cout << "[test] v0.27 task-state decoder\n";
            const auto trace_path = std::filesystem::temp_directory_path() / "chq_v027_task_trace_test.log";
            std::filesystem::remove(trace_path);
            {
                auto traced = std::make_unique<chq::Runtime>(main_program(), sub_program());
                chq::TraceConfig cfg;
                cfg.cpu_mask = 1;
                cfg.max_lines = 100;
                cfg.task_slots.push_back(0x100080);
                traced->configure_generic_trace(cfg, trace_path);
                traced->set_trace_frame(77);
                traced->bus.set_trace_pc(chq::BusSpace::Main, 0x1096);
                traced->bus.write(0x100080, 1, 2, chq::BusSpace::Main);
                traced->bus.write(0x100080, 1, 2, chq::BusSpace::Main);
                traced->bus.write(0x100080, 0, 2, chq::BusSpace::Main);
            }
            std::ifstream tf(trace_path);
            const std::string text((std::istreambuf_iterator<char>(tf)), std::istreambuf_iterator<char>());
            tf.close();
            check(text.find("TASK_STATE frame=77 slot=100080 old=0000 new=0001") != std::string::npos, "task activation missing");
            check(text.find("TASK_STATE frame=77 slot=100080 old=0001 new=0000") != std::string::npos, "task termination missing");
            std::filesystem::remove(trace_path);
        }

        {
            std::cout << "[test] v0.43 provenance option parsing\n";
            const char* argv[] = {"runtime", "--follow-address", "108002:108003", "--provenance-from-frame", "10", "--provenance-to-frame", "20", "--provenance-depth", "12", "--fast-forward-to", "100"};
            const auto opts = chq::parse_options(static_cast<int>(std::size(argv)), const_cast<char**>(argv));
            check(opts.provenance.enabled(), "provenance options not enabled");
            check(opts.provenance.follow_ranges.size() == 1 && opts.provenance.follow_ranges[0].start == 0x108002, "follow range parse");
            check(opts.provenance.from_frame == 10 && opts.provenance.to_frame == 20, "provenance frame window parse");
            check(opts.provenance.max_depth == 12, "provenance depth parse");
            check(opts.fast_forward_to == 100, "fast-forward parse");
        }

        {
            std::cout << "[test] v0.43 address provenance export\n";
            const auto dir = std::filesystem::temp_directory_path() / "chq_v042_provenance";
            std::filesystem::remove_all(dir);
            {
                auto traced = std::make_unique<chq::Runtime>(main_program(), sub_program());
                chq::ProvenanceConfig cfg;
                cfg.follow_ranges.push_back({0x108002,0x108003});
                cfg.trace_callstack = true;
                cfg.trace_callgraph = true;
                cfg.profile_functions = true;
                traced->configure_provenance(cfg, dir);
                traced->reset();
                traced->run(5000);
                traced->write_provenance_reports();
            }
            std::ifstream pf(dir / "provenance_address_flow.csv");
            const std::string ptext((std::istreambuf_iterator<char>(pf)), std::istreambuf_iterator<char>());
            pf.close();
            check(ptext.find("108002") != std::string::npos, "provenance address flow missing watched access");
            check(std::filesystem::exists(dir / "provenance_callgraph.csv"), "provenance callgraph missing");
            check(std::filesystem::exists(dir / "provenance_functions.csv"), "provenance function profile missing");
            std::filesystem::remove_all(dir);
        }

        {
            std::cout << "[test] v0.43 dynamic callgraph reconstruction\n";
            const auto dir = std::filesystem::temp_directory_path() / "chq_v042_callgraph";
            std::filesystem::remove_all(dir);
            {
                auto traced = std::make_unique<chq::Runtime>(call_program(), sub_program());
                chq::ProvenanceConfig cfg;
                cfg.follow_ranges.push_back({0x108020,0x108021});
                cfg.trace_callstack=true; cfg.trace_callgraph=true; cfg.profile_functions=true;
                traced->configure_provenance(cfg, dir);
                traced->reset();
                traced->run(1000);
                traced->write_provenance_reports();
            }
            std::ifstream cf(dir / "provenance_callgraph.csv");
            const std::string ctext((std::istreambuf_iterator<char>(cf)), std::istreambuf_iterator<char>());
            cf.close();
            check(ctext.find("000100,000120") != std::string::npos, "dynamic callgraph did not resolve BSR target");
            std::filesystem::remove_all(dir);
        }


        {
            std::cout << "[test] v0.43 focused provenance + forensic workflow parsing\n";
            const char* argv[] = {"runtime", "--follow-address", "10a076", "--pixel-provenance-pair", "44:45", "--pixel-provenance-max", "8", "--break-frame", "1627", "--break-frame-continue", "--break-history", "4096", "--auto-zip-logs", "--zip-name", "evidence.zip", "--batch", "jobs.txt"};
            const auto opts = chq::parse_options(static_cast<int>(std::size(argv)), const_cast<char**>(argv));
            check(opts.provenance.enabled(), "focused provenance not enabled");
            check(!opts.provenance.trace_callgraph && !opts.provenance.profile_functions, "follow-address should be focused by default in v0.43");
            check(opts.graphics_trace.pixel_provenance && opts.graphics_trace.pixel_provenance_pair.has_value(), "pixel provenance pair parse");
            check(opts.graphics_trace.pixel_provenance_pair->first == 44 && opts.graphics_trace.pixel_provenance_pair->second == 45, "pixel provenance pair values");
            check(opts.graphics_trace.pixel_provenance_max == 8, "pixel provenance max parse");
            check(opts.break_frames.size() == 1 && opts.break_frames[0] == 1627 && opts.break_frame_continue, "break frame parse");
            check(opts.break_history == 4096, "break history parse");
            check(opts.auto_zip_logs && opts.zip_name == "evidence.zip", "auto zip parse");
            check(opts.batch_file == "jobs.txt", "batch file parse");
        }

        {
            std::cout << "[test] v0.44 graphics experiment parsing\n";
            const char* argv[] = {"runtime", "--gfx-trace-pixel", "150:224", "--experiment-slot-zero-transparent", "44", "--experiment-palette-zero-transparent", "70", "--experiment-sprite-pair-order", "44:45", "--experiment-sprite-order", "ascending", "--experiment-layer-order", "bottom,upper,road,sprites,text", "--experiment-layer-matrix"};
            const auto opts = chq::parse_options(static_cast<int>(std::size(argv)), const_cast<char**>(argv));
            check(opts.graphics_trace.explicit_location, "explicit graphics location parse");
            check(opts.graphics_trace.experiment_zero_slot && *opts.graphics_trace.experiment_zero_slot == 44, "slot zero experiment parse");
            check(opts.graphics_trace.experiment_zero_palette_bank && *opts.graphics_trace.experiment_zero_palette_bank == 70, "palette zero experiment parse");
            check(opts.graphics_trace.experiment_sprite_pair_order && opts.graphics_trace.experiment_sprite_pair_order->first == 44 && opts.graphics_trace.experiment_sprite_pair_order->second == 45, "pair order experiment parse");
            check(opts.graphics_trace.experiment_sprite_order == "ascending", "sprite order experiment parse");
            check(opts.graphics_trace.experiment_layer_order.size() == 5, "layer order experiment parse");
            check(opts.graphics_trace.experiment_layer_matrix, "layer matrix experiment parse");
        }

        {
            std::cout << "[test] v0.45 multi-pixel + known-bad regression set parsing\n";
            const char* argv[] = {"runtime", "--gfx-trace-pixels", "99:165,150:224", "--pixel-provenance-pixels", "[(122,239),(197,239)]"};
            const auto opts = chq::parse_options(static_cast<int>(std::size(argv)), const_cast<char**>(argv));
            check(opts.graphics_trace.explicit_location, "multi-pixel explicit location parse");
            check(opts.graphics_trace.pixel_provenance, "multi-pixel provenance enable");
            check(opts.graphics_trace.pixels.size() == 4, "multi-pixel list count");
            check(opts.graphics_trace.pixels[0] == std::make_pair(99,165), "multi-pixel first coordinate");
            check(opts.graphics_trace.pixels[3] == std::make_pair(197,239), "multi-pixel final coordinate");
        }

        {
            std::cout << "[test] v0.45 known-bad graphics regression set\n";
            const char* argv[] = {"runtime", "--gfx-trace-regression-set", "known-bad"};
            const auto opts = chq::parse_options(static_cast<int>(std::size(argv)), const_cast<char**>(argv));
            check(opts.graphics_trace.pixel_provenance, "known-bad enables pixel provenance");
            check(opts.graphics_trace.pixels.size() == 4, "known-bad pixel count");
            check(opts.graphics_trace.pixels[0] == std::make_pair(99,165), "known-bad continue conflict");
            check(opts.graphics_trace.pixels[1] == std::make_pair(150,224), "known-bad car conflict");
            check(opts.graphics_trace.pixels[2] == std::make_pair(122,239), "known-bad left edge");
            check(opts.graphics_trace.pixels[3] == std::make_pair(197,239), "known-bad right edge");
        }

        {
            std::cout << "[test] v0.46 primask matrix + investigate profile + watch parsing\n";
            const char* argv[] = {"runtime", "--experiment-sprite-mask-prio0", "0f", "--experiment-sprite-mask-prio1", "fc", "--experiment-sprite-mask-matrix", "f0,fc,0f", "--watch-address", "A:byte:100200:race_timer"};
            const auto opts = chq::parse_options(static_cast<int>(std::size(argv)), const_cast<char**>(argv));
            check(opts.graphics_trace.experiment_sprite_mask_prio0 && *opts.graphics_trace.experiment_sprite_mask_prio0 == 0x0f, "prio0 primask parse");
            check(opts.graphics_trace.experiment_sprite_mask_prio1 && *opts.graphics_trace.experiment_sprite_mask_prio1 == 0xfc, "prio1 primask parse");
            check(opts.graphics_trace.experiment_sprite_mask_matrix.size() == 3 && opts.graphics_trace.experiment_sprite_mask_matrix[2] == 0x0f, "primask matrix parse");
            check(opts.memory_watches.size() == 1 && opts.memory_watches[0].address == 0x100200 && opts.memory_watches[0].label == "race_timer", "memory watch parse");
        }

        {
            std::cout << "[test] v0.46 one-command compositor investigation profile\n";
            const char* argv[] = {"runtime", "--investigate", "compositor-known-bad"};
            const auto opts = chq::parse_options(static_cast<int>(std::size(argv)), const_cast<char**>(argv));
            check(opts.graphics_trace.pixel_provenance && opts.graphics_trace.pixels.size() == 4, "investigate known-bad pixels");
            check(opts.graphics_trace.experiment_sprite_mask_matrix.size() == 6, "investigate primask matrix");
            check(opts.auto_zip_logs, "investigate auto zip");
            check(!opts.boot_debug && !opts.sub_debug && !opts.road_focus_debug, "legacy debug should be off by default");
        }


        {
            std::cout << "[test] v0.48 SDL continuous-default frame intent parsing\n";
            const char* argv0[] = {"runtime"};
            const auto def = chq::parse_options(static_cast<int>(std::size(argv0)), const_cast<char**>(argv0));
            check(!def.frames_explicit && def.frames == 120, "default frame budget remains available to non-SDL tools");
            const char* argv1[] = {"runtime", "--frames", "240"};
            const auto bounded = chq::parse_options(static_cast<int>(std::size(argv1)), const_cast<char**>(argv1));
            check(bounded.frames_explicit && bounded.frames == 240, "explicit frame bound tracked for SDL frontend");
        }

        {
            std::cout << "[test] v0.47 transition/capture/checkpoint option parsing\n";
            const char* argv[] = {"runtime", "--capture-frames", "1900,1950,2000", "--capture-on-change", "A:byte:100200:timer", "--capture-on-value", "A:byte:100200:00:timeout", "--capture-relative", "-1,0,1,30", "--capture-after-patch", "0,50,100", "--save-checkpoint", "1600:stage1.chqstate", "--load-checkpoint", "stage1.chqstate", "--diagnostics", "graphics"};
            const auto opts = chq::parse_options(static_cast<int>(std::size(argv)), const_cast<char**>(argv));
            check(opts.capture_frames.size()==3 && opts.capture_frames[1]==1950, "capture frame list parse");
            check(opts.transition_triggers.size()==2 && opts.transition_triggers[0].on_change, "transition trigger parse");
            check(opts.transition_triggers[1].value && *opts.transition_triggers[1].value==0, "transition value parse");
            check(opts.capture_relative_offsets.size()==4 && opts.capture_relative_offsets.front()==-1, "relative capture parse");
            check(opts.capture_after_patch_offsets.size()==3 && opts.capture_after_patch_offsets.back()==100, "patch-relative capture parse");
            check(opts.save_checkpoints.size()==1 && opts.save_checkpoints[0].frame==1600, "checkpoint save parse");
            check(opts.load_checkpoint=="stage1.chqstate", "checkpoint load parse");
            check(opts.diagnostics_profile=="graphics", "diagnostics profile parse");
        }

        {
            std::cout << "[test] v0.55 course-survey assist option parsing\n";
            const char* argv[] = {"runtime", "--course-survey", "--course-follow-steer", "40", "--course-follow-deadzone", "5"};
            const auto opts = chq::parse_options(static_cast<int>(std::size(argv)), const_cast<char**>(argv));
            check(opts.course_follow && opts.course_follow_steer==40 && opts.course_follow_deadzone==5, "course-follow parse");
            check(opts.course_survey && opts.infinite_time, "course-survey/infinite-time parse");
            check(opts.auto_turbo && opts.unlimited_turbo, "auto-turbo implies unlimited-turbo");
        }

        {
            std::cout << "[test] v0.47 checkpoint round-trip\n";
            const auto path = std::filesystem::temp_directory_path() / "chq_v047_checkpoint.chqstate";
            std::filesystem::remove(path);
            auto r = std::make_unique<chq::Runtime>(main_program(), sub_program());
            r->reset();
            r->bus.write(0x100080, 0x1234, 2, chq::BusSpace::Main);
            r->save_checkpoint(path, 321);
            r->bus.write(0x100080, 0xabcd, 2, chq::BusSpace::Main);
            const auto restored_frame=r->load_checkpoint(path);
            check(restored_frame==321, "checkpoint frame round-trip");
            check(r->bus.peek(0x100080,2,chq::BusSpace::Main)==0x1234, "checkpoint RAM round-trip");
            std::filesystem::remove(path);
        }

        {
            std::cout << "[test] v0.60.2 research watch IDs, removal, replace patch, event limit, registers\n";
            auto r = std::make_unique<chq::Runtime>(main_program(), sub_program());
            r->reset();
            r->bus.research_enable(true);
            r->bus.research_set_event_limit(64);
            check(r->bus.research_event_limit()==64, "research event limit set");
            chq::ResearchWatch w{}; w.space=chq::BusSpace::Main; w.start=0x100080; w.end=0x100083; w.writes=true; w.change_only=true;
            const auto wid=r->bus.research_add_watch(w);
            check(wid!=0 && r->bus.research_watches().size()==1, "research watch id");
            r->bus.write(0x100080,0x1111,2,chq::BusSpace::Main);
            check(!r->bus.research_events().empty(), "research range watch event");
            check(r->bus.research_remove_watch(wid) && r->bus.research_watches().empty(), "research watch removal");
            r->bus.debug_write(0x100080,0x2222,2,chq::BusSpace::Main);
            const auto pid=r->bus.research_add_patch(chq::BusSpace::Main,0x100080,2,0x3333,chq::LivePatchMode::Replace);
            r->bus.write(0x100080,0x4444,2,chq::BusSpace::Main);
            check(r->bus.peek(0x100080,2,chq::BusSpace::Main)==0x3333, "replace-on-write intervention");
            check(r->bus.research_remove_patch(pid) && r->bus.research_patches().empty(), "research patch removal");
            const auto pc=r->debug_get_register(chq::BusSpace::Main,"PC");
            check(r->debug_set_register(chq::BusSpace::Main,"D0",0x12345678), "register set accepted");
            check(r->debug_get_register(chq::BusSpace::Main,"D0")==0x12345678, "register read/write round trip");
            check(r->debug_get_register(chq::BusSpace::Main,"PC")==pc, "register edit preserves PC");
        }

        {
            std::cout << "[test] v0.66.8 forensic timeline full-write sink\n";
            auto r = std::make_unique<chq::Runtime>(main_program(), sub_program());
            r->reset();
            std::vector<chq::MemoryWriteTraceEvent> events;
            r->bus.set_timeline_write_sink([&](const chq::MemoryWriteTraceEvent& e){ events.push_back(e); });
            r->bus.write(0x100080,0x1234,2,chq::BusSpace::Main);
            r->bus.write(0x108020,0x56,1,chq::BusSpace::Sub);
            check(events.size()==2, "timeline sink records all normal bus writes");
            check(events[0].space==chq::BusSpace::Main && events[0].address==0x100080 && events[0].size==2, "timeline sink main write metadata");
            check(events[0].old_value==0 && events[0].new_value==0x1234 && events[0].changed, "timeline sink old/new values");
            check(events[1].space==chq::BusSpace::Sub && events[1].address==0x108020 && events[1].size==1, "timeline sink sub write metadata");
            const auto before=events.size();
            r->bus.debug_write(0x100080,0xabcd,2,chq::BusSpace::Main);
            check(events.size()==before, "timeline sink excludes research-internal debug writes");
            r->bus.clear_timeline_write_sink();
            r->bus.write(0x100082,0xbeef,2,chq::BusSpace::Main);
            check(events.size()==before, "timeline sink stops cleanly");
        }

        {
            std::cout << "[test] v0.43 focused provenance suppresses global graph exports\n";
            const auto dir = std::filesystem::temp_directory_path() / "chq_v043_focused_provenance";
            std::filesystem::remove_all(dir);
            {
                auto traced = std::make_unique<chq::Runtime>(call_program(), sub_program());
                chq::ProvenanceConfig cfg;
                cfg.follow_ranges.push_back({0x108020,0x108021});
                traced->configure_provenance(cfg, dir);
                traced->reset(); traced->run(1000); traced->write_provenance_reports();
            }
            check(std::filesystem::exists(dir / "provenance_address_flow.csv"), "focused provenance flow missing");
            check(!std::filesystem::exists(dir / "provenance_callgraph.csv"), "focused provenance unexpectedly emitted global callgraph");
            check(!std::filesystem::exists(dir / "provenance_functions.csv"), "focused provenance unexpectedly emitted function profile");
            std::filesystem::remove_all(dir);
        }

        {
            std::cout << "[test] CPU B reset latch\n";
            auto main_reset = main_program();
            // replace release value with zero so CPU B is held in reset after A runs
            word(main_reset, 0x10a, 0x0000);
            auto held = std::make_unique<chq::Runtime>(main_reset, sub_program());
            held->reset();
            held->run(5000);
            check(!held->bus.sub_enabled(), "CPU B reset not asserted");
        }

        std::cout << "PASS: dual ROMs, maps, TC0040IOC + input injection, tracer gates, scheduler, provenance, shared RAM, road RAM, CPU B reset control\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
}
