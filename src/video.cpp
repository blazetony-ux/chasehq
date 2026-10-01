#include "video.h"
#include "version.h"
#include "tc0100scn_geometry.h"
#include "sprite_priority.h"
#include "sprite_gfx.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cctype>
#include <filesystem>
#include <iomanip>
#include <fstream>
#include <limits>
#include <iostream>
#include <sstream>
#include <string_view>
#include <vector>

namespace chq {

static std::uint32_t diag_crc32(const std::uint8_t* data, std::size_t n) {
    std::uint32_t c = 0xffffffffu;
    for (std::size_t i=0;i<n;++i) { c ^= data[i]; for (int k=0;k<8;++k) c=(c>>1)^(0xedb88320u & (0u-(c&1u))); }
    return c ^ 0xffffffffu;
}
static std::uint32_t diag_adler32(const std::vector<std::uint8_t>& d) {
    std::uint32_t a=1,b=0; for (auto v:d){a=(a+v)%65521u;b=(b+a)%65521u;} return (b<<16)|a;
}
static void diag_be32(std::vector<std::uint8_t>& o,std::uint32_t v){o.push_back(v>>24);o.push_back(v>>16);o.push_back(v>>8);o.push_back(v);}
static void diag_chunk(std::vector<std::uint8_t>& o,const char t[4],const std::vector<std::uint8_t>& d){diag_be32(o,(std::uint32_t)d.size());auto st=o.size();o.insert(o.end(),t,t+4);o.insert(o.end(),d.begin(),d.end());diag_be32(o,diag_crc32(o.data()+st,o.size()-st));}
static bool diag_save_png(const std::filesystem::path& path,int w,int h,const std::vector<std::uint32_t>& argb) {
    if(w<=0||h<=0||argb.size()<(std::size_t)w*h)return false;
    std::vector<std::uint8_t> raw; raw.reserve((std::size_t)h*(1+w*4));
    for(int y=0;y<h;++y){raw.push_back(0);for(int x=0;x<w;++x){auto c=argb[(std::size_t)y*w+x];raw.push_back((c>>16)&255);raw.push_back((c>>8)&255);raw.push_back(c&255);raw.push_back((c>>24)&255);}}
    std::vector<std::uint8_t> z{0x78,0x01}; std::size_t pos=0;
    while(pos<raw.size()){std::size_t n=std::min<std::size_t>(65535,raw.size()-pos);bool last=pos+n==raw.size();z.push_back(last?1:0);std::uint16_t nn=(std::uint16_t)n,nc=(std::uint16_t)~nn;z.push_back(nn);z.push_back(nn>>8);z.push_back(nc);z.push_back(nc>>8);z.insert(z.end(),raw.begin()+pos,raw.begin()+pos+n);pos+=n;}
    diag_be32(z,diag_adler32(raw)); std::vector<std::uint8_t> out{137,80,78,71,13,10,26,10},ih;diag_be32(ih,w);diag_be32(ih,h);ih.insert(ih.end(),{8,6,0,0,0});diag_chunk(out,"IHDR",ih);diag_chunk(out,"IDAT",z);diag_chunk(out,"IEND",{});
    std::ofstream f(path,std::ios::binary);f.write((const char*)out.data(),(std::streamsize)out.size());return (bool)f;
}

bool Video::initialise(const std::filesystem::path& log_directory) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return false;
    }

    const std::string window_title = std::string(kWindowProduct) + " v" + kNativeVersion + " - " + kNativePlatform;
    window_ = SDL_CreateWindow(
        window_title.c_str(),
        WIDTH * 3,
        HEIGHT * 3,
        SDL_WINDOW_RESIZABLE);

    if (!window_) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        return false;
    }

    renderer_ = SDL_CreateRenderer(window_, nullptr);
    if (!renderer_) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << "\n";
        return false;
    }

    texture_ = SDL_CreateTexture(
        renderer_,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        WIDTH,
        HEIGHT);

    if (!texture_) {
        std::cerr << "SDL_CreateTexture failed: " << SDL_GetError() << "\n";
        return false;
    }

    SDL_SetTextureScaleMode(texture_, SDL_SCALEMODE_NEAREST);

    std::error_code ec;
    std::filesystem::create_directories(log_directory, ec);
    log_directory_ = log_directory;
    prom_log_.open(log_directory / "prom_mixer.log", std::ios::out | std::ios::trunc);
    if (prom_log_) {
        prom_log_ << "# Chase H.Q. Native v" << kNativeVersion << " PROM wiring explorer\n"
                  << "# unique accesses: source address raw class block_mask\n";
    }
    road_probe_log_.open(log_directory / "road_probe.log", std::ios::out | std::ios::trunc);
    if (road_probe_log_) {
        road_probe_log_ << "# Chase H.Q. Native v" << kNativeVersion << " TC0150ROD raw-signal probe\n";
    }
    road_layout_log_.open(log_directory / "road_ram_layout.log", std::ios::out | std::ios::trunc);
    if (road_layout_log_) {
        road_layout_log_ << "# Chase H.Q. Native v0.32 TC0150ROD RAM layout explorer\n"
                         << "# Tracks all 0x1000 words at CPU-B 0x800000-0x801fff.\n";
    }
    return true;
}

bool Video::set_always_on_top(bool enabled) {
    if (!window_) return false;
    if (!SDL_SetWindowAlwaysOnTop(window_, enabled)) {
        std::cerr << "SDL_SetWindowAlwaysOnTop failed: " << SDL_GetError() << "\n";
        return false;
    }
    always_on_top_ = enabled;
    return true;
}

bool Video::set_fullscreen(bool enabled) {
    if (!window_) return false;
    if (!SDL_SetWindowFullscreen(window_, enabled)) {
        std::cerr << "SDL_SetWindowFullscreen failed: " << SDL_GetError() << "\n";
        return false;
    }
    fullscreen_ = enabled;
    if (!enabled) SDL_SetWindowSize(window_, WIDTH * window_scale_, HEIGHT * window_scale_);
    return true;
}

bool Video::set_window_scale(int scale) {
    if (!window_ || scale < 1 || scale > 8) return false;
    window_scale_ = scale;
    if (fullscreen_) {
        if (!SDL_SetWindowFullscreen(window_, false)) return false;
        fullscreen_ = false;
    }
    if (!SDL_SetWindowSize(window_, WIDTH * scale, HEIGHT * scale)) {
        std::cerr << "SDL_SetWindowSize failed: " << SDL_GetError() << "\n";
        return false;
    }
    return true;
}

bool Video::show_window(bool visible) {
    if (!window_) return false;
    const bool ok = visible ? SDL_ShowWindow(window_) : SDL_HideWindow(window_);
    if (!ok) { std::cerr << "SDL show/hide failed: " << SDL_GetError() << "\n"; return false; }
    window_visible_ = visible;
    if (visible) window_minimized_ = false;
    return true;
}

bool Video::minimize_window() {
    if (!window_) return false;
    if (!SDL_MinimizeWindow(window_)) { std::cerr << "SDL_MinimizeWindow failed: " << SDL_GetError() << "\n"; return false; }
    window_minimized_ = true; window_visible_ = true; return true;
}

bool Video::restore_window() {
    if (!window_) return false;
    if (!SDL_RestoreWindow(window_)) { std::cerr << "SDL_RestoreWindow failed: " << SDL_GetError() << "\n"; return false; }
    window_minimized_ = false; window_visible_ = true; return true;
}

void Video::configure_diagnostics(const std::vector<unsigned>& frames, bool enabled, bool raw_maps) {
    diagnostics_frames_ = frames;
    diagnostics_enabled_ = enabled;
    diagnostics_raw_maps_ = raw_maps;
}

void Video::add_diagnostics_frame(unsigned frame) {
    if (std::find(diagnostics_frames_.begin(), diagnostics_frames_.end(), frame) == diagnostics_frames_.end()) diagnostics_frames_.push_back(frame);
    diagnostics_enabled_ = true;
}

void Video::configure_forensics(const GraphicsTraceConfig& config) {
    forensics_ = config;
}

void Video::configure_layer_offsets(const Options& options) {
    layer_offset_bg0_ = options.layer_offset_bg0;
    layer_offset_bg1_ = options.layer_offset_bg1;
    layer_offset_text_ = options.layer_offset_text;
    layer_offset_sprites_ = options.layer_offset_sprites;
    layer_offset_road_ = options.layer_offset_road;
}

bool Video::set_layer_offset(const std::string& layer, int x, int y) {
    if(x < -4096 || x > 4096 || y < -4096 || y > 4096) return false;
    if(layer == "all") { for(const auto* name : {"bg0","bg1","text","sprites","road"}) set_layer_offset(name,x,y); return true; }
    auto* offset = layer=="bg0"?&layer_offset_bg0_:layer=="bg1"?&layer_offset_bg1_:layer=="text"?&layer_offset_text_:layer=="sprites"?&layer_offset_sprites_:layer=="road"?&layer_offset_road_:nullptr;
    if(!offset) return false;
    offset->x=x; offset->y=y; return true;
}
std::string Video::layer_offsets_status() const {
    std::ostringstream out; out << "OK";
    for(const auto& entry : {std::pair{"bg0",layer_offset_bg0_},std::pair{"bg1",layer_offset_bg1_},std::pair{"text",layer_offset_text_},std::pair{"sprites",layer_offset_sprites_},std::pair{"road",layer_offset_road_}})
        out << ' ' << entry.first << "_x=" << entry.second.x << ' ' << entry.first << "_y=" << entry.second.y;
    return out.str();
}

void Video::configure_sprite_tie_break(const Options& options) {
    sprite_tie_break_higher_ = options.sprite_tie_break == Options::SpriteTieBreak::HigherSlot;
}

void Video::configure_tc0100scn_trace(const Options& options) {
    tc0100scn_trace_ = options.tc0100scn_trace; tc0100scn_trace_from_=options.tc0100scn_trace_from; tc0100scn_trace_to_=options.tc0100scn_trace_to;
    tc0100scn_scan_from_=options.tc0100scn_trace_scanline_from; tc0100scn_scan_to_=options.tc0100scn_trace_scanline_to; tc0100scn_scan_step_=std::max(1u,options.tc0100scn_trace_scanline_step);
    if(tc0100scn_trace_){ tc0100scn_trace_log_.open(log_directory_/"tc0100scn_trace.csv",std::ios::out|std::ios::trunc); tc0100scn_trace_log_<<"frame,screen_y,hardware_y,layer,ctrl0,ctrl1,ctrl2,ctrl3,ctrl4,ctrl5,ctrl6,ctrl7,scrollx,scrolly,rowscroll_address,rowscroll_value,source_x_at_screen0,source_y,tile_x_at_screen0,tile_y,presentation_x,presentation_y,flip\n"; }
}

bool Video::diagnostics_frame(unsigned frame) const {
    if (!diagnostics_enabled_) return false;
    if (diagnostics_frames_.empty()) return false;
    return std::find(diagnostics_frames_.begin(), diagnostics_frames_.end(), frame) != diagnostics_frames_.end();
}

void Video::configure_proms(const Machine& machine) {
    const auto& mix = machine.priority_prom();
    const auto& road = machine.road_priority_prom();

    for (std::size_t i = 0; i < 256; ++i) {
        const std::uint8_t mr = i < mix.size() ? mix[i] : 0;
        const std::uint8_t rr = i < road.size() ? road[i] : 0;

        // B52-01 is treated as a mask generator, not as a scalar priority.
        // Each output bit blocks the sprite behind one already-present class.
        // Polarity/pin wiring is still under investigation, but preserving the
        // four raw output bits separately makes the experiment observable.
        mix_lut_[i].raw = mr;
        mix_lut_[i].output_class = static_cast<std::uint8_t>(mr & 0x0f);
        mix_lut_[i].block_mask = static_cast<std::uint8_t>(
            ((mr & 0x01) ? LAYER_BG_BOTTOM : 0) |
            ((mr & 0x02) ? LAYER_ROAD_A : 0) |
            ((mr & 0x04) ? LAYER_ROAD_B : 0) |
            ((mr & 0x08) ? LAYER_BG_UPPER : 0));

        // B52-06 controls the relationship between the two road sources.
        // Keep the raw nibble and expose its low two bits as an internal road
        // class. block_mask is used only for A/B arbitration, never to flatten
        // the result into the old v0.15 numeric priority.
        road_lut_[i].raw = rr;
        road_lut_[i].output_class = static_cast<std::uint8_t>(rr & 0x03);
        road_lut_[i].block_mask = static_cast<std::uint8_t>(
            ((rr & 0x01) ? LAYER_ROAD_A : 0) |
            ((rr & 0x02) ? LAYER_ROAD_B : 0));
    }

    seen_mix_.fill(false);
    seen_road_a_.fill(false);
    seen_road_b_.fill(false);
    mix_hist_.fill(0); road_a_hist_.fill(0); road_b_hist_.fill(0);
    mix_bit_high_.fill(0); road_a_bit_high_.fill(0); road_b_bit_high_.fill(0);
    mix_bit_toggle_.fill(0); road_a_bit_toggle_.fill(0); road_b_bit_toggle_.fill(0);
    have_last_prom_addr_.fill(false); last_prom_addr_.fill(0);
    for (auto& a : road_field_high_) a.fill(0);
    for (auto& a : road_field_toggle_) a.fill(0);
    road_field_last_.fill(0); road_field_have_last_.fill(false);
    for (auto& a : road_field_low_hist_) a.fill(0);
    for (auto& a : road_field_high_hist_) a.fill(0);
    road_scanlines_sampled_ = 0;
    for (auto& src : road_pen_hist_) for (auto& kind : src) kind.fill(0);
    road_signal_high_.fill(0); road_signal_toggle_.fill(0);
    road_signal_last_ = 0; road_signal_have_last_ = false;
    road_candidate_a_hist_.fill(0); road_candidate_b_hist_.fill(0);
    road_prev_be_.fill(0);
    road_min_be_.fill(0xffff); road_max_be_.fill(0);
    road_min_le_.fill(0xffff); road_max_le_.fill(0);
    road_frame_changes_.fill(0); road_nonzero_frames_.fill(0); road_last_write_counts_.fill(0); road_last_bytes_.fill(0);
    road_have_snapshot_ = false; road_snapshots_ = 0;
    prom_configured_ = true;
}


void Video::update_fast_forward_status(unsigned frame, unsigned target) {
    if (!window_) return;
    std::ostringstream ss;
    ss << kWindowProduct << " v" << kNativeVersion << " - FAST FORWARD " << frame << "/" << target;
    SDL_SetWindowTitle(window_, ss.str().c_str());
}


void Video::show_script_prompt(const std::string& message) {
    script_prompt_message_ = message;
    script_prompt_active_ = true;
    script_prompt_accepted_ = false;
    script_prompt_cancelled_ = false;
}
void Video::clear_script_prompt() {
    script_prompt_active_ = false;
    script_prompt_accepted_ = false;
    script_prompt_cancelled_ = false;
    script_prompt_message_.clear();
}

bool Video::process_events(
    bool& running,
    SceneState& scene,
    bool& changed,
    DebugUiActions& actions) {

    changed = false;
    SDL_Event e{};

    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT)
            running = false;

        if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && debug_overlay_) {
            int ww = WIDTH, wh = HEIGHT;
            SDL_GetWindowSize(window_, &ww, &wh);
            if (ww > 0 && wh > 0) {
                debug_mouse_x_ = std::clamp(static_cast<int>(e.button.x * WIDTH / ww), 0, WIDTH - 1);
                debug_mouse_y_ = std::clamp(static_cast<int>(e.button.y * HEIGHT / wh), 0, HEIGHT - 1);
                const std::size_t pi = static_cast<std::size_t>(debug_mouse_y_) * WIDTH + debug_mouse_x_;
                if (pi < last_sprite_owner_.size() && last_sprite_owner_[pi] != 0xffff)
                    selected_sprite_ = static_cast<int>(last_sprite_owner_[pi]);
                else {
                    selected_sprite_ = -1;
                    for (auto it = debug_sprites_.rbegin(); it != debug_sprites_.rend(); ++it) {
                        if (debug_mouse_x_ >= it->x && debug_mouse_x_ < it->x + it->width &&
                            debug_mouse_y_ >= it->y && debug_mouse_y_ < it->y + it->height) {
                            selected_sprite_ = static_cast<int>(it->slot); break;
                        }
                    }
                }
                changed = true;
            }
            continue;
        }

        // v0.66.2: release gameplay controls back through the same IOC path.
        if (!research_workbench_ && !debug_overlay_ && e.type == SDL_EVENT_KEY_UP) {
            const auto* keys = SDL_GetKeyboardState(nullptr);
            if (e.key.scancode == SDL_SCANCODE_LEFT || e.key.scancode == SDL_SCANCODE_RIGHT) {
                const bool left = keys[SDL_SCANCODE_LEFT];
                const bool right = keys[SDL_SCANCODE_RIGHT];
                actions.gameplay_steering_set = true;
                actions.gameplay_steering = left == right ? 0x0000u : (left ? 0x0FA0u : 0x0060u);
                changed = true; continue;
            }
            if (e.key.scancode == SDL_SCANCODE_UP) { actions.gameplay_accel_set=true; actions.gameplay_accel=false; changed=true; continue; }
            if (e.key.scancode == SDL_SCANCODE_DOWN) { actions.gameplay_brake_set=true; actions.gameplay_brake=false; changed=true; continue; }
            if (e.key.scancode == SDL_SCANCODE_SPACE) { actions.gameplay_turbo_set=true; actions.gameplay_turbo=false; changed=true; continue; }
        }

        if (e.type != SDL_EVENT_KEY_DOWN)
            continue;

        // v0.66.3.1: SDL-native Research Script prompt. Enter continues, Escape cancels.
        // Other keys deliberately continue through normal gameplay handling so prompts can
        // ask the researcher to hold a real physical control before continuing.
        if (script_prompt_active_) {
            if (e.key.scancode == SDL_SCANCODE_RETURN || e.key.scancode == SDL_SCANCODE_KP_ENTER) {
                script_prompt_active_ = false; script_prompt_accepted_ = true; script_prompt_cancelled_ = false; changed = true; continue;
            }
            if (e.key.scancode == SDL_SCANCODE_ESCAPE) {
                script_prompt_active_ = false; script_prompt_cancelled_ = true; script_prompt_accepted_ = false; changed = true; continue;
            }
        }

        // RC2.9: immediate SDL pause/resume for manual research capture. Pause/Break
        // is primary. Existing F-key diagnostics remain unchanged; API/script pause/resume remain available.
        if (e.key.scancode == SDL_SCANCODE_PAUSE) {
            actions.toggle_pause = true; changed = true; continue;
        }

        // v0.60.0: F12 opens a dedicated live Research Workbench.  It is kept
        // separate from the compact F1 HUD so investigation controls can grow
        // without making the gameplay overlay unreadable.
        if (e.key.scancode == SDL_SCANCODE_F12) {
            research_workbench_ = !research_workbench_;
            if (research_workbench_) { actions.workbench_opened = true; sprite_change_events_.clear(); previous_debug_sprites_.clear(); }
            else actions.workbench_closed = true;
            changed = true;
            continue;
        }

        if (research_workbench_) {
            auto hex_digit = [](SDL_Scancode sc)->char {
                if (sc >= SDL_SCANCODE_0 && sc <= SDL_SCANCODE_9) return char('0' + (sc - SDL_SCANCODE_0));
                if (sc >= SDL_SCANCODE_A && sc <= SDL_SCANCODE_F) return char('A' + (sc - SDL_SCANCODE_A));
                return 0;
            };
            const char hd = hex_digit(e.key.scancode);
            if (hd && (research_edit_field_==0 || research_edit_field_==1)) {
                auto& dst = research_edit_field_==0 ? research_address_hex_ : research_value_hex_;
                const std::size_t limit = research_edit_field_==0 ? 6u : (research_width_==1?2u:(research_width_==2?4u:8u));
                if (dst.size() < limit) dst.push_back(hd);
                changed = true; continue;
            }
            switch(e.key.scancode) {
            case SDL_SCANCODE_ESCAPE: research_workbench_=false; actions.workbench_closed=true; changed=true; continue;
            case SDL_SCANCODE_SPACE: actions.toggle_pause=true; changed=true; continue;
            case SDL_SCANCODE_PERIOD: actions.step_frames += (e.key.mod & SDL_KMOD_SHIFT)?60u:1u; changed=true; continue;
            case SDL_SCANCODE_F2: actions.screenshot=true; changed=true; continue;
            case SDL_SCANCODE_F3: if(e.key.mod & SDL_KMOD_SHIFT) actions.step_instruction_b=true; else actions.step_instruction_a=true; changed=true; continue;
            case SDL_SCANCODE_F5: actions.snapshot_pause_point=true; changed=true; continue;
            case SDL_SCANCODE_F6: actions.step_frames+=1; changed=true; continue;
            case SDL_SCANCODE_F7: actions.step_frames+=5; changed=true; continue;
            case SDL_SCANCODE_F8: actions.step_frames+=30; changed=true; continue;
            case SDL_SCANCODE_F9: actions.restore_pause_point=true; changed=true; continue;
            case SDL_SCANCODE_F10: actions.export_bundle=true; changed=true; continue;
            case SDL_SCANCODE_DELETE:
                if(e.key.mod & SDL_KMOD_CTRL) actions.clear_research_events=true;
                else if(e.key.mod & SDL_KMOD_SHIFT) actions.clear_research_watches=true;
                else actions.clear_research_patches=true;
                changed=true; continue;
            case SDL_SCANCODE_BACKSPACE: {
                if(research_edit_field_==0 && !research_address_hex_.empty()) research_address_hex_.pop_back();
                if(research_edit_field_==1 && !research_value_hex_.empty()) research_value_hex_.pop_back();
                changed=true; continue; }
            case SDL_SCANCODE_TAB:
                research_edit_field_=(research_edit_field_+1u)%5u; changed=true; continue;
            case SDL_SCANCODE_LEFT:
            case SDL_SCANCODE_RIGHT: {
                const int dir=e.key.scancode==SDL_SCANCODE_RIGHT?1:-1;
                if(research_edit_field_==2){ const unsigned widths[3]={1,2,4}; unsigned wi=research_width_==1?0:(research_width_==2?1:2); wi=(wi+3+dir)%3; research_width_=widths[wi]; research_value_hex_.clear(); }
                else if(research_edit_field_==3) research_space_=research_space_==BusSpace::Main?BusSpace::Sub:BusSpace::Main;
                else if(research_edit_field_==4){
                    using RC=DebugUiActions::ResearchCommand; const RC modes[]={RC::WriteOnce,RC::Freeze,RC::Suppress,RC::WatchWrite,RC::WatchChange};
                    unsigned mi=0; for(unsigned i=0;i<5;++i) if(modes[i]==research_mode_) mi=i;
                    mi=(mi+5+dir)%5; research_mode_=modes[mi];
                }
                changed=true; continue; }
            case SDL_SCANCODE_RETURN:
            case SDL_SCANCODE_KP_ENTER: {
                auto parse_hex=[](const std::string& t)->std::uint32_t{ std::uint32_t v=0; for(char c:t){v<<=4; if(c>='0'&&c<='9')v|=unsigned(c-'0'); else if(c>='A'&&c<='F')v|=unsigned(c-'A'+10);} return v; };
                if(!research_address_hex_.empty()){
                    actions.research_command=research_mode_; actions.research_space=research_space_; actions.research_address=parse_hex(research_address_hex_); actions.research_value=parse_hex(research_value_hex_); actions.research_size=research_width_; changed=true;
                }
                continue; }
            default: break;
            }
            // While the Workbench owns focus, do not leak keys into road/render controls.
            continue;
        }

        const int step =
            (e.key.mod & SDL_KMOD_SHIFT) ? 8 : 2;

        switch (e.key.scancode) {
        case SDL_SCANCODE_F1:
            debug_overlay_ = !debug_overlay_; changed = true; break;
        case SDL_SCANCODE_SPACE:
            if (debug_overlay_) { actions.toggle_pause = true; changed = true; break; }
            actions.gameplay_turbo_set = true; actions.gameplay_turbo = true; changed = true; break;
        case SDL_SCANCODE_PERIOD:
            if (debug_overlay_) { actions.step_frames += (e.key.mod & SDL_KMOD_SHIFT) ? 60u : 1u; changed = true; break; }
            break;
        case SDL_SCANCODE_F2:
            if (debug_overlay_) { actions.screenshot = true; changed = true; break; }
            break;
        case SDL_SCANCODE_F3:
            if (debug_overlay_) { actions.capture_graphics = true; changed = true; break; }
            break;
        case SDL_SCANCODE_F4:
            if (debug_overlay_) { actions.timer_freeze_toggle = true; changed = true; break; }
            break;
        case SDL_SCANCODE_F5:
            if (debug_overlay_) { actions.save_checkpoint = true; changed = true; break; }
            break;
        case SDL_SCANCODE_F6:
            if (debug_overlay_) { actions.timer_preset_bcd = 0x60; changed = true; break; }
            break;
        case SDL_SCANCODE_F7:
            if (debug_overlay_) { actions.timer_preset_bcd = 0x10; changed = true; break; }
            break;
        case SDL_SCANCODE_F8:
            if (debug_overlay_) { actions.timer_preset_bcd = 0x01; changed = true; break; }
            break;
        case SDL_SCANCODE_F9:
            if (debug_overlay_) { actions.load_checkpoint = true; changed = true; break; }
            break;
        case SDL_SCANCODE_F10:
            if (debug_overlay_) { actions.export_bundle = true; changed = true; break; }
            break;
        case SDL_SCANCODE_F11:
            if (debug_overlay_) {
                if (e.key.mod & SDL_KMOD_SHIFT) debug_page_ = (debug_page_ + 3u) % 4u;
                else debug_page_ = (debug_page_ + 1u) % 4u;
                changed = true; break;
            }
            break;
        case SDL_SCANCODE_TAB:
            if (debug_overlay_ && !debug_sprites_.empty()) {
                std::size_t idx = 0;
                for (std::size_t i=0;i<debug_sprites_.size();++i) if ((int)debug_sprites_[i].slot == selected_sprite_) { idx=(i+1)%debug_sprites_.size(); break; }
                selected_sprite_ = static_cast<int>(debug_sprites_[idx].slot); changed = true; break;
            }
            break;
        case SDL_SCANCODE_ESCAPE:
            running = false;
            break;

        case SDL_SCANCODE_LEFT:
        case SDL_SCANCODE_RIGHT: {
            const auto* keys = SDL_GetKeyboardState(nullptr);
            const bool left = keys[SDL_SCANCODE_LEFT];
            const bool right = keys[SDL_SCANCODE_RIGHT];
            actions.gameplay_steering_set = true;
            actions.gameplay_steering = left == right ? 0x0000u : (left ? 0x0FA0u : 0x0060u);
            changed = true;
            break;
        }

        case SDL_SCANCODE_UP:
            actions.gameplay_accel_set = true; actions.gameplay_accel = true; changed = true; break;

        case SDL_SCANCODE_DOWN:
            actions.gameplay_brake_set = true; actions.gameplay_brake = true; changed = true; break;

        case SDL_SCANCODE_A:
            scene.road_offset = (scene.road_offset - step * 4) & 0x3ff;
            changed = true;
            break;

        case SDL_SCANCODE_D:
            scene.road_offset = (scene.road_offset + step * 4) & 0x3ff;
            changed = true;
            break;

        case SDL_SCANCODE_R:
            scene.show_road = !scene.show_road;
            changed = true;
            break;

        case SDL_SCANCODE_T:
            scene.show_sprites = !scene.show_sprites;
            changed = true;
            break;

        case SDL_SCANCODE_Y:
            scene.show_tiles = !scene.show_tiles;
            changed = true;
            break;

        case SDL_SCANCODE_U:
            scene.show_text = !scene.show_text;
            changed = true;
            break;

        case SDL_SCANCODE_I:
            scene.show_priority = !scene.show_priority;
            changed = true;
            break;

        case SDL_SCANCODE_O:
            scene.show_prom_trace = !scene.show_prom_trace;
            changed = true;
            break;

        case SDL_SCANCODE_P:
            if (scene.use_reference_mixer) { scene.use_reference_mixer = false; scene.use_prom_mixer = true; }
            else if (scene.use_prom_mixer) { scene.use_prom_mixer = false; }
            else { scene.use_reference_mixer = true; }
            changed = true;
            break;

        case SDL_SCANCODE_B:
            scene.show_prom_bits = !scene.show_prom_bits;
            changed = true;
            break;

        case SDL_SCANCODE_V:
            scene.show_road_probe = !scene.show_road_probe;
            changed = true;
            break;

        case SDL_SCANCODE_H:
            scene.show_road_ram_map = !scene.show_road_ram_map;
            changed = true;
            break;

        case SDL_SCANCODE_LEFTBRACKET:
            scene.road_ram_scanline = (scene.road_ram_scanline + 255) & 0xff;
            changed = true;
            break;

        case SDL_SCANCODE_RIGHTBRACKET:
            scene.road_ram_scanline = (scene.road_ram_scanline + 1) & 0xff;
            changed = true;
            break;

        case SDL_SCANCODE_1: case SDL_SCANCODE_2: case SDL_SCANCODE_3: case SDL_SCANCODE_4:
        case SDL_SCANCODE_5: case SDL_SCANCODE_6: case SDL_SCANCODE_7: case SDL_SCANCODE_8: {
            if (debug_overlay_) { checkpoint_slot_ = static_cast<unsigned>(e.key.scancode - SDL_SCANCODE_0); changed = true; break; }
            const int bit = static_cast<int>(e.key.scancode - SDL_SCANCODE_1);
            scene.prom_input_mask ^= static_cast<std::uint8_t>(1u << bit);
            changed = true;
            break;
        }

        case SDL_SCANCODE_9:
            if (debug_overlay_) { checkpoint_slot_ = 9; changed = true; }
            break;

        case SDL_SCANCODE_0:
            if (debug_overlay_) { checkpoint_slot_ = 0; changed = true; break; }
            scene.prom_input_mask = 0xff;
            changed = true;
            break;

        case SDL_SCANCODE_HOME:
            scene = SceneState{};
            changed = true;
            break;

        default:
            break;
        }
    }

    return running;
}

std::uint8_t Video::sprite_pixel(
    const std::vector<std::uint8_t>& region,
    std::size_t tile_index,
    int x,
    int y) {

    return decode_sprite_pixel_4bpp(region, tile_index, x, y);
}

std::uint32_t Video::sprite_color(
    std::uint8_t palette_bank,
    std::uint8_t pen) {

    if (pen == 0)
        return 0;

    const std::uint8_t phase =
        static_cast<std::uint8_t>((palette_bank * 29) & 0xff);

    const std::uint8_t r =
        static_cast<std::uint8_t>(45 + ((pen * 51 + phase) % 200));

    const std::uint8_t g =
        static_cast<std::uint8_t>(45 + ((pen * 89 + phase / 2) % 200));

    const std::uint8_t b =
        static_cast<std::uint8_t>(45 + ((pen * 137 + phase / 3) % 200));

    return 0xff000000u |
           (static_cast<std::uint32_t>(r) << 16) |
           (static_cast<std::uint32_t>(g) << 8) |
           b;
}


