# v0.50.5 player-car semantic completion

This patch closes the missing-body gaps observed in the v0.50.4 `player_car` semantic evidence run.

## Evidence-backed body maps

Current player-car body maps:

- 472
- 475
- 476
- 508
- 514
- 516

Palettes: 64 and 65.

Current player-car shadow maps:

- 572
- 575
- 576
- 590
- 592
- 594

Shadow palette: 70.

The added body maps were identified from frames where the semantic object reported a shadow but no known body. In every such sampled frame, one of maps 475, 476, or 508 occupied the same position as the shadow with palette 64/65.

## Immediate validation goal

Re-run the established Stage 1 checkpoint window and confirm that `semantic_objects.csv` no longer reports `body_slot=-1` while the player-car shadow is present. Once complete, move directly to player-car priority/compositor analysis rather than further general object discovery.
