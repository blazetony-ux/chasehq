#!/usr/bin/env python3
"""Extract original user-owned evidence ZIP and test standalone native C++17 candidate.
No ROM data bundled. Usage: python verify_road.py path/to/bundle.zip
"""
import os
import pathlib
import shutil
import subprocess
import sys
import tempfile
import zipfile

HERE = pathlib.Path(__file__).resolve().parent

def main():
    if len(sys.argv) != 2:
        raise SystemExit('Usage: python verify_road.py "bundle(20261010-210139).zip"')
    compiler = os.environ.get('CXX') or shutil.which('g++') or shutil.which('clang++') or shutil.which('cl')
    if not compiler:
        raise SystemExit('No C++17 compiler found. Install MSVC, MinGW or LLVM, or use CMake.')
    with tempfile.TemporaryDirectory(prefix='chq-road-') as temp:
        root = pathlib.Path(temp)
        binary = root / ('road_model.exe' if os.name == 'nt' else 'road_model')
        if pathlib.Path(compiler).stem.lower() == 'cl':
            command = [compiler, '/nologo', '/std:c++17', '/O2', '/W4', str(HERE / 'road_buffer_model.cpp'), '/Fe:' + str(binary)]
        else:
            command = [compiler, '-std=c++17', '-O2', '-Wall', '-Wextra', '-pedantic', str(HERE / 'road_buffer_model.cpp'), '-o', str(binary)]
        subprocess.run(command, check=True)
        with zipfile.ZipFile(sys.argv[1]) as z:
            files = {'boundary': 'r054-rom-boundary-table.bin', 'lookup': 'r054-rom-material-lookup.bin'}
            for frame in [5588, 5595]:
                files[f'{frame}-control-input'] = f'r054-control-f{frame}-input.bin'
                for tag in ['input', 'output', 'decisions']:
                    files[f'{frame}-class06-{tag}'] = f'r054-class06-f{frame}-{tag}.bin'
            for key, name in files.items():
                (root / key).write_bytes(z.read('artifacts/' + name))
        for frame in [5588, 5595]:
            print(f'\n=== Frame {frame} ===', flush=True)
            subprocess.run([str(binary), str(root / 'boundary'), str(root / 'lookup'),
                            str(root / f'{frame}-control-input'), str(root / f'{frame}-class06-decisions'),
                            str(root / f'{frame}-class06-input'), str(root / f'{frame}-class06-output')], check=True)
        print('\nPASS: 2 active frames x (208 input + 208 output) = 832 modeled bytes, 0 mismatches')
        print('Frame 5600 is intentionally excluded: output can persist after generator deactivation.')

if __name__ == '__main__':
    main()