std::uint32_t Video::runtime_palette_color(
    const Bus& bus,
    std::uint16_t palette_bank,
    std::uint8_t pen) {

    if (pen == 0)
        return 0;

    // TC0110PCR entries are 16-bit RGB data. This first runtime view uses
    // the common xBBBBBGGGGGRRRRR interpretation; keeping the conversion
    // isolated here makes it easy to refine when we validate against MAME.
    const std::uint16_t index = static_cast<std::uint16_t>(((palette_bank & 0xff) << 4) | (pen & 0x0f));
    const std::uint16_t raw = bus.palette[index & 0x0fff];
    const auto expand5 = [](unsigned v) -> std::uint8_t {
        v &= 0x1f;
        return static_cast<std::uint8_t>((v << 3) | (v >> 2));
    };
    const std::uint8_t r = expand5(raw >> 0);
    const std::uint8_t g = expand5(raw >> 5);
    const std::uint8_t b = expand5(raw >> 10);

    // Hardware value $0000 is black. Debug pseudo-colours are never
    // substituted into the normal runtime image in v0.38.

    return 0xff000000u |
        (static_cast<std::uint32_t>(r) << 16) |
        (static_cast<std::uint32_t>(g) << 8) |
        b;
}

std::uint16_t Video::be16(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    if (offset + 1 >= bytes.size())
        return 0;
    return static_cast<std::uint16_t>((bytes[offset] << 8) | bytes[offset + 1]);
}

const Region* Video::find_region(const Bus& bus, const char* name) {
    for (const auto& r : bus.regions)
        if (std::string_view(r.name) == name)
            return &r;
    return nullptr;
}

// The TC0150ROD ROM stores 2bpp pixels in 16-bit words.
// A 0x200-byte tile is 256 words = two 1024-pixel lines:
// logical_x 0..1023   = edge line
// logical_x 1024..2047 = body line
std::uint8_t Video::road_pixel(
    const std::vector<std::uint8_t>& road,
    int tile,
    int logical_x) {

    tile &= 0x3ff;
    logical_x &= 0x7ff;

    const std::size_t word_index =
        static_cast<std::size_t>(tile) * 0x100 +
        static_cast<std::size_t>(logical_x >> 3);

    const std::size_t byte_index = word_index * 2;
    if (byte_index + 1 >= road.size())
        return 0;

    const std::uint16_t word =
        static_cast<std::uint16_t>(road[byte_index]) |
        (static_cast<std::uint16_t>(road[byte_index + 1]) << 8);

    const int bit = 7 - (logical_x & 7);

    return static_cast<std::uint8_t>(
        (((word >> (bit + 8)) & 1) << 1) |
         ((word >> bit) & 1));
}


std::uint8_t Video::tile_pixel(
    const std::vector<std::uint8_t>& tiles,
    std::uint16_t code, int x, int y) {

    const std::size_t base = static_cast<std::size_t>(code) * 32;
    const std::size_t off = base + static_cast<std::size_t>(y) * 4 + static_cast<std::size_t>(x >> 1);
    if (off >= tiles.size()) return 0;
    const std::uint8_t b = tiles[off];
    return (x & 1) ? static_cast<std::uint8_t>(b & 0x0f)
                   : static_cast<std::uint8_t>(b >> 4);
}

std::uint8_t Video::text_pixel(const Region& tilemap, std::uint8_t code, int x, int y) {
    // TC0100SCN RAM chars: 256 x 8x8, 2bpp, 16 bytes/character.
    const std::size_t off = 0x6000 + static_cast<std::size_t>(code) * 16 + static_cast<std::size_t>(y) * 2;
    if (off + 1 >= tilemap.bytes.size()) return 0;
    const int bit = 7 - x;
    const std::uint8_t p0 = static_cast<std::uint8_t>((tilemap.bytes[off + 0] >> bit) & 1);
    const std::uint8_t p1 = static_cast<std::uint8_t>((tilemap.bytes[off + 1] >> bit) & 1);
    return static_cast<std::uint8_t>(p0 | (p1 << 1));
}

std::uint32_t Video::road_color(
    std::uint8_t pixel,
    int band,
    bool edge) {

    if (edge) {
        static constexpr std::uint32_t edge_cols[4] = {
            0xff355b2du, 0xffeeeeeeu, 0xffb73333u, 0xfff0d36cu
        };
        return edge_cols[pixel & 3];
    }

    const bool alt = (band & 1) != 0;
    static constexpr std::uint32_t body_a[4] = {
        0xff252525u, 0xff444444u, 0xff777777u, 0xffeeeeeeu
    };
    static constexpr std::uint32_t body_b[4] = {
        0xff282828u, 0xff4b4b4bu, 0xff707070u, 0xffd8d8d8u
    };

    return alt ? body_b[pixel & 3] : body_a[pixel & 3];
}

void Video::draw_sky(
    std::vector<std::uint32_t>& pixels,
    int horizon) {

    for (int y = 0; y < HEIGHT; ++y) {
        std::uint32_t c;

        if (y < horizon) {
            const int t = (y * 80) / std::max(1, horizon);
            const std::uint8_t r = static_cast<std::uint8_t>(22 + t / 3);
            const std::uint8_t g = static_cast<std::uint8_t>(48 + t / 2);
            const std::uint8_t b = static_cast<std::uint8_t>(92 + t);
            c = 0xff000000u |
                (static_cast<std::uint32_t>(r) << 16) |
                (static_cast<std::uint32_t>(g) << 8) |
                b;
        } else {
            c = 0xff31552bu;
        }

        for (int x = 0; x < WIDTH; ++x)
            pixels[y * WIDTH + x] = c;
    }
}

void Video::draw_road(
    std::vector<std::uint32_t>& pixels,
    const std::vector<std::uint8_t>& road,
    const SceneState& scene) {

    const int horizon = scene.horizon;

    for (int y = horizon; y < HEIGHT; ++y) {
        const int depth = y - horizon;
        const int denom = std::max(1, HEIGHT - horizon - 1);

        // Perspective: road widens quadratically toward the viewer.
        const double p =
            static_cast<double>(depth) /
            static_cast<double>(denom);

        const int half_width =
            10 + static_cast<int>(150.0 * p * p);

        const int curve_shift =
            static_cast<int>(
                scene.curve * (1.0 - p) * (1.0 - p));

        const int center =
            WIDTH / 2 + curve_shift;

        const int left = center - half_width;
        const int right = center + half_width;

        // Higher road tile numbers are generally wider in Chase H.Q.'s data.
        const int tile =
            std::clamp(
                1 + static_cast<int>(p * 0x1ff),
                1,
                0x1ff);

        const int band =
            ((depth + scene.road_offset / 4) / 12) & 1;

        // Fill verge.
        for (int x = 0; x < WIDTH; ++x) {
            if (x >= left && x <= right)
                continue;

            // Use edge ROM line around the road boundaries.
            const int d =
                x < left ? left - x : x - right;

            if (d < 22) {
                const int edge_src =
                    x < left
                        ? (511 - d * 10 + scene.road_offset)
                        : (512 + d * 10 + scene.road_offset);

                const auto pix =
                    road_pixel(road, tile, edge_src);

                pixels[y * WIDTH + x] =
                    road_color(pix, band, true);
            }
        }

        if (left > right)
            continue;

        // Road body samples the second 1024-pixel line.
        const int span = std::max(1, right - left + 1);

        for (int x = std::max(0, left);
             x <= std::min(WIDTH - 1, right);
             ++x) {

            const int rel = x - left;

            int src =
                1024 +
                (rel * 1023) / span;

            src =
                1024 +
                ((src - 1024 + scene.road_offset) & 0x3ff);

            const auto pix =
                road_pixel(road, tile, src);

            pixels[y * WIDTH + x] =
                road_color(pix, band, false);
        }
    }
}

void Video::draw_scaled_chunk(
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
    const Bus* bus,
    std::vector<std::uint8_t>* layer_mask,
    std::uint8_t layer_bit,
    std::uint8_t block_mask,
    std::vector<std::uint8_t>* prom_addr,
    std::vector<std::uint8_t>* prom_out,
    std::vector<std::uint8_t>* prom_source,
    std::uint8_t debug_addr,
    std::uint8_t debug_out,
    PromSource debug_source,
    std::vector<std::uint16_t>* sprite_owner,
    std::vector<std::uint16_t>* sprite_coverage,
    std::uint16_t sprite_id,
    int diagnostic_palette_mode,
    std::vector<std::uint8_t>* sprite_occupied) {

    if (width <= 0 || height <= 0)
        return;

    const std::size_t tile = static_cast<std::size_t>(code & 0x3fff);

    for (int dy = 0; dy < height; ++dy) {
        int sy = std::clamp((dy * 16) / height, 0, 15);
        if (flip_y) sy = 15 - sy;

        for (int dx = 0; dx < width; ++dx) {
            int sx = std::clamp((dx * 16) / width, 0, 15);
            if (flip_x) sx = 15 - sx;

            const auto pen = sprite_pixel(region, tile, sx, sy);
            if (pen == 0) continue;

            const int px = x + dx;
            const int py = y + dy;
            if (px < 0 || px >= WIDTH || py < 0 || py >= HEIGHT) continue;

            const std::size_t dst = static_cast<std::size_t>(py * WIDTH + px);

            // Preserve BG/road/text arbitration; sprite occupancy is independent.
            // A masked nonzero pen still reserves the pixel, as in MAME pdrawgfx.
            std::uint8_t isolated_occupancy = 0;
            auto& occupied = sprite_occupied ? (*sprite_occupied)[dst] : isolated_occupancy;
            auto* coverage = sprite_coverage ? &(*sprite_coverage)[dst] : nullptr;
            if (!accept_sprite_pixel(pen, layer_mask && ((*layer_mask)[dst] & block_mask),
                                     occupied, coverage)) continue;

            if (bus && diagnostic_palette_mode == 1) {
                const auto pidx = static_cast<std::uint16_t>(((palette_bank & 0xff) << 4) | (pen & 0x0f));
                if (bus->palette[pidx & 0x0fff] == 0) continue; // experiment: unwritten/zero colour is transparent
            }
            if (diagnostic_palette_mode == 2) {
                // Raw-pen forensic view: pen identity only, independent of TC0110PCR.
                const unsigned v = static_cast<unsigned>(pen) * 17u;
                pixels[dst] = 0xff000000u | (v << 16) | (v << 8) | v;
            } else {
                pixels[dst] = bus
                    ? runtime_palette_color(*bus, static_cast<std::uint16_t>(palette_bank), pen)
                    : sprite_color(palette_bank, pen);
            }

            if (layer_mask) (*layer_mask)[dst] |= layer_bit;
            if (prom_addr) (*prom_addr)[dst] = debug_addr;
            if (prom_out) (*prom_out)[dst] = debug_out;
            if (prom_source) (*prom_source)[dst] = static_cast<std::uint8_t>(debug_source);
            if (sprite_owner && dst < sprite_owner->size()) (*sprite_owner)[dst] = sprite_id;
        }
    }
}

void Video::draw_sprite(
    std::vector<std::uint32_t>& pixels,
    const Machine& machine,
    const SpriteInstance& sprite,
    const Bus* bus,
    std::vector<std::uint8_t>* layer_mask,
    std::uint8_t layer_bit,
    std::uint8_t block_mask,
    std::vector<std::uint8_t>* prom_addr,
    std::vector<std::uint8_t>* prom_out,
    std::vector<std::uint8_t>* prom_source,
    std::uint8_t debug_addr,
    std::uint8_t debug_out,
    PromSource debug_source,
    std::vector<std::uint16_t>* sprite_owner,
    std::vector<std::uint16_t>* sprite_coverage,
    std::uint16_t sprite_id,
    int diagnostic_palette_mode,
    std::vector<std::uint8_t>* sprite_occupied) {

    const SpriteFormat format = Machine::format_for_zoomx(sprite.zoom_x);

    int cols = 0;
    int rows = 8;
    std::size_t map_offset = 0;
    const std::vector<std::uint8_t>* gfx = nullptr;

    switch (format) {
    case SpriteFormat::Obj128x128:
        cols = 8;
        map_offset = static_cast<std::size_t>(sprite.tile_number) << 6;
        gfx = &machine.sprites_a();
        break;
    case SpriteFormat::Obj64x128:
        cols = 4;
        map_offset = (static_cast<std::size_t>(sprite.tile_number) << 5) + 0x20000;
        gfx = &machine.sprites_b();
        break;
    case SpriteFormat::Obj32x128:
        cols = 2;
        map_offset = (static_cast<std::size_t>(sprite.tile_number) << 4) + 0x30000;
        gfx = &machine.sprites_b();
        break;
    default:
        return;
    }

    const int base_x = sprite.x;
    const int base_y = sprite.y + (128 - sprite.zoom_y);

    for (int j = 0; j < rows; ++j) {
        for (int k = 0; k < cols; ++k) {
            const int map_x = sprite.flip_x ? (cols - 1 - k) : k;
            const int map_y = sprite.flip_y ? (rows - 1 - j) : j;

            const auto code = machine.spritemap_word(
                map_offset + static_cast<std::size_t>(map_x + map_y * cols));
            // Chase H.Q./Night Striker hardware discards the upper spritemap bits;
            // MAME deliberately still draws 0xffff as tile 0x3fff (used as a mask sprite).

            const int cur_x = base_x + (k * sprite.zoom_x) / cols;
            const int cur_y = base_y + (j * sprite.zoom_y) / rows;
            const int next_x = base_x + ((k + 1) * sprite.zoom_x) / cols;
            const int next_y = base_y + ((j + 1) * sprite.zoom_y) / rows;

            draw_scaled_chunk(
                pixels, *gfx, code,
                cur_x, cur_y, next_x - cur_x, next_y - cur_y,
                sprite.flip_x, sprite.flip_y, sprite.color,
                bus, layer_mask, layer_bit, block_mask,
                prom_addr, prom_out, prom_source,
                debug_addr, debug_out, debug_source,
                sprite_owner, sprite_coverage, sprite_id, diagnostic_palette_mode, sprite_occupied);
        }
    }
}

void Video::draw_runtime_tile_layer(
    std::vector<std::uint32_t>& pixels,
    std::vector<std::uint8_t>& priority,
    const Machine& machine,
    const Bus& bus,
    int layer,
    std::uint8_t pri,
    bool opaque) {

    const Region* tr = find_region(bus, "tilemap");
    const Region* cr = find_region(bus, "tile_control");
    if (!tr || !cr || tr->bytes.size() < 0x10000 || cr->bytes.size() < 0x10)
        return;

    const std::uint16_t ctrl0 = be16(cr->bytes, 0);
    const std::uint16_t ctrl1 = be16(cr->bytes, 2);
    const std::uint16_t ctrl3 = be16(cr->bytes, 6);
    const std::uint16_t ctrl4 = be16(cr->bytes, 8);
    const std::uint16_t ctrl6 = be16(cr->bytes, 12);
    const std::uint16_t ctrl7 = be16(cr->bytes, 14);

    // TC0100SCN standard-width layout: BG0 at 0x0000 bytes, BG1 at 0x8000.
    // Each tile is attribute word + code word.  Layer 0/1 disable bits are 0/1.
    if (ctrl6 & (1u << layer))
        return;

    const std::size_t base = layer ? 0x8000 : 0x0000;
    const std::size_t rowscroll_base = layer ? 0xc400 : 0xc000;
    const bool flip_screen = (ctrl7 & 1) != 0;

    const auto& layer_offset = layer ? layer_offset_bg1_ : layer_offset_bg0_;
    for (int sy = 0; sy < HEIGHT; ++sy) {
        // Presentation offset is applied in output space: negative Y moves the layer upward,
        // so the source coordinate sampled for a given destination pixel moves downward.
        const int sample_y = sy - layer_offset.y;
        const int hw_y = sample_y + VISIBLE_Y_START;
        const std::size_t roff = rowscroll_base + static_cast<std::size_t>((hw_y & 0x1ff) * 2);
        const std::int16_t rowscroll = static_cast<std::int16_t>(be16(tr->bytes, roff));
        for (int sx = 0; sx < WIDTH; ++sx) {
            const int sample_x = sx - layer_offset.x;
            int mx = tc0100scn_legacy_x_ ? (sample_x + static_cast<std::int16_t>(layer ? ctrl1 : ctrl0) + rowscroll + 16) & 0x1ff : tc0100scn_source_x(sample_x, layer ? ctrl1 : ctrl0, rowscroll);
            int my = tc0100scn_source_y(hw_y, layer ? ctrl4 : ctrl3);
            if (flip_screen) { mx = 0x1ff - mx; my = 0x1ff - my; }

            const int tx = (mx >> 3) & 63;
            const int ty = (my >> 3) & 63;
            const int px = mx & 7;
            const int py = my & 7;
            const std::size_t tile_index = static_cast<std::size_t>(ty * 64 + tx);
            const std::size_t off = base + tile_index * 4;
            if (off + 3 >= tr->bytes.size()) continue;

            const std::uint16_t attr = be16(tr->bytes, off);
            const std::uint16_t code = be16(tr->bytes, off + 2);
            const bool fx = (attr & 0x4000) != 0;
            const bool fy = (attr & 0x8000) != 0;
            const int qx = fx ? 7 - px : px;
            const int qy = fy ? 7 - py : py;
            const std::uint8_t pen = tile_pixel(machine.tile_gfx(), code, qx, qy);
            // Pen 0 is transparent on the overlay pass. On the bottom pass it
            // represents the layer's palette entry, but it still participates
            // in the priority bitmap rather than blindly erasing later planes.
            if (!opaque && pen == 0) continue;
            const std::size_t dst = static_cast<std::size_t>(sy * WIDTH + sx);
            pixels[dst] = runtime_palette_color(bus, static_cast<std::uint16_t>(attr & 0xff), pen);
            priority[dst] |= pri;
        }
    }
}

void Video::draw_runtime_text_layer(
    std::vector<std::uint32_t>& pixels,
    std::vector<std::uint8_t>& priority,
    const Bus& bus,
    std::uint8_t pri) {

    const Region* tr = find_region(bus, "tilemap");
    const Region* cr = find_region(bus, "tile_control");
    if (!tr || !cr || tr->bytes.size() < 0x7000 || cr->bytes.size() < 0x10) return;

    const std::uint16_t ctrl2 = be16(cr->bytes, 4);
    const std::uint16_t ctrl5 = be16(cr->bytes, 10);
    const std::uint16_t ctrl6 = be16(cr->bytes, 12);
    const std::uint16_t ctrl7 = be16(cr->bytes, 14);
    if (ctrl6 & 0x04) return;
    const bool flip_screen = (ctrl7 & 1) != 0;

    for (int sy = 0; sy < HEIGHT; ++sy) {
        const int sample_y = sy - layer_offset_text_.y;
        const int hw_y = sample_y + VISIBLE_Y_START;
        for (int sx = 0; sx < WIDTH; ++sx) {
            const int sample_x = sx - layer_offset_text_.x;
            const int mx = tc0100scn_text_source_x(sample_x, ctrl2, flip_screen);
            int my = tc0100scn_source_y(hw_y, ctrl5);
            if (flip_screen) my = 0x1ff - my;
            const int tx = (mx >> 3) & 63, ty = (my >> 3) & 63;
            const std::size_t off = 0x4000 + static_cast<std::size_t>(ty * 64 + tx) * 2;
            if (off + 1 >= tr->bytes.size()) continue;
            const std::uint16_t attr = be16(tr->bytes, off);
            const std::uint8_t code = static_cast<std::uint8_t>(attr & 0xff);
            int px = mx & 7, py = my & 7;
            if (attr & 0x4000) px = 7 - px;
            if (attr & 0x8000) py = 7 - py;
            const std::uint8_t pen = text_pixel(*tr, code, px, py);
            if (pen == 0) continue;
            const std::uint16_t color = static_cast<std::uint16_t>((attr >> 8) & 0x3f);
            const std::size_t dst = static_cast<std::size_t>(sy * WIDTH + sx);
            pixels[dst] = runtime_palette_color(bus, color, pen);
            priority[dst] |= pri;
        }
    }
}


void Video::note_road_scanline(
    std::uint16_t ctl,
    std::uint16_t xpos,
    std::uint16_t srcw,
    std::uint16_t shape) {

    const std::array<std::uint16_t, RF_COUNT> values{ctl, xpos, srcw, shape};
    ++road_scanlines_sampled_;

    for (std::size_t f = 0; f < RF_COUNT; ++f) {
        const std::uint16_t v = values[f];
        ++road_field_low_hist_[f][v & 0xff];
        ++road_field_high_hist_[f][(v >> 8) & 0xff];
        for (int bit = 0; bit < 16; ++bit)
            if (v & (1u << bit)) ++road_field_high_[f][bit];

        if (road_field_have_last_[f]) {
            const std::uint16_t changed = static_cast<std::uint16_t>(road_field_last_[f] ^ v);
            for (int bit = 0; bit < 16; ++bit)
                if (changed & (1u << bit)) ++road_field_toggle_[f][bit];
        }
        road_field_last_[f] = v;
        road_field_have_last_[f] = true;
    }
}

void Video::note_road_pixel(
    std::uint16_t ctl,
    std::uint8_t pen,
    bool road_b,
    bool edge,
    bool right_side) {

    pen &= 3;
    ++road_pen_hist_[road_b ? 1 : 0][edge ? 1 : 0][pen];

    // v0.18 deliberately records more candidate signals than fit in the PROM.
    // This lets the log tell us which inputs are dynamic before we choose PCB
    // address pins. Signal order is documented in road_probe.log.
    std::uint16_t sig = 0;
    sig |= static_cast<std::uint16_t>((pen & 1) << 0);
    sig |= static_cast<std::uint16_t>(((pen >> 1) & 1) << 1);
    sig |= static_cast<std::uint16_t>((road_b ? 1 : 0) << 2);
    sig |= static_cast<std::uint16_t>((edge ? 1 : 0) << 3);
    sig |= static_cast<std::uint16_t>((right_side ? 1 : 0) << 4);
    sig |= static_cast<std::uint16_t>(((ctl >> 0) & 1) << 5);
    sig |= static_cast<std::uint16_t>(((ctl >> 1) & 1) << 6);
    sig |= static_cast<std::uint16_t>(((ctl >> 4) & 1) << 7);
    sig |= static_cast<std::uint16_t>(((ctl >> 5) & 1) << 8);
    sig |= static_cast<std::uint16_t>(((ctl >> 7) & 1) << 9);
    sig |= static_cast<std::uint16_t>(((ctl >> 8) & 1) << 10);
    sig |= static_cast<std::uint16_t>(((ctl >> 9) & 1) << 11);

    for (std::size_t bit = 0; bit < ROAD_SIGNAL_COUNT; ++bit)
        if (sig & (1u << bit)) ++road_signal_high_[bit];

    if (road_signal_have_last_) {
        const std::uint16_t changed = static_cast<std::uint16_t>(road_signal_last_ ^ sig);
        for (std::size_t bit = 0; bit < ROAD_SIGNAL_COUNT; ++bit)
            if (changed & (1u << bit)) ++road_signal_toggle_[bit];
    }
    road_signal_last_ = sig;
    road_signal_have_last_ = true;

    // Probe-only 8-bit candidate address. This is NOT used by the mixer in
    // v0.18. It exists to see whether raw TC0150ROD state can create useful
    // address diversity before we wire it into B52-06/B52-01.
    std::uint8_t candidate = 0;
    candidate |= static_cast<std::uint8_t>((pen & 0x03) << 0); // A0-A1 pixel
    candidate |= static_cast<std::uint8_t>((edge ? 1 : 0) << 2);
    candidate |= static_cast<std::uint8_t>((right_side ? 1 : 0) << 3);
    candidate |= static_cast<std::uint8_t>(((ctl >> 0) & 1) << 4);
    candidate |= static_cast<std::uint8_t>(((ctl >> 4) & 1) << 5);
    candidate |= static_cast<std::uint8_t>(((ctl >> 8) & 1) << 6);
    candidate |= static_cast<std::uint8_t>(((ctl >> 9) & 1) << 7);
    ++(road_b ? road_candidate_b_hist_ : road_candidate_a_hist_)[candidate];
}

void Video::draw_runtime_road(
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
    bool reference_priority) {

    const Region* rr = find_region(bus, "road");
    if (!rr || rr->bytes.size() < 0x2000)
        return;

    // v0.28: TC0150ROD RAM is four 0x800-byte banks of 256 interleaved
    // 8-byte scanline records, not four planar 0x200-byte arrays.  CPU-B
    // tracing proved the live Chase H.Q. writer emits exactly this layout.
    // The control word at $801FFE selects the A/B banks. Chase H.Q. uses a
    // y offset of -1 and palette base $c0 (matching the hardware behaviour
    // documented by the established TC0150ROD implementation).
    const std::uint16_t road_ctrl = be16(rr->bytes, 0x1ffe);
    constexpr int y_offs = -1;
    constexpr int x_offs = 0x0a7;
    constexpr std::uint16_t palette_offs = 0x00c0;

    const std::size_t road_a_word_base = static_cast<std::size_t>((road_ctrl & 0x0300) << 2);
    const std::size_t road_b_word_base = static_cast<std::size_t>((road_ctrl & 0x0c00));

    auto draw_source = [&](int y, std::size_t word_base, bool road_b) {
        const int line = (y - layer_offset_road_.y) + VISIBLE_Y_START + y_offs;
        if (line < 0 || line > 255) return;
        const std::size_t wi = word_base + static_cast<std::size_t>(line) * 4;
        const std::size_t off = wi * 2;
        if (off + 7 >= rr->bytes.size()) return;

        // Confirmed TC0150ROD record layout:
        // W0 right clip/control, W1 left clip/control,
        // W2 body X offset/control, W3 colour bank + road gfx tile.
        const std::uint16_t clipr = be16(rr->bytes, off + 0);
        const std::uint16_t clipl = be16(rr->bytes, off + 2);
        const std::uint16_t body  = be16(rr->bytes, off + 4);
        const std::uint16_t gfx   = be16(rr->bytes, off + 6);
        note_road_scanline(clipr, clipl, body, gfx);

        if ((clipr | clipl) == 0) return;
        // Road B is only active when the TC0150ROD control enables it.
        if (road_b && !(road_ctrl & 0x0800)) return;

        const int xoffset = body & 0x07ff;
        const int tile = gfx & 0x03ff;
        const int colbank = (gfx & 0xf000) >> 10;
        const int pal_body = (body & 0x1800) >> 11;
        const int pal_left = (clipl & 0x1000) >> 11;
        const int pal_right = (clipr & 0x1000) >> 11;
        const int road_center = 0x5ff - ((-xoffset + x_offs) & 0x7ff);
        const int left_edge = road_center - (clipl & 0x03ff);
        const int right_edge = road_center + 1 + (clipr & 0x03ff);

        auto plot = [&](int logical_x, std::uint8_t pen, bool edge, bool right_side, int paloff) {
            // TC0150ROD line graphics are stored back-to-front; the original
            // hardware-facing implementation reverses the completed line.
            const int x = WIDTH - 1 - logical_x + layer_offset_road_.x;
            if (x < 0 || x >= WIDTH) return;
            const std::size_t dst = static_cast<std::size_t>(y * WIDTH + x);
            note_road_pixel(body, pen, road_b, edge, right_side);

            const std::uint8_t probe = static_cast<std::uint8_t>(
                (pen & 3) | (road_b ? 0x04 : 0) | (edge ? 0x08 : 0) |
                (right_side ? 0x10 : 0) | ((body & 1) ? 0x20 : 0) |
                ((body & 0x80) ? 0x40 : 0) | ((gfx & 0x8000) ? 0x80 : 0));
            road_probe[dst] = probe;

            const std::uint8_t bit = reference_priority ? static_cast<std::uint8_t>(road_b ? 0x02 : 0x01) : (road_b ? LAYER_ROAD_B : LAYER_ROAD_A);
            std::uint8_t addr = 0;
            PromEntry entry{};
            if (!reference_priority && use_prom_mixer && prom_configured_) {
                addr = road_prom_address(body, pen, road_b, input_mask);
                entry = road_lut_[addr];
                note_prom(road_b ? PromSource::RoadB : PromSource::RoadA, addr, entry);
                const std::uint8_t opposite = road_b ? LAYER_ROAD_A : LAYER_ROAD_B;
                if ((entry.block_mask & opposite) && (layer_mask[dst] & opposite)) return;
            }

            const std::uint16_t bank = static_cast<std::uint16_t>(palette_offs + colbank + paloff);
            // Chase H.Q. TC0150ROD type 0 uses pens 4..7 for road pixels.
            pixels[dst] = runtime_palette_color(bus, bank, static_cast<std::uint8_t>(pen + 4));
            layer_mask[dst] |= bit;
            if (!reference_priority && use_prom_mixer && prom_configured_) {
                prom_addr[dst] = addr;
                prom_out[dst] = entry.raw;
                prom_source[dst] = static_cast<std::uint8_t>(road_b ? PromSource::RoadB : PromSource::RoadA);
            }
        };

        // Body: second 1024-pixel half of each 0x200-byte road-gfx tile.
        const int begin = std::max(0, left_edge + 1);
        const int end = std::min(WIDTH, right_edge);
        int xi = (-xoffset + x_offs + begin) & 0x7ff;
        for (int x = begin; x < end; ++x, xi = (xi + 1) & 0x7ff) {
            if (!tile) continue;
            const std::uint8_t pen = road_pixel(machine.road_gfx(), tile, xi);
            if (pen) plot(x, pen, false, false, pal_body);
        }

        // Left/right edge graphics use the first 1024-pixel half. Pen zero is
        // transparent unless the corresponding background-fill control bit is set.
        if (left_edge >= 0 && left_edge < WIDTH) {
            int ex = 511;
            for (int x = left_edge; x >= 0; --x, ex = (ex - 1) & 0x7ff) {
                const std::uint8_t pen = road_pixel(machine.road_gfx(), tile, ex);
                if (pen || (clipl & 0x8000)) plot(x, pen, true, false, pal_left);
            }
        }
        if (right_edge >= 0 && right_edge < WIDTH) {
            int ex = 512;
            for (int x = right_edge; x < WIDTH; ++x, ex = (ex + 1) & 0x7ff) {
                const std::uint8_t pen = road_pixel(machine.road_gfx(), tile, ex);
                if (pen || (clipr & 0x8000)) plot(x, pen, true, true, pal_right);
            }
        }
    };

    for (int y = 0; y < HEIGHT; ++y) {
        draw_source(y, road_a_word_base, false);
        draw_source(y, road_b_word_base, true);
    }
}

