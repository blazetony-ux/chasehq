# -*- coding: utf-8 -*-
#@category ChaseHQ.NativeReconstruction
#@runtime Jython
# Ghidra headless POST-analysis script. Export *candidate* function ranges,
# instruction evidence and static XREFs without implying review or verification.
# Outputs the existing RC4.5 chq-rea-static-export-v1 schema.

import io
import os
import json
import traceback

args = list(getScriptArgs())
if len(args) < 2:
    raise RuntimeError("Usage: ChaseHQExport68000.py OUTPUT_DIRECTORY EXPECTED_ROM_SHA256 [CANDIDATE_PC...]")
out_dir, sha = args[:2]
sha = sha.lower()
if len(sha) != 64 or any(c not in '0123456789abcdef' for c in sha):
    raise RuntimeError("64-character ROM SHA-256 is required")
if not os.path.isdir(out_dir):
    os.makedirs(out_dir)

language = str(currentProgram.getLanguageID())
if language != '68000:BE:32:default':
    raise RuntimeError('Wrong processor: ' + language)
mem = currentProgram.getMemory()
if (mem.getMinAddress().getOffset() != 0 or
        mem.getMaxAddress().getOffset() != 0x7ffff):
    raise RuntimeError('Unexpected 512-KiB ROM memory extent')

sites = [
    ('health_decrement_A112', 0x00A112),
    ('steering_conversion_writer_0820C', 0x00820C),
    ('hardware_sprite_copy_40540', 0x040540),
    ('cache_copy_40682', 0x040682),
    ('projection_clip_434EE', 0x0434EE),
    ('projection_lookup_4351C', 0x04351C),
    ('geometry_table_reader_4359A', 0x04359A),
    ('lateral_consumer_435D6', 0x0435D6),
    ('prepared_sprite_writer_43828', 0x043828)
]
# Candidate PCs are constrained to CPU-A 512-KiB ROM, even aligned, unique.
# Never treat an operator-supplied instruction PC as a verified function entry.
seen = set(pc for name, pc in sites)
for token in args[2:]:
    if not token.startswith('0x') or len(token) > 8:
        raise RuntimeError('Invalid candidate PC: ' + token)
    try:
        pc = int(token[2:], 16)
    except ValueError:
        raise RuntimeError('Invalid candidate PC: ' + token)
    if pc < 8 or pc >= 0x80000 or pc % 2:
        raise RuntimeError('Candidate PC out of CPU-A ROM bounds/alignment: ' + token)
    if pc not in seen:
        sites.append(('requested_cpuA_%06X' % pc, pc))
        seen.add(pc)
fm = currentProgram.getFunctionManager()
listing = currentProgram.getListing()
refs = currentProgram.getReferenceManager()

def hx(v):
    return '0x%06X' % int(v)

def describe_site(name, pc):
    addr = toAddr(pc)
    insn = listing.getInstructionAt(addr)
    if insn is None:
        # Headless auto analysis sometimes misses disconnected ROM islands.
        disassemble(addr)
        insn = listing.getInstructionAt(addr)
    function = fm.getFunctionContaining(addr)
    function_details = None
    if function is not None:
        body = function.getBody()
        function_details = {
            'name': str(function.getName()),
            'entry': hx(function.getEntryPoint().getOffset()),
            'bodyMin': hx(body.getMinAddress().getOffset()),
            'bodyMaxInclusive': hx(body.getMaxAddress().getOffset()),
            'isEntryAtSite': function.getEntryPoint().equals(addr),
            'rangeCount': int(body.getNumAddressRanges()),
            'source': 'ghidra-auto-analysis-unreviewed'
        }
    xrefs = []
    iterator = refs.getReferencesTo(addr)
    for reference in iterator:
        if len(xrefs) >= 80:
            break
        xrefs.append({
            'from': hx(reference.getFromAddress().getOffset()),
            'type': str(reference.getReferenceType()),
            'source': str(reference.getSource())
        })
    # Export independent static control-flow/reference provenance without
    # converting an automatically discovered CFG edge into verified ownership.
    outgoing = []
    control_edges = []
    if insn is not None:
        for reference in refs.getReferencesFrom(addr):
            if len(outgoing) >= 80:
                break
            outgoing.append({'to': hx(reference.getToAddress().getOffset()),
                             'type': str(reference.getReferenceType()),
                             'source': str(reference.getSource())})
    insts = []
    offset = max(0, pc - 16)
    while offset <= min(0x7ffff, pc + 30):
        inst = listing.getInstructionAt(toAddr(offset))
        if inst is not None:
            flows = []
            for target in inst.getFlows():
                if target.getOffset() < 0x80000:
                    flows.append(hx(target.getOffset()))
                    if len(control_edges) < 96:
                        control_edges.append({'from': hx(offset),
                                              'to': hx(target.getOffset()),
                                              'flowType': str(inst.getFlowType()),
                                              'confidence': 'Ghidra static disassembly; unreviewed'})
            insts.append({'pc': hx(offset), 'text': str(inst),
                          'length': int(inst.getLength()),
                          'flowType': str(inst.getFlowType()),
                          'flows': flows})
            offset += max(2, int(inst.getLength()))
        else:
            offset += 2
    return {
        'site': name, 'pc': hx(pc),
        'instruction': str(insn) if insn is not None else None,
        'instructionLength': int(insn.getLength()) if insn is not None else None,
        'functionCandidate': function_details,
        'incomingReferences': xrefs,
        'outgoingReferences': outgoing,
        'localControlFlowEdges': control_edges,
        'nearbyInstructions': insts
    }

