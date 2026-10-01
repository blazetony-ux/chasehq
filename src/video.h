#pragma once
#include "machine.h"
#include "runtime.h"

#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <deque>
#include <string>
#include <vector>

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

namespace chq {

struct DebugUiActions {
    bool toggle_pause = false;
    unsigned step_frames = 0;
    bool screenshot = false;
    bool capture_graphics = false;
    bool save_checkpoint = false;
    bool load_checkpoint = false;
    bool timer_freeze_toggle = false;
    int timer_preset_bcd = -1;
    bool export_bundle = false;

    // v0.60.0 Research Workbench actions.
    bool workbench_opened = false;
    bool workbench_closed = false;
    bool snapshot_pause_point = false;
    bool restore_pause_point = false;
    bool step_instruction_a = false;
    bool step_instruction_b = false;
    bool clear_research_patches = false;
    bool clear_research_watches = false;
    bool clear_research_events = false;
    enum class ResearchCommand { None, WriteOnce, Freeze, Suppress, WatchWrite, WatchChange } research_command = ResearchCommand::None;
    BusSpace research_space = BusSpace::Main;
    std::uint32_t research_address = 0;
    std::uint32_t research_value = 0;
    unsigned research_size = 2;

    // v0.66.2: gameplay controls routed through the authentic IOC input path.
    bool gameplay_steering_set = false;
    std::uint16_t gameplay_steering = 0x0000; // signed 12-bit raw IOC encoding
    bool gameplay_accel_set = false;
    bool gameplay_accel = false;
    bool gameplay_brake_set = false;
    bool gameplay_brake = false;
    bool gameplay_turbo_set = false;
    bool gameplay_turbo = false;
};

struct DebugSpriteInfo {
    unsigned slot = 0;
    std::uint16_t raw_w0=0, raw_w1=0, raw_w2=0, raw_w3=0;
    int raw_x=0, raw_y=0;
    int x = 0, y = 0, width = 0, height = 0;
    std::uint16_t map = 0;
    std::uint8_t palette = 0;
    int priority = 0;
    std::uint64_t visible_pixels = 0;
    int visible_left = -1, visible_top = -1, visible_right = -1, visible_bottom = -1;
};

struct SpriteChangeEvent {
    unsigned frame = 0;
    unsigned slot = 0;
    std::uint16_t old_map = 0, new_map = 0;
    std::uint8_t old_palette = 0, new_palette = 0;
    int old_x = 0, old_y = 0, new_x = 0, new_y = 0;
    bool old_visible = false, new_visible = false;
    bool semantic = false; // map/palette/visibility/priority/shape, not movement alone
    std::uint32_t writer_pc = 0;
};

class Video {
public:
    static constexpr int WIDTH = 320;
    static constexpr int HEIGHT = 240;
    // Chase H.Q. hardware raster is 262 lines; MAME configures visible Y=16..255.
    // Runtime framebuffer remains 240 lines, but hardware-space Y starts at 16.
    static constexpr int VISIBLE_Y_START = 16;

    bool initialise(const std::filesystem::path& log_directory = "logs");
    void configure_proms(const Machine& machine);
    void configure_diagnostics(const std::vector<unsigned>& frames, bool enabled, bool raw_maps);
    void add_diagnostics_frame(unsigned frame);
    void configure_forensics(const GraphicsTraceConfig& config);
    void configure_layer_offsets(const Options& options);
    bool set_layer_offset(const std::string& layer, int x, int y);
    std::string layer_offsets_status() const;
    void set_tc0100scn_legacy_x(bool enabled) { tc0100scn_legacy_x_ = enabled; }
    bool tc0100scn_legacy_x() const { return tc0100scn_legacy_x_; }
    void configure_sprite_tie_break(const Options& options);
    void set_sprite_tie_break_higher(bool higher) { sprite_tie_break_higher_ = higher; }
    bool sprite_tie_break_higher() const { return sprite_tie_break_higher_; }
    void configure_tc0100scn_trace(const Options& options);
    void shutdown();

