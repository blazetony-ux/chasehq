package main

// RC6.3.4 records independent ROM/runtime evidence for the HEALTH *observed*
// entry/RTS/return boundary. It deliberately does not equate the observation
// with an exhaustive Ghidra function-boundary or caller/callgraph review.
import (
	"encoding/binary"
	"encoding/json"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"sort"
	"strconv"
	"strings"
)

type rc634BoundaryCase struct {
	Case                               string   `json:"case"`
	ReturnPC                           string   `json:"observedReturnPC"`
	EntrySP                            string   `json:"observedEntrySP"`
	OriginalNativeRollbackEntriesEqual bool     `json:"originalNativeRollbackEntriesEqual"`
	ReturnMatchesOriginalCPUState      bool     `json:"returnMatchesOriginalCPUState"`
	CallerEncoding                     string   `json:"callerEncoding"`
	CallerTargetVerified               bool     `json:"callerTargetVerified"`
	DecodedCallerTarget                string   `json:"decodedCallerTarget"`
	CallerInstructionPC                string   `json:"callerInstructionPC"`
	DynamicallyObservedCallTarget      string   `json:"dynamicallyObservedCallTarget"`
	DynamicCallerToTailVerified        bool     `json:"dynamicCallerToTailVerified"`
	ExecutedCallerToTailPCPath         []string `json:"executedCallerToTailPCPath,omitempty"`
}
type rc634BoundaryAudit struct {
	Schema                                string              `json:"schema"`
	Status                                string              `json:"status"`
	CPU                                   string              `json:"cpu"`
	CandidateEntry                        string              `json:"candidateEntry"`
	CandidateEndExclusive                 string              `json:"candidateEndExclusive"`
	AuthenticROMRTS                       bool                `json:"authenticRomRTS"`
	ObservedFunctionTailOnly              bool                `json:"observedFunctionTailOnly"`
	DistinctObservedPaths                 int                 `json:"distinctObservedPaths"`
	Cases                                 []rc634BoundaryCase `json:"cases"`
	FunctionBoundaryIndependentlyReviewed bool                `json:"functionBoundaryIndependentlyReviewed"`
	AllCallersDiscovered                  bool                `json:"allCallersDiscovered"`
	FunctionPromotionAllowed              bool                `json:"functionPromotionAllowed"`
	Limitations                           []string            `json:"limitations"`
}

func rc634ReadWord(rom []byte, at uint64) (uint16, bool) {
	if at+2 > uint64(len(rom)) {
		return 0, false
	}
	return binary.BigEndian.Uint16(rom[at : at+2]), true
}
func rc634ReadLong(rom []byte, at uint64) (uint32, bool) {
	if at+4 > uint64(len(rom)) {
		return 0, false
	}
	return binary.BigEndian.Uint32(rom[at : at+4]), true
}

// This decodes the actual target of supported direct callsite encodings.
// The target is NOT assumed to equal the generated candidate tail entry: a
// BSR to an enclosing routine can legitimately fall through into the tail.
type rc635Call struct {
	Name       string
	PC, Target uint64
	Decoded    bool
}

