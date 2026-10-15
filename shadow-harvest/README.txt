RC5.2 authenticated native-equivalence checkpoint corpus
=======================================================
Nine checkpoints were recovered from the user's original Workbench evidence:
actual session 20261011_084951, Runs 003, 006, 008 and 009.
The manifest records exact source archive members, byte counts and SHA256.
No ROM files are included in this RC5.2 release source candidate.

The RC5.2 runner checks the nine SHA-256 hashes before executing.
Use ONLY tools/Run-RC5.2-Native-Clear-Equivalence.ps1 in the RC5.2 source.
Do NOT use the old RC5.1 harvested wrapper or its scheduled patch fallback.
The ordinary baseline reference checkpoints and ROM set are supplied from
separate previously qualified RC5 directory via -BaselineRoot.

This corpus combines unmodified reference gameplay, experimental class 0/4/6
material overrides, and assisted autopilot states. Never label the assisted
or experimental branches as unmodified reference gameplay.

The RC5.2 runner tests compare (ROM) and guarded native modes separately.
A missing checkpoint, failure to verify hashes, absence of native executions
in positive-clear cases, CPU/RAM/cycle mismatch or shadow failure MUST FAIL.
No Workbench emulator restart. This is not a Windows qualification run.
