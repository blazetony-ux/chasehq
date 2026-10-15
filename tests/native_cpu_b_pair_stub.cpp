#include "native_cpu_b_abi.h"
// Synthetic test-only fixture; production pair always regenerated from
// authenticated real CPU-B Ghidra instruction evidence.
#if defined(_WIN32)
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT extern "C" __attribute__((visibility("default")))
#endif
EXPORT int chq_native_cpu_b_tst_beq_pair_v1(const PairInput* in,PairOutput* out) {
 if(!in||!out||in->version!=1||in->pc!=0x4d8u||in->address!=0x100804u||in->tstOpcode!=0x4a79u||in->branchOpcode!=0x67f8u)return 0;
 bool zero=in->readWord==0;
 out->sr=static_cast<std::uint16_t>((in->sr&0xfff0u)|(in->readWord&0x8000u?8u:0u)|(zero?4u:0u));
 out->pc=zero?0x4d8u:0x4e0u;out->address=in->address;out->readWidth=2;out->version=1;out->reserved=0;
 out->cycles=zero?26u:24u;return 1;
}
