# v0.64.6 — Concurrent Web Host Fix

## Why this patch exists

v0.64.2 proved that the browser-side Script Console could execute long scripts command-by-command, but the PowerShell `HttpListener` backend still accepted and handled only one request at a time. A slow native/debug request could therefore delay unrelated local operations such as Script Library load until the browser timeout fired.

## Concurrent frontend

The public Web Workbench port is now handled by an in-process .NET concurrent frontend. It accepts requests independently and forwards native/debug operations to the existing serialized PowerShell backend on an adjacent localhost-only port. This keeps the public web endpoint responsive even while the native bridge is busy.

Local/fast operations are served directly by the concurrent frontend and cannot be starved behind native debugger work:

- Script Library list/load/save
- Next Steps JSON
- image-region definitions
- Logs list/tail
- frontend health

Native/debug requests remain serialized by the existing backend for safety. The frontend applies an 8-second backend timeout and returns a controlled `DEGRADED` response instead of wedging the browser.

## Browser behavior

- routine native polling is sequential rather than four simultaneous requests
- transient backend failures show `DEGRADED`, preserving the distinction from a dead frontend
- timeout messages identify the actual request path
- Script Library load is served locally by the concurrent frontend

## Compatibility

The public URL and API paths remain unchanged. The internal backend uses `HttpPort + 1` and is bound only to `127.0.0.1`.