    bool process_events(bool& running, SceneState& scene, bool& changed, DebugUiActions& actions);
    void update_fast_forward_status(unsigned frame, unsigned target);

    void draw_scene(const Machine& machine, const SceneState& scene);
    void draw_runtime(const Machine& machine, const Runtime& runtime, const SceneState& scene, unsigned frame, bool paused = false, bool timer_frozen = false);
    bool save_current_screenshot(const std::filesystem::path& path) const;
    bool save_layered_snapshot(const std::filesystem::path& root, const Machine& machine, const Runtime& runtime, const SceneState& scene, unsigned frame);
    bool debug_overlay_visible() const { return debug_overlay_; }
    bool set_always_on_top(bool enabled);
    bool always_on_top() const { return always_on_top_; }
    bool set_fullscreen(bool enabled);
    bool set_window_scale(int scale);
    bool show_window(bool visible);
    bool minimize_window();
    bool restore_window();
    bool fullscreen() const { return fullscreen_; }
    bool window_visible() const { return window_visible_; }
    bool window_minimized() const { return window_minimized_; }
    int window_scale() const { return window_scale_; }
    void show_script_prompt(const std::string& message);
    void clear_script_prompt();
    bool script_prompt_active() const { return script_prompt_active_; }
    bool script_prompt_accepted() const { return script_prompt_accepted_; }
    bool script_prompt_cancelled() const { return script_prompt_cancelled_; }
    const std::string& script_prompt_message() const { return script_prompt_message_; }
    bool research_workbench_visible() const { return research_workbench_; }
    int selected_sprite() const { return selected_sprite_; }
    const std::deque<SpriteChangeEvent>& sprite_change_events() const { return sprite_change_events_; }
    const std::vector<DebugSpriteInfo>& debug_sprites() const { return debug_sprites_; }
    void set_runtime_sprite_override(unsigned slot, int map, int palette, int visible);
    void clear_runtime_sprite_overrides();
    void set_runtime_sprite_visual(unsigned slot, const std::string& mode);
    void clear_runtime_sprite_visual();
    unsigned checkpoint_slot() const { return checkpoint_slot_; }
    void set_checkpoint_slot(unsigned slot) { checkpoint_slot_ = slot % 10u; }
    void set_course_survey_assists(bool follow, bool infinite_time, bool unlimited_turbo, bool auto_turbo) { assist_course_follow_=follow; assist_infinite_time_=infinite_time; assist_unlimited_turbo_=unlimited_turbo; assist_auto_turbo_=auto_turbo; }
    bool write_sprite_evidence_csv(const std::filesystem::path& path, unsigned frame) const;
    bool write_sprite_quad_evidence_csv(const std::filesystem::path& path, unsigned frame) const;
    bool write_semantic_object_evidence_csv(const std::filesystem::path& path, unsigned frame, const std::vector<std::string>& objects) const;
    void configure_sprite_presentation(const std::vector<Options::SpriteSelector>& solo, const std::vector<Options::SpriteSelector>& hide, const std::vector<Options::SpriteQuadCandidate>& quads, Options::DiagnosticBackground background);

private:
    // v0.16: the priority surface is now a *layer bitmask*, not a scalar rank.
    // This mirrors the way arcade priority bitmaps are commonly consumed:
    // sprite/road pixels are tested against sets of already-present layers.
    static constexpr std::uint8_t LAYER_BG_BOTTOM = 0x01;
    static constexpr std::uint8_t LAYER_ROAD_A    = 0x02;
    static constexpr std::uint8_t LAYER_ROAD_B    = 0x04;
    static constexpr std::uint8_t LAYER_BG_UPPER  = 0x08;
    static constexpr std::uint8_t LAYER_SPRITE    = 0x10;
    static constexpr std::uint8_t LAYER_TEXT      = 0x20;

    enum class PromSource : std::uint8_t {
        None = 0,
        RoadA = 1,
        RoadB = 2,
        Sprite = 3
    };

