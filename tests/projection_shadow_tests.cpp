#include "projection_shadow.h"
#include <cassert>
#include <cstdint>
#include <string>

int main() {
    using chq::native_projection_decide;
    const auto near = native_projection_decide(0x158a07,0x158b87); // Run 021 inclusive boundary
    assert(near.accepted && near.separation == -0x180 && near.table_record == 0x54040);
    assert(!native_projection_decide(0x158a06,0x158b87).accepted);
    assert(native_projection_decide(0x160a0f,0x158a10).table_record == 0x580f8); // Run 022 +0x7fff
    assert(!native_projection_decide(0x160a10,0x158a10).accepted); // Run 022 +0x8000
    assert(native_projection_decide(0x158e22,0x158b87).table_record == 0x54248); // Run 017 control +0x29b
    assert(!native_projection_decide(0,0).accepted);
    assert(!native_projection_decide(0xffffffff,0).accepted);
    assert(native_projection_decide(0x1000,0x1000).accepted);
    assert(native_projection_decide(0x1000,0x1180).accepted);
    assert(!native_projection_decide(0x1000,0x1181).accepted);
    chq::ProjectionShadow shadow;
    assert(!shadow.enabled());
    shadow.enable();
    shadow.entry(0x158a07,0x158b87,5587);shadow.lookup(0x54040,5587);
    shadow.entry(0x158a06,0x158b87,5587);shadow.exit(5587);
    auto report = shadow.report_json();
    assert(report.find("\"matched\":2") != std::string::npos);
    shadow.entry(0x158a07,0x158b87,5587);shadow.lookup(0x54048,5587);
    assert(shadow.report_json().find("\"mismatched\":1") != std::string::npos);
    shadow.entry(0x158a07,0x158b87,5587);
    assert(shadow.report_json().find("\"unresolved\":1") != std::string::npos);
    shadow.reset();assert(shadow.report_json().find("\"entries\":0") != std::string::npos);
}
