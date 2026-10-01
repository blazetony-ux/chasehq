# v0.62.4 Evidence + Controller Capture Fix

## Evidence capture correction
The v0.62.3 Web Debugger **Capture evidence** button called `snapshot save` directly. Named snapshot creation intentionally pauses the emulator, so the button left a previously-running machine paused after capture.

v0.62.4 routes the button through `/api/v1/evidence/capture`. The endpoint now:
1. records whether the machine was running or paused;
2. pauses a running machine to establish one consistent capture boundary;
3. saves `state.chqstate`, `screenshot.png`, `events.csv`, `sprites.csv`, and `manifest.json` in one evidence directory;
4. resumes only if the machine was running before capture;
5. records pause/resume provenance in the manifest.

This preserves deliberate paused-state investigations while making evidence capture non-disruptive during normal execution.

## Controller mapping wizard
The Web Debugger now includes a guided controller mapping wizard. It prompts sequentially for steering-left, steering-right, accelerator, brake, turbo, start, and credit; detects the controller axis/button used; records raw samples; and saves the resulting JSON under `controller-mappings/` through `/api/v1/controller/mapping`.

The wizard discovers the physical controller mapping only. Binding those actions to Chase H.Q. IOC masks remains a separate evidence-driven step.
