# Continue ChaseHQ-Native from v0.66.9.0-RC3.0

Treat RC2.9 as the authoritative Windows/SDL-proven baseline and RC3.0 as the current Course Mapping / Live Survey candidate until Windows proof completes. This is the authoritative continuation.

RC3.0 exposes the native course follower live through Workbench/API/Script Console, adds lateral bias for controlled branch mapping, Track Recorder v2, player driven-line overlay, confirmed Stage-1 target tracking, generic unclassified visible-sprite evidence, raw unclassified surface signatures, reconstructable map exports, and bundled mapping survey scripts.

Do not assign AI-car/obstacle/material semantics without evidence. Do not reopen solved target-health, brake-lamp or selector mechanisms.

## Immediate next task

Build RC3.0 on Windows, run the focused RC3.0 mapping regression and Full Regression, then run the 600-frame mapping shakedown before any lengthy survey.

RC3.0 corrected-candidate note: the first RC3.0 archive is withdrawn because the focused Windows regression failed at `course.follow.configure`; `Set-CourseFollowConfig($args)` collided with PowerShell automatic `$args` (`System.Object[]`) and failed on `ContainsKey()`. The corrected replacement uses `$config` and adds a validator guard. Re-run the focused regression before surveys.
