#include "runtime.h"
#include "version.h"
#include <iostream>

int main(int argc, char** argv) {
    try {
        const auto options = chq::parse_options(argc, argv);
        if (options.help) { chq::print_help(); return 0; }
        if (options.scene_only) throw std::runtime_error("Use ChaseHQNative for --scene-only");

        chq::Runtime runtime(
            chq::load_main_rom(options.roms),
            chq::load_sub_rom(options.roms));

        std::filesystem::create_directories(options.logs);
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

        for (unsigned frame = 0; frame < options.frames && runtime.fault.empty(); ++frame) {
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
        return runtime.fault.empty() ? 0 : 2;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}
