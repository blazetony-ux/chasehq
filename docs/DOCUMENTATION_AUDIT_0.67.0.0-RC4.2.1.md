# Documentation audit: v0.67.0.0-RC4.2.3

Candidate 0.67.0.0-RC4.2.3; Windows qualification pending. chqdev is authoritative.

This release changes only the regression-suite wiring, audit coverage, version identity and documentation. The RC4.2 emulator, content browser and native runtime source remain unchanged.

The RC2.1, RC3.2 and RC4.2 BG Shadow scripts have identical executable bodies after stripping release-version, echo and comments. The Full Regression previously ran all three. The consolidated Full Regression runs the current RC4.2.1 equivalent once; historical scripts are preserved and runnable separately. This is a genuine redundant end-to-end scenario, not merely repeated setup commands.

The RC3.0/RC3.1 course mapping scripts remain included because their diagnostic assertions and export artifacts differ. Other repeated checkpoint loads, clears and state reads are deliberately retained where stateful isolation or coverage would otherwise be at risk.

The legacy <850 structural-command budget remains unchanged. If it still fails, the candidate must not be promoted until a separate evidence-backed consolidation closes that gap without losing unique coverage. Fresh Windows `chqdev qualify --fresh` is mandatory.
