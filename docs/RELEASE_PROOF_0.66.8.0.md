# v0.66.8.0 release proof

**Release status: PROVEN — 2026-09-29**

The final release is the RC7.4 runtime promoted without emulator/runtime semantic changes. Promotion changed documentation and release/package state only.

## Final Windows/SDL gates

- `Regression - v0.66.8.0 RC7.4 Bounded History Discovery`
  - status: PASS
  - duration: 1.071 s
  - session: `20260929_195302`
  - run: `001`
  - purpose: prove script-run discovery is bounded and summary-only so accumulated history cannot exhaust browser memory.
- `Full Regression Suite`
  - status: PASS
  - duration: 78.422 s
  - session: `20260929_195302`
  - run: `002`
  - bundle contents: 902 entries
  - terminal marker: `=== ChaseHQ Full Regression: COMPLETE ===`

The final Full Regression passed through the `script.runs` history-discovery section that had crashed Chrome in RC7.2/RC7.3. RC7.4 bounded/paginated summary-first history discovery therefore closes that release blocker.

## Earlier focused v0.66.8.0 proof

- portable automated timeline-analysis regression: PASS;
- real `Analyze Existing Turbo Forensic Timeline`: PASS without replaying gameplay;
- long-run timeline recording/progress-aware control proved on Windows;
- legacy first turbo capture successfully reopened and analysed offline;
- historical recorded states inspected through normal gameplay/sprite APIs;
- recorded-state SDL playback and timeline unload/live restoration exercised by regression.

## Release conclusion

v0.66.8.0 is the current proven baseline. The release gate is closed and normal project work should return to Chase H.Q. gameplay/rendering/reverse-engineering objectives. Further tooling should be driven by concrete investigation needs.
