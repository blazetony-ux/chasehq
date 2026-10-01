# Target health, damage and defeat — causal proof

**Maturity:** BEHAVIOUR UNDERSTOOD  
**Authoritative live counter:** CPU-A `0x1002AE` (16-bit)

## Damage instruction

On a genuine damaging target interaction, CPU-A PC `0xA112` executes a word decrement of `0x1002AE`. The following `bpl` at `0xA116` selects normal versus terminal response.

```text
A112  subq.w #1, 0x1002AE
A116  bpl     A120
A118  move.w  #0x1400, 0x1002D8   ; terminal
...
A124  add.w   D0, 0x1002D8        ; normal hit
```

## Causal evidence

From `stage1-target-immediate-pre-damage-6333.chqstate`, the next frame is a known authentic damage event.

- Natural: `000E -> 000D`.
- Forced start `0002`: `0002 -> 0001`; normal hit.
- Forced start `0001`: `0001 -> 0000`; normal hit; no defeat.
- Forced start `0000`: `0000 -> FFFF`; BPL is not taken; PC `0xA118` writes `0x1002D8=0x1400`; defeat initialization follows.

Therefore `0xFFFF` is the exhausted/terminal sentinel and `0x0000` means **one final damaging hit remains**.

## Downstream defeat state

Terminal underflow is followed by the already-proven defeat sequence:

- `0x10018D` gains defeat-mode flags (`00 -> 01 -> 05`).
- `0x10A096` converges to defeated target setpoint `0x0080`.
- `0x1002D0` is initialized to `0x0096` (150-frame minimum delay) and counts down via PC `0x77FC`.
- readiness `0x1002C8` becomes necessary for leaving defeat mode.
- completion handoff `0x1002CE` is necessary and sufficient to launch the results worker.
- worker completion `0x100248` precedes intermission gate `0x1002E8`.

## One-hit Target Kill

RC2.9 exposes a research/cheat toggle. It does not freeze the counter. Immediately before authentic damage PC `0xA112`, it arms any live nonterminal counter to zero. The original instruction then performs `0000 -> FFFF` and the original terminal code path executes with authentic CPU flags. This is scoped to the proven special Stage-1 target mechanism.

API: `api game.target.one-hit enabled=true|false`  
Native debugger: `target one-hit status|on|off`  
Startup: `--one-hit-target`
