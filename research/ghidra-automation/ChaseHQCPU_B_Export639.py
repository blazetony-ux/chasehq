# -*- coding: utf-8 -*-
#@category ChaseHQ.NativeReconstruction
#@runtime Jython
# CPU-B REAL-ROM read-only static discovery: no inferred complete function status.
import io, json, os
args=list(getScriptArgs())
if len(args)!=7:
    raise RuntimeError("Expected evidence directory, SHA256, and five PC:opcode fingerprints")
outdir, digest=args[:2]
if str(currentProgram.getLanguageID())!='68000:BE:32:default':
    raise RuntimeError('Wrong processor language for CPU-B')
mem=currentProgram.getMemory()
if mem.getMinAddress().getOffset()!=0 or mem.getMaxAddress().getOffset()!=0x1ffff:
    raise RuntimeError('CPU-B ROM MUST be mapped to 0x000000..0x01FFFF')
import java.security.MessageDigest as MD
hasher=MD.getInstance('SHA-256')
# Ghidra Jython signed bytes: digest update(int) uses low eight bits.
for off in xrange(0x20000):
    hasher.update(mem.getByte(toAddr(off)))
actual=''.join('%02x'%(int(b)&0xff) for b in hasher.digest())
if actual.lower()!=digest.lower():
    raise RuntimeError('CPU-B Ghidra imported bytes mismatch raw SHA256')
listing=currentProgram.getListing()
fm=currentProgram.getFunctionManager()
rows=[]
for token in args[2:]:
    if ':' not in token:
        raise RuntimeError('Invalid site/fingerprint '+token)
    textpc, textopcode=token.split(':',1)
    pc=int(textpc,16); expected=int(textopcode,16)
    if pc<8 or pc>=0x20000 or pc&1:
        raise RuntimeError('Site outside CPU-B executable ROM')
    got=((int(mem.getByte(toAddr(pc)))&0xff)<<8)|(int(mem.getByte(toAddr(pc+1)))&0xff)
    if got!=expected:
        raise RuntimeError('CPU-B opcode mismatch at 0x%06x' % pc)
    ins=listing.getInstructionAt(toAddr(pc))
    if ins is None:
        disassemble(toAddr(pc))
        ins=listing.getInstructionAt(toAddr(pc))
    if ins is None or int(ins.getLength())<2:
        raise RuntimeError('Ghidra could not disassemble CPU-B 0x%06x' % pc)
    flows=[]
    for dest in ins.getFlows():
        flows.append('0x%06X'%int(dest.getOffset()))
    fun=fm.getFunctionContaining(toAddr(pc))
    rows.append({'pc':'0x%06X'%pc,'opcode':'0x%04X'%got,'text':str(ins),
                 'length':int(ins.getLength()),'flowTargets':flows,
                 'functionCandidate':str(fun.getName()) if fun is not None else None,
                 'functionReviewed':False})
# RC6.3.18: independently disassemble five original-ROM *continuation*
# addresses justified by the CPU-B observed edge trace. These are not an
# enclosing function; instruction lengths and branch flows are authenticated
# by the Go crosswalk against five original-ROM checkpoint recordings.
frontier=[]
for pc,expected in [(0x4e0,0x4279),(0x4e6,0x5279),(0x4ec,0x3039),(0x516,0x6614),(0x52c,0x60aa)]:
    got=((int(mem.getByte(toAddr(pc)))&0xff)<<8)|(int(mem.getByte(toAddr(pc+1)))&0xff)
    if got != expected:
        raise RuntimeError('CPU-B frontier ROM opcode mismatch at 0x%06x'%pc)
    ins=listing.getInstructionAt(toAddr(pc))
    if ins is None:
        disassemble(toAddr(pc))
        ins=listing.getInstructionAt(toAddr(pc))
    if ins is None or int(ins.getLength())<2:
        raise RuntimeError('Ghidra could not disassemble CPU-B frontier 0x%06x'%pc)
    fall=ins.getFallThrough()
    fun=fm.getFunctionContaining(toAddr(pc))
    frontier.append({'pc':'0x%06X'%pc, 'opcode':'0x%04X'%got,
                     'text':str(ins), 'length':int(ins.getLength()),
                     'flowTargets':['0x%06X'%int(d.getOffset()) for d in ins.getFlows()],
                     'fallThrough':'0x%06X'%int(fall.getOffset()) if fall is not None else None,
                     'functionCandidate':str(fun.getName()) if fun is not None else None,
                     'functionReviewed':False})

