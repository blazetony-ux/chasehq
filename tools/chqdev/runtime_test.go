package main

import (
	"encoding/json"
	"fmt"
	"net/http"
	"net/http/httptest"
	"os"
	"path/filepath"
	"strings"
	"sync/atomic"
	"testing"
	"time"
)

func TestRuntimeCheck(t *testing.T) {
	release := "0.66.9.0-test"
	mux := http.NewServeMux()
	mux.HandleFunc("/api/health", func(w http.ResponseWriter, r *http.Request) {
		json.NewEncoder(w).Encode(map[string]any{"ok": true, "frontendBootstrap": "ready"})
	})
	mux.HandleFunc("/api/v1/status", func(w http.ResponseWriter, r *http.Request) {
		json.NewEncoder(w).Encode(map[string]any{"build": release, "frame": 1})
	})
	mux.HandleFunc("/api/v1/mobile/pairing", func(w http.ResponseWriter, r *http.Request) {
		json.NewEncoder(w).Encode(map[string]any{"ok": true, "url": "/console"})
	})
	mux.HandleFunc("/api/v1/script/runs", func(w http.ResponseWriter, r *http.Request) {
		json.NewEncoder(w).Encode(map[string]any{"runs": []any{}})
	})
	srv := httptest.NewServer(mux)
	defer srv.Close()
	rep := runtimeCheck(t.TempDir(), Config{Release: release}, srv.URL)
	if rep.Status != "PASS" {
		t.Fatalf("runtime check failed: %+v", rep.Checks)
	}
}

func TestRuntimeStartupCheckSkipsAuthoritativeHistory(t *testing.T) {
	release := "0.66.9.0-test"
	var historyCalls int32
	mux := http.NewServeMux()
	mux.HandleFunc("/api/health", func(w http.ResponseWriter, r *http.Request) {
		json.NewEncoder(w).Encode(map[string]any{"ok": true, "frontendBootstrap": "ready"})
	})
	mux.HandleFunc("/api/v1/status", func(w http.ResponseWriter, r *http.Request) {
		json.NewEncoder(w).Encode(map[string]any{"build": release, "frame": 1})
	})
	mux.HandleFunc("/api/v1/mobile/pairing", func(w http.ResponseWriter, r *http.Request) {
		json.NewEncoder(w).Encode(map[string]any{"ok": true, "url": "/console"})
	})
	mux.HandleFunc("/api/v1/script/runs", func(w http.ResponseWriter, r *http.Request) {
		atomic.AddInt32(&historyCalls, 1)
		time.Sleep(50 * time.Millisecond)
		json.NewEncoder(w).Encode(map[string]any{"runs": []any{}})
	})
	srv := httptest.NewServer(mux)
	defer srv.Close()
	rep := runtimeStartupCheck(t.TempDir(), Config{Release: release}, srv.URL)
	if rep.Status != "PASS" {
		t.Fatalf("startup check failed: %+v", rep.Checks)
	}
	if atomic.LoadInt32(&historyCalls) != 0 {
		t.Fatalf("startup readiness called authoritative history %d time(s)", historyCalls)
	}
}

func TestRuntimeHistoryCheckIsSeparateGate(t *testing.T) {
	var historyCalls int32
	mux := http.NewServeMux()
	mux.HandleFunc("/api/v1/script/runs", func(w http.ResponseWriter, r *http.Request) {
		atomic.AddInt32(&historyCalls, 1)
		json.NewEncoder(w).Encode(map[string]any{"runs": []any{}})
	})
	srv := httptest.NewServer(mux)
	defer srv.Close()
	rep := runtimeHistoryCheck(t.TempDir(), Config{Release: "0.66.9.0-test"}, srv.URL)
	if rep.Status != "PASS" {
		t.Fatalf("history gate failed: %+v", rep.Checks)
	}
	if atomic.LoadInt32(&historyCalls) != 1 {
		t.Fatalf("history gate calls=%d want 1", historyCalls)
	}
}

