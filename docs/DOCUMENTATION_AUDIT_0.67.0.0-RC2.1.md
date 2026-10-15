# Documentation Audit — v0.67.0.0-RC2.1

> Current development candidate: **v0.67.0.0-RC2.1**. Exact source baseline: qualified **v0.67.0.0-RC1.1** archive SHA-256 `ea4ebaafe07cd1c15b64c07ed8e92e32fd0e39cdcc360e31dd7b09c676b77f1d`.
## v0.67.0.0-RC2.1 — TC0100SCN BG opaque pen-0 corrective

- Windows RC2 focused Run001 and Full Regression Run002 both completed PASS, but the new BG Shadow telemetry exposed a deterministic renderer mismatch rather than a qualification-infrastructure failure.
- **Physical BG0 compared cleanly throughout**: focused `228/228` and Full Regression final focused section `125/125` comparisons with zero BG0 mismatches.
- **Physical BG1 mismatched on every compared sample** when it was selected as the opaque bottom plane. First divergence was `original_pixel=0x00000000` versus `native_pixel=0xff000000` with matching coverage at the bottom-screen pen-0 area.
- The captured Stage-2 reconstruction reports `tileControl6=0x0028`; bit 3 therefore selects physical BG1 as the bottom/opaque TC0100SCN plane.
- Source inspection localized the defect exactly: the authoritative runtime returns pixel value `0` for palette pen 0 while still setting bottom-layer priority/coverage; the independent renderer instead converted pen 0 through TC0110PCR and produced opaque black.
- RC2.1 makes independent BG palette conversion return zero for pen 0 while retaining coverage on opaque passes, and adds a unit regression for the exact physical-BG1 opaque-pen-0 contract. No sprite, text/HUD, road, gameplay, script-engine or general-tooling behaviour is intentionally changed.


## Purpose

RC2 is a native-rendering implementation milestone, not another general tooling pass. The broad-tooling freeze remains in force. It promotes the independently verified TC0100SCN text/HUD renderer to the normal Native runtime path and introduces the first independent TC0100SCN BG0/BG1 Shadow/native implementation.

## Promotion evidence carried into RC2

The sprite subsystem remains **NATIVE**. RC1.1 qualification closed the forensic overlap-density corrective and Full Regression passed.

The TC0100SCN text/HUD renderer is promoted to **NATIVE** on the evidence already collected on RC1.1:

- Full Regression accumulated **14,397 Shadow-compared frames with 0 mismatch frames**.
- Canonical Run003 exercised ten Stage-1/Stage-2 checkpoints and recorded **230 clean Shadow frames**, **380 Native-only frames**, **0 Native-only blocked frames** and `promotionReady=true`.
- Native-only structured snapshots at Stage-1 gameplay, Stage-1 end-level and Stage-2 gameplay reconstructed exactly with zero mismatched pixels.

Accordingly RC2 changes the default text backend from Shadow to Native while retaining Original, Shadow and Native-only verification modes.

## New independent TC0100SCN BG0/BG1 implementation

RC2 adds `src/tc0100scn_bg_native.cpp/.h`. The implementation independently performs:

- standard-width BG0/BG1 tilemap addressing;
- packed 8x8x4 ROM tile decode;
- per-scanline rowscroll lookup;
- signed X/Y scroll and visible-area origin handling;
- screen flip plus per-tile X/Y flips;
- TC0110PCR palette conversion;
- bottom-layer opaque pen-0 semantics and upper-layer transparent pen-0 semantics.

The independent path deliberately does **not** call `tc0100scn_geometry.h` or `Video::tile_pixel`, so Shadow comparison can detect errors in the authoritative/original implementation instead of sharing its helpers.

Runtime modes are `Original`, `Shadow`, `Native`, and `Native-only`. **Shadow is the RC2 default BG mode** while parity is established. Per-layer telemetry distinguishes BG0 and BG1 comparisons/mismatches and records the first divergent frame/layer/pixel.

## API / Workbench surface

RC2 adds:

- `api backend.bg.get`
- `api backend.bg.set mode=original|shadow|native|native-only`
- `api backend.bg.reset-verification`

`native.progress` now reports the promoted Native text/HUD subsystem and live TC0100SCN BG Shadow/native status.

## Focused regression

`research/scripts/regression/regression-v0670-rc21.chqscript` locks:

- Native sprites as normal runtime path;
- Native TC0100SCN text/HUD as normal runtime path;
- independent BG Shadow comparison at Stage1 gameplay 2064, Stage1 driving/rowscroll 2352, first damage 6334, Stage2 gameplay 7500 and Stage2 enemy 10514;
- exact structured snapshot reconstruction with the mixed Native/Shadow display stack.

## Qualification boundary

Portable tests prove the independent decoder contract and project consistency only. Promotion of BG0/BG1 beyond `NATIVE-CANDIDATE` requires the exact RC2 Windows/SDL archive to pass `chqdev.bat qualify --fresh` and produce clean live Shadow evidence. Do not mark BG0/BG1 `SHADOW VERIFIED` or `NATIVE` before that evidence exists.

## Next implementation milestone

If RC2 qualification reports zero BG0/BG1 pixel and coverage mismatches, widen the canonical BG corpus and move BG0/BG1 to Native-only hard-disable proof. The intended visible-rendering sequence remains:

**Native sprites + Native text/HUD -> TC0100SCN BG0/BG1 -> TC0150ROD road -> compositor.**

## Release-process authority

`chqdev` is the **de facto and authoritative build/release process** for this candidate. The canonical fresh promotion command remains `chqdev.bat qualify --fresh`; it is fail-closed and requires **zero tolerated CTest failures; any CTest failure is fatal**. Packaging must remain a **deterministic archive** with a byte-identical second-package reproducibility check and exact extraction verification. PowerShell/BAT release tooling remains **compatibility/fallback only** and must not become a second release authority.