    struct PromEntry {
        std::uint8_t raw = 0;
        std::uint8_t output_class = 0;
        std::uint8_t block_mask = 0;
    };

    static std::uint8_t sprite_pixel(const std::vector<std::uint8_t>& region, std::size_t tile_index, int x, int y);
    static std::uint32_t sprite_color(std::uint8_t palette_bank, std::uint8_t pen);
    static std::uint32_t runtime_palette_color(const Bus& bus, std::uint16_t index, std::uint8_t pen);
    static std::uint8_t road_pixel(const std::vector<std::uint8_t>& road, int tile, int logical_x);
    static std::uint8_t tile_pixel(const std::vector<std::uint8_t>& tiles, std::uint16_t code, int x, int y);
    static std::uint8_t text_pixel(const Region& tilemap, std::uint8_t code, int x, int y);
    static std::uint32_t road_color(std::uint8_t pixel, int band, bool edge);
    static std::uint16_t be16(const std::vector<std::uint8_t>& bytes, std::size_t offset);
    static const Region* find_region(const Bus& bus, const char* name);

    static void draw_sky(std::vector<std::uint32_t>& pixels, int horizon);
    static void draw_road(std::vector<std::uint32_t>& pixels, const std::vector<std::uint8_t>& road, const SceneState& scene);

    void draw_runtime_road(
        std::vector<std::uint32_t>& pixels,
        std::vector<std::uint8_t>& layer_mask,
        std::vector<std::uint8_t>& prom_addr,
        std::vector<std::uint8_t>& prom_out,
        std::vector<std::uint8_t>& prom_source,
        std::vector<std::uint8_t>& road_probe,
        const Machine& machine,
        const Bus& bus,
        bool use_prom_mixer,
        std::uint8_t input_mask,
        bool reference_priority = false);

    void draw_runtime_tile_layer(
        std::vector<std::uint32_t>& pixels,
        std::vector<std::uint8_t>& layer_mask,
        const Machine& machine,
        const Bus& bus,
        int layer,
        std::uint8_t layer_bit,
        bool opaque);

    void draw_runtime_text_layer(
        std::vector<std::uint32_t>& pixels,
        std::vector<std::uint8_t>& layer_mask,
        const Bus& bus,
        std::uint8_t layer_bit);

    static void draw_scaled_chunk(
        std::vector<std::uint32_t>& pixels,
        const std::vector<std::uint8_t>& region,
        std::uint16_t code,
        int x,
        int y,
        int width,
        int height,
        bool flip_x,
        bool flip_y,
        std::uint8_t palette_bank,
        const Bus* bus = nullptr,
        std::vector<std::uint8_t>* layer_mask = nullptr,
        std::uint8_t layer_bit = 0,
        std::uint8_t block_mask = 0,
        std::vector<std::uint8_t>* prom_addr = nullptr,
        std::vector<std::uint8_t>* prom_out = nullptr,
        std::vector<std::uint8_t>* prom_source = nullptr,
        std::uint8_t debug_addr = 0,
        std::uint8_t debug_out = 0,
        PromSource debug_source = PromSource::None,
        std::vector<std::uint16_t>* sprite_owner = nullptr,
        std::vector<std::uint16_t>* sprite_coverage = nullptr,
        std::uint16_t sprite_id = 0xffff,
        int diagnostic_palette_mode = 0,
        std::vector<std::uint8_t>* sprite_occupied = nullptr);

    static void draw_sprite(
        std::vector<std::uint32_t>& pixels,
        const Machine& machine,
        const SpriteInstance& sprite,
        const Bus* bus = nullptr,
        std::vector<std::uint8_t>* layer_mask = nullptr,
        std::uint8_t layer_bit = 0,
        std::uint8_t block_mask = 0,
        std::vector<std::uint8_t>* prom_addr = nullptr,
        std::vector<std::uint8_t>* prom_out = nullptr,
        std::vector<std::uint8_t>* prom_source = nullptr,
        std::uint8_t debug_addr = 0,
        std::uint8_t debug_out = 0,
        PromSource debug_source = PromSource::None,
        std::vector<std::uint16_t>* sprite_owner = nullptr,
        std::vector<std::uint16_t>* sprite_coverage = nullptr,
        std::uint16_t sprite_id = 0xffff,
        int diagnostic_palette_mode = 0,
        std::vector<std::uint8_t>* sprite_occupied = nullptr);