func TestFocusedRegressionOrchestration(t *testing.T) {
	root := t.TempDir()
	rel := "research/scripts/regression/focused.chqscript"
	if err := os.MkdirAll(filepath.Dir(filepath.Join(root, filepath.FromSlash(rel))), 0755); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(root, filepath.FromSlash(rel)), []byte("echo ok\n"), 0644); err != nil {
		t.Fatal(err)
	}
	var calls int32
	mux := http.NewServeMux()
	mux.HandleFunc("/api/v1/script", func(w http.ResponseWriter, r *http.Request) {
		n := atomic.AddInt32(&calls, 1)
		var body map[string]any
		json.NewDecoder(r.Body).Decode(&body)
		if n == 1 {
			json.NewEncoder(w).Encode(map[string]any{"ok": true, "runId": "001", "build": "0.66.9.0-test", "sessionId": "s", "count": 1, "results": []any{map[string]any{"output": "ok"}}})
			return
		}
		if body["finalizeRun"] != true {
			t.Errorf("second request did not finalize: %#v", body)
		}
		json.NewEncoder(w).Encode(map[string]any{"ok": true, "status": "PASS", "runId": "001", "build": "0.66.9.0-test"})
	})
	srv := httptest.NewServer(mux)
	defer srv.Close()
	rep := regressionCommand(root, Config{Release: "0.66.9.0-test", FocusedRegression: rel}, "focused", srv.URL)
	if rep.Status != "PASS" {
		t.Fatalf("regression orchestration failed: %+v", rep.Checks)
	}
	if atomic.LoadInt32(&calls) != 2 {
		t.Fatalf("expected run+finalize, calls=%d", calls)
	}
}

func TestGoScriptRunnerControlFlowAndIncludes(t *testing.T) {
	root := t.TempDir()
	base := filepath.Join(root, "research", "scripts", "regression")
	if err := os.MkdirAll(base, 0755); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(base, "child.chqscript"), []byte("repeat 2\n echo child-${repeat_index}\nend\n"), 0644); err != nil {
		t.Fatal(err)
	}
	rel := "research/scripts/regression/full.chqscript"
	src := "include \"regression/child.chqscript\"\nif status paused = 1\n echo paused\nelse\n echo running\nend\ntry\n echo body\nfinally\n echo cleanup\nend\n"
	if err := os.WriteFile(filepath.Join(root, filepath.FromSlash(rel)), []byte(src), 0644); err != nil {
		t.Fatal(err)
	}
	var commands []string
	var authoritativeSource string
	var finalizedCommandCount int
	mux := http.NewServeMux()
	mux.HandleFunc("/api/status", func(w http.ResponseWriter, r *http.Request) { fmt.Fprint(w, "paused=1 frame=2352") })
	mux.HandleFunc("/api/v1/script", func(w http.ResponseWriter, r *http.Request) {
		var body map[string]any
		json.NewDecoder(r.Body).Decode(&body)
		if body["transient"] == true {
			json.NewEncoder(w).Encode(map[string]any{"ok": true, "results": []any{map[string]any{"output": "OK"}}})
			return
		}
		if body["finalizeRun"] == true {
			if v, ok := body["commandCount"].(float64); ok {
				finalizedCommandCount = int(v)
			}
			json.NewEncoder(w).Encode(map[string]any{"ok": true, "status": "PASS", "runId": "001", "build": "0.66.9.0-test"})
			return
		}
		if len(commands) == 0 {
			authoritativeSource = fmt.Sprint(body["runSource"])
		} else if _, present := body["runSource"]; present {
			t.Errorf("runSource should only be sent while creating the authoritative run")
		}
		commands = append(commands, fmt.Sprint(body["script"]))
		json.NewEncoder(w).Encode(map[string]any{"ok": true, "runId": "001", "build": "0.66.9.0-test", "sessionId": "s", "count": 1, "results": []any{map[string]any{"output": "OK"}}})
	})
	srv := httptest.NewServer(mux)
	defer srv.Close()
	rep := regressionCommand(root, Config{Release: "0.66.9.0-test", ScriptRoot: "research/scripts", FullRegression: rel}, "full", srv.URL)
	if rep.Status != "PASS" {
		t.Fatalf("full runner failed: %+v data=%+v", rep.Checks, rep.Data)
	}
	got := strings.Join(commands, "|")
	for _, want := range []string{"echo child-1", "echo child-2", "echo paused", "echo body", "echo cleanup"} {
		if !strings.Contains(got, want) {
			t.Fatalf("missing %q in %s", want, got)
		}
	}
	if strings.Contains(got, "echo running") {
		t.Fatalf("else branch unexpectedly executed: %s", got)
	}
	if authoritativeSource != src {
		t.Fatalf("authoritative source was not preserved; got %q want %q", authoritativeSource, src)
	}
	if finalizedCommandCount != len(commands) || finalizedCommandCount != 5 {
		t.Fatalf("finalized command count=%d commands=%d; expected 5", finalizedCommandCount, len(commands))
	}
}