unsigned Video::draw_runtime_sprites(
    std::vector<std::uint32_t>& pixels,
    std::vector<std::uint8_t>& layer_mask,
    std::vector<std::uint8_t>& prom_addr,
    std::vector<std::uint8_t>& prom_out,
    std::vector<std::uint8_t>& prom_source,
    const Machine& machine,
    const Bus& bus,
    bool use_prom_mixer,
    std::uint8_t input_mask,
    bool reference_priority,
    std::vector<std::uint16_t>* sprite_owner,
    std::vector<std::uint16_t>* sprite_coverage,
    bool ascending_order) {

    const Region* sr = find_region(bus, "sprites");
    if (!sr) return 0;

    unsigned drawn = 0;
    // MAME descends through RAM, but its priority pixel operation reserves each
    // nonzero-pen location: first encountered sprite wins, even if masked by BG.
    // Legacy mode names select traversal, not the winner: lower-slot=descending,
    // higher-slot=ascending. Both use identical occupancy and layer-mask rules.
    std::vector<std::uint8_t> sprite_occupied(WIDTH*HEIGHT, 0);
    const std::size_t sprite_count = sr->bytes.size() / 8;
    std::vector<std::size_t> sprite_order; sprite_order.reserve(sprite_count);
    if (ascending_order) for (std::size_t i=0;i<sprite_count;++i) sprite_order.push_back(i);
    else for (std::size_t i=sprite_count;i-->0;) sprite_order.push_back(i);
    for (std::size_t pos : sprite_order) {
        const std::size_t off = pos * 8;
        const std::uint16_t w0 = be16(sr->bytes, off + 0);
        const std::uint16_t w1 = be16(sr->bytes, off + 2);
        const std::uint16_t w2 = be16(sr->bytes, off + 4);
        const std::uint16_t w3 = be16(sr->bytes, off + 6);

        int y = sprite_visible_y(w0, VISIBLE_Y_START, layer_offset_sprites_.y);

        const int zoom_y = ((w0 >> 9) & 0x7f) + 1;
        const int zoom_x = (w1 & 0x7f) + 1;
        std::uint8_t color = static_cast<std::uint8_t>((w1 & 0x7f80) >> 7);
        const bool priority = (w1 & 0x8000) != 0;
        int x = w2 & 0x1ff;
        if (x > 0x140) x -= 0x200;
        x += layer_offset_sprites_.x;

        // Chase H.Q. uses 11 sprite-number bits.  Higher W3 bits are observed
        // by the original game too, but are not part of the spritemap index.
        std::uint16_t tile = static_cast<std::uint16_t>(w3 & 0x07ff);
        if (tile == 0) continue;

        for (const auto& ov : runtime_sprite_overrides_) if (ov.slot == pos) {
            if (ov.visible == 0) tile = 0;
            if (ov.map >= 0) tile = static_cast<std::uint16_t>(ov.map & 0x07ff);
            if (ov.palette >= 0) color = static_cast<std::uint8_t>(ov.palette & 0xff);
            break;
        }
        if (tile == 0) continue;
        if (runtime_visual_slot_ >= 0) {
            if (runtime_visual_mode_ == "solo" && static_cast<int>(pos) != runtime_visual_slot_) continue;
            if (runtime_visual_mode_ == "hide" && static_cast<int>(pos) == runtime_visual_slot_) continue;
            if (runtime_visual_mode_ == "flash" && static_cast<int>(pos) == runtime_visual_slot_ && ((runtime_visual_frame_ / 15u) & 1u)) continue;
        }

        const auto selector_match = [&](const Options::SpriteSelector& sel) { return sel.matches((unsigned)pos, tile, color, priority ? 1 : 0); };
        if (!sprite_solo_.empty() && std::none_of(sprite_solo_.begin(), sprite_solo_.end(), selector_match)) continue;
        if (!sprite_hide_.empty() && std::any_of(sprite_hide_.begin(), sprite_hide_.end(), selector_match)) continue;

        SpriteInstance sprite;
        sprite.x = x;
        sprite.y = y;
        sprite.zoom_x = zoom_x;
        sprite.zoom_y = zoom_y;
        sprite.tile_number = tile;
        sprite.color = color;
        sprite.flip_x = (w2 & 0x4000) != 0;
        sprite.flip_y = (w2 & 0x8000) != 0;
        sprite.priority = priority ? 1 : 0;

        if (Machine::format_for_zoomx(sprite.zoom_x) == SpriteFormat::Invalid) continue;

        std::uint8_t block_mask = 0;
        std::uint8_t addr = 0;
        std::uint8_t raw = 0;

        if (reference_priority) {
            // Chase H.Q. reference path: MAME's pdrawgfx primasks are
            // {0xf0,0xfc}.  The priority bitmap contains the values written
            // by upper BG (1), TC0150ROD (1/2) and text (4).
            block_mask = sprite.priority ? 0xfc : 0xf0;
        } else if (use_prom_mixer && prom_configured_) {
            addr = sprite_prom_address(sprite, w2, input_mask);
            const PromEntry& entry = mix_lut_[addr];
            raw = entry.raw;
            block_mask = entry.block_mask;
            note_prom(PromSource::Sprite, addr, entry);
        } else {
            block_mask = sprite.priority ? 0 : LAYER_BG_UPPER;
        }

        const int forensic_palette_mode =
            (!sprite_solo_.empty() && diagnostic_background_ != Options::DiagnosticBackground::None) ? 2 : 0;
        draw_sprite(
            pixels, machine, sprite, &bus,
            &layer_mask, reference_priority ? 0 : LAYER_SPRITE, block_mask,
            &prom_addr, &prom_out, &prom_source,
            addr, raw, PromSource::Sprite,
            sprite_owner, sprite_coverage, static_cast<std::uint16_t>(pos), forensic_palette_mode, &sprite_occupied);
        ++drawn;
    }
    return drawn;
}

std::uint8_t Video::road_prom_address(
    std::uint16_t ctl,
    std::uint8_t pen,
    bool road_b,
    std::uint8_t input_mask) {

    // v0.17 candidate wiring. Every candidate input occupies a named address
    // bit so the operator can suppress bits 0..7 individually with keys 1..8.
    // This is intentionally a reconstruction aid, not a claim of final PCB wiring.
    std::uint8_t addr = 0;
    addr |= static_cast<std::uint8_t>((pen & 0x01) << 0);       // pixel pen bit 0
    addr |= static_cast<std::uint8_t>(((pen >> 1) & 1) << 1);  // pixel pen bit 1
    addr |= static_cast<std::uint8_t>((road_b ? 1 : 0) << 2);  // road A/B source
    addr |= static_cast<std::uint8_t>(((ctl >> 0) & 1) << 3);  // control bit 0
    addr |= static_cast<std::uint8_t>(((ctl >> 4) & 1) << 4);  // control bit 4
    addr |= static_cast<std::uint8_t>(((ctl >> 5) & 1) << 5);  // control bit 5
    addr |= static_cast<std::uint8_t>(((ctl >> 8) & 1) << 6);  // control bit 8
    addr |= static_cast<std::uint8_t>(((ctl >> 9) & 1) << 7);  // control bit 9
    return static_cast<std::uint8_t>(addr & input_mask);
}

std::uint8_t Video::sprite_prom_address(
    const SpriteInstance& sprite,
    std::uint16_t raw_attr,
    std::uint8_t input_mask) {

    // Candidate sprite-side wiring is likewise decomposed into individual
    // address bits so the histogram/toggle report can show which signals vary.
    std::uint8_t addr = 0;
    addr |= static_cast<std::uint8_t>((raw_attr >> 9) & 0x03);        // A0-A1 attr
    addr |= static_cast<std::uint8_t>((sprite.color & 0x07) << 2);   // A2-A4 colour
    addr |= static_cast<std::uint8_t>(((sprite.color >> 3) & 1) << 5);
    addr |= static_cast<std::uint8_t>(((sprite.color >> 4) & 1) << 6);
    addr |= static_cast<std::uint8_t>((sprite.priority ? 1 : 0) << 7);
    return static_cast<std::uint8_t>(addr & input_mask);
}

void Video::note_prom(PromSource source, std::uint8_t address, const PromEntry& entry) {
    std::array<bool, 256>* seen = nullptr;
    std::array<std::uint64_t, 256>* hist = nullptr;
    std::array<std::uint64_t, 8>* highs = nullptr;
    std::array<std::uint64_t, 8>* toggles = nullptr;
    std::size_t source_index = 0;
    const char* name = "?";
    switch (source) {
    case PromSource::RoadA:
        seen = &seen_road_a_; hist = &road_a_hist_; highs = &road_a_bit_high_; toggles = &road_a_bit_toggle_; source_index = 0; name = "roadA"; break;
    case PromSource::RoadB:
        seen = &seen_road_b_; hist = &road_b_hist_; highs = &road_b_bit_high_; toggles = &road_b_bit_toggle_; source_index = 1; name = "roadB"; break;
    case PromSource::Sprite:
        seen = &seen_mix_; hist = &mix_hist_; highs = &mix_bit_high_; toggles = &mix_bit_toggle_; source_index = 2; name = "sprite"; break;
    default: return;
    }

    ++(*hist)[address];
    for (int bit = 0; bit < 8; ++bit)
        if (address & (1u << bit)) ++(*highs)[bit];

    if (have_last_prom_addr_[source_index]) {
        const std::uint8_t changed = static_cast<std::uint8_t>(last_prom_addr_[source_index] ^ address);
        for (int bit = 0; bit < 8; ++bit)
            if (changed & (1u << bit)) ++(*toggles)[bit];
    }
    last_prom_addr_[source_index] = address;
    have_last_prom_addr_[source_index] = true;

    if ((*seen)[address]) return;
    (*seen)[address] = true;

    if (prom_log_) {
        prom_log_ << name
                  << " 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned>(address)
                  << " 0x" << std::setw(2) << static_cast<unsigned>(entry.raw)
                  << " 0x" << std::setw(2) << static_cast<unsigned>(entry.output_class)
                  << " 0x" << std::setw(2) << static_cast<unsigned>(entry.block_mask)
                  << std::dec << '\n';
    }
}

void Video::visualise_layer_mask(
    std::vector<std::uint32_t>& pixels,
    const std::vector<std::uint8_t>& layer_mask) {

    // Additive channel encoding makes combinations visible rather than hiding
    // them behind one scalar priority colour.
    for (std::size_t i = 0; i < pixels.size() && i < layer_mask.size(); ++i) {
        const std::uint8_t m = layer_mask[i];
        unsigned r = 12, g = 12, b = 12;
        if (m & LAYER_BG_BOTTOM) { b += 70; }
        if (m & LAYER_ROAD_A)    { g += 95; }
        if (m & LAYER_ROAD_B)    { r += 95; g += 65; }
        if (m & LAYER_BG_UPPER)  { b += 80; r += 35; }
        if (m & LAYER_SPRITE)    { r += 120; }
        if (m & LAYER_TEXT)      { r += 90; g += 90; b += 90; }
        r = std::min(r, 255u); g = std::min(g, 255u); b = std::min(b, 255u);
        pixels[i] = 0xff000000u | (r << 16) | (g << 8) | b;
    }
}

void Video::visualise_prom_bits(
    std::vector<std::uint32_t>& pixels,
    const std::vector<std::uint8_t>& prom_addr,
    const std::vector<std::uint8_t>& prom_source,
    std::uint8_t enabled_mask) {

    // Compact address-bit view: RGB encodes groups A0-A2, A3-A5, A6-A7.
    // Disabled candidate bits are removed before display, matching the lookup.
    for (std::size_t i = 0; i < pixels.size(); ++i) {
        const std::uint8_t a = static_cast<std::uint8_t>(prom_addr[i] & enabled_mask);
        if (prom_source[i] == 0) { pixels[i] = 0xff050505u; continue; }
        const std::uint8_t r = static_cast<std::uint8_t>(((a >> 0) & 0x07) * 36);
        const std::uint8_t g = static_cast<std::uint8_t>(((a >> 3) & 0x07) * 36);
        const std::uint8_t b = static_cast<std::uint8_t>(((a >> 6) & 0x03) * 85);
        pixels[i] = 0xff000000u | (static_cast<std::uint32_t>(r) << 16) |
                    (static_cast<std::uint32_t>(g) << 8) | b;
    }
}

void Video::visualise_prom_trace(
    std::vector<std::uint32_t>& pixels,
    const std::vector<std::uint8_t>& prom_addr,
    const std::vector<std::uint8_t>& prom_out,
    const std::vector<std::uint8_t>& prom_source) {

    // PROM trace view: R = exact address, G = raw output nibble expanded,
    // B = source (Road A / Road B / Sprite). Exact tuples are also written to
    // logs/prom_mixer.log, so the visual regions can be correlated with bytes.
    for (std::size_t i = 0; i < pixels.size(); ++i) {
        const auto src = static_cast<PromSource>(prom_source[i]);
        if (src == PromSource::None) {
            pixels[i] = 0xff080808u;
            continue;
        }
        const std::uint8_t r = prom_addr[i];
        const std::uint8_t g = static_cast<std::uint8_t>((prom_out[i] & 0x0f) * 17);
        std::uint8_t b = 0;
        if (src == PromSource::RoadA) b = 70;
        else if (src == PromSource::RoadB) b = 150;
        else if (src == PromSource::Sprite) b = 240;
        pixels[i] = 0xff000000u |
            (static_cast<std::uint32_t>(r) << 16) |
            (static_cast<std::uint32_t>(g) << 8) |
            b;
    }
}


void Video::visualise_road_probe(
    std::vector<std::uint32_t>& pixels,
    const std::vector<std::uint8_t>& road_probe) {

    // v0.18 road-signal view. For each generated road pixel:
    // R = pen/source group (bits 0..2), G = edge/right/control group (3..5),
    // B = ctl7/shape15 group (6..7). Non-road pixels remain near black.
    for (std::size_t i = 0; i < pixels.size() && i < road_probe.size(); ++i) {
        const std::uint8_t v = road_probe[i];
        if (v == 0) {
            pixels[i] = 0xff050505u;
            continue;
        }
        const std::uint8_t r = static_cast<std::uint8_t>((v & 0x07) * 36);
        const std::uint8_t g = static_cast<std::uint8_t>(((v >> 3) & 0x07) * 36);
        const std::uint8_t b = static_cast<std::uint8_t>(((v >> 6) & 0x03) * 85);
        pixels[i] = 0xff000000u |
            (static_cast<std::uint32_t>(r) << 16) |
            (static_cast<std::uint32_t>(g) << 8) | b;
    }
}


void Video::sample_road_ram(const Bus& bus, unsigned frame) {
    (void)frame;
    const Region* rr = find_region(bus, "road");
    if (!rr || rr->bytes.size() < 0x2000)
        return;

    ++road_snapshots_;
    const auto& write_counts = bus.road_word_writes();
    const auto& write_changes = bus.road_word_value_changes();
    const auto& nonzero_writes = bus.road_word_nonzero_writes();
    const auto& write_min = bus.road_word_min_values();
    const auto& write_max = bus.road_word_max_values();
    const auto& write_last = bus.road_word_last_values();
    for (std::size_t w = 0; w < ROAD_RAM_WORDS; ++w) {
        road_last_write_counts_[w] = write_counts[w];
        road_write_value_changes_[w] = write_changes[w];
        road_nonzero_write_events_[w] = nonzero_writes[w];
        road_write_min_be_[w] = write_min[w];
        road_write_max_be_[w] = write_max[w];
        road_write_last_be_[w] = write_last[w];
        const std::size_t off = w * 2;
        const std::uint16_t be = static_cast<std::uint16_t>((rr->bytes[off] << 8) | rr->bytes[off + 1]);
        const std::uint16_t le = static_cast<std::uint16_t>(rr->bytes[off] | (rr->bytes[off + 1] << 8));
        road_min_be_[w] = std::min(road_min_be_[w], be);
        road_max_be_[w] = std::max(road_max_be_[w], be);
        road_min_le_[w] = std::min(road_min_le_[w], le);
        road_max_le_[w] = std::max(road_max_le_[w], le);
        if (be != 0) ++road_nonzero_frames_[w];
        if (road_have_snapshot_ && road_prev_be_[w] != be)
            ++road_frame_changes_[w];
        road_prev_be_[w] = be;
        road_last_bytes_[off] = rr->bytes[off];
        road_last_bytes_[off + 1] = rr->bytes[off + 1];
    }
    road_have_snapshot_ = true;
}

void Video::visualise_road_ram_map(
    std::vector<std::uint32_t>& pixels,
    const Bus& bus) const {

    pixels.assign(WIDTH * HEIGHT, 0xff050505u);
    const auto& writes = bus.road_word_writes();

    std::uint64_t max_writes = 1;
    std::uint64_t max_changes = 1;
    for (std::size_t i = 0; i < ROAD_RAM_WORDS; ++i) {
        max_writes = std::max(max_writes, writes[i]);
        max_changes = std::max(max_changes, road_write_value_changes_[i]);
    }

    auto scale_log = [](std::uint64_t v, std::uint64_t vmax) -> std::uint8_t {
        if (!v) return 0;
        const double a = std::log1p(static_cast<double>(v));
        const double b = std::log1p(static_cast<double>(vmax));
        return static_cast<std::uint8_t>(std::clamp<int>(static_cast<int>(255.0 * a / b), 0, 255));
    };

    // 4096 words -> 64x64 logical cells. Red = CPU-B write activity,
    // green = write-time value changes, blue = currently non-zero.
    for (std::size_t w = 0; w < ROAD_RAM_WORDS; ++w) {
        const int cx = static_cast<int>(w & 63);
        const int cy = static_cast<int>(w >> 6);
        const int x0 = cx * WIDTH / 64;
        const int x1 = (cx + 1) * WIDTH / 64;
        const int y0 = cy * HEIGHT / 64;
        const int y1 = (cy + 1) * HEIGHT / 64;
        const std::uint8_t r = scale_log(writes[w], max_writes);
        const std::uint8_t g = scale_log(road_write_value_changes_[w], max_changes);
        const std::uint8_t b = road_prev_be_[w] ? 180 : 0;
        const std::uint32_t c = 0xff000000u |
            (static_cast<std::uint32_t>(r) << 16) |
            (static_cast<std::uint32_t>(g) << 8) | b;
        for (int y = y0; y < y1; ++y)
            for (int x = x0; x < x1; ++x)
                pixels[static_cast<std::size_t>(y) * WIDTH + x] = c;
    }

    // Candidate 0x200-byte banks are 256 words wide. Draw bank boundaries so
    // repeating line-oriented structures are obvious in screenshots.
    for (int bank = 1; bank < 16; ++bank) {
        const std::size_t word = static_cast<std::size_t>(bank) * 0x100;
        const int cy = static_cast<int>(word >> 6);
        const int y = cy * HEIGHT / 64;
        if (y >= 0 && y < HEIGHT)
            for (int x = 0; x < WIDTH; ++x)
                pixels[static_cast<std::size_t>(y) * WIDTH + x] = 0xffffffffu;
    }
}

void Video::update_runtime_title(const Runtime& runtime, const SceneState& scene, unsigned frame, unsigned sprites, bool paused) {
    std::ostringstream ss;
    ss << kWindowProduct << " v" << kNativeVersion << " FORENSIC PIXEL PROVENANCE"
       << " | frame " << frame
       << (paused ? " | PAUSED" : " | RUNNING")
       << " | CPU A " << runtime.instructions_a
       << " | CPU B " << runtime.instructions_b
       << " | sprites " << sprites
       << " | mixer " << (scene.use_reference_mixer ? "REFERENCE" : (scene.use_prom_mixer ? "PROM-MASK" : "LEGACY"))
       << " | mask 0x" << std::hex << static_cast<unsigned>(scene.prom_input_mask) << std::dec
       << " | view " << (scene.show_road_ram_map ? "RAM" : (scene.show_road_probe ? "ROAD" : (scene.show_prom_bits ? "BITS" : (scene.show_prom_trace ? "PROM" : (scene.show_priority ? "LAYERS" : "RGB")))))
       << " | line " << scene.road_ram_scanline;

    if (const Region* rr = find_region(runtime.bus, "road")) {
        // Show one word from each of the sixteen 0x200-byte candidate banks for
        // the selected 0..255 line. This is intentionally raw: v0.19 is about
        // locating the real TC0150ROD line structure before decoding fields.
        ss << " | words";
        const std::size_t yoff = static_cast<std::size_t>(scene.road_ram_scanline & 0xff) * 2;
        for (int bank = 0; bank < 16; ++bank) {
            const std::size_t off = static_cast<std::size_t>(bank) * 0x200 + yoff;
            if (off + 1 >= rr->bytes.size()) break;
            const auto v = be16(rr->bytes, off);
            if (v != 0)
                ss << ' ' << std::hex << bank << ':' << std::setw(4) << std::setfill('0') << v << std::dec;
        }
    }
    ss << " | ctrl 0x" << std::hex << static_cast<unsigned>(runtime.bus.cpu_control());
    SDL_SetWindowTitle(window_, ss.str().c_str());
}


