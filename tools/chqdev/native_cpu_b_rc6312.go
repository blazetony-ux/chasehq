package main

// RC6.3.12: independently generated two-instruction CPU-B candidate contract.
// This is an OFFLINE ABI precondition for any future live range substitution;
// the previously authenticated single BEQ hook remains the only CPU-B live
// candidate. Read + flags + branch are generated from original Ghidra sites
// after comparing the Ghidra operand with ORIGINAL CPU-B extension bytes.
import (
	"encoding/binary"
	"errors"
	"fmt"
	"regexp"
	"strconv"
	"strings"
)

type rc6312PairProof struct {
	Schema                     string `json:"schema"`
	Status                     string `json:"status"`
	ROMSHA256                  string `json:"cpuBRomSha256"`
	Start                      string `json:"entry"`
	EndExclusive               string `json:"endExclusive"`
	SourceAddress              string `json:"operandAddress"`
	OriginalConsecutiveEdges   int64  `json:"originalConsecutiveEdges"`
	ABIPaths                   int    `json:"abiPaths"`
	BusReadContract            string `json:"busReadContract"`
	FullFunctionVerified       bool   `json:"fullFunctionVerified"`
	LivePairSubstitutionProven bool   `json:"livePairSubstitutionProven"`
	PromotionAllowed           bool   `json:"promotionAllowed"`
}

var rc6312TstSyntax = regexp.MustCompile(`(?i)^tst\.w\s+\(0x([a-f0-9]{8})\)\.l$`)

