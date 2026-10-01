# ChaseHQ-Native v0.66.8.0-RC7.3

Focused Workbench browser-memory and run-history usability hotfix on the RC7.2 runtime baseline.

## Changes

- Script Console **Show thumbnails** now defaults OFF.
- Artifact output is summarized by file type instead of rendering hundreds of context-free cards by default.
- Opt-in thumbnail rendering is bounded and lazy-loaded (24 PNG previews plus a small non-image set).
- Live Script Console DOM is bounded to the newest 120 output blocks; older blocks are explicitly marked as elided while complete authoritative output remains on disk/in the run bundle.
- Recent Script Runs now derives the package/RC label from the run path where available (for example `0.66.8.0-RC7.2`) instead of collapsing all RCs into `0.66.8.0`.
- Added run-history Status filter: All / PASS / FAIL / CANCELLED.
- Current-session filtering now selects the derived RC-aware version label and current session.
- Added static validation for the RC7.3 browser-memory and history-filter contract.

No emulator/runtime semantics changed from RC7.2.
