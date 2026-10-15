#include "steering_conversion.h"
#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
int main(int argc, char** argv) {
  if (argc != 2) { std::cerr << "CSV path required\n"; return 2; }
  std::ifstream csv(argv[1]);
  if (!csv) return 3;
  std::string line; std::getline(csv,line);
  int count=0;
  while (std::getline(csv,line)) {
    std::istringstream ss(line); std::string input,addr,word,output;
    std::getline(ss,input,','); std::getline(ss,addr,','); std::getline(ss,word,','); std::getline(ss,output,',');
    const int s=std::stoi(input), expected=std::stoi(output);
    if (s != count-96 || chasehq::native::convert_steering_normal(static_cast<int16_t>(s)) != expected) return 4;
    auto result=chasehq::native::convert_steering({static_cast<int16_t>(s),0});
    if (!result.verified_mode || result.output != expected) return 5;
    ++count;
  }
  if (count!=193) return 6;
  if (chasehq::native::convert_steering_normal(-32768)!=-96) return 7;
  if (chasehq::native::convert_steering_normal(32767)!=96) return 8;
  if (chasehq::native::convert_steering({50,1}).verified_mode) return 9;
  // RC6.2.4 authentic acceptance captures: frame 2065/2113/2161/2209/2257.
  // In particular 0x0FE0 is -32 after the original CPU's EXT.W D1.
  struct SteeringEvidence { int16_t ram; int16_t index; int16_t value; uint32_t address; };
  constexpr SteeringEvidence cases[] = {
    {static_cast<int16_t>(0xFFA0u), -96, -96, 0x3126cu},
    {0x0000, 0, 0, 0x3132cu},
    {0x0060, 96, 96, 0x313ecu},
    {0x0FE0, -32, -12, 0x312ecu},
    {0x0020, 32, 12, 0x3136cu},
    {static_cast<int16_t>(0xFFE0u), -32, -12, 0x312ecu}
  };
  for (const auto& v : cases) {
    const auto index = chasehq::native::steering_ext_w_index(v.ram);
    const auto result = chasehq::native::convert_steering({v.ram, 0});
    if (index != v.index || !result.verified_mode || result.output != v.value ||
        chasehq::native::kSteeringRomTableBase + 2 * index != v.address) return 10;
  }
  if (chasehq::native::steering_ext_w_index(0x0F80) != -128) return 11;
  std::cout << "PASS: 193/193 reference entries, clamp extremes, fail-closed alternate mode\n";
}
