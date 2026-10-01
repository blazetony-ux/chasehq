# Recipe: verify an input intervention reaches the emulated consumer

1. Start from a deterministic checkpoint.
2. Trace the hardware input register/selector read without intervention.
3. Apply one small intervention and verify the debugger/control layer reports the changed value.
4. Resume and trace the exact CPU-visible hardware read.
5. If the debugger value changes but the CPU-visible read does not, inspect layer ownership/clobbering before interpreting gameplay results.
6. Only classify semantic effects after the intervention is proven at the consumer boundary.

This prevents a tooling failure from being mistaken for a negative reverse-engineering result.
