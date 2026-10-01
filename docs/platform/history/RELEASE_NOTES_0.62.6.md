# v0.62.6 — Web Response Resilience Fix

## Fix

The PowerShell `HttpListener` bridge no longer exits when a browser/client disconnects after a response has already begun.

Observed on Windows in v0.62.5:

```text
Exception setting "ContentLength64": "This operation cannot be performed after the response has been submitted."
```

The original request had already started/submitted its response. The per-request catch block then attempted to send a second JSON error response on the same `HttpListenerContext`; that second send threw and escaped the request loop, causing the whole Web Workbench host to terminate.

### Changes

- `Send` treats `IOException`, `HttpListenerException`, and `ObjectDisposedException` as client/transport disconnects and closes the response best-effort.
- The request-handler catch path now logs failures and wraps the fallback JSON error response in its own `try/catch`.
- A single malformed/disconnected request can no longer stop the HTTP listener loop.
- Bridge/OpenAPI documentation identifies the web bridge as v0.62.6.

No Chase H.Q. emulation behaviour changed.
