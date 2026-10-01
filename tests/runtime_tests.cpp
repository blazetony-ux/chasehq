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
            const char* argv[] = {"runtime", "--target-state-log", "--no-collisions", "--course-follow-controller", "predictive", "--course-follow-lateral-kd", "0.02", "--course-follow-slew", "6", "--course-follow-speed-control", "on", "--progress-status", "240"};
            const auto opts = chq::parse_options(static_cast<int>(std::size(argv)), const_cast<char**>(argv));
            check(opts.target_state_log, "target-state log option parse");
            check(opts.no_collisions, "no-collisions option parse");
            check(opts.course_follow_controller == "predictive" && opts.course_follow_lateral_kd == 0.02 && opts.course_follow_slew == 6 && opts.course_follow_speed_control, "predictive course-follow parse");
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
            std::cout << "[test] dual CPU scheduler/shared RAM\n";
            chq::Runtime runtime(main_program(), sub_program());
            runtime.reset();
            runtime.run(5000);
            check(runtime.bus.peek(0x108000, 2) == 0x1234, "CPU A shared write");
            check(runtime.bus.peek(0x108002, 2) == 0x1235, "CPU B shared communication");
            check(runtime.bus.peek(0x800000, 2, chq::BusSpace::Sub) == 0xbeef, "CPU B road write");
            check(runtime.instructions_a > 0 && runtime.instructions_b > 0, "both CPUs executed");
        }

        {
            std::cout << "[test] checkpoint resume after runtime reconstruction\n";
            const auto state_path = std::filesystem::temp_directory_path() / "chq_checkpoint_resume_test.chqstate";
            std::filesystem::remove(state_path);
            {
                chq::Runtime first(main_program(), sub_program());
                first.reset();
                first.run(5000);
                first.save_checkpoint(state_path, 123);
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
                chq::Runtime resumed(main_program(), sub_program());
                resumed.reset();
                const auto restored_frame = resumed.load_checkpoint(state_path);
                check(restored_frame == 123, "checkpoint frame restore");
                resumed.run(5000);
                check(resumed.fault.empty(), "checkpoint resume fault");
                check(resumed.instructions_a > 0, "checkpoint CPU A did not resume");
            }
            std::filesystem::remove(state_path);
        }

        {
            std::cout << "[test] generic runtime tracer\n";
            const auto trace_path = std::filesystem::temp_directory_path() / "chq_v026_trace_test.log";
            std::filesystem::remove(trace_path);
            {
                chq::Runtime traced(main_program(), sub_program());
                chq::TraceConfig cfg;
                cfg.cpu_mask = 2;
                cfg.max_lines = 1000;
                cfg.before = 8;
                cfg.after = 8;
                cfg.mem_ranges.push_back({0x108002, 0x108003, false, true, false});
                cfg.pc_ranges.push_back({0x100, 0x120});
                cfg.triggers.push_back({0x108002, 0x108003, chq::TraceTriggerKind::Write});
                traced.configure_generic_trace(cfg, trace_path);
                traced.reset();
                traced.run(5000);
            }
            std::ifstream tf(trace_path);
            const std::string text((std::istreambuf_iterator<char>(tf)), std::istreambuf_iterator<char>());
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
                chq::Runtime traced(main_program(), sub_program());
                chq::TraceConfig cfg;
                cfg.cpu_mask = 2;
                cfg.max_lines = 1000;
                cfg.mem_ranges.push_back({0x108002, 0x108003, false, true, false});
                cfg.mem_pc_ranges.push_back({0x100, 0x107}); // excludes CPU-B write at 0x108
                traced.configure_generic_trace(cfg, trace_path);
                traced.reset();
                traced.run(5000);
            }
            std::ifstream tf(trace_path);
            const std::string text((std::istreambuf_iterator<char>(tf)), std::istreambuf_iterator<char>());
            check(text.find("MEM_W frame=0 cpu=B") == std::string::npos, "memory PC gate failed to suppress out-of-range write");
            check(text.find("# MEM_PC 000100-000107") != std::string::npos, "memory PC gate header missing");
            std::filesystem::remove(trace_path);
        }

        {
            std::cout << "[test] generic memory PC gate matching write\n";
            const auto trace_path = std::filesystem::temp_directory_path() / "chq_v0262_mem_pc_trace_match_test.log";
            std::filesystem::remove(trace_path);
            {
                chq::Runtime traced(main_program(), sub_program());
                chq::TraceConfig cfg;
                cfg.cpu_mask = 2;
                cfg.max_lines = 1000;
                cfg.mem_ranges.push_back({0x108002, 0x108003, false, true, false});
                cfg.mem_pc_ranges.push_back({0x108, 0x10d});
                traced.configure_generic_trace(cfg, trace_path);
                traced.reset();
                traced.run(5000);
            }
            std::ifstream tf(trace_path);
            const std::string text((std::istreambuf_iterator<char>(tf)), std::istreambuf_iterator<char>());
            check(text.find("MEM_W frame=0 cpu=B") != std::string::npos, "memory PC gate suppressed matching write");
            std::filesystem::remove(trace_path);
        }

        {
            std::cout << "[test] v0.27 memory arming + frame tag\n";
            const auto trace_path = std::filesystem::temp_directory_path() / "chq_v027_arm_trace_test.log";
            std::filesystem::remove(trace_path);
            {
                chq::Runtime traced(main_program(), sub_program());
                chq::TraceConfig cfg;
                cfg.cpu_mask = 1;
                cfg.max_lines = 100;
                cfg.mem_ranges.push_back({0x100080, 0x100081, false, true, false, false});
                cfg.arm_mem_writes.push_back({0x100172, 0x3800});
                traced.configure_generic_trace(cfg, trace_path);
                traced.set_trace_frame(42);
                traced.bus.set_trace_pc(chq::BusSpace::Main, 0x2000);
                traced.bus.write(0x100080, 0x1111, 2, chq::BusSpace::Main); // must be silent
                traced.bus.write(0x100172, 0x3800, 4, chq::BusSpace::Main); // arms
                traced.bus.write(0x100080, 0x2222, 2, chq::BusSpace::Main); // logged
            }
            std::ifstream tf(trace_path);
            const std::string text((std::istreambuf_iterator<char>(tf)), std::istreambuf_iterator<char>());
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
                chq::Runtime traced(main_program(), sub_program());
                chq::TraceConfig cfg;
                cfg.cpu_mask = 1;
                cfg.max_lines = 100;
                cfg.mem_ranges.push_back({0x100080, 0x100081, false, true, false, true});
                traced.configure_generic_trace(cfg, trace_path);
                traced.bus.set_trace_pc(chq::BusSpace::Main, 0x2000);
                traced.bus.write(0x100080, 1, 2, chq::BusSpace::Main);
                traced.bus.write(0x100080, 1, 2, chq::BusSpace::Main);
            }
            std::ifstream tf(trace_path);
            const std::string text((std::istreambuf_iterator<char>(tf)), std::istreambuf_iterator<char>());
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
                chq::Runtime traced(main_program(), sub_program());
                chq::TraceConfig cfg;
                cfg.cpu_mask = 1;
                cfg.max_lines = 100;
                cfg.task_slots.push_back(0x100080);
                traced.configure_generic_trace(cfg, trace_path);
                traced.set_trace_frame(77);
                traced.bus.set_trace_pc(chq::BusSpace::Main, 0x1096);
                traced.bus.write(0x100080, 1, 2, chq::BusSpace::Main);
                traced.bus.write(0x100080, 1, 2, chq::BusSpace::Main);
                traced.bus.write(0x100080, 0, 2, chq::BusSpace::Main);
            }
            std::ifstream tf(trace_path);
            const std::string text((std::istreambuf_iterator<char>(tf)), std::istreambuf_iterator<char>());
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
                chq::Runtime traced(main_program(), sub_program());
                chq::ProvenanceConfig cfg;
                cfg.follow_ranges.push_back({0x108002,0x108003});
                cfg.trace_callstack = true;
                cfg.trace_callgraph = true;
                cfg.profile_functions = true;
                traced.configure_provenance(cfg, dir);
                traced.reset();
                traced.run(5000);
                traced.write_provenance_reports();
            }
            std::ifstream pf(dir / "provenance_address_flow.csv");
            const std::string ptext((std::istreambuf_iterator<char>(pf)), std::istreambuf_iterator<char>());
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
                chq::Runtime traced(call_program(), sub_program());
                chq::ProvenanceConfig cfg;
                cfg.follow_ranges.push_back({0x108020,0x108021});
                cfg.trace_callstack=true; cfg.trace_callgraph=true; cfg.profile_functions=true;
                traced.configure_provenance(cfg, dir);
                traced.reset();
                traced.run(1000);
                traced.write_provenance_reports();
            }
            std::ifstream cf(dir / "provenance_callgraph.csv");
            const std::string ctext((std::istreambuf_iterator<char>(cf)), std::istreambuf_iterator<char>());
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
            chq::Runtime r(main_program(), sub_program());
            r.reset();
            r.bus.write(0x100080, 0x1234, 2, chq::BusSpace::Main);
            r.save_checkpoint(path, 321);
            r.bus.write(0x100080, 0xabcd, 2, chq::BusSpace::Main);
            const auto restored_frame=r.load_checkpoint(path);
            check(restored_frame==321, "checkpoint frame round-trip");
            check(r.bus.peek(0x100080,2,chq::BusSpace::Main)==0x1234, "checkpoint RAM round-trip");
            std::filesystem::remove(path);
        }

        {
            std::cout << "[test] v0.60.2 research watch IDs, removal, replace patch, event limit, registers\n";
            chq::Runtime r(main_program(), sub_program());
            r.reset();
            r.bus.research_enable(true);
            r.bus.research_set_event_limit(64);
            check(r.bus.research_event_limit()==64, "research event limit set");
            chq::ResearchWatch w{}; w.space=chq::BusSpace::Main; w.start=0x100080; w.end=0x100083; w.writes=true; w.change_only=true;
            const auto wid=r.bus.research_add_watch(w);
            check(wid!=0 && r.bus.research_watches().size()==1, "research watch id");
            r.bus.write(0x100080,0x1111,2,chq::BusSpace::Main);
            check(!r.bus.research_events().empty(), "research range watch event");
            check(r.bus.research_remove_watch(wid) && r.bus.research_watches().empty(), "research watch removal");
            r.bus.debug_write(0x100080,0x2222,2,chq::BusSpace::Main);
            const auto pid=r.bus.research_add_patch(chq::BusSpace::Main,0x100080,2,0x3333,chq::LivePatchMode::Replace);
            r.bus.write(0x100080,0x4444,2,chq::BusSpace::Main);
            check(r.bus.peek(0x100080,2,chq::BusSpace::Main)==0x3333, "replace-on-write intervention");
            check(r.bus.research_remove_patch(pid) && r.bus.research_patches().empty(), "research patch removal");
            const auto pc=r.debug_get_register(chq::BusSpace::Main,"PC");
            check(r.debug_set_register(chq::BusSpace::Main,"D0",0x12345678), "register set accepted");
            check(r.debug_get_register(chq::BusSpace::Main,"D0")==0x12345678, "register read/write round trip");
            check(r.debug_get_register(chq::BusSpace::Main,"PC")==pc, "register edit preserves PC");
        }

        {
            std::cout << "[test] v0.66.8 forensic timeline full-write sink\n";
            chq::Runtime r(main_program(), sub_program());
            r.reset();
            std::vector<chq::MemoryWriteTraceEvent> events;
            r.bus.set_timeline_write_sink([&](const chq::MemoryWriteTraceEvent& e){ events.push_back(e); });
            r.bus.write(0x100080,0x1234,2,chq::BusSpace::Main);
            r.bus.write(0x108020,0x56,1,chq::BusSpace::Sub);
            check(events.size()==2, "timeline sink records all normal bus writes");
            check(events[0].space==chq::BusSpace::Main && events[0].address==0x100080 && events[0].size==2, "timeline sink main write metadata");
            check(events[0].old_value==0 && events[0].new_value==0x1234 && events[0].changed, "timeline sink old/new values");
            check(events[1].space==chq::BusSpace::Sub && events[1].address==0x108020 && events[1].size==1, "timeline sink sub write metadata");
            const auto before=events.size();
            r.bus.debug_write(0x100080,0xabcd,2,chq::BusSpace::Main);
            check(events.size()==before, "timeline sink excludes research-internal debug writes");
            r.bus.clear_timeline_write_sink();
            r.bus.write(0x100082,0xbeef,2,chq::BusSpace::Main);
            check(events.size()==before, "timeline sink stops cleanly");
        }

        {
            std::cout << "[test] v0.43 focused provenance suppresses global graph exports\n";
            const auto dir = std::filesystem::temp_directory_path() / "chq_v043_focused_provenance";
            std::filesystem::remove_all(dir);
            {
                chq::Runtime traced(call_program(), sub_program());
                chq::ProvenanceConfig cfg;
                cfg.follow_ranges.push_back({0x108020,0x108021});
                traced.configure_provenance(cfg, dir);
                traced.reset(); traced.run(1000); traced.write_provenance_reports();
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
            chq::Runtime held(main_reset, sub_program());
            held.reset();
            held.run(5000);
            check(!held.bus.sub_enabled(), "CPU B reset not asserted");
        }

        std::cout << "PASS: dual ROMs, maps, TC0040IOC + input injection, tracer gates, scheduler, provenance, shared RAM, road RAM, CPU B reset control\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
}
