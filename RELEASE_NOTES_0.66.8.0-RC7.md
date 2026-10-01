# v0.66.8.0-RC7 — Portable Automated Timeline Analysis

- Derived timeline analysis now writes into the current run artifact tree; source `.chqtimeline` recordings remain immutable.
- `source.json` records source path and SHA-256 provenance for manifest/write stream.
- Candidate ranking now rewards event-edge bounded state and tags confirmed turbo reference addresses.
- New `timeline.analyze.auto` performs writer-PC disassembly, historical milestone state capture, layered snapshots and HUD_TURBO graphics correlation.
- New `timeline.play` performs paced SDL playback of recorded frame checkpoints. This is state playback, not CPU re-execution.
- New `Analyze Existing Turbo Forensic Timeline` and `Replay Turbo Timeline in SDL` scripts.
- New permanent RC7 focused regression wired into Full Regression.
- No gameplay replay is required for the existing turbo investigation.
