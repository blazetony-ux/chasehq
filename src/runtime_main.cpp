#include "runtime.h"
#include <iostream>
int main(int argc, char** argv) {
    try {
        const auto options = chq::parse_options(argc, argv);
        if (options.help) { chq::print_help(); return 0; }
        if (options.scene_only) throw std::runtime_error("Use ChaseHQNative for --scene-only");
        chq::Runtime runtime(chq::load_main_rom(options.roms));
        std::filesystem::create_directories(options.logs);
        runtime.bus.open_log(options.logs / "bus.log", 20000, options.all);
        runtime.reset();
        for (unsigned frame = 0; frame < options.frames && runtime.fault.empty(); ++frame) {
            if (options.irq) runtime.irq4();
            runtime.run(200000);
        }
        runtime.summary(std::cout);
        std::ofstream summary(options.logs / "summary.txt");
        runtime.summary(summary);
        if (!summary) throw std::runtime_error("Cannot write summary");
        runtime.bus.dump(options.logs / "ram");
        return runtime.fault.empty() ? 0 : 2;
    } catch (const std::exception& e) { std::cerr << "Error: " << e.what() << '\n'; return 1; }
}
