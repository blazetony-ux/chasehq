#pragma once
#include <cstdint>
// Exact C ABI emitted by the independently authenticated Ghidra CPU-B branch generator.
struct BInput {std::uint32_t pc;std::uint16_t sr,opcode,version;};
struct BOutput {std::uint32_t pc;std::uint16_t sr,version;std::uint32_t cycles;};
using NativeCpuBBranchFunction = int(*)(const BInput*, BOutput*);

// RC6.3.13 generated Ghidra CPU-B TST.W absolute-long + BEQ.B pair.
// Identical layout to independently emitted RC6.3.12 C++ ABI.
struct PairInput {std::uint32_t pc,address;std::uint16_t tstOpcode,branchOpcode,readWord,sr,version,reserved;};
struct PairOutput {std::uint32_t pc,address;std::uint16_t sr,readWidth,version,reserved;std::uint32_t cycles;};
using NativeCpuBPairFunction = int(*)(const PairInput*,PairOutput*);