func TestMemoryPredicateHexSemantics(t *testing.T) {
	for _, tc := range []struct{ a, b string }{{"0x0060", "0x0060"}, {"0x0011", "0x0011"}, {"0x000E", "0x000E"}} {
		ok, err := compareScriptValues(tc.a, "=", tc.b)
		if err != nil || !ok {
			t.Fatalf("hex equality failed %s %s: ok=%v err=%v", tc.a, tc.b, ok, err)
		}
	}
}

func TestGoScriptRunnerParameterDefaults(t *testing.T) {
	root := t.TempDir()
	rel := "research/scripts/regression/focused-defaults.chqscript"
	p := filepath.Join(root, filepath.FromSlash(rel))
	if err := os.MkdirAll(filepath.Dir(p), 0755); err != nil {
		t.Fatal(err)
	}
	src := "# param frames type=select values=2,4,8 default=4\necho frames=${frames}\napi control.run-frames frames=${frames}\n"
	if err := os.WriteFile(p, []byte(src), 0644); err != nil {
		t.Fatal(err)
	}
	var commands []string
	mux := http.NewServeMux()
	mux.HandleFunc("/api/v1/script", func(w http.ResponseWriter, r *http.Request) {
		var body map[string]any
		json.NewDecoder(r.Body).Decode(&body)
		if body["finalizeRun"] == true {
			json.NewEncoder(w).Encode(map[string]any{"ok": true, "status": "PASS", "runId": "001", "build": "0.66.9.0-test"})
			return
		}
		commands = append(commands, fmt.Sprint(body["script"]))
		json.NewEncoder(w).Encode(map[string]any{"ok": true, "runId": "001", "build": "0.66.9.0-test", "sessionId": "s", "count": 1, "results": []any{map[string]any{"output": "OK"}}})
	})
	srv := httptest.NewServer(mux)
	defer srv.Close()
	rep := regressionCommand(root, Config{Release: "0.66.9.0-test", ScriptRoot: "research/scripts", FocusedRegression: rel}, "focused", srv.URL)
	if rep.Status != "PASS" {
		t.Fatalf("parameter-default runner failed: %+v data=%+v", rep.Checks, rep.Data)
	}
	got := strings.Join(commands, "|")
	for _, want := range []string{"echo frames=4", "api control.run-frames frames=4"} {
		if !strings.Contains(got, want) {
			t.Fatalf("default parameter not resolved; missing %q in %s", want, got)
		}
	}
	if strings.Contains(got, "${frames}") {
		t.Fatalf("unresolved parameter leaked to Workbench: %s", got)
	}
}

func TestTupleLoopCompilation(t *testing.T) {
	items, err := compileScriptLines(strings.Split("for address,width in [(1002AE,16),(1002D8,8)]\n api memory.read cpu=A address=${address} width=${width}\nend\n", "\n"), map[string]string{})
	if err != nil {
		t.Fatal(err)
	}
	if len(items) != 2 {
		t.Fatalf("tuple loop expanded to %d items; want 2", len(items))
	}
	if items[0].Command != "api memory.read cpu=A address=1002AE width=16" || items[1].Command != "api memory.read cpu=A address=1002D8 width=8" {
		t.Fatalf("unexpected tuple expansion: %#v", items)
	}
	if items[0].Line != 2 || items[1].Line != 2 {
		t.Fatalf("tuple source lines=%d,%d want 2,2", items[0].Line, items[1].Line)
	}
}

