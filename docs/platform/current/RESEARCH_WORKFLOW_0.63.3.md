# v0.63.3 IOC & Evidence Patch

## Why this patch exists

During live v0.63.2 research, `api input.ioc.xor port=3 mask=01` changed `input ports` from `3F` to `3E` while paused, proving the API/debug layer stored the override. A simultaneous CPU-A TC0040IOC trace still read `3F` after emulation resumed. Source inspection found the cause: the legacy scenario/CLI frame runner rewrote the same IOC XOR array from `active_ioc_xor` every frame, normally with zero, erasing the persistent debugger override before the CPU consumed it.

## Corrected layering

IOC input now has two independent XOR sources:

- debugger/research XOR: persistent live intervention controlled by API/debugger
- scenario/CLI XOR: deterministic per-frame pulses and automated driving

The value presented to TC0040IOC is `base ^ debugger_xor ^ scenario_xor`. Scenario updates therefore cannot clobber debugger state. `input ports` exposes each layer plus the effective XOR.

Neither intervention layer is restored as authentic machine state. The historical debugger-XOR checkpoint field remains in place for binary compatibility, but v0.63.3 writes it as zero and discards its legacy contents on load; both debugger and scenario layers are cleared after checkpoint restore.

## Script validator hardening

PowerShell can unwrap a single-element function result to a scalar string. The v0.63.2 validator then indexed it and received a `System.Char`, causing bare commands such as `resume` to fail on `.ToLowerInvariant()`. v0.63.3 forces the token result to an array and normalizes the first token to string.

## Evidence UX

Evidence cards are now selectable, the selected capture is explicit, ZIP readiness is shown, ready ZIPs expose direct download, and newly produced evidence can select newest automatically. Prefix-based experiment packaging is available through the UI/API and Script Console:

```text
zip evidence-match ioc-p3- ioc-p3-held-sweep
```

This packages all matching capture directories into one uploadable ZIP.
