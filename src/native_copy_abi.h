#pragma once
#include <cstdint>
// Identical versioned POD layout in the constrained Ghidra-derived module.
// No emulator pointers cross this ABI boundary.
struct CopyInput {std::uint32_t a0,a1; std::uint16_t sr,version;std::uint32_t source[16];};
struct CopyOutput {std::uint32_t a0,a1;std::uint16_t sr,version;std::uint32_t values[16];};
using NativeCopyFunction=int (*)(const CopyInput*,CopyOutput*);
