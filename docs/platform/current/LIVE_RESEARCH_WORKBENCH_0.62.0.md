# Live Research Workbench 0.62.0

## Principle
Research controls follow **Observe → Graph/compare → Override → Restore**. v0.62.0 implements the first live runtime-parameter and palette controls and retains rolling browser history. Every override must be reversible and visible in status/evidence.

## Native commands
- `handling get`
- `handling set 1.15 1.00`
- `handling reset`
- `palette status`
- `palette bank 0`
- `palette get 42`
- `palette freeze 42 7FFF`
- `palette restore 42`
- `palette clear`
- `input xor set 3 20`
- `input xor clear`
- `input pulse 3 20 3`

## Browser
The localhost Web Debugger adds Driving Tuning, Palette Lab, raw Gamepad telemetry, held IOC control, API discovery and interactive dependency-free API documentation.

Palette Lab manipulates the emulated palette state only as a research override. It does not assert that an alternative palette is correct. Restoring an entry returns it to game control; `palette clear` removes every research palette freeze.

## API
OpenAPI: `/api/v1/openapi.json`
Interactive docs: `/api/docs/`
Health: `/api/health`

The PowerShell bridge remains localhost-only. The native debugger TCP API remains the lower-level control plane.
