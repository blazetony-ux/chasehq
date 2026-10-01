## v0.66.9.0-RC2.4

Adds the confirmed TC0100SCN Y-scroll sign/neutral-offset finding to `documentation.json`. No knowledge schema change.

## v0.66.9.0-RC1

Knowledge now includes the proven turbo gameplay-to-HUD path and documents the new candidate Game Lab/tile-inspector capabilities. v0.66.8.0 remains the proven baseline until candidate regression completes.

# ChaseHQ research knowledge base

Machine-readable, evidence-linked project knowledge. Entries should distinguish `hypothesis`, `probable`, and `confirmed` rather than silently promoting an inference to fact. Evidence should point at experiment/session/checkpoint/trace identifiers where possible.

The Web Debugger and future Native Oracle are consumers of this database; authoritative runtime state remains in the emulator/debug API.

## User handbook source (v0.66.7.6)
`handbook.json` is the authored user-facing manual/navigation layer. It contains getting-started material, Workbench guides and research recipes. The Workbench combines it with `documentation.json`, the API action schema and the script-language schema to render the complete Docs / Knowledge experience and full handbook exports.
