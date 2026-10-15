package main

// RC6.3 function-discovery / basic-block reconstruction prototype. It consumes
// genuine Ghidra data, emits C++ from a strictly parsed 68000 subset, compiles
// the output and verifies it against independently archived reference vectors.
// This does NOT promote a whole function or enable general native substitution.
import (
	"archive/zip"
	"context"
	"crypto/sha256"
	"encoding/csv"
	"encoding/hex"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"os"
	"os/exec"
	"path/filepath"
	"regexp"
	"runtime"
	"sort"
	"strconv"
	"strings"
	"time"
)

type rc63Instruction struct {
	PC     string `json:"pc"`
	Text   string `json:"text"`
	Length int    `json:"length"`
}
type rc63Function struct {
	Entry   string `json:"entry"`
	BodyMin string `json:"bodyMin"`
	BodyMax string `json:"bodyMaxInclusive"`
	Name    string `json:"name"`
}
type rc63Site struct {
	PC           string            `json:"pc"`
	Function     *rc63Function     `json:"functionCandidate"`
	Instructions []rc63Instruction `json:"nearbyInstructions"`
}
type rc63Findings struct {
	Schema string     `json:"schema"`
	CPU    string     `json:"cpu"`
	ROM    string     `json:"rawRomSha256"`
	Sites  []rc63Site `json:"sites"`
}
type rc63Candidate struct {
	ID                string            `json:"id"`
	CPU               string            `json:"cpu"`
	Entry             string            `json:"entry"`
	End               string            `json:"endExclusive"`
	Instructions      []rc63Instruction `json:"instructions"`
	Family            string            `json:"supportedFamily"`
	FunctionCandidate string            `json:"ghidraFunctionCandidate,omitempty"`
	FullFunction      bool              `json:"contiguousGhidraFunctionRange"`
	Verification      string            `json:"verification"`
	Promotion         string            `json:"promotion"`
}

var rc63Movea = regexp.MustCompile(`(?i)^movea\.l #0x([0-9a-f]+),A([0-7])$`)
var rc63Load = regexp.MustCompile(`(?i)^move\.w \((-?0x[0-9a-f]+),A([0-7])\),D([0-7])w$`)
var rc63Ext = regexp.MustCompile(`(?i)^ext\.w D([0-7])w$`)
var rc63Adda = regexp.MustCompile(`(?i)^adda\.w D([0-7])w,A([0-7])$`)
var rc63Store = regexp.MustCompile(`(?i)^move\.w \(A([0-7])\),\((-?0x[0-9a-f]+),A([0-7])\)$`)
var rc63Tst = regexp.MustCompile(`(?i)^tst\.w \((-?0x[0-9a-f]+),A([0-7])\)$`)
var rc63Bcc = regexp.MustCompile(`(?i)^(bne|bpl|bra)\.b (0x[0-9a-f]+)$`)
var rc63Subq = regexp.MustCompile(`(?i)^subq\.w #0x([0-9a-f]+),\((-?0x[0-9a-f]+),A([0-7])\)$`)
var rc63MoveImmediate = regexp.MustCompile(`(?i)^move\.w #0x([0-9a-f]+),\((-?0x[0-9a-f]+),A([0-7])\)$`)
var rc63Add = regexp.MustCompile(`(?i)^add\.w D([0-7])w,\((-?0x[0-9a-f]+),A([0-7])\)$`)
var rc63Copy = regexp.MustCompile(`(?i)^move\.l \(A([0-7])\)\+,\(A([0-7])\)\+$`)

