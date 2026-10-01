# Chase H.Q. Native v0.60.3 — Research Session Orchestrator

v0.60.3 focuses on startup ergonomics for the live Research Workbench.

## Added

- `Start-ChaseHQResearch.ps1`: one-command launch of the SDL emulator plus Research API/control console.
- API readiness polling before priming commands are sent.
- `-Attach` mode for an already-running emulator.
- `-Preset` and `-Session` JSON session definitions.
- Optional `.chqscript` priming through `-Experiment`.
- Arbitrary live API priming through repeatable/array `-PrimeCommand` values.
- Arbitrary launch-time diagnostic pass-through through `-EmulatorArgs`.
- Convenience switches for checkpoint, no-collisions, infinite time, unlimited turbo, auto turbo and handling overrides.
- Explicit shared `-Port` handling for emulator and `chqctl`.
- Optional `-Fresh` evidence-directory reset and `-NoShell` automation mode.
- Included `sessions/default.json` and `sessions/target-posthit.json` examples.

All v0.60.2 Live Research API commands and established launch-time CLI diagnostics remain available.
