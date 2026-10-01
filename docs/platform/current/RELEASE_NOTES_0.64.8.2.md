# ChaseHQ-Native v0.64.8.2

Frontend reliability hotfix for v0.64.8.x.

- Restores the Evidence Viewer client functions accidentally omitted from v0.64.8/0.64.8.1. Their missing definition caused successful Script Console runs to be relabelled FAILED during post-run refresh.
- Post-run refresh/preview/evidence hooks are now isolated from command execution status. A successful script remains COMPLETE even if a secondary UI refresh fails.
- Copy Output now attempts the synchronous legacy copy path while the browser click activation is still valid, then Clipboard API, then selects the complete output and instructs Ctrl+C if browser policy blocks both programmatic paths.
- Diagnostics copy uses the same robust copy helper.

No emulator/gameplay semantics were changed.
