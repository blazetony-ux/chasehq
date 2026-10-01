# Recipe — discover damage / health state

**Prerequisites:** reproducible pre-hit state and controllable damage event.

1. Capture a no-hit control run.
2. Restore and perform exactly one hit; diff RAM/events.
3. Repeat with multiple hit counts and compare monotonic/step behaviour.
4. Watch candidate writes at the exact hit event.
5. Trace writers and downstream defeat/phase transitions.
6. Intervene on candidates before/after the hit.
7. Verify whether changing the candidate alters survivability/damage outcome rather than merely animation, score, timer or phase state.
8. Test boundary values carefully (near zero, zero, one above/below) and document underflow/saturation.

A hit counter, response timer or animation phase must not be labelled health merely because it decrements on impact.
