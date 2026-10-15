> **AUTHORITATIVE PROMOTED BASELINE — v0.67.0.0-RC4.6 (2026-10-10).** Windows `chqdev.bat qualify --fresh` reports **11/11 stages PASS** for exact frozen ZIP SHA-256 `9e76a107e19f3e28a54a8a8b3bdaed630511e02f1da3e3e7ae274c07d863580f`. User explicitly authorised promotion following focused, full, and successful headless Ghidra/REA crosswalk evidence. This is a **post-qualification documentation overlay**; it does not change the immutable tested archive. See `docs/RELEASE_VERIFICATION_RC46.md`, `promotion/PROMOTION_RECORD.json`, and `docs/REA_GHIDRA_NEXT_SCOPE.md`. Earlier RC4.6 “candidate/unqualified” statements below are **historical build-time notes, not current release status**.

> RC4.6 source candidate documentation identity: `0.67.0.0-RC4.6`; prior sections retain historical findings. Promoted baseline is RC4.5.

# v0.67.0.0-RC4.6 source and qualification audit (original build-time audit; see promotion supplement for final status)

- Source parent: Windows-qualified and user-promoted RC4.5 SHA-256 `443ffe623a38de4813691c436e05d6f92d9e113a8abae0219824fcc77e9aae11`; the RC4.5 ZIP is immutable.
- New scope: `docs/GHIDRA_WORKBENCH_RC46.md`. Fixed packaged headless Ghidra runner, job status, static findings, REA crosswalk, and Workbench controls. Standalone Ghidra installation remains operator-owned. No actual REA agent inference or game CPU replacement.
- New script actions and HTTP endpoints: `ghidra.status`, `ghidra.start`, `ghidra.results`, `ghidra.import`; schema and allowlist must match; new focused regression is `regression-v0670-rc46.chqscript`.
- Known limitation: old Go Full Regression expanded-command budget still exceeds threshold; do not weaken its assertions. Windows fresh full qualification is pending and required before promotion. Ghidra 12.1.4 Windows execution has NOT been run by the source builder.
- Validation should cover: correct ROM SHA, same-live-session invariants, quick asynchronous action return, one active job only, invalid input denied, failed job reports, valid static export, three genuine census crosswalks, all automatic evidence, and independent run-history verification.
