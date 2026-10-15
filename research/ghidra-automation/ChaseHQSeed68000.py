# -*- coding: utf-8 -*-
#@category ChaseHQ.NativeReconstruction
#@runtime Jython
# Ghidra headless PRE-analysis script. Seed legitimate instruction addresses,
# but only create a function at the verified 68000 reset-vector entry.
# No guesses about game-function entry points are promoted into function symbols.

from ghidra.program.model.symbol import SourceType

if str(currentProgram.getLanguageID()) != "68000:BE:32:default":
    raise RuntimeError("CPU-A analysis requires 68000:BE:32:default")

if currentProgram.getMemory().getMinAddress().getOffset() != 0:
    raise RuntimeError("CPU-A image must be mapped at ROM base 0x000000")

rom = currentProgram.getMemory()
header = [0x00,0x10,0x3f,0x00,0x00,0x00,0x01,0x40]
for i, v in enumerate(header):
    b = int(rom.getByte(toAddr(i))) & 0xff
    if b != v:
        raise RuntimeError("Unexpected CPU-A reset-vector byte at 0x%X" % i)

sites = [0x000140, 0x00A112, 0x00820C, 0x040540, 0x040682, 0x0434EE,
         0x04351C, 0x04359A, 0x0435D6, 0x043828]
# Additional validated instruction sites are passed by the launcher.
for token in list(getScriptArgs()):
    if not token.startswith('0x') or len(token) > 8:
        raise RuntimeError('Invalid candidate PC: ' + token)
    try:
        pc = int(token[2:], 16)
    except ValueError:
        raise RuntimeError('Invalid candidate PC: ' + token)
    if pc < 8 or pc >= 0x80000 or pc % 2:
        raise RuntimeError('Candidate PC out of CPU-A ROM bounds/alignment: ' + token)
    if pc not in sites:
        sites.append(pc)
for offset in sites:
    addr = toAddr(offset)
    if not rom.contains(addr):
        raise RuntimeError("Known site outside imported CPU-A ROM: 0x%06X" % offset)
    # These addresses are independently recorded as 68000 instruction PCs.
    # Disassembling at them does NOT establish that they begin functions.
    if currentProgram.getListing().getInstructionAt(addr) is None:
        success = disassemble(addr)
        print("CHQ_SEED pc=0x%06X disassembled=%s" % (offset, str(success)))

reset = toAddr(0x140)
if currentProgram.getFunctionManager().getFunctionAt(reset) is None:
    try:
        f = createFunction(reset, "cpuA_reset_0140")
        print("CHQ_SEED reset_function=%s" % (f.getName() if f is not None else "unresolved"))
    except Exception as err:
        print("CHQ_SEED reset_function_error=%s" % str(err))

print("CHQ_SEED complete: mapped CPU-A header and known instruction sites")
