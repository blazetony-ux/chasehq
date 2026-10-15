# Embedded RC5 synthetic preview status

This preview is now bundled in the **RC5 source candidate** generated from the uploaded RC4.7 source. It is not the playable/native Chase H.Q. client.

- Windows x64 PE32+ standalone GUI cross-compilation: **PASS** on Linux; GUI startup on Windows **NOT RUN**.
- Four synthetic Go reference scenarios and mismatch/invalid-input tests: **PASS**.
- Independent C++ sprite/road candidates compiled into original `chq_runtime` and tested under portable CTest: **PASS**.
- Instruction-boundary read-only shadow hooks compiled and synthetic entry/exit comparison tested: **PASS**, real game checkpoint verification **NOT RUN**.
- Actual Workbench Next Steps JSON: **UPDATED** (`games/chasehq/next-steps.json`), Windows GUI rendering **NOT RUN**.
- Exact-archive Windows authoritative `chqdev.bat qualify --fresh` (build/CTest/focused/full/Workbench/mobile/deterministic package/promotable): **NOT RUN**.
- Official RC5 complete: **NO**. Inherited Go full-regression structural command-budget test fails, and remaining live shadow/UI/tooling gates are not completed.

Authoritative RC5 status: `../docs/RC5_NATIVE_PREVIEW_AND_SHADOW.md` and `../docs/RC5_PORTABLE_VALIDATION.md`.
