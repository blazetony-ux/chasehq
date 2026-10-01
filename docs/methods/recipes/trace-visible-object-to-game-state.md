# Recipe — trace a visible object back to authoritative game state

1. Identify the visible sprite/tile assembly at a reproducible frame.
2. Locate its hardware descriptor and software staging source.
3. Trace the writer PC(s) of descriptor fields that change with the object.
4. Follow source reads backwards into object/gameplay state.
5. Perform a causal intervention on the suspected object record.
6. Confirm that the visible object changes/disappears/moves as predicted while unrelated rendering remains stable.
7. Record the full chain: gameplay object -> staging -> hardware descriptor -> graphics/palette -> screen.
