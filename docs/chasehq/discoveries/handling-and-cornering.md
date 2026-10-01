# Discovery record — handling / cornering transform

**Status:** coefficient path CONFIRMED; semantic interpretation beyond movement coefficients remains cautious

## Outcome
The normal player movement path uses paired ROM coefficients around `$00AD46` with internal speed in D2.

- `$008AAA`: D4 contains forward coefficient before multiply.
- `$008AAC`: D4 contains forward product before arithmetic shift by 8.
- `$008AB2`: D3 contains lateral coefficient before multiply.
- `$008AB4`: D3 contains lateral product before arithmetic shift by 8.
- normal lateral movement ultimately feeds `$008B84 -> $10A044`.

Observed forward coefficients include `$0100`, `$00FE`, `$00F4`; lateral coefficient increases with turn magnitude.

## Method
Instruction-level tracing captured coefficient lifetime after table read and before multiplication. Experimental overrides were deliberately applied after original table selection but before multiply, preserving ROM bytes and downstream game logic.

## Experimental aid
`--cornering-scale 1.15 --cornering-speed-retain 1.00` is the best tested **mapping aid** carried into the current project state. It is not asserted to be authentic arcade behaviour.

## Open questions
Whether another road-surface/airborne/traction multiplier exists downstream; whether any single coefficient can correctly be described as tyre grip; and what authentic tuning best matches the original behaviour.
