# Chase H.Q. Native v0.60.2 — Live Research API Expansion

v0.60.2 is a tooling/productivity release. It extends the v0.60.1 external API so more investigations can be assembled interactively from a second console without restarting the emulator.

Highlights:

- named live snapshots (`snapshot save/restore/list/remove`)
- 68000 register inspection and live register edits
- stable watch IDs, arbitrary watch ranges and watch removal
- individual patch removal
- new `patch replace` mode for replace-on-write experiments
- configurable Research Event buffer size
- `why mem` recent writer/reader query
- `changed` range query for recent changed writes
- live IOC steering / port / XOR override inspection
- live configuration of the established heavyweight generic tracer
- live configuration of execution provenance/callstack/callgraph capture
- synchronous `chqctl --wait run N`
- reusable `chqctl script FILE` and shell `source FILE`
- expanded capabilities/version reporting

The original launch-time diagnostics remain supported and are still appropriate for reproducible unattended forensic runs.
