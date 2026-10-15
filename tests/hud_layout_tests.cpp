#include "hud_layout.h"

#include <iostream>
#include <stdexcept>

static void check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

int main() {
    try {
        using chq::HudElement;
        std::cout << "[test] semantic HUD cell map\n";
        check(chq::hud_element_for_text_cell(3,2)==HudElement::HighScore, "HIGH SCORE label start");
        check(chq::hud_element_for_text_cell(17,3)==HudElement::HighScore, "HIGH SCORE value end");
        check(chq::hud_element_for_text_cell(20,2)==HudElement::Time, "TIME label");
        check(chq::hud_element_for_text_cell(23,5)==HudElement::Time, "TIME digit block");
        check(chq::hud_element_for_text_cell(26,2)==HudElement::Distance, "DISTANCE label");
        check(chq::hud_element_for_text_cell(38,2)==HudElement::Distance, "DISTANCE value");
        check(chq::hud_element_for_text_cell(3,4)==HudElement::Score, "SCORE label");
        check(chq::hud_element_for_text_cell(17,5)==HudElement::Score, "SCORE value");
        check(chq::hud_element_for_text_cell(26,4)==HudElement::Speed, "SPEED label");
        check(chq::hud_element_for_text_cell(32,4)==HudElement::Speed, "SPEED destination block start");
        check(chq::hud_element_for_text_cell(35,5)==HudElement::Speed, "SPEED live-digit extension");
        check(chq::hud_element_for_text_cell(41,5)==HudElement::Speed, "SPEED destination block end");
        check(chq::hud_element_for_text_cell(31,5)==HudElement::None, "SPEED overclaimed cell before destination block");
        check(chq::hud_element_for_text_cell(42,5)==HudElement::None, "SPEED overclaimed cell after destination block");
        check(chq::hud_element_for_text_cell(3,29)==HudElement::Turbo, "turbo markers");
        check(chq::hud_element_for_text_cell(40,29)==HudElement::Stage, "stage/credit field");
        check(chq::hud_element_for_text_cell(18,2)==HudElement::None, "gap incorrectly claimed");

        std::cout << "[test] HUD presets and naming\n";
        chq::HudLayoutState state{};
        check(!chq::hud_layout_active(state), "default layout should be authentic");
        check(chq::hud_layout_apply_preset(state,"minimal"), "minimal preset rejected");
        check(chq::hud_layout_active(state), "minimal preset should be custom");
        check(state.elements[static_cast<std::size_t>(HudElement::Time)].visible, "minimal TIME hidden");
        check(state.elements[static_cast<std::size_t>(HudElement::Distance)].visible, "minimal DISTANCE hidden");
        check(state.elements[static_cast<std::size_t>(HudElement::Turbo)].visible, "minimal TURBO hidden");
        check(!state.elements[static_cast<std::size_t>(HudElement::HighScore)].visible, "minimal HIGH SCORE visible");
        check(chq::hud_layout_apply_preset(state,"none"), "none preset rejected");
        for (const auto& e : state.elements) check(!e.visible, "none preset left element visible");
        check(chq::hud_layout_apply_preset(state,"authentic"), "authentic preset rejected");
        check(!chq::hud_layout_active(state), "authentic preset did not reset");
        check(chq::hud_element_from_name("high_score")==HudElement::HighScore, "high_score alias");
        check(chq::hud_element_from_name("turbo-icons")==HudElement::Turbo, "turbo-icons alias");
        check(!chq::hud_element_from_name("unknown"), "unknown HUD element accepted");
        std::cout << "hud_layout_tests PASS\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "hud_layout_tests FAIL: " << e.what() << "\n";
        return 1;
    }
}