func rc635DecodeCaller(rom []byte, ret uint64) rc635Call {
	unresolved := rc635Call{Name: "CALLER_OPCODE_UNRESOLVED"}
	if ret >= 6 {
		op, ok := rc634ReadWord(rom, ret-6)
		addr, ok2 := rc634ReadLong(rom, ret-4)
		if ok && ok2 && op == 0x4eb9 {
			return rc635Call{"JSR absolute-long", ret - 6, uint64(addr), true}
		}
	}
	if ret >= 4 {
		op, ok := rc634ReadWord(rom, ret-4)
		displacement, ok2 := rc634ReadWord(rom, ret-2)
		if ok && ok2 {
			if op == 0x4eb8 {
				return rc635Call{"JSR absolute-word", ret - 4, uint64(uint32(int32(int16(displacement)))), true}
			}
			if op == 0x4eba {
				return rc635Call{"JSR PC+d16", ret - 4, uint64(int64(ret-2) + int64(int16(displacement))), true}
			}
			if op == 0x6100 {
				return rc635Call{"BSR word", ret - 4, uint64(int64(ret-2) + int64(int16(displacement))), true}
			}
		}
	}
	if ret >= 2 {
		op, ok := rc634ReadWord(rom, ret-2)
		if ok && op>>8 == 0x61 && op&0xff != 0 {
			return rc635Call{"BSR byte", ret - 2, uint64(int64(ret) + int64(int8(op&0xff))), true}
		}
	}
	return unresolved
}
func rc634Caller(rom []byte, ret, target uint64) (string, bool) {
	decoded := rc635DecodeCaller(rom, ret)
	return decoded.Name, decoded.Decoded && decoded.Target == target
}

type rc635DynamicPath struct {
	Return   uint64
	CallPC   uint64
	CalledPC uint64
	Seen     bool
	PCs      []string
}

