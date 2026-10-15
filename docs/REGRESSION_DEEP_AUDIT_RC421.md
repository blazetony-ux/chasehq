# RC4.2.1 Full Regression deeper source audit

Status: source inspection only; not Windows-qualified. No test coverage removed in this pass.

## Structural baseline

- Full Regression: 41 include statements and 1,170 compiled structural commands (`go test -run TestFullRegressionConsolidatedCommandBudget` confirms 1,170).
- Textual non-comment/nonblank statements (including control-flow keywords): 1,241; this is NOT the structural metric.
- The largest contributors by textual statements: main script 203; regression-v06695-rc11 110; regression-v06695-rc2 86; regression-v0670-rc11 58; regression-v06694 54; regression-v0670-rc421 52; regression-v06693 49; regression-v06695-rc3 48.
- After removing `echo` statements, no two *included complete scenario bodies* are identical. The earlier RC2.1/RC3.2 BG Shadow duplicates were already removed in RC4.2.1.

## Common operations, NOT proven redundant

- checkpoint.load gameplay-2064: 43 occurrences; driving-2352: 30. Stateful isolation and frame selection are significant.
- `api status`: 25 occurrences; many are diagnostic but require evidence/consumer review before removal.
- `api sprite.visual.clear`: 21; `api patch.clear`: 20; `api sprite.override.clear`: 20; `api input.ioc.clear`: 19. State cleanup should not be removed on command-count grounds alone.
- `api release.assert-version`: 14; historical scripts are also standalone regressions, so their local assertions should be preserved.

## Corrected sequencing defect

Moved `echo === ChaseHQ Full Regression: COMPLETE ===` to *after* the RC4.2.1 include. Previously it occurred before the final scenario, giving misleading completion evidence.

## Gate and conclusion

The unchanged structural budget is <850. Current suite still fails at 1,170. This audit did not find additional whole-script duplicate coverage. To reduce further safely, instrument runtime command duration and capture per-scenario assertion/evidence dependencies; then compare candidates with original and trimmed runs on the same checkpoints, including structured output/evidence parity. Do not remove restores, clear operations or unique API assertions based only on matching command text. Do not relax the budget without a separate policy decision.
