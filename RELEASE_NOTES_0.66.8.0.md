# ChaseHQ-Native v0.66.8.0 — Forensic Timeline v1

**Status: PROVEN RELEASE — 2026-09-29**

This release is the promoted RC7.4 runtime with documentation/release metadata updated to final. No emulator/runtime semantics changed during promotion.

## Major capabilities

- Forensic Timeline v1: deterministic per-frame CHQSTATE capture plus complete bus-write provenance.
- Read-only historical inspection through the same memory/register/gameplay/sprite/palette surfaces used for paused LIVE state.
- Recorded-state SDL playback with `timeline.play`; `timeline.fork-live` restores a selected historical state into LIVE execution for causal experiments.
- Immutable source timelines with derived event windows/analyses written into the current run artifact tree.
- Streaming large-write analysis with ranked byte/address candidates, writer groups and automated writer disassembly.
- Portable automated timeline analysis including before/active/mid/expiry/after historical milestones and HUD/graphics correlation.
- Legacy quoted-CSV compatibility for the first turbo capture; no gameplay replay is required to reuse it.
- Progress-aware long timeline recording waits.
- Script-library autoload/bootstrap fixes and improved Workbench startup diagnostics.
- Bounded, summary-first run-history discovery and bounded browser output to prevent Full Regression browser OOM.
- Script history version/status filtering, including RC-aware labels and PASS/FAIL/CANCELLED filtering.
- Thumbnails default OFF; large artifact sets are summarized rather than eagerly rendered.

## Proven Windows/SDL gates

- RC7 portable automated timeline-analysis focused regression: PASS.
- Real `Analyze Existing Turbo Forensic Timeline`: PASS, producing portable candidate/writer/disassembly/milestone/graphics analysis without replaying gameplay.
- `Regression - v0.66.8.0 RC7.4 Bounded History Discovery`: PASS, 1.071 s.
- Final `Full Regression Suite`: PASS, 78.422 s, session `20260929_195302`, run `002`.

## Known limitation

Authentic Chase H.Q. audio remains unavailable because the native runtime still exposes `sound_stub`. Timeline manifests state this explicitly; no release claim is made for real sound CPU/chip/PCM capture or playback.

## Next substantive direction

The release gate is closed. Development can return to game fidelity/reverse-engineering work, using the timeline system to reduce repeated manual gameplay reproduction. Immediate research priorities include the turbo HUD/source-compositor path, broader authoritative gameplay-state discovery, target/end-level causal analysis, and graphics/rendering fidelity.


Release evidence summary: `docs/RELEASE_PROOF_0.66.8.0.md`.
