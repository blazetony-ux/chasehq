# RC3.2 LAN / Mobile Script Console

RC3.2 treats LAN/mobile access as a first-class one-command-launch feature.

## Default launch

```powershell
.\Start-ChaseHQ.ps1 -Restart
```

The launcher now:

1. resolves the preferred non-loopback IPv4 address from the adapter with the default gateway (falling back to DNS host addresses);
2. generates a per-session access token unless `-AccessToken` was explicitly supplied;
3. verifies the Windows HTTP namespace and Private-profile firewall prerequisite for the public Workbench port;
4. on first use only, requests elevation and creates the required HTTP URL reservation/firewall rule;
5. starts native/private services on loopback only;
6. exposes only the authenticated public Workbench front listener on the LAN address;
7. prints ready-to-use full Workbench and `/console` URLs;
8. performs an authenticated host-side `/api/health` + `/console` smoke before reporting READY.

Use `-LocalOnly` to disable LAN/mobile exposure explicitly. `-LanAccess` remains accepted for backward compatibility but is no longer required by the normal one-command launcher.

## Security boundary

The native Research API (`37600`) and private Workbench backend (`HttpPort+1`, normally `37681`) remain bound to `127.0.0.1`. Only the public front listener (`37680` by default) is exposed. Remote requests require the session token. The automatic firewall rule is restricted to the Windows **Private** network profile.

## Windows HTTP.sys prerequisite

`System.Net.HttpListener` cannot bind a non-loopback prefix for a standard user unless HTTP.sys grants that namespace. `scripts/Ensure-ChaseHQLanAccess.ps1` owns this prerequisite. Do not manually hard-code an IP into release instructions. The helper detects the current address/port, requests UAC elevation only when required, creates an HTTP URL reservation and ensures the Private-profile inbound TCP firewall rule.

If the machine LAN address changes, the launcher revalidates the effective listener before starting the emulator.

## Regression / release proof

`regression/regression-v06690-rc32.chqscript` is the RC3.2 whole-release script regression. It proves script parameters plus run-local trace/checkpoint evidence and final-state restoration. Host transport is a separate layer: every default launch performs the authenticated LAN health/mobile smoke, and Windows promotion additionally requires a second-device `/console` check.

RC3.2 LAN ACL correction: the elevated prerequisite phase must never use an elevated HttpListener bind as evidence that the normal user owns the URL namespace. It now ensures and verifies the exact `http://<resolved-lan-ip>:37680/` URL ACL for the original user, then the parent non-elevated process performs the final bind proof.
