package main

import (
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"strings"
	"time"
)

const (
	runtimeStartupHTTPTimeout = 10 * time.Second
	runtimeHistoryHTTPTimeout = 30 * time.Second
)

func workbenchBase(cfg Config, override string) string {
	if strings.TrimSpace(override) != "" {
		return strings.TrimRight(strings.TrimSpace(override), "/")
	}
	if strings.TrimSpace(cfg.WorkbenchURL) != "" {
		return strings.TrimRight(strings.TrimSpace(cfg.WorkbenchURL), "/")
	}
	return "http://127.0.0.1:37680"
}

func httpGetJSON(client *http.Client, url string, dst any) error {
	resp, err := client.Get(url)
	if err != nil {
		return err
	}
	defer resp.Body.Close()
	if resp.StatusCode < 200 || resp.StatusCode >= 300 {
		b, _ := io.ReadAll(io.LimitReader(resp.Body, 4096))
		return fmt.Errorf("HTTP %d: %s", resp.StatusCode, strings.TrimSpace(string(b)))
	}
	return json.NewDecoder(resp.Body).Decode(dst)
}

func elapsedDetail(start time.Time, detail string) string {
	return fmt.Sprintf("%s; elapsed=%s", detail, time.Since(start).Round(time.Millisecond))
}

func appendChecks(dst, src *Report) {
	if dst == nil || src == nil {
		return
	}
	dst.Checks = append(dst.Checks, src.Checks...)
	if src.Status == "FAIL" {
		dst.Status = "FAIL"
	}
}

// runtimeStartupCheck is deliberately lightweight. It proves the Workbench,
// exact native build identity and mobile descriptor are immediately usable,
// without triggering the authoritative run-history scan. The latter can walk
// many sibling release/evidence trees and is therefore a qualification gate,
// not a startup/readiness dependency.
func runtimeStartupCheck(root string, cfg Config, overrideURL string) *Report {
	r := newReport("runtime startup check", cfg.Release)
	base := workbenchBase(cfg, overrideURL)
	client := &http.Client{Timeout: runtimeStartupHTTPTimeout}
	r.Data["workbenchUrl"] = base

	started := time.Now()
	var health map[string]any
	if err := httpGetJSON(client, base+"/api/health", &health); err != nil {
		r.fail("workbench-health", elapsedDetail(started, err.Error()))
		return r
	}
	if v, ok := health["frontendBootstrap"].(string); ok && v != "" && !strings.EqualFold(v, "ready") {
		r.fail("frontend-bootstrap", elapsedDetail(started, "reported "+v))
		return r
	}
	r.pass("workbench-health", elapsedDetail(started, "HTTP/API ready"))

	started = time.Now()
	var status map[string]any
	if err := httpGetJSON(client, base+"/api/v1/status", &status); err != nil {
		r.fail("native-status", elapsedDetail(started, err.Error()))
		return r
	}
	build := fmt.Sprint(status["build"])
	if build != "" && build != "<nil>" && build != cfg.Release {
		r.fail("native-identity", elapsedDetail(started, fmt.Sprintf("runtime=%s expected=%s", build, cfg.Release)))
		return r
	}
	if build == cfg.Release {
		r.pass("native-identity", elapsedDetail(started, build))
	} else {
		r.pass("native-status", elapsedDetail(started, "responsive"))
	}

	started = time.Now()
	var pairing map[string]any
	if err := httpGetJSON(client, base+"/api/v1/mobile/pairing", &pairing); err != nil {
		r.fail("mobile-pairing", elapsedDetail(started, err.Error()))
		return r
	}
	r.pass("mobile-pairing", elapsedDetail(started, "descriptor available"))
	return r
}

// runtimeHistoryCheck proves the authoritative history endpoint separately
// from startup. It intentionally has a larger timeout because a cold history
// request may need to enumerate accumulated evidence across sibling builds.
func runtimeHistoryCheck(root string, cfg Config, overrideURL string) *Report {
	r := newReport("runtime history check", cfg.Release)
	base := workbenchBase(cfg, overrideURL)
	client := &http.Client{Timeout: runtimeHistoryHTTPTimeout}
	r.Data["workbenchUrl"] = base
	started := time.Now()
	var history map[string]any
	if err := httpGetJSON(client, base+"/api/v1/script/runs?limit=1", &history); err != nil {
		r.fail("run-history", elapsedDetail(started, err.Error()))
		return r
	}
	r.pass("run-history", elapsedDetail(started, "authoritative endpoint available"))
	return r
}

// runtimeCheck remains the explicit comprehensive user-facing check. Unlike
// startup readiness it includes the potentially expensive authoritative
// history endpoint and reports per-component elapsed time.
func runtimeCheck(root string, cfg Config, overrideURL string) *Report {
	r := newReport("runtime check", cfg.Release)
	startup := runtimeStartupCheck(root, cfg, overrideURL)
	r.Data["startup"] = startup
	appendChecks(r, startup)
	if startup.Status == "FAIL" {
		return r
	}
	history := runtimeHistoryCheck(root, cfg, overrideURL)
	r.Data["history"] = history
	appendChecks(r, history)
	return r
}
