# Documentation audit — v0.66.9.4-RC3.1.2

RC3.1.2 documentation is aligned to the semantic player-car replacement persistence and complete dynamic SPEED suppression corrections. The focused regression is `research/scripts/regression/regression-v06694-rc312.chqscript`; `chqdev.json` points to it and Full Regression includes it.

The candidate retains RC3.1 intrinsic cross-format content fitting, chqdev v0.5.7 readiness retry and qualification START/COMPLETE/FAILED reporting. Exact-archive Windows `chqdev qualify --fresh` remains mandatory before promotion.

Source lineage remains immutable promoted v0.66.9.3 SHA-256 `6da88889dc8214c6058644e44e4d29f6a2117c9f0e92d99c8d906c280fc3462e`; v0.66.9.4-RC2 remains the last fully qualified candidate at SHA-256 `50a8fb6d2c4be5fdafdea5b3804d6f2e1f1e5e62685160d3dc6f6d65237a6f41`.

## RC3.1.2 release-process contract

`chqdev` is the de facto and authoritative build/release process. Promotion requires fresh exact-archive Windows `chqdev qualify --fresh`; zero tolerated CTest failures (any CTest failure is fatal); deterministic archive packaging and exact verification; focused regression plus Full Regression PASS; and promotable PASS. PowerShell/BAT/shell launchers remain shell-compatibility surfaces, not a second release authority. Release-owned text must remain valid under LF and CRLF line endings.
