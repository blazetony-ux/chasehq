#include "native_cpu_b_abi.h"
// Synthetic test-only ABI stand-in. The production DLL is ALWAYS regenerated
// from authenticated CPU-B Ghidra and is not this fixture.
#if defined(_WIN32)
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT extern "C" __attribute__((visibility("default")))
#endif
EXPORT int chq_native_cpu_b_beq_v1(const BInput* in,BOutput* out){
 if(!in||!out||in->version!=1||in->pc!=0x4de||in->opcode!=0x67f8)return 0;
 bool taken=(in->sr&4)!=0;
 out->version=1;out->sr=in->sr;out->pc=taken?0x4d8:0x4e0;
 out->cycles=taken?10:8;return 1;
}
