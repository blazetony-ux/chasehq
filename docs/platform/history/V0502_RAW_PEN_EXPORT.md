# v0.50.2 Raw-pen sprite export visibility

The palette-70 investigation showed that the legacy `--sprite-export` PNGs could contain structurally valid sprite geometry and alpha while appearing black because the current runtime TC0110PCR palette entries resolved to black/zero RGB. This made full reconstructed sprites visually unreadable even though the spritemap/tile reconstruction itself was valid.

v0.50.2 keeps the original native-palette files for authenticity and adds deterministic forensic companions for every exported sprite:

- `assembled_raw_pen.png` — unscaled spritemap reconstruction with each non-zero 4bpp pen rendered as greyscale `pen * 17`.
- `rendered_raw_pen.png` — the same forensic pen view after the current zoom/chunk distribution.
- `mask.png` — binary white-on-transparent silhouette of every non-zero source pixel in the unscaled reconstruction.
- `contact_sheet_raw_pen.png` — a contact sheet built from the raw-pen reconstructions.

These files do not depend on palette RAM and therefore distinguish "valid graphics hidden by palette state" from "no sprite graphics reconstructed" without changing the emulated machine state, priority logic, sprite selection, or normal renderer.

The native `assembled_unscaled.png`, `rendered_zoomed.png`, and `contact_sheet.png` remain present and unchanged for comparison.