func rc635ParseDynamicPath(data []byte, caseName string, ret uint64) (rc635DynamicPath, error) {
	out := rc635DynamicPath{Return: ret}
	rows := strings.Split(strings.TrimSpace(string(data)), "\n")
	if len(rows) != 1 {
		return out, fmt.Errorf("expected one dynamic entry path for %s, got %d", caseName, len(rows))
	}
	f := strings.Split(strings.TrimSuffix(rows[0], "\r"), "\t")
	if len(f) != 6 || f[1] != caseName {
		return out, fmt.Errorf("invalid dynamic caller path columns/case in %s", caseName)
	}
	recorded, err := strconv.ParseUint(f[2], 16, 32)
	if err != nil || recorded != ret {
		return out, fmt.Errorf("caller path return PC differs from stack in %s", caseName)
	}
	if f[3] == "UNOBSERVED" && f[4] == "UNOBSERVED" && f[5] == "UNOBSERVED" {
		return out, nil
	}
	if f[3] == "UNOBSERVED" || f[4] == "UNOBSERVED" || f[5] == "UNOBSERVED" {
		return out, fmt.Errorf("partial dynamic caller path in %s", caseName)
	}
	out.CallPC, err = strconv.ParseUint(f[3], 16, 32)
	if err != nil {
		return out, fmt.Errorf("invalid observed caller PC: %w", err)
	}
	out.CalledPC, err = strconv.ParseUint(f[4], 16, 32)
	if err != nil {
		return out, fmt.Errorf("invalid observed call destination: %w", err)
	}
	out.PCs = strings.Split(f[5], ",")
	if len(out.PCs) < 2 || len(out.PCs) > 16384 {
		return out, fmt.Errorf("invalid dynamic PC path length %d", len(out.PCs))
	}
	if strings.ToLower(out.PCs[0]) != strings.ToLower(f[3]) || strings.ToLower(out.PCs[1]) != strings.ToLower(f[4]) {
		return out, fmt.Errorf("dynamic path does not begin at callsite and destination")
	}
	if !strings.EqualFold(out.PCs[len(out.PCs)-1], "a10c") {
		return out, fmt.Errorf("dynamic path never reaches candidate entry")
	}
	for _, pc := range out.PCs {
		if _, err := strconv.ParseUint(pc, 16, 32); err != nil {
			return out, fmt.Errorf("invalid PC in dynamic path: %w", err)
		}
	}
	out.Seen = true
	return out, nil
}
func rc634ParseEntry(d []byte) (sp, ret string, e error) {
	rows := strings.Split(strings.TrimSpace(string(d)), "\n")
	if len(rows) != 1 {
		return "", "", fmt.Errorf("expected one entry record, got %d", len(rows))
	}
	f := strings.Split(rows[0], "\t")
	if len(f) != 6 {
		return "", "", fmt.Errorf("bad entry record columns %d", len(f))
	}
	if _, e = strconv.ParseUint(f[0], 10, 64); e != nil {
		return
	}
	for _, t := range f[2:] {
		if _, e = strconv.ParseUint(t, 16, 32); e != nil {
			return
		}
	}
	return f[2], f[3], nil
}
func rc634AuditHealthBoundary(dest string, candidates []rc63Candidate, rom []byte, matrix rc633BranchMatrix) (rc634BoundaryAudit, error) {
	a := rc634BoundaryAudit{Schema: "chq-rc635-observed-boundary-audit-v2", Status: "NOT_VERIFIED", CPU: "A", FunctionBoundaryIndependentlyReviewed: false, AllCallersDiscovered: false, FunctionPromotionAllowed: false, Limitations: []string{"Only one known checkpoint and controlled entry states, not full gameplay/callgraph coverage", "Ghidra auto-analysis function boundary is unreviewed; a valid observed RTS/return is not an exhaustive boundary proof", "Indirect calls or unobserved callers remain unresolved", "0xA10C is an independently verified native execution range, not a proven complete function entry"}}
	var candidate *rc63Candidate
	for i := range candidates {
		if candidates[i].Family == "conditional-health-function" {
			if candidate != nil {
				return a, errors.New("ambiguous Ghidra health function")
			}
			candidate = &candidates[i]
		}
	}
	if candidate == nil || !candidate.FullFunction || candidate.Entry != "0x0000A10C" || candidate.End != "0x0000A12A" {
		return a, errors.New("health Ghidra candidate lacks contiguous proposed function span")
	}
	a.CandidateEntry = candidate.Entry
	a.CandidateEndExclusive = candidate.End
	if word, ok := rc634ReadWord(rom, 0xA128); !ok || word != 0x4e75 {
		return a, fmt.Errorf("real ROM not RTS at A128 (0x%04X)", word)
	}
	a.AuthenticROMRTS = true
	a.DistinctObservedPaths = matrix.DistinctROMBranchPaths
	if matrix.Status != "LIVE_CONTROLLED_BRANCH_MATRIX_PASS" || a.DistinctObservedPaths < 3 || matrix.ObservedROMInstructionSites != 9 {
		return a, errors.New("cannot audit boundaries without complete controlled original-ROM matrix")
	}
	for _, c := range matrix.Cases {
		base := filepath.Join(dest, "branch-matrix", c.EntryCase, "live-health")
		orig, err := os.ReadFile(filepath.Join(base, "original", "native-health-entry.tsv"))
		if err != nil {
			return a, err
		}
		nat, err := os.ReadFile(filepath.Join(base, "generated-native", "native-health-entry.tsv"))
		if err != nil {
			return a, err
		}
		rollback, err := os.ReadFile(filepath.Join(base, "forced-mismatch", "native-health-entry.tsv"))
		if err != nil {
			return a, err
		}
		same := string(orig) == string(nat) && string(orig) == string(rollback)
		if !same {
			return a, fmt.Errorf("entry stack/return mismatch in %s", c.EntryCase)
		}
		sp, ret, err := rc634ParseEntry(orig)
		if err != nil {
			return a, err
		}
		r, err := strconv.ParseUint(ret, 16, 32)
		if err != nil {
			return a, err
		}
		if r >= 0xA10C && r < 0xA12A {
			return a, fmt.Errorf("health return PC erroneously inside function: %X", r)
		}
		state, err := os.ReadFile(filepath.Join(base, "original", "native-health-state.tsv"))
		if err != nil {
			return a, err
		}
		f := strings.Split(strings.TrimSpace(string(state)), "\t")
		if len(f) < 4 || f[1] != "rom" {
			return a, errors.New("bad original return-state record")
		}
		observed, err := strconv.ParseUint(f[2], 16, 32)
		if err != nil || observed != r {
			return a, fmt.Errorf("return PC does not match stacked address %s: %s", ret, f[2])
		}
		decoded := rc635DecodeCaller(rom, r)
		pathFile := func(mode string) string { return filepath.Join(base, mode, "native-health-caller-path.tsv") }
		origPath, err := os.ReadFile(pathFile("original"))
		if err != nil {
			return a, err
		}
		for _, mode := range []string{"generated-native", "forced-mismatch"} {
			comparison, err := os.ReadFile(pathFile(mode))
			if err != nil {
				return a, err
			}
			if string(comparison) != string(origPath) {
				return a, fmt.Errorf("original/native/rollback dynamic caller path diverges in %s/%s", c.EntryCase, mode)
			}
		}
		actual, err := rc635ParseDynamicPath(origPath, c.EntryCase, r)
		if err != nil {
			return a, err
		}
		info := rc634BoundaryCase{Case: c.EntryCase, ReturnPC: fmt.Sprintf("0x%08X", r), EntrySP: "0x" + strings.ToUpper(sp), OriginalNativeRollbackEntriesEqual: same, ReturnMatchesOriginalCPUState: true, CallerEncoding: decoded.Name, CallerTargetVerified: decoded.Decoded && decoded.Target == 0xA10C}
		if decoded.Decoded {
			info.DecodedCallerTarget = fmt.Sprintf("0x%08X", decoded.Target)
			info.CallerInstructionPC = fmt.Sprintf("0x%08X", decoded.PC)
		}
		if actual.Seen {
			info.DynamicallyObservedCallTarget = fmt.Sprintf("0x%08X", actual.CalledPC)
			info.ExecutedCallerToTailPCPath = actual.PCs
			// A decodable call whose real runtime successor differs from its
			// ROM instruction encoding indicates corrupt evidence, not an
			// alternative function boundary.
			if decoded.Decoded && (decoded.PC != actual.CallPC || decoded.Target != actual.CalledPC) {
				return a, fmt.Errorf("actual call transition contradicts ROM call encoding in %s: call=%06X predicted=%06X observed=%06X", c.EntryCase, actual.CallPC, decoded.Target, actual.CalledPC)
			}
			info.DynamicCallerToTailVerified = decoded.Decoded
		}
		if decoded.Decoded && decoded.Target != 0xA10C && !actual.Seen {
			return a, fmt.Errorf("authentic caller targets earlier ROM range but dynamic caller-to-tail transition not observed in %s", c.EntryCase)
		}
		if decoded.Decoded && decoded.Target != 0xA10C {
			// A different authentic caller target is not an error: it is
			// evidence that 0xA10C can be a tail of a larger routine.
			a.ObservedFunctionTailOnly = true
		}
		a.Cases = append(a.Cases, info)
	}
	if len(a.Cases) != 5 {
		return a, fmt.Errorf("missing live boundary cases: %d", len(a.Cases))
	}
	if a.ObservedFunctionTailOnly {
		a.Status = "OBSERVED_TAIL_ENTRY_RTS_RETURN_PASS_FULL_BOUNDARY_UNVERIFIED"
	} else {
		a.Status = "OBSERVED_ENTRY_RTS_RETURN_PASS_NOT_FULL_FUNCTION_REVIEW"
	}
	return a, nil
}