void Video::export_runtime_diagnostics(
    const Machine& machine, const Runtime& runtime, const SceneState& scene, unsigned frame,
    const std::vector<std::uint32_t>& final_pixels, const std::vector<std::uint8_t>& layer_mask,
    const std::vector<std::uint8_t>& prom_addr, const std::vector<std::uint8_t>& prom_out,
    const std::vector<std::uint8_t>& prom_source, const std::vector<std::uint8_t>& road_probe,
    const std::vector<std::uint16_t>& sprite_owner, const std::vector<std::uint16_t>& sprite_coverage) {

    const auto root = log_directory_ / ("graphics_frame_" + std::to_string(frame));
    std::error_code ec; std::filesystem::create_directories(root, ec);
    diag_save_png(root / "00_final_selected.png", WIDTH, HEIGHT, final_pixels);

    auto render = [&](bool bottom_on, bool road_on, bool upper_on, bool sprites_on, bool text_on, bool prom,
                      std::vector<std::uint16_t>* owners=nullptr, std::vector<std::uint16_t>* coverage=nullptr) {
        std::vector<std::uint32_t> px(WIDTH*HEIGHT, 0xff080808u);
        std::vector<std::uint8_t> lm(WIDTH*HEIGHT,0), pa(WIDTH*HEIGHT,0), po(WIDTH*HEIGHT,0), ps(WIDTH*HEIGHT,0), rp(WIDTH*HEIGHT,0);
        const Region* cr=find_region(runtime.bus,"tile_control");
        const std::uint16_t ctrl6=(cr&&cr->bytes.size()>=14)?be16(cr->bytes,12):0;
        const int bottom=(ctrl6&0x08)?1:0, upper=bottom^1;
        if(bottom_on) draw_runtime_tile_layer(px,lm,machine,runtime.bus,bottom,LAYER_BG_BOTTOM,true);
        if(road_on) draw_runtime_road(px,lm,pa,po,ps,rp,machine,runtime.bus,prom,scene.prom_input_mask,false);
        if(upper_on) draw_runtime_tile_layer(px,lm,machine,runtime.bus,upper,LAYER_BG_UPPER,false);
        if(sprites_on) draw_runtime_sprites(px,lm,pa,po,ps,machine,runtime.bus,prom,scene.prom_input_mask,false,owners,coverage,sprite_tie_break_higher_);
        if(text_on) draw_runtime_text_layer(px,lm,runtime.bus,LAYER_TEXT);
        return px;
    };


    auto render_reference = [&](bool bottom_on, bool road_on, bool upper_on, bool sprites_on, bool text_on,
                                std::vector<std::uint16_t>* owners=nullptr, std::vector<std::uint16_t>* coverage=nullptr,
                                std::vector<std::uint8_t>* out_priority=nullptr) {
        std::vector<std::uint32_t> px(WIDTH*HEIGHT, 0xff080808u);
        std::vector<std::uint8_t> pri(WIDTH*HEIGHT,0), pa(WIDTH*HEIGHT,0), po(WIDTH*HEIGHT,0), ps(WIDTH*HEIGHT,0), rp(WIDTH*HEIGHT,0);
        const Region* cr=find_region(runtime.bus,"tile_control");
        const std::uint16_t ctrl6=(cr&&cr->bytes.size()>=14)?be16(cr->bytes,12):0;
        const int bottom=(ctrl6&0x08)?1:0, upper=bottom^1;
        if(bottom_on) draw_runtime_tile_layer(px,pri,machine,runtime.bus,bottom,0x00,true);
        if(upper_on) draw_runtime_tile_layer(px,pri,machine,runtime.bus,upper,0x01,false);
        if(road_on) draw_runtime_road(px,pri,pa,po,ps,rp,machine,runtime.bus,false,scene.prom_input_mask,true);
        if(text_on) draw_runtime_text_layer(px,pri,runtime.bus,0x04);
        if(sprites_on) draw_runtime_sprites(px,pri,pa,po,ps,machine,runtime.bus,false,scene.prom_input_mask,true,owners,coverage,sprite_tie_break_higher_);
        if(out_priority) *out_priority=pri;
        return px;
    };

    // Individual hardware sources, independent of the final compositor.
    diag_save_png(root/"10_bg_bottom_only.png",WIDTH,HEIGHT,render(true,false,false,false,false,true));
    diag_save_png(root/"11_road_only.png",WIDTH,HEIGHT,render(false,true,false,false,false,true));
    diag_save_png(root/"12_bg_upper_only.png",WIDTH,HEIGHT,render(false,false,true,false,false,true));
    diag_save_png(root/"13_sprites_only.png",WIDTH,HEIGHT,render(false,false,false,true,false,true));
    diag_save_png(root/"14_text_only.png",WIDTH,HEIGHT,render(false,false,false,false,true,true));

    // Incremental composition stages make the first bad stage visually obvious.
    diag_save_png(root/"20_stage_bottom.png",WIDTH,HEIGHT,render(true,false,false,false,false,true));
    diag_save_png(root/"21_stage_bottom_road.png",WIDTH,HEIGHT,render(true,true,false,false,false,true));
    diag_save_png(root/"22_stage_bottom_road_upper.png",WIDTH,HEIGHT,render(true,true,true,false,false,true));
    diag_save_png(root/"23_stage_before_text.png",WIDTH,HEIGHT,render(true,true,true,true,false,true));
    const auto legacy=render(true,true,true,true,true,false);
    diag_save_png(root/"24_composite_legacy_no_prom.png",WIDTH,HEIGHT,legacy);

    std::vector<std::uint8_t> reference_priority;
    std::vector<std::uint16_t> ref_owner(WIDTH*HEIGHT,0xffff), ref_coverage(WIDTH*HEIGHT,0);
    const auto reference = render_reference(true,true,true,true,true,&ref_owner,&ref_coverage,&reference_priority);
    const auto prom_final = render(true,true,true,true,true,true);
    diag_save_png(root/"00_reference_final.png",WIDTH,HEIGHT,reference);
    diag_save_png(root/"01_prom_final.png",WIDTH,HEIGHT,prom_final);
    diag_save_png(root/"02_legacy_final.png",WIDTH,HEIGHT,legacy);
    std::vector<std::uint32_t> refpromdiff(WIDTH*HEIGHT,0xff000000u);std::uint64_t ref_prom_diff_count=0;
    for(std::size_t i=0;i<refpromdiff.size();++i){if(reference[i]!=prom_final[i]){++ref_prom_diff_count;auto a=reference[i],b=prom_final[i];unsigned d=((std::abs((int)((a>>16)&255)-(int)((b>>16)&255))+std::abs((int)((a>>8)&255)-(int)((b>>8)&255))+std::abs((int)(a&255)-(int)(b&255)))/3);d=std::max(48u,std::min(255u,d));refpromdiff[i]=0xff000000u|(d<<16)|(d/3<<8);}}
    diag_save_png(root/"03_reference_vs_prom_difference.png",WIDTH,HEIGHT,refpromdiff);
    auto refpri=reference; visualise_layer_mask(refpri,reference_priority); diag_save_png(root/"04_reference_priority_bitmap.png",WIDTH,HEIGHT,refpri);

    // Visualise the raw priority/PROM evidence captured by the live path.
    auto lmvis=final_pixels; visualise_layer_mask(lmvis,layer_mask); diag_save_png(root/"30_layer_mask.png",WIDTH,HEIGHT,lmvis);
    auto pbits=final_pixels; visualise_prom_bits(pbits,prom_addr,prom_source,scene.prom_input_mask); diag_save_png(root/"31_prom_address_bits.png",WIDTH,HEIGHT,pbits);
    auto ptrace=final_pixels; visualise_prom_trace(ptrace,prom_addr,prom_out,prom_source); diag_save_png(root/"32_prom_address_output.png",WIDTH,HEIGHT,ptrace);
    auto rprobe=final_pixels; visualise_road_probe(rprobe,road_probe); diag_save_png(root/"33_road_signal_probe.png",WIDTH,HEIGHT,rprobe);

    // Pixel-difference view between the PROM compositor and legacy mask path. This is independent of the live selected mixer.
    std::vector<std::uint32_t> diff(WIDTH*HEIGHT,0xff000000u); std::uint64_t diff_count=0;
    for(std::size_t i=0;i<diff.size();++i){if(prom_final[i]!=legacy[i]){++diff_count;auto a=prom_final[i],b=legacy[i];unsigned d=((std::abs((int)((a>>16)&255)-(int)((b>>16)&255))+std::abs((int)((a>>8)&255)-(int)((b>>8)&255))+std::abs((int)(a&255)-(int)(b&255)))/3);d=std::max(48u,std::min(255u,d));diff[i]=0xff000000u|(d<<16)|(d/3<<8);} }
    diag_save_png(root/"34_prom_vs_legacy_difference.png",WIDTH,HEIGHT,diff);

    // Sprite owner map: each accepted sprite pixel records the logical RAM slot.
    std::vector<std::uint32_t> ownimg(WIDTH*HEIGHT,0xff000000u),covimg(WIDTH*HEIGHT,0xff000000u);
    std::array<std::uint64_t,256> visible{};
    std::uint16_t maxcov=1;
    for(std::size_t i=0;i<sprite_owner.size();++i){
        const auto id=sprite_owner[i]; if(id!=0xffff && id<256){++visible[id];unsigned r=(id*73+61)%224+31,g=(id*151+29)%224+31,b=(id*199+17)%224+31;ownimg[i]=0xff000000u|(r<<16)|(g<<8)|b;}
        maxcov=std::max(maxcov,sprite_coverage[i]);
    }
    for(std::size_t i=0;i<sprite_coverage.size();++i){auto c=sprite_coverage[i]; if(!c)continue;unsigned v=std::min(255u,(unsigned)c*255u/std::max(1u,(unsigned)maxcov));covimg[i]=0xff000000u|(v<<16)|(v<<8)|v;}
    diag_save_png(root/"40_sprite_owner.png",WIDTH,HEIGHT,ownimg);
    diag_save_png(root/"41_sprite_overlap_count.png",WIDTH,HEIGHT,covimg);

    // Sprite placement overlay + detailed table. Rectangles are the renderer's exact logical bounds.
    auto overlay=final_pixels;
    std::ofstream scsv(root/"sprite_screen_placement.csv");
    scsv<<"slot,w0,w1,w2,w3,x,y,base_y,zoom_x,zoom_y,right,bottom,format,map_base,palette,priority,flip_x,flip_y,prom_addr,prom_raw,block_mask,candidate_pixels,accepted_vs_layers,blocked_by_layers,final_owner_pixels,lost_to_sprite_overlap\n";
    const Region* sr=find_region(runtime.bus,"sprites");
    // Build the exact pre-sprite layer mask once so every sprite can be tested independently against road/BG priority.
    std::vector<std::uint32_t> prepx(WIDTH*HEIGHT,0xff080808u);std::vector<std::uint8_t> prelm(WIDTH*HEIGHT,0),prepa(WIDTH*HEIGHT,0),prepo(WIDTH*HEIGHT,0),preps(WIDTH*HEIGHT,0),prerp(WIDTH*HEIGHT,0);
    const Region* cr2=find_region(runtime.bus,"tile_control");const std::uint16_t c6=(cr2&&cr2->bytes.size()>=14)?be16(cr2->bytes,12):0;const int bot2=(c6&0x08)?1:0,up2=bot2^1;draw_runtime_tile_layer(prepx,prelm,machine,runtime.bus,bot2,LAYER_BG_BOTTOM,true);draw_runtime_road(prepx,prelm,prepa,prepo,preps,prerp,machine,runtime.bus,true,scene.prom_input_mask,false);draw_runtime_tile_layer(prepx,prelm,machine,runtime.bus,up2,LAYER_BG_UPPER,false);
    auto isol=root/"sprite_screen_isolated";std::filesystem::create_directories(isol,ec);
    struct DiagSpriteRec {
        unsigned slot=0; int x=0,y=0,zx=0,zy=0,palette=0,priority=0;
        std::uint64_t candidate=0,accepted=0,final_visible=0,blocked=0,lost_overlap=0;
        std::uint64_t dominant_pixels=0; unsigned unique_colors=0; double dominant_fraction=0.0;
        unsigned best_pair=0xffff; std::uint64_t best_pair_overlap=0; double best_pair_fraction=0.0;
        std::vector<std::uint32_t> pixels;
    };
    std::vector<DiagSpriteRec> diag_sprites;
    std::vector<std::uint16_t> accepted_density(WIDTH*HEIGHT,0), rejected_layer_density(WIDTH*HEIGHT,0);
    if(sr){for(std::size_t pos=0;pos*8+7<sr->bytes.size();++pos){const auto off=pos*8;auto w0=be16(sr->bytes,off),w1=be16(sr->bytes,off+2),w2=be16(sr->bytes,off+4),w3=be16(sr->bytes,off+6);auto tile=(std::uint16_t)(w3&0x7ff);if(!tile)continue;int zy=((w0>>9)&0x7f)+1,zx=(w1&0x7f)+1;auto fmt=Machine::format_for_zoomx(zx);if(fmt==SpriteFormat::Invalid)continue;int x=w2&0x1ff;if(x>0x140)x-=0x200;int y=sprite_visible_y(w0,VISIBLE_Y_START,layer_offset_sprites_.y);x+=layer_offset_sprites_.x;int by=y+(128-zy);std::size_t base=fmt==SpriteFormat::Obj128x128?((std::size_t)tile<<6):fmt==SpriteFormat::Obj64x128?(((std::size_t)tile<<5)+0x20000):(((std::size_t)tile<<4)+0x30000);SpriteInstance q{x,y,zx,zy,tile,(std::uint8_t)((w1&0x7f80)>>7),(w2&0x4000)!=0,(w2&0x8000)!=0,(w1&0x8000)?1:0};auto addr=sprite_prom_address(q,w2,scene.prom_input_mask);auto pe=mix_lut_[addr];
        std::vector<std::uint32_t> solopx(WIDTH*HEIGHT,0);std::vector<std::uint8_t> sololm(WIDTH*HEIGHT,0),d8(WIDTH*HEIGHT,0);std::vector<std::uint16_t> soloown(WIDTH*HEIGHT,0xffff),solocov(WIDTH*HEIGHT,0);draw_sprite(solopx,machine,q,&runtime.bus,&sololm,LAYER_SPRITE,0,&d8,&d8,&d8,addr,pe.raw,PromSource::Sprite,&soloown,&solocov,(std::uint16_t)pos);std::uint64_t candidate=0;for(auto v:soloown)if(v==(std::uint16_t)pos)++candidate;
        auto testpx=prepx;auto testlm=prelm;auto tpa=prepa,tpo=prepo,tps=preps;std::vector<std::uint16_t> testown(WIDTH*HEIGHT,0xffff),testcov(WIDTH*HEIGHT,0);draw_sprite(testpx,machine,q,&runtime.bus,&testlm,LAYER_SPRITE,pe.block_mask,&tpa,&tpo,&tps,addr,pe.raw,PromSource::Sprite,&testown,&testcov,(std::uint16_t)pos);std::uint64_t accepted=0;for(auto v:testown)if(v==(std::uint16_t)pos)++accepted;auto finalv=visible[pos];auto blocked=candidate>accepted?candidate-accepted:0;auto overlap=accepted>finalv?accepted-finalv:0;
        DiagSpriteRec dr; dr.slot=(unsigned)pos;dr.x=x;dr.y=by;dr.zx=zx;dr.zy=zy;dr.palette=q.color;dr.priority=q.priority;dr.candidate=candidate;dr.accepted=accepted;dr.final_visible=finalv;dr.blocked=blocked;dr.lost_overlap=overlap;
        std::array<std::uint64_t,256> coarse_col{}; std::uint64_t dom=0;
        for(std::size_t pi=0;pi<soloown.size();++pi){if(soloown[pi]!=(std::uint16_t)pos)continue;dr.pixels.push_back((std::uint32_t)pi);auto c=solopx[pi];auto key=(unsigned)((((c>>16)&0xe0))|(((c>>8)&0xe0)>>3)|((c&0xc0)>>6));auto n=++coarse_col[key];dom=std::max(dom,n);if(testown[pi]==(std::uint16_t)pos){if(accepted_density[pi]!=0xffff)++accepted_density[pi];}else{if(rejected_layer_density[pi]!=0xffff)++rejected_layer_density[pi];}}
        dr.dominant_pixels=dom;for(auto n:coarse_col)if(n)++dr.unique_colors;dr.dominant_fraction=candidate?double(dom)/double(candidate):0.0;diag_sprites.push_back(std::move(dr));
        std::ostringstream fn;fn<<"slot_"<<std::setfill('0')<<std::setw(3)<<pos<<".png";diag_save_png(isol/fn.str(),WIDTH,HEIGHT,solopx);
        scsv<<pos<<','<<std::hex<<w0<<','<<w1<<','<<w2<<','<<w3<<std::dec<<','<<x<<','<<y<<','<<by<<','<<zx<<','<<zy<<','<<(x+zx-1)<<','<<(by+zy-1)<<','<<Machine::format_name(fmt)<<','<<base<<','<<(unsigned)q.color<<','<<q.priority<<','<<q.flip_x<<','<<q.flip_y<<','<<(unsigned)addr<<','<<(unsigned)pe.raw<<','<<(unsigned)pe.block_mask<<','<<candidate<<','<<accepted<<','<<blocked<<','<<finalv<<','<<overlap<<"\n";
        unsigned col=0xff000000u|(((pos*73+61)%224+31)<<16)|(((pos*151+29)%224+31)<<8)|((pos*199+17)%224+31);int x0=std::max(0,x),x1=std::min(WIDTH-1,x+zx-1),y0=std::max(0,by),y1=std::min(HEIGHT-1,by+zy-1);if(x0<=x1&&y0<=y1){for(int xx=x0;xx<=x1;++xx){overlay[(std::size_t)y0*WIDTH+xx]=col;overlay[(std::size_t)y1*WIDTH+xx]=col;}for(int yy=y0;yy<=y1;++yy){overlay[(std::size_t)yy*WIDTH+x0]=col;overlay[(std::size_t)yy*WIDTH+x1]=col;}} }}
    diag_save_png(root/"42_sprite_bounding_boxes.png",WIDTH,HEIGHT,overlay);

    // v0.38: exact sprite-pair overlap analysis using the isolated pixel sets.
    std::ofstream paircsv(root/"sprite_overlap_pairs.csv");
    paircsv<<"slot_a,slot_b,pixels_a,pixels_b,overlap_pixels,overlap_pct_a,overlap_pct_b,same_bounds,same_palette,same_priority\n";
    struct PairRec{unsigned a=0,b=0;std::uint64_t n=0;double fa=0,fb=0;}; std::vector<PairRec> pairs;
    for(std::size_t a=0;a<diag_sprites.size();++a)for(std::size_t b=a+1;b<diag_sprites.size();++b){
        const auto&A=diag_sprites[a];const auto&B=diag_sprites[b];std::size_t ia=0,ib=0;std::uint64_t ov=0;
        while(ia<A.pixels.size()&&ib<B.pixels.size()){auto va=A.pixels[ia],vb=B.pixels[ib];if(va==vb){++ov;++ia;++ib;}else if(va<vb)++ia;else ++ib;}
        if(!ov)continue;double fa=A.candidate?100.0*double(ov)/double(A.candidate):0,fb=B.candidate?100.0*double(ov)/double(B.candidate):0;
        bool same_bounds=A.x==B.x&&A.y==B.y&&A.zx==B.zx&&A.zy==B.zy;
        paircsv<<A.slot<<','<<B.slot<<','<<A.candidate<<','<<B.candidate<<','<<ov<<','<<fa<<','<<fb<<','<<same_bounds<<','<<(A.palette==B.palette)<<','<<(A.priority==B.priority)<<"\n";
        pairs.push_back({A.slot,B.slot,ov,fa,fb});
        if(ov>A.best_pair_overlap){diag_sprites[a].best_pair=B.slot;diag_sprites[a].best_pair_overlap=ov;diag_sprites[a].best_pair_fraction=fa;}
        if(ov>B.best_pair_overlap){diag_sprites[b].best_pair=A.slot;diag_sprites[b].best_pair_overlap=ov;diag_sprites[b].best_pair_fraction=fb;}
    }
    std::sort(pairs.begin(),pairs.end(),[](const PairRec&a,const PairRec&b){return a.n>b.n;});

    // v0.43 final-pixel provenance. Re-run the reference-priority decision at
    // selected pixels and explain every sprite candidate from hardware sprite RAM
    // through spritemap/source pen/palette/priority to the final winner.
    if (forensics_.pixel_provenance) {
        std::vector<std::pair<int,int>> selected_pixels = forensics_.pixels;
        auto add_pixel = [&](int x, int y) {
            if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return;
            auto p = std::make_pair(x,y);
            if (std::find(selected_pixels.begin(), selected_pixels.end(), p) == selected_pixels.end())
                selected_pixels.push_back(p);
        };
        // Explicit user pixel/region selections take precedence over preset/auto pair selection.
        if (forensics_.explicit_location && forensics_.pixels.empty() && !forensics_.regions.empty()) {
            for (const auto& r : forensics_.regions) {
                for (int y=r.y1; y<=r.y2 && selected_pixels.size()<forensics_.pixel_provenance_max; ++y)
                    for (int x=r.x1; x<=r.x2 && selected_pixels.size()<forensics_.pixel_provenance_max; ++x) {
                        const auto i=(std::size_t)y*WIDTH+x;
                        if (ref_coverage[i] > 1) add_pixel(x,y);
                    }
            }
        }
        if (!forensics_.explicit_location && forensics_.pixel_provenance_pair) {
            const auto [sa,sb] = *forensics_.pixel_provenance_pair;
            auto ia = std::find_if(diag_sprites.begin(),diag_sprites.end(),[&](const DiagSpriteRec&d){return d.slot==sa;});
            auto ib = std::find_if(diag_sprites.begin(),diag_sprites.end(),[&](const DiagSpriteRec&d){return d.slot==sb;});
            if (ia != diag_sprites.end() && ib != diag_sprites.end()) {
                std::size_t a=0,b=0;
                while (a<ia->pixels.size() && b<ib->pixels.size() && selected_pixels.size()<forensics_.pixel_provenance_max) {
                    const auto va=ia->pixels[a], vb=ib->pixels[b];
                    if (va==vb) { add_pixel((int)(va%WIDTH),(int)(va/WIDTH)); ++a; ++b; }
                    else if (va<vb) ++a; else ++b;
                }
            }
        }
        if (selected_pixels.empty() && !forensics_.explicit_location) {
            for (int y=0;y<HEIGHT && selected_pixels.size()<forensics_.pixel_provenance_max;++y)
                for (int x=0;x<WIDTH && selected_pixels.size()<forensics_.pixel_provenance_max;++x) {
                    const auto i=(std::size_t)y*WIDTH+x;
                    if (ref_coverage[i]>1) add_pixel(x,y);
                }
        }
        // The max is an auto-sampling cap. Explicit coordinates are authoritative:
        // if the user supplied 40 diagnostic pixels, trace all 40 rather than silently
        // dropping the tail of the requested regression set.
        if (!forensics_.explicit_location && selected_pixels.size() > forensics_.pixel_provenance_max)
            selected_pixels.resize(forensics_.pixel_provenance_max);

        std::vector<std::uint8_t> base_priority, pri_upper, pri_road, pri_text;
        const auto base_no_sprites = render_reference(true,true,true,false,true,nullptr,nullptr,&base_priority);
        const auto layer_bottom = render_reference(true,false,false,false,false);
        const auto layer_upper  = render_reference(false,false,true,false,false,nullptr,nullptr,&pri_upper);
        const auto layer_road   = render_reference(false,true,false,false,false,nullptr,nullptr,&pri_road);
        const auto layer_text   = render_reference(false,false,false,false,true,nullptr,nullptr,&pri_text);
        std::ofstream pric(root/"priority_provenance.csv");
        pric << "frame,x,y,upper_priority,road_priority,text_priority,combined_priority,upper_argb,road_argb,text_argb,pre_sprite_argb,final_argb,final_owner\n";
        std::ofstream prit(root/"priority_provenance.txt");
        prit << "Chase H.Q. Native v"<<kNativeVersion<<" priority-bitmap provenance\n"
             << "Reference codes currently supplied to the priority bitmap: upper=1, roadA=1, roadB=2, text=4. Sprite primasks are 0xf0/0xfc.\n"
             << "This report exposes the exact contributors; it does not assume candidate wiring is hardware-correct.\n\n";

        struct Sample {
            bool hit=false; std::uint16_t map_code=0; std::uint8_t pen=0; int sx=0,sy=0;
            const char* gfx=""; SpriteInstance sprite{}; std::uint16_t w0=0,w1=0,w2=0,w3=0;
        };
        auto sample_slot = [&](std::size_t pos,int px,int py,Sample& o)->bool {
            if(!sr || pos*8+7>=sr->bytes.size()) return false;
            const std::size_t off=pos*8;
            o.w0=be16(sr->bytes,off);o.w1=be16(sr->bytes,off+2);o.w2=be16(sr->bytes,off+4);o.w3=be16(sr->bytes,off+6);
            int y=o.w0&0x1ff; const int zy=((o.w0>>9)&0x7f)+1; const int zx=(o.w1&0x7f)+1;
            const std::uint8_t color=(std::uint8_t)((o.w1&0x7f80)>>7); const bool priority=(o.w1&0x8000)!=0;
            int x=o.w2&0x1ff;if(x>0x140)x-=0x200;x+=layer_offset_sprites_.x;y=sprite_visible_y(o.w0,VISIBLE_Y_START,layer_offset_sprites_.y);
            const std::uint16_t tile=(std::uint16_t)(o.w3&0x07ff);if(!tile)return false;
            o.sprite={x,y,zx,zy,tile,color,(o.w2&0x4000)!=0,(o.w2&0x8000)!=0,priority?1:0};
            const auto fmt=Machine::format_for_zoomx(zx);int cols=0,rows=8;std::size_t mapoff=0;const std::vector<std::uint8_t>*gfx=nullptr;
            if(fmt==SpriteFormat::Obj128x128){cols=8;mapoff=(std::size_t)tile<<6;gfx=&machine.sprites_a();o.gfx="OBJ-A";}
            else if(fmt==SpriteFormat::Obj64x128){cols=4;mapoff=((std::size_t)tile<<5)+0x20000;gfx=&machine.sprites_b();o.gfx="OBJ-B";}
            else if(fmt==SpriteFormat::Obj32x128){cols=2;mapoff=((std::size_t)tile<<4)+0x30000;gfx=&machine.sprites_b();o.gfx="OBJ-B";} else return false;
            const int by=y+(128-zy);
            for(int j=0;j<rows;++j)for(int k=0;k<cols;++k){
                const int mx=o.sprite.flip_x?(cols-1-k):k,my=o.sprite.flip_y?(rows-1-j):j;
                const auto code=machine.spritemap_word(mapoff+(std::size_t)(mx+my*cols));
                const int x0=x+(k*zx)/cols,y0=by+(j*zy)/rows,x1=x+((k+1)*zx)/cols,y1=by+((j+1)*zy)/rows;
                if(px<x0||px>=x1||py<y0||py>=y1||x1<=x0||y1<=y0)continue;
                int sx=std::clamp(((px-x0)*16)/(x1-x0),0,15),sy=std::clamp(((py-y0)*16)/(y1-y0),0,15);
                if(o.sprite.flip_x)sx=15-sx;if(o.sprite.flip_y)sy=15-sy;
                const auto pen=sprite_pixel(*gfx,(std::size_t)(code&0x3fff),sx,sy);if(!pen)return false;
                o.hit=true;o.map_code=code;o.pen=pen;o.sx=sx;o.sy=sy;return true;
            }
            return false;
        };

        // v0.44 high-information pair selector. Once sprite sampling is available,
        // rank overlap pixels instead of blindly taking the first scanline. Zero-over-
        // nonzero palette cases and differing pens/palettes are intentionally prioritised.
        if (!forensics_.explicit_location && forensics_.pixel_provenance_pair) {
            const auto [sa,sb]=*forensics_.pixel_provenance_pair;
            auto ia=std::find_if(diag_sprites.begin(),diag_sprites.end(),[&](const DiagSpriteRec&d){return d.slot==sa;});
            auto ib=std::find_if(diag_sprites.begin(),diag_sprites.end(),[&](const DiagSpriteRec&d){return d.slot==sb;});
            struct RankedPixel{int score=0,x=0,y=0;std::uint16_t rawa=0,rawb=0;std::uint8_t pena=0,penb=0,pala=0,palb=0;};
            std::vector<RankedPixel> ranked;
            if(ia!=diag_sprites.end()&&ib!=diag_sprites.end()){
                std::size_t a=0,b=0;
                while(a<ia->pixels.size()&&b<ib->pixels.size()){
                    auto va=ia->pixels[a],vb=ib->pixels[b];
                    if(va==vb){int x=(int)(va%WIDTH),y=(int)(va/WIDTH);Sample A,B;if(sample_slot(sa,x,y,A)&&sample_slot(sb,x,y,B)){
                        auto pa=((A.sprite.color&255)<<4)|(A.pen&15), pb=((B.sprite.color&255)<<4)|(B.pen&15);
                        auto ra=runtime.bus.palette[pa&0xfff], rb=runtime.bus.palette[pb&0xfff]; int score=0;
                        if((ra==0)!=(rb==0))score+=100;if(ra!=rb)score+=30;if(A.pen!=B.pen)score+=15;if(A.sprite.color!=B.sprite.color)score+=10;
                        if(ref_owner[va]==sa||ref_owner[va]==sb)score+=5;
                        ranked.push_back({score,x,y,ra,rb,A.pen,B.pen,A.sprite.color,B.sprite.color});}
                        ++a;++b;} else if(va<vb)++a;else ++b;
                }
            }
            std::sort(ranked.begin(),ranked.end(),[](const RankedPixel&a,const RankedPixel&b){if(a.score!=b.score)return a.score>b.score;if(a.y!=b.y)return a.y<b.y;return a.x<b.x;});
            selected_pixels.clear(); for(const auto&r:ranked){add_pixel(r.x,r.y);if(selected_pixels.size()>=forensics_.pixel_provenance_max)break;}
            std::ofstream ip(root/"interesting_pixels.csv");ip<<"rank,score,x,y,slot_a,slot_b,pen_a,pen_b,palette_a,palette_b,raw_a,raw_b,final_owner,reason\n";
            unsigned rank=0;for(const auto&r:ranked){if(rank>=std::min<std::size_t>(100,ranked.size()))break;std::string reason;if((r.rawa==0)!=(r.rawb==0))reason="zero_vs_nonzero";else if(r.rawa!=r.rawb)reason="palette_value_difference";else if(r.pena!=r.penb)reason="pen_difference";else reason="overlap";auto i=(std::size_t)r.y*WIDTH+r.x;ip<<rank++<<','<<r.score<<','<<r.x<<','<<r.y<<','<<sa<<','<<sb<<','<<(unsigned)r.pena<<','<<(unsigned)r.penb<<','<<(unsigned)r.pala<<','<<(unsigned)r.palb<<",0x"<<std::hex<<r.rawa<<",0x"<<r.rawb<<std::dec<<','<<ref_owner[i]<<','<<reason<<"\n";}
        }

        std::ofstream csv(root/"pixel_provenance.csv");
        csv << "frame,x,y,draw_order,slot,w0,w1,w2,w3,gfx,map_code,source_x,source_y,pen,palette_bank,palette_entry,palette_raw,argb,priority,reference_priority,reference_block_mask,reference_blocked,reference_accepted,winner_after,prom_addr,prom_raw,prom_block_mask,pair_member,final_owner,final_argb\n";
        std::ofstream txt(root/"pixel_provenance.txt");
        txt << "Chase H.Q. Native v" << kNativeVersion << " final-pixel provenance\nframe="<<frame<<"\n";
        if(forensics_.pixel_provenance_pair) txt<<"focus_pair="<<forensics_.pixel_provenance_pair->first<<':'<<forensics_.pixel_provenance_pair->second<<"\n";
        txt << "Reference sprite rule: pen 0 transparent; sprite priority bit selects primask {0xf0,0xfc}; nonzero pens reserve sprite occupancy even when layer-masked; later entries cannot overwrite earlier occupancy.\n\n";
        for(const auto& [px,py]:selected_pixels){
            const auto idx=(std::size_t)py*WIDTH+px;
            txt << "PIXEL "<<px<<','<<py<<" final=0x"<<std::hex<<std::setw(8)<<std::setfill('0')<<reference[idx]<<std::dec
                <<" owner="<<ref_owner[idx]<<" coverage="<<ref_coverage[idx]<<" reference_priority=0x"<<std::hex<<(unsigned)reference_priority[idx]<<std::dec<<"\n";
            txt << "  layers: bottom=0x"<<std::hex<<layer_bottom[idx]<<" upper=0x"<<layer_upper[idx]<<" road=0x"<<layer_road[idx]<<" text=0x"<<layer_text[idx]<<" pre_sprite=0x"<<base_no_sprites[idx]<<std::dec<<"\n";
            pric<<frame<<','<<px<<','<<py<<','<<(unsigned)pri_upper[idx]<<','<<(unsigned)pri_road[idx]<<','<<(unsigned)pri_text[idx]<<','<<(unsigned)base_priority[idx]
                <<",0x"<<std::hex<<layer_upper[idx]<<",0x"<<layer_road[idx]<<",0x"<<layer_text[idx]<<",0x"<<base_no_sprites[idx]<<",0x"<<reference[idx]<<std::dec<<','<<ref_owner[idx]<<"\n";
            prit<<"PIXEL "<<px<<','<<py<<" contributors: upper=0x"<<std::hex<<(unsigned)pri_upper[idx]<<" road=0x"<<(unsigned)pri_road[idx]<<" text=0x"<<(unsigned)pri_text[idx]<<" combined=0x"<<(unsigned)base_priority[idx]<<std::dec
                <<" final_owner="<<ref_owner[idx]<<"\n";
            int winner=-1, order=0; unsigned candidates=0; std::uint8_t occupied=0;
            if(sr){for(std::size_t iter=0;iter<sr->bytes.size()/8;++iter){const auto pos=sprite_tie_break_higher_?iter:sr->bytes.size()/8-1-iter;Sample sm;if(!sample_slot(pos,px,py,sm))continue;++candidates;
                const auto pidx=(std::uint16_t)(((sm.sprite.color&0xff)<<4)|(sm.pen&15));const auto raw=runtime.bus.palette[pidx&0x0fff];const auto argb=runtime_palette_color(runtime.bus,sm.sprite.color,sm.pen);
                const std::uint8_t refmask=sm.sprite.priority?0xfc:0xf0;const bool accepted=accept_sprite_pixel(sm.pen,(base_priority[idx]&refmask)!=0,occupied,nullptr);const bool blocked=!accepted;if(accepted)winner=(int)pos;
                const auto pa=sprite_prom_address(sm.sprite,sm.w2,scene.prom_input_mask);const auto&pe=mix_lut_[pa];bool pair_member=forensics_.pixel_provenance_pair&&(pos==forensics_.pixel_provenance_pair->first||pos==forensics_.pixel_provenance_pair->second);
                csv<<frame<<','<<px<<','<<py<<','<<order++<<','<<pos<<",0x"<<std::hex<<sm.w0<<",0x"<<sm.w1<<",0x"<<sm.w2<<",0x"<<sm.w3<<','<<sm.gfx<<",0x"<<sm.map_code<<std::dec<<','<<sm.sx<<','<<sm.sy<<','<<(unsigned)sm.pen<<','<<(unsigned)sm.sprite.color<<','<<pidx<<",0x"<<std::hex<<raw<<",0x"<<argb<<std::dec<<','<<sm.sprite.priority<<','<<(unsigned)base_priority[idx]<<",0x"<<std::hex<<(unsigned)refmask<<std::dec<<','<<blocked<<','<<accepted<<','<<winner<<",0x"<<std::hex<<(unsigned)pa<<",0x"<<(unsigned)pe.raw<<",0x"<<(unsigned)pe.block_mask<<std::dec<<','<<pair_member<<','<<ref_owner[idx]<<",0x"<<std::hex<<reference[idx]<<std::dec<<"\n";
                txt<<"  sprite "<<pos<<(pair_member?" [PAIR]":"")<<" map=0x"<<std::hex<<sm.map_code<<std::dec<<" pen="<<(unsigned)sm.pen<<" pal="<<(unsigned)sm.sprite.color<<'/'<<(unsigned)sm.pen<<" raw=0x"<<std::hex<<raw<<" argb=0x"<<argb<<std::dec<<" pri="<<sm.sprite.priority<<" mask=0x"<<std::hex<<(unsigned)refmask<<" bitmap=0x"<<(unsigned)base_priority[idx]<<std::dec<<(blocked?" BLOCKED":" ACCEPTED")<<" winner_after="<<winner<<"\n";
                prit<<"  sprite "<<pos<<" pri="<<sm.sprite.priority<<" mask=0x"<<std::hex<<(unsigned)refmask<<" bitmap=0x"<<(unsigned)base_priority[idx]
                    <<" (bitmap & mask)=0x"<<(unsigned)(base_priority[idx]&refmask)<<std::dec<<" => "<<(blocked?"BLOCK":"ACCEPT")
                    <<"; PROM candidate=0x"<<std::hex<<(unsigned)pa<<" raw=0x"<<(unsigned)pe.raw<<std::dec
                    <<" bits[A7..A0]="<<((pa>>7)&1)<<((pa>>6)&1)<<((pa>>5)&1)<<((pa>>4)&1)<<((pa>>3)&1)<<((pa>>2)&1)<<((pa>>1)&1)<<(pa&1)<<"\n";
            }}
            txt<<"  RESULT reconstructed_winner="<<winner<<" recorded_owner="<<ref_owner[idx]<<" candidates="<<candidates<<(winner==(int)ref_owner[idx]?" MATCH":" MISMATCH")<<"\n\n";
        }
        std::ofstream sum(root/"pixel_provenance_summary.txt");
        sum<<"frame="<<frame<<"\npixels="<<selected_pixels.size()<<"\nexplicit_location="<<forensics_.explicit_location<<"\n";
        if(selected_pixels.empty() && forensics_.explicit_location) sum<<"selection_status=no overlapping/eligible pixels inside requested location\n";
        if(forensics_.pixel_provenance_pair)sum<<"pair="<<forensics_.pixel_provenance_pair->first<<':'<<forensics_.pixel_provenance_pair->second<<"\n";
        sum<<"outputs=pixel_provenance.csv,pixel_provenance.txt\n";
    }

    // Current-mixer acceptance/rejection density maps.
    auto density_image=[&](const std::vector<std::uint16_t>& v){std::vector<std::uint32_t> out(WIDTH*HEIGHT,0xff000000u);std::uint16_t mx=1;for(auto n:v)mx=std::max(mx,n);for(std::size_t i=0;i<v.size();++i)if(v[i]){unsigned q=std::max(40u,(unsigned)v[i]*255u/(unsigned)mx);out[i]=0xff000000u|(q<<16)|((q/2)<<8);}return out;};
    diag_save_png(root/"43_sprite_rejected_by_current_layer_mask.png",WIDTH,HEIGHT,density_image(rejected_layer_density));
    auto accimg=density_image(accepted_density);for(auto&c:accimg){auto r=(c>>16)&255;c=0xff000000u|((r/3)<<16)|(r<<8)|(r/3);}diag_save_png(root/"44_sprite_accepted_density.png",WIDTH,HEIGHT,accimg);

    // Palette forensics: dump every TC0110PCR entry and flag banks used by active sprites.
    std::array<bool,256> active_pal{};for(const auto&d:diag_sprites)active_pal[d.palette&255]=true;
    std::ofstream palcsv(root/"palette_table.csv");palcsv<<"bank,pen,index,raw15,r5,g5,b5,argb,active_sprite_bank,write_count,first_write_frame,last_write_frame,last_write_pc,first_written_value,last_written_value,was_ever_written\n";
    for(unsigned bank=0;bank<256;++bank)for(unsigned pen=0;pen<16;++pen){auto idx=(bank<<4)|pen;auto raw=runtime.bus.palette[idx];auto c=runtime_palette_color(runtime.bus,(std::uint16_t)bank,(std::uint8_t)pen);palcsv<<bank<<','<<pen<<','<<idx<<","<<raw<<','<<(raw&31)<<','<<((raw>>5)&31)<<','<<((raw>>10)&31)<<","<<std::hex<<std::setw(8)<<std::setfill('0')<<c<<std::dec<<','<<active_pal[bank]<<','<<runtime.bus.palette_write_count[idx]<<','<<runtime.bus.palette_first_write_frame[idx]<<','<<runtime.bus.palette_last_write_frame[idx]<<","<<std::hex<<runtime.bus.palette_last_write_pc[idx]<<std::dec<<','<<runtime.bus.palette_first_written_value[idx]<<','<<runtime.bus.palette_last_written_value[idx]<<','<<(runtime.bus.palette_write_count[idx]!=0)<<"\n";}
    std::ofstream activepal(root/"active_sprite_palettes.csv");activepal<<"slot,palette,priority,candidate_pixels,unique_coarse_colors,dominant_fraction\n";for(const auto&d:diag_sprites)activepal<<d.slot<<','<<d.palette<<','<<d.priority<<','<<d.candidate<<','<<d.unique_colors<<','<<d.dominant_fraction<<"\n";

    // PROM truth tables, effective raw bytes and per-frame usage. Preserve raw outputs separately from our candidate decoding.
    std::vector<std::uint8_t> mixraw(256),roadraw(256);std::ofstream truth(root/"prom_truth_tables.csv");truth<<"prom,address,raw,output_class,block_mask,A0,A1,A2,A3,A4,A5,A6,A7\n";
    for(unsigned a=0;a<256;++a){mixraw[a]=mix_lut_[a].raw;roadraw[a]=road_lut_[a].raw;for(int which=0;which<2;++which){const auto&e=which?road_lut_[a]:mix_lut_[a];truth<<(which?"road":"mix")<<','<<a<<','<<(unsigned)e.raw<<','<<(unsigned)e.output_class<<','<<(unsigned)e.block_mask;for(int bit=0;bit<8;++bit)truth<<','<<((a>>bit)&1);truth<<"\n";}}
    {std::ofstream f(root/"prom_mix_effective.bin",std::ios::binary);f.write((const char*)mixraw.data(),256);}{std::ofstream f(root/"prom_road_effective.bin",std::ios::binary);f.write((const char*)roadraw.data(),256);}
    std::ofstream usage(root/"prom_frame_usage.csv");usage<<"source,address,raw,pixels\n";std::array<std::array<std::uint64_t,256>,4> use{};for(std::size_t i=0;i<prom_addr.size();++i)if(prom_source[i]<4)++use[prom_source[i]][prom_addr[i]];for(unsigned src=1;src<4;++src)for(unsigned a=0;a<256;++a)if(use[src][a])usage<<src<<','<<a<<','<<(unsigned)((src==3)?mix_lut_[a].raw:road_lut_[a].raw)<<','<<use[src][a]<<"\n";
    std::ofstream bitcsv(root/"prom_bit_activity.csv");bitcsv<<"domain,bit,high_count,toggle_count\n";for(int b=0;b<8;++b){bitcsv<<"sprite,"<<b<<','<<mix_bit_high_[b]<<','<<mix_bit_toggle_[b]<<"\n";bitcsv<<"roadA,"<<b<<','<<road_a_bit_high_[b]<<','<<road_a_bit_toggle_[b]<<"\n";bitcsv<<"roadB,"<<b<<','<<road_b_bit_high_[b]<<','<<road_b_bit_toggle_[b]<<"\n";}

    // Hardware-visible register/state snapshot for correlation with the images.
    std::ofstream regs(root/"video_register_snapshot.txt");regs<<"frame="<<frame<<"\ncpu_control=0x"<<std::hex<<(unsigned)runtime.bus.cpu_control()<<std::dec<<"\n";
    auto dump_region_words=[&](const char* name,std::size_t maxbytes){if(const Region*r=find_region(runtime.bus,name)){regs<<"["<<name<<"] bytes="<<r->bytes.size()<<"\n";for(std::size_t o=0;o+1<std::min(maxbytes,r->bytes.size());o+=2)regs<<"+0x"<<std::hex<<std::setw(4)<<std::setfill('0')<<o<<" = 0x"<<std::setw(4)<<be16(r->bytes,o)<<std::dec<<"\n";}};
    dump_region_words("tile_control",0x40);dump_region_words("road",0x40);dump_region_words("sprites",0x40);dump_region_words("shared",0x80);
    if(const Region*r=find_region(runtime.bus,"road")){if(r->bytes.size()>0x1fff)regs<<"road_801ffe=0x"<<std::hex<<be16(r->bytes,0x1ffe)<<std::dec<<"\n";}
    regs<<"prom_mix_crc32=0x"<<std::hex<<diag_crc32(mixraw.data(),mixraw.size())<<"\nprom_road_crc32=0x"<<diag_crc32(roadraw.data(),roadraw.size())<<std::dec<<"\n";

    // Mask/control-candidate scoring. This is deliberately heuristic and never changes emulation behaviour.
    std::ofstream maskcsv(root/"sprite_mask_candidates.csv");maskcsv<<"slot,candidate_pixels,accepted,final_visible,blocked,lost_overlap,palette,priority,unique_colors,dominant_fraction,best_pair,best_pair_overlap,best_pair_fraction,mask_candidate_score\n";
    struct MaskRank{unsigned slot;double score;};std::vector<MaskRank> ranks;
    for(const auto&d:diag_sprites){double overlap_ratio=d.candidate?double(d.lost_overlap)/double(d.candidate):0.0;double uniform=std::max(0.0,(d.dominant_fraction-0.35)/0.65);double pair=std::min(1.0,d.best_pair_fraction/100.0);double score=0.45*pair+0.30*overlap_ratio+0.25*uniform;maskcsv<<d.slot<<','<<d.candidate<<','<<d.accepted<<','<<d.final_visible<<','<<d.blocked<<','<<d.lost_overlap<<','<<d.palette<<','<<d.priority<<','<<d.unique_colors<<','<<d.dominant_fraction<<','<<d.best_pair<<','<<d.best_pair_overlap<<','<<d.best_pair_fraction<<','<<score<<"\n";ranks.push_back({d.slot,score});}
    std::sort(ranks.begin(),ranks.end(),[](const MaskRank&a,const MaskRank&b){return a.score>b.score;});
    std::vector<std::uint32_t> maskowner(WIDTH*HEIGHT,0xff000000u);for(std::size_t ri=0;ri<std::min<std::size_t>(8,ranks.size());++ri){auto slot=ranks[ri].slot;auto it=std::find_if(diag_sprites.begin(),diag_sprites.end(),[&](const DiagSpriteRec&d){return d.slot==slot;});if(it==diag_sprites.end())continue;unsigned intensity=255u-(unsigned)ri*22u;for(auto px:it->pixels)maskowner[px]=0xff000000u|(intensity<<16)|((40u+ri*20u)<<8)|40u;}diag_save_png(root/"45_mask_candidate_owner.png",WIDTH,HEIGHT,maskowner);

    // Controlled experiments: suppress or force the priority bit of the highest-ranked candidates without changing normal execution.
    auto render_variant=[&](int skip_slot,int force_slot,int force_priority){
        std::vector<std::uint8_t> occupied(WIDTH*HEIGHT,0);
        std::vector<std::uint32_t> px(WIDTH*HEIGHT,0xff080808u);std::vector<std::uint8_t> lm(WIDTH*HEIGHT,0),pa(WIDTH*HEIGHT,0),po(WIDTH*HEIGHT,0),ps(WIDTH*HEIGHT,0),rp(WIDTH*HEIGHT,0);
        const Region*cr=find_region(runtime.bus,"tile_control");const std::uint16_t ctrl=(cr&&cr->bytes.size()>=14)?be16(cr->bytes,12):0;const int bot=(ctrl&0x08)?1:0,up=bot^1;draw_runtime_tile_layer(px,lm,machine,runtime.bus,bot,LAYER_BG_BOTTOM,true);draw_runtime_road(px,lm,pa,po,ps,rp,machine,runtime.bus,true,scene.prom_input_mask,false);draw_runtime_tile_layer(px,lm,machine,runtime.bus,up,LAYER_BG_UPPER,false);
        if(sr){for(std::size_t pos=sr->bytes.size()/8;pos-->0;){if((int)pos==skip_slot)continue;auto off=pos*8;auto w0=be16(sr->bytes,off),w1=be16(sr->bytes,off+2),w2=be16(sr->bytes,off+4),w3=be16(sr->bytes,off+6);auto tile=(std::uint16_t)(w3&0x7ff);if(!tile)continue;int zy=((w0>>9)&0x7f)+1,zx=(w1&0x7f)+1;if(Machine::format_for_zoomx(zx)==SpriteFormat::Invalid)continue;int x=w2&0x1ff;if(x>0x140)x-=0x200;int y=sprite_visible_y(w0,VISIBLE_Y_START,layer_offset_sprites_.y);x+=layer_offset_sprites_.x;SpriteInstance q{x,y,zx,zy,tile,(std::uint8_t)((w1&0x7f80)>>7),(w2&0x4000)!=0,(w2&0x8000)!=0,(w1&0x8000)?1:0};if((int)pos==force_slot&&force_priority>=0)q.priority=force_priority;auto addr=sprite_prom_address(q,w2,scene.prom_input_mask);auto pe=mix_lut_[addr];draw_sprite(px,machine,q,&runtime.bus,&lm,LAYER_SPRITE,pe.block_mask,&pa,&po,&ps,addr,pe.raw,PromSource::Sprite,nullptr,nullptr,(std::uint16_t)pos,0,&occupied);}}
        draw_runtime_text_layer(px,lm,runtime.bus,LAYER_TEXT);return px;};
    auto expdir=root/"experiments";std::filesystem::create_directories(expdir,ec);for(std::size_t ri=0;ri<std::min<std::size_t>(4,ranks.size());++ri){auto slot=(int)ranks[ri].slot;std::ostringstream a,b,c;a<<"suppress_slot_"<<std::setfill('0')<<std::setw(3)<<slot<<".png";b<<"force_slot_"<<std::setfill('0')<<std::setw(3)<<slot<<"_priority0.png";c<<"force_slot_"<<std::setfill('0')<<std::setw(3)<<slot<<"_priority1.png";diag_save_png(expdir/a.str(),WIDTH,HEIGHT,render_variant(slot,-1,-1));diag_save_png(expdir/b.str(),WIDTH,HEIGHT,render_variant(-1,slot,0));diag_save_png(expdir/c.str(),WIDTH,HEIGHT,render_variant(-1,slot,1));}


    // Hardware palette versus the historical debug fallback.  Normal rendering
    // now uses hardware TC0110PCR values only; these images make any remaining
    // palette-bank issue inspectable without contaminating the live picture.
    constexpr int PAL_GRID=64, PAL_CELL=4;
    std::vector<std::uint32_t> pal_hw(PAL_GRID*PAL_CELL*PAL_GRID*PAL_CELL,0xff000000u),pal_dbg=pal_hw,pal_diff=pal_hw;
    for(unsigned index=0;index<4096;++index){unsigned bank=index>>4,pen=index&15, gx=index%PAL_GRID,gy=index/PAL_GRID;auto hw=runtime_palette_color(runtime.bus,(std::uint16_t)bank,(std::uint8_t)pen);auto raw=runtime.bus.palette[index];auto dbg=hw;if(pen && raw==0)dbg=sprite_color((std::uint8_t)bank,(std::uint8_t)pen);unsigned dr=std::abs((int)((hw>>16)&255)-(int)((dbg>>16)&255)),dg=std::abs((int)((hw>>8)&255)-(int)((dbg>>8)&255)),db=std::abs((int)(hw&255)-(int)(dbg&255));auto dc=0xff000000u|(std::min(255u,dr*2)<<16)|(std::min(255u,dg*2)<<8)|std::min(255u,db*2);for(int yy=0;yy<PAL_CELL;++yy)for(int xx=0;xx<PAL_CELL;++xx){auto d=(std::size_t)(gy*PAL_CELL+yy)*(PAL_GRID*PAL_CELL)+(gx*PAL_CELL+xx);pal_hw[d]=hw?hw:0xff000000u;pal_dbg[d]=dbg?dbg:0xff000000u;pal_diff[d]=dc;}}
    diag_save_png(root/"50_palette_hardware.png",PAL_GRID*PAL_CELL,PAL_GRID*PAL_CELL,pal_hw);
    diag_save_png(root/"51_palette_debug_fallback.png",PAL_GRID*PAL_CELL,PAL_GRID*PAL_CELL,pal_dbg);
    diag_save_png(root/"52_palette_difference.png",PAL_GRID*PAL_CELL,PAL_GRID*PAL_CELL,pal_diff);

    // Summary identifies suspicious overlap / mixer behaviour without image inspection.

    // v0.40: special-sprite/palette forensic package.  This is intentionally
    // redundant: the same hypothesis can be checked from raw pen identity,
    // TC0110PCR provenance, pair overlap and controlled renderer variants.
    {
        std::ofstream pw(root/"palette_write_provenance.csv");
        pw << "index,bank,pen,current_raw,write_count,first_frame,last_frame,last_pc,first_value,last_value\n";
        for(unsigned i=0;i<4096;++i) if(runtime.bus.palette_write_count[i])
            pw << i << ',' << (i>>4) << ',' << (i&15) << ',' << runtime.bus.palette[i] << ','
               << runtime.bus.palette_write_count[i] << ',' << runtime.bus.palette_first_write_frame[i] << ','
               << runtime.bus.palette_last_write_frame[i] << ",0x" << std::hex << runtime.bus.palette_last_write_pc[i]
               << std::dec << ',' << runtime.bus.palette_first_written_value[i] << ',' << runtime.bus.palette_last_written_value[i] << "\n";

        std::ofstream sp(root/"sprite_pen_palette_forensics.csv");
        sp << "slot,palette,pen,palette_index,palette_raw,was_written,write_count,first_frame,last_frame,pixels_in_isolated_sprite\n";
        if(sr){
            for(const auto& d:diag_sprites){
                std::array<std::uint64_t,16> penhist{};
                auto off=(std::size_t)d.slot*8; auto w0=be16(sr->bytes,off),w1=be16(sr->bytes,off+2),w2=be16(sr->bytes,off+4),w3=be16(sr->bytes,off+6);
                auto tile=(std::uint16_t)(w3&0x7ff); int zy=((w0>>9)&0x7f)+1,zx=(w1&0x7f)+1; auto fmt=Machine::format_for_zoomx(zx);
                int x=w2&0x1ff;if(x>0x140)x-=0x200; int y=sprite_visible_y(w0,VISIBLE_Y_START,layer_offset_sprites_.y);x+=layer_offset_sprites_.x; SpriteInstance q{x,y,zx,zy,tile,(std::uint8_t)((w1&0x7f80)>>7),(w2&0x4000)!=0,(w2&0x8000)!=0,(w1&0x8000)?1:0};
                int cols=fmt==SpriteFormat::Obj128x128?8:fmt==SpriteFormat::Obj64x128?4:2; std::size_t base=fmt==SpriteFormat::Obj128x128?((std::size_t)tile<<6):fmt==SpriteFormat::Obj64x128?(((std::size_t)tile<<5)+0x20000):(((std::size_t)tile<<4)+0x30000); const auto& gfx=(fmt==SpriteFormat::Obj128x128)?machine.sprites_a():machine.sprites_b();
                int by=y+(128-zy);
                for(int j=0;j<8;++j)for(int k=0;k<cols;++k){int mx=q.flip_x?(cols-1-k):k,my=q.flip_y?(7-j):j;auto code=machine.spritemap_word(base+(std::size_t)(mx+my*cols));int cx=x+(k*zx)/cols,cy=by+(j*zy)/8,nx=x+((k+1)*zx)/cols,ny=by+((j+1)*zy)/8;int cw=nx-cx,ch=ny-cy;if(cw<=0||ch<=0)continue;for(int dy=0;dy<ch;++dy){int sy=std::clamp((dy*16)/ch,0,15);if(q.flip_y)sy=15-sy;for(int dx=0;dx<cw;++dx){int sx=std::clamp((dx*16)/cw,0,15);if(q.flip_x)sx=15-sx;auto pen=sprite_pixel(gfx,(std::size_t)(code&0x3fff),sx,sy);if(pen){int px=cx+dx,py=cy+dy;if(px>=0&&px<WIDTH&&py>=0&&py<HEIGHT)++penhist[pen];}}}}
                for(unsigned pen=1;pen<16;++pen) if(penhist[pen]){unsigned pi=((d.palette&255)<<4)|pen;sp<<d.slot<<','<<d.palette<<','<<pen<<','<<pi<<','<<runtime.bus.palette[pi]<<','<<(runtime.bus.palette_write_count[pi]!=0)<<','<<runtime.bus.palette_write_count[pi]<<','<<runtime.bus.palette_first_write_frame[pi]<<','<<runtime.bus.palette_last_write_frame[pi]<<','<<penhist[pen]<<"\n";}
            }
        }

        // Focus report for the coincident player-car pair discovered in v0.37/v0.38.
        std::ofstream car(root/"player_car_pair_044_045.txt");
        car << "v"<<kNativeVersion<<" focused pair report (slots 44/45)\n";
        for(unsigned slot: {44u,45u}) if(slot<diag_sprites.size() || sr){
            if(!sr || slot*8+7>=sr->bytes.size()) continue;auto off=(std::size_t)slot*8;auto w0=be16(sr->bytes,off),w1=be16(sr->bytes,off+2),w2=be16(sr->bytes,off+4),w3=be16(sr->bytes,off+6);unsigned pal=(w1&0x7f80)>>7;
            car << "slot="<<slot<<" raw="<<std::hex<<w0<<' '<<w1<<' '<<w2<<' '<<w3<<std::dec<<" palette="<<pal<<" priority="<<((w1>>15)&1)<<"\n";
            for(unsigned pen=1;pen<16;++pen){unsigned pi=((pal&255)<<4)|pen;car<<" pen="<<pen<<" index="<<pi<<" raw="<<runtime.bus.palette[pi]<<" writes="<<runtime.bus.palette_write_count[pi]<<" first="<<runtime.bus.palette_first_write_frame[pi]<<" last="<<runtime.bus.palette_last_write_frame[pi]<<" pc=0x"<<std::hex<<runtime.bus.palette_last_write_pc[pi]<<std::dec<<"\n";}
        }

        // Bottom-screen ownership/provenance: this isolates the city-layer disagreement.
        std::ofstream low(root/"bottom_100px_reference_vs_prom.csv");
        low << "x,y,reference_argb,prom_argb,different,reference_priority,prom_source,prom_addr,prom_out,layer_mask,sprite_owner,sprite_coverage\n";
        for(int y=HEIGHT-100;y<HEIGHT;++y)for(int x=0;x<WIDTH;++x){auto d=(std::size_t)y*WIDTH+x;if(reference[d]!=prom_final[d])low<<x<<','<<y<<",0x"<<std::hex<<reference[d]<<",0x"<<prom_final[d]<<std::dec<<",1,"<<(unsigned)reference_priority[d]<<','<<(unsigned)prom_source[d]<<','<<(unsigned)prom_addr[d]<<','<<(unsigned)prom_out[d]<<','<<(unsigned)layer_mask[d]<<','<<sprite_owner[d]<<','<<sprite_coverage[d]<<"\n";}

        // Road/reference priority histograms expose missing classes immediately.
        std::array<std::uint64_t,256> refph{},lmh{},rph{};for(std::size_t i=0;i<reference_priority.size();++i){++refph[reference_priority[i]];++lmh[layer_mask[i]];++rph[road_probe[i]];}
        std::ofstream ph(root/"priority_class_histograms.csv");ph<<"kind,value,count\n";for(unsigned i=0;i<256;++i){if(refph[i])ph<<"reference_priority,"<<i<<','<<refph[i]<<"\n";if(lmh[i])ph<<"prom_layer_mask,"<<i<<','<<lmh[i]<<"\n";if(rph[i])ph<<"road_probe,"<<i<<','<<rph[i]<<"\n";}

        // Controlled zero-palette-entry experiment for the strongest candidates.
        auto render_variant_palette=[&](int skip_slot,int special_slot,int palette_mode){
            std::vector<std::uint8_t> occupied(WIDTH*HEIGHT,0);
            std::vector<std::uint32_t> px(WIDTH*HEIGHT,0xff080808u);std::vector<std::uint8_t> lm(WIDTH*HEIGHT,0),pa(WIDTH*HEIGHT,0),po(WIDTH*HEIGHT,0),ps(WIDTH*HEIGHT,0),rp(WIDTH*HEIGHT,0);const Region* cc=find_region(runtime.bus,"tile_control");auto cv=(cc&&cc->bytes.size()>=14)?be16(cc->bytes,12):0;int bt=(cv&8)?1:0,up=bt^1;draw_runtime_tile_layer(px,lm,machine,runtime.bus,bt,0x00,true);draw_runtime_tile_layer(px,lm,machine,runtime.bus,up,0x01,false);draw_runtime_road(px,lm,pa,po,ps,rp,machine,runtime.bus,false,scene.prom_input_mask,true);draw_runtime_text_layer(px,lm,runtime.bus,0x04);
            if(sr){for(std::size_t pos=sr->bytes.size()/8;pos-->0;){if((int)pos==skip_slot)continue;auto off=pos*8;auto w0=be16(sr->bytes,off),w1=be16(sr->bytes,off+2),w2=be16(sr->bytes,off+4),w3=be16(sr->bytes,off+6);auto tile=(std::uint16_t)(w3&0x7ff);if(!tile)continue;int zy=((w0>>9)&0x7f)+1,zx=(w1&0x7f)+1;if(Machine::format_for_zoomx(zx)==SpriteFormat::Invalid)continue;int x=w2&0x1ff;if(x>0x140)x-=0x200;int y=sprite_visible_y(w0,VISIBLE_Y_START,layer_offset_sprites_.y);x+=layer_offset_sprites_.x;SpriteInstance q{x,y,zx,zy,tile,(std::uint8_t)((w1&0x7f80)>>7),(w2&0x4000)!=0,(w2&0x8000)!=0,(w1&0x8000)?1:0};int pm=((int)pos==special_slot)?palette_mode:0;draw_sprite(px,machine,q,&runtime.bus,&lm,LAYER_SPRITE,(q.priority?0xfc:0xf0),&pa,&po,&ps,0,0,PromSource::Sprite,nullptr,nullptr,(std::uint16_t)pos,pm,&occupied);}}
            return px;
        };
        auto e2=root/"experiments_v039";std::filesystem::create_directories(e2,ec);
        diag_save_png(e2/"slot044_zero_palette_transparent.png",WIDTH,HEIGHT,render_variant_palette(-1,44,1));
        diag_save_png(e2/"slot045_zero_palette_transparent.png",WIDTH,HEIGHT,render_variant_palette(-1,45,1));
        diag_save_png(e2/"slot044_raw_pen_identity.png",WIDTH,HEIGHT,render_variant_palette(-1,44,2));
        diag_save_png(e2/"slot045_raw_pen_identity.png",WIDTH,HEIGHT,render_variant_palette(-1,45,2));
        diag_save_png(e2/"suppress_044_reference.png",WIDTH,HEIGHT,render_variant_palette(44,-1,0));
        diag_save_png(e2/"suppress_045_reference.png",WIDTH,HEIGHT,render_variant_palette(45,-1,0));
    }

    // v0.44 controlled compositor experiments. These are forensic-only and never
    // alter the default gameplay renderer. The same raw sprite records and reference
    // priority masks are used while one variable is changed at a time.
    if (forensics_.experiment_layer_matrix || !forensics_.experiment_layer_order.empty() ||
        forensics_.experiment_sprite_pair_order || forensics_.experiment_zero_slot ||
        forensics_.experiment_zero_palette_bank || forensics_.experiment_sprite_mask_prio0 ||
        forensics_.experiment_sprite_mask_prio1 || !forensics_.experiment_sprite_mask_matrix.empty() ||
        forensics_.experiment_sprite_order != "descending") {
        auto exdir=root/"experiments_v046"; std::filesystem::create_directories(exdir,ec);
        auto render_experiment=[&](const std::vector<std::string>& order, bool ascending,
                                   std::optional<std::pair<unsigned,unsigned>> pair_order,
                                   std::optional<unsigned> zero_slot,
                                   std::optional<unsigned> zero_bank,
                                   std::optional<unsigned> mask0,
                                   std::optional<unsigned> mask1) {
            std::vector<std::uint32_t> px(WIDTH*HEIGHT,0xff080808u);
            std::vector<std::uint8_t> pri(WIDTH*HEIGHT,0),pa(WIDTH*HEIGHT,0),po(WIDTH*HEIGHT,0),ps(WIDTH*HEIGHT,0),rp(WIDTH*HEIGHT,0);
            const Region* cc=find_region(runtime.bus,"tile_control"); auto cv=(cc&&cc->bytes.size()>=14)?be16(cc->bytes,12):0;
            int bt=(cv&8)?1:0,up=bt^1;
            auto draw_sprites_custom=[&](){
                if(!sr) return;
                std::vector<std::uint8_t> occupied(WIDTH*HEIGHT,0);
                const std::size_t n=sr->bytes.size()/8; std::vector<std::size_t> ord; ord.reserve(n);
                if(ascending) for(std::size_t i=0;i<n;++i) ord.push_back(i); else for(std::size_t i=n;i-->0;) ord.push_back(i);
                if(pair_order){
                    auto a=std::find(ord.begin(),ord.end(),pair_order->first), b=std::find(ord.begin(),ord.end(),pair_order->second);
                    if(a!=ord.end()&&b!=ord.end()){
                        // Ensure the requested A:B means A is drawn before B without disturbing other slots more than necessary.
                        auto ia=std::distance(ord.begin(),a), ib=std::distance(ord.begin(),b);
                        if(ia>ib) std::iter_swap(a,b);
                    }
                }
                for(auto pos:ord){
                    auto off=pos*8; auto w0=be16(sr->bytes,off),w1=be16(sr->bytes,off+2),w2=be16(sr->bytes,off+4),w3=be16(sr->bytes,off+6);
                    auto tile=(std::uint16_t)(w3&0x7ff); if(!tile)continue; int zy=((w0>>9)&0x7f)+1,zx=(w1&0x7f)+1;
                    if(Machine::format_for_zoomx(zx)==SpriteFormat::Invalid)continue; int x=w2&0x1ff;if(x>0x140)x-=0x200;int y=sprite_visible_y(w0,VISIBLE_Y_START,layer_offset_sprites_.y);x+=layer_offset_sprites_.x;
                    SpriteInstance q{x,y,zx,zy,tile,(std::uint8_t)((w1&0x7f80)>>7),(w2&0x4000)!=0,(w2&0x8000)!=0,(w1&0x8000)?1:0};
                    int pm=((zero_slot&&pos==*zero_slot)||(zero_bank&&q.color==*zero_bank))?1:0;
                    const std::uint8_t primask=(std::uint8_t)(q.priority ? mask1.value_or(0xfc) : mask0.value_or(0xf0));
                    draw_sprite(px,machine,q,&runtime.bus,&pri,0,primask,&pa,&po,&ps,0,0,PromSource::Sprite,nullptr,nullptr,(std::uint16_t)pos,pm,&occupied);
                }
            };
            for(const auto& layer:order){
                if(layer=="bottom"||layer=="bg0") draw_runtime_tile_layer(px,pri,machine,runtime.bus,bt,0x00,true);
                else if(layer=="upper"||layer=="bg1") draw_runtime_tile_layer(px,pri,machine,runtime.bus,up,0x01,false);
                else if(layer=="road") draw_runtime_road(px,pri,pa,po,ps,rp,machine,runtime.bus,false,scene.prom_input_mask,true);
                else if(layer=="text") draw_runtime_text_layer(px,pri,runtime.bus,0x04);
                else if(layer=="sprites"||layer=="sprite") draw_sprites_custom();
            }
            return px;
        };
        const std::vector<std::string> normal_order={"bottom","upper","road","text","sprites"};
        std::ofstream cmp(exdir/"experiment_comparison.csv"); cmp<<"name,changed_pixels,changed_in_requested_location\n";
        std::vector<std::pair<std::string,std::vector<std::uint32_t>>> experiment_images;
        auto emit=[&](const std::string& name,const std::vector<std::uint32_t>& img){
            diag_save_png(exdir/(name+".png"),WIDTH,HEIGHT,img); std::uint64_t all=0,roi=0;
            auto loc=[&](int x,int y){if(forensics_.pixels.empty()&&forensics_.regions.empty())return true;for(auto q:forensics_.pixels)if(q.first==x&&q.second==y)return true;for(auto&r:forensics_.regions)if(x>=r.x1&&x<=r.x2&&y>=r.y1&&y<=r.y2)return true;return false;};
            for(int y=0;y<HEIGHT;++y)for(int x=0;x<WIDTH;++x){auto i=(std::size_t)y*WIDTH+x;if(img[i]!=reference[i]){++all;if(loc(x,y))++roi;}}
            cmp<<name<<','<<all<<','<<roi<<"\n"; experiment_images.push_back({name,img});
        };
        if(forensics_.experiment_zero_slot) emit("zero_transparent_slot_"+std::to_string(*forensics_.experiment_zero_slot),render_experiment(normal_order,false,std::nullopt,forensics_.experiment_zero_slot,std::nullopt,forensics_.experiment_sprite_mask_prio0,forensics_.experiment_sprite_mask_prio1));
        if(forensics_.experiment_zero_palette_bank) emit("zero_transparent_palette_"+std::to_string(*forensics_.experiment_zero_palette_bank),render_experiment(normal_order,false,std::nullopt,std::nullopt,forensics_.experiment_zero_palette_bank,forensics_.experiment_sprite_mask_prio0,forensics_.experiment_sprite_mask_prio1));
        if(forensics_.experiment_sprite_pair_order){auto pr=*forensics_.experiment_sprite_pair_order;emit("pair_order_"+std::to_string(pr.first)+"_then_"+std::to_string(pr.second),render_experiment(normal_order,false,pr,std::nullopt,std::nullopt,forensics_.experiment_sprite_mask_prio0,forensics_.experiment_sprite_mask_prio1));emit("pair_order_"+std::to_string(pr.second)+"_then_"+std::to_string(pr.first),render_experiment(normal_order,false,std::make_pair(pr.second,pr.first),std::nullopt,std::nullopt,forensics_.experiment_sprite_mask_prio0,forensics_.experiment_sprite_mask_prio1));}
        if(forensics_.experiment_sprite_order=="ascending") emit("sprite_order_ascending",render_experiment(normal_order,true,std::nullopt,std::nullopt,std::nullopt,forensics_.experiment_sprite_mask_prio0,forensics_.experiment_sprite_mask_prio1));
        if(!forensics_.experiment_layer_order.empty()) emit("custom_layer_order",render_experiment(forensics_.experiment_layer_order,forensics_.experiment_sprite_order=="ascending",forensics_.experiment_sprite_pair_order,forensics_.experiment_zero_slot,forensics_.experiment_zero_palette_bank,forensics_.experiment_sprite_mask_prio0,forensics_.experiment_sprite_mask_prio1));
        if(forensics_.experiment_layer_matrix){
            const std::vector<std::pair<std::string,std::vector<std::string>>> matrix={
                {"layer_reference",normal_order},
                {"layer_road_before_upper",{"bottom","road","upper","text","sprites"}},
                {"layer_sprites_before_text",{"bottom","upper","road","sprites","text"}},
                {"layer_sprites_before_road",{"bottom","upper","sprites","road","text"}},
                {"layer_road_last",{"bottom","upper","text","sprites","road"}}
            };
            for(const auto&m:matrix) emit(m.first,render_experiment(m.second,false,std::nullopt,std::nullopt,std::nullopt,forensics_.experiment_sprite_mask_prio0,forensics_.experiment_sprite_mask_prio1));
        }
        if(forensics_.experiment_sprite_mask_prio0 || forensics_.experiment_sprite_mask_prio1) {
            std::ostringstream name; name<<"primask_p0_"<<std::hex<<std::setw(2)<<std::setfill('0')<<forensics_.experiment_sprite_mask_prio0.value_or(0xf0)
                                      <<"_p1_"<<std::setw(2)<<forensics_.experiment_sprite_mask_prio1.value_or(0xfc);
            emit(name.str(),render_experiment(normal_order,false,std::nullopt,std::nullopt,std::nullopt,
                                               forensics_.experiment_sprite_mask_prio0,forensics_.experiment_sprite_mask_prio1));
        }
        if(!forensics_.experiment_sprite_mask_matrix.empty()) {
            std::ofstream mcsv(exdir/"primask_matrix.csv");
            mcsv<<"mask0,mask1,changed_pixels,target_pixels_changed,collateral_pixels,heuristic_score,saved_image\n";
            struct MRes{unsigned m0=0,m1=0;std::uint64_t changed=0,target=0,collateral=0;double score=0;std::vector<std::uint32_t> img;};
            std::vector<MRes> results;
            auto is_target=[&](int x,int y){for(auto q:forensics_.pixels)if(q.first==x&&q.second==y)return true;return false;};
            for(auto m0:forensics_.experiment_sprite_mask_matrix) for(auto m1:forensics_.experiment_sprite_mask_matrix) {
                auto img=render_experiment(normal_order,false,std::nullopt,std::nullopt,std::nullopt,m0,m1);
                MRes r;r.m0=m0;r.m1=m1;r.img=std::move(img);
                for(int y=0;y<HEIGHT;++y)for(int x=0;x<WIDTH;++x){auto i=(std::size_t)y*WIDTH+x;if(r.img[i]!=reference[i]){++r.changed;if(is_target(x,y))++r.target;else ++r.collateral;}}
                // Diagnostic heuristic only: reward changing known-bad targets while strongly penalising collateral damage.
                r.score=double(r.target)*10000.0-double(r.collateral);
                results.push_back(std::move(r));
            }
            std::sort(results.begin(),results.end(),[](const MRes&a,const MRes&b){if(a.score!=b.score)return a.score>b.score;return a.collateral<b.collateral;});
            for(std::size_t i=0;i<results.size();++i){auto&r=results[i];bool save=i<std::min<std::size_t>(8,results.size());std::string fn;
                if(save){std::ostringstream n;n<<"primask_rank"<<std::dec<<(i+1)<<"_p0_"<<std::hex<<std::setw(2)<<std::setfill('0')<<r.m0<<"_p1_"<<std::setw(2)<<r.m1;fn=n.str()+".png";diag_save_png(exdir/fn,WIDTH,HEIGHT,r.img);experiment_images.push_back({n.str(),r.img});}
                mcsv<<std::hex<<std::setw(2)<<std::setfill('0')<<r.m0<<','<<std::setw(2)<<r.m1<<std::dec<<','<<r.changed<<','<<r.target<<','<<r.collateral<<','<<r.score<<','<<fn<<"\n";
            }
            std::ofstream ms(exdir/"primask_matrix_summary.txt");
            ms<<"v"<<kNativeVersion<<" sprite primask experiment matrix\n"<<"NOTE: heuristic score is diagnostic ranking only, not proof of hardware correctness.\n";
            ms<<"targets="<<forensics_.pixels.size()<<" candidates="<<forensics_.experiment_sprite_mask_matrix.size()<<" combinations="<<results.size()<<"\n";
            for(std::size_t i=0;i<std::min<std::size_t>(8,results.size());++i){auto&r=results[i];ms<<"rank="<<(i+1)<<" mask0=0x"<<std::hex<<r.m0<<" mask1=0x"<<r.m1<<std::dec<<" target_changes="<<r.target<<" collateral="<<r.collateral<<" score="<<r.score<<"\n";}
        }
        std::ofstream pd(exdir/"experiment_pairwise_diff.csv");
        pd<<"experiment_a,experiment_b,different_pixels,first_x,first_y,second_x,second_y\n";
        for(std::size_t a=0;a<experiment_images.size();++a)for(std::size_t b=a+1;b<experiment_images.size();++b){
            std::uint64_t n=0;int x1=-1,y1=-1,x2=-1,y2=-1;
            for(int y=0;y<HEIGHT;++y)for(int x=0;x<WIDTH;++x){auto i=(std::size_t)y*WIDTH+x;if(experiment_images[a].second[i]!=experiment_images[b].second[i]){if(n==0){x1=x;y1=y;}else if(n==1){x2=x;y2=y;}++n;}}
            pd<<experiment_images[a].first<<','<<experiment_images[b].first<<','<<n<<','<<x1<<','<<y1<<','<<x2<<','<<y2<<"\n";
        }
        std::ofstream info(exdir/"README.txt");
        info<<"Chase H.Q. Native v"<<kNativeVersion<<" controlled compositor experiments.\n"
            <<"These images change one hypothesised rule at a time and are NOT default renderer fixes.\n"
            <<"Reference order: bottom,upper,road,text,sprites; default sprite traversal: descending; default primasks: {0xf0,0xfc}.\n";
    }


    // v0.40 parameterised graphics-forensics outputs. These filters are deliberately
    // independent of the heavyweight full-frame bundle so the same binary can be
    // reused for surgical follow-up investigations without recompilation.
    if (forensics_.enabled() && frame >= forensics_.from_frame && frame <= forensics_.to_frame) {
        auto has_u = [](const std::vector<unsigned>& v, unsigned x){return std::find(v.begin(),v.end(),x)!=v.end();};
        auto in_prom_range = [&](unsigned a){ if(forensics_.prom_addr_ranges.empty()) return true; for(const auto& r:forensics_.prom_addr_ranges) if(a>=r.start&&a<=r.end)return true; return false; };
        auto in_location = [&](int x,int y){
            if(forensics_.pixels.empty() && forensics_.regions.empty()) return true;
            for(auto p:forensics_.pixels) if(p.first==x&&p.second==y) return true;
            for(const auto& r:forensics_.regions) if(x>=r.x1&&x<=r.x2&&y>=r.y1&&y<=r.y2) return true;
            return false;
        };
        auto slot_selected = [&](unsigned id){
            if(forensics_.sprite_slots.empty() && forensics_.sprite_pairs.empty()) return true;
            if(has_u(forensics_.sprite_slots,id)) return true;
            for(auto pr:forensics_.sprite_pairs) if(id==pr.first||id==pr.second)return true;
            return false;
        };

        std::ofstream cfg(root/"graphics_trace_config.txt");
        cfg << "Chase H.Q. Native v"<<kNativeVersion<<" parameterised graphics trace\nframe="<<frame<<"\nfrom_frame="<<forensics_.from_frame<<"\nto_frame="<<forensics_.to_frame<<"\nmax_lines="<<forensics_.max_lines<<"\n";
        for(auto v:forensics_.sprite_slots)cfg<<"slot="<<v<<"\n";for(auto p:forensics_.sprite_pairs)cfg<<"pair="<<p.first<<':'<<p.second<<"\n";
        for(auto v:forensics_.palette_banks)cfg<<"palette_bank="<<v<<"\n";for(auto v:forensics_.palette_entries)cfg<<"palette_entry="<<v<<"\n";for(auto v:forensics_.pens)cfg<<"pen="<<v<<"\n";
        for(auto r:forensics_.prom_addr_ranges)cfg<<"prom_addr=0x"<<std::hex<<r.start<<":0x"<<r.end<<std::dec<<"\n";if(forensics_.prom_value)cfg<<"prom_value=0x"<<std::hex<<*forensics_.prom_value<<std::dec<<"\n";
        for(auto v:forensics_.priority_classes)cfg<<"priority="<<v<<"\n";for(auto p:forensics_.pixels)cfg<<"pixel="<<p.first<<':'<<p.second<<"\n";for(auto r:forensics_.regions)cfg<<"region="<<r.x1<<':'<<r.y1<<':'<<r.x2<<':'<<r.y2<<"\n";
        for(auto& l:forensics_.layers)cfg<<"layer="<<l<<"\n";if(forensics_.road_priority)cfg<<"road_priority="<<*forensics_.road_priority<<"\n";cfg<<"zero_palette="<<forensics_.zero_palette_use<<"\nanomalies="<<forensics_.anomalies_only<<"\n";

        std::ofstream pix(root/"targeted_pixel_pipeline.csv");
        pix << "frame,x,y,argb,layer_mask,reference_priority,prom_addr,prom_out,prom_source,road_probe,sprite_owner,sprite_coverage,selected\n";
        std::uint64_t lines=0, matched=0;
        for(int y=0;y<HEIGHT && lines<forensics_.max_lines;++y) for(int x=0;x<WIDTH && lines<forensics_.max_lines;++x){auto i=(std::size_t)y*WIDTH+x;if(!in_location(x,y))continue;unsigned own=sprite_owner[i];if(own!=0xffff && !slot_selected(own))continue;if(!in_prom_range(prom_addr[i]))continue;if(forensics_.prom_value && prom_out[i]!=*forensics_.prom_value)continue;if(!forensics_.priority_classes.empty()&&!has_u(forensics_.priority_classes,reference_priority[i]))continue;if(forensics_.road_priority&&road_probe[i]!=*forensics_.road_priority)continue;
            bool anomaly=(sprite_coverage[i]>1)||(road_probe[i]!=0&&reference_priority[i]==0)||(prom_source[i]!=0&&prom_addr[i]==0&&prom_out[i]==0);
            if(forensics_.anomalies_only&&!anomaly)continue;
            pix<<frame<<','<<x<<','<<y<<",0x"<<std::hex<<final_pixels[i]<<std::dec<<','<<(unsigned)layer_mask[i]<<','<<(unsigned)reference_priority[i]<<','<<(unsigned)prom_addr[i]<<','<<(unsigned)prom_out[i]<<','<<(unsigned)prom_source[i]<<','<<(unsigned)road_probe[i]<<','<<sprite_owner[i]<<','<<sprite_coverage[i]<<",1\n";++lines;++matched;}

        std::ofstream pal(root/"targeted_palette.csv");
        pal << "entry,bank,pen,raw15,written,write_count,first_frame,last_frame,last_pc,selected\n";
        for(unsigned e=0;e<4096;++e){unsigned bank=e>>4,pen=e&15;bool sel=(forensics_.palette_entries.empty()&&forensics_.palette_banks.empty()&&forensics_.pens.empty());if(has_u(forensics_.palette_entries,e))sel=true;if(has_u(forensics_.palette_banks,bank)&&(forensics_.pens.empty()||has_u(forensics_.pens,pen)))sel=true;if(!forensics_.pens.empty()&&has_u(forensics_.pens,pen)&&forensics_.palette_banks.empty())sel=true;if(forensics_.zero_palette_use&&runtime.bus.palette[e]==0)sel=true;if(!sel)continue;pal<<e<<','<<bank<<','<<pen<<",0x"<<std::hex<<runtime.bus.palette[e]<<std::dec<<','<<(runtime.bus.palette_write_count[e]!=0)<<','<<runtime.bus.palette_write_count[e]<<','<<runtime.bus.palette_first_write_frame[e]<<','<<runtime.bus.palette_last_write_frame[e]<<",0x"<<std::hex<<runtime.bus.palette_last_write_pc[e]<<std::dec<<",1\n";}

        std::ofstream su(root/"targeted_sprite_slots.csv");
        su << "slot,w0,w1,w2,w3,native_x,native_y,visible_x,visible_y,presented_x,presented_y,offset_x,offset_y,zoom_x,zoom_y,palette,priority,map_code,selected\n";
        if(sr){for(unsigned slot=0;slot<sr->bytes.size()/8;++slot){if(!slot_selected(slot))continue;auto off=(std::size_t)slot*8;auto w0=be16(sr->bytes,off),w1=be16(sr->bytes,off+2),w2=be16(sr->bytes,off+4),w3=be16(sr->bytes,off+6);auto tile=w3&0x7ff;if(!tile)continue;int x=w2&0x1ff;if(x>0x140)x-=0x200;int y=(w0&0x1ff)+7;if(y>0x140)y-=0x200;{int vy=y-VISIBLE_Y_START; int px=x+layer_offset_sprites_.x; int py=vy+layer_offset_sprites_.y; su<<slot<<",0x"<<std::hex<<w0<<",0x"<<w1<<",0x"<<w2<<",0x"<<w3<<std::dec<<','<<x<<','<<y<<','<<x<<','<<vy<<','<<px<<','<<py<<','<<layer_offset_sprites_.x<<','<<layer_offset_sprites_.y<<','<<((w1&0x7f)+1)<<','<<(((w0>>9)&0x7f)+1)<<','<<((w1&0x7f80)>>7)<<','<<((w1>>15)&1)<<",0x"<<std::hex<<tile<<std::dec<<",1\n";}}}

        std::ofstream pu(root/"targeted_prom_usage.csv"); pu<<"address,output,mix_hits,roadA_hits,roadB_hits,selected\n";
        for(unsigned a=0;a<256;++a){if(!in_prom_range(a))continue;if(forensics_.prom_value&&mix_lut_[a].raw!=*forensics_.prom_value&&road_lut_[a].raw!=*forensics_.prom_value)continue;if(mix_hist_[a]||road_a_hist_[a]||road_b_hist_[a])pu<<"0x"<<std::hex<<a<<",0x"<<(unsigned)mix_lut_[a].raw<<std::dec<<','<<mix_hist_[a]<<','<<road_a_hist_[a]<<','<<road_b_hist_[a]<<",1\n";}

        std::ofstream an(root/"targeted_anomalies.csv");an<<"kind,x,y,slot,value,detail\n";std::uint64_t ac=0;for(int y=0;y<HEIGHT&&ac<forensics_.max_lines;++y)for(int x=0;x<WIDTH&&ac<forensics_.max_lines;++x){auto i=(std::size_t)y*WIDTH+x;if(!in_location(x,y))continue;if(sprite_coverage[i]>1){an<<"sprite_overlap,"<<x<<','<<y<<','<<sprite_owner[i]<<','<<sprite_coverage[i]<<",multiple sprite candidates\n";++ac;}if(road_probe[i]!=0&&reference_priority[i]==0){an<<"road_priority_collapse,"<<x<<','<<y<<','<<sprite_owner[i]<<','<<(unsigned)road_probe[i]<<",raw road state collapsed to reference priority 0\n";++ac;}}

        std::ofstream ts(root/"targeted_trace_summary.txt");ts<<"frame="<<frame<<"\nmatched_pixel_rows="<<matched<<"\nanomaly_rows="<<ac<<"\nline_cap="<<forensics_.max_lines<<"\n";
    }

    std::ofstream summary(root/"diagnostics_summary.txt");
    summary<<"Chase H.Q. Native v"<<kNativeVersion<<" special-sprite + priority/palette forensic diagnostics\nframe="<<frame<<"\nselected_mixer="<<(scene.use_reference_mixer?"reference":(scene.use_prom_mixer?"prom":"legacy"))<<"\nprom_input_mask=0x"<<std::hex<<(unsigned)scene.prom_input_mask<<std::dec<<"\nreference_vs_prom_different_pixels="<<ref_prom_diff_count<<" / "<<(WIDTH*HEIGHT)<<"\nprom_vs_legacy_different_pixels="<<diff_count<<" / "<<(WIDTH*HEIGHT)<<"\nmax_sprite_overlap="<<maxcov<<"\n";
    summary<<"visible_sprite_pixels_by_slot:\n";for(unsigned i=0;i<visible.size();++i)if(visible[i])summary<<"  "<<i<<"="<<visible[i]<<"\n";
    std::array<std::uint64_t,64> maskhist{};for(auto m:layer_mask)if(m<maskhist.size())++maskhist[m];summary<<"layer_mask_histogram:\n";for(unsigned i=0;i<maskhist.size();++i)if(maskhist[i])summary<<"  0x"<<std::hex<<i<<std::dec<<"="<<maskhist[i]<<"\n";
    std::array<std::uint64_t,256> pah{},poh{};std::array<std::uint64_t,4> psh{};for(std::size_t i=0;i<prom_addr.size();++i){if(prom_source[i]){++pah[prom_addr[i]];++poh[prom_out[i]];if(prom_source[i]<4)++psh[prom_source[i]];}}summary<<"prom_source_pixels roadA="<<psh[1]<<" roadB="<<psh[2]<<" sprite="<<psh[3]<<"\n";

    if(diagnostics_raw_maps_){
        auto dumpbin=[&](const char* name,const auto& vec){std::ofstream f(root/name,std::ios::binary);f.write((const char*)vec.data(),(std::streamsize)(vec.size()*sizeof(typename std::decay_t<decltype(vec)>::value_type)));};
        dumpbin("layer_mask.bin",layer_mask);dumpbin("prom_addr.bin",prom_addr);dumpbin("prom_out.bin",prom_out);dumpbin("prom_source.bin",prom_source);dumpbin("road_probe.bin",road_probe);dumpbin("sprite_owner_u16.bin",sprite_owner);dumpbin("sprite_coverage_u16.bin",sprite_coverage);
        std::ofstream pcsv(root/"pixel_pipeline.csv");pcsv<<"x,y,argb,layer_mask,prom_addr,prom_out,prom_source,road_probe,sprite_owner,sprite_coverage\n";for(int y=0;y<HEIGHT;++y)for(int x=0;x<WIDTH;++x){auto i=(std::size_t)y*WIDTH+x;pcsv<<x<<','<<y<<','<<std::hex<<final_pixels[i]<<std::dec<<','<<(unsigned)layer_mask[i]<<','<<(unsigned)prom_addr[i]<<','<<(unsigned)prom_out[i]<<','<<(unsigned)prom_source[i]<<','<<(unsigned)road_probe[i]<<','<<sprite_owner[i]<<','<<sprite_coverage[i]<<"\n";}
    }

    std::ofstream readme(root/"README.txt");
    readme<<"v"<<kNativeVersion<<" special-sprite + priority/palette forensic laboratory. 00_reference_final retains the pre-v0.51 reference path for comparison; 01_prom_final is the validated default PROM path; 03 is their pixel difference.\n"
          <<"40_sprite_owner identifies the final accepted logical sprite slot per pixel; 41 shows overlap density.\n"
          <<"34 highlights pixels where the PROM-mask and legacy mixer paths disagree. sprite_screen_placement.csv gives exact final bounds and PROM data.\n";
    std::cout<<"[GRAPHICS DIAGNOSTICS] frame="<<frame<<" -> "<<root.string()<<"\n";
}


