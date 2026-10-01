# v0.64.8.2 — Intervention Tooling & Track Intelligence

## Workbench / Script Console
- Exposes safe Research Script actions for `memory.write`, register read/write, live patch freeze/replace/suppress/list/remove/clear, and checkpoint save.
- Adds optional `expect=` compare-before-write guards for memory writes and freezes.
- Raw `write` is also permitted through the safe native-command bridge; shell metacharacters remain blocked.
- Script lifecycle UI now disables run controls while executing, enables Stop only while a stoppable run is active, continuously shows elapsed time, and persists COMPLETE / FAILED line N / STOPPED state.
- PowerShell launch/web scripts are UTF-8 BOM encoded for Windows PowerShell 5.1; HTTP text/JSON responses explicitly declare UTF-8.

## Gameplay Registry
- Groups cards by Vehicle, Road / Track, Vehicle / Render, Driving / Road, Turbo / Input, and Research / Open.
- Adds current record +0 signed curvature and direction.
- Adds record +1 signed target, smoothed `0x10A04E`, and the confirmed three-band negative/neutral/positive renderer classification.
- Adds player render selector `0x10A04A`, cached selector `0x10A076`, decoded bank/sub-index, selected edge state `0x10A04C`, and selected edge distance/control `0x100300`.
- Adds the complete current 8-byte live track record and record address/index.
- Shows checkpoint/mode context when a packaged checkpoint is loaded through the Workbench.
- Improves raw/decoded presentation for internal speed and track-record offset.

## Track View
- Keeps the authoritative +0 curvature-derived centreline.
- Colours individual recorded segments by the confirmed +1 three-band renderer state: blue negative, grey neutral, orange positive.
- Hover text exposes record index, +0 curvature, +1 target, smoothed state and band.
- The legend deliberately avoids claiming uphill/downhill until the causal visual experiment proves the physical meaning.

## Research state carried forward
- `0x10A05A` is the current 8-byte track-record offset into the mutable `0x109000` live page/cache.
- Record +0 signed curvature is confirmed: negative=left, positive=right.
- Record +1 -> `0x10A04E` smoothing -> +/-0x40 player render selector bank mechanism is confirmed; physical gradient meaning remains in progress.
- Player `+0x28` is a packed sprite/render attribute word; bit `0x0080` toggles periodically, exact hardware semantic unresolved.


## v0.64.8.2 Script Console Reliability Fix

- Fixes successful scripts being marked FAILED when only the post-run dashboard refresh fails.
- Post-run refresh failures are now warnings and do not overwrite a successful script result.
- Copy Output now falls back to a hidden textarea/document.execCommand copy path when the Clipboard API is present but rejects the request.
- Diagnostics copy uses the same robust clipboard helper.
