# Stage-1 road material type-6 — RC3.2 evidence state

Status: **BEHAVIOUR UNDERSTOOD for the tested Stage-1 contexts; semantic name remains deliberately unclassified.**

The causal path observed in the October 2, 2026 evidence is:

`0x10944C -> CPU-B cached byte 0x1021CA -> reader PC 0x004EF0 -> CF2/CF4 intermediate state -> 0x003B70/0x004120 -> render buffers -> writer PCs 0x0046BC/0x0046C0/0x0046CC/0x0046CE -> TC0150 road RAM`.

In the controlled comparisons, course byte `0x00` produced the paved-like branch and forced `0x06` produced the dirt-like branch. Representative road-RAM words included paved `8054 80A8 8A8D 2102` versus type-6 `99C0 99C0 9A8D F244`. These are evidence-backed observations for the tested Stage-1 records, not a global declaration that byte `0x06` means “dirt” everywhere.

A one-frame checkpoint or screenshot can still show the previous rendered state because the game alternates/back-buffers road state. Material investigations therefore need multi-frame structured evidence and write traces rather than a single visual frame.

RC3.2 closes two tooling gaps exposed by this work: `legacytrace output` files are copied into the authoritative run bundle with source/hash metadata, and script-created/loaded checkpoints are recorded under the run-local `artifacts/checkpoints/` tree.
