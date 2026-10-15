#include "native_copy_abi.h"
extern "C" {
#if defined(_WIN32)
__declspec(dllexport)
#else
__attribute__((visibility("default")))
#endif
int chq_native_copy_v1(const CopyInput* in,CopyOutput* out) {
    if(!in||!out||in->version!=1)return 0;
    out->version=1;out->a0=in->a0+64;out->a1=in->a1+64;
    for(unsigned i=0;i<16;++i)out->values[i]=in->source[i];
    auto v=in->source[15];out->sr=static_cast<std::uint16_t>((in->sr&0xfff0u)|(v==0?4u:0u)|(v&0x80000000u?8u:0u));
    return 1;
}
}