func rc63ReadFullGhidra(path string) (rc63Findings, error) {
	var g rc63Findings
	z, err := zip.OpenReader(path)
	if err != nil {
		return g, err
	}
	defer z.Close()
	for _, f := range z.File {
		name := strings.ToLower(strings.ReplaceAll(f.Name, "\\", "/"))
		if !strings.HasSuffix(name, "/analysis/chq-ghidra-findings.json") {
			continue
		}
		if f.UncompressedSize64 > 8<<20 {
			return g, errors.New("Ghidra findings size exceeds limit")
		}
		r, e := f.Open()
		if e != nil {
			return g, e
		}
		b, e := io.ReadAll(io.LimitReader(r, 8<<20+1))
		_ = r.Close()
		if e != nil {
			return g, e
		}
		if e = json.Unmarshal(b, &g); e != nil {
			return g, e
		}
		if g.Schema != "chq-ghidra-static-findings-v1" || len(g.Sites) == 0 {
			return g, errors.New("unexpected Ghidra schema or empty sites")
		}
		return g, nil
	}
	return g, errors.New("real Ghidra findings absent")
}
func rc63Address(p string) (uint64, error) {
	n, err := strconv.ParseUint(strings.TrimSpace(p), 0, 32)
	if err != nil || n&1 != 0 {
		return 0, fmt.Errorf("invalid 68000 PC %q", p)
	}
	return n, nil
}
func rc63Supported(text string) bool {
	return rc63Movea.MatchString(text) || rc63Load.MatchString(text) || rc63Ext.MatchString(text) || rc63Adda.MatchString(text) || rc63Store.MatchString(text) || rc63Copy.MatchString(text) || rc63Tst.MatchString(text) || rc63Bcc.MatchString(text) || rc63Subq.MatchString(text) || rc63MoveImmediate.MatchString(text) || rc63Add.MatchString(text) || strings.EqualFold(strings.TrimSpace(text), "rts")
}
func rc63Family(ins []rc63Instruction) string {
	if len(ins) >= 4 && rc63Movea.MatchString(ins[0].Text) && strings.EqualFold(ins[len(ins)-1].Text, "rts") {
		load, ext, adda, store := 0, 0, 0, 0
		for _, i := range ins {
			if rc63Load.MatchString(i.Text) {
				load++
			}
			if rc63Ext.MatchString(i.Text) {
				ext++
			}
			if rc63Adda.MatchString(i.Text) {
				adda++
			}
			if rc63Store.MatchString(i.Text) {
				store++
			}
		}
		if load == 1 && ext == 1 && adda == 2 && store == 1 && len(ins) == 7 {
			return "steering-lookup-tail"
		}
	}
	if len(ins) == 9 && rc63Tst.MatchString(ins[0].Text) && strings.EqualFold(ins[len(ins)-1].Text, "rts") {
		if rc63Bcc.MatchString(ins[1].Text) && rc63Subq.MatchString(ins[2].Text) && rc63Bcc.MatchString(ins[3].Text) && rc63MoveImmediate.MatchString(ins[4].Text) && rc63Bcc.MatchString(ins[5].Text) && rc63Load.MatchString(ins[6].Text) && rc63Add.MatchString(ins[7].Text) {
			return "conditional-health-function"
		}
	}
	if len(ins) >= 4 {
		for _, i := range ins {
			if !rc63Copy.MatchString(i.Text) {
				return ""
			}
		}
		return "sequential-longword-copy"
	}
	return ""
}
func rc63Discover(g rc63Findings) ([]rc63Candidate, error) {
	if g.Schema != "chq-ghidra-static-findings-v1" || (g.CPU != "A" && g.CPU != "B") || len(g.ROM) != 64 || len(g.Sites) == 0 {
		return nil, errors.New("invalid real Ghidra provenance")
	}
	if _, err := hex.DecodeString(g.ROM); err != nil {
		return nil, err
	}
	found := map[string]rc63Candidate{}
	for _, s := range g.Sites {
		ins := append([]rc63Instruction(nil), s.Instructions...)
		sort.Slice(ins, func(a, b int) bool { ap, _ := rc63Address(ins[a].PC); bp, _ := rc63Address(ins[b].PC); return ap < bp })
		// Supported contiguous runs become independent basic-block candidates;
		// Ghidra function boundaries are NOT inferred from a local window.
		var run []rc63Instruction
		flush := func() {
			fam := rc63Family(run)
			if fam == "" {
				return
			}
			p, _ := rc63Address(run[0].PC)
			last, _ := rc63Address(run[len(run)-1].PC)
			c := rc63Candidate{ID: fmt.Sprintf("%s:%08X-%08X", g.CPU, p, last+uint64(run[len(run)-1].Length)), CPU: g.CPU, Entry: fmt.Sprintf("0x%08X", p), End: fmt.Sprintf("0x%08X", last+uint64(run[len(run)-1].Length)), Instructions: append([]rc63Instruction(nil), run...), Family: fam, Verification: "STATIC_SUPPORTED_RUNTIME_UNVERIFIED", Promotion: "BLOCKED"}
			if s.Function != nil {
				c.FunctionCandidate = s.Function.Entry
			}
			if s.Function != nil {
				entry, entryErr := rc63Address(s.Function.Entry)
				end, endErr := strconv.ParseUint(s.Function.BodyMax, 0, 32)
				if entryErr == nil && endErr == nil && entry == p && end == last+uint64(run[len(run)-1].Length)-1 {
					c.FullFunction = true
				}
			}
			found[c.ID] = c
		}
		var previous uint64
		for _, i := range ins {
			pc, e := rc63Address(i.PC)
			if e != nil || i.Length < 2 || i.Length > 10 || i.Length%2 != 0 {
				return nil, fmt.Errorf("malformed Ghidra instruction in site %s", s.PC)
			}
			if len(run) > 0 && (pc != previous || !rc63Supported(i.Text)) {
				flush()
				run = nil
			}
			if !rc63Supported(i.Text) {
				previous = pc + uint64(i.Length)
				continue
			}
			run = append(run, i)
			previous = pc + uint64(i.Length)
			if strings.EqualFold(i.Text, "rts") {
				flush()
				run = nil
			}
		}
		flush()
	}
	out := make([]rc63Candidate, 0, len(found))
	for _, v := range found {
		out = append(out, v)
	}
	sort.Slice(out, func(i, j int) bool { return out[i].ID < out[j].ID })
	return out, nil
}
func rc63Literal(h string) string {
	n, err := strconv.ParseInt(h, 0, 32)
	if err != nil {
		return "INVALID"
	}
	return fmt.Sprintf("%d", n)
}
func rc63EmitInstructions(c rc63Candidate) (string, error) {
	known := map[uint64]bool{}
	for _, i := range c.Instructions {
		pc, e := rc63Address(i.PC)
		if e != nil {
			return "", e
		}
		known[pc] = true
	}
	for _, i := range c.Instructions {
		if m := rc63Bcc.FindStringSubmatch(i.Text); m != nil {
			pc, e := rc63Address(m[2])
			if e != nil || !known[pc] {
				return "", fmt.Errorf("branch escapes supported block at %s", i.PC)
			}
		}
	}
	var b strings.Builder
	for _, i := range c.Instructions {
		addr, _ := rc63Address(i.PC)
		fmt.Fprintf(&b, "pc_%X:\n", addr)
		t := i.Text
		var m []string
		switch {
		case rc63Movea.MatchString(t):
			m = rc63Movea.FindStringSubmatch(t)
			fmt.Fprintf(&b, "    c.a[%s] = 0x%sU;\n", m[2], m[1])
		case rc63Load.MatchString(t):
			m = rc63Load.FindStringSubmatch(t)
			fmt.Fprintf(&b, "    { uint16_t v=c.mem.read16(uint32_t(c.a[%s]+int32_t(%s))); c.d[%s]=(c.d[%s]&0xffff0000U)|v; c.setNZ16(v); }\n", m[2], rc63Literal(m[1]), m[3], m[3])
		case rc63Ext.MatchString(t):
			m = rc63Ext.FindStringSubmatch(t)
			fmt.Fprintf(&b, "    { uint16_t v=uint16_t(int16_t(int8_t(c.d[%s]&0xffU))); c.d[%s]=(c.d[%s]&0xffff0000U)|v; c.setNZ16(v); }\n", m[1], m[1], m[1])
		case rc63Adda.MatchString(t):
			m = rc63Adda.FindStringSubmatch(t)
			fmt.Fprintf(&b, "    c.a[%s] = uint32_t(c.a[%s]+int32_t(int16_t(c.d[%s]&0xffffU)));\n", m[2], m[2], m[1])
		case rc63Store.MatchString(t):
			m = rc63Store.FindStringSubmatch(t)
			fmt.Fprintf(&b, "    { uint16_t v=c.mem.read16(c.a[%s]); c.mem.write16(uint32_t(c.a[%s]+int32_t(%s)),v); c.setNZ16(v); }\n", m[1], m[3], rc63Literal(m[2]))
		case rc63Tst.MatchString(t):
			m = rc63Tst.FindStringSubmatch(t)
			fmt.Fprintf(&b, "    { uint16_t v=c.mem.read16(uint32_t(c.a[%s]+int32_t(%s))); c.setNZ16(v); }\n", m[2], rc63Literal(m[1]))
		case rc63Bcc.MatchString(t):
			m = rc63Bcc.FindStringSubmatch(t)
			target, e := rc63Address(m[2])
			if e != nil {
				return "", e
			}
			condition := "true"
			switch strings.ToLower(m[1]) {
			case "bne":
				condition = "(c.sr&4U)==0"
			case "bpl":
				condition = "(c.sr&8U)==0"
			}
			fmt.Fprintf(&b, "    if(%s) goto pc_%X;\n", condition, target)
		case rc63Subq.MatchString(t):
			m = rc63Subq.FindStringSubmatch(t)
			n, e := strconv.ParseUint(m[1], 16, 16)
			if e != nil || n < 1 || n > 8 {
				return "", errors.New("unsupported SUBQ immediate")
			}
			fmt.Fprintf(&b, "    { uint32_t addr=uint32_t(c.a[%s]+int32_t(%s)); uint16_t lhs=c.mem.read16(addr); uint16_t v=uint16_t(lhs-%dU); c.mem.write16(addr,v); c.setSubFlags16(lhs,%dU,v); }\n", m[3], rc63Literal(m[2]), n, n)
		case rc63MoveImmediate.MatchString(t):
			m = rc63MoveImmediate.FindStringSubmatch(t)
			fmt.Fprintf(&b, "    { uint16_t v=0x%sU; c.mem.write16(uint32_t(c.a[%s]+int32_t(%s)),v); c.setNZ16(v); }\n", m[1], m[3], rc63Literal(m[2]))
		case rc63Add.MatchString(t):
			m = rc63Add.FindStringSubmatch(t)
			fmt.Fprintf(&b, "    { uint32_t addr=uint32_t(c.a[%s]+int32_t(%s)); uint16_t lhs=c.mem.read16(addr); uint16_t rhs=uint16_t(c.d[%s]);uint16_t v=uint16_t(lhs+rhs);c.mem.write16(addr,v);c.setAddFlags16(lhs,rhs,v); }\n", m[3], rc63Literal(m[2]), m[1])
		case rc63Copy.MatchString(t):
			m = rc63Copy.FindStringSubmatch(t)
			fmt.Fprintf(&b, "    { uint32_t v=c.mem.read32(c.a[%s]); c.mem.write32(c.a[%s],v); c.a[%s]+=4U; c.a[%s]+=4U; c.setNZ32(v); }\n", m[1], m[2], m[1], m[2])
		case strings.EqualFold(t, "rts"):
			b.WriteString("    c.pc=c.mem.read32(c.a[7]); c.a[7]+=4U; c.returned=true;\n")
		default:
			return "", fmt.Errorf("unsupported instruction: %q", t)
		}
	}
	return b.String(), nil
}
func rc63ReadVectors(csvPath string) ([]uint16, []int16, error) {
	f, e := os.Open(csvPath)
	if e != nil {
		return nil, nil, e
	}
	defer f.Close()
	rows, e := csv.NewReader(f).ReadAll()
	if e != nil {
		return nil, nil, e
	}
	if len(rows) != 194 || len(rows[0]) != 4 || rows[0][0] != "signed_input" {
		return nil, nil, fmt.Errorf("reference must contain exact independent 193-vector corpus: %d rows", len(rows)-1)
	}
	words := make([]uint16, 193)
	expect := make([]int16, 193)
	for j, r := range rows[1:] {
		if len(r) != 4 {
			return nil, nil, errors.New("reference row length")
		}
		input, e := strconv.Atoi(r[0])
		if e != nil || input != j-96 {
			return nil, nil, errors.New("reference indices invalid")
		}
		addr, e := strconv.ParseUint(r[1], 0, 32)
		if e != nil || addr != uint64(0x3132c+input*2) {
			return nil, nil, errors.New("reference address mismatch")
		}
		raw, e := strconv.ParseUint(r[2], 0, 16)
		if e != nil {
			return nil, nil, e
		}
		out, e := strconv.ParseInt(r[3], 10, 16)
		if e != nil || int16(raw) != int16(out) {
			return nil, nil, errors.New("reference ROM word/output mismatch")
		}
		words[j] = uint16(raw)
		expect[j] = int16(out)
	}
	return words, expect, nil
}
func rc63Cpp(cands []rc63Candidate, word []uint16, expect []int16) (string, error) {
	if len(word) != 193 || len(expect) != 193 {
		return "", errors.New("193 reference values required")
	}
	var b strings.Builder
	b.WriteString(`#include <array>
#include <cstdint>
#include <cstdio>
#include <unordered_map>
struct Mem { std::unordered_map<uint32_t,uint8_t> cells;
 uint16_t read16(uint32_t a) const { auto x=cells.find(a);auto y=cells.find(a+1);return uint16_t(((x==cells.end()?0:x->second)<<8)|(y==cells.end()?0:y->second)); }
 uint32_t read32(uint32_t a) const { return (uint32_t(read16(a))<<16)|read16(a+2); }
 void write16(uint32_t a,uint16_t v){cells[a]=uint8_t(v>>8);cells[a+1]=uint8_t(v);}
 void write32(uint32_t a,uint32_t v){write16(a,uint16_t(v>>16));write16(a+2,uint16_t(v));}
};
struct Cpu {uint32_t a[8]{};uint32_t d[8]{};uint32_t pc{};uint16_t sr{0x10};bool returned{};Mem mem;
 void setNZ16(uint16_t v){sr=uint16_t((sr&~0x000fU)|(v==0?4U:0U)|((v&0x8000U)?8U:0U));}
 void setNZ32(uint32_t v){sr=uint16_t((sr&~0x000fU)|(v==0?4U:0U)|((v&0x80000000U)?8U:0U));}
 void setSubFlags16(uint16_t lhs,uint16_t rhs,uint16_t v){bool c=lhs<rhs;bool ov=(((lhs^rhs)&(lhs^v)&0x8000U)!=0);sr=uint16_t((sr&~0x001fU)|(v==0?4U:0U)|((v&0x8000U)?8U:0U)|(ov?2U:0U)|(c?0x11U:0U));}
 void setAddFlags16(uint16_t lhs,uint16_t rhs,uint16_t v){bool c=(uint32_t(lhs)+uint32_t(rhs))>0xffffU;bool ov=((~(lhs^rhs)&(lhs^v)&0x8000U)!=0);sr=uint16_t((sr&~0x001fU)|(v==0?4U:0U)|((v&0x8000U)?8U:0U)|(ov?2U:0U)|(c?0x11U:0U));}
};
`)
	sidx, cidx, hidx := -1, -1, -1
	for k, c := range cands {
		body, e := rc63EmitInstructions(c)
		if e != nil {
			return "", e
		}
		fmt.Fprintf(&b, "// Ghidra CPU-%s basic block/function candidate %s; static boundaries unreviewed.\nvoid candidate_%d(Cpu &c) {\n%s}\n", c.CPU, c.ID, k, body)
		if c.Family == "steering-lookup-tail" {
			sidx = k
		}
		if c.Family == "sequential-longword-copy" {
			cidx = k
		}
		if c.Family == "conditional-health-function" {
			hidx = k
		}
	}
	if sidx < 0 || cidx < 0 || hidx < 0 {
		return "", errors.New("real Ghidra windows must include steering tail, copier block, and branched health function")
	}
	b.WriteString("static constexpr std::array<uint16_t,193> raw_rom_words = {")
	for i, v := range word {
		if i > 0 {
			b.WriteString(",")
		}
		fmt.Fprintf(&b, "0x%04xU", v)
	}
	b.WriteString("};\nstatic constexpr std::array<int16_t,193> expected_outputs = {")
	for i, v := range expect {
		if i > 0 {
			b.WriteString(",")
		}
		fmt.Fprintf(&b, "%d", v)
	}
	b.WriteString("};\nint main(){\n")
	fmt.Fprintf(&b, ` for(int i=0;i<193;i++){Cpu c;for(int j=0;j<193;j++)c.mem.write16(uint32_t(0x3126c+j*2),raw_rom_words[j]);
    c.a[5]=0x107d00;c.a[7]=0x300000;c.mem.write32(0x300000,0x12345678);c.d[1]=0xfedc0000;
    c.mem.write16(0x100000,uint16_t(0x0f00U|uint8_t(i-96)));
    candidate_%d(c);
    if(!c.returned||c.pc!=0x12345678||c.a[7]!=0x300004||c.a[0]!=uint32_t(0x3132c+(i-96)*2)||int16_t(c.mem.read16(0x100000))!=expected_outputs[i]||int16_t(c.d[1]&0xffff)!=(i-96)||((c.sr&0x000f)!=uint16_t((expected_outputs[i]==0?4U:0U)|((expected_outputs[i]<0)?8U:0U)))||(c.sr&0x10)==0){std::printf("FAIL steering %%d\n",i-96);return 2;}}
`, sidx)
	fmt.Fprintf(&b, ` for(int variation=0;variation<7;variation++){ Cpu c;c.a[0]=0x1000;c.a[1]=0x2000;
    for(int i=0;i<16;i++)c.mem.write32(uint32_t(0x1000+i*4),uint32_t(0xdead0000U+i*131U+variation));
    candidate_%d(c);
    if(c.a[0]!=0x1040||c.a[1]!=0x2040||c.returned){std::puts("FAIL copy registers");return 3;}
    for(int i=0;i<16;i++)if(c.mem.read32(uint32_t(0x2000+i*4))!=uint32_t(0xdead0000U+i*131U+variation)){std::puts("FAIL copy data");return 4;}
    if((c.sr&0xf)!=(uint16_t((c.mem.read32(0x203c)&0x80000000U)?8:0))){std::puts("FAIL copy flags");return 5;}}
`, cidx)
	fmt.Fprintf(&b, ` struct HCase {uint16_t gate,hits,increment,score,wantHits,wantScore;bool visitedAdd;};
 static const HCase health[] = {
 {1,5,3,6,5,9,true},{0,5,3,6,4,9,true},{0,0,3,6,0xffff,0x1400,false},
 {0,0x8000,3,6,0x7fff,9,true},{1,0,0xffff,1,0,0,true},
 {0,1,2,0xffff,0,1,true},{0,0xffff,0,5,0xfffe,0x1400,false},{1,1,0,5,1,5,true}};
 for(const auto &h:health){Cpu c;c.a[5]=0x107ed2;c.a[7]=0x300000;c.mem.write32(0x300000,0x12345678);
 c.d[0]=0xcafe0000;c.mem.write16(0x100000,h.gate);c.mem.write16(0x100180,h.hits);
 c.mem.write16(0x10018c,h.increment);c.mem.write16(0x1001aa,h.score);
 candidate_%d(c);
 uint32_t wantD0=h.visitedAdd?(0xcafe0000U|h.increment):0xcafe0000U;
 if(!c.returned||c.pc!=0x12345678||c.a[7]!=0x300004||c.mem.read16(0x100180)!=h.wantHits||c.mem.read16(0x1001aa)!=h.wantScore||c.d[0]!=wantD0){std::printf("FAIL health gate=%%u hits=%%u\n",h.gate,h.hits);return 6;}
 }
`, hidx)
	b.WriteString(" std::puts(\"PASS real-Ghidra generated blocks: steering 193/193; copier 112/112; branched full-range health 8/8\");return 0;}\n")
	return b.String(), nil
}