    unsigned draw_runtime_sprites(
        std::vector<std::uint32_t>& pixels,
        std::vector<std::uint8_t>& layer_mask,
        std::vector<std::uint8_t>& prom_addr,
        std::vector<std::uint8_t>& prom_out,
        std::vector<std::uint8_t>& prom_source,
        const Machine& machine,
        const Bus& bus,
        bool use_prom_mixer,
        std::uint8_t input_mask,
        bool reference_priority = false,
        std::vector<std::uint16_t>* sprite_owner = nullptr,
        std::vector<std::uint16_t>* sprite_coverage = nullptr,
        bool ascending_order = false);

    static std::uint8_t road_prom_address(std::uint16_t ctl, std::uint8_t pen, bool road_b, std::uint8_t input_mask);
    static std::uint8_t sprite_prom_address(const SpriteInstance& sprite, std::uint16_t raw_attr, std::uint8_t input_mask);

    void note_prom(PromSource source, std::uint8_t address, const PromEntry& entry);
    static void visualise_layer_mask(std::vector<std::uint32_t>& pixels, const std::vector<std::uint8_t>& layer_mask);
    static void visualise_prom_bits(
        std::vector<std::uint32_t>& pixels,
        const std::vector<std::uint8_t>& prom_addr,
        const std::vector<std::uint8_t>& prom_source,
        std::uint8_t enabled_mask);
    static void visualise_prom_trace(
        std::vector<std::uint32_t>& pixels,
        const std::vector<std::uint8_t>& prom_addr,
        const std::vector<std::uint8_t>& prom_out,
        const std::vector<std::uint8_t>& prom_source);
    static void visualise_road_probe(
        std::vector<std::uint32_t>& pixels,
        const std::vector<std::uint8_t>& road_probe);
    void sample_road_ram(const Bus& bus, unsigned frame);
    void visualise_road_ram_map(
        std::vector<std::uint32_t>& pixels,
        const Bus& bus) const;

    void note_road_scanline(std::uint16_t ctl, std::uint16_t xpos, std::uint16_t srcw, std::uint16_t shape);
    void note_road_pixel(std::uint16_t ctl, std::uint8_t pen, bool road_b, bool edge, bool right_side);

    void export_runtime_diagnostics(const Machine& machine, const Runtime& runtime, const SceneState& scene, unsigned frame,
        const std::vector<std::uint32_t>& final_pixels, const std::vector<std::uint8_t>& layer_mask,
        const std::vector<std::uint8_t>& prom_addr, const std::vector<std::uint8_t>& prom_out,
        const std::vector<std::uint8_t>& prom_source, const std::vector<std::uint8_t>& road_probe,
        const std::vector<std::uint16_t>& sprite_owner, const std::vector<std::uint16_t>& sprite_coverage);
    bool diagnostics_frame(unsigned frame) const;
    void update_title(const SceneState& scene);
    void rebuild_debug_sprite_list(const Runtime& runtime, const std::vector<std::uint16_t>& sprite_owner);
    void draw_research_workbench(std::vector<std::uint32_t>& pixels,const Runtime& runtime,unsigned frame,bool paused);
    void update_sprite_change_events(const Runtime& runtime,unsigned frame);
    void draw_script_prompt(std::vector<std::uint32_t>& pixels);
    void draw_debug_overlay(std::vector<std::uint32_t>& pixels, const Runtime& runtime, unsigned frame, bool paused, bool timer_frozen);
    static void debug_text(std::vector<std::uint32_t>& pixels, int x, int y, const std::string& text, std::uint32_t color, int scale = 1);
    static void debug_rect(std::vector<std::uint32_t>& pixels, int x0, int y0, int x1, int y1, std::uint32_t color);
    void update_runtime_title(const Runtime& runtime, const SceneState& scene, unsigned frame, unsigned sprites, bool paused);
    void present(const std::vector<std::uint32_t>& pixels);

