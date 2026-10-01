## v0.66.9.0-RC2.4 schema state

No API, Research Script grammar, evidence-schema or Workbench endpoint change. RC2.4 is an emulator/render-coordinate correction plus documentation/regression update.

## v0.66.9.0-RC1 schema state

The candidate extends the API action schema without changing the script-language grammar. Game Lab and tile-inspector actions are present in both Workbench live schema and `research/schema/api-actions.json`; regression/schema parity must remain mandatory.

# API / Script Schema Maintenance Policy
> **Current validation baseline:** v0.66.9.0-RC2.8 (Windows/SDL proven). RC2.9 is the current source candidate; new RC2.9 interfaces remain candidate-only until focused Windows/SDL and Full Regression proof.


No API or language change is complete unless all four are changed together:

1. implementation;
2. authoritative schema;
3. reference/changelog documentation;
4. regression coverage.

`api actions` and `$scriptActionSchemas` must remain one-to-one. `schema.all` and `/api/v1/schema/actions` are produced from that table. Script grammar is exposed by `script.schema` and `/api/v1/schema/script`.

Removed/deprecated actions must be recorded in the changelog and regression expectations changed in the same build.

## RC2 contract note
Sprite-order diagnostics are first-class scriptable API actions. UI controls in Game Lab are aliases over the same API; no UI-only mutation is permitted.
