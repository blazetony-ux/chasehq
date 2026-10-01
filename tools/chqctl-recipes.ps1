# ChaseHQ-Native v0.60.2 live API examples.
# Run ChaseHQNative first, then paste individual commands from this file.
$ctl = ".\out\build\x64-Debug\chqctl.exe"

# Inspect the live machine.
& $ctl status
& $ctl regs A
& $ctl input ports

# Safe reversible experiment loop.
& $ctl pause
& $ctl snapshot save baseline
& $ctl write A 10A096 16 01AA
& $ctl --wait run 30
& $ctl why mem A 10A096
& $ctl snapshot restore baseline

# Live heavyweight trace without relaunching.
& $ctl legacytrace reset
& $ctl legacytrace cpu A
& $ctl legacytrace mem 10A080:10A0BF w change
& $ctl legacytrace pc 007700:007850
& $ctl legacytrace context 256 512
& $ctl legacytrace output ".\evidence\live_target_trace.log"
& $ctl legacytrace start

# Live provenance.
& $ctl provenance reset
& $ctl provenance follow 10A080:10A0BF
& $ctl provenance access rw
& $ctl provenance callstack on
& $ctl provenance output ".\evidence\live_target_provenance"
& $ctl provenance start