func TestNestedSourceLinesAndClientWaitEvidence(t *testing.T) {
	root := t.TempDir()
	rel := "research/scripts/regression/source-lines.chqscript"
	p := filepath.Join(root, filepath.FromSlash(rel))
	if err := os.MkdirAll(filepath.Dir(p), 0755); err != nil {
		t.Fatal(err)
	}
	src := "echo first\nwait 1\nfor address,width in [(1002AE,16)]\n  try\n    api memory.read cpu=A address=${address} width=${width}\n  finally\n    echo cleanup\n  end\nend\n"
	if err := os.WriteFile(p, []byte(src), 0644); err != nil {
		t.Fatal(err)
	}
	var lines []int
	var clientFlags []bool
	var resolvedSource string
	var finalizedCount int
	mux := http.NewServeMux()
	mux.HandleFunc("/api/v1/script", func(w http.ResponseWriter, r *http.Request) {
		var body map[string]any
		json.NewDecoder(r.Body).Decode(&body)
		if body["finalizeRun"] == true {
			if v, ok := body["commandCount"].(float64); ok {
				finalizedCount = int(v)
			}
			json.NewEncoder(w).Encode(map[string]any{"ok": true, "status": "PASS", "runId": "001", "build": "0.66.9.3-RC3"})
			return
		}
		if v, ok := body["sourceLine"].(float64); ok {
			lines = append(lines, int(v))
		}
		clientFlags = append(clientFlags, body["clientRecord"] == true)
		if resolvedSource == "" {
			resolvedSource = fmt.Sprint(body["runResolvedSource"])
		}
		json.NewEncoder(w).Encode(map[string]any{"ok": true, "runId": "001", "build": "0.66.9.3-RC3", "sessionId": "s", "count": 1, "results": []any{map[string]any{"output": "OK"}}})
	})
	srv := httptest.NewServer(mux)
	defer srv.Close()
	rep := regressionCommand(root, Config{Release: "0.66.9.3-RC3", ScriptRoot: "research/scripts", FocusedRegression: rel}, "focused", srv.URL)
	if rep.Status != "PASS" {
		t.Fatalf("runner failed: %+v data=%+v", rep.Checks, rep.Data)
	}
	want := []int{1, 2, 5, 7}
	if fmt.Sprint(lines) != fmt.Sprint(want) {
		t.Fatalf("source lines=%v want %v", lines, want)
	}
	if len(clientFlags) != 4 || clientFlags[0] || !clientFlags[1] || clientFlags[2] || clientFlags[3] {
		t.Fatalf("clientRecord flags=%v; only wait should be client-recorded", clientFlags)
	}
	if finalizedCount != 4 {
		t.Fatalf("final command count=%d want 4", finalizedCount)
	}
	if resolvedSource != src {
		t.Fatalf("resolved source mismatch: %q", resolvedSource)
	}
}

func TestScriptCommandLimitEnv(t *testing.T) {
	t.Setenv("CHQ_SCRIPT_MAX_COMMANDS", "3072")
	if got := scriptCommandLimit(); got != 3072 {
		t.Fatalf("limit=%d want 3072", got)
	}
	t.Setenv("CHQ_SCRIPT_MAX_COMMANDS", "99999")
	if got := scriptCommandLimit(); got != 2048 {
		t.Fatalf("invalid override limit=%d want default 2048", got)
	}
}

func TestFullRegressionConsolidatedCommandBudget(t *testing.T) {
	root, err := filepath.Abs(filepath.Join("..", ".."))
	if err != nil {
		t.Fatal(err)
	}
	p := filepath.Join(root, "research", "scripts", "regression", "full-regression.chqscript")
	b, err := os.ReadFile(p)
	if err != nil {
		t.Fatal(err)
	}
	src, err := expandScriptIncludes(root, "research/scripts", string(b), 0)
	if err != nil {
		t.Fatal(err)
	}
	items, err := compileScriptLines(strings.Split(src, "\n"), scriptDefaultVars(src))
	if err != nil {
		t.Fatal(err)
	}
	structural := countScriptItems(items)
	if structural < 1176 || structural > 1536 {
		t.Fatalf("Full Regression structural command count %d outside certified coverage/budget range [1176,1536]; never silently drop tests", structural)
	}
	if structural > scriptCommandLimit() {
		t.Fatalf("Full Regression count %d exceeds Go runner limit %d", structural, scriptCommandLimit())
	}
	for _, historical := range []string{
		"regression-v06690-rc32.chqscript", "regression-v06690-rc33.chqscript", "regression-v06690-rc34.chqscript",
		"regression-v06690-rc341.chqscript", "regression-v06690-rc342.chqscript", "regression-v06690-rc343.chqscript",
		"regression-v06690-rc344.chqscript", "regression-v06690-rc345.chqscript", "regression-v06690-rc346.chqscript",
		"regression-v06690-rc347.chqscript", "regression-v06690-rc348.chqscript", "regression-v06690-rc349.chqscript",
		"regression-v06691.chqscript", "regression-v06692-rc3.chqscript", "regression-v06692-rc4.chqscript", "regression-v06692.chqscript",
	} {
		if strings.Contains(string(b), historical) {
			t.Fatalf("historical duplicate still auto-included: %s", historical)
		}
	}
	t.Logf("Full Regression consolidated structural command count: %d (top-level items=%d)", structural, len(items))
}