static std::array<std::uint8_t,7> dbg_glyph(char c) {
    c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
#define G(a,b,c,d,e,f,g) return {a,b,c,d,e,f,g}
    switch(c) {
    case 'A': G(14,17,17,31,17,17,17); case 'B': G(30,17,17,30,17,17,30);
    case 'C': G(14,17,16,16,16,17,14); case 'D': G(30,17,17,17,17,17,30);
    case 'E': G(31,16,16,30,16,16,31); case 'F': G(31,16,16,30,16,16,16);
    case 'G': G(14,17,16,23,17,17,15); case 'H': G(17,17,17,31,17,17,17);
    case 'I': G(31,4,4,4,4,4,31); case 'J': G(7,2,2,2,18,18,12);
    case 'K': G(17,18,20,24,20,18,17); case 'L': G(16,16,16,16,16,16,31);
    case 'M': G(17,27,21,21,17,17,17); case 'N': G(17,25,21,19,17,17,17);
    case 'O': G(14,17,17,17,17,17,14); case 'P': G(30,17,17,30,16,16,16);
    case 'Q': G(14,17,17,17,21,18,13); case 'R': G(30,17,17,30,20,18,17);
    case 'S': G(15,16,16,14,1,1,30); case 'T': G(31,4,4,4,4,4,4);
    case 'U': G(17,17,17,17,17,17,14); case 'V': G(17,17,17,17,17,10,4);
    case 'W': G(17,17,17,21,21,21,10); case 'X': G(17,17,10,4,10,17,17);
    case 'Y': G(17,17,10,4,4,4,4); case 'Z': G(31,1,2,4,8,16,31);
    case '0': G(14,17,19,21,25,17,14); case '1': G(4,12,4,4,4,4,14);
    case '2': G(14,17,1,2,4,8,31); case '3': G(30,1,1,14,1,1,30);
    case '4': G(2,6,10,18,31,2,2); case '5': G(31,16,16,30,1,1,30);
    case '6': G(14,16,16,30,17,17,14); case '7': G(31,1,2,4,8,8,8);
    case '8': G(14,17,17,14,17,17,14); case '9': G(14,17,17,15,1,1,14);
    case ':': G(0,4,4,0,4,4,0); case '.': G(0,0,0,0,0,6,6);
    case '-': G(0,0,0,31,0,0,0); case '/': G(1,2,2,4,8,8,16);
    case '[': G(14,8,8,8,8,8,14); case ']': G(14,2,2,2,2,2,14);
    case '(': G(2,4,8,8,8,4,2); case ')': G(8,4,2,2,2,4,8);
    case '=': G(0,31,0,31,0,0,0); case '+': G(0,4,4,31,4,4,0);
    case '_': G(0,0,0,0,0,0,31); case '#': G(10,31,10,10,31,10,0);
    case '?': G(14,17,1,2,4,0,4); case ' ': default: G(0,0,0,0,0,0,0);
    }
#undef G
}

