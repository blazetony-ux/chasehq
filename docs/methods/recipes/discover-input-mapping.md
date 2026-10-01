# Recipe — discover unknown input mapping

1. Observe raw physical controller/keyboard state separately from emulated input state.
2. Change exactly one physical control at a time.
3. Record IOC/device port reads/writes and game-state consequences.
4. Distinguish momentary/pulse inputs from held inputs and analogue values from thresholds.
5. Repeat press/release cycles to establish polarity and debounce/pulse requirements.
6. Map physical -> browser/input mapper -> emulated device -> game-observed state.
7. Save the mapping as a profile only after repeated confirmation.
