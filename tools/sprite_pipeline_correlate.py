#!/usr/bin/env python3
"""Opt-in, offline staged-sprite evidence correlator (no ROM/emulator changes).

Input is an explicit, operator-reviewed event export. This program never infers
identity solely from the hardware slot, because slots change between frames.
"""
import argparse
import json
from pathlib import Path


def correlate(payload):
    if payload.get('schema') != 'chq-sprite-pipeline-events-v1':
        raise ValueError('unsupported event schema')
    prepared = payload.get('prepared', [])
    copied = payload.get('hardware', [])
    shown = payload.get('presented', [])
    for kind, entries in [('prepared', prepared), ('hardware', copied), ('presented', shown)]:
        if not isinstance(entries, list):
            raise ValueError(f'{kind} must be an array')
        for event in entries:
            if not all(k in event for k in ('frame', 'map', 'x')):
                raise ValueError(f'{kind} event requires frame/map/x')
    matches = []
    for p in prepared:
        cands = [h for h in copied if h['frame'] >= p['frame']
                 and h['map'] == p['map'] and h['x'] == p['x']]
        if not cands:
            matches.append({'prepared': p, 'status': 'unresolved-no-hardware-match'})
            continue
        h = min(cands, key=lambda x: (x['frame'], x.get('slot', -1)))
        snapshots = [v for v in shown if v['frame'] >= h['frame']
                     and v['map'] == p['map'] and v['x'] == p['x']
                     and ('slot' not in h or h['slot'] == v.get('slot'))]
        if not snapshots:
            matches.append({'prepared': p, 'hardware': h,
                            'status': 'unresolved-no-presented-match'})
            continue
        v = min(snapshots, key=lambda x: (x['frame'], x.get('slot', -1)))
        matches.append({'prepared': p, 'hardware': h, 'presented': v,
                        'lagToHardware': h['frame'] - p['frame'],
                        'lagToPresentation': v['frame'] - p['frame'],
                        'status': 'correlated-by-map-x-and-temporal-order'})
    return {'schema': 'chq-sprite-pipeline-correlation-v1',
            'scope': 'observed input events; not global sprite identity',
            'warning': 'same map/X may be ambiguous; a correlation is not proof of unique ownership',
            'matches': matches}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--input', required=True, type=Path)
    parser.add_argument('--out', required=True, type=Path)
    args = parser.parse_args()
    result = correlate(json.loads(args.input.read_text(encoding='utf-8')))
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')
    print(f'Correlated {len(result["matches"])} prepared records; evidence written to {args.out}')


if __name__ == '__main__':
    main()
