# Validation — v0.66.8.0 Forensic Timeline v1

## Final release result

**PASS / PROVEN RELEASE — 2026-09-29.**

The release was promoted from RC7.4 after the focused bounded-history gate and one complete Full Regression pass on Windows/SDL:

- Bounded History Discovery: PASS, 1.071 s.
- Full Regression Suite: PASS, 78.422 s, session `20260929_195302`, run `002`.
- Browser remained stable through the historical `script.runs` section that previously exhausted Chrome memory.
- Earlier RC7 focused portable-analysis and real turbo-analysis runs passed.

The checklist below is retained as the historical v0.66.8.0 qualification path; all promotion-blocking gates are closed.

## Focused gate

1. `Build-Debug.bat`
2. `Start-ChaseHQ.ps1 -Restart`
3. Run `Regression - v0.66.8.0 Forensic Timeline v1`.
4. Confirm one timeline directory contains `manifest.json`, `frames.csv`, `writes.csv` and four frame checkpoints from the regression.
5. Confirm `timeline.load` sets `status source=timeline`; ordinary memory/register/sprite/gameplay actions succeed; `timeline.unload` restores `source=live`.
6. Run `Capture Turbo Complete-Cycle Forensic Timeline`.
7. Confirm trim leaves exactly the event window around `0x10040F & 0x02`, with one pre-frame and one post-frame.
8. Confirm `analysis/memory-write-candidates.csv` and `analysis/ranked-candidates.json` are produced.
9. Restart, import the same timeline by name and inspect it without replaying gameplay.
10. Test `timeline.fork-live`, then perform a harmless read/step on the fork.
11. Run Full Regression once.

## Audio expectation

Do not fail the build because authentic audio timeline capture is absent: current native audio is still a stub. The release must instead expose this limitation truthfully in the timeline manifest and audio capabilities.

## RC2 Windows hotfix validation

RC2 fixes three defects exposed by the first real timeline run:

1. Script-library dropdown/filter selection must auto-load the selected script into the editor; browser bootstrap must fail if script discovery succeeds but no selected script loads.
2. Timeline-heavy `control.run-frames` waits are progress-aware and no longer fail solely because full-state recording takes longer than 15 seconds. A true no-progress stall remains bounded.
3. Launcher browser bootstrap uses a unique query URL, clears stale frontend-error diagnostics for the new server instance, and allows a larger startup window so an old browser tab/error cannot be mistaken for the new launch.

The Build-Debug / Prepare-Project progress labels are also normalized so only the top-level build owns `[x/6]` numbering.

The existing RC1 turbo recording is valid evidence despite the browser timeout: 256 frames, frame 2064 through 2319, 711,374 recorded writes, turbo-active rise at 2065 and fall at 2275. Do not replay gameplay merely to replace that dataset; use the packaged recovery script after RC2 boots.

## RC3 streaming-analysis validation

RC2 long-run resilience is already proven on Windows: PASS, 161 recorded frames, 438,415 writes, 18.372 s. RC3 specifically addresses the later 360-second `timeline.trim.event` timeout on the 711,374-write turbo capture.

Required focused proof:

1. Build/launch normally.
2. Run `Timeline / Recover Existing Turbo Forensic Timeline`.
3. `timeline.trim.event` must return a non-destructive event window with rise frame 2065, fall frame 2275, view 2064..2276.
4. The original recording must still report its complete source range 2064..2319 and retain all original frame checkpoints/write stream.
5. `timeline.scan.memory` must complete from the event-window view and emit ranked candidates/writer groups/report.
6. Timeline import + ordinary gameplay/sprite inspection must still work afterwards.

Do not replay turbo to perform this validation.

## RC5 focused startup/recovery gate

1. `Build-Debug.bat` must pass PowerShell parse validation.
2. `Start-ChaseHQ.ps1 -Restart` must reach Web Workbench READY.
3. Startup/session metadata must report build `0.66.8.0`, not the stale `0.66.7.4`.
4. If Web startup fails, the support bundle must contain `web-startup.log` plus redirected stdout/stderr with the real child-process failure.
5. Run `Recover Existing Turbo Forensic Timeline`; it must not invoke `timeline.trim.event` and must analyze trigger 2065 / stop 2275 directly.

## RC6 focused validation

Run `Regression - v0.66.8.0 RC6 Legacy Timeline CSV Compatibility`, then `Recover Existing Turbo Forensic Timeline`. The existing RC1/RC2-derived turbo recording must be analysed without replaying gameplay. `timeline.scan.memory` must accept both quoted legacy CSV fields and native unquoted CSV fields.

## RC7 focused validation

1. Run `Regression - v0.66.8.0 RC7 Portable Automated Timeline Analysis`.
2. Confirm the run bundle contains `artifacts/timeline-analysis/`, ranked candidates, writer groups, disassembly, milestone JSON, structured frame snapshots and HUD_TURBO graphics-correlation JSON.
3. Confirm the regression's `timeline.play` visibly advances the SDL historical frames and leaves the timeline read-only/loaded until unload.
4. Run `Analyze Existing Turbo Forensic Timeline` against the existing turbo capture; do not replay gameplay.
5. Only after both focused runs pass, run Full Regression once as the promotion gate.

## RC7.3 Workbench browser-memory/history validation
- Start-ChaseHQWeb.ps1 remains ASCII-only: PASS.
- Extracted browser JavaScript passes `node --check`: PASS.
- Show thumbnails default is OFF: PASS.
- Live output DOM bound (`MAX_LIVE_OUTPUT_BLOCKS=120`): PASS.
- Artifact thumbnail bound (`MAX_ARTIFACT_THUMBNAILS=24`) and lazy loading: PASS.
- Compact artifact type summary path present: PASS.
- RC-aware run version derivation and Status filter wiring present: PASS.
- ZIP integrity: PASS.
- Windows/PowerShell build/runtime proof remains required on the user development machine.


## RC7.4 bounded discovery hotfix
- script.runs list responses are summary-only and paginated.
- Browser display has a per-command text cap; authoritative logs remain complete.
- Focused regression: regression-v06680-rc74-bounded-history.chqscript.
