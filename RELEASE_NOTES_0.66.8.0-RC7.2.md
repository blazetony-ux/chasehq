# ChaseHQ-Native v0.66.8.0-RC7.2

Build-validation-only hotfix for RC7.1. No emulator/runtime behavior change.

- Fixes the RC7 portable timeline analysis/playback static check in `Validate-Workbench.ps1`.
- RC7.1 incorrectly tested `$web`, which is only the filesystem path to `Start-ChaseHQWeb.ps1`, instead of `$text`, which contains the Workbench source text.
- The actual RC7 Workbench already contains `Get-TimelineAnalysisDirectory`, `Invoke-TimelineAutomatedAnalysis`, `timeline.analyze.auto`, and `timeline.play`; the validator was checking the wrong variable.
- Adds direct package-side validation that all four RC7 wiring markers exist in the Workbench source and that the timeline scan targets the current-run analysis artifact directory.
- Runtime behavior remains identical to RC7.1.
