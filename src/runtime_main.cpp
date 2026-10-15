#include "runtime.h"
#include "content_pack.h"
#include "version.h"
#include <iostream>
#include <fstream>
#include <string>

int main(int argc, char** argv) {
    try {
        const auto options = chq::parse_options(argc, argv);
        if (options.help) { chq::print_help(); return 0; }
        if (options.scene_only) throw std::runtime_error("Use ChaseHQNative for --scene-only");

        chq::ContentPack pack;
        const bool use_pack=!options.content_pack.empty();
        if(use_pack){std::string err;if(!pack.load(options.content_pack,&err)) throw std::runtime_error("Content pack: "+err);}
        if (!options.content_overlays.empty() && !use_pack) throw std::runtime_error("Content overlays require --content-pack");
        for (const auto& path : options.content_overlays) {
            chq::ContentPack overlay; std::string err;
            if (!overlay.load(path, &err) || !chq::apply_content_overlay(pack, overlay, &err))
                throw std::runtime_error("Content overlay " + path.string() + ": " + err);
            std::cout << "[CHQOVERLAY] applied " << path.string() << " entries=" << overlay.entry_count() << "\n";
        }
        auto req=[&](const char* n)->chq::Bytes{auto* p=pack.find(n);if(!p)throw std::runtime_error(std::string("Content pack missing ")+n);return *p;};
        chq::Runtime runtime(
            use_pack ? req("cpu/main.bin") : chq::load_main_rom(options.roms),
            use_pack ? req("cpu/sub.bin") : chq::load_sub_rom(options.roms));

        if (options.rom_census) runtime.enable_rom_execution_census();
        if (options.cpu_b_edge_probe) runtime.enable_cpu_b_edge_probe();
        if (!options.native_cpu_b_library.empty()) runtime.load_native_cpu_b_library(options.native_cpu_b_library);
        if (!options.native_cpu_b_pair_library.empty()) runtime.load_native_cpu_b_pair_library(options.native_cpu_b_pair_library);
        if (options.native_cpu_b_pair_replace || options.native_cpu_b_pair_fault_probe)
            runtime.enable_native_cpu_b_pair_replacement(options.native_cpu_b_pair_fault_probe);
        if (options.native_cpu_b_replace || options.native_cpu_b_fault_probe)
            runtime.enable_native_cpu_b_replacement(options.native_cpu_b_fault_probe);
        if (options.native_projection_shadow) runtime.enable_native_projection_shadow();
        if (options.native_steering_shadow) runtime.enable_native_steering_shadow();
        if (!options.native_steering_library.empty()) runtime.load_native_steering_library(options.native_steering_library);
        if (options.native_steering_replace || options.native_steering_rollback_probe)
            runtime.enable_native_steering_replacement(options.native_steering_rollback_probe);
        if (!options.native_copy_library.empty()) runtime.load_native_copy_library(options.native_copy_library);
        if(options.native_copy_replace || options.native_copy_fault_probe)
            runtime.enable_native_copy_replacement(options.native_copy_fault_probe);
        if (!options.native_health_library.empty()) runtime.load_native_health_library(options.native_health_library);
        runtime.set_native_health_case(options.native_health_case);
        if (options.native_health_replace || options.native_health_fault_probe)
            runtime.enable_native_health_replacement(options.native_health_fault_probe);
        for (const auto& spec : options.native_observe_entries) {
            const auto pos = spec.find(':');
            if (pos == std::string::npos || pos == 0 || pos + 1 >= spec.size())
                throw std::runtime_error("--native-observe-entry expects A:0xPC or B:0xPC");
            const auto cpu = spec.substr(0,pos);
            if (cpu != "A" && cpu != "B") throw std::runtime_error("native entry CPU must be A or B");
            std::size_t end = 0;
            const auto pc = std::stoul(spec.substr(pos+1), &end, 0);
            if (end != spec.size()-pos-1 || pc > 0xffffffu || (pc&1u))
                throw std::runtime_error("invalid native observation PC (even address required)");
            runtime.observe_native_entry(cpu == "B" ? chq::BusSpace::Sub : chq::BusSpace::Main,
                                         static_cast<std::uint32_t>(pc),spec);
        }
        std::filesystem::create_directories(options.logs);
        if (options.native_dispatch_audit) runtime.enable_native_dispatch_audit(options.logs / "native-dispatch.tsv");
        if (options.native_cpu_b_state_capture || options.native_cpu_b_replace || options.native_cpu_b_fault_probe)
            runtime.enable_native_cpu_b_capture(options.logs / "native-cpu-b-state.tsv");
        if (options.native_cpu_b_pair_state_capture || options.native_cpu_b_pair_replace || options.native_cpu_b_pair_fault_probe)
            runtime.enable_native_cpu_b_pair_capture(options.logs / "native-cpu-b-pair-state.tsv");
        if (options.native_steering_state_capture) runtime.enable_native_steering_state_capture(options.logs / "native-steering-state.tsv");
        if (options.native_steering_state_capture) runtime.enable_native_steering_bus_capture(options.logs / "native-steering-bus.tsv");
        if (options.native_copy_state_capture || options.native_copy_replace || options.native_copy_fault_probe)
            runtime.enable_native_copy_capture(options.logs / "native-copy-state.tsv");
        if (options.native_health_state_capture || options.native_health_replace || options.native_health_fault_probe)
            runtime.enable_native_health_state_capture(options.logs / "native-health-state.tsv");
        runtime.bus.open_log(options.logs / "bus.log", 40000, options.all);
        runtime.enable_boot_debug(options.boot_debug, options.loop_samples);
        if (options.boot_debug)
            runtime.open_boot_debug_log(options.logs / "boot_loop.log", 20000);
        runtime.enable_sub_debug(options.sub_debug, options.sub_samples);
        if (options.sub_debug)
            runtime.open_sub_debug_log(options.logs / "cpu_b_loop.log", 40000);
        if (options.handshake_debug)
            runtime.open_handshake_log(options.logs / "handshake.log", 60000);
        runtime.enable_road_focus_debug(options.road_focus_debug, options.road_samples);
        if (options.road_focus_debug)
            runtime.open_road_focus_log(options.logs / "road_write_focus.log", 80000);
        if (options.road_flow_debug)
            runtime.open_road_data_flow_log(options.logs / "road_data_flow.log", options.flow_samples);
        runtime.enable_road_state_debug(options.road_state_debug, options.state_samples);
        runtime.set_force_road_flag(options.force_road_flag);
        if (options.road_state_debug)
            runtime.open_road_state_log(options.logs / "road_state.log", options.state_samples);
        if (options.road_pending_debug)
            runtime.bus.open_road_pending_log(options.logs / "road_pending.log", options.pending_samples);
        if (options.trace.enabled())
            runtime.configure_generic_trace(options.trace, options.logs / "trace.log");
        if (options.provenance.enabled())
            runtime.configure_provenance(options.provenance, options.logs);
        runtime.reset();
        unsigned start_frame = 0;
        if (!options.load_checkpoint.empty()) {
            start_frame = runtime.load_checkpoint(options.load_checkpoint);
            std::cout << "[CHECKPOINT LOAD] frame=" << start_frame << " <- " << options.load_checkpoint.string() << "\n";
        }
        // Standalone census evidence must prove the *actual* starting frame.
        // A checkpoint is not optional: load failures throw before metadata or CPU execution.
        {
            std::ofstream mf(options.logs / "run_manifest.json",std::ios::binary|std::ios::trunc);
            if(!mf)throw std::runtime_error("Cannot create run_manifest.json");
            mf << "{\n  \"schema\": \"chq-headless-run-v1\",\n"
               << "  \"release\": \"" << chq::kNativeVersion << "\",\n"
               << "  \"checkpointRequested\": " << (!options.load_checkpoint.empty()?"true":"false") << ",\n"
               << "  \"checkpointLoadVerified\": " << (!options.load_checkpoint.empty()?"true":"false") << ",\n"
               << "  \"startFrame\": " << start_frame << ",\n"
               << "  \"endFrameExclusive\": " << options.frames << ",\n"
               << "  \"framesScheduled\": " << (options.frames>start_frame?options.frames-start_frame:0) << ",\n"
               << "  \"romCensus\": " << (options.rom_census?"true":"false") << ",\n"
               << "  \"projectionShadow\": " << (options.native_projection_shadow?"true":"false") << "\n}\n";
            if(!mf)throw std::runtime_error("Cannot write run_manifest.json");
            std::ofstream pf(options.logs / "run_phase.txt",std::ios::binary|std::ios::trunc);
            if(!pf)throw std::runtime_error("Cannot create run_phase.txt");
            pf << "checkpoint_load_complete start=" << start_frame << " end=" << options.frames << "\n";
        }
        if (options.frames <= start_frame)
            std::cout << "[RUN TARGET] checkpoint is already at/past target; finalising without emulation\n";
        if (!options.input_pulses.empty())
            runtime.bus.open_ioc_pulse_log(options.logs / "ioc_pulse.log");

        struct PulseState {
            bool armed = false;
            bool observed = false;
            unsigned active_from = 0;
            std::uint64_t reads_before = 0;
        };
        std::vector<PulseState> pulse_state(options.input_pulses.size());
        std::array<std::uint8_t, 16> active_ioc_xor{};

        for (unsigned frame = start_frame; frame < options.frames && runtime.fault.empty(); ++frame) {
            runtime.set_trace_frame(frame);
            runtime.bus.set_ioc_debug_frame(frame);
            active_ioc_xor.fill(0);
            if(options.steering_fixed) runtime.bus.set_ioc_steering(*options.steering_fixed);
            for(const auto& se:options.steering_events) if(frame>=se.start_frame && frame<=se.end_frame) runtime.bus.set_ioc_steering(se.value);

            for (std::size_t i = 0; i < options.input_pulses.size(); ++i) {
                const auto& pulse = options.input_pulses[i];
                auto& state = pulse_state[i];
                const auto port = static_cast<std::uint8_t>(pulse.port & 0x0f);

                if (!state.armed && frame >= pulse.start_frame) {
                    state.armed = true;
                    state.active_from = frame;
                    state.reads_before = runtime.bus.ioc_read_count(port);
                    std::cout << "[IOC PULSE ARMED] frame=" << std::dec << frame
                              << " port=0x" << std::hex << static_cast<unsigned>(port)
                              << " xor=0x" << static_cast<unsigned>(pulse.mask)
                              << std::dec << " duration=" << pulse.duration_frames << "\n";
                }

                if (state.armed && !state.observed && runtime.bus.ioc_read_count(port) > state.reads_before) {
                    state.observed = true;
                    state.active_from = frame;
                    std::cout << "[IOC PULSE HIT] frame=" << frame
                              << " port=0x" << std::hex << static_cast<unsigned>(port)
                              << std::dec << "\n";
                }

                if (state.armed && (!state.observed ||
                    static_cast<std::uint64_t>(frame) < static_cast<std::uint64_t>(state.active_from) + pulse.duration_frames))
                    active_ioc_xor[port] ^= pulse.mask;
            }

            for (unsigned port = 0; port < active_ioc_xor.size(); ++port)
                runtime.bus.set_ioc_scenario_xor_mask(static_cast<std::uint8_t>(port), active_ioc_xor[port]);

            if (options.irq) runtime.irq4();
            runtime.run(200000);
        }
        runtime.bus.clear_ioc_scenario_overrides();

        runtime.summary(std::cout);
        std::ofstream summary(options.logs / "summary.txt");
        runtime.summary(summary);
        if (!summary) throw std::runtime_error("Cannot write summary");
        runtime.bus.dump(options.logs / "ram");
        runtime.write_provenance_reports();
        if (options.native_steering_state_capture || options.native_health_state_capture || options.native_health_replace || options.native_health_fault_probe || options.native_copy_state_capture || options.native_copy_replace || options.native_copy_fault_probe || options.native_cpu_b_state_capture || options.native_cpu_b_replace || options.native_cpu_b_fault_probe || options.native_cpu_b_pair_state_capture || options.native_cpu_b_pair_replace || options.native_cpu_b_pair_fault_probe)
            runtime.export_native_endstate(options.logs / "native-machine-endstate.json");
        if (options.rom_census || options.native_projection_shadow || options.native_steering_shadow || options.native_steering_replace || options.native_steering_rollback_probe || options.native_health_replace || options.native_health_fault_probe || options.native_copy_state_capture || options.native_copy_replace || options.native_copy_fault_probe || options.native_cpu_b_state_capture || options.native_cpu_b_replace || options.native_cpu_b_fault_probe || options.native_cpu_b_pair_state_capture || options.native_cpu_b_pair_replace || options.native_cpu_b_pair_fault_probe || !options.native_observe_entries.empty())
            runtime.write_native_execution_evidence(options.logs, "runtime-cli-checkpoint:" + options.load_checkpoint.string() + ":start:" + std::to_string(start_frame) + ":end:" + std::to_string(options.frames));
        return runtime.fault.empty() ? 0 : 2;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}