# RC6.3.19: verify the ORIGINAL-ROM JSR/BTST/RTS sites that are linked
# by observed CPU-B call/return edges in all five checkpoints. The JSR is
# indirect; a sampled destination is never a claim of an exhaustive callee set.
call_return=[]
for pc,expected in [(0x050c,0x4e91),(0x050e,0x0839),(0x08ac,0x4e75)]:
    got=((int(mem.getByte(toAddr(pc)))&0xff)<<8)|(int(mem.getByte(toAddr(pc+1)))&0xff)
    if got!=expected:
        raise RuntimeError('CPU-B call/return ROM opcode mismatch at 0x%06x'%pc)
    ins=listing.getInstructionAt(toAddr(pc))
    if ins is None:
        disassemble(toAddr(pc))
        ins=listing.getInstructionAt(toAddr(pc))
    if ins is None or int(ins.getLength())<2:
        raise RuntimeError('CPU-B call/return instruction unresolvable at 0x%06x'%pc)
    ft=ins.getFallThrough()
    call_return.append({'pc':'0x%06X'%pc,'opcode':'0x%04X'%got,
         'text':str(ins),'length':int(ins.getLength()),
         'flowTargets':['0x%06X'%int(x.getOffset()) for x in ins.getFlows()],
         'fallThrough':'0x%06X'%int(ft.getOffset()) if ft is not None else None,
         'functionReviewed':False})
# 0x0518 is a *statically reachable BNE fallthrough* only. No current
# original-ROM checkpoint observed executing this address. Do not invent an
# opcode, next PC, side effects, or native-function coverage for this path.
unknown_pc=0x0518
unknown_word=((int(mem.getByte(toAddr(unknown_pc)))&0xff)<<8)|(int(mem.getByte(toAddr(unknown_pc+1)))&0xff)
unknown_ins=listing.getInstructionAt(toAddr(unknown_pc))
if unknown_ins is None:
    disassemble(toAddr(unknown_pc))
    unknown_ins=listing.getInstructionAt(toAddr(unknown_pc))
unknown_fall=unknown_ins.getFallThrough() if unknown_ins is not None else None
unobserved={'pc':'0x000518','rawOpcode':'0x%04X'%unknown_word,
            'disassemblyAvailable':unknown_ins is not None,
            'text':str(unknown_ins) if unknown_ins is not None else None,
            'length':int(unknown_ins.getLength()) if unknown_ins is not None else None,
            'flowTargets':['0x%06X'%int(x.getOffset()) for x in unknown_ins.getFlows()] if unknown_ins is not None else [],
            'fallThrough':'0x%06X'%int(unknown_fall.getOffset()) if unknown_fall is not None else None,
            'originalRuntimeObserved':False,'completeFunctionReviewed':False}

# RC6.3.21: read-only *nested* BSR/RTS sites supported by genuine
# original-ROM CPU-B edge observations. BSR returns are not whole-function
# proof; no dynamic stack trace or native function promotion is inferred.
nested_return=[]
for pc, expected in [(0x4f70,0x6100),(0x4f74,0x6100),(0x4f78,0x6100),(0x4f7c,0x4e75),(0x5062,0x4e75)]:
    got=((int(mem.getByte(toAddr(pc)))&0xff)<<8)|(int(mem.getByte(toAddr(pc+1)))&0xff)
    if got!=expected:
        raise RuntimeError('CPU-B nested call/return ROM opcode mismatch at 0x%06x'%pc)
    ins=listing.getInstructionAt(toAddr(pc))
    if ins is None:
        disassemble(toAddr(pc))
        ins=listing.getInstructionAt(toAddr(pc))
    if ins is None or int(ins.getLength())<2:
        raise RuntimeError('CPU-B nested call/return unresolved at 0x%06x'%pc)
    ft=ins.getFallThrough()
    nested_return.append({'pc':'0x%06X'%pc,'opcode':'0x%04X'%got,
        'text':str(ins),'length':int(ins.getLength()),
        'flowTargets':['0x%06X'%int(x.getOffset()) for x in ins.getFlows()],
        'fallThrough':'0x%06X'%int(ft.getOffset()) if ft is not None else None,
        'functionReviewed':False})

output={'schema':'chq-rc639-cpu-b-ghidra-sites-v1','cpu':'B',
        'language':'68000:BE:32:default','rawRomSha256':actual,
        'romBase':'0x000000','romEndExclusive':'0x020000',
        'staticSource':'Ghidra 68000 headless analysis of independently CRC-verified ORIGINAL CPU-B ROM',
        'verificationScope':'Five authentic opcode sites; function boundaries UNREVIEWED',
        'sites':rows,'frontierSites':frontier,'callReturnSites':call_return,'nestedReturnSites':nested_return,'unobservedFallthrough':unobserved,'nativePromotionAllowed':False}
if not os.path.isdir(outdir):
    os.makedirs(outdir)
with io.open(os.path.join(outdir,'cpu-b-ghidra-sites.json'),'w',encoding='utf-8') as f:
    f.write(unicode(json.dumps(output,sort_keys=True,indent=2))+'\n')
print('CHQ_RC639_CPU_B_GHIDRA_OK sites=%d frontier=%d callReturn=%d nestedReturn=%d sha256=%s'%(len(rows),len(frontier),len(call_return),len(nested_return),actual))
