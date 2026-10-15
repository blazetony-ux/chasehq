#include "steering_conversion.h"
#include <array>
#include <algorithm>
namespace chasehq::native {
namespace {
// Literal independent table extracted from CPU-A ROM; no emulator CPU calls.
constexpr std::array<int16_t, 193> kTable = {{
  -96, -96, -96, -96, -96, -96, -96, -96, -94, -92, -90, -88, -86, -84, -82, -80,
  -78, -76, -74, -72, -70, -68, -66, -64, -63, -61, -59, -57, -56, -54, -52, -51,
  -49, -48, -46, -45, -43, -42, -40, -39, -37, -36, -35, -33, -32, -31, -30, -28,
  -27, -26, -25, -24, -23, -22, -21, -20, -19, -18, -17, -16, -15, -14, -13, -12,
  -12, -11, -10, -9, -9, -8, -7, -7, -6, -6, -5, -5, -4, -4, -3, -3,
  -2, -2, -2, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2,
  3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 9, 9, 10, 11, 12,
  12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,
  28, 30, 31, 32, 33, 35, 36, 37, 39, 40, 42, 43, 45, 46, 48, 49,
  51, 52, 54, 56, 57, 59, 61, 63, 64, 66, 68, 70, 72, 74, 76, 78,
  80, 82, 84, 86, 88, 90, 92, 94, 96, 96, 96, 96, 96, 96, 96, 96,
  96
}};
}
int16_t convert_steering_normal(int16_t signed_input) noexcept {
  const int index = std::clamp(static_cast<int>(signed_input), -96, 96);
  return kTable[static_cast<unsigned>(index + 96)];
}
SteeringResult convert_steering(SteeringSample sample) noexcept {
  // Fail closed: do not claim the unverified nonzero-gate branch.
  if (sample.mode_gate != 0) return {0, false};
  return {convert_steering_normal(steering_ext_w_index(sample.raw)), true};
}
}
