# Documentation audit - v0.66.9.3-RC4

Current candidate identity: **v0.66.9.3-RC4**. Immutable promoted baseline remains v0.66.9.2 until RC4 completes its own Windows `chqdev.bat qualify --fresh` gate.

RC4 documentation has been reconciled with RC3 Runs 003-020. Stale claims corrected include the old map181/palette75/77 player-car candidate, P3:10 Brake mapping, missing-Turbo-HUD wording, unresolved palette bit packing, provisional incline/decline sign and foundation-only sound-command observability.

The current API/schema/feature/semantic registries document the new independent placed sprite-layer shadow compare, bounded read provenance, structured collision events, sound command-interface events, alpha/occupancy-aware image comparison, IOC sweep v2 state capture and strict signed-12-bit steering input validation. Audio documentation explicitly does not claim a real sound CPU, chip, mixer or PCM output.

Windows qualification remains pending for this candidate. Portable build/test/package results are evidence of source portability only and must not be promoted to Windows/SDL/Workbench qualification.
