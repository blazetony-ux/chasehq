# Recipe — characterise collision response without destroying detection evidence

1. Reproduce one collision from a checkpoint.
2. Trace writes to authoritative player position/speed/state around contact.
3. Identify response writes separately from upstream collision detection/event selection.
4. Suppress one proven consequence only and repeat.
5. Expand the suppression set only when another response path is independently observed.
6. Confirm that detection, scoring/object state and event telemetry remain active.
7. Document exactly what the intervention does **not** suppress.

This produces an evidence-scoped research aid rather than a misleading global `no collisions` claim.
