# Recipe — controlled A/B intervention

**Use when:** correlation suggests a variable/routine matters but causality is not proven.

1. Start from one deterministic checkpoint.
2. Run a control with unchanged state/input and capture the relevant evidence window.
3. Restore exactly the same checkpoint.
4. Change one candidate variable/condition only.
5. Replay the same input/window.
6. Find the first behavioural/state divergence.
7. Repeat enough times to rule out nondeterminism.
8. Record both supporting and contradictory effects.

**Confirmation standard:** the intervention should produce the predicted downstream change while unrelated processing remains intact. If several variables are changed, the experiment is not a clean causal test.

**Failure mode:** a candidate can correlate perfectly because it is a display/derived value. Manipulation and downstream provenance distinguish correlation from authority.
