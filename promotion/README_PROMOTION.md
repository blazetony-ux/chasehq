# ChaseHQ-Native RC4.6 — immutable baseline promotion

User-authorised Windows-qualified baseline: **`v0.67.0.0-RC4.6`**. Previous RC4.5 remains preserved; the exact qualified RC4.6 ZIP is unchanged.

- **Frozen archive:** `ChaseHQ-Native-v0.67.0.0-RC4.6.zip`
- **SHA-256:** `9e76a107e19f3e28a54a8a8b3bdaed630511e02f1da3e3e7ae274c07d863580f`
- **Windows `qualification.json` (operator-pasted and transcribed):** all 11 stages PASS, including `promotable`.
- **Regression evidence:** focused Run 001 (13 commands PASS), Full Regression Run 002 (1,153 commands PASS), integrity checks PASS.
- **Ghidra/REA research milestone:** Ghidra headless PASS and Run 035 3 real-census crosswalk reports PASS (19 commands). Ghidra function ranges are unreviewed hypotheses.

## Files and application

This supplement is a **documentation and promotion-status overlay**. It is not a new build, Windows qualification or new ZIP for RC4.6. Apply to a **working extraction** of RC4.6, preserving the original release archive and a clean extraction for SHA/qualification provenance. Files mirror repository-relative locations (`AGENTS.md`, `docs/`, `promotion/`, etc.). Overwriting documentation in a working copy does not change the original tested archive.

Review `docs/RELEASE_VERIFICATION_RC46.md`, `docs/PROJECT_STATE.md`, `docs/ROADMAP.md`, `docs/REA_GHIDRA_NEXT_SCOPE.md` and `docs/INVESTIGATION_LIMITATIONS.md`. RC4.6's current UI is the narrow Native Lab Static Analysis / REA panel; **a dedicated top-level tab is specified for the next substantive build**, not implemented here.

Historical RC4.x candidate notes remain inside several long-form documents for provenance. The new promoted-baseline banner at their beginning has precedence; earlier “NOT QUALIFIED” language describes the build-time stage.
