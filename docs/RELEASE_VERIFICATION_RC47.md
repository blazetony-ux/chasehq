# RC4.7 — Reverse Engineering Workbench

**Candidate status (not promoted):** v0.67.0.0-RC4.7. Source parent is immutable promoted RC4.6 SHA-256 `9e76a107e19f3e28a54a8a8b3bdaed630511e02f1da3e3e7ae274c07d863580f`. Fresh Windows `chqdev.bat qualify --fresh` on the exact final ZIP is mandatory. No ROMs, JDK, Jython or Ghidra binaries are bundled.

## Scope delivered (portable proof pending)

- Full-width top-level **Static Analysis / REA** tab (Native Lab retains native rendering controls); responsive grid, report/function filter, automatic function list loading and Ghidra job controls.
- Corrected `ghidra.results` file lookup (`evidence/ghidra-rea-run032-.../analysis/`), preserving explicit `count=0` when genuinely no census is staged.
- New Research API/script actions `ghidra.wait` and `ghidra.evidence.attach`. Browser Workbench Script Console long wait polls status asynchronously up to 15 minutes, preserving browser responsiveness and cancellation; server-side direct API waits are capped at 30 seconds. Mobile/CLI direct waiting uses the short bounded action, and can repeat explicitly.
- Evidence attachment copies Ghidra result/job console/error files, static JSON, manifests and available headless logs to the authoritative Script Run evidence on PASS or FAIL. A missing/not-started job yields a benign empty attachment.
- `chqdev runtime start/restart` detects **verified JDK 25** under `JAVA_HOME`, standard Windows install directories or `PATH`, and passes `JAVA_HOME` and `PATH` to launched Workbench child processes only. Missing Java is nonfatal for normal gameplay but Ghidra is marked not ready.
- `chqdev ghidra doctor` checks prerequisites and `chqdev ghidra setup --install-jython` is a deliberate opt-in to install a **bundled, version-matching** Jython extension into the user's Ghidra extensions directory; no network download, administrator privileges or arbitrary script input.
- Ghidra's headless export now includes nearby instruction flow types, outgoing references and local control-flow edges; all function ownership remains **unreviewed hypothesis**.
- Focused regression verifies identity and all new action schemas without triggering an external tool; Run 036 is the actual end-to-end Windows investigation.

## Deferred / not implemented

- **Capstone M68K**: upstream source archive is not available in this offline build environment, so source-bundled C/C++ Capstone and an independent Capstone-vs-Musashi oracle cannot be qualified. Do not describe this candidate as shipping Capstone. Keep it P1 for the next meaningful tooling update when a version-pinned source vendor and license/audit checks are available.
- Live Workbench ROM census start/stop is not implemented; use previously verified checkpoint-based CPU-only census reports. No direct REA agent provider for 68000 raw ROM.
- Script runner mobile and direct HTTP actions have only bounded 30-second synchronous wait; desktop has long nonblocking polling. Full parity will require a shared asynchronous job-await protocol.
- The inherited Go Full Regression structural-budget target (<850 expanded commands) is still FAIL against more than 1,100 commands. The authenticated runtime Full Regression and Windows `qualify --fresh` gates remain mandatory; do not relax unique test coverage to make this pass.

## Windows qualification before promotion

1. `chqdev.bat qualify --fresh` on the exact frozen archive, 11/11 stages PASS and archive SHA verified.
2. Test `chqdev ghidra doctor` with empty and present `JAVA_HOME`; verify that emulator remains usable with no Java and Ghidra preflight closes safely.
3. Test Run 036 against Ghidra 12.1.4, CPU-A SHA `5CF1C9E786A39795EBD8E99C8E8BE0D2398CC4F74221067CF2844945308D6C5A`, a genuine census from Run 030/035; verify one complete script-run evidence ZIP, independent static results, no emulation mutation, clean failure and timeout.
4. Verify Workbench tab sizing, function filtering, direct API/mobile parity limitations, and correct static-confidence labelling.

## Portable and Windows evidence

Portable testing and packaging reports are provided alongside the release candidate, if they pass. The **final Windows qualification is pending** until the user runs and uploads `out/chqdev/qualification.json`. This source candidate is never promoted by packaging alone.

## Portable qualification outcomes (2026-10-10)

- CPU-only CMake Debug configure/build: **PASS**.
- CPU-only CTest: **11/11 PASS**.
- `tools/chqdev` targeted RC4.6/RC4.7 Ghidra and Java/Jython contract tests: **PASS**, including safe user-extension archive extraction tests.
- Complete `tools/chqdev` Go test suite: **FAIL**, one inherited `TestFullRegressionConsolidatedCommandBudget`: 1176 expanded commands with strict target below 850. This issue is **not waived, bypassed or fixed**. Portable validation is consequently **not fully green** despite runtime regressions previously qualifying RC4.6 on Windows.
- Full Workbench JavaScript syntax extraction and `node --check`: **PASS**.
- Ghidra Jython export script Python syntax: **PASS**; actual headless 12.1.4 run on Windows for RC4.7: **NOT RUN**.
- `chqdev preflight`, deterministic package, exact ZIP verify and fresh-extract preflight: see portable evidence attached alongside candidate; the release remains **unqualified** until Windows `chqdev.bat qualify --fresh` completes.
- Capstone M68K source-build: **NOT IMPLEMENTED**; lack of vetted offline upstream source. Do not claim Capstone capability.

Because the command-budget test remains failing, the candidate is deliberately described as **portable-tested with an unresolved test gate**, not as a fully successful release. Keep RC4.6 as authoritative baseline.
