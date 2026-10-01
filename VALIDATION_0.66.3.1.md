# v0.66.3.1 validation

Static/package validation performed before handoff:
- JavaScript extracted from Workbench HTML passes `node --check`.
- all research script `include` targets resolve.
- all research scripts contain `# name`, `# category`, and `# purpose`; regression scripts also contain regression version/scope metadata.
- full-regression COMPLETE marker is final.
- obsolete `web-evidence` and `script-runs` paths are absent from launcher/workbench implementation.
- machine-readable JSON manifests/schemas parse successfully.
- generated current-layout regression snapshot path remains comfortably below the traditional Windows MAX_PATH threshold for the user's known project root.

Windows runtime validation still required on the development machine: build, `-Restart`, run v0.66.3.1 reliability regression, then Full Regression and verify inline/artifact-viewer thumbnails plus selected-row history actions.
