# Documentation audit - v0.66.9.3-RC5

Current candidate identity: **v0.66.9.3-RC5**. Immutable promoted baseline remains v0.66.9.2 until RC5 completes its own Windows `chqdev.bat qualify --fresh` gate.

RC5 is a qualification-repair candidate over the RC4 capability set. The first Windows RC4 `qualify --fresh` built successfully and passed 7/8 CTest; only `cli_evidence_tests` failed because it hard-coded stale build identity `0.66.9.2` while the generated evidence correctly identified the running RC4 build. Packaging/runtime/focused/full gates were not reached and therefore were pending, not failed.

RC5 removes that duplicated release identity from the Windows CLI evidence test: CMake now injects the authoritative `CHQ_RELEASE_VERSION`, and portable preflight rejects a hard-coded evidence-build assertion. Preflight also validates CMake project-version alignment, focused-regression header identity and configured machine-readable `build` fields so stale identity drift is caught before Windows packaging.

The independent sprite-layer shadow, semantic gameplay/object promotion, read provenance, structured collision events and sound-command event instrumentation from RC4 are otherwise carried forward unchanged.
