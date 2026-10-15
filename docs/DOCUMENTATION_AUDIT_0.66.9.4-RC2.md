# Documentation audit - v0.66.9.4-RC2

## Candidate scope

v0.66.9.4-RC2 is a coherent native-reconstruction and asset-tooling candidate following the fully Windows-qualified v0.66.9.4-RC1. The immutable promoted source baseline remains v0.66.9.3 SHA-256 `6da88889dc8214c6058644e44e4d29f6a2117c9f0e92d99c8d906c280fc3462e`; RC1 archive SHA-256 `82d0ee720b40f248624e6c3b2b532ecbef83a68c80a45c68e70556a49b1b1a3a` is retained as development-line provenance.

RC2 adds the following implementation surfaces without changing the default authentic/original behaviour:

- **Native Reconstruction Lab** foundation in Workbench.
- **Sprite backend selector**: `Original`, `Shadow`, `Native`. Original remains default and oracle. Shadow displays Original and accumulates independent placed-source mismatch/first-divergence counters. Native displays the already independently reconstructed placed-sprite source renderer and remains experimental until exact-archive Windows prove-off.
- **Global existing-sprite map substitution**: presentation-only source-map -> target-map replacement with optional palette override, independent of transient sprite slot assignment. This is the first practical bridge for existing-asset car/billboard appearance experiments; arbitrary PNG/RGBA replacement is not claimed in RC2.
- **Complete TC0100SCN live charset export**: all 256 RAM-defined 8x8 2bpp characters, 16x16 atlas PNG, 256 glyph PNGs, raw 4096-byte character RAM, current text-map usage counts, JSON metadata and ZIP.
- **Stage-2 checkpoints**: `stage2-gameplay-7500.chqstate` and `stage2-enemy-10514.chqstate`, with deliberately conservative semantics. The focused regression uses both for cross-stage independent sprite-layer verification.
- **HUD Layout responsive-width fix**: the Experimental/Game Lab HUD card receives sufficient width on desktop and responsive stacking on narrower layouts so Apply controls are not clipped.
- RC1 police-beacon and semantic HUD presentation controls carry forward unchanged.

## Evidence boundary

The independent 4bpp decoder and placed sprite-source renderer remain **SHADOW VERIFIED** from the v0.66.9.3 canonical Stage-1 corpus. RC2 exposes that implementation as an experimental live Native backend, but this candidate is **not promoted to REPLACEABLE/NATIVE maturity merely because a switch exists**. Exact RC2 Windows build/runtime/shadow/native prove-off is required first. Original/emulated remains the default authoritative path.

Global sprite-map substitution changes presentation only; it does not rewrite game sprite RAM or make an enemy object become the player gameplay object. Charset export snapshots the **live** 256-character RAM set at a particular frame; dynamic character slots mean one export is not claimed to be every glyph ever used across the complete game.

The Stage-2 checkpoint names claim only general gameplay at frame 7500 and enemy-present context at frame 10514. They intentionally do not claim pre-contact, damage, target health or other stronger semantics.

## API / Script parity

New Script API actions are:

- `backend.sprite.get`
- `backend.sprite.set mode=original|shadow|native`
- `backend.sprite.reset-verification`
- `sprite.map-override source=N target=N [palette=-1..255]`
- `sprite.map-override.clear`
- `text.charset.export [palette=0..255] [scale=1..8]`

Native command parity is provided by `backend sprites ...` and `sprites map-override ...`; Workbench surfaces the backend/map-replacement controls in Native Lab and complete charset export in Graphics Lab.

## Regression / qualification contract

The focused release regression is `research/scripts/regression/regression-v06694-rc2.chqscript`. It asserts the release identity, verifies Stage-1 and both Stage-2 checkpoints with the independent placed-sprite shadow comparator, exercises live Shadow and Native backend switching, exercises global map substitution, performs a complete 256-glyph charset export, and restores Original/authentic presentation state. Full Regression includes the RC2 focused contract at its tail.

`chqdev` is the **de facto and authoritative build/release process**. Fresh Windows exact-archive qualification is `chqdev qualify --fresh`. There are **zero tolerated CTest failures; any CTest failure is fatal**. Packaging must be **deterministic** with exact **archive** verification. The focused regression and **Full Regression** must both pass before `promotable`. PowerShell/BAT **shell compatibility** wrappers remain compatibility/fallback surfaces; they do not replace chqdev authority.

## Promotion status

RC2 is a release candidate only. Portable/static validation can reject a bad package, but only fresh qualification of the exact packaged archive on Windows can promote it.