void Video::debug_text(std::vector<std::uint32_t>& pixels, int x, int y, const std::string& text, std::uint32_t color, int scale) {
    int cx=x; scale=std::max(1,scale);
    for(char ch:text){ auto g=dbg_glyph(ch); for(int yy=0;yy<7;++yy)for(int xx=0;xx<5;++xx)if(g[yy]&(1u<<(4-xx)))for(int sy=0;sy<scale;++sy)for(int sx=0;sx<scale;++sx){int px=cx+xx*scale+sx,py=y+yy*scale+sy;if(px>=0&&px<WIDTH&&py>=0&&py<HEIGHT)pixels[(std::size_t)py*WIDTH+px]=color;} cx+=6*scale; if(cx>=WIDTH-4)break; }
}

void Video::debug_rect(std::vector<std::uint32_t>& pixels, int x0,int y0,int x1,int y1,std::uint32_t color){
    x0=std::clamp(x0,0,WIDTH-1);x1=std::clamp(x1,0,WIDTH-1);y0=std::clamp(y0,0,HEIGHT-1);y1=std::clamp(y1,0,HEIGHT-1);if(x0>x1||y0>y1)return;
    for(int x=x0;x<=x1;++x){pixels[(std::size_t)y0*WIDTH+x]=color;pixels[(std::size_t)y1*WIDTH+x]=color;}
    for(int y=y0;y<=y1;++y){pixels[(std::size_t)y*WIDTH+x0]=color;pixels[(std::size_t)y*WIDTH+x1]=color;}
}

void Video::rebuild_debug_sprite_list(const Runtime& runtime,const std::vector<std::uint16_t>& sprite_owner){
    debug_sprites_.clear(); last_sprite_owner_=sprite_owner; const Region* sr=find_region(runtime.bus,"sprites"); if(!sr)return;
    std::array<std::uint64_t,256> vis{}; std::array<int,256> vl,vt,vr,vb; vl.fill(WIDTH); vt.fill(HEIGHT); vr.fill(-1); vb.fill(-1); for(std::size_t pi=0;pi<sprite_owner.size();++pi){auto id=sprite_owner[pi];if(id<256){++vis[id];int px=(int)(pi%WIDTH),py=(int)(pi/WIDTH);vl[id]=std::min(vl[id],px);vt[id]=std::min(vt[id],py);vr[id]=std::max(vr[id],px);vb[id]=std::max(vb[id],py);}}
    for(std::size_t pos=0;pos*8+7<sr->bytes.size();++pos){auto off=pos*8;auto w0=be16(sr->bytes,off),w1=be16(sr->bytes,off+2),w2=be16(sr->bytes,off+4),w3=be16(sr->bytes,off+6);auto tile=(std::uint16_t)(w3&0x7ff);if(!tile)continue;int zy=((w0>>9)&0x7f)+1,zx=(w1&0x7f)+1;if(Machine::format_for_zoomx(zx)==SpriteFormat::Invalid)continue;int x=w2&0x1ff;if(x>0x140)x-=0x200;int y=sprite_visible_y(w0,VISIBLE_Y_START,layer_offset_sprites_.y);x+=layer_offset_sprites_.x;int by=y+(128-zy);DebugSpriteInfo d;d.slot=(unsigned)pos;d.raw_w0=w0;d.raw_w1=w1;d.raw_w2=w2;d.raw_w3=w3;d.raw_x=(int)(w2&0x1ff);d.raw_y=(int)(w0&0x1ff);d.x=x;d.y=by;d.width=zx;d.height=zy;d.map=tile;d.palette=(std::uint8_t)((w1&0x7f80)>>7);d.priority=(w1&0x8000)?1:0;d.visible_pixels=vis[pos];if(vis[pos]){d.visible_left=vl[pos];d.visible_top=vt[pos];d.visible_right=vr[pos];d.visible_bottom=vb[pos];}debug_sprites_.push_back(d);}
    std::stable_sort(debug_sprites_.begin(),debug_sprites_.end(),[](const auto&a,const auto&b){if(a.visible_pixels!=b.visible_pixels)return a.visible_pixels>b.visible_pixels;return a.slot<b.slot;});
}



void Video::set_runtime_sprite_override(unsigned slot, int map, int palette, int visible) {
    auto it=std::find_if(runtime_sprite_overrides_.begin(),runtime_sprite_overrides_.end(),[&](const auto& q){return q.slot==slot;});
    if(it==runtime_sprite_overrides_.end()){ RuntimeSpriteOverride q; q.slot=slot; runtime_sprite_overrides_.push_back(q); it=runtime_sprite_overrides_.end()-1; }
    if(map>=-1) it->map=map; if(palette>=-1) it->palette=palette; if(visible>=-1) it->visible=visible;
}
void Video::clear_runtime_sprite_overrides(){ runtime_sprite_overrides_.clear(); }
void Video::set_runtime_sprite_visual(unsigned slot,const std::string& mode){ runtime_visual_slot_=(int)slot; runtime_visual_mode_=mode; }
void Video::clear_runtime_sprite_visual(){ runtime_visual_slot_=-1; runtime_visual_mode_.clear(); }

void Video::configure_sprite_presentation(const std::vector<Options::SpriteSelector>& solo, const std::vector<Options::SpriteSelector>& hide, const std::vector<Options::SpriteQuadCandidate>& quads, Options::DiagnosticBackground background) {
    sprite_solo_ = solo;
    sprite_hide_ = hide;
    sprite_quad_candidates_ = quads;
    diagnostic_background_ = background;
}

bool Video::write_sprite_evidence_csv(const std::filesystem::path& path, unsigned frame) const {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path); if(!out) return false;
    out << "frame,slot,raw_w0,raw_w1,raw_w2,raw_w3,raw_x9,raw_y9,map,palette,priority,flip_x,flip_y,screen_x,screen_y,render_width,render_height,render_left,render_top,render_right,render_bottom,visible_pixels,visible_left,visible_top,visible_right,visible_bottom\n";
    for(const auto& s: debug_sprites_) {
        out << frame << ',' << s.slot << ',' << s.raw_w0 << ',' << s.raw_w1 << ',' << s.raw_w2 << ',' << s.raw_w3 << ',' << s.raw_x << ',' << s.raw_y << ','
            << s.map << ',' << (unsigned)s.palette << ',' << s.priority << ',' << ((s.raw_w2&0x4000)?1:0) << ',' << ((s.raw_w2&0x8000)?1:0) << ','
            << s.x << ',' << s.y << ',' << s.width << ',' << s.height << ',' << s.x << ',' << s.y << ','
            << (s.x+s.width-1) << ',' << (s.y+s.height-1) << ',' << s.visible_pixels << ','
            << s.visible_left << ',' << s.visible_top << ',' << s.visible_right << ',' << s.visible_bottom << "\n";
    }
    return (bool)out;
}

bool Video::write_sprite_quad_evidence_csv(const std::filesystem::path& path, unsigned frame) const {
    if(sprite_quad_candidates_.empty()) return true;
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path); if(!out) return false;
    out << "frame,group,instance,tl_slot,tr_slot,bl_slot,br_slot,palette,priority,left,top,right,bottom,width,height,visible_pixels\n";
    unsigned instance=0;
    auto compatible=[&](const DebugSpriteInfo& a,const DebugSpriteInfo& b,const Options::SpriteQuadCandidate& q){
        return a.width==b.width && a.height==b.height && a.palette==b.palette && a.priority==b.priority &&
               (q.palette<0 || (int)a.palette==q.palette) && (q.priority<0 || a.priority==q.priority);
    };
    for(const auto& q:sprite_quad_candidates_) {
        for(const auto& tl:debug_sprites_) {
            if((int)tl.map!=q.tl || (q.palette>=0 && (int)tl.palette!=q.palette) || (q.priority>=0 && tl.priority!=q.priority)) continue;
            const DebugSpriteInfo *tr=nullptr,*bl=nullptr,*br=nullptr;
            for(const auto& s:debug_sprites_) {
                if(!compatible(tl,s,q)) continue;
                if((int)s.map==q.tr && s.x==tl.x+tl.width && s.y==tl.y) tr=&s;
                else if((int)s.map==q.bl && s.x==tl.x && s.y==tl.y+tl.height) bl=&s;
                else if((int)s.map==q.br && s.x==tl.x+tl.width && s.y==tl.y+tl.height) br=&s;
            }
            if(!tr||!bl||!br) continue;
            const auto vp=tl.visible_pixels+tr->visible_pixels+bl->visible_pixels+br->visible_pixels;
            out<<frame<<','<<q.name<<','<<instance++<<','<<tl.slot<<','<<tr->slot<<','<<bl->slot<<','<<br->slot<<','
               <<(unsigned)tl.palette<<','<<tl.priority<<','<<tl.x<<','<<tl.y<<','<<(tl.x+2*tl.width-1)<<','<<(tl.y+2*tl.height-1)<<','
               <<(2*tl.width)<<','<<(2*tl.height)<<','<<vp<<"\n";
        }
    }
    return (bool)out;
}


bool Video::write_semantic_object_evidence_csv(const std::filesystem::path& path, unsigned frame, const std::vector<std::string>& objects) const {
    if(objects.empty()) return true;
    std::filesystem::create_directories(path.parent_path());
    const bool exists=std::filesystem::exists(path);
    std::ofstream out(path,std::ios::app); if(!out) return false;
    if(!exists) out << "frame,object,present,body_slot,body_map,body_palette,shadow_slot,shadow_map,shadow_palette,x,y,left,top,right,bottom,visible_pixels\n";
    auto in=[](unsigned v,std::initializer_list<unsigned> vals){ return std::find(vals.begin(),vals.end(),v)!=vals.end(); };
    for(const auto& name:objects){
        if(name!="player_car") continue;
        const DebugSpriteInfo* best_body=nullptr; const DebugSpriteInfo* best_shadow=nullptr; int best_dist=1<<30;
        for(const auto& b:debug_sprites_){
            if(!in(b.map,{472u,475u,476u,508u,514u,516u}) || !in(b.palette,{64u,65u})) continue;
            for(const auto& sh:debug_sprites_){
                if(!in(sh.map,{572u,575u,576u,590u,592u,594u}) || sh.palette!=70u) continue;
                const int d=std::abs(b.x-sh.x)+std::abs(b.y-sh.y);
                if(d<best_dist){ best_dist=d; best_body=&b; best_shadow=&sh; }
            }
        }
        if(!best_body){ for(const auto& b:debug_sprites_) if(in(b.map,{472u,475u,476u,508u,514u,516u}) && in(b.palette,{64u,65u})) { best_body=&b; break; } }
        if(!best_shadow){ for(const auto& sh:debug_sprites_) if(in(sh.map,{572u,575u,576u,590u,592u,594u}) && sh.palette==70u) { best_shadow=&sh; break; } }
        if(!best_body && !best_shadow){ out<<frame<<','<<name<<",0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0\n"; continue; }
        int left=1<<30,top=1<<30,right=-(1<<30),bottom=-(1<<30); std::uint64_t vp=0;
        auto absorb=[&](const DebugSpriteInfo* q){ if(!q)return; left=std::min(left,q->x); top=std::min(top,q->y); right=std::max(right,q->x+q->width-1); bottom=std::max(bottom,q->y+q->height-1); vp+=q->visible_pixels; };
        absorb(best_body); absorb(best_shadow);
        const int x=best_body?best_body->x:best_shadow->x, y=best_body?best_body->y:best_shadow->y;
        out<<frame<<','<<name<<",1,"
           <<(best_body?(int)best_body->slot:-1)<<','<<(best_body?(int)best_body->map:-1)<<','<<(best_body?(int)best_body->palette:-1)<<','
           <<(best_shadow?(int)best_shadow->slot:-1)<<','<<(best_shadow?(int)best_shadow->map:-1)<<','<<(best_shadow?(int)best_shadow->palette:-1)<<','
           <<x<<','<<y<<','<<left<<','<<top<<','<<right<<','<<bottom<<','<<vp<<"\n";
    }
    return (bool)out;
}


void Video::update_sprite_change_events(const Runtime& runtime,unsigned frame){
    // Slot identity is stable enough for frame-to-frame provenance.  Record all
    // changes, tagging map/palette/visibility/shape changes as semantic so HUD
    // and animation changes can be separated from ordinary movement noise.
    auto find_slot=[](const std::vector<DebugSpriteInfo>& v,unsigned slot)->const DebugSpriteInfo*{
        for(const auto& q:v) if(q.slot==slot) return &q; return nullptr;
    };
    std::vector<unsigned> slots; slots.reserve(previous_debug_sprites_.size()+debug_sprites_.size());
    for(const auto& q:previous_debug_sprites_) slots.push_back(q.slot);
    for(const auto& q:debug_sprites_) if(std::find(slots.begin(),slots.end(),q.slot)==slots.end()) slots.push_back(q.slot);
    for(unsigned slot:slots){
        const auto* old=find_slot(previous_debug_sprites_,slot); const auto* now=find_slot(debug_sprites_,slot);
        if(!old && !now) continue;
        const bool ov=old && old->visible_pixels>0, nv=now && now->visible_pixels>0;
        const bool moved=old&&now&&(old->x!=now->x||old->y!=now->y);
        const bool semantic=(!old||!now)||old->map!=now->map||old->palette!=now->palette||old->priority!=now->priority||old->width!=now->width||old->height!=now->height||ov!=nv;
        if(!semantic && !moved) continue;
        SpriteChangeEvent ev{}; ev.frame=frame; ev.slot=slot; ev.old_map=old?old->map:0; ev.new_map=now?now->map:0; ev.old_palette=old?old->palette:0; ev.new_palette=now?now->palette:0;
        ev.old_x=old?old->x:0; ev.old_y=old?old->y:0; ev.new_x=now?now->x:0; ev.new_y=now?now->y:0; ev.old_visible=ov; ev.new_visible=nv; ev.semantic=semantic;
        const std::uint32_t start=0xd00000u+slot*8u, end=start+7u;
        for(auto it=runtime.bus.research_events().rbegin(); it!=runtime.bus.research_events().rend(); ++it){
            if(it->kind!=ResearchEventKind::Write && it->kind!=ResearchEventKind::SuppressedWrite) continue;
            const auto ee=it->address+it->size-1; if(it->space==BusSpace::Main && it->address<=end && ee>=start){ev.writer_pc=it->pc;break;}
        }
        sprite_change_events_.push_back(ev); while(sprite_change_events_.size()>256) sprite_change_events_.pop_front();
    }
    previous_debug_sprites_=debug_sprites_;
}

