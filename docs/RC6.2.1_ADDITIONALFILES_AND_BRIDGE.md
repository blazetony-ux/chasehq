# RC6.2.1 follow-up development candidate — AdditionalFiles + native evidence bridge

Status: SOURCE CANDIDATE, Windows qualification outstanding. This is a follow-up to the RC6.2.1 Ghidra exporter corrective source; version intentionally remains `0.67.0.0-RC6.2.1`. RC6.2 parent remains immutable. RC6.3 is deferred.

## Shared dependency discovery

`chqdev doctor` and `chqdev build` now use the same `loadLocalConfig` fallback. If no explicit `build-local.json` specifies sources, a release rooted at `<project-root>/ChaseHQ-Native-v2-NoGit/ChaseHQ-Native-v0.67.0.0-RC6.2.1` searches `<project-root>/AdditionalFiles/roms` and `<project-root>/AdditionalFiles/SDL`. These paths are derived from the current root; nothing is hard-coded to drive C or a username. Existing explicit config and `CHASEHQ_ROM_SOURCE`/`CHASEHQ_SDL_SOURCE` remain higher-priority. ROM size verification and SDL CMakeLists checks remain enforced. No third-party SDL source or user ROM is shipped in this archive. Regression test `TestAdditionalFilesDiscoveryInWindowsStyleLayout` mirrors the reported folder structure; `TestExplicitLocalConfigOverridesAdditionalFiles` protects override behavior.

## Native Go M1 -> M2/M3 evidence bridge (non-promotable)

The Python-free `tools/native_bridge/bin/native-bridge-windows-amd64.exe` accepts a Ghidra ZIP and produces a JSON report:

`./tools/native_bridge/bin/native-bridge-windows-amd64.exe --ghidra-zip "C:\\path\\to\\ghidra-rea-run032.zip" --out "C:\\path\\to\\bridge.json"`

Optional `--runtime-census` is a JSON array containing records `{ "cpu":"A", "pc":"0x00820C", "romSha256":"<64-digit digest>", "hits":7 }`. CPU and ROM hash mismatches and duplicate PCs are rejected. With no census the tool records NO runtime coverage, and never pretends the Ghidra window is a whole-function denominator. Emitted contracts are explicitly unreviewed, incomplete, generation-blocked and promotion-blocked. This is *not* the complete M2/M3 automatic inference implementation, and M4–M7 live native generation/shadow/promotion/orchestration remain unproven. The original Python foundation is preserved, not required to run the Go bridge.

## Acceptance evidence and limitations

- Go contract tests exercise shared folder discovery, explicit config priority, runtime census rejection, fail-closed contract generation and promotion.
- Go bridge has been executed on the actual user-provided `ghidra-rea-run032-20261012-202023-625.zip` with steering at `0x820C`; Ghidra function boundaries remain unreviewed.
- Windows must re-run `chqdev.bat doctor` in a new PowerShell window *without* `CHASEHQ_*` variables, then `chqdev.bat qualify --fresh`. Preflight, CTest and regressions are independent requirements. No qualification claim is made here.
- Further engineering is required to extract and connect CPU-A runtime census automatically, infer complete routine contracts, generate safe generalized C++, execute live independent differentials, and orchestrate fail-closed promotion.
