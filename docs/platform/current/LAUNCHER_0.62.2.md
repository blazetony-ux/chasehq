# v0.62.2 launcher hotfix

## Fixed

`Start-ChaseHQ.ps1` v0.62.1 accidentally passed the child research launcher's named parameters as an array. PowerShell treats array splatting positionally, causing `-Checkpoint` to be bound to the integer `-Port` parameter.

v0.62.2 uses hashtable splatting so `Port`, `Roms`, `SessionName`, `Checkpoint`, `PrimeCommand`, `EmulatorArgs`, and `Attach` are bound by name.

## Canonical startup

```powershell
.\Start-ChaseHQ.ps1
```

This defaults to the Stage 1 player-gameplay checkpoint and starts the Web Workbench.

## Windows downloaded-ZIP security prompt

Windows can mark scripts extracted from an Internet-downloaded ZIP. To remove repeated prompts for this trusted local project copy, run once from the project root:

```powershell
Get-ChildItem -Recurse -File | Unblock-File
```

This changes only Windows' Mark-of-the-Web metadata; it does not alter script contents.
