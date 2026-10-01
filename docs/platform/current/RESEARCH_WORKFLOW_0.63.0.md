# v0.63.0 Research Workflow

v0.63.0 turns the Web Workbench into the normal day-to-day research surface rather than a collection of low-level controls.

## Workflow additions

- **Current SDL frame preview**: `/api/v1/frame.png` serves a fresh PNG from the running renderer. The browser supports manual refresh and low-frequency 0.5/1/2/5-second refresh.
- **Canonical checkpoint browser**: the Workbench lists packaged `.chqstate` files together with metadata from the matching JSON sidecars and supports **Load & run** or **Load paused**.
- **Game timer freeze/resume**: the native API now exposes `timer freeze`, `timer resume`, `timer status` and `timer toggle`. The Workbench header has direct controls. This is intended for research sessions where the arcade countdown would otherwise terminate the observation window.
- **SDL always-on-top**: the native API exposes `window top get|on|off`; the Workbench can keep the SDL renderer above the browser while testing.
- **Responsive side-by-side layout**: below 1200 CSS pixels the Workbench collapses to a single column, making Windows snap-left/snap-right use practical.
- **Session log tail**: the launcher passes the active evidence session into the Web host. The Workbench can list and tail `.log`, `.txt`, and `.csv` outputs without leaving the browser.
- **Known IOC pulse buttons**: the unique pulse values already used by the Stage-1 scenario are exposed as quick buttons. They are deliberately labelled by hardware port/mask/duration rather than guessed semantic names.
- **Automated IOC single-bit sweep**: a single action restores the same canonical checkpoint for every mask, applies one pulse, advances a fixed number of emulated frames, captures a screenshot/status/events, then restores the baseline. This avoids manual repeated clicking while the in-game timer counts down.
- **Controller calibration correction**: steering-left/right discovery now requires the correct sign as well as a movement threshold, preventing return-to-centre from being accepted as steering-right.
- **Palette restore correction**: the native palette override layer now remembers the pre-override raw value and writes it back on Restore Entry / Restore All. The bank selector is explicitly labelled **View bank** because browsing a bank is not the same as forcing a hardware palette bank.

## IOC sweep evidence

A sweep creates an evidence directory under the current Web evidence root containing:

- one PNG per tested mask;
- one event-tail text file per mask;
- machine/IOC status in the JSON response;
- `manifest.json` describing checkpoint, port, masks, pulse duration and observation window.

The default masks are `01,02,04,08,10,20,40,80` on port 3. The default starting point is whichever canonical checkpoint is selected in the browser.

## Safety / authenticity

Timer freeze, handling tuning, palette freezes and IOC manipulation are research interventions. They must not be treated as evidence of authentic arcade behaviour unless the intervention itself is the subject of the experiment. Checkpoint-based sweeps deliberately reset state between candidates to prevent one intervention contaminating the next.
