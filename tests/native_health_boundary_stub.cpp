// Synthetic test-only ABI provider, NOT a reconstructed or ROM-verified function.
// Its sole purpose is to exercise live-hook dispatch and bus-trace attribution.
#include "native_health_abi.h"
#if defined(_WIN32)
#define TEST_EXPORT extern "C" __declspec(dllexport)
#else
#define TEST_EXPORT extern "C" __attribute__((visibility("default")))
#endif
TEST_EXPORT int chq_native_health_v1(const chq::HealthFunctionInput* in, chq::HealthFunctionOutput* out) {
    if (!in || !out || in->gate != 1 || in->hits != 14 || in->score != 0 || in->increment != 0x155)
        return 0;
    out->d0 = (in->d0 & 0xffff0000u) | in->increment;
    out->sp = in->sp + 4;
    out->pc = in->return_pc;
    out->returned = 1;
    out->sr = static_cast<std::uint16_t>(in->sr & ~0x001fu); // fixture-specific NZVC/X for positive sum
    out->hits = in->hits;
    out->score = in->increment;
    return 1;
}
