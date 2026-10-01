# Recipe — discover a game state-machine transition

1. Obtain checkpoints immediately before and after the transition where possible.
2. Run a control that does not trigger the transition.
3. Trigger the transition deterministically and capture rolling pre/post event history.
4. Diff RAM/registers/events and identify values that change before presentation changes.
5. Watch candidate writers and trace the branch/routine that commits the new state.
6. Intervene on the candidate to delay/prevent/force the transition.
7. Separate trigger condition, committed state, response timer and visual aftermath.
8. Document the earliest causal state change as well as downstream presentation effects.
