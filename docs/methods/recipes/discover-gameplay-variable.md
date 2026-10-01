# Recipe — discover an authoritative gameplay variable

1. Define an observable state with clear control/intervention cases.
2. Capture synchronized RAM/state snapshots around transitions.
3. Diff runs and rank values by temporal correlation and plausible encoding.
4. Add write watches to candidates; record writer PC and old/new values.
5. Repeat from the same checkpoint with controlled input.
6. Manipulate the candidate at a safe boundary.
7. Observe whether gameplay changes, only presentation changes, or nothing causal changes.
8. Trace readers/writers to distinguish authoritative, derived and HUD/display values.
9. Record encoding, reset behaviour, writer(s), consumers and counterexamples.

Promote to CONFIRMED only when the evidence supports the claimed scope.
