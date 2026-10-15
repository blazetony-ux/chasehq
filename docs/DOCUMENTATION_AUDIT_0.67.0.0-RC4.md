> **AUTHORITATIVE CURRENT — v0.67.0.0-RC6.3.11 CPU-B IRQ-OWNERSHIP CORRECTIVE SOURCE CANDIDATE (2026-10-14).** RC6.3.10 actual Windows `native reconstruct --cpu-b-live` failed only at the CPU-B original-ROM baseline because 12 interrupt-vector/stack bus effects (four genuine IRQ entry groups) were wrongly tagged as BEQ.B data effects by stale `trace_pc`; its CPU-A health/copy, shared dispatcher and real CPU-B Ghidra/compiled ABI gates all PASS. RC6.3.11 uses Musashi interrupt ACK to assign each IRQ vector/stack transaction to a dedicated ordered IRQ stream, preserves strict zero-branch-data and original/native/fallback CPU/cycle/bus/machine parity, and adds real-ACK synthetic and missing/mutated IRQ evidence tests. **RC6.3.11 authentic Windows native CPU-B equivalence is PENDING**, as are complete-function/autonomous promotion and inherited Go Full Regression budget 1176>850. See `docs/RC6.3.11_IRQ_OWNERSHIP_CORRECTIVE.md`. Earlier status blocks remain historical.

# Documentation audit: v0.67.0.0-RC4

Current candidate: v0.67.0.0-RC4.

chqdev is the de facto and authoritative build/release process. PowerShell/BAT is compatibility only. Fresh Windows qualification: `chqdev qualify --fresh`. Zero tolerated CTest failures; any CTest failure is fatal. Deterministic archive verification and Full Regression are required.

RC3.2 includes persistent content-source selection and ROM-free launcher corrections.

RC4 development scope: sparse CHQPACK1 graphics/PROM overlay loading; strict region-size validation, ordered overrides, and native runtime CLI integration. This candidate requires Windows qualification and explicit user approval before baseline promotion.
