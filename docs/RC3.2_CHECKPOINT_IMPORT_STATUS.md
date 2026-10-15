# RC3.2 checkpoint import status

RC3.2 carries the eight canonical checkpoint binaries already present in the RC3.1 source package:

- `stage1-driving-2352.chqstate`
- `stage1-gameplay-2064.chqstate`
- `stage1-target-immediate-pre-contact-5587.chqstate`
- `stage1-target-first-physical-contact-5588.chqstate`
- `stage1-target-immediate-pre-damage-6333.chqstate`
- `stage1-target-first-damage-6334.chqstate`
- `stage1-target-post-final-hit-9548.chqstate`
- `stage1-end-level-9988.chqstate`

Two additional October 2 research states are known from the project evidence store but their raw state bytes were not available to this Linux packaging session: `stage1-page4-material-pre-4704.chqstate` and `stage1-selector5-page0-8664.chqstate`. They are deliberately **not** represented by placeholders or reconstructed data in this archive.

When the real files are available locally, import and validate them as genuine checkpoint binaries before adding them to a later exact package. This does not block the RC3.2 script/evidence/LAN implementation, but it remains a content-import gap relative to the wider Stage-1 research checkpoint set.