// rc631HealthLibrarySource emits a DLL/SO directly from the SAME Ghidra-derived
// generated instruction functions used by the offline executable. The exported
// POD ABI never contains pointers to emulator memory, and cannot alter host RAM.
func rc631HealthLibrarySource(code string, candidates []rc63Candidate) (string, error) {
	idx := -1
	for i, c := range candidates {
		if c.Family == "conditional-health-function" && c.FullFunction && c.Entry == "0x0000A10C" && c.End == "0x0000A12A" {
			idx = i
		}
	}
	if idx < 0 {
		return "", errors.New("Ghidra health full-range candidate unavailable")
	}
	head, _, ok := strings.Cut(code, "int main(){")
	if !ok {
		return "", errors.New("generated function body missing")
	}
	var out strings.Builder
	out.WriteString(head)
	out.WriteString(`
#if defined(_WIN32)
#define CHQ_EXPORT extern "C" __declspec(dllexport)
#else
#define CHQ_EXPORT extern "C" __attribute__((visibility("default")))
#endif
struct HealthFunctionInput {uint32_t d0,a5,sp,return_pc;uint16_t sr,gate,hits,increment,score;};
struct HealthFunctionOutput {uint32_t d0,sp,pc,returned;uint16_t sr,hits,score;};
CHQ_EXPORT int chq_native_health_v1(const HealthFunctionInput* in, HealthFunctionOutput* out){
 if(!in||!out) return 0;
 Cpu c; c.d[0]=in->d0;c.a[5]=in->a5;c.a[7]=in->sp;c.sr=in->sr;
 c.mem.write16(in->a5-0x7ed2U,in->gate);
 c.mem.write16(in->a5-0x7d52U,in->hits);
 c.mem.write16(in->a5-0x7d46U,in->increment);
 c.mem.write16(in->a5-0x7d28U,in->score);
 c.mem.write32(in->sp,in->return_pc);
`)
	fmt.Fprintf(&out, " candidate_%d(c);\n", idx)
	out.WriteString(` if(!c.returned||c.pc!=in->return_pc||c.a[7]!=in->sp+4U) return 0;
 out->d0=c.d[0];out->sp=c.a[7];out->pc=c.pc;out->returned=1;
 out->sr=c.sr;out->hits=c.mem.read16(in->a5-0x7d52U);
 out->score=c.mem.read16(in->a5-0x7d28U);
 return 1;
}
`)
	return out.String(), nil
}

func rc631HealthABITest() string {
	return `#include <cstdint>
#include <cstdio>
struct HealthFunctionInput {uint32_t d0,a5,sp,return_pc;uint16_t sr,gate,hits,increment,score;};
struct HealthFunctionOutput {uint32_t d0,sp,pc,returned;uint16_t sr,hits,score;};
extern "C" int chq_native_health_v1(const HealthFunctionInput*,HealthFunctionOutput*);
int main(){
 struct Case {uint16_t gate,hits,inc,score,wantHits,wantScore;bool adds;};
 const Case cases[]={{1,5,3,6,5,9,true},{0,5,3,6,4,9,true},{0,0,3,6,0xffff,0x1400,false},
 {0,0x8000,3,6,0x7fff,9,true},{1,0,0xffff,1,0,0,true},
 {0,1,2,0xffff,0,1,true},{0,0xffff,0,5,0xfffe,0x1400,false},{1,1,0,5,1,5,true}};
 for(auto t: cases){
    HealthFunctionInput in{0xcafe0000U,0x108000U,0x100700U,0x123456U,0x10U,t.gate,t.hits,t.inc,t.score};
    HealthFunctionOutput out{};
    int ok=chq_native_health_v1(&in,&out);
    auto expected=t.adds?(0xcafe0000U|t.inc):0xcafe0000U;
    if(ok!=1||out.returned!=1||out.pc!=in.return_pc||out.sp!=in.sp+4||out.hits!=t.wantHits||out.score!=t.wantScore||out.d0!=expected){
      std::printf("FAIL generated shared health gate=%u hits=%u got score=%u hits=%u d0=%u\n",t.gate,t.hits,out.score,out.hits,out.d0);return 5;
    }
 }
 if(chq_native_health_v1(nullptr,nullptr)!=0){std::puts("FAIL ABI accepted null arguments");return 6;}
 std::puts("PASS generated Ghidra health shared library ABI 8/8 and null rejection");return 0;
}
`
}

