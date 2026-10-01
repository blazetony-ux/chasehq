# Performance and Profiling Plan

Correctness remains the gate for optimization. Measure first; optimize only evidence-backed hotspots, then verify equivalent emulated state/output.

Planned instrumentation:
- Per-frame and per-subsystem host timings (CPU A/B, road, tilemaps, sprites, compositor, audio, SDL presentation, diagnostics, capture I/O).
- Mean/median/P95/P99/max frame times and variance, not averages alone.
- Work counters (sprites processed/visible, pixels tested/drawn, priority tests/rejections, CPU cycles/instructions, selected memory/device events).
- Deterministic benchmark scenarios/checkpoints with throttling disabled where appropriate.
- Machine-readable benchmark baselines and regression comparison across builds.
- Performance-spike detection capable of producing evidence bundles around anomalous frames.
- Separate accounting for core emulation and optional diagnostic overhead.
- Future profiler timeline export (for example Chrome/Perfetto-compatible events) where useful.

Project rule: an optimization is accepted only when deterministic correctness evidence remains equivalent or any intended difference is explicitly explained.
