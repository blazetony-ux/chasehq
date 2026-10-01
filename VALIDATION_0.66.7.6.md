# v0.66.7.6 validation

Status: **RELEASE CANDIDATE - Windows/SDL proof required**.

## Required Windows prove-off

1. Run `Validate-Release.ps1 -SkipBuild` for static release gates.
2. Run `Build-Debug.bat` and require the normal test gate (only the exactly documented historical `cpu_bus_rom_tests (SEGFAULT)` signature may be tolerated).
3. Launch with `Start-ChaseHQ.ps1 -Restart`.
4. Exercise all five Docs / Knowledge modes and article-only searches: `writer context`, `authoritative producer`, `aggregate sprite contribution`.
5. Verify the same queries through `/api/v1/docs/search` and `.chqscript` actions.
6. Export final HTML with key evidence; verify article sections and no empty API/script/checkpoint/docs headings. Regenerate/spot-check PDF once.
7. Verify the complete ChatGPT handover display, copy and download path.
8. Exercise SDL pause/resume/step, show/hide, minimize/restore, fullscreen, scale and always-on-top with API parity.
9. Capture a layered frame and confirm exact reconstruction plus individual-sprite hide/show/solo/opacity/cropped-asset metadata.
10. Smoke-test major Workbench tabs and run `Regression - v0.66.7.6 Docs / Knowledge Consolidation`, then Full Regression.

Do not mark v0.66.7.6 proven until the above passes and an evidence bundle is retained.
