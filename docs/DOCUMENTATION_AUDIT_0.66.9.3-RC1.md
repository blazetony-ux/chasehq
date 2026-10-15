# Documentation audit - v0.66.9.3-RC1

Baseline: exact final Windows-qualified v0.66.9.2 SHA-256 `f3c730e5bc41886189c11e23d4998967f205b085fa172a9c30fc812330227b26`.

RC1 documentation was updated for the tooling-hardening state rather than a promotion state. Current docs identify v0.66.9.3-RC1, retain `chqdev` as the sole authoritative build/release controller, record that Windows qualification is intentionally deferred, document the new sprite-shadow/runtime-query/tuple/corpus/evidence-index contracts, and map the Full Regression consolidation in `REGRESSION_COVERAGE_MAP.md`. Historical version references remain provenance rather than current instructions.

The release root remains consolidated: `CHANGELOG.md`, `VALIDATION.md`, generic `PACKAGE_MANIFEST.*`, and no version-specific qualification wrappers.