void Video::draw_research_workbench(std::vector<std::uint32_t>& pixels,const Runtime& runtime,unsigned frame,bool paused){
    if(!research_workbench_) return;
    for(auto& c:pixels){ unsigned r=((c>>16)&255)/7,g=((c>>8)&255)/7,b=(c&255)/7; c=0xff000000u|(r<<16)|(g<<8)|b; }
    debug_rect(pixels,1,1,WIDTH-2,HEIGHT-2,0xffb0b0b0u);
    auto& bus=const_cast<Bus&>(runtime.bus);
    auto mode_name=[](DebugUiActions::ResearchCommand m){using RC=DebugUiActions::ResearchCommand; switch(m){case RC::WriteOnce:return "WRITE ONCE";case RC::Freeze:return "FREEZE";case RC::Suppress:return "SUPPRESS";case RC::WatchWrite:return "WATCH WRITE";case RC::WatchChange:return "WATCH CHANGE";default:return "NONE";}};
    debug_text(pixels,5,5,"RESEARCH WORKBENCH  F12 CLOSE",0xff80ffffu);
    {std::ostringstream q;q<<"FRAME "<<frame<<"  "<<(paused?"PAUSED":"RUNNING")<<"  CPUA "<<std::hex<<std::uppercase<<bus.trace_pc(BusSpace::Main)<<" CPUB "<<bus.trace_pc(BusSpace::Sub);debug_text(pixels,5,15,q.str(),paused?0xffffff80u:0xff80ff80u);}
    debug_text(pixels,5,27,"SPACE RUN/PAUSE  F6 +1  F7 +5  F8 +30",0xffd8d8d8u);
    debug_text(pixels,5,36,"F3 STEP CPU-A  SHIFT+F3 CPU-B",0xffd8d8d8u);
    debug_text(pixels,5,45,"F5 SNAPSHOT  F9 RESTORE  F2 SHOT  F10 BUNDLE",0xffd8d8d8u);

    debug_text(pixels,5,59,"LIVE INTERVENTION",0xffffffffu);
    auto field=[&](unsigned idx,int y,const std::string& text){debug_text(pixels,7,y,(research_edit_field_==idx?"> ":"  ")+text,research_edit_field_==idx?0xffffff80u:0xffd8d8d8u);};
    field(0,69,"ADDR  0x"+research_address_hex_);
    field(1,78,"VALUE 0x"+(research_value_hex_.empty()?std::string("0"):research_value_hex_));
    field(2,87,"WIDTH "+std::to_string(research_width_*8)+" bit");
    field(3,96,std::string("CPU   ")+(research_space_==BusSpace::Main?"A / MAIN":"B / SUB"));
    field(4,105,std::string("MODE  ")+mode_name(research_mode_));
    debug_text(pixels,7,116,"TAB FIELD  LEFT/RIGHT OPTION  ENTER APPLY",0xffb0b0b0u);
    std::uint32_t addr=0; for(char c:research_address_hex_){addr<<=4;addr|=(c<='9'?c-'0':c-'A'+10);} addr&=0xffffffu;
    {auto cur=bus.peek(addr,research_width_,research_space_);std::ostringstream q;q<<"CURRENT 0x"<<std::hex<<std::uppercase<<std::setw(research_width_*2)<<std::setfill('0')<<cur;debug_text(pixels,7,126,q.str(),0xff80ff80u);}

    debug_text(pixels,166,59,"ACTIVE PATCHES",0xffffffffu);
    int y=69; unsigned shown=0; for(auto it=bus.research_patches().rbegin();it!=bus.research_patches().rend()&&shown<5;++it,++shown){std::ostringstream q;q<<"#"<<it->id<<" "<<(it->space==BusSpace::Main?'A':'B')<<":"<<std::hex<<std::uppercase<<it->address<<"="<<it->value<<" "<<(it->mode==LivePatchMode::Freeze?"FZ":it->mode==LivePatchMode::Replace?"REP":"SUP")<<" x"<<std::dec<<it->interceptions;debug_text(pixels,168,y,q.str(),0xffffc080u);y+=9;}
    if(!shown) debug_text(pixels,168,y,"(none)",0xff909090u);
    debug_text(pixels,166,116,"DEL PATCHES  SHIFT+DEL WATCHES",0xffb0b0b0u);
    debug_text(pixels,166,125,"CTRL+DEL EVENTS",0xffb0b0b0u);

    debug_text(pixels,5,143,"RECENT MEMORY EVENTS",0xffffffffu);
    y=153; shown=0; for(auto it=bus.research_events().rbegin();it!=bus.research_events().rend()&&shown<8;++it,++shown){
        const char* k=it->kind==ResearchEventKind::Read?"R":(it->kind==ResearchEventKind::Intervention?"PATCH":(it->kind==ResearchEventKind::SuppressedWrite?"SUP":"W"));
        std::ostringstream q;q<<"F"<<std::dec<<it->frame<<" "<<k<<" "<<(it->space==BusSpace::Main?'A':'B')<<":"<<std::hex<<std::uppercase<<it->address<<" "<<it->old_value<<">"<<it->new_value<<" @"<<it->pc;debug_text(pixels,7,y,q.str(),it->changed?0xff80ffffu:0xffb0b0b0u);y+=9;
    }
    if(!shown) debug_text(pixels,7,y,"No watched/intervention events yet",0xff909090u);

    debug_text(pixels,166,143,"SEMANTIC SPRITE CHANGES",0xffffffffu);
    y=153; shown=0; for(auto it=sprite_change_events_.rbegin();it!=sprite_change_events_.rend()&&shown<8;++it){if(!it->semantic)continue;++shown;std::ostringstream q;q<<"F"<<it->frame<<" S"<<it->slot<<" M"<<it->old_map<<">"<<it->new_map<<" P"<<unsigned(it->old_palette)<<">"<<unsigned(it->new_palette);if(it->writer_pc)q<<" @"<<std::hex<<std::uppercase<<it->writer_pc;debug_text(pixels,168,y,q.str(),0xffffc080u);y+=9;}
    if(!shown) debug_text(pixels,168,y,"No semantic changes yet",0xff909090u);
}

void Video::draw_script_prompt(std::vector<std::uint32_t>& pixels) {
    if(!script_prompt_active_) return;
    for(auto& c:pixels){ unsigned r=((c>>16)&255)/3,g=((c>>8)&255)/3,b=(c&255)/3; c=0xff000000u|(r<<16)|(g<<8)|b; }
    const int x0=24,y0=72,x1=WIDTH-25,y1=168;
    debug_rect(pixels,x0,y0,x1,y1,0xffd0d0d0u);
    debug_text(pixels,x0+8,y0+8,"RESEARCH SCRIPT INPUT",0xff80ffffu);
    std::string msg=script_prompt_message_;
    const std::size_t maxc=42; int y=y0+25;
    while(!msg.empty() && y<y1-28){ std::size_t n=std::min(maxc,msg.size()); if(n<msg.size()){auto sp=msg.rfind(' ',n);if(sp!=std::string::npos&&sp>8)n=sp;} auto line=msg.substr(0,n); debug_text(pixels,x0+8,y,line,0xffffffffu); msg.erase(0,n); while(!msg.empty()&&msg.front()==' ')msg.erase(msg.begin()); y+=10; }
    debug_text(pixels,x0+8,y1-20,"ENTER = CONTINUE   ESC = CANCEL",0xffffff80u);
}

void Video::draw_debug_overlay(std::vector<std::uint32_t>& pixels,const Runtime& runtime,unsigned frame,bool paused,bool timer_frozen){
    if(!debug_overlay_)return;
    // v0.59.4: paged overlay. Keep one compact right-hand panel rather than continually
    // appending rows until useful state falls off the 240-line display.
    const int panel_x=152;
    for(int y=0;y<HEIGHT;++y)for(int x=panel_x;x<WIDTH;++x){auto&c=pixels[(std::size_t)y*WIDTH+x];unsigned r=((c>>16)&255)/4,g=((c>>8)&255)/4,b=(c&255)/4;c=0xff000000u|(r<<16)|(g<<8)|b;}
    debug_rect(pixels,panel_x,0,WIDTH-1,HEIGHT-1,0xffb0b0b0u);
    auto& bus=const_cast<Bus&>(runtime.bus);
    const auto timer=bus.peek(0x100200,1,BusSpace::Main); const auto frac=bus.peek(0x100201,1,BusSpace::Main);
    static constexpr const char* pages[] = {"DRIVING","HANDLING","TARGET","SYSTEM"};
    std::ostringstream title; title<<"DEBUG "<<pages[debug_page_%4u]<<" "<<(paused?"PAUSE":"RUN");
    debug_text(pixels,panel_x+4,4,title.str(),paused?0xffffff80u:0xff80ff80u);
    std::ostringstream hdr; hdr<<"F"<<frame<<"  PAGE "<<(debug_page_%4u)+1<<"/4"; debug_text(pixels,panel_x+4,14,hdr.str(),0xffffffffu);
    std::ostringstream tim; tim<<"TIME "<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<timer<<":"<<std::setw(2)<<frac<<(timer_frozen?" FREEZE":""); debug_text(pixels,panel_x+4,24,tim.str(),0xffffff80u);
    debug_text(pixels,panel_x+4,34,"F11 NEXT  SHIFT+F11 PREV",0xffc0c0c0u);
    int yy=46; auto line=[&](const std::string&q,std::uint32_t col=0xffd8d8d8u){ if(yy<=226) debug_text(pixels,panel_x+4,yy,q,col); yy+=9; };

    const auto speed=bus.peek(0x100400,2,BusSpace::Main);
    const auto speed_internal=bus.peek(0x10041c,4,BusSpace::Main);
    const auto dist=bus.peek(0x102fc0,4,BusSpace::Main);
    const auto score=bus.peek(0x100408,2,BusSpace::Main);
    const auto steer=static_cast<std::int16_t>(bus.peek(0x100300,2,BusSpace::Main));
    const auto rf=bus.peek(0x10a048,1,BusSpace::Main);
    const auto cp=bus.peek(0x10080e,4,BusSpace::Sub); const unsigned bank=bus.peek(0x1021bc,1,BusSpace::Sub)&0x0f; const unsigned off=(cp&0x3e000u)>>10; const auto ca=0x109000u+bank*0x100u+(off&0xf8u);
    const int curve=static_cast<std::int8_t>(bus.peek(ca,1,BusSpace::Sub)); const int grade=static_cast<std::int8_t>(bus.peek(ca+1,1,BusSpace::Sub));
    const char* grade_name=grade>0?"UP":(grade<0?"DOWN":"LEVEL"); const char* road=(rf&0x04)?"OFF":((rf&0x06)==0x02?"EDGE":"INTERIOR");
    const auto bounds=bus.peek(0x10a05c,4,BusSpace::Main);
    const std::uint16_t left=static_cast<std::uint16_t>((bounds>>16)&0xffffu), right=static_cast<std::uint16_t>(bounds&0xffffu);
    const auto width_delta=static_cast<std::int16_t>(static_cast<std::uint16_t>(right-left));
    const int width=std::abs(int(width_delta)); const std::uint16_t centre=static_cast<std::uint16_t>(left+width_delta/2);
    const std::uint16_t car=static_cast<std::uint16_t>(bus.peek(0x10a044,2,BusSpace::Main));
    const auto err=static_cast<std::int16_t>(static_cast<std::uint16_t>(car-centre)); const double norm=width?double(err)/(double(width)*0.5):0.0;

    switch(debug_page_%4u){
    case 0: { // Driving / course geometry
        {std::ostringstream q;q<<"SPEED "<<std::hex<<std::uppercase<<speed<<" INT "<<std::dec<<speed_internal;line(q.str(),0xffffffffu);}
        {std::ostringstream q;q<<"DIST "<<std::fixed<<std::setprecision(1)<<(dist/256.0);line(q.str());}
        {std::ostringstream q;q<<"SCORE "<<std::hex<<std::uppercase<<score<<" STEER "<<std::dec<<steer;line(q.str());}
        {std::ostringstream q;q<<"ROAD "<<road<<" F"<<std::hex<<unsigned(rf);line(q.str(),(rf&0x04)?0xffff8080u:0xff80ff80u);}
        {std::ostringstream q;q<<"LEFT "<<std::hex<<left<<" RIGHT "<<right;line(q.str());}
        {std::ostringstream q;q<<"WIDTH "<<std::dec<<width<<" CENT "<<centre;line(q.str());}
        {std::ostringstream q;q<<"CAR "<<std::dec<<car<<" ERR "<<err;line(q.str());}
        {std::ostringstream q;q<<"NORM "<<std::fixed<<std::setprecision(3)<<norm;line(q.str(),std::abs(norm)>0.75?0xffff8080u:0xff80ff80u);}
        {std::ostringstream q;q<<"CURVE "<<curve<<" GRADE "<<grade_name<<" "<<grade;line(q.str(),0xffffff80u);}
        {std::ostringstream q;q<<"COURSE B"<<bank<<" R"<<std::dec<<((off&0xffu)/8u);line(q.str());}
        {std::ostringstream q;q<<"TURBO "<<(bus.peek(0x100212,2,BusSpace::Main)?"ON":"OFF")<<" LEFT "<<bus.peek(0x1003a2,2,BusSpace::Main);line(q.str());}
        {std::ostringstream q;q<<(assist_course_follow_?"FOLLOW ":"")<<(assist_infinite_time_?"TIME* ":"")<<(assist_unlimited_turbo_?"TURBO* ":"")<<(assist_auto_turbo_?"AUTO":"");line(q.str(),0xff80ffffu);}
        break; }
    case 1: { // Handling / cornering model
        const auto& hs=runtime.handling_snapshot();
        {std::ostringstream q;q<<"STEER "<<steer<<" CURVE "<<curve;line(q.str());}
        {std::ostringstream q;q<<"ROAD ERR "<<err<<" N "<<std::fixed<<std::setprecision(2)<<norm;line(q.str(),std::abs(norm)>0.75?0xffff8080u:0xff80ff80u);}
        if(hs.valid){
            {std::ostringstream q;q<<"TURN "<<hs.turn_state<<" IDX "<<hs.table_index;line(q.str(),0xff80ffffu);}
            {std::ostringstream q;q<<"NATIVE F "<<std::fixed<<std::setprecision(3)<<(double(static_cast<std::int16_t>(hs.forward_coeff))/256.0);line(q.str(),0xff80ffffu);}
            {std::ostringstream q;q<<"NATIVE L "<<std::fixed<<std::setprecision(3)<<(double(static_cast<std::int16_t>(hs.lateral_coeff))/256.0);line(q.str(),0xff80ffffu);}
            {std::ostringstream q;q<<"APPLY F "<<std::fixed<<std::setprecision(3)<<(double(static_cast<std::int16_t>(hs.applied_forward_coeff))/256.0);line(q.str(),hs.override_active?0xffffc080u:0xffd8d8d8u);}
            {std::ostringstream q;q<<"APPLY L "<<std::fixed<<std::setprecision(3)<<(double(static_cast<std::int16_t>(hs.applied_lateral_coeff))/256.0);line(q.str(),hs.override_active?0xffffc080u:0xffd8d8d8u);}
            {std::ostringstream q;q<<"COMP F "<<hs.forward_component;line(q.str());}
            {std::ostringstream q;q<<"COMP L "<<hs.lateral_component;line(q.str());}
            line(hs.override_active?"OVERRIDE ACTIVE":"NATIVE HANDLING",hs.override_active?0xffffc080u:0xff80ff80u);
        } else line("HANDLING: NO SAMPLE",0xffff8080u);
        {std::ostringstream q;q<<"SPEED INT "<<std::dec<<speed_internal;line(q.str());}
        line("BEST SURVEY SCALE 1.15",0xff80ff80u);
        break; }
    case 2: { // Target / defeat / collision research
        const auto target_status=bus.peek(0x10a089,1,BusSpace::Main);
        const auto target_speed=bus.peek(0x10a092,2,BusSpace::Main);
        const auto target_motion=bus.peek(0x10a096,2,BusSpace::Main);
        const auto target_pos=bus.peek(0x10a080,4,BusSpace::Main);
        const auto player_pos=bus.peek(0x10a040,4,BusSpace::Main);
        const std::int32_t separation=static_cast<std::int32_t>(target_pos-player_pos);
        const auto defeat_flags=bus.peek(0x10018d,1,BusSpace::Main);
        const auto defeat_timer=bus.peek(0x1002d0,2,BusSpace::Main);
        const auto interact_counter=bus.peek(0x1002ae,2,BusSpace::Main);
        const auto interact_timer=bus.peek(0x1002c6,2,BusSpace::Main);
        const auto interact_type=bus.peek(0x10042e,2,BusSpace::Main);
        {std::ostringstream q;q<<"TARGET POS "<<std::hex<<std::uppercase<<target_pos;line(q.str());}
        {std::ostringstream q;q<<"PLAYER POS "<<std::hex<<std::uppercase<<player_pos;line(q.str());}
        {std::ostringstream q;q<<"SEPARATION "<<std::dec<<separation;line(q.str());}
        {std::ostringstream q;q<<"OBJ FLAGS "<<std::hex<<std::uppercase<<target_status<<" CONTACT "<<((target_status&0x20)?"YES":"NO");line(q.str(),(target_status&0x20)?0xffff8080u:0xffd8d8d8u);}
        {std::ostringstream q;q<<"MOVE SPD "<<std::hex<<target_speed<<" CTRL "<<target_motion;line(q.str(),0xff80ffffu);}
        {std::ostringstream q;q<<"TYPE "<<std::dec<<interact_type<<" TIMER "<<interact_timer;line(q.str());}
        {std::ostringstream q;q<<"HIT/PHASE "<<std::hex<<std::uppercase<<interact_counter;line(q.str());}
        {std::ostringstream q;q<<"DEFEAT FLAGS "<<std::hex<<std::uppercase<<defeat_flags;line(q.str(),(defeat_flags&0x05)==0x05?0xffffc080u:0xffd8d8d8u);}
        {std::ostringstream q;q<<"DEFEAT TIMER "<<std::dec<<defeat_timer;line(q.str(),defeat_timer?0xffffc080u:0xffd8d8d8u);}
        line((defeat_flags&0x05)==0x05?"DEFEAT MODE CANDIDATE":"TARGET ACTIVE/UNKNOWN",(defeat_flags&0x05)==0x05?0xffffc080u:0xff80ff80u);
        line("S1 SPR 319-322 PAL152",0xffc0c0c0u);
        break; }
    default: { // System / controls
        {std::ostringstream q;q<<"CPU A "<<std::hex<<std::uppercase<<runtime.bus.trace_pc(BusSpace::Main);line(q.str(),0xffd0d0ffu);}
        {std::ostringstream q;q<<"CPU B "<<std::hex<<std::uppercase<<runtime.bus.trace_pc(BusSpace::Sub);line(q.str(),0xffd0d0ffu);}
        {std::ostringstream q;q<<"STATE SLOT "<<checkpoint_slot_<<" [0-9]";line(q.str(),0xffffff80u);}
        line("SPACE PAUSE   . STEP");
        line("SHIFT+. = 60 FRAMES");
        line("F2 SCREENSHOT");
        line("F3 GRAPHICS CAPTURE");
        line("F4 TIMER FREEZE");
        line("F5 SAVE   F9 LOAD");
        line("F6 60  F7 10  F8 01");
        line("F10 EVIDENCE BUNDLE");
        line("TAB/CLOCK SPRITE SELECT");
        line("F1 CLOSE DEBUG UI",0xffffff80u);
        break; }
    }
    if(selected_sprite_>=0){for(const auto&s:debug_sprites_)if((int)s.slot==selected_sprite_){std::ostringstream q;q<<"SEL S"<<s.slot<<" X"<<s.x<<" Y"<<s.y<<" Z"<<s.width<<"X"<<s.height;debug_text(pixels,4,4,q.str(),0xffffff00u);debug_rect(pixels,s.x,s.y,s.x+s.width-1,s.y+s.height-1,0xffffff00u);break;}}
    if(debug_mouse_x_>=0){std::ostringstream m;m<<"PIX "<<debug_mouse_x_<<","<<debug_mouse_y_;debug_text(pixels,4,14,m.str(),0xffffff00u);}
}

bool Video::save_current_screenshot(const std::filesystem::path& path) const {
    if(last_presented_pixels_.size()!=static_cast<std::size_t>(WIDTH*HEIGHT))return false;
    std::error_code ec;std::filesystem::create_directories(path.parent_path(),ec);return diag_save_png(path,WIDTH,HEIGHT,last_presented_pixels_);
}

