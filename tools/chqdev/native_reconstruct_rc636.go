package main

// RC6.3.6: the independent sprite longword-copy block is emitted from the
// exact supported Ghidra instruction candidate, not from a hand-coded copy
// routine. Function boundary and live eligibility remain deliberately bounded.
import (
	"errors"
	"fmt"
	"strings"
)

func rc636CopyLibrarySource(code string, candidates []rc63Candidate) (string, error) {
	idx := -1
	for i, c := range candidates {
		if c.Family == "sequential-longword-copy" && c.Entry == "0x00040540" && c.End == "0x00040560" && len(c.Instructions) == 16 {
			idx = i
		}
	}
	if idx < 0 {
		return "", errors.New("authentic CPU-A Ghidra copy block 0x40540..0x40560 unavailable")
	}
	head, _, ok := strings.Cut(code, "int main(){")
	if !ok {
		return "", errors.New("generated core missing")
	}
	var out strings.Builder
	out.WriteString(head)
	out.WriteString(`
#if defined(_WIN32)
#define CHQ_COPY_EXPORT extern "C" __declspec(dllexport)
#else
#define CHQ_COPY_EXPORT extern "C" __attribute__((visibility("default")))
#endif
struct CopyInput {uint32_t a0,a1; uint16_t sr,version;uint32_t source[16];};
struct CopyOutput {uint32_t a0,a1;uint16_t sr,version;uint32_t values[16];};
CHQ_COPY_EXPORT int chq_native_copy_v1(const CopyInput* in,CopyOutput* out) {
 if(!in||!out||in->version!=1||in->a0&1U||in->a1&1U) return 0;
 const uint64_t srcEnd=uint64_t(in->a0)+64U, dstEnd=uint64_t(in->a1)+64U;
 if(srcEnd>0x1000000ULL||dstEnd>0x1000000ULL||
    !(srcEnd<=in->a1||dstEnd<=in->a0)) return 0;
 Cpu c;c.a[0]=in->a0;c.a[1]=in->a1;c.sr=in->sr;
 for(unsigned i=0;i<16;++i)c.mem.write32(in->a0+i*4U,in->source[i]);
`)
	fmt.Fprintf(&out, " candidate_%d(c);\n", idx)
	out.WriteString(` if(c.returned||c.a[0]!=in->a0+64U||c.a[1]!=in->a1+64U) return 0;
 out->a0=c.a[0];out->a1=c.a[1];out->sr=c.sr;out->version=1;
 for(unsigned i=0;i<16;++i)out->values[i]=c.mem.read32(in->a1+i*4U);
 return 1;
}
`)
	return out.String(), nil
}

func rc636CopyABITest() string {
	return `#include <cstdint>
#include <cstdio>
struct CopyInput {uint32_t a0,a1;uint16_t sr,version;uint32_t source[16];};
struct CopyOutput {uint32_t a0,a1;uint16_t sr,version;uint32_t values[16];};
extern "C" int chq_native_copy_v1(const CopyInput*,CopyOutput*);
int main(){
 for(unsigned variant=0;variant<7;++variant){
  CopyInput in{};CopyOutput out{};in.a0=0x1000;in.a1=0x2000;in.sr=0x2710;in.version=1;
  for(unsigned i=0;i<16;++i)in.source[i]=0x12000000U+i*0x101U+variant;
  if(chq_native_copy_v1(&in,&out)!=1||out.a0!=0x1040||out.a1!=0x2040||out.version!=1){std::puts("FAIL generated copy ABI output registers");return 3;}
  for(unsigned i=0;i<16;++i)if(out.values[i]!=in.source[i]){std::puts("FAIL generated copy ABI data");return 4;}
  if(out.sr!=0x2710){std::puts("FAIL generated copy ABI SR");return 5;}
 }
 CopyInput in{};CopyOutput out{};in.a0=0x1000;in.a1=0x2000;in.version=1;
 if(chq_native_copy_v1(nullptr,&out)!=0||chq_native_copy_v1(&in,nullptr)!=0){std::puts("FAIL copy ABI null guard");return 6;}
 in.a1=0x1002;
 if(chq_native_copy_v1(&in,&out)!=0){std::puts("FAIL copy ABI overlap guard");return 7;}
 std::puts("PASS generated Ghidra copy shared-library ABI 112/112 and safety guards");return 0;
}
`
}
