# Cross-Game Comparative Reverse Engineering

## Scope rule

**Chase H.Q. is the sole current development target.** Other games are research evidence only when comparison can materially advance Chase H.Q. accuracy or understanding. This is not a commitment to implement additional games.

A comparison is justified by a concrete Chase H.Q. question: uncertain register semantics, sprite/road/priority behaviour, IOC/input handling, interrupts, CPU/device communication, sound, palette/mixer behaviour, initialization, or another hardware mystery.

Use comparison to distinguish game-specific behaviour from shared hardware behaviour. Record what was compared, the evidence, differences, and confidence; do not copy assumptions blindly between revisions or related boards.

## Games of future research interest

Primary Taito Z-family research candidates include S.C.I. / Special Criminal Investigation (especially relevant as the Chase H.Q. sequel), Night Striker, Continental Circus, Enforce, Battle Shark, Aqua Jack, Double Axle / Power Wheels, Racing Beat, and Space Gun. Super Chase: Criminal Termination is of later lineage interest rather than an immediate like-for-like comparison.

The useful candidate depends on the subsystem under investigation. Maintain this as a research list, not a support roadmap. Other related titles may be added when evidence shows they share a device or behaviour relevant to Chase H.Q.

## Method

Triangulate where possible: Chase H.Q. observation <-> related-game observation <-> trusted hardware/MAME/source evidence. Prefer targeted traces and register/state comparisons over visual similarity. Cross-game findings should feed the hardware knowledge/provenance documentation and reusable interfaces only after the behaviour is understood in Chase H.Q.