bool Video::save_layered_snapshot(const std::filesystem::path& root, const Machine& machine, const Runtime& runtime, const SceneState& scene, unsigned frame) {
    std::error_code ec;
    std::filesystem::create_directories(root / "layers", ec);
    std::filesystem::create_directories(root / "sources", ec);
    if (ec) return false;

    const Region* tile_ram = find_region(runtime.bus, "tilemap");
    const int nonzero_column_words=tile_ram && tile_ram->bytes.size()>=0x10000 ? static_cast<int>(tc0100scn_nonzero_column_words(tile_ram->bytes.data()+0xe000)) : -1;
    const Region* cr = find_region(runtime.bus, "tile_control");
    const std::uint16_t ctrl6 = (cr && cr->bytes.size() >= 14) ? be16(cr->bytes, 12) : 0;
    const int bottom = (ctrl6 & 0x08) ? 1 : 0;
    const int upper = bottom ^ 1;
    const bool reference = scene.use_reference_mixer;
    const bool prom = !reference && scene.use_prom_mixer;

    auto blank_state = [&](std::uint32_t fill) {
        struct S { std::vector<std::uint32_t> px; std::vector<std::uint8_t> lm,pa,po,ps,rp; } q;
        q.px.assign(WIDTH*HEIGHT, fill); q.lm.assign(WIDTH*HEIGHT,0); q.pa.assign(WIDTH*HEIGHT,0);
        q.po.assign(WIDTH*HEIGHT,0); q.ps.assign(WIDTH*HEIGHT,0); q.rp.assign(WIDTH*HEIGHT,0); return q;
    };
    auto opaque_copy = [&](const std::vector<std::uint32_t>& in) {
        auto out=in;
        for(auto& p:out) p=0xff000000u | (p & 0x00ffffffu);
        return out;
    };
    auto contribution = [&](const std::vector<std::uint32_t>& before, const std::vector<std::uint32_t>& after) {
        std::vector<std::uint32_t> out(after.size(),0u);
        // The live SDL framebuffer is visually opaque even when a compositor write carries
        // alpha=0 in its packed pixel.  Contribution PNGs therefore encode every changed
        // output pixel as an opaque replacement colour.  This makes ordinary alpha-over
        // reconstruction lossless, including transitions to transparent-black/black.
        for (std::size_t i=0;i<after.size();++i)
            if (before[i]!=after[i]) out[i]=0xff000000u | (after[i] & 0x00ffffffu);
        return out;
    };
    auto save_source = [&](const char* name, auto draw) {
        auto q=blank_state(0u); draw(q); return diag_save_png(root/"sources"/name,WIDTH,HEIGHT,q.px);
    };
    bool ok=true;
    ok &= save_source("bg-bottom.png", [&](auto& q){ if(scene.show_tiles) draw_runtime_tile_layer(q.px,q.lm,machine,runtime.bus,bottom,reference?0x00:LAYER_BG_BOTTOM,true); });
    ok &= save_source("road.png", [&](auto& q){ if(scene.show_road) draw_runtime_road(q.px,q.lm,q.pa,q.po,q.ps,q.rp,machine,runtime.bus,prom,scene.prom_input_mask,reference); });
    ok &= save_source("bg-upper.png", [&](auto& q){ if(scene.show_tiles) draw_runtime_tile_layer(q.px,q.lm,machine,runtime.bus,upper,reference?0x01:LAYER_BG_UPPER,false); });
    ok &= save_source("sprites.png", [&](auto& q){ if(scene.show_sprites) draw_runtime_sprites(q.px,q.lm,q.pa,q.po,q.ps,machine,runtime.bus,prom,scene.prom_input_mask,reference,nullptr,nullptr,sprite_tie_break_higher_); });
    ok &= save_source("text-hud.png", [&](auto& q){ if(scene.show_text) draw_runtime_text_layer(q.px,q.lm,runtime.bus,reference?0x04:LAYER_TEXT); });

    auto q=blank_state(0xff101010u);
    std::vector<std::uint16_t> snapshot_owner(WIDTH*HEIGHT,0xffff), snapshot_coverage(WIDTH*HEIGHT,0);
    if (!scene.show_tiles) draw_sky(q.px,72);
    std::vector<std::string> order;
    std::vector<std::vector<std::uint32_t>> reconstruction_layers;
    unsigned stage=0;
    {
        std::ostringstream fn; fn<<std::setfill('0')<<std::setw(2)<<stage++<<"_base.png";
        auto base=opaque_copy(q.px);
        ok &= diag_save_png(root/"layers"/fn.str(),WIDTH,HEIGHT,base);
        order.push_back(fn.str());
        reconstruction_layers.push_back(std::move(base));
    }
    auto apply = [&](const char* logical, auto draw) {
        auto before=q.px; draw(q); auto d=contribution(before,q.px);
        std::ostringstream fn; fn<<std::setfill('0')<<std::setw(2)<<stage++<<"_"<<logical<<".png";
        ok &= diag_save_png(root/"layers"/fn.str(),WIDTH,HEIGHT,d); order.push_back(fn.str());
        reconstruction_layers.push_back(std::move(d));
    };
    auto sns=blank_state(0xff101010u);
    if(reference){
        if(scene.show_tiles) apply("bg-bottom",[&](auto& z){draw_runtime_tile_layer(z.px,z.lm,machine,runtime.bus,bottom,0x00,true);});
        if(scene.show_tiles) apply("bg-upper",[&](auto& z){draw_runtime_tile_layer(z.px,z.lm,machine,runtime.bus,upper,0x01,false);});
        if(scene.show_road) apply("road",[&](auto& z){draw_runtime_road(z.px,z.lm,z.pa,z.po,z.ps,z.rp,machine,runtime.bus,false,scene.prom_input_mask,true);});
        if(scene.show_text) apply("text-hud",[&](auto& z){draw_runtime_text_layer(z.px,z.lm,runtime.bus,0x04);});
        sns=q;
        if(scene.show_sprites) apply("sprites",[&](auto& z){draw_runtime_sprites(z.px,z.lm,z.pa,z.po,z.ps,machine,runtime.bus,false,scene.prom_input_mask,true,&snapshot_owner,&snapshot_coverage,sprite_tie_break_higher_);});
    }else{
        if(scene.show_tiles) apply("bg-bottom",[&](auto& z){draw_runtime_tile_layer(z.px,z.lm,machine,runtime.bus,bottom,LAYER_BG_BOTTOM,true);});
        if(scene.show_road) apply("road",[&](auto& z){draw_runtime_road(z.px,z.lm,z.pa,z.po,z.ps,z.rp,machine,runtime.bus,prom,scene.prom_input_mask,false);});
        if(scene.show_tiles) apply("bg-upper",[&](auto& z){draw_runtime_tile_layer(z.px,z.lm,machine,runtime.bus,upper,LAYER_BG_UPPER,false);});
        sns=q;
        if(scene.show_sprites) apply("sprites",[&](auto& z){draw_runtime_sprites(z.px,z.lm,z.pa,z.po,z.ps,machine,runtime.bus,prom,scene.prom_input_mask,false,&snapshot_owner,&snapshot_coverage,sprite_tie_break_higher_);});
        if(scene.show_text) apply("text-hud",[&](auto& z){draw_runtime_text_layer(z.px,z.lm,runtime.bus,LAYER_TEXT);});
    }
    const auto final_raw=q.px;
    const auto final=opaque_copy(final_raw);
    ok &= diag_save_png(root/"final.png",WIDTH,HEIGHT,final);
    // Sprite-stage ownership: 65535 means no sprite painted this pixel. Coverage
    // includes masked and occluded nonzero-pen candidates, not just the winner.
    std::ofstream ownership(root/"sprite-ownership.csv");
    ownership << "x,y,owner,coverage\n";
    for(int y=0;y<HEIGHT;++y) for(int x=0;x<WIDTH;++x) {
        const auto i=static_cast<std::size_t>(y*WIDTH+x);
        ownership<<x<<','<<y<<','<<snapshot_owner[i]<<','<<snapshot_coverage[i]<<'\n';
    }
    ok &= static_cast<bool>(ownership);

    // Self-verify the advertised alpha-over reconstruction before reporting success.
    // All contribution pixels are either fully transparent (no change) or fully opaque
    // replacement colours (change), so alpha-over is equivalent to replacement here.
    auto reconstructed=reconstruction_layers.front();
    for(std::size_t li=1; li<reconstruction_layers.size(); ++li)
        for(std::size_t i=0;i<reconstructed.size();++i)
            if((reconstruction_layers[li][i]>>24)!=0) reconstructed[i]=reconstruction_layers[li][i];
    std::size_t reconstruction_mismatches=0;
    for(std::size_t i=0;i<final.size();++i) if(reconstructed[i]!=final[i]) ++reconstruction_mismatches;
    ok &= (reconstruction_mismatches==0);

    // HUD isolation is source-aware: Chase H.Q.'s HUD is currently identified with the TC0100SCN text layer.
    auto nh=blank_state(0xff101010u); if(!scene.show_tiles) draw_sky(nh.px,72);
    if(reference){
        if(scene.show_tiles) draw_runtime_tile_layer(nh.px,nh.lm,machine,runtime.bus,bottom,0x00,true);
        if(scene.show_tiles) draw_runtime_tile_layer(nh.px,nh.lm,machine,runtime.bus,upper,0x01,false);
        if(scene.show_road) draw_runtime_road(nh.px,nh.lm,nh.pa,nh.po,nh.ps,nh.rp,machine,runtime.bus,false,scene.prom_input_mask,true);
        if(scene.show_sprites) draw_runtime_sprites(nh.px,nh.lm,nh.pa,nh.po,nh.ps,machine,runtime.bus,false,scene.prom_input_mask,true,nullptr,nullptr,sprite_tie_break_higher_);
    }else{
        if(scene.show_tiles) draw_runtime_tile_layer(nh.px,nh.lm,machine,runtime.bus,bottom,LAYER_BG_BOTTOM,true);
        if(scene.show_road) draw_runtime_road(nh.px,nh.lm,nh.pa,nh.po,nh.ps,nh.rp,machine,runtime.bus,prom,scene.prom_input_mask,false);
        if(scene.show_tiles) draw_runtime_tile_layer(nh.px,nh.lm,machine,runtime.bus,upper,LAYER_BG_UPPER,false);
        if(scene.show_sprites) draw_runtime_sprites(nh.px,nh.lm,nh.pa,nh.po,nh.ps,machine,runtime.bus,prom,scene.prom_input_mask,false,nullptr,nullptr,sprite_tie_break_higher_);
    }
    const auto nh_final=opaque_copy(nh.px);
    ok &= diag_save_png(root/"final-no-hud.png",WIDTH,HEIGHT,nh_final);
    auto hud=contribution(nh.px,final_raw); ok &= diag_save_png(root/"hud-only.png",WIDTH,HEIGHT,hud);

    // v0.66.7 individual-sprite forensic controls.  The exact reconstruction still uses
    // the aggregate sprite contribution layer; these cropped transparent assets are
    // exploratory views for hide/solo/opacity/provenance in Graphics Lab.
    if(!reference && scene.show_text) draw_runtime_text_layer(sns.px,sns.lm,runtime.bus,LAYER_TEXT);
    ok &= diag_save_png(root/"scene-no-sprites.png",WIDTH,HEIGHT,opaque_copy(sns.px));
    std::size_t scene_no_sprites_changed=0, sprite_owned_pixels=0;
    const auto sns_final=opaque_copy(sns.px);
    for(std::size_t i=0;i<final.size();++i) { if(final[i]!=sns_final[i]) ++scene_no_sprites_changed; if(snapshot_owner[i]!=0xffff) ++sprite_owned_pixels; }
    std::filesystem::create_directories(root/"sprites",ec);
    std::ofstream sj(root/"sprites"/"sprites.json");
    sj << "{\n  \"schema\": \"chq-frame-sprites-v1\",\n  \"frame\": " << frame << ",\n  \"sprites\": [\n";
    bool first_sprite=true; unsigned sprite_asset_count=0;
    const Region* snap_sr=find_region(runtime.bus,"sprites");
    if(snap_sr){
        for(std::size_t pos=0; pos*8+7<snap_sr->bytes.size(); ++pos){
            const auto off=pos*8; auto w0=be16(snap_sr->bytes,off),w1=be16(snap_sr->bytes,off+2),w2=be16(snap_sr->bytes,off+4),w3=be16(snap_sr->bytes,off+6);
            const auto tile=(std::uint16_t)(w3&0x7ff); if(!tile) continue;
            int zy=((w0>>9)&0x7f)+1,zx=(w1&0x7f)+1; auto fmt=Machine::format_for_zoomx(zx); if(fmt==SpriteFormat::Invalid) continue;
            int x=w2&0x1ff; if(x>0x140)x-=0x200; int y=sprite_visible_y(w0,VISIBLE_Y_START,layer_offset_sprites_.y); y += (128-zy); x += layer_offset_sprites_.x;
            SpriteInstance sp{x,y-(128-zy),zx,zy,tile,(std::uint8_t)((w1&0x7f80)>>7),(w2&0x4000)!=0,(w2&0x8000)!=0,(w1&0x8000)?1:0};
            auto addr=sprite_prom_address(sp,w2,scene.prom_input_mask); auto pe=mix_lut_[addr];
            std::vector<std::uint32_t> solo(WIDTH*HEIGHT,0u); std::vector<std::uint8_t> slm(WIDTH*HEIGHT,0),d8(WIDTH*HEIGHT,0); std::vector<std::uint16_t> own(WIDTH*HEIGHT,0xffff),cov(WIDTH*HEIGHT,0);
            draw_sprite(solo,machine,sp,&runtime.bus,&slm,LAYER_SPRITE,0,&d8,&d8,&d8,addr,pe.raw,PromSource::Sprite,&own,&cov,(std::uint16_t)pos);
            int minx=WIDTH,miny=HEIGHT,maxx=-1,maxy=-1; std::uint64_t pixels=0;
            for(int yy=0;yy<HEIGHT;++yy) for(int xx=0;xx<WIDTH;++xx){auto c=solo[(std::size_t)yy*WIDTH+xx]; if((c>>24)!=0){++pixels;minx=std::min(minx,xx);miny=std::min(miny,yy);maxx=std::max(maxx,xx);maxy=std::max(maxy,yy);}}
            if(!pixels) continue;
            const int cw=maxx-minx+1,ch=maxy-miny+1; std::vector<std::uint32_t> crop((std::size_t)cw*ch,0u);
            for(int yy=0;yy<ch;++yy) for(int xx=0;xx<cw;++xx) crop[(std::size_t)yy*cw+xx]=solo[(std::size_t)(miny+yy)*WIDTH+(minx+xx)];
            std::ostringstream fn; fn<<"slot_"<<std::setfill('0')<<std::setw(3)<<pos<<".png"; ok &= diag_save_png(root/"sprites"/fn.str(),cw,ch,crop);
            if(!first_sprite) sj << ",\n"; first_sprite=false;
            sj << "    {\"slot\":"<<pos<<",\"asset\":\"sprites/"<<fn.str()<<"\",\"x\":"<<minx<<",\"y\":"<<miny<<",\"width\":"<<cw<<",\"height\":"<<ch<<",\"map\":"<<tile<<",\"palette\":"<<(unsigned)sp.color<<",\"priority\":"<<sp.priority<<",\"zoomX\":"<<zx<<",\"zoomY\":"<<zy<<",\"flipX\":"<<(sp.flip_x?"true":"false")<<",\"flipY\":"<<(sp.flip_y?"true":"false")<<",\"promAddress\":"<<(unsigned)addr<<",\"promRaw\":"<<(unsigned)pe.raw<<",\"pixels\":"<<pixels<<"}";
            ++sprite_asset_count;
        }
    }
    sj << "\n  ],\n  \"count\": "<<sprite_asset_count<<"\n}\n"; ok &= static_cast<bool>(sj);

    // Preserve the complete live TC0110PCR state so colours can be correlated back
    // to hardware palette entries without requiring the emulator to still be running.
    std::ofstream pal(root/"palette.csv");
    pal<<"index,bank,pen,raw,write_count,first_write_frame,last_write_frame,last_write_pc\n";
    for(unsigned i=0;i<4096;++i) pal<<i<<','<<(i>>4)<<','<<(i&15)<<",0x"<<std::hex<<std::setw(4)<<std::setfill('0')<<runtime.bus.palette[i]<<std::dec<<','<<runtime.bus.palette_write_count[i]<<','<<runtime.bus.palette_first_write_frame[i]<<','<<runtime.bus.palette_last_write_frame[i]<<",0x"<<std::hex<<runtime.bus.palette_last_write_pc[i]<<std::dec<<'\n';
    ok &= static_cast<bool>(pal);

    std::ofstream meta(root/"reconstruction.json");
    meta<<"{\n  \"schema\": \"chq-frame-reconstruction-v2\",\n  \"build\": \""<<kNativeVersion<<"\",\n  \"frame\": "<<frame<<",\n"
        <<"  \"width\": "<<WIDTH<<", \"height\": "<<HEIGHT<<",\n  \"compositor\": \""<<(reference?"reference":(prom?"prom-mask":"legacy"))<<"\",\n"
        <<"  \"promInputMask\": "<<unsigned(scene.prom_input_mask)<<",\n  \"tileControl6\": "<<ctrl6<<",\n  \"hudModel\": \"tc0100scn-text-layer\",\n"
        <<"  \"hudModes\": {\"normal\":\"final.png\",\"hidden\":\"final-no-hud.png\",\"only\":\"hud-only.png\"},\n"
        <<"  \"offsets\": {\"bg0\":["<<layer_offset_bg0_.x<<','<<layer_offset_bg0_.y<<"],\"bg1\":["<<layer_offset_bg1_.x<<','<<layer_offset_bg1_.y<<"],\"text\":["<<layer_offset_text_.x<<','<<layer_offset_text_.y<<"],\"sprites\":["<<layer_offset_sprites_.x<<','<<layer_offset_sprites_.y<<"],\"road\":["<<layer_offset_road_.x<<','<<layer_offset_road_.y<<"]},\n"
        <<"  \"sourceRegions\": {\"background\":\"tilemap/tile_control\",\"road\":\"road\",\"sprites\":\"sprites + sprite graphics/spritemap\",\"textHud\":\"tilemap text plane\",\"palette\":\"TC0110PCR\"},\n"
        <<"  \"paletteState\": \"palette.csv\",\n"
        <<"  \"spriteForensics\": {\"purpose\":\"exploratory-per-sprite-control\",\"aggregateSpritesRemainAuthoritativeForExactReconstruction\":true,\"sceneNoSprites\":\"scene-no-sprites.png\",\"metadata\":\"sprites/sprites.json\",\"assetDirectory\":\"sprites/\",\"count\":"<<sprite_asset_count<<"},\n"
        <<"  \"reconstruction\": {\"method\":\"alpha-over-contribution-layers\",\"exact\":true,\"verifiedExact\":"<<(reconstruction_mismatches==0?"true":"false")<<",\"mismatchedPixels\":"<<reconstruction_mismatches<<",\"baseLayer\":\"layers/00_base.png\",\"layers\":[";
    for(std::size_t i=0;i<order.size();++i){if(i)meta<<',';meta<<"\"layers/"<<order[i]<<"\"";} meta<<"]},\n"
        <<"  \"sceneNoSprites\": {\"method\":\"authoritative pre-sprite compositor state plus subsequent text pass\",\"changedPixels\":"<<scene_no_sprites_changed<<",\"spriteOwnedPixels\":"<<sprite_owned_pixels<<"},\n"
        <<"  \"presentationOffsets\": {\"bg0\":["<<layer_offset_bg0_.x<<','<<layer_offset_bg0_.y<<"],\"bg1\":["<<layer_offset_bg1_.x<<','<<layer_offset_bg1_.y<<"],\"text\":["<<layer_offset_text_.x<<','<<layer_offset_text_.y<<"],\"sprites\":["<<layer_offset_sprites_.x<<','<<layer_offset_sprites_.y<<"],\"road\":["<<layer_offset_road_.x<<','<<layer_offset_road_.y<<"]},\n"
        <<"  \"tc0100XMode\": \""<<(tc0100scn_legacy_x_?"legacy":"corrected")<<"\",\n"
        <<"  \"columnScroll\": {\"supported\":false,\"address\":\"0xC0E000\",\"nonzeroWords\":"<<nonzero_column_words<<",\"zeroStateOnly\":true},\n"
        <<"  \"sources\": [\"sources/bg-bottom.png\",\"sources/road.png\",\"sources/bg-upper.png\",\"sources/sprites.png\",\"sources/text-hud.png\"],\n"
        <<"  \"notes\": [\"Contribution layers reproduce the selected compositor output in the recorded order.\",\"Raw source renders are independent forensic views and are not guaranteed to alpha-compose to the final frame.\"]\n}\n";
    return ok && static_cast<bool>(meta);
}

void Video::draw_runtime(
    const Machine& machine,
    const Runtime& runtime,
    const SceneState& scene,
    unsigned frame,
    bool paused,
    bool timer_frozen) {
    if (tc0100scn_trace_ && frame>=tc0100scn_trace_from_ && (!tc0100scn_trace_to_ || frame<=tc0100scn_trace_to_) && tc0100scn_trace_log_) {
        const Region* tr=find_region(runtime.bus,"tilemap"); const Region* cr=find_region(runtime.bus,"tile_control");
        if(tr && cr && tr->bytes.size()>=0x10000 && cr->bytes.size()>=0x10){
            std::uint16_t c[8]; for(int i=0;i<8;++i)c[i]=be16(cr->bytes,i*2);
            for(int layer=0;layer<2;++layer){ const int scrollx=-static_cast<std::int16_t>(layer?c[1]:c[0]); const int scrolly=-static_cast<std::int16_t>(layer?c[4]:c[3]); const std::size_t rb=layer?0xc400:0xc000; const auto& lo=layer?layer_offset_bg1_:layer_offset_bg0_;
                for(unsigned sy=tc0100scn_scan_from_;sy<=tc0100scn_scan_to_ && sy<HEIGHT;sy+=tc0100scn_scan_step_){ const int sample_y=(int)sy-lo.y; const int hw_y=sample_y+VISIBLE_Y_START; const std::size_t roff=rb+static_cast<std::size_t>((hw_y&0x1ff)*2); const std::int16_t rs=static_cast<std::int16_t>(be16(tr->bytes,roff)); int mx=tc0100scn_legacy_x_ ? (-lo.x+static_cast<std::int16_t>(layer?c[1]:c[0])+rs+16)&0x1ff : tc0100scn_source_x(-lo.x,layer?c[1]:c[0],rs); int my=tc0100scn_source_y(hw_y,layer?c[4]:c[3]); const bool flip=(c[7]&1)!=0; if(flip){mx=0x1ff-mx;my=0x1ff-my;}
                    tc0100scn_trace_log_<<frame<<','<<sy<<','<<hw_y<<','<<layer; for(auto v:c)tc0100scn_trace_log_<<",0x"<<std::hex<<v<<std::dec; tc0100scn_trace_log_<<','<<scrollx<<','<<scrolly<<",0x"<<std::hex<<roff<<std::dec<<','<<rs<<','<<mx<<','<<my<<','<<(mx>>3)<<','<<(my>>3)<<','<<lo.x<<','<<lo.y<<','<<(flip?1:0)<<'\n';
                    if(sy+tc0100scn_scan_step_<sy) break; } } tc0100scn_trace_log_.flush(); }
    }


    runtime_visual_frame_ = frame;
    std::vector<std::uint32_t> pixels(WIDTH * HEIGHT, 0xff101010u);
    std::vector<std::uint8_t> layer_mask(WIDTH * HEIGHT, 0);
    std::vector<std::uint8_t> prom_addr(WIDTH * HEIGHT, 0);
    std::vector<std::uint8_t> prom_out(WIDTH * HEIGHT, 0);
    std::vector<std::uint8_t> prom_source(WIDTH * HEIGHT, 0);
    std::vector<std::uint8_t> road_probe(WIDTH * HEIGHT, 0);
    std::vector<std::uint16_t> sprite_owner(WIDTH * HEIGHT, 0xffff);
    std::vector<std::uint16_t> sprite_coverage(WIDTH * HEIGHT, 0);

    sample_road_ram(runtime.bus, frame);

    const Region* cr = find_region(runtime.bus, "tile_control");
    const std::uint16_t ctrl6 = (cr && cr->bytes.size() >= 14) ? be16(cr->bytes, 12) : 0;
    const int bottom = (ctrl6 & 0x08) ? 1 : 0;
    const int upper = bottom ^ 1;

    unsigned sprites = 0;
    if (scene.use_reference_mixer) {
        // Chase H.Q. reference order from the established Taito Z renderer:
        // bottom BG(pri 0), upper BG(pri 1), TC0150ROD(pri 1/2), text(pri 4),
        // then back-to-front sprites using primasks {0xf0,0xfc}.
        if (scene.show_tiles)
            draw_runtime_tile_layer(pixels, layer_mask, machine, runtime.bus, bottom, 0x00, true);
        else
            draw_sky(pixels, 72);
        if (scene.show_tiles)
            draw_runtime_tile_layer(pixels, layer_mask, machine, runtime.bus, upper, 0x01, false);
        if (scene.show_road)
            draw_runtime_road(pixels, layer_mask, prom_addr, prom_out, prom_source, road_probe,
                              machine, runtime.bus, false, scene.prom_input_mask, true);
        if (scene.show_text)
            draw_runtime_text_layer(pixels, layer_mask, runtime.bus, 0x04);
        if (scene.show_sprites)
            sprites = draw_runtime_sprites(pixels, layer_mask, prom_addr, prom_out, prom_source,
                                           machine, runtime.bus, false, scene.prom_input_mask, true,
                                           &sprite_owner, &sprite_coverage, sprite_tie_break_higher_);
    } else {
        if (scene.show_tiles)
            draw_runtime_tile_layer(pixels, layer_mask, machine, runtime.bus, bottom, LAYER_BG_BOTTOM, true);
        else
            draw_sky(pixels, 72);
        if (scene.show_road)
            draw_runtime_road(pixels, layer_mask, prom_addr, prom_out, prom_source, road_probe,
                              machine, runtime.bus, scene.use_prom_mixer, scene.prom_input_mask, false);
        if (scene.show_tiles)
            draw_runtime_tile_layer(pixels, layer_mask, machine, runtime.bus, upper, LAYER_BG_UPPER, false);
        if (scene.show_sprites)
            sprites = draw_runtime_sprites(pixels, layer_mask, prom_addr, prom_out, prom_source,
                                           machine, runtime.bus, scene.use_prom_mixer, scene.prom_input_mask, false,
                                           &sprite_owner, &sprite_coverage, sprite_tie_break_higher_);
        if (scene.show_text)
            draw_runtime_text_layer(pixels, layer_mask, runtime.bus, LAYER_TEXT);
    }

    const auto compositor_pixels = pixels;
    if (diagnostics_frame(frame))
        export_runtime_diagnostics(machine, runtime, scene, frame, compositor_pixels, layer_mask, prom_addr, prom_out, prom_source, road_probe, sprite_owner, sprite_coverage);

    if (scene.show_road_ram_map)
        visualise_road_ram_map(pixels, runtime.bus);
    else if (scene.show_road_probe)
        visualise_road_probe(pixels, road_probe);
    else if (scene.show_prom_bits)
        visualise_prom_bits(pixels, prom_addr, prom_source, scene.prom_input_mask);
    else if (scene.show_prom_trace)
        visualise_prom_trace(pixels, prom_addr, prom_out, prom_source);
    else if (scene.show_priority)
        visualise_layer_mask(pixels, layer_mask);

    // v0.49.1 presentation-only diagnostic background. Preserve normal composition/priority
    // decisions, then replace non-sprite-owned final pixels for clean isolation.
    if (diagnostic_background_ != Options::DiagnosticBackground::None && !scene.show_road_ram_map && !scene.show_road_probe && !scene.show_prom_bits && !scene.show_prom_trace && !scene.show_priority) {
        for (std::size_t i=0;i<pixels.size();++i) if (sprite_owner[i]==0xffff) {
            if (diagnostic_background_ == Options::DiagnosticBackground::Black) pixels[i]=0xff000000u;
            else if (diagnostic_background_ == Options::DiagnosticBackground::White) pixels[i]=0xffffffffu;
            else { int x=(int)(i%WIDTH), y=(int)(i/WIDTH); bool light=((x/8)+(y/8))&1; pixels[i]=light?0xffd0d0d0u:0xff404040u; }
        }
    }

    rebuild_debug_sprite_list(runtime, sprite_owner);
    update_sprite_change_events(runtime, frame);
    if (research_workbench_) draw_research_workbench(pixels, runtime, frame, paused);
    else draw_debug_overlay(pixels, runtime, frame, paused, timer_frozen);
    draw_script_prompt(pixels);
    update_runtime_title(runtime, scene, frame, sprites, paused);
    present(pixels);
}

void Video::update_title(const SceneState& scene) {
    std::ostringstream ss;

    ss << kWindowProduct << " v" << kNativeVersion << " SYNTHETIC PREVIEW | "
       << "curve " << scene.curve
       << " | horizon " << scene.horizon
       << " | road phase " << scene.road_offset
       << " | road " << (scene.show_road ? "ON" : "OFF")
       << " | sprites " << (scene.show_sprites ? "ON" : "OFF");

    SDL_SetWindowTitle(window_, ss.str().c_str());
}

void Video::draw_scene(
    const Machine& machine,
    const SceneState& scene) {

    std::vector<std::uint32_t> pixels(
        WIDTH * HEIGHT,
        0xff000000u);

    draw_sky(pixels, scene.horizon);

    if (scene.show_road)
        draw_road(pixels, machine.road_gfx(), scene);

    if (scene.show_sprites) {
        // Far vehicle.
        draw_sprite(
            pixels,
            machine,
            SpriteInstance{
                144, 105,
                64, 52,
                0x0a2,
                0x31,
                false, false,
                0
            });

        // Road-side sign.
        draw_sprite(
            pixels,
            machine,
            SpriteInstance{
                228, 95,
                72, 72,
                0x11c,
                0x44,
                false, false,
                1
            });

        // Medium-distance car using the confirmed rear-quarter view.
        draw_sprite(
            pixels,
            machine,
            SpriteInstance{
                80, 118,
                96, 82,
                0x003,
                0x28,
                false, false,
                1
            });

        // Player/near car.
        draw_sprite(
            pixels,
            machine,
            SpriteInstance{
                96, 135,
                128, 118,
                0x001,
                0x20,
                false, false,
                2
            });
    }

    update_title(scene);
    present(pixels);
}

void Video::present(
    const std::vector<std::uint32_t>& pixels) {

    last_presented_pixels_ = pixels;
    SDL_UpdateTexture(
        texture_,
        nullptr,
        pixels.data(),
        WIDTH * static_cast<int>(sizeof(std::uint32_t)));

    SDL_SetRenderDrawColor(
        renderer_,
        0, 0, 0, 255);

    SDL_RenderClear(renderer_);
    SDL_RenderTexture(
        renderer_,
        texture_,
        nullptr,
        nullptr);

    SDL_RenderPresent(renderer_);
}

void Video::shutdown() {
    if (road_probe_log_) {
        static constexpr const char* field_names[RF_COUNT] = {"ctl", "xpos", "srcw", "shape"};
        road_probe_log_ << "# scanlines_sampled " << road_scanlines_sampled_ << "\n";
        for (std::size_t f = 0; f < RF_COUNT; ++f) {
            road_probe_log_ << "\n# FIELD " << field_names[f] << " bit high toggle\n";
            for (int b = 0; b < 16; ++b)
                road_probe_log_ << "bit" << b << " " << road_field_high_[f][b] << " " << road_field_toggle_[f][b] << "\n";

            road_probe_log_ << "# " << field_names[f] << " low-byte histogram value count\n";
            for (unsigned v = 0; v < 256; ++v)
                if (road_field_low_hist_[f][v])
                    road_probe_log_ << "0x" << std::hex << std::setw(2) << std::setfill('0') << v
                                    << std::dec << " " << road_field_low_hist_[f][v] << "\n";
            road_probe_log_ << "# " << field_names[f] << " high-byte histogram value count\n";
            for (unsigned v = 0; v < 256; ++v)
                if (road_field_high_hist_[f][v])
                    road_probe_log_ << "0x" << std::hex << std::setw(2) << std::setfill('0') << v
                                    << std::dec << " " << road_field_high_hist_[f][v] << "\n";
        }

        road_probe_log_ << "\n# PEN HIST source kind pen count\n";
        for (int src = 0; src < 2; ++src)
            for (int kind = 0; kind < 2; ++kind)
                for (int pen = 0; pen < 4; ++pen)
                    road_probe_log_ << (src ? "roadB" : "roadA") << " " << (kind ? "edge" : "body")
                                    << " " << pen << " " << road_pen_hist_[src][kind][pen] << "\n";

        static constexpr const char* sig_names[ROAD_SIGNAL_COUNT] = {
            "pen0", "pen1", "roadB", "edge", "right_side", "ctl0", "ctl1", "ctl4", "ctl5", "ctl7", "ctl8", "ctl9"
        };
        road_probe_log_ << "\n# SIGNAL high toggle\n";
        for (std::size_t i = 0; i < ROAD_SIGNAL_COUNT; ++i)
            road_probe_log_ << sig_names[i] << " " << road_signal_high_[i] << " " << road_signal_toggle_[i] << "\n";

        auto dump_candidate = [this](const char* name, const std::array<std::uint64_t,256>& hist) {
            std::uint64_t total = 0; unsigned unique = 0;
            for (auto v : hist) { total += v; if (v) ++unique; }
            road_probe_log_ << "\n# CANDIDATE " << name << " total=" << total << " unique=" << unique << "\n";
            road_probe_log_ << "# address count (probe-only; NOT used by mixer)\n";
            for (unsigned a = 0; a < 256; ++a)
                if (hist[a])
                    road_probe_log_ << "0x" << std::hex << std::setw(2) << std::setfill('0') << a
                                    << std::dec << " " << hist[a] << "\n";
        };
        dump_candidate("roadA", road_candidate_a_hist_);
        dump_candidate("roadB", road_candidate_b_hist_);
        road_probe_log_.close();
    }

    if (prom_log_) {
        auto dump_stats = [this](const char* name, const std::array<std::uint64_t,256>& hist,
                                 const std::array<std::uint64_t,8>& highs, const std::array<std::uint64_t,8>& toggles) {
            std::uint64_t total = 0; unsigned unique = 0;
            for (auto v : hist) { total += v; if (v) ++unique; }
            prom_log_ << "\n# STATS " << name << " total=" << total << " unique=" << unique << "\n# bit high toggle\n";
            for (int b = 0; b < 8; ++b) prom_log_ << "bit" << b << " " << highs[b] << " " << toggles[b] << "\n";
            prom_log_ << "# histogram address count raw\n";
            for (unsigned a = 0; a < 256; ++a) if (hist[a]) {
                const auto raw = (name[0] == 's') ? mix_lut_[a].raw : road_lut_[a].raw;
                prom_log_ << "0x" << std::hex << std::setw(2) << std::setfill('0') << a
                          << std::dec << " " << hist[a] << " 0x" << std::hex << std::setw(2) << static_cast<unsigned>(raw) << std::dec << "\n";
            }
        };
        dump_stats("roadA", road_a_hist_, road_a_bit_high_, road_a_bit_toggle_);
        dump_stats("roadB", road_b_hist_, road_b_bit_high_, road_b_bit_toggle_);
        dump_stats("sprite", mix_hist_, mix_bit_high_, mix_bit_toggle_);
        prom_log_.close();
    }

    if (road_layout_log_) {
        road_layout_log_ << "# snapshots " << road_snapshots_ << "\n";
        std::uint64_t total_writes = 0, total_changes = 0;
        std::size_t active_words = 0, nonzero_words = 0;
        for (std::size_t w = 0; w < ROAD_RAM_WORDS; ++w) {
            total_writes += road_last_write_counts_[w];
            total_changes += road_frame_changes_[w];
            if (road_last_write_counts_[w] || road_frame_changes_[w]) ++active_words;
            if (road_prev_be_[w]) ++nonzero_words;
        }
        std::uint64_t total_write_value_changes = 0, total_nonzero_write_events = 0;
        for (std::size_t w = 0; w < ROAD_RAM_WORDS; ++w) {
            total_write_value_changes += road_write_value_changes_[w];
            total_nonzero_write_events += road_nonzero_write_events_[w];
        }
        road_layout_log_ << "# total_cpuB_word_write_events " << total_writes << "\n"
                         << "# total_write_value_changes " << total_write_value_changes << "\n"
                         << "# total_nonzero_write_events " << total_nonzero_write_events << "\n"
                         << "# total_frame_value_changes " << total_changes << "\n"
                         << "# active_words " << active_words << " / " << ROAD_RAM_WORDS << "\n"
                         << "# final_nonzero_words " << nonzero_words << "\n";

        struct Row { std::size_t w; std::uint64_t score; };
        std::vector<Row> rows;
        rows.reserve(ROAD_RAM_WORDS);
        for (std::size_t w = 0; w < ROAD_RAM_WORDS; ++w) {
            const auto score = road_last_write_counts_[w] + road_frame_changes_[w] * 1024;
            if (score) rows.push_back({w, score});
        }
        std::sort(rows.begin(), rows.end(), [](const Row& a, const Row& b) { return a.score > b.score; });
        road_layout_log_ << "\n# TOP ACTIVE WORDS byte_offset writes write_changes nonzero_writes frame_changes nonzero_frames writeMinBE writeMaxBE writeLastBE snapMinBE snapMaxBE finalBE\n";
        const std::size_t topn = std::min<std::size_t>(128, rows.size());
        for (std::size_t i = 0; i < topn; ++i) {
            const auto w = rows[i].w;
            road_layout_log_ << "0x" << std::hex << std::setw(4) << std::setfill('0') << (w * 2)
                             << std::dec << " " << road_last_write_counts_[w]
                             << " " << road_write_value_changes_[w]
                             << " " << road_nonzero_write_events_[w]
                             << " " << road_frame_changes_[w]
                             << " " << road_nonzero_frames_[w]
                             << " 0x" << std::hex << std::setw(4) << road_write_min_be_[w]
                             << " 0x" << std::setw(4) << road_write_max_be_[w]
                             << " 0x" << std::setw(4) << road_write_last_be_[w]
                             << " 0x" << std::setw(4) << road_min_be_[w]
                             << " 0x" << std::setw(4) << road_max_be_[w]
                             << " 0x" << std::setw(4) << road_prev_be_[w] << std::dec << "\n";
        }

        road_layout_log_ << "\n# 0x100-BYTE BLOCK ACTIVITY block byte_start writes frame_changes final_nonzero_words\n";
        for (std::size_t block = 0; block < ROAD_RAM_WORDS / 0x80; ++block) {
            const std::size_t w0 = block * 0x80;
            std::uint64_t wr = 0, ch = 0; std::size_t nz = 0;
            for (std::size_t w = w0; w < w0 + 0x80; ++w) {
                wr += road_last_write_counts_[w]; ch += road_frame_changes_[w]; if (road_prev_be_[w]) ++nz;
            }
            road_layout_log_ << std::dec << block << " 0x" << std::hex << std::setw(4) << std::setfill('0') << (w0 * 2)
                             << std::dec << " " << wr << " " << ch << " " << nz << "\n";
        }

        auto dump_mod = [this](std::size_t period, const char* label) {
            std::vector<std::pair<std::uint64_t,std::size_t>> lanes;
            lanes.reserve(period);
            for (std::size_t lane = 0; lane < period; ++lane) {
                std::uint64_t score = 0;
                for (std::size_t w = lane; w < ROAD_RAM_WORDS; w += period)
                    score += road_frame_changes_[w] * 1024 + road_last_write_counts_[w];
                if (score) lanes.push_back({score,lane});
            }
            std::sort(lanes.begin(), lanes.end(), [](auto a, auto b) { return a.first > b.first; });
            road_layout_log_ << "\n# " << label << " TOP LANES lane score (tests repeating scanline structures)\n";
            for (std::size_t i = 0; i < std::min<std::size_t>(64, lanes.size()); ++i)
                road_layout_log_ << lanes[i].second << " " << lanes[i].first << "\n";
        };
        dump_mod(0x100, "MOD256-WORD");
        dump_mod(0x200, "MOD512-WORD");

        road_layout_log_ << "\n# FINAL RAW WORDS BY 0x200-BYTE BANK; rows are y=0..255, columns bank0..bank15\n";
        for (int y = 0; y < 256; ++y) {
            road_layout_log_ << "y=" << std::dec << y;
            for (int bank = 0; bank < 16; ++bank) {
                const std::size_t off = static_cast<std::size_t>(bank) * 0x200 + static_cast<std::size_t>(y) * 2;
                if (off + 1 >= road_last_bytes_.size()) break;
                const std::uint16_t v = static_cast<std::uint16_t>((road_last_bytes_[off] << 8) | road_last_bytes_[off+1]);
                road_layout_log_ << " " << std::hex << std::setw(4) << std::setfill('0') << v;
            }
            road_layout_log_ << std::dec << "\n";
        }
        road_layout_log_.close();

        if (!log_directory_.empty()) {
            std::ofstream raw(log_directory_ / "road_ram_final.bin", std::ios::binary | std::ios::trunc);
            raw.write(reinterpret_cast<const char*>(road_last_bytes_.data()), static_cast<std::streamsize>(road_last_bytes_.size()));
        }
    }

    if (texture_)
        SDL_DestroyTexture(texture_);

    if (renderer_)
        SDL_DestroyRenderer(renderer_);

    if (window_)
        SDL_DestroyWindow(window_);

    texture_ = nullptr;
    renderer_ = nullptr;
    window_ = nullptr;

    SDL_Quit();
}

}
