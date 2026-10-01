# Chase H.Q. Native v0.60.1 — Live Research API and `chqctl`

v0.60.1 exposes the v0.60 Research Workbench through a localhost-only command API so an investigation can be driven from a second console without restarting the emulator or rebuilding the source.

## Architecture

`ChaseHQNative.exe` listens on `127.0.0.1:37600` by default. `chqctl.exe` is a small client that sends one command and prints the response. The API is deliberately bound to loopback only; it is not exposed on the LAN.

Change the port with `--debug-api-port N` or disable the listener with `--no-debug-api`.

The same Workbench state is shared between F12 and the API: RAM edits, live patches, watches, research events, baseline snapshots and controlled-frame execution are not separate implementations.

## Basic live loop

```powershell
.\chqctl.exe status
.\chqctl.exe pause
.\chqctl.exe baseline save
.\chqctl.exe read A 10A096 16
.\chqctl.exe write A 10A096 16 01AA
.\chqctl.exe run 10
.\chqctl.exe read A 10A092 16
.\chqctl.exe baseline restore
```

`run N` keeps the machine logically paused but allows exactly N complete emulated frames to execute, matching the Workbench controlled-trial model.

## Commands

### Execution

```text
version
capabilities
status
pause
resume
baseline save
baseline restore
run N
step frame [N]
step instr A|B
```

### Live memory

```text
read A|B ADDR WIDTH
write A|B ADDR WIDTH VALUE
```

WIDTH accepts 8/16/32 (or 1/2/4 bytes). Memory addresses and memory values are hexadecimal by default; an optional `0x` prefix is accepted. Frame counts and other counts are decimal.

### Reversible interventions

```text
patch freeze A|B ADDR WIDTH VALUE
patch suppress A|B ADDR WIDTH
patch list
patch clear
```

`freeze` writes the chosen value immediately and then replaces matching writes. `suppress` preserves the value present when the rule is armed. These use the same v0.60 intervention engine as the F12 Workbench.

### Live watches / trace capture

```text
watch add A|B ADDR WIDTH [read|write|rw] [change] [break]
watch list
watch clear

trace start
trace stop
trace add mem A|B ADDR WIDTH [read|write|rw] [change] [break]
trace list
trace clear
```

The `trace` commands are aliases over the generic Research Event Bus in v0.60.1. They can be added and removed while the emulator is already running. `break` pauses at the next safe frame boundary when the watched event occurs.

### Event queries / trace export

```text
events tail [N]
events clear
events save PATH
sprites tail [N] [semantic]
sprites save PATH
```

This makes it possible to create a watch, run a small window, and save the resulting trace directly from a console without relaunching the emulator.

### Checkpoints and screenshots

```text
checkpoint save PATH
checkpoint load PATH
screenshot [PATH]
```

## Interactive command shell

```powershell
.\chqctl.exe shell
```

Example:

```text
CHQ> pause
CHQ> baseline save
CHQ> read A 10A096 16
CHQ> write A 10A096 16 01AA
CHQ> run 30
CHQ> events tail 10
CHQ> baseline restore
```

Individual commands remain independently copy/pasteable from PowerShell; the shell is optional.

## First intended use

The Stage-1 target/hit-bar investigation can now be driven live:

```powershell
.\chqctl.exe trace start
.\chqctl.exe trace add mem A D00000 16 write change
.\chqctl.exe sprites tail 30 semantic
.\chqctl.exe events tail 30
```

Once a candidate gameplay field is identified, pause, save the baseline, alter it, run a controlled number of frames, inspect sprite/memory events, restore, and repeat.

## v0.60.1 boundaries

- The API currently uses a compact text command protocol over localhost TCP. The internal command/control boundary is intentionally separate from the UI so a structured client/protocol can grow later without changing the Workbench primitives.
- Watch-triggered breaks still stop at the next safe frame boundary, as in v0.60.0; resumable mid-frame execution is future work.
- `trace` in v0.60.1 controls Research Event Bus watches, not the older heavyweight generic trace/provenance configuration. Live reconfiguration of every legacy trace mode remains future work.
- FREEZE/SUPPRESS still use v0.60.0 exact address/width matching.
- The first API release is intentionally local-only and does not include authentication or remote-network exposure.