func rc6312GeneratePair(doc rc639Static, rom []byte, observed rc637SharedProof) (string, rc6312PairProof, error) {
	p := rc6312PairProof{Schema: "chq-rc6312-cpu-b-tst-beq-pair-v1", Status: "BLOCKED", Start: "B:0x0004D8", EndExclusive: "B:0x0004E0", BusReadContract: "one mapped R16 from authenticated Ghidra TST.W absolute-long operand; no writes", ABIPaths: 4}
	if len(rom) != 0x20000 || binary.BigEndian.Uint16(rom[0x4d8:0x4da]) != 0x4a79 || binary.BigEndian.Uint16(rom[0x4de:0x4e0]) != 0x67f8 {
		return "", p, errors.New("CPU-B original ROM candidate opcodes do not match Ghidra pair")
	}
	var tst, beq *rc639Site
	for i := range doc.Sites {
		switch strings.ToUpper(doc.Sites[i].PC) {
		case "0X0004D8":
			tst = &doc.Sites[i]
		case "0X0004DE":
			beq = &doc.Sites[i]
		}
	}
	if tst == nil || beq == nil || tst.Length != 6 || beq.Length != 2 || tst.FunctionReviewed || beq.FunctionReviewed || tst.Opcode != "0x4A79" || beq.Opcode != "0x67F8" || doc.NativePromotionAllowed {
		return "", p, errors.New("CPU-B pair is not exactly two independently Ghidra-verified unpromoted instructions")
	}
	m := rc6312TstSyntax.FindStringSubmatch(strings.TrimSpace(tst.Text))
	if len(m) != 2 {
		return "", p, errors.New("Ghidra CPU-B TST.W operand unsupported or ambiguous")
	}
	address, e := strconv.ParseUint(m[1], 16, 32)
	if e != nil || address < 0x100000 || address > 0x10fffe || address&1 != 0 || uint32(address) != binary.BigEndian.Uint32(rom[0x4da:0x4de]) {
		return "", p, errors.New("Ghidra CPU-B operand does not match authentic ROM extension words / mapped RAM address")
	}
	if !strings.HasPrefix(strings.ToLower(strings.TrimSpace(beq.Text)), "beq") || len(beq.FlowTargets) != 1 || !strings.EqualFold(beq.FlowTargets[0], "0x0004D8") {
		return "", p, errors.New("Ghidra CPU-B BEQ target contradictory")
	}
	var consecutive int64
	for _, node := range observed.CPU_BCandidates {
		if !strings.EqualFold(node.PC, "0x0004D8") || !strings.EqualFold(node.OpcodeWord, "0x4A79") {
			continue
		}
		for _, edge := range node.Destinations {
			if strings.EqualFold(edge.To, "0x0004DE") && strings.EqualFold(edge.Opcode, "0x4A79") {
				consecutive += edge.Hits
			}
		}
	}
	if consecutive < 1 || observed.Status != "LIVE_SHARED_DISPATCH_AND_OBSERVED_CPU_B_EDGES_PASS" {
		return "", p, errors.New("CPU-B pair lacks authentic consecutive original-ROM execution provenance")
	}
	p.OriginalConsecutiveEdges = consecutive
	p.SourceAddress = fmt.Sprintf("0x%08X", address)
	p.ROMSHA256 = doc.RawROM
	// This intentionally exposes a deterministic READ request (not a memory
	// callback) and takes the sampled word as an input. Live bus ordering and
	// interrupt ownership must be separately proved BEFORE enabling substitution.
	code := fmt.Sprintf(`#include <cstdint>
#if defined(_WIN32)
#define CHQ_EXPORT extern "C" __declspec(dllexport)
#else
#define CHQ_EXPORT extern "C" __attribute__((visibility("default")))
#endif
struct PairInput {uint32_t pc, address;uint16_t tstOpcode,branchOpcode,readWord,sr,version,reserved;};
struct PairOutput {uint32_t pc,address;uint16_t sr,readWidth,version,reserved;uint32_t cycles;};
// ORIGINAL CPU-B Ghidra TST.W (0x%%08X).L + BEQ.B; one R16, zero writes.
CHQ_EXPORT int chq_native_cpu_b_tst_beq_pair_v1(const PairInput* in,PairOutput* out){
 if(!in||!out||in->version!=1||in->pc!=0x4D8U||in->address!=0x%XU||in->tstOpcode!=0x4A79U||in->branchOpcode!=0x67F8U)return 0;
 const uint16_t oldX=in->sr&0x10U;
 const bool zero=(in->readWord==0);
 const uint16_t n=(in->readWord&0x8000U)?0x8U:0U;
 out->sr=static_cast<uint16_t>((in->sr&0xFFE0U)|oldX|n|(zero?4U:0U));
 out->pc=zero?0x4D8U:0x4E0U;
 out->address=0x%XU;out->readWidth=2;out->version=1;out->reserved=0;
 out->cycles=16U+(zero?10U:8U);return 1;
}
`, address, address)
	return code, p, nil
}
func rc6312PairTestSource(addr string) string {
	return fmt.Sprintf(`#include <cstdint>
#include <cstdio>
struct PairInput{uint32_t pc,address;uint16_t tstOpcode,branchOpcode,readWord,sr,version,reserved;};
struct PairOutput{uint32_t pc,address;uint16_t sr,readWidth,version,reserved;uint32_t cycles;};
extern "C" int chq_native_cpu_b_tst_beq_pair_v1(const PairInput*,PairOutput*);
int main(){PairInput i{0x4d8,%sU,0x4a79,0x67f8,0,0x201b,1,0};PairOutput o{};
 if(!chq_native_cpu_b_tst_beq_pair_v1(&i,&o)||o.pc!=0x4d8||o.sr!=0x2014||o.cycles!=26||o.readWidth!=2||o.address!=i.address)return 1;
 i.readWord=0x8001;i.sr=0x2015;
 if(!chq_native_cpu_b_tst_beq_pair_v1(&i,&o)||o.pc!=0x4e0||o.sr!=0x2018||o.cycles!=24)return 2;
 i.readWord=0x0001;i.sr=0x2000;
 if(!chq_native_cpu_b_tst_beq_pair_v1(&i,&o)||o.pc!=0x4e0||o.sr!=0x2000||o.cycles!=24)return 3;
 i.readWord=0;i.sr=0x2010;
 if(!chq_native_cpu_b_tst_beq_pair_v1(&i,&o)||o.pc!=0x4d8||o.sr!=0x2014)return 4;
 if(chq_native_cpu_b_tst_beq_pair_v1(nullptr,&o)||chq_native_cpu_b_tst_beq_pair_v1(&i,nullptr))return 5;
 i.address+=2;if(chq_native_cpu_b_tst_beq_pair_v1(&i,&o))return 6;i.address-=2;
 i.branchOpcode=0x67fa;if(chq_native_cpu_b_tst_beq_pair_v1(&i,&o))return 7;i.branchOpcode=0x67f8;
 i.pc=0x4de;if(chq_native_cpu_b_tst_beq_pair_v1(&i,&o))return 8;
 puts("PASS RC6312 Ghidra CPU-B TST.W+BEQ pair ABI 4/4 condition paths, one-R16 contract, corruption/null guards; NOT LIVE");return 0;}
`, addr)
}