findings = [describe_site(name, pc) for name, pc in sites]

# Select auto-discovered contiguous, non-overlapping function bodies where possible.
# Otherwise represent only the independently known PC, using a small site window.
# Every emitted range stays a HYPOTHESIS, even when Ghidra created a function.
records = []
used = []
for finding in findings:
    site = finding['site']
    pc = int(finding['pc'], 16)
    candidate = finding['functionCandidate']
    start = pc
    end = pc + max(2, finding['instructionLength'] or 2)
    reason = 'known-instruction-site-window; not a function boundary'
    if candidate is not None and candidate['rangeCount'] == 1:
        c_start = int(candidate['bodyMin'], 16)
        c_end = int(candidate['bodyMaxInclusive'], 16) + 1
        if (c_start <= pc < c_end and c_start % 2 == 0 and c_end % 2 == 0
                and 0 <= c_start < c_end <= 0x80000 and c_end - c_start <= 0x4000):
            start, end = c_start, c_end
            reason = 'unreviewed-ghidra-contiguous-function-candidate'
    # Avoid overlap (the REA adapter explicitly rejects overlapping windows).
    if any(start < e and s < end for s, e in used):
        start, end = pc, pc + max(2, finding['instructionLength'] or 2)
        reason = 'known-instruction-site-fallback-due-to-function-overlap'
        if any(start < e and s < end for s, e in used):
            finding['reaWindowNote'] = 'covered by an earlier candidate; omitted duplicate'
            continue
    # 68000 addresses must be word-aligned in the crosswalk adapter.
    end = (end + 1) & ~1
    if end > 0x80000 or start % 2 or end <= start:
        finding['reaWindowNote'] = 'unusable/misaligned range; omitted'
        continue
    used.append((start, end))
    records.append({
        'name': 'ghidra_candidate_' + site,
        'start': hx(start),
        'end': hx(end),
        'confidence': 'hypothesis'
    })
    finding['reaWindowNote'] = reason

records.sort(key=lambda x: int(x['start'], 16))
if not records:
    raise RuntimeError('No candidate ROM windows generated')

export = {
    'schema': 'chq-rea-static-export-v1',
    'cpu': 'A',
    'romSha256': sha,
    'imageBase': '0x000000',
    'functions': records
}
report = {
    'schema': 'chq-ghidra-static-findings-v1',
    'cpu': 'A',
    'language': language,
    'rawRomSha256': sha,
    'romBase': '0x000000',
    'romEndExclusive': '0x080000',
    'source': 'Ghidra 68000 headless disassembly and automatic analysis',
    'verificationStatus': 'unreviewed - function ownership and edges remain hypotheses',
    'note': 'No original ROM bytes are exported',
    'sites': findings,
    'exportedWindows': len(records)
}

def dump(path, obj):
    with io.open(path, 'w', encoding='utf-8') as stream:
        stream.write(unicode(json.dumps(obj, indent=2, sort_keys=True, ensure_ascii=True)))
        stream.write(u'\n')

dump(os.path.join(out_dir, 'chq-ghidra-static.json'), export)
dump(os.path.join(out_dir, 'chq-ghidra-findings.json'), report)
print('CHQ_GHIDRA_EXPORT_OK windows=%d sites=%d output=%s' %
      (len(records), len(findings), out_dir))
