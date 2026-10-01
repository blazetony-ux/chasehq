# v0.59.4 Paged Debug UI

The F1 overlay is now page-based so new telemetry does not overflow one narrow panel.

- F1: show/hide debug overlay
- F11: next page
- Shift+F11: previous page

## Page 1 — DRIVING

Live speed/internal speed, distance, score, processed steering, road classification/flags, left/right boundaries, road width, centre, car lateral position, absolute + normalized centre error, curve, grade, course bank/record, turbo state and enabled assists.

## Page 2 — HANDLING

Turn state/table index, native and applied forward/lateral handling coefficients, forward/lateral components, internal speed and handling override state. The page deliberately keeps native and applied values separate.

## Page 3 — TARGET

Player/target longitudinal positions, separation, object-local status/contact candidate, `0x10A092` target movement magnitude, `0x10A096` motion-control value, interaction type/timer/counter, `0x10018D` defeat flags and `0x1002D0` defeat timer candidate.

The UI uses candidate language for defeat semantics that have not yet been fully generalized across stages.

## Page 4 — SYSTEM

CPU PCs, checkpoint slot and interactive debugger controls. Moving the key reference to its own page frees the other pages for state that matters during live experiments.