func rc63WriteJSON(p string, obj any) error {
	b, e := json.MarshalIndent(obj, "", "  ")
	if e != nil {
		return e
	}
	return os.WriteFile(p, append(b, '\n'), 0644)
}
func nativeReconstructCommand(root string, cfg Config, args []string) *Report {
	r := newReport("native reconstruct", cfg.Release)
	opts := map[string]string{}
	for i := 0; i < len(args); i++ {
		switch args[i] {
		case "reconstruct":
		case "--fresh":
			opts["fresh"] = "yes"
		case "--live-health":
			opts["live-health"] = "yes"
		case "--cpu-b-discover":
			opts["cpu-b-discover"] = "yes"
		case "--cpu-b-live":
			opts["cpu-b-live"] = "yes"
			opts["cpu-b-discover"] = "yes"
		case "--ghidra-zip", "--out", "--ghidra-home":
			if i+1 >= len(args) {
				r.fail("arguments", "missing option value")
				return r
			}
			opts[args[i]] = args[i+1]
			i++
		default:
			r.fail("arguments", "unknown argument: "+args[i])
			return r
		}
	}
	if opts["cpu-b-discover"] == "yes" && opts["live-health"] != "yes" {
		r.fail("usage", "--cpu-b-discover requires --live-health to authenticate the original ROM PC/edge census")
		return r
	}
	if opts["fresh"] != "yes" {
		r.fail("usage", "chqdev native reconstruct --fresh [--live-health] [--cpu-b-discover] [--cpu-b-live] [--ghidra-home DIR] [--ghidra-zip FILE] [--out DIR]")
		return r
	}
	ghidra := opts["--ghidra-zip"]
	if ghidra == "" {
		ghidra = filepath.Join(root, "research", "acceptance", "ghidra-steering-RC621-verified.zip")
	}
	dest := opts["--out"]
	if dest == "" {
		dest = filepath.Join(root, "evidence", "native-reconstruct-"+time.Now().Format("20060102-150405"))
	}
	if !filepath.IsAbs(dest) {
		dest = filepath.Join(root, dest)
	}
	if e := prepareNativeEvidenceDestination(dest); e != nil {
		r.fail("output", e.Error())
		return r
	}
	status := map[string]any{"schema": "chq-rc63-reconstruction-v1", "release": cfg.Release, "scope": "real Ghidra: two multi-instruction blocks plus one complete-range conditional health function candidate; none eligible for live promotion", "fullFunctionM1M7SignedOff": false, "nativePromotionAllowed": false, "status": "BLOCKED"}
	defer func() {
		if r.Status == "PASS" {
			status["status"] = "BLOCK_PROTOTYPES_VERIFIED"
		}
		if e := rc63WriteJSON(filepath.Join(dest, "reconstruction.json"), status); e != nil {
			r.fail("status-write", e.Error())
		}
		if e := writeNativeEvidenceManifest(dest); e != nil {
			r.fail("manifest", e.Error())
			return
		}
		if e := bundleNativeEvidence(dest); e != nil {
			r.fail("bundle", e.Error())
		} else {
			r.pass("evidence-zip", dest+".zip")
			r.Data["bundle"] = dest + ".zip"
		}
		r.Data["fullFunctionM1M7SignedOff"] = false
	}()
	g, raw, e := readNativeGhidra(ghidra)
	if e != nil {
		r.fail("M1-Ghidra", e.Error())
		return r
	}
	// Re-read the actual ZIP's structured findings: the old RC6.2 parser drops
	// per-instruction lengths, which are mandatory for contiguous block proof.
	full, e := rc63ReadFullGhidra(ghidra)
	if e != nil {
		r.fail("M1-instructions", e.Error())
		return r
	}
	if full.ROM != g.RawRomSHA || full.CPU != g.CPU {
		r.fail("M1-consistency", "Ghidra parsers disagree on CPU/ROM fingerprint")
		return r
	}
	r.pass("M1-Ghidra", fmt.Sprintf("verified CPU-%s ROM provenance; %d site windows", g.CPU, len(full.Sites)))
	candidates, e := rc63Discover(full)
	if e != nil {
		r.fail("M1-discover", e.Error())
		return r
	}
	status["sourceGhidraFindingsSHA256"] = fmt.Sprintf("%x", sha256.Sum256(raw))
	status["cpu"] = full.CPU
	status["romSHA256"] = full.ROM
	status["staticSiteWindows"] = len(full.Sites)
	status["generatedBasicBlocks"] = candidates
	status["fullFunctionBoundariesReviewed"] = false
	if len(candidates) == 0 {
		r.fail("M1-discover", "no supported contiguous Ghidra basic blocks found")
		return r
	}
	r.pass("M1-discover", fmt.Sprintf("%d distinct supported candidate blocks discovered; all unreviewed for whole-function replacement", len(candidates)))
	dossier, dossierErr := rc6315AssessFunctions(full, candidates)
	if dossierErr != nil {
		r.fail("M2-function-dossier", dossierErr.Error())
		return r
	}
	if dossierErr = rc63WriteJSON(filepath.Join(dest, "function-boundary-dossier.json"), dossier); dossierErr != nil {
		r.fail("M2-function-dossier", dossierErr.Error())
		return r
	}
	status["functionBoundaryDossier"] = "function-boundary-dossier.json"
	r.pass("M2-function-dossier", fmt.Sprintf("%d Ghidra-backed bounded candidate dossiers; unresolved caller/CFG/exit coverage explicit; no whole-function promotion", len(dossier.Candidates)))
	// RC6.3.4: candidate queue is evidence-derived and persistent, never an
	// unsupported native-promotion list. It includes a CPU-B discovery blocker.
	portfolio := rc634MakePortfolio(cfg.Release, candidates, false)
	if err := rc634WritePortfolio(root, dest, portfolio); err != nil {
		r.fail("M7-candidate-portfolio", err.Error())
		return r
	}
	r.pass("M7-candidate-portfolio", "persistent per-candidate stage ledger created; CPU-B explicitly blocked, no autonomous promotion")
	words, expected, e := rc63ReadVectors(filepath.Join(root, "docs", "reference", "steering_reference_193.csv"))
	if e != nil {
		r.fail("M3-reference", e.Error())
		return r
	}
	src, e := rc63Cpp(candidates, words, expected)
	if e != nil {
		r.fail("M4-generation", e.Error())
		return r
	}
	codeDir := filepath.Join(dest, "generated")
	_ = os.MkdirAll(codeDir, 0755)
	if e := os.WriteFile(filepath.Join(codeDir, "reconstructed.cpp"), []byte(src), 0644); e != nil {
		r.fail("M4-output", e.Error())
		return r
	}
	librarySource, e := rc631HealthLibrarySource(src, candidates)
	if e != nil {
		r.fail("M4-health-library-source", e.Error())
		return r
	}
	copySource, e := rc636CopyLibrarySource(src, candidates)
	if e != nil {
		r.fail("M4-copy-library-source", e.Error())
		return r
	}
	if e = os.WriteFile(filepath.Join(codeDir, "generated-copy.cpp"), []byte(copySource), 0644); e != nil {
		r.fail("M4-copy-library-source", e.Error())
		return r
	}
	if e = os.WriteFile(filepath.Join(codeDir, "generated-copy-abi-test.cpp"), []byte(rc636CopyABITest()), 0644); e != nil {
		r.fail("M4-copy-ABI-source", e.Error())
		return r
	}
	if e := os.WriteFile(filepath.Join(codeDir, "generated-health.cpp"), []byte(librarySource), 0644); e != nil {
		r.fail("M4-health-library-source", e.Error())
		return r
	}
	if e := os.WriteFile(filepath.Join(codeDir, "generated-health-abi-test.cpp"), []byte(rc631HealthABITest()), 0644); e != nil {
		r.fail("M4-health-ABI-fixture", e.Error())
		return r
	}
	mutated := strings.Replace(src, "if((c.sr&4U)==0) goto pc_A120;", "if((c.sr&4U)!=0) goto pc_A120;", 1)
	if mutated == src {
		r.fail("M4-mutation", "missing expected branch to corrupt")
		return r
	}
	if err := os.WriteFile(filepath.Join(codeDir, "reconstructed-mutated.cpp"), []byte(mutated), 0644); err != nil {
		r.fail("M4-mutation", err.Error())
		return r
	}
	_ = os.WriteFile(filepath.Join(codeDir, "CMakeLists.txt"), []byte("cmake_minimum_required(VERSION 3.20)\nproject(CHQRC63Blocks LANGUAGES CXX)\nset(CMAKE_CXX_STANDARD 17)\nadd_executable(chq_rc63_blocks reconstructed.cpp)\nadd_executable(chq_rc63_mutated reconstructed-mutated.cpp)\nadd_library(chq_rc63_health SHARED generated-health.cpp)\nadd_executable(chq_rc63_health_abi_test generated-health-abi-test.cpp)\ntarget_link_libraries(chq_rc63_health_abi_test PRIVATE chq_rc63_health)\nadd_library(chq_rc636_copy SHARED generated-copy.cpp)\nadd_executable(chq_rc636_copy_abi_test generated-copy-abi-test.cpp)\ntarget_link_libraries(chq_rc636_copy_abi_test PRIVATE chq_rc636_copy)\n"), 0644)
	r.pass("M4-generation", "C++17 generated from Ghidra instruction text: memory/register/flags and conditional control-flow; 3 structural families")
	cmake, e := findTool(root, "cmake")
	if e != nil {
		r.fail("M4-CMake", e.Error())
		return r
	}
	buildDir := filepath.Join(dest, "build")
	run := func(timeout time.Duration, exe string, a ...string) ([]byte, error) {
		ctx, cancel := context.WithTimeout(context.Background(), timeout)
		defer cancel()
		cmd := exec.CommandContext(ctx, exe, a...)
		cmd.Dir = root
		return cmd.CombinedOutput()
	}
	log, e := run(2*time.Minute, cmake, "-S", codeDir, "-B", buildDir)
	_ = os.WriteFile(filepath.Join(dest, "cmake-configure.log"), log, 0644)
	if e != nil {
		r.fail("M4-configure", e.Error())
		return r
	}
	r.pass("M4-configure", "generated multi-block C++ project configured")
	log, e = run(3*time.Minute, cmake, "--build", buildDir, "--config", "Release")
	_ = os.WriteFile(filepath.Join(dest, "cmake-build.log"), log, 0644)
	if e != nil {
		r.fail("M4-compile", e.Error())
		return r
	}
	r.pass("M4-compile", "independent C++17 multi-block binaries and generated health shared library built")
	exe := filepath.Join(buildDir, "chq_rc63_blocks")
	if runtime.GOOS == "windows" {
		exe = filepath.Join(buildDir, "Release", "chq_rc63_blocks.exe")
	}
	log, e = run(30*time.Second, exe)
	_ = os.WriteFile(filepath.Join(dest, "generated-verify.log"), log, 0644)
	if e == nil && !strings.Contains(string(log), "PASS real-Ghidra generated blocks: steering 193/193; copier 112/112; branched full-range health 8/8") {
		e = errors.New("generated executable did not report expected independent verification")
	}
	if e != nil {
		r.fail("M4-offline-verification", fmt.Sprintf("%v; see generated-verify.log", e))
		return r
	}
	r.pass("M4-offline-verification", "real-Ghidra generated steering 193/193, copier 112/112 and branched health 8/8 fixtures")
	badExe := filepath.Join(buildDir, "chq_rc63_mutated")
	if runtime.GOOS == "windows" {
		badExe = filepath.Join(buildDir, "Release", "chq_rc63_mutated.exe")
	}
	badLog, badErr := run(30*time.Second, badExe)
	_ = os.WriteFile(filepath.Join(dest, "mutant-verify.log"), badLog, 0644)
	var exitErr *exec.ExitError
	if !errors.As(badErr, &exitErr) || exitErr.ExitCode() != 6 || !strings.Contains(string(badLog), "FAIL health") {
		r.fail("M4-negative-branch", fmt.Sprintf("mutated branch not rejected: %v", badErr))
		return r
	}
	r.pass("M4-negative-branch", "independently compiled corrupted branch rejected by controlled health scenarios")
	abiExe := filepath.Join(buildDir, "chq_rc63_health_abi_test")
	if runtime.GOOS == "windows" {
		abiExe = filepath.Join(buildDir, "Release", "chq_rc63_health_abi_test.exe")
	}
	abiLog, abiErr := run(30*time.Second, abiExe)
	_ = os.WriteFile(filepath.Join(dest, "health-shared-abi-test.log"), abiLog, 0644)
	if abiErr != nil || !strings.Contains(string(abiLog), "PASS generated Ghidra health shared library ABI 8/8") {
		r.fail("M4-generated-shared-ABI", fmt.Sprintf("%v; see health-shared-abi-test.log", abiErr))
		return r
	}
	r.pass("M4-generated-shared-ABI", "loaded and invoked generated function DLL/SO; 8/8 branched health cases and null ABI guard")
	copyABIExe := filepath.Join(buildDir, "chq_rc636_copy_abi_test")
	if runtime.GOOS == "windows" {
		copyABIExe = filepath.Join(buildDir, "Release", "chq_rc636_copy_abi_test.exe")
	}
	copyLog, copyErr := run(30*time.Second, copyABIExe)
	_ = os.WriteFile(filepath.Join(dest, "copy-shared-abi-test.log"), copyLog, 0644)
	if copyErr != nil || !strings.Contains(string(copyLog), "PASS generated Ghidra copy shared-library ABI 112/112") {
		r.fail("M4-generated-copy-ABI", fmt.Sprintf("%v; see copy-shared-abi-test.log", copyErr))
		return r
	}
	r.pass("M4-generated-copy-ABI", "second distinct real Ghidra block compiled and invoked from generated shared library, 112/112 words; separate ABI")

	if opts["live-health"] == "yes" {
		romDir, err := nativeResolveROMDir(root, cfg)
		if err != nil {
			r.fail("M2-live-ROM-source", err.Error())
			return r
		}
		rom, err := nativeMainROM(romDir)
		if err != nil || !strings.EqualFold(hexDigest(rom), full.ROM) {
			r.fail("M2-live-ROM-identity", "live ROM source missing or not identical to verified Ghidra CPU-A ROM")
			return r
		}
		lib := filepath.Join(buildDir, "libchq_rc63_health.so")
		if runtime.GOOS == "windows" {
			lib = filepath.Join(buildDir, "Release", "chq_rc63_health.dll")
		}
		matrix, err := rc633RunBranchMatrix(root, dest, romDir, lib, candidates)
		_ = rc633SaveBranchMatrix(dest, matrix)
		if err != nil {
			r.fail("M5-branch-matrix", err.Error())
			return r
		}
		status["liveBranchMatrix"] = matrix
		r.pass("M5-branch-matrix", "original/native/forced ROM rollback checked across natural and controlled health entry-state cases; three real ROM branch paths required")
		// RC6.3.5: CPU-B runtime census is independent of any CPU-A
		// function-boundary classification. Never strand the CPU-B evidence
		// when a caller is unresolved or the boundary audit fails closed.
		portfolio = rc634MakePortfolio(cfg.Release, candidates, true)
		cpuBCount, censusErr := rc634DiscoverCPU_B(dest, &portfolio)
		if censusErr != nil {
			r.warn("M2-CPU-B-census", censusErr.Error())
		} else if cpuBCount > 0 {
			r.pass("M2-CPU-B-census", fmt.Sprintf("%d authentic original CPU-B PCs observed; 5 ranked for future Ghidra function discovery, no CPU-B native substitution", cpuBCount))
		} else {
			r.warn("M2-CPU-B-census", "no CPU-B hits observed in the bounded checkpoint window")
		}
		audit, auditErr := rc634AuditHealthBoundary(dest, candidates, rom, matrix)
		if err := rc63WriteJSON(filepath.Join(dest, "observed-boundary-audit.json"), audit); err != nil {
			r.fail("M2-observed-boundary-audit", err.Error())
			return r
		}
		if audit.ObservedFunctionTailOnly {
			for i := range portfolio.Candidates {
				if portfolio.Candidates[i].Family == "conditional-health-function" {
					portfolio.Candidates[i].Stage = "VERIFIED_NATIVE_RANGE_FUNCTION_TAIL"
					portfolio.Candidates[i].Blocker = "Original ROM caller targets an earlier function; 0xA10C is a verified tail range, not independently verified full function; natural caller/CFG coverage and CPU-B substitution unresolved"
				}
			}
		}
		if err := rc634WritePortfolio(root, dest, portfolio); err != nil {
			r.fail("M7-candidate-portfolio-update", err.Error())
			return r
		}
		r.pass("M7-candidate-portfolio-update", "live CPU-A range and independent CPU-B census persisted; no autonomous promotion")
		if auditErr != nil {
			r.fail("M2-observed-boundary-audit", auditErr.Error())
			return r
		}
		status["observedBoundaryAudit"] = audit
		if audit.ObservedFunctionTailOnly {
			r.pass("M2-observed-boundary-audit", "authentic ROM call target differs from A10C; dynamic caller-to-tail transition and entry/RTS/return checked; correctly classified as verified function tail, NOT a whole function")
		} else {
			r.pass("M2-observed-boundary-audit", "authentic ROM observed entry/RTS/return checked; all callers and complete function boundary remain unreviewed")
		}
		copyLib := filepath.Join(buildDir, "libchq_rc636_copy.so")
		if runtime.GOOS == "windows" {
			copyLib = filepath.Join(buildDir, "Release", "chq_rc636_copy.dll")
		}
		copyProof, copyErr := rc636RunCopyProof(root, dest, romDir, copyLib)
		_ = rc63WriteJSON(filepath.Join(dest, "live-copy-proof.json"), copyProof)
		if copyErr != nil {
			r.fail("M5-second-generated-copy-live", copyErr.Error())
			return r
		}
		status["liveSecondCandidate"] = copyProof
		for i := range portfolio.Candidates {
			if portfolio.Candidates[i].Family == "sequential-longword-copy" && portfolio.Candidates[i].CandidateEntry == "0x00040540" {
				portfolio.Candidates[i].Stage = "LIVE_VERIFIED_BOUNDED_COPY_BLOCK"
				portfolio.Candidates[i].LiveNativeVerified = true
				portfolio.Candidates[i].Blocker = "Full enclosing copier function/caller coverage remains unreviewed; CPU-B one-opcode BEQ live verified in RC6.3.11; full CPU-B function and autonomous promotion unreviewed"
			}
		}
		if err := rc634WritePortfolio(root, dest, portfolio); err != nil {
			r.fail("M7-copy-portfolio", err.Error())
			return r
		}
		r.pass("M7-copy-portfolio", "second independently generated bounded native block recorded; no function-wide promotion")
		r.pass("M5-second-generated-copy-live", fmt.Sprintf("real CPU-A 16-opcode copy block generated-library substitution %d/%d; exact CPU/bus/machine parity and forced original-ROM rollback", copyProof.NativeCommits, copyProof.OriginalReturns))
		// The shared dispatcher/CPU-B graph is a separate real-ROM gate, not
		// inferred from the preceding candidate-specific acceptance results.
		shared, sharedErr := rc637SharedLiveProof(dest, copyProof)
		_ = rc637SaveProof(dest, shared)
		if sharedErr != nil {
			r.fail("M5-M7-shared-dispatch-and-CPU-B-edges", sharedErr.Error())
			return r
		}
		status["sharedDispatcherAndCPUBEdges"] = shared
		for i := range portfolio.Candidates {
			for _, b := range shared.CPU_BCandidates {
				if portfolio.Candidates[i].CPU == "B" && portfolio.Candidates[i].CandidateEntry == b.PC {
					portfolio.Candidates[i].Stage = "OBSERVED_ORIGINAL_ROM_OPCODE_AND_EDGES_NOT_NATIVE"
					portfolio.Candidates[i].Blocker = fmt.Sprintf("observed ROM opcode %s and %d outgoing transitions; CPU-B Ghidra provenance and one-opcode BEQ live qualification are covered by later M1-M5 gates; this individual PC not promoted", b.OpcodeWord, b.EdgeHits)
				}
			}
		}
		if err := rc634WritePortfolio(root, dest, portfolio); err != nil {
			r.fail("M7-shared-dispatch-portfolio", err.Error())
			return r
		}
		r.pass("M5-M7-shared-dispatch-and-CPU-B-edges", fmt.Sprintf("two generated CPU-A adapters share strict ROM/native/fallback dispatcher audit; %d CPU-B observed edges, %d hot opcode sites fingerprinted", shared.CPU_BObservedEdges, len(shared.CPU_BCandidates)))
		// RC6.3.9: independently authenticate the CPU-B raw ROM through headless
		// Ghidra and compile an opt-in generated branch ABI. Never conflate that
		// offline gate with genuine CPU-B native runtime parity.
		if opts["cpu-b-discover"] == "yes" {
			proof, discoverErr := rc639DiscoverCPU_B(root, dest, romDir, opts["--ghidra-home"], shared)
			_ = rc63WriteJSON(filepath.Join(dest, "cpu-b-ghidra-discovery-proof.json"), proof)
			status["cpuBGhidraDiscovery"] = proof
			if discoverErr != nil {
				r.fail("M1-M4-CPU-B-Ghidra-and-generated-ABI", discoverErr.Error())
				return r
			}
			r.pass("M1-M4-CPU-B-Ghidra-and-generated-ABI", "real ROM CRC/SHA and five Ghidra opcode sites matched CPU-B runtime census; BEQ DLL 2/2 plus consecutive TST.W+BEQ generated ABI 4/4 and mutation guards PASS; pair NOT live")
			pairProofBytes, pairProofErr := os.ReadFile(filepath.Join(dest, "cpu-b-two-instruction-proof.json"))
			var pairStatic rc6312PairProof
			if pairProofErr == nil {
				pairProofErr = json.Unmarshal(pairProofBytes, &pairStatic)
			}
			if pairProofErr != nil {
				r.fail("M2-CPU-B-function-dossier", pairProofErr.Error())
				return r
			}
			cpuBDossier, dossierErr := rc6315CPUBFunctionDossier(proof, pairStatic, nil)
			if dossierErr != nil {
				r.fail("M2-CPU-B-function-dossier", dossierErr.Error())
				return r
			}
			if e := rc63WriteJSON(filepath.Join(dest, "cpu-b-function-boundary-dossier.json"), cpuBDossier); e != nil {
				r.fail("M2-CPU-B-function-dossier", e.Error())
				return r
			}
			r.pass("M2-CPU-B-function-dossier", "authenticated CPU-B Ghidra five-site and pair-edge dossier; enclosing function CFG/callers/exits unresolved")
			if opts["cpu-b-live"] == "yes" {
				live, liveErr := rc6310RunCPU_BLive(root, dest, romDir, filepath.Join(dest, filepath.FromSlash(proof.GeneratedLibrary)))
				_ = rc63WriteJSON(filepath.Join(dest, "cpu-b-live-proof.json"), live)
				status["cpuBLiveDifferential"] = live
				if liveErr != nil {
					r.fail("M5-CPU-B-Ghidra-native-live", liveErr.Error())
					return r
				}
				for i := range portfolio.Candidates {
					if portfolio.Candidates[i].CPU == "B" && portfolio.Candidates[i].CandidateEntry != "0x0004DE" {
						portfolio.Candidates[i].Blocker = "CPU-B 0x4DE BEQ single opcode live verified; THIS candidate not native. The 0x4D8 TST+BEQ pair is generated/offline only; function boundary unreviewed"
					}
					if portfolio.Candidates[i].CPU == "B" && portfolio.Candidates[i].CandidateEntry == "0x0004DE" {
						portfolio.Candidates[i].Stage = "LIVE_VERIFIED_BOUNDED_BEQ_INSTRUCTION"
						portfolio.Candidates[i].LiveNativeVerified = true
						portfolio.Candidates[i].Blocker = "Single original-ROM BEQ instruction only; interrupt edges and whole CPU-B function boundary not independently certified for autonomous promotion"
					}
				}
				if e := rc634WritePortfolio(root, dest, portfolio); e != nil {
					r.fail("M7-CPU-B-portfolio", e.Error())
					return r
				}
				r.pass("M5-CPU-B-Ghidra-native-live", fmt.Sprintf("CPU-B Ghidra BEQ ROM/native/fallback parity: %d original, %d native, %d forced rollback; both branch paths and whole-machine parity", live.OriginalInstructions, live.NativeCommits, live.Rollbacks))
				pair, pairErr := rc6313RunCPUBPairLive(root, dest, romDir, filepath.Join(dest, filepath.FromSlash(proof.PairLibrary)))
				_ = rc63WriteJSON(filepath.Join(dest, "cpu-b-pair-live-proof.json"), pair)
				status["cpuBPairLiveDifferential"] = pair
				if pairErr != nil {
					r.fail("M5-CPU-B-Ghidra-generated-TST-BEQ-pair-live", pairErr.Error())
					return r
				}
				r.pass("M5-CPU-B-Ghidra-generated-TST-BEQ-pair-live", fmt.Sprintf("CPU-B Ghidra pair native %d commits; R16, full CPU/flags/cycles, IRQ and machine original/native/forced rollback exact", pair.NativeCommits))
				cpuBDossier, dossierErr = rc6315CPUBFunctionDossier(proof, pairStatic, &pair)
				if dossierErr != nil {
					r.fail("M2-CPU-B-function-dossier-update", dossierErr.Error())
					return r
				}
				if e := rc63WriteJSON(filepath.Join(dest, "cpu-b-function-boundary-dossier.json"), cpuBDossier); e != nil {
					r.fail("M2-CPU-B-function-dossier-update", e.Error())
					return r
				}
				r.pass("M2-CPU-B-function-dossier-update", "bounded pair now LIVE_VERIFIED; enclosing CPU-B function remains unresolved and non-promotable")
				effects, effectsErr := rc6315PairSideEffects(dest, proof.CPUBROM_SHA256)
				if effectsErr != nil {
					r.fail("M5-CPU-B-instruction-side-effects", effectsErr.Error())
					return r
				}
				if e := rc63WriteJSON(filepath.Join(dest, "cpu-b-instruction-side-effects.json"), effects); e != nil {
					r.fail("M5-CPU-B-instruction-side-effects", e.Error())
					return r
				}
				r.pass("M5-CPU-B-instruction-side-effects", "Genuine pair TST.W R16 data vs branch zero data; IRQ ACK-owned vector/stack streams, all three modes exact; prefetch/function/scheduling still unresolved")
				edgeAudit, edgeErr := rc6316AuditPairEdges(dest, proof.CPUBROM_SHA256)
				if edgeErr != nil {
					r.fail("M2-CPU-B-bounded-edge-audit", edgeErr.Error())
					return r
				}
				if err := rc63WriteJSON(filepath.Join(dest, "cpu-b-bounded-edge-audit.json"), edgeAudit); err != nil {
					r.fail("M2-CPU-B-bounded-edge-audit", err.Error())
					return r
				}
				r.pass("M2-CPU-B-bounded-edge-audit", "five authentic checkpoint edge graphs: pair entry, both BEQ paths, exits and IRQ diversion attributed; whole function still blocked")
				frontier, frontierErr := rc6317ObservedFrontier(dest, proof.CPUBROM_SHA256)
				if frontierErr != nil {
					r.fail("M2-CPU-B-observed-function-frontier", frontierErr.Error())
					return r
				}
				if err := rc63WriteJSON(filepath.Join(dest, "cpu-b-observed-function-frontier.json"), frontier); err != nil {
					r.fail("M2-CPU-B-observed-function-frontier", err.Error())
					return r
				}
				if frontier.CompleteFunctionVerified || frontier.NativePromotionAllowed || len(frontier.ResearchQueue) == 0 {
					r.fail("M2-CPU-B-observed-function-frontier", "unsafe or empty research frontier")
					return r
				}
				r.pass("M2-CPU-B-observed-function-frontier", fmt.Sprintf("%d original-ROM edges; %d bounded research frontier nodes, %d selected edges; no whole-function inference", frontier.ObservedEdges, len(frontier.Nodes), len(frontier.Edges)))
				r.pass("M7-CPU-B-evidence-prioritisation", fmt.Sprintf("%d risk-labelled research targets, top %s; heuristic priority only, zero native promotions", len(frontier.ResearchQueue), frontier.ResearchQueue[0].Target))
				// RC6.3.18 couples five *additional* genuine CPU-B Ghidra
				// disassemblies to the observed original-ROM continuation graph.
				// Not a closed CFG or a replaceable function.
				crosswalk, crossErr := rc6318Crosswalk(dest, proof.CPUBROM_SHA256)
				if crossErr != nil {
					r.fail("M2-CPU-B-Ghidra-continuation-crosswalk", crossErr.Error())
					return r
				}
				if e := rc63WriteJSON(filepath.Join(dest, "cpu-b-ghidra-continuation-crosswalk.json"), crosswalk); e != nil {
					r.fail("M2-CPU-B-Ghidra-continuation-crosswalk", e.Error())
					return r
				}
				r.pass("M2-CPU-B-Ghidra-continuation-crosswalk", fmt.Sprintf("%d original-ROM Ghidra sites matched %d authentic checkpoint paths; unknown 0x0516 fallthrough and complete enclosing CFG remain blocked", len(crosswalk.Sites), len(crosswalk.SourceCases)))
				callReturn, callErr := rc6319CallReturnAudit(dest, proof.CPUBROM_SHA256)
				if callErr != nil {
					r.fail("M2-CPU-B-original-ROM-call-return-audit", callErr.Error())
					return r
				}
				if e := rc63WriteJSON(filepath.Join(dest, "cpu-b-original-rom-call-return-audit.json"), callReturn); e != nil {
					r.fail("M2-CPU-B-original-ROM-call-return-audit", e.Error())
					return r
				}
				r.pass("M2-CPU-B-original-ROM-call-return-audit", fmt.Sprintf("3 genuine Ghidra call/return sites, five original-ROM JSR→callee→RTS→return cases; alternate BNE fallthrough %s static-only, full-function/ABI promotion blocked", callReturn.UnobservedFallthrough.PC))
				nested, nestedErr := rc6321NestedReturnAudit(dest, proof.CPUBROM_SHA256)
				if nestedErr != nil {
					r.fail("M2-CPU-B-nested-original-ROM-return-audit", nestedErr.Error())
					return r
				}
				if e := rc63WriteJSON(filepath.Join(dest, "cpu-b-nested-original-rom-return-audit.json"), nested); e != nil {
					r.fail("M2-CPU-B-nested-original-ROM-return-audit", e.Error())
					return r
				}
				r.pass("M2-CPU-B-nested-original-ROM-return-audit", fmt.Sprintf("%d fresh Ghidra BSR/RTS sites, %d authentic checkpoint cases, %d sampled nested invocations/case; temporal stack proof, unobserved 0x0518 and whole-function promotion BLOCKED", len(nested.GhidraSites), len(nested.SourceCases), nested.OuterNestedVisitsPerCase))
				ordered, orderedErr := rc6322OrderedAudit(dest, proof.CPUBROM_SHA256)
				if orderedErr != nil {
					r.fail("M2-CPU-B-ordered-original-ROM-stack-audit", orderedErr.Error())
					return r
				}
				if e := rc63WriteJSON(filepath.Join(dest, "cpu-b-ordered-original-rom-stack-audit.json"), ordered); e != nil {
					r.fail("M2-CPU-B-ordered-original-ROM-stack-audit", e.Error())
					return r
				}
				if !ordered.SelectedNestedTemporalOrderVerified || !ordered.SelectedStackPointerPairsVerified || ordered.NativePromotionAllowed || ordered.CompleteFunctionVerified || ordered.InterruptRTEStackParityVerified {
					r.fail("M2-CPU-B-ordered-original-ROM-stack-audit", "unsafe or incomplete selected chronology certification")
					return r
				}
				r.pass("M2-CPU-B-ordered-original-ROM-stack-audit", fmt.Sprintf("%d original-ROM checkpoint sequences; sampled chronological BSR/RTS and SP matched; IRQ/RTE/full CFG/0x0518/native promotion remain BLOCKED", len(ordered.Cases)))
				irqCrosswalk, irqErr := rc6324IRQHookAudit(dest, proof.CPUBROM_SHA256)
				if irqErr != nil {
					r.fail("M2-CPU-B-original-ROM-IRQ-RTE-hook-context", irqErr.Error())
					return r
				}
				if e := rc63WriteJSON(filepath.Join(dest, "cpu-b-original-rom-irq-rte-hook-context.json"), irqCrosswalk); e != nil {
					r.fail("M2-CPU-B-original-ROM-IRQ-RTE-hook-context", e.Error())
					return r
				}
				if !irqCrosswalk.SelectedHandlerRTEOrderVerified || !irqCrosswalk.SampledHandlerRTEStackPointersMatched || irqCrosswalk.ExceptionFrameBusVerified || irqCrosswalk.IRQVectorOwnershipVerified || irqCrosswalk.IRQDeliveryAndSchedulerTimingParityVerified || irqCrosswalk.NativePromotionAllowed || irqCrosswalk.CompleteFunctionVerified {
					r.fail("M2-CPU-B-original-ROM-IRQ-RTE-hook-context", "unsafe IRQ proof / maturity state")
					return r
				}
				r.pass("M2-CPU-B-original-ROM-IRQ-RTE-hook-context", fmt.Sprintf("%d original-ROM checkpoint cases; selected handler-entry/RTE order and sampled SP matched; exception frame/bus/vector/scheduler/full function/native promotion remain BLOCKED", len(irqCrosswalk.Cases)))
				frameAudit, frameErr := rc6325ExceptionFrameAudit(dest, proof.CPUBROM_SHA256)
				if frameErr != nil {
					r.fail("M2-CPU-B-original-ROM-exception-frame-restoration", frameErr.Error())
					return r
				}
				if e := rc63WriteJSON(filepath.Join(dest, "cpu-b-original-rom-exception-frame-restoration.json"), frameAudit); e != nil {
					r.fail("M2-CPU-B-original-ROM-exception-frame-restoration", e.Error())
					return r
				}
				if !frameAudit.SampledExceptionFrameBytesStable || !frameAudit.SampledInterruptedPCSRRestored || !frameAudit.ImmediatePostRTEHookVerified || !frameAudit.StackBankSwitchVerified || !frameAudit.SupervisorFramePopVerified || frameAudit.ExceptionStackBusOrderVerified || frameAudit.IRQVectorACKOwnershipVerified || frameAudit.CompleteIRQDeliveryVerified || frameAudit.SchedulerParityVerified || frameAudit.CompleteFunctionVerified || frameAudit.NativePromotionAllowed || len(frameAudit.Cases) != 5 {
					r.fail("M2-CPU-B-original-ROM-exception-frame-restoration", "unsafe or incomplete exception-frame maturity")
					return r
				}
				r.pass("M2-CPU-B-original-ROM-exception-frame-restoration", fmt.Sprintf("%d authentic cases; sampled six-byte frame RAM snapshots and post-RTE original PC/SR/USP/ISP/active-SP bank selection confirmed; actual bus/vector/IRQ completeness/scheduling/function/native promotion BLOCKED", len(frameAudit.Cases)))
				busIRQ, busErr := rc6327IRQDeliveryAudit(dest, proof.CPUBROM_SHA256)
				if busErr != nil {
					r.fail("M2-CPU-B-original-ROM-IRQ-vector-and-stack-bus-order", busErr.Error())
					return r
				}
				if e := rc63WriteJSON(filepath.Join(dest, "cpu-b-original-rom-irq-vector-stack-bus-order.json"), busIRQ); e != nil {
					r.fail("M2-CPU-B-original-ROM-IRQ-vector-and-stack-bus-order", e.Error())
					return r
				}
				if !busIRQ.SampledOriginalACKObserved || !busIRQ.SelectedVectorTableReadMatched || !busIRQ.OrderedExceptionStackWritesMatched || !busIRQ.FirstHandlerHookMatched || busIRQ.HardwareVectorOwnershipVerified || busIRQ.CompleteIRQDeliveryVerified || busIRQ.ExactCycleTimingVerified || busIRQ.CompleteFunctionVerified || busIRQ.NativePromotionAllowed || len(busIRQ.Cases) != 5 {
					r.fail("M2-CPU-B-original-ROM-IRQ-vector-and-stack-bus-order", "unsafe or incomplete bounded IRQ bus provenance")
					return r
				}
				r.pass("M2-CPU-B-original-ROM-IRQ-vector-and-stack-bus-order", fmt.Sprintf("%d authentic cases; ACK→vector read→exception PC/SR writes→handler hook ordered and crosschecked with frame snapshots; physical IRQ ownership/exact cycles/all IRQs/function/native promotion BLOCKED", len(busIRQ.Cases)))
				branchFrontier, branchFrontierErr := rc6328BranchFrontierAudit(dest, proof.CPUBROM_SHA256)
				if branchFrontierErr != nil {
					r.fail("M2-CPU-B-original-ROM-0516-BNE-condition-and-next-PC", branchFrontierErr.Error())
					return r
				}
				if e := rc63WriteJSON(filepath.Join(dest, "cpu-b-original-rom-0516-bne-condition-next-pc.json"), branchFrontier); e != nil {
					r.fail("M2-CPU-B-original-ROM-0516-BNE-condition-and-next-PC", e.Error())
					return r
				}
				if !branchFrontier.RecordedSRMatchesSelectedOriginalHooks || !branchFrontier.ObservedNextPCMatchesBranchCondition || !branchFrontier.OrderedVisitsMatchOriginalROMEdges || branchFrontier.Observed0518Fallthrough || branchFrontier.EntirePathCoverageVerified || branchFrontier.EnclosingFunctionCFGVerified || branchFrontier.NativePromotionAllowed || len(branchFrontier.Cases) != 5 {
					r.fail("M2-CPU-B-original-ROM-0516-BNE-condition-and-next-PC", "unsafe or incomplete bounded branch-condition audit")
					return r
				}
				r.pass("M2-CPU-B-original-ROM-0516-BNE-condition-and-next-PC", "5 authentic cases; recorded BNE SR.Z and immediate original-ROM successor align with 0x0516->0x052C edges; 0x0518/unreviewed CFG/full function/native promotion BLOCKED")
				btstSource, btstErr := rc6330BTSTSourceAudit(dest, proof.CPUBROM_SHA256)
				if btstErr != nil {
					r.fail("M2-CPU-B-original-ROM-050E-BTST-source-bit7", btstErr.Error())
					return r
				}
				if e := rc63WriteJSON(filepath.Join(dest, "cpu-b-original-rom-050e-btst-bit7-source.json"), btstSource); e != nil {
					r.fail("M2-CPU-B-original-ROM-050E-BTST-source-bit7", e.Error())
					return r
				}
				if !btstSource.GhidraBTSTSourceOperandVerified || !btstSource.OriginalByteMatchesBNEZeroFlag || !btstSource.ImmediateSuccessorMatchesOrderedHooks || !btstSource.MatchesOriginalROMBranchDecisions || btstSource.OriginalRAMReadIsBusTransactionProof || btstSource.Observed0518Fallthrough || btstSource.CompleteFunctionCFGVerified || btstSource.NativePromotionAllowed || len(btstSource.Cases) != 5 {
					r.fail("M2-CPU-B-original-ROM-050E-BTST-source-bit7", "unsafe or incomplete source-byte provenance")
					return r
				}
				r.pass("M2-CPU-B-original-ROM-050E-BTST-source-bit7", "5 original-ROM cases; genuine BTST.B bit7 operand at 0x100802 crosschecked with immediate 0x0516 SR.Z and 0x052C; 0x0518/function/native promotion BLOCKED")
				// RC6.3.31: build one multi-source, fail-closed function-blocker
				// dossier and writer witness across all five original-ROM runs.
				// No writer observed is evidence of an incomplete window, NOT
				// permission to infer producer PC or promote a function.
				writerDossier, writerErr := rc6331FunctionAndWriterDossier(dest, proof.CPUBROM_SHA256)
				if writerErr != nil {
					r.fail("M2-CPU-B-autonomous-function-frontier-and-writer-dossier", writerErr.Error())
					return r
				}
				if e := rc63WriteJSON(filepath.Join(dest, "cpu-b-automated-function-and-writer-dossier.json"), writerDossier); e != nil {
					r.fail("M2-CPU-B-autonomous-function-frontier-and-writer-dossier", e.Error())
					return r
				}
				if writerDossier.NativePromotionAllowed || writerDossier.VerifiedCompleteCFG ||
					writerDossier.WriterProvenanceComplete || writerDossier.WholeFunctionNativeEquivalence ||
					writerDossier.Observed0518Fallthrough || len(writerDossier.Cases) != 5 ||
					writerDossier.ObservedFunctionEdges < 1 {
					r.fail("M2-CPU-B-autonomous-function-frontier-and-writer-dossier", "unsafe function dossier maturity")
					return r
				}
				r.pass("M2-CPU-B-autonomous-function-frontier-and-writer-dossier",
					"5 cases crosschecked: CPU-A/B 0x100802 writer windows + original BTST, BNE and observed CFG; missing provenance explicitly BLOCKED, no whole-function promotion")
				frontier6332, boundaryErr := rc6332FunctionBoundaryEvidence(writerDossier)
				if boundaryErr != nil {
					r.fail("M2-RC6.3.32-boundary-confidence", boundaryErr.Error())
					return r
				}
				if err := rc63WriteJSON(filepath.Join(dest, "cpu-b-rc6332-boundary-confidence.json"), frontier6332); err != nil {
					r.fail("M2-RC6.3.32-boundary-confidence", err.Error())
					return r
				}
				r.pass("M2-RC6.3.32-boundary-confidence", "genuine CPU-B 0x04A8 writer counts and preserved bit7 classified; full CFG/0x0518/native promotion BLOCKED")
				replay, replayErr := rc6332Smoke(dest, root)
				if replayErr != nil {
					r.fail("M2-RC6.3.32-executable-replay-reducer-smoke", replayErr.Error())
					return r
				}
				if replay.NativePromotionAllowed || replay.EvidenceClass != "SYNTHETIC_REPLAY_FIXTURE" {
					r.fail("M2-RC6.3.32-executable-replay-reducer-smoke", "unsafe replay evidence classification")
					return r
				}
				r.pass("M2-RC6.3.32-executable-replay-reducer-smoke", "two actual subprocesses per bound, SHA-pinned synthetic checkpoint, first divergence frame 5; genuine full-function replay NOT YET PROVEN")
				// RC6.3.33: GENUINE checkpoint parity at multiple frame bounds,
				// using original ROM, Ghidra-generated pair and forced fallback.
				// On any mismatching semantic stream the subprocess replay
				// reducer is automatically invoked and evidence retained.
				genuine6333, genuine6333Err := rc6333GenuinePairReplay(root, dest, romDir, filepath.Join(dest, filepath.FromSlash(proof.PairLibrary)))
				_ = rc63WriteJSON(filepath.Join(dest, "cpu-b-rc6333-genuine-checkpoint-replay.json"), genuine6333)
				if genuine6333Err != nil {
					r.fail("M5-RC6.3.33-genuine-CPU-B-checkpoint-replay-and-rollback", genuine6333Err.Error())
					return r
				}
				if !genuine6333.BoundedPairParityVerified || !genuine6333.CleanConfirmation || !genuine6333.NegativeTraceMutationDetected || genuine6333.NativePromotionAllowed || genuine6333.WholeFunctionVerified || len(genuine6333.Bounds) != 2 {
					r.fail("M5-RC6.3.33-genuine-CPU-B-checkpoint-replay-and-rollback", "untrusted genuine replay or unsafe whole-function assertion")
					return r
				}
				r.pass("M5-RC6.3.33-genuine-CPU-B-checkpoint-replay-and-rollback", "genuine checkpoint original/Ghidra-generated CPU-B pair/forced rollback across frames 6337+6340; CPU/R16/IRQ/machine/raw-bus parity, fresh deterministic confirmation and controlled negative mutation PASS; full function promotion BLOCKED")

			} else {
				r.warn("M5-CPU-B-native-live", "not executed; use --cpu-b-live to require genuine CPU-B ROM/native/rollback replay")
			}
		} else {
			r.warn("M1-M4-CPU-B-Ghidra", "not executed; use --cpu-b-discover on Windows with Ghidra and original ROMs; CPU-B native replacement remains prohibited")
		}

	} else {
		r.warn("M5-live-function", "offline-only run; use --live-health after Windows qualification to require real native substitution")
	}
	// Cross-stage final report disambiguates early-stage not-native messages
	// from the later genuine single-opcode live milestone, and keeps the
	// new two-instruction generated contract explicitly NOT live.
	maturity, maturityErr := rc6312WriteCurrentMaturity(dest)
	if maturityErr != nil {
		r.fail("M7-reconciled-candidate-maturity", maturityErr.Error())
		return r
	}
	status["currentCandidateMaturity"] = maturity
	if err := rc6316ReconcilePortfolio(dest, root, &portfolio, maturity); err != nil {
		r.fail("M7-final-candidate-portfolio-reconciliation", err.Error())
		return r
	}
	r.pass("M7-final-candidate-portfolio-reconciliation", "CPU-B single and pair bounded LIVE stages match authentic proof; other candidates remain unpromoted and persistent ledger reconciled")
	r.pass("M7-reconciled-candidate-maturity", fmt.Sprintf("CPU-B BEQ %s; Ghidra two-instruction pair %s; no function/autonomous promotion", maturity.CPUBBEQ, maturity.CPUBPair))
	status["staticBasicBlocksCompiled"] = len(candidates)
	status["generatedHealthSharedLibrary"] = "chq_rc63_health (unverified live, opt-in guarded)"
	if opts["live-health"] != "yes" {
		status["runtimeDifferential"] = "NOT_RUN: use --live-health for bounded target-health original/native/rollback replay"
	} else {
		status["runtimeDifferential"] = "CONTROLLED_BRANCH_PROOF_ONLY: see branch-matrix-proof.json; not naturally observed complete coverage"
	}
	r.warn("M6-M7", "CPU-B single-opcode proof if enabled is bounded; neither complete CPU-A/B function boundaries nor autonomous multi-routine promotion are certified")
	return r
}
