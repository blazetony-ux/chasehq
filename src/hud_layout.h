#pragma once

#include <array>
#include <cctype>
#include <cstddef>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>

namespace chq {

// v0.66.9.4-RC1 presentation-only semantic HUD controls.
// These source-tile groups are mapped from all seven canonical Stage-1 gameplay
// checkpoints in v0.66.9.3. They intentionally do not mutate TC0100SCN RAM.
enum class HudElement : unsigned {
    HighScore = 0,
    Time,
    Distance,
    Score,
    Speed,
    Turbo,
    Stage,
    Count,
    None = 0xffu
};

struct HudElementPresentation {
    bool visible = true;
    int dx = 0;
    int dy = 0;
};

struct HudLayoutState {
    std::array<HudElementPresentation, static_cast<std::size_t>(HudElement::Count)> elements{};
};

inline constexpr std::array<const char*, static_cast<std::size_t>(HudElement::Count)> kHudElementNames{
    "high-score", "time", "distance", "score", "speed", "turbo", "stage"
};

inline const char* hud_element_name(HudElement e) noexcept {
    const auto i = static_cast<std::size_t>(e);
    return i < kHudElementNames.size() ? kHudElementNames[i] : "none";
}

inline std::string hud_normalize_name(std::string_view in) {
    std::string out;
    out.reserve(in.size());
    for (char c : in) {
        if (c == '_' || c == ' ') c = '-';
        out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }
    if (out == "highscore") out = "high-score";
    if (out == "stage-credit" || out == "stage/credit" || out == "credit") out = "stage";
    if (out == "turbos" || out == "turbo-markers" || out == "turbo-icons") out = "turbo";
    return out;
}

inline std::optional<HudElement> hud_element_from_name(std::string_view name) {
    const auto n = hud_normalize_name(name);
    for (std::size_t i = 0; i < kHudElementNames.size(); ++i)
        if (n == kHudElementNames[i]) return static_cast<HudElement>(i);
    return std::nullopt;
}

// Source-cell map for the Chase H.Q. TC0100SCN text plane.
// At canonical Stage-1 gameplay ctrl2=ctrl5=0: screen tile (c,r) maps to
// source tile (c+2,r+1). Grouping on source cells therefore remains stable
// even when presentation offsets move an element on screen.
inline HudElement hud_element_for_text_cell(int tx, int ty) noexcept {
    // HIGH SCORE label (source row 2, cols 3..12) + value (row 3, cols 11..17)
    if ((ty == 2 && tx >= 3 && tx <= 12) || (ty == 3 && tx >= 11 && tx <= 17))
        return HudElement::HighScore;
    // TIME label + 4x3 large-digit block.
    if ((ty == 2 && tx >= 20 && tx <= 23) || (ty >= 3 && ty <= 5 && tx >= 20 && tx <= 23))
        return HudElement::Time;
    // DISTANCE label and four-character value on the same row.
    if (ty == 2 && tx >= 26 && tx <= 38)
        return HudElement::Distance;
    // SCORE label + right-aligned seven-character value field.
    if ((ty == 4 && tx >= 3 && tx <= 7) || (ty == 5 && tx >= 11 && tx <= 17))
        return HudElement::Score;
    // SPEED label + complete observed destination text-cell footprint for the
    // changing numeric speed / km/h display. RC3.1.1 suppressed only columns
    // 36..41, which is sufficient while stationary but misses live digits that
    // expand left into columns 32..35 once speed changes. Suppress the complete
    // destination block while authentic character-RAM generation continues.
    if ((ty == 4 && tx >= 26 && tx <= 30) || (ty >= 4 && ty <= 5 && tx >= 32 && tx <= 41))
        return HudElement::Speed;
    // Three 2x2 turbo markers at the lower-left.
    if (ty >= 29 && ty <= 30 && tx >= 3 && tx <= 8)
        return HudElement::Turbo;
    // STAGE 1 during gameplay; CREDIT 0 in attract mode uses the same cells.
    if (ty == 29 && tx >= 34 && tx <= 40)
        return HudElement::Stage;
    return HudElement::None;
}

inline bool hud_layout_active(const HudLayoutState& state) noexcept {
    for (const auto& e : state.elements)
        if (!e.visible || e.dx != 0 || e.dy != 0) return true;
    return false;
}

inline void hud_layout_reset(HudLayoutState& state) noexcept {
    for (auto& e : state.elements) e = HudElementPresentation{};
}

inline bool hud_layout_apply_preset(HudLayoutState& state, std::string_view preset) {
    const auto p = hud_normalize_name(preset);
    hud_layout_reset(state);
    if (p == "authentic" || p == "default" || p == "reset") return true;
    if (p == "none" || p == "off" || p == "no-hud") {
        for (auto& e : state.elements) e.visible = false;
        return true;
    }
    if (p == "minimal") {
        for (auto& e : state.elements) e.visible = false;
        state.elements[static_cast<std::size_t>(HudElement::Time)].visible = true;
        state.elements[static_cast<std::size_t>(HudElement::Distance)].visible = true;
        state.elements[static_cast<std::size_t>(HudElement::Turbo)].visible = true;
        return true;
    }
    return false;
}

inline std::string hud_layout_status(const HudLayoutState& state) {
    std::ostringstream o;
    o << "active=" << (hud_layout_active(state) ? 1 : 0);
    for (std::size_t i = 0; i < kHudElementNames.size(); ++i) {
        const auto& e = state.elements[i];
        o << ' ' << kHudElementNames[i] << '=' << (e.visible ? 1 : 0) << ',' << e.dx << ',' << e.dy;
    }
    return o.str();
}

} // namespace chq
