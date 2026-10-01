# Special Sprite / Palette Forensics (v0.39)

Purpose: distinguish ordinary sprite colour, zero/unwritten TC0110PCR state, overlay/stencil behaviour, and compositor errors using independently checkable evidence.

Key outputs: `palette_write_provenance.csv`, `sprite_pen_palette_forensics.csv`, `player_car_pair_044_045.txt`, `bottom_100px_reference_vs_prom.csv`, `priority_class_histograms.csv`, and `experiments_v039/`.

A palette raw value of `$0000` is black in normal rendering. v0.39 separately records whether that zero was ever written by the game; an unwritten zero and an explicit game-written zero must not be conflated. Slot 44 remains a hypothesis-driven investigation target, not a confirmed mask sprite.
