# Reusable research methods

These documents describe **how an outcome was obtained**, independent of Chase H.Q. They are deliberately separate from both game findings and Workbench implementation documentation.

A useful method records: prerequisites, controlled starting state, control/intervention runs, evidence captured, candidate filtering, causal test, rejection criteria, reproducibility requirements and common failure modes.

Current recipes:
- `recipes/discover-gameplay-variable.md`
- `recipes/discover-damage-or-health.md`
- `recipes/trace-visible-object-to-game-state.md`
- `recipes/diagnose-colour-palette-error.md`
- `recipes/discover-input-mapping.md`
- `recipes/discover-state-transition.md`
- `recipes/characterise-collision-response.md`
- `recipes/controlled-a-b-intervention.md`

## Current recommended method
For already-captured events, prefer timeline-backed A/B rendering over replaying gameplay. RC2 sprite-order experiments are the current example: seek one historical frame, render both tie-break modes, compare outputs.
