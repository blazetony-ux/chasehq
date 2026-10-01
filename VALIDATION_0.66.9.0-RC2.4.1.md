# Validation — v0.66.9.0-RC2.4.1

## Packaging checks

- [x] `Start-ChaseHQ.ps1` restart cleanup inspected against the reported 37680 timeout.
- [x] Root cause identified: HTTP.sys exposes the listener as PID 4 while the owning `Start-ChaseHQWeb.ps1` process can belong to a different extracted ChaseHQ build.
- [x] Cleanup changed to match stale Workbench hosts by requested `-HttpPort` / backend `HttpPort+1`, not current build path.
- [x] PID 4/System remains protected and is never terminated.
- [x] ChaseHQ Workbench hosts using unrelated ports remain outside the cleanup scope.

## Windows validation gate

- [ ] Leave an older ChaseHQ build's Workbench running on 37680, then run `./Start-ChaseHQ.ps1 -Restart` from RC2.4.1; stale host is stopped and launch continues.
- [ ] `./Start-ChaseHQ.ps1 -Restart` works when restarting an RC2.4.1-recorded session.
- [ ] `./Validate-TC0100SCN-Y.ps1` still passes after Windows build.
