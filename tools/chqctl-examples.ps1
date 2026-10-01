# ChaseHQ-Native v0.60.1 live Research API examples.
# Run ChaseHQNative.exe first, then paste commands from another PowerShell window.

.\out\build\x64-Debug\chqctl.exe status
.\out\build\x64-Debug\chqctl.exe pause
.\out\build\x64-Debug\chqctl.exe baseline save
.\out\build\x64-Debug\chqctl.exe read A 10A096 16
.\out\build\x64-Debug\chqctl.exe write A 10A096 16 01AA
.\out\build\x64-Debug\chqctl.exe run 10
.\out\build\x64-Debug\chqctl.exe events tail 20
.\out\build\x64-Debug\chqctl.exe baseline restore

# Arm a live change-triggered break without restarting the emulator.
.\out\build\x64-Debug\chqctl.exe trace start
.\out\build\x64-Debug\chqctl.exe trace add mem A 10A096 16 write change break

# Review semantic sprite changes only.
.\out\build\x64-Debug\chqctl.exe sprites tail 30 semantic