    std::array<PromEntry, 256> mix_lut_{};
    std::array<PromEntry, 256> road_lut_{};
    std::array<bool, 256> seen_mix_{};
    std::array<bool, 256> seen_road_a_{};
    std::array<bool, 256> seen_road_b_{};
    std::array<std::uint64_t, 256> mix_hist_{};
    std::array<std::uint64_t, 256> road_a_hist_{};
    std::array<std::uint64_t, 256> road_b_hist_{};
    std::array<std::uint64_t, 8> mix_bit_high_{};
    std::array<std::uint64_t, 8> road_a_bit_high_{};
    std::array<std::uint64_t, 8> road_b_bit_high_{};
    std::array<std::uint64_t, 8> mix_bit_toggle_{};
    std::array<std::uint64_t, 8> road_a_bit_toggle_{};
    std::array<std::uint64_t, 8> road_b_bit_toggle_{};
    std::array<std::uint8_t, 3> last_prom_addr_{};
    std::array<bool, 3> have_last_prom_addr_{};
    bool prom_configured_ = false;
    std::ofstream prom_log_;
    std::ofstream road_probe_log_;
    std::ofstream road_layout_log_;
    std::filesystem::path log_directory_;
    std::vector<unsigned> diagnostics_frames_;
    bool diagnostics_enabled_ = false;
    bool diagnostics_raw_maps_ = false;
    GraphicsTraceConfig forensics_{};

    enum RoadField : std::size_t { RF_CTL = 0, RF_XPOS = 1, RF_SRCW = 2, RF_SHAPE = 3, RF_COUNT = 4 };
    std::array<std::array<std::uint64_t, 16>, RF_COUNT> road_field_high_{};
    std::array<std::array<std::uint64_t, 16>, RF_COUNT> road_field_toggle_{};
    std::array<std::uint16_t, RF_COUNT> road_field_last_{};
    std::array<bool, RF_COUNT> road_field_have_last_{};
    std::array<std::array<std::uint64_t, 256>, RF_COUNT> road_field_low_hist_{};
    std::array<std::array<std::uint64_t, 256>, RF_COUNT> road_field_high_hist_{};
    std::uint64_t road_scanlines_sampled_ = 0;

    // [road A/B][body/edge][pen 0..3]
    std::array<std::array<std::array<std::uint64_t, 4>, 2>, 2> road_pen_hist_{};
    static constexpr std::size_t ROAD_SIGNAL_COUNT = 12;
    std::array<std::uint64_t, ROAD_SIGNAL_COUNT> road_signal_high_{};
    std::array<std::uint64_t, ROAD_SIGNAL_COUNT> road_signal_toggle_{};
    std::uint16_t road_signal_last_ = 0;
    bool road_signal_have_last_ = false;
    std::array<std::uint64_t, 256> road_candidate_a_hist_{};
    std::array<std::uint64_t, 256> road_candidate_b_hist_{};

