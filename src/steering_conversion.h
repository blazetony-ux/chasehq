#pragma once
#include <cstdint>
namespace chasehq::native {
// This models ONLY the verified CPU-A normal lookup branch, not complete steering physics.
struct SteeringSample { int16_t raw; int16_t mode_gate; };
struct SteeringResult { int16_t output; bool verified_mode; };
// CPU-A 0x8206 EXT.W D1 sign-extends the low BYTE before the ROM lookup.
// 0x0FE0 and 0xFFE0 therefore both address the -32 entry (0x312EC).
constexpr int16_t steering_ext_w_index(int16_t raw) noexcept {
  const int16_t low = static_cast<int16_t>(static_cast<uint16_t>(raw) & 0x00ffu);
  return (low & 0x80) != 0 ? static_cast<int16_t>(low - 0x100) : low;
}
int16_t convert_steering_normal(int16_t signed_input) noexcept;
SteeringResult convert_steering(SteeringSample sample) noexcept;
constexpr uint32_t kSteeringRomTableBase = 0x3132C;
}
