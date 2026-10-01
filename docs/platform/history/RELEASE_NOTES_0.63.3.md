# ChaseHQ-Native v0.63.3 - IOC & Evidence Patch

Built from v0.63.2 after live reverse-engineering exposed a real intervention-layer defect.

## Fixed
- Held Research API/debugger IOC XOR masks no longer get overwritten by the legacy per-frame scenario/CLI pulse updater.
- Scenario/CLI and debugger/research XOR masks are separate and XOR-composed at the TC0040IOC read path.
- Script v2 validation no longer treats a bare single-token command as `System.Char`.
- Evidence Viewer selection is explicit and capture cards are clickable.

## Added
- `input ports` now reports debugger XOR, scenario XOR and effective XOR.
- Evidence ZIP readiness indicators and direct selected ZIP download.
- Prefix experiment bundles in the UI/API.
- Script verb `zip evidence-match PREFIX [BUNDLE_NAME]`.
- Chase H.Q. IOC polling discovery record and reusable controlled-intervention recipe update.

## Research correction
The v0.63.2 held port-3 bit sweep is marked invalid for semantic classification because the override was removed before CPU-A sampled it. Its captures remain useful as tooling-history evidence.