    static constexpr std::size_t ROAD_RAM_WORDS = 0x1000;
    std::array<std::uint16_t, ROAD_RAM_WORDS> road_prev_be_{};
    std::array<std::uint16_t, ROAD_RAM_WORDS> road_min_be_{};
    std::array<std::uint16_t, ROAD_RAM_WORDS> road_max_be_{};
    std::array<std::uint16_t, ROAD_RAM_WORDS> road_min_le_{};
    std::array<std::uint16_t, ROAD_RAM_WORDS> road_max_le_{};
    std::array<std::uint64_t, ROAD_RAM_WORDS> road_frame_changes_{};
    std::array<std::uint64_t, ROAD_RAM_WORDS> road_nonzero_frames_{};
    std::array<std::uint64_t, ROAD_RAM_WORDS> road_last_write_counts_{};
    std::array<std::uint64_t, ROAD_RAM_WORDS> road_write_value_changes_{};
    std::array<std::uint64_t, ROAD_RAM_WORDS> road_nonzero_write_events_{};
    std::array<std::uint16_t, ROAD_RAM_WORDS> road_write_min_be_{};
    std::array<std::uint16_t, ROAD_RAM_WORDS> road_write_max_be_{};
    std::array<std::uint16_t, ROAD_RAM_WORDS> road_write_last_be_{};
    std::array<std::uint8_t, 0x2000> road_last_bytes_{};
    bool road_have_snapshot_ = false;
    std::uint64_t road_snapshots_ = 0;

    bool debug_overlay_ = false;
    unsigned debug_page_ = 0; // v0.59.4: F11 cycles DRIVING/HANDLING/TARGET/SYSTEM pages
    bool research_workbench_ = false;
    unsigned research_edit_field_ = 0; // 0 address, 1 value, 2 width, 3 CPU, 4 mode
    std::string research_address_hex_ = "10A096";
    std::string research_value_hex_ = "01AA";
    unsigned research_width_ = 2;
    BusSpace research_space_ = BusSpace::Main;
    DebugUiActions::ResearchCommand research_mode_ = DebugUiActions::ResearchCommand::WriteOnce;
    std::deque<SpriteChangeEvent> sprite_change_events_;
    std::vector<DebugSpriteInfo> previous_debug_sprites_;
    bool assist_course_follow_ = false, assist_infinite_time_ = false, assist_unlimited_turbo_ = false, assist_auto_turbo_ = false;
    int selected_sprite_ = -1;
    unsigned checkpoint_slot_ = 0;
    int debug_mouse_x_ = -1, debug_mouse_y_ = -1;
    std::vector<DebugSpriteInfo> debug_sprites_;
    std::vector<std::uint16_t> last_sprite_owner_;
    std::vector<std::uint32_t> last_presented_pixels_;
    std::vector<Options::SpriteSelector> sprite_solo_;
    std::vector<Options::SpriteSelector> sprite_hide_;
    struct RuntimeSpriteOverride { unsigned slot=0; int map=-1; int palette=-1; int visible=-1; };
    std::vector<RuntimeSpriteOverride> runtime_sprite_overrides_;
    int runtime_visual_slot_ = -1;
    std::string runtime_visual_mode_;
    unsigned runtime_visual_frame_ = 0;
    std::vector<Options::SpriteQuadCandidate> sprite_quad_candidates_;
    Options::DiagnosticBackground diagnostic_background_ = Options::DiagnosticBackground::None;
    bool tc0100scn_legacy_x_ = false;
    Options::LayerOffset layer_offset_bg0_{};
    Options::LayerOffset layer_offset_bg1_{};
    Options::LayerOffset layer_offset_text_{};
    Options::LayerOffset layer_offset_sprites_{};
    Options::LayerOffset layer_offset_road_{};
    bool sprite_tie_break_higher_ = false;
    bool tc0100scn_trace_ = false;
    unsigned tc0100scn_trace_from_ = 0, tc0100scn_trace_to_ = 0;
    unsigned tc0100scn_scan_from_ = 0, tc0100scn_scan_to_ = 239, tc0100scn_scan_step_ = 8;
    std::ofstream tc0100scn_trace_log_;

    SDL_Window* window_ = nullptr;
    bool always_on_top_ = false;
    bool fullscreen_ = false;
    bool window_visible_ = true;
    bool window_minimized_ = false;
    int window_scale_ = 3;
    bool script_prompt_active_ = false;
    bool script_prompt_accepted_ = false;
    bool script_prompt_cancelled_ = false;
    std::string script_prompt_message_;
    SDL_Renderer* renderer_ = nullptr;
    SDL_Texture* texture_ = nullptr;
};

}