type rc634QueueItem struct {
	ID, CPU, Family, Stage, Blocker string
	CandidateEntry                  string `json:"candidateEntry"`
	LiveNativeVerified              bool   `json:"liveNativeVerified"`
	PromotionAllowed                bool   `json:"promotionAllowed"`
}
type rc634Portfolio struct {
	Schema, Release, Status           string
	Candidates                        []rc634QueueItem `json:"candidates"`
	GeneralAutonomousPromotionAllowed bool             `json:"generalAutonomousPromotionAllowed"`
}

func rc634MakePortfolio(release string, c []rc63Candidate, live bool) rc634Portfolio {
	p := rc634Portfolio{Schema: "chq-rc634-candidate-portfolio-v1", Release: release, Status: "IN_PROGRESS_NOT_AUTONOMOUS", GeneralAutonomousPromotionAllowed: false}
	for _, v := range c {
		q := rc634QueueItem{ID: v.ID, CPU: v.CPU, Family: v.Family, CandidateEntry: v.Entry, Stage: "STATIC_GENERATED", Blocker: "No live complete-range equivalence; Ghidra boundary not reviewed"}
		if v.Family == "conditional-health-function" && live {
			q.Stage = "CONTROLLED_LIVE_PARITY_VERIFIED"
			q.LiveNativeVerified = true
			q.Blocker = "Natural branch/caller coverage and full independent function boundary review outstanding"
		}
		p.Candidates = append(p.Candidates, q)
	}
	p.Candidates = append(p.Candidates, rc634QueueItem{ID: "B:DISCOVERY-PENDING", CPU: "B", Family: "unselected-runtime-candidate", Stage: "BLOCKED_MISSING_CPU_B_GHIDRA_AND_LIVE_EVIDENCE", Blocker: "Independently verified CPU-B function discovery, generated execution and rollback not present"})
	sort.Slice(p.Candidates, func(i, j int) bool { return p.Candidates[i].ID < p.Candidates[j].ID })
	return p
}
func rc634WritePortfolio(root, dest string, p rc634Portfolio) error {
	b, err := json.MarshalIndent(p, "", "  ")
	if err != nil {
		return err
	}
	b = append(b, '\n')
	if err = os.WriteFile(filepath.Join(dest, "candidate-portfolio.json"), b, 0644); err != nil {
		return err
	}
	// Persist stage state across invocations. Atomic replace, no promotion grants.
	state := filepath.Join(root, "sessions", "native-reconstruction-portfolio.json")
	if err = os.MkdirAll(filepath.Dir(state), 0755); err != nil {
		return err
	}
	tmp := state + ".tmp"
	if err = os.WriteFile(tmp, b, 0644); err != nil {
		return err
	}
	return os.Rename(tmp, state)
}

