# RC3.1 content source controls (candidate)

`chqdev content mode rom|pack|auto` saves a per-extraction choice in `.chq/content-source.json`. Default is `rom` for compatibility. `auto` prefers an existing CHQPACK and falls back to the ROM directory; explicit modes never silently fall back. `chqdev content status` displays configured and effective modes; `content import` invokes the Windows debug importer; `content verify` checks source presence (runtime verifies pack integrity during load). `chqdev runtime start --content pack` overrides the persisted selection for that invocation. Runtime launch still bridges the Windows PowerShell host. The ROM-directory validation in Start-ChaseHQResearch.ps1 now applies only to ROM mode.

Prior RC3 evidence: 2064-to-2100 checkpoint execution matched reported CPU/hardware counters between ROM and CHQPACK; frame PNGs 2065, 2080, 2100 and final hardware RAM dumps matched. This is not proof of native game-logic replacement.

Pending Windows qualification: full fresh build, 12+ CTests, ROM-free launcher with Workbench/mobile, parity regression integration, and Workbench readiness latency investigation. Do not promote until these pass.
