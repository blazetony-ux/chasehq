# Validation — v0.66.7.4

## Purpose
Full build-identity sweep after Windows smoke testing showed native v0.66.7.3 while the Workbench still displayed v0.66.7.2.

## Corrected
- Workbench header/version variable and startup log.
- `/api/health` bridge version.
- OpenAPI `info.version` and API landing-page branding.
- checkpoint-save default provenance text.
- current schema/knowledge/native/package identities.
- current regression wiring.
- handbook current proving target.

## Required Windows proof
1. SDL title reports v0.66.7.4.
2. Workbench header reports v0.66.7.4.
3. `/api/health` bridge reports 0.66.7.4.
4. `/api/v1/openapi.json` info.version reports 0.66.7.4.
5. Run `Regression - v0.66.7.4 Workbench Knowledge / SDL / Sprite Controls`.
6. Snapshot `manifest.json` reports build 0.66.7.4.

No API semantics or scripting-language syntax changed.