// CPU-B candidates come from REAL CPU-B instruction-hook hits during the
// natural original-ROM replay. A hot PC is NOT an inferred function boundary.
func rc634DiscoverCPU_B(dest string, p *rc634Portfolio) (int, error) {
	file := filepath.Join(dest, "branch-matrix", "natural", "live-health", "original", "rom-execution-pc-counts.tsv")
	hits, err := readNativeTsv(file)
	if err != nil {
		return 0, fmt.Errorf("CPU-B original-ROM census absent or malformed: %w", err)
	}
	type pair struct {
		pc   string
		hits int64
	}
	var found []pair
	for key, n := range hits {
		if strings.HasPrefix(key, "B:0x") && n > 0 {
			found = append(found, pair{key, n})
		}
	}
	sort.Slice(found, func(i, j int) bool {
		if found[i].hits != found[j].hits {
			return found[i].hits > found[j].hits
		}
		return found[i].pc < found[j].pc
	})
	if len(found) == 0 {
		return 0, nil
	}
	// Replace the placeholder with five actual runtime-observed CPU-B PCs.
	keep := p.Candidates[:0]
	for _, item := range p.Candidates {
		if item.CPU != "B" {
			keep = append(keep, item)
		}
	}
	p.Candidates = keep
	for i, x := range found {
		if i >= 5 {
			break
		}
		p.Candidates = append(p.Candidates, rc634QueueItem{
			ID: x.pc, CPU: "B", Family: "runtime-executed-PC", CandidateEntry: strings.TrimPrefix(x.pc, "B:"),
			Stage: "OBSERVED_RUNTIME_NOT_A_FUNCTION", Blocker: fmt.Sprintf("%d original-ROM hits; independent CPU-B Ghidra function discovery, generation and native replay required", x.hits)})
	}
	sort.Slice(p.Candidates, func(i, j int) bool { return p.Candidates[i].ID < p.Candidates[j].ID })
	return len(found), nil
}
