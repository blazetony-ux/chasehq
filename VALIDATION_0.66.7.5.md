# v0.66.7.5 Validation — User Handbook / Live Reference

Status: **IMPLEMENTED — awaiting Windows/Workbench prove-off**.

## Scope
- Docs / Knowledge redesigned as a user-facing handbook rather than a raw knowledge catalogue.
- Authored handbook navigation/guides in `research/knowledge/handbook.json`.
- Complete API Reference generated from the authoritative action schema.
- Complete Scripting Reference generated from the authoritative script-language schema.
- Cross-domain search across handbook, API actions, scripting constructs and project knowledge.
- Copyable examples and richer evidence-backed finding pages.
- Full HTML/PDF handbook export combines handbook + API + scripting + knowledge/evidence.
- New read-only API/script actions: `docs.handbook`, `docs.page`.

## Static checks expected before packaging
- JSON parse: handbook, knowledge, API schema, script schema.
- Workbench JavaScript syntax check.
- Native non-SDL build and CTest where available.
- `Validate-Release.ps1 -SkipBuild` on Windows must verify handbook/build identity and current regression wiring.

## Windows proof
Run `Regression - v0.66.7.5 User Handbook / Live Reference`. Confirm the Handbook, API Reference, Scripting Reference, Project Knowledge and Glossary views render correctly and that the exported HTML is included in the run bundle.
