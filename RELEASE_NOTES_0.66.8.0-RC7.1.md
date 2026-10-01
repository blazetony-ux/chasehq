# ChaseHQ-Native v0.66.8.0-RC7.1

Packaging/build-validation hotfix for RC7. No emulator/runtime behavior change.

- Fixes `Validate-Workbench.ps1` so function extraction ends at the next PowerShell function declaration instead of assuming two implementation functions are adjacent.
- This removes the false failure `timeline event-window implementation is not streaming/non-destructive` introduced when RC7 inserted helper functions between the already-proven timeline functions.
- ROM staging validation output now labels source and staged-destination checks separately instead of printing the same generic PASS line twice.
- RC7 timeline analysis/playback implementation is otherwise unchanged.
