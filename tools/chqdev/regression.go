package main

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"path/filepath"
	"strings"
)

type scriptRunResponse struct {
	OK        bool             `json:"ok"`
	RunID     string           `json:"runId"`
	Build     string           `json:"build"`
	SessionID string           `json:"sessionId"`
	Count     int              `json:"count"`
	Results   []map[string]any `json:"results"`
}
type finalizeResponse struct {
	OK     bool   `json:"ok"`
	Status string `json:"status"`
	RunID  string `json:"runId"`
	Build  string `json:"build"`
}

func postJSON(client *http.Client, url string, body any, dst any) error {
	b, e := json.Marshal(body)
	if e != nil {
		return e
	}
	resp, e := client.Post(url, "application/json", bytes.NewReader(b))
	if e != nil {
		return e
	}
	defer resp.Body.Close()
	data, e := io.ReadAll(resp.Body)
	if e != nil {
		return e
	}
	if resp.StatusCode < 200 || resp.StatusCode >= 300 {
		return fmt.Errorf("HTTP %d: %s", resp.StatusCode, strings.TrimSpace(string(data)))
	}
	if e = json.Unmarshal(data, dst); e != nil {
		return fmt.Errorf("decode response: %w; body=%s", e, strings.TrimSpace(string(data)))
	}
	return nil
}

func regressionPath(cfg Config, kind string) (string, string, error) {
	switch kind {
	case "focused":
		if cfg.FocusedRegression == "" {
			return "", "", fmt.Errorf("focusedRegression not configured")
		}
		return cfg.FocusedRegression, cfg.Release + " focused release regression", nil
	case "full":
		if cfg.FullRegression == "" {
			return "", "", fmt.Errorf("fullRegression not configured")
		}
		return cfg.FullRegression, "Full Regression Suite", nil
	default:
		return "", "", fmt.Errorf("expected focused or full")
	}
}

func regressionCommand(root string, cfg Config, kind, overrideURL string) *Report {
	r := newReport("regression "+kind, cfg.Release)
	g, err := runScriptWithGoRunner(root, cfg, kind, overrideURL)
	if g != nil {
		r.Data["runId"] = g.runID
		r.Data["sessionId"] = g.sessionID
		r.Data["script"] = filepath.ToSlash(g.sourceRel)
		r.Data["commands"] = g.commandCount
		r.Data["timingSchema"] = "chqdev-command-timings-v1"
		r.Data["runner"] = "go-script-console-v2"
		if g.sessionID != "" && g.runID != "" {
			runPath := filepath.Join(root, "evidence", "sessions", g.sessionID, "runs", g.runID)
			r.Data["runPath"] = runPath
			r.Data["bundlePath"] = filepath.Join(runPath, "bundle.zip")
		}
	}
	if err != nil {
		detail := err.Error()
		if g != nil && g.sessionID != "" && g.runID != "" {
			detail += fmt.Sprintf("; session=%s run=%s runPath=%s", g.sessionID, g.runID, r.Data["runPath"])
		}
		r.fail("regression", detail)
		return r
	}
	r.pass("regression", fmt.Sprintf("%s PASS; run=%s commands=%d runner=go", kind, g.runID, g.commandCount))
	return r
}
