## v0.66.9.0-RC1 knowledge additions

The knowledge set now records the proven turbo gameplay/HUD chain: activation/consumption around CPU-A `0x8460`, timer expiry around `0x84BE`, active HUD characters `0x0C..0x0F` written around `0x1C82/0x1C8A`, normal characters `0x7C..0x7F` drawn around `0x1CE6/0x1CEE`, and expiry clear around `0x1CD4/0x1CD8`. The new tile inspector is documented as a platform capability awaiting Windows/SDL prove-off.

# Workbench Docs / Knowledge System (v0.66.7)

v0.66.7 introduces a machine-readable documentation/knowledge layer consumed by the Workbench, Research API and Research Script API actions.

## Authoritative source
`research/knowledge/documentation.json` is the first authoritative collection. UI text should not become a second hidden knowledge database. Entries support stable IDs, kind/category, status, confidence, version metadata, tags, related IDs, docs/scripts/API links and evidence references.

## Status and confidence
Use `confirmed` only for evidence-backed conclusions. Use `provisional`, `open`, `planned`, `implemented-awaiting-proof`, `historical` or `superseded` as appropriate. Confidence/evidence method is separate from lifecycle status.

## Evidence
Curated documentation evidence lives under `docs/evidence/`. Image evidence can be embedded in Workbench and HTML/PDF export. Evidence entries should retain build/checkpoint/frame/script/run/hash provenance where available. Layered snapshots, HUD variants, SVG, CSV, JSON and trace artifacts may all be referenced.

## API / scripting
Read-only actions: `docs.index`, `docs.search`, `docs.get`, `docs.glossary`, `docs.handover`, `docs.export`, `knowledge.list`, `knowledge.get`, `knowledge.search`, `knowledge.related`. They are available through `.chqscript` using the existing `api ...` syntax.

## Export
`docs.export format=html|pdf evidence=none|key|full`. HTML is deterministic from the knowledge source. PDF uses installed Microsoft Edge headless printing; absence of Edge is a reported capability/environment limitation rather than silently producing a fake PDF.

## Handover
`docs/CHATGPT_HANDOVER_PROMPT.md` is the authoritative continuation prompt shipped beside the ZIP. It must be updated on every revision and its build/proof metadata must match the package.

## Validation
Release validation checks parseability, unique IDs, required metadata, relationship/evidence targets, schema build metadata and handover build metadata. Confirmed entries should have an evidence trail where practical.


## v0.66.7.5 user-facing handbook layer
The Workbench no longer presents the machine-readable knowledge catalogue as the main documentation UX. `research/knowledge/handbook.json` is the authored user-manual source and drives handbook navigation, getting-started material, Workbench guides and task-oriented research recipes. The API and scripting reference pages are generated from the live schema contracts so they cannot silently drift from implementation. `research/knowledge/documentation.json` remains the evidence-backed project-knowledge and glossary source.

The Docs / Knowledge tab has five views: **Handbook**, **API Reference**, **Scripting Reference**, **Project Knowledge**, and **Glossary**. Search spans all four data domains. Code examples are copyable. Full HTML/PDF export combines the same handbook, API schema, script schema and project knowledge rather than exporting only the short knowledge records.

New read-only script/API actions: `docs.handbook` returns the full authored handbook model and `docs.page id=PAGE` returns one authored page plus its parent section.

## v0.66.8.0 release state

The Docs/Knowledge system ships in the proven v0.66.8.0 baseline. The authoritative handover/project-state records now identify v0.66.8.0 as current. Forensic Timeline knowledge is promoted from implemented-awaiting-proof to confirmed after focused Windows/SDL analysis and final Full Regression passed.

## v0.66.9.0-RC2 knowledge additions
The knowledge base now distinguishes HUD display speed from internal physics speed and records the confirmed turbo HUD animation pipeline plus the active player-car sprite-order investigation.
