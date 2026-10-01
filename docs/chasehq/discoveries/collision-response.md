# Discovery record — collision response family

**Status:** listed response writes CONFIRMED; complete family not guaranteed

## Method
A narrow collision response write was first suppressed, then longer surveys revealed additional response PCs. Each candidate was traced to its effect on player lateral position or internal speed. Suppression was kept evidence-scoped so upstream detection/event logic remained active.

## Proven response writes
Player lateral coordinate `0x10A044`:
- `$00A142` subtract displacement
- `$00A156` add displacement
- `$00A1BE` alternate subtract
- `$00A1C4` alternate add

Internal speed `0x10041C`:
- `$00A200` computes/stores 3/4 of prior speed (25% loss)

## Intervention
`--no-collisions` suppresses these proven consequences for mapping experiments while retaining upstream interaction state, scoring and target/object processing.

## Limitation
This must not be described as globally disabling every collision consequence. Scenery, airborne/landing, off-road or other object families may use additional paths.
