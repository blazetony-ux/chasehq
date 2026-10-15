#!/usr/bin/env python3
"""Fail-closed M1-M7 research automation foundation; never patches emulator code."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import sys

SCHEMA = 'chq-native-auto-pipeline-v1'
STAGES = ('UNKNOWN', 'OBSERVED', 'MAPPED', 'BEHAVIOUR UNDERSTOOD',
          'NATIVE-CANDIDATE', 'SHADOW VERIFIED', 'REPLACEABLE', 'NATIVE')
HEX = re.compile(r'^(?:0x)?[0-9a-fA-F]{1,8}$')
CPU = ('A', 'B')

def load(path):
    return json.loads(Path(path).read_text(encoding='utf-8'))

def atomic_json(path, value):
    path = Path(path); path.parent.mkdir(parents=True, exist_ok=True)
    tmp = path.with_suffix(path.suffix + '.tmp')
    tmp.write_text(json.dumps(value, sort_keys=True, indent=2) + '\n', encoding='utf-8')
    os.replace(tmp, path)

def pc(v):
    if not isinstance(v, str) or not HEX.fullmatch(v):
        raise ValueError('invalid ROM PC')
    n = int(v, 16)
    if n >= 0x80000 or n % 2: raise ValueError('ROM PC outside aligned 512KiB image')
    return n

def source_report(data):
    if data.get('schema') != 'chq-ghidra-static-findings-v1':
        raise ValueError('expected audited Ghidra findings schema')
    if data.get('cpu') not in CPU: raise ValueError('invalid CPU')
    digest = data.get('rawRomSha256', '')
    if not re.fullmatch('[0-9a-f]{64}', digest): raise ValueError('missing ROM provenance')
    if data.get('romBase') != '0x000000' or data.get('romEndExclusive') != '0x080000':
        raise ValueError('unexpected ROM image base/extent')
    sites = data.get('sites')
    if not isinstance(sites, list): raise ValueError('sites required')
    seen = set(); result = []
    for item in sites:
        address = pc(item['pc']); key = (data['cpu'], address)
        if key in seen: raise ValueError('duplicate site')
        seen.add(key)
        candidate = item.get('functionCandidate') or {}
        inst = item.get('nearbyInstructions') or []
        instr_pc = sorted({pc(e['pc']) for e in inst if isinstance(e, dict)})
        if candidate:
            start = pc(candidate['bodyMin']); finish = pc(candidate['bodyMaxInclusive'])
            if start > address or finish < address: raise ValueError('function does not contain site')
        result.append({'id': '%s:%06X' % key, 'cpu': data['cpu'],
                       'pc': '0x%06X' % address, 'staticInstructionPCs': ['0x%06X' % x for x in instr_pc],
                       'functionCandidate': candidate.get('name'), 'confidence': 'hypothesis',
                       'maturity': 'MAPPED' if instr_pc else 'OBSERVED',
                       'source': 'Ghidra unreviewed', 'romSha256': digest})
    return result

def build_ledger(static, dynamic):
    observed = {}
    for sample in dynamic:
        cpu = sample.get('cpu'); digest = sample.get('romSha256')
        if cpu not in CPU or not re.fullmatch('[0-9a-f]{64}', str(digest)):
            raise ValueError('dynamic census lacks CPU or ROM provenance')
        address = pc(sample['pc']); hits = sample['hits']
        if type(hits) is not int or hits < 0: raise ValueError('invalid hit count')
        key = (cpu, digest, address)
        if key in observed: raise ValueError('duplicate dynamic PC (no double counting)')
        observed[key] = hits
    rows = []
    for routine in static:
        keys = {pc(v) for v in routine['staticInstructionPCs']}
        seen = {v for v in keys if (routine['cpu'], routine['romSha256'], v) in observed}
        hit_sum = sum(observed[(routine['cpu'], routine['romSha256'], p)] for p in seen)
        rows.append({**routine, 'staticDecodedUnique': len(keys), 'executedUniqueWithinStatic': len(seen),
                     'dynamicExecutionsWithinStatic': hit_sum,
                     'staticDenominatorIsAuthoritative': False,
                     'coverageFraction': None, 'nativeSubstitutedExecutions': 0,
                     'note': 'Ghidra unreviewed local windows; not global ROM code denominator'})
    return {'schema': 'chq-native-coverage-ledger-v1', 'routines': rows,
            'dynamicObservedUnique': len(observed), 'dynamicObservedHits': sum(observed.values()),
            'nativeInstructionsConvertedUnique': None, 'completionPercent': None}

def validate_contracts(rows):
    output=[]
    for c in rows:
        if c.get('cpu') not in CPU or not c.get('routineId'): raise ValueError('contract identity missing')
        for required in ('inputs', 'outputs', 'registerEffects', 'memoryEffects', 'calls', 'cyclePolicy', 'romSha256'):
            if required not in c: raise ValueError('contract missing '+required)
        if c.get('readyForGeneration') and (c.get('unresolvedDependencies') or not c.get('reviewed')):
            raise ValueError('cannot generate unreviewed/unresolved contract')
        output.append({**c, 'status': 'reviewed' if c.get('reviewed') else 'hypothesis'})
    return output

def generate_candidate(contract):
    """Narrow, audited table-lookup generator; refuses any other opcode family."""
    if not (contract.get('reviewed') and contract.get('readyForGeneration') and
            not contract.get('unresolvedDependencies')):
        raise ValueError('generation requires reviewed dependency-free contract')
    if contract.get('kind') != 'signed-i16-lookup':
        raise ValueError('unsupported routine; human translation needed')
    table = contract.get('table')
    if not isinstance(table, list) or not table or len(table) > 1024 or any(type(x) is not int or not -32768 <= x <= 32767 for x in table):
        raise ValueError('bad reviewed lookup data')
    lo = contract.get('inputMin'); hi = contract.get('inputMax')
    if type(lo) is not int or type(hi) is not int or hi - lo + 1 != len(table):
        raise ValueError('input range != table length')
    values = ', '.join(str(v) for v in table)
    return ('#include <cstdint>\n#include <array>\nnamespace chq_native_auto {\n'
            'static constexpr std::array<int16_t, %d> kTable = {%s};\n' % (len(table), values) +
            'int16_t candidate(int16_t raw) {\n'
            '  if (raw < %d) raw = %d;\n' % (lo, lo) +
            '  if (raw > %d) raw = %d;\n' % (hi, hi) +
            '  return kTable[static_cast<unsigned>(raw - (%d))];\n}\n}\n' % lo)

def verify_cases(contract, cases):
    table=contract['table']; lo=contract['inputMin']; hi=contract['inputMax']
    results=[]
    for item in cases:
        raw=item['raw']; expected=item['originalOutput']
        if type(raw) is not int or type(expected) is not int: raise ValueError('non-integral case')
        actual=table[max(lo,min(hi,raw))-lo]
        results.append({'raw':raw, 'originalOutput':expected, 'nativeOutput':actual,
                        'match': actual==expected, 'source':'offline fixture, not live shadow'})
    return {'schema':'chq-native-differential-v1', 'cases':results,
            'matched':sum(v['match'] for v in results), 'total':len(results),
            'liveShadowVerified':False, 'substitutionAuthorized':False}

def plan_promotion(contract, result):
    # Always fail closed in offline automation. Live CPU/RAM/cycle/bus parity cannot be forged.
    return {'routineId':contract['routineId'], 'authorized':False,
            'reason':'requires independently validated live CPU, cycles, ordered RAM/bus effects, ROM fingerprint, full corpus and fresh Windows qualification',
            'offlineMatched':result['matched'], 'offlineTotal':result['total']}

def orchestrate(manifest, out):
    if manifest.get('schema') != SCHEMA: raise ValueError('wrong manifest schema')
    static=[]
    for filename in manifest.get('ghidraReports',[]): static.extend(source_report(load(filename)))
    dynamic=load(manifest['census']) if manifest.get('census') else []
    if not isinstance(dynamic,list): raise ValueError('census must be list')
    ledger=build_ledger(static,dynamic)
    contracts=validate_contracts(load(manifest['contracts'])) if manifest.get('contracts') else []
    out=Path(out); out.mkdir(parents=True,exist_ok=True)
    atomic_json(out/'coverage-ledger.json',ledger)
    records=[]
    for index, contract in enumerate(sorted(contracts,key=lambda c:(c['cpu'],c['routineId']))):
        rid=contract['routineId']
        if not re.fullmatch(r'[A-Za-z0-9_-]{1,80}',rid): raise ValueError('unsafe routine ID')
        row={'routineId':rid, 'cpu':contract['cpu'], 'status':'blocked', 'attempts':1,
             'maturity':'MAPPED', 'promotionAuthorized':False}
        try:
            code=generate_candidate(contract)
            dest=out/('candidate-%s.cpp'%rid); dest.write_text(code,encoding='utf-8')
            fixtures=contract.get('fixtures',[])
            result=verify_cases(contract,fixtures)
            atomic_json(out/('verify-%s.json'%rid),result)
            atomic_json(out/('promotion-%s.json'%rid),plan_promotion(contract,result))
            row.update(status='offline-pass' if result['total'] and result['total']==result['matched'] else 'offline-fail',
                       maturity='NATIVE-CANDIDATE', offlineCases=result['total'],
                       offlineMatched=result['matched'], candidateSha256=hashlib.sha256(code.encode()).hexdigest())
        except (ValueError,KeyError,TypeError) as error:
            row['reason']=str(error)
        records.append(row)
    atomic_json(out/'queue.json',{'schema':SCHEMA,'records':records,'automaticRetries':0,
        'note':'No self-promoting code, arbitrary compilation or unattended mutation of release code'})
    atomic_json(out/'provenance.json',{'schema':SCHEMA, 'inputSha256':{str(p):hashlib.sha256(Path(p).read_bytes()).hexdigest()
           for p in [*manifest.get('ghidraReports',[]), *([manifest['census']] if manifest.get('census') else []),
                     *([manifest['contracts']] if manifest.get('contracts') else [])]},
           'generatedFrom':'reviewed contracts only; original ROM authoritative'})
    return records

def main():
    ap=argparse.ArgumentParser(description='ChaseHQ native research automation (no auto-promotion)')
    ap.add_argument('command',choices=['run']); ap.add_argument('--manifest',required=True); ap.add_argument('--out',required=True)
    args=ap.parse_args()
    try:
        records=orchestrate(load(args.manifest),args.out)
    except (ValueError,KeyError,TypeError,OSError,json.JSONDecodeError) as exc:
        print('NATIVE_AUTO_FAIL:',str(exc),file=sys.stderr);return 1
    print('NATIVE_AUTO_DONE routines=%d offline_pass=%d promoted=0' %
          (len(records),sum(r['status']=='offline-pass' for r in records)))
    return 0
if __name__=='__main__':sys.exit(main())
