# Documentation audit - v0.66.9.4-RC3

RC3 documentation is aligned to the format-aware semantic asset replacement and dynamic HUD suppression changes. Current API/schema, focused regression, Native Reconstruction notes, project state, roadmap, handover and release governance identify v0.66.9.4-RC3 as a candidate requiring exact-archive Windows `chqdev qualify --fresh`. Immutable promoted v0.66.9.3 lineage is preserved; fully qualified RC2 SHA-256 is `50a8fb6d2c4be5fdafdea5b3804d6f2e1f1e5e62685160d3dc6f6d65237a6f41`.


## RC3 release-process contract

Current candidate: v0.66.9.4-RC3. `chqdev` is the de facto and authoritative build/release process. Promotion requires `chqdev qualify --fresh`; zero tolerated CTest failures (any CTest failure is fatal); deterministic exact archive packaging/verification; focused regression plus Full Regression PASS; and promotable PASS. PowerShell/BAT/shell launchers remain compatibility surfaces, not a second release authority. Release-owned text must remain valid under LF and CRLF line endings.
