#pragma once
#include <cstdint>
// Versioned, POD-only host/generated-code boundary. No C++ allocation or
// machine pointers cross the shared library ABI.
namespace chq {
struct HealthFunctionInput {
    std::uint32_t d0, a5, sp, return_pc;
    std::uint16_t sr, gate, hits, increment, score;
};
struct HealthFunctionOutput {
    std::uint32_t d0, sp, pc, returned;
    std::uint16_t sr, hits, score;
};
using NativeHealthFunction = int (*)(const HealthFunctionInput*, HealthFunctionOutput*);
}
