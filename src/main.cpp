#include "machine.h"
#include "video.h"
#include "runtime.h"
#include <chrono>
#include <iostream>
#include <thread>

int main(int argc, char* argv[]) {
    try {
        const auto options = chq::parse_options(argc, argv);
        if (options.help) { chq::print_help(); return 0; }
        std::cout << "Chase H.Q. Native 0.8 - CPU/runtime prototype\n";
        std::unique_ptr<chq::Runtime> runtime;
        if (!options.scene_only) {
            runtime = std::make_unique<chq::Runtime>(chq::load_main_rom(options.roms));
            std::filesystem::create_directories(options.logs);
            runtime->bus.open_log(options.logs / "bus.log", 20000, options.all);
            runtime->reset();
        }
        chq::Machine machine;
        if (!machine.load_roms(options.roms)) return 1;
        chq::Video video;
        if (!video.initialise()) return 1;
        std::cout << "Scene preview: arrows = curve/horizon, Shift = larger steps,\n"
                     "A/D = road phase, R/T = road/sprites, Home = scene reset, Esc = quit.\n"
                     "The scene remains synthetic; CPU RAM is captured in logs/ram.\n"
                     "CPU execution stops at the frame budget; the scene stays interactive.\n";
        chq::SceneState scene;
        bool running = true, changed = true, saved = false;
        unsigned frame = 0;
        auto save = [&] {
            if (!runtime || saved) return;
            runtime->summary(std::cout);
            std::ofstream summary(options.logs / "summary.txt");
            runtime->summary(summary);
            if (!summary) throw std::runtime_error("Cannot write summary");
            runtime->bus.dump(options.logs / "ram");
            saved = true;
        };
        while (running) {
            const auto tick = std::chrono::steady_clock::now();
            video.process_events(running, scene, changed);
            if (runtime && running && frame < options.frames && runtime->fault.empty()) {
                if (options.irq) runtime->irq4();
                runtime->run(200000);
                ++frame;
            }
            if (runtime && (frame >= options.frames || !runtime->fault.empty())) save();
            if (changed) { video.draw_scene(machine, scene); changed = false; }
            std::this_thread::sleep_until(tick + std::chrono::microseconds(16667));
        }
        save();
        video.shutdown();
        return runtime && !runtime->fault.empty() ? 2 : 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n'; return 1;
    }
}